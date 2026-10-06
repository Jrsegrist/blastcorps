/* Threads, message queues, events, timers and the virtual clock.
 *
 * Model: libultra's single-CPU priority scheduler, run cooperatively.  Every
 * game thread is a host fiber; exactly one runs at a time, and control only
 * changes hands where libultra would switch threads:
 *   - a thread blocks (osRecvMesg on an empty queue, osSendMesg on a full one),
 *   - a thread readies a higher-priority thread (osStartThread, osSendMesg,
 *     osSetThreadPri),
 *   - an "interrupt" (VI retrace, timer, RSP/RDP done) readies a thread that
 *     outranks the running one.
 * Run queues are ordered by priority, FIFO among equals, as in libultra.
 *
 * Time is virtual (count units, 46.875 MHz) and only moves when every game
 * thread is blocked (or a busy-wait calls port_spin): the clock then jumps
 * to the next pending event (retrace, timer, task completion).  Running game
 * code takes no virtual time; task durations (PlatConfig *_cycles) model the
 * RCP.  So a run is a pure function of the ROM, the options and the input.
 *
 * The idle thread (priority 0, `while (1) {}`) is parked when it lowers
 * itself to OS_PRIORITY_IDLE: the host loop plays its part. */
#include "plat.h"

#define MAX_THREADS 16

typedef struct {
    OSThread *t;
    HostFiber fiber;
    void (*entry)(void *);
    void *arg;
    int alive;
} HostThread;

static HostThread g_threads[MAX_THREADS];
static HostFiber g_main_fiber;
static OSThread *g_running;   /* NULL while the host loop runs */
static OSThread *g_runq;      /* ready threads, highest priority first */
static OSThread *g_active;    /* all created threads (tlnext) */
static OSThread *g_idle;      /* parked idle thread */

u64 plat_now;
u32 plat_vi_count;

OSThread *plat_running(void) {
    return g_running;
}

static HostThread *host_of(OSThread *t) {
    int i;
    for (i = 0; i < MAX_THREADS; i++)
        if (g_threads[i].t == t) return &g_threads[i];
    return NULL;
}

const char *plat_thread_name(OSThread *t) {
    static char buf[32];
    if (t == NULL) return "host";
    /* "id@addr" */
    {
        char *p = buf;
        u32 id = (u32) t->id, a = (u32) t, i;
        const char *hex = "0123456789ABCDEF";
        if (id >= 10) *p++ = '0' + (id / 10) % 10;
        *p++ = '0' + id % 10;
        *p++ = '@';
        for (i = 0; i < 8; i++) *p++ = hex[(a >> (28 - 4 * i)) & 0xF];
        *p = 0;
    }
    return buf;
}

/* ---- queues ------------------------------------------------------------ */

/* insert behind every thread of the same or higher priority */
static void enqueue(OSThread **q, OSThread *t) {
    OSThread **pp = q;
    while (*pp != NULL && (*pp)->priority >= t->priority) pp = &(*pp)->next;
    t->next = *pp;
    *pp = t;
    t->queue = q;
}

static OSThread *pop(OSThread **q) {
    OSThread *t = *q;
    if (t != NULL) {
        *q = t->next;
        t->next = NULL;
        t->queue = NULL;
    }
    return t;
}

static void unlink_thread(OSThread *t) {
    OSThread **pp = t->queue;
    if (pp == NULL) return;
    while (*pp != NULL && *pp != t) pp = &(*pp)->next;
    if (*pp == t) *pp = t->next;
    t->next = NULL;
    t->queue = NULL;
}

/* ---- switching --------------------------------------------------------- */

/* Give the CPU to the best ready thread (or the host loop).  Called by the
 * running thread after it has put itself where it belongs (a queue, or
 * nowhere if stopped).  Returns when this thread is dispatched again. */
static void dispatch_from_thread(void) {
    OSThread *next = pop(&g_runq);
    plat_stats.thread_switches++;
    if (next == NULL) {
        g_running = NULL;
        host_fiber_switch(g_main_fiber);
    } else {
        HostThread *h = host_of(next);
        g_running = next;
        next->state = OS_STATE_RUNNING;
        host_fiber_switch(h->fiber);
    }
}

static void enqueue_and_yield(OSThread **q) {
    if (q != NULL) enqueue(q, g_running);
    dispatch_from_thread();
}

/* after readying threads: let a higher-priority one take over */
static void maybe_preempt(void) {
    if (g_running != NULL && g_runq != NULL && g_running->priority < g_runq->priority) {
        g_running->state = OS_STATE_RUNNABLE;
        enqueue_and_yield(&g_runq);
    }
}

static void fiber_main(void *arg) {
    HostThread *h = arg;
    h->entry(h->arg);
    /* libultra's __osCleanupThread: a returning thread destroys itself */
    osDestroyThread(NULL);
}

/* ---- threads ----------------------------------------------------------- */

void osCreateThread(OSThread *t, OSId id, void (*entry)(void *), void *arg, void *sp, OSPri pri) {
    HostThread *h = host_of(t);
    int i;
    (void) sp; /* game stacks stay unused; fibers have host stacks */
    if (h == NULL) {
        for (i = 0; i < MAX_THREADS && g_threads[i].t != NULL; i++) {}
        if (i == MAX_THREADS) host_fatal("osCreateThread: too many threads");
        h = &g_threads[i];
    } else if (h->fiber != NULL) {
        /* re-created (the front end's pak thread): drop the old fiber */
        host_fiber_delete(h->fiber);
    }
    h->t = t;
    h->entry = entry;
    h->arg = arg;
    h->alive = 1;
    h->fiber = host_fiber_create(fiber_main, h);
    t->next = NULL;
    t->priority = pri;
    t->queue = NULL;
    t->tlnext = g_active;
    g_active = t;
    t->state = OS_STATE_STOPPED;
    t->flags = 0;
    t->id = id;
    t->fp = 0;
    if (host_verbose) host_log("thread: create %s pri %d entry %p\n", plat_thread_name(t), (int) pri, entry);
}

void osStartThread(OSThread *t) {
    switch (t->state) {
        case OS_STATE_WAITING:
            t->state = OS_STATE_RUNNABLE;
            enqueue(&g_runq, t);
            break;
        case OS_STATE_STOPPED:
            if (t->queue == NULL || t->queue == &g_runq) {
                t->state = OS_STATE_RUNNABLE;
                enqueue(&g_runq, t);
            } else {
                t->state = OS_STATE_WAITING;
                enqueue(t->queue, t);
                {
                    OSThread *w = pop(t->queue);
                    w->state = OS_STATE_RUNNABLE;
                    enqueue(&g_runq, w);
                }
            }
            break;
        default:
            break;
    }
    maybe_preempt();
}

void osDestroyThread(OSThread *t) {
    OSThread **pp;
    HostThread *h;
    if (t == NULL) t = g_running;
    if (t == NULL) return;
    if (t->state != OS_STATE_STOPPED) unlink_thread(t);
    for (pp = &g_active; *pp != NULL; pp = &(*pp)->tlnext) {
        if (*pp == t) {
            *pp = t->tlnext;
            break;
        }
    }
    t->state = OS_STATE_STOPPED;
    h = host_of(t);
    if (host_verbose) host_log("thread: destroy %s\n", plat_thread_name(t));
    if (t == g_running) {
        if (h) h->alive = 0; /* its fiber is deleted when the slot is reused */
        dispatch_from_thread();
        host_fatal("destroyed thread resumed");
    }
    if (h != NULL) {
        host_fiber_delete(h->fiber);
        h->fiber = NULL;
        h->alive = 0;
    }
}

void osSetThreadPri(OSThread *t, OSPri pri) {
    if (t == NULL) t = g_running;
    if (t == NULL) return;
    if (t == g_running && pri == OS_PRIORITY_IDLE) {
        /* the idle thread parks; the host loop is the idle loop */
        t->priority = pri;
        t->state = OS_STATE_RUNNABLE;
        g_idle = t;
        if (host_verbose) host_log("thread: %s is the idle thread (parked)\n", plat_thread_name(t));
        dispatch_from_thread();
        host_fatal("idle thread resumed");
    }
    if (t->priority != pri) {
        t->priority = pri;
        if (t != g_running && t->state != OS_STATE_STOPPED && t->queue != NULL) {
            OSThread **q = t->queue;
            unlink_thread(t);
            enqueue(q, t);
        }
        maybe_preempt();
    }
}

OSPri osGetThreadPri(OSThread *t) {
    if (t == NULL) t = g_running;
    return t ? t->priority : 0;
}

void osYieldThread(void) {
    if (g_running == NULL) return;
    g_running->state = OS_STATE_RUNNABLE;
    enqueue_and_yield(&g_runq);
}

/* ---- messages ---------------------------------------------------------- */

void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msg, s32 count) {
    mq->mtqueue = NULL;
    mq->fullqueue = NULL;
    mq->validCount = 0;
    mq->first = 0;
    mq->msgCount = count;
    mq->msg = msg;
}

static void block_on(OSThread **q) {
    if (g_running == NULL) host_fatal("blocking message call outside a game thread");
    g_running->state = OS_STATE_WAITING;
    enqueue_and_yield(q);
}

static void wake_one(OSThread **q) {
    OSThread *t = pop(q);
    if (t != NULL) {
        t->state = OS_STATE_RUNNABLE;
        enqueue(&g_runq, t);
        maybe_preempt();
    }
}

s32 osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flag) {
    while (mq->validCount >= mq->msgCount) {
        if (flag != OS_MESG_BLOCK) return -1;
        block_on(&mq->fullqueue);
    }
    mq->msg[(mq->first + mq->validCount) % mq->msgCount] = msg;
    mq->validCount++;
    wake_one(&mq->mtqueue);
    return 0;
}

s32 osJamMesg(OSMesgQueue *mq, OSMesg msg, s32 flag) {
    while (mq->validCount >= mq->msgCount) {
        if (flag != OS_MESG_BLOCK) return -1;
        block_on(&mq->fullqueue);
    }
    mq->first = (mq->first + mq->msgCount - 1) % mq->msgCount;
    mq->msg[mq->first] = msg;
    mq->validCount++;
    wake_one(&mq->mtqueue);
    return 0;
}

s32 osRecvMesg(OSMesgQueue *mq, OSMesg *msg, s32 flag) {
    while (mq->validCount == 0) {
        if (flag == OS_MESG_NOBLOCK) return -1;
        block_on(&mq->mtqueue);
    }
    if (msg != NULL) *msg = mq->msg[mq->first];
    mq->first = (mq->first + 1) % mq->msgCount;
    mq->validCount--;
    wake_one(&mq->fullqueue);
    return 0;
}

/* ---- events ------------------------------------------------------------ */

static struct {
    OSMesgQueue *mq;
    OSMesg msg;
} g_events[OS_NUM_EVENTS];

void osSetEventMesg(OSEvent e, OSMesgQueue *mq, OSMesg msg) {
    if (e < OS_NUM_EVENTS) {
        g_events[e].mq = mq;
        g_events[e].msg = msg;
    }
}

void plat_post_event(int e) {
    if (e >= 0 && e < OS_NUM_EVENTS && g_events[e].mq != NULL) {
        if (osSendMesg(g_events[e].mq, g_events[e].msg, OS_MESG_NOBLOCK) < 0 && host_verbose)
            host_log("event %d: queue full, dropped\n", e);
    }
}

/* interrupt masks: nothing is asynchronous here, so they only need to
 * round-trip the value */
static OSIntMask g_intmask = OS_IM_ALL;
OSIntMask osSetIntMask(OSIntMask im) {
    OSIntMask old = g_intmask;
    g_intmask = im;
    return old;
}

/* COP0 Status as a thread sees it: CU1, all interrupts unmasked, IE.  The
 * scheduler (func_802A1320 & 0x1000, the pre-NMI line) treats a clear IM4
 * as a reset in progress. */
u32 __osGetSR(void) {
    return 0x2000FF01;
}
void __osSetSR(u32 v) {
    (void) v;
}

/* ---- virtual-time events ------------------------------------------------ */

#define MAX_PEV 64
typedef struct {
    u64 when;
    u64 seq;
    int kind;
    void *p;
} PEvent;
static PEvent g_pev[MAX_PEV];
static int g_npev;
static u64 g_seq;

void plat_event_add(u64 when, int kind, void *p) {
    if (g_npev == MAX_PEV) host_fatal("event queue full");
    g_pev[g_npev].when = when;
    g_pev[g_npev].seq = g_seq++;
    g_pev[g_npev].kind = kind;
    g_pev[g_npev].p = p;
    g_npev++;
}

void plat_event_remove(int kind, void *p) {
    int i;
    for (i = 0; i < g_npev; i++) {
        if (g_pev[i].kind == kind && g_pev[i].p == p) {
            g_pev[i] = g_pev[--g_npev];
            i--;
        }
    }
}

/* earliest event; ties: retrace first, then in the order they were armed */
static int next_event(void) {
    int i, best = -1;
    for (i = 0; i < g_npev; i++) {
        if (best < 0 || g_pev[i].when < g_pev[best].when ||
            (g_pev[i].when == g_pev[best].when &&
             ((g_pev[i].kind == PEV_VI) > (g_pev[best].kind == PEV_VI) ||
              ((g_pev[i].kind == PEV_VI) == (g_pev[best].kind == PEV_VI) && g_pev[i].seq < g_pev[best].seq))))
            best = i;
    }
    return best;
}

static void deliver(PEvent ev) {
    switch (ev.kind) {
        case PEV_VI:
            plat_vi_count++;
            plat_event_add(ev.when + PLAT_VI_PERIOD, PEV_VI, NULL);
            plat_vi_retrace();
            if (plat_cfg.max_vis && plat_vi_count >= plat_cfg.max_vis) {
                host_log("stopping: %u retraces reached\n", (unsigned) plat_vi_count);
                host_exit(0);
            }
            break;
        case PEV_TIMER:
            plat_timer_fire((OSTimer *) ev.p);
            break;
        case PEV_SP_DONE:
            plat_post_event(OS_EVENT_SP);
            break;
        case PEV_DP_DONE:
            plat_post_event(OS_EVENT_DP);
            break;
    }
}

int plat_advance(void) {
    int i = next_event();
    PEvent ev;
    if (i < 0) return 0;
    ev = g_pev[i];
    g_pev[i] = g_pev[--g_npev];
    if (ev.when > plat_now) plat_now = ev.when;
    deliver(ev);
    return 1;
}

/* A game busy-wait (PORT_SPIN in the game source): the spinning thread keeps
 * the CPU, so time passes until the next interrupt, which may hand the CPU
 * to a higher-priority thread for a while. */
void port_spin(void) {
    if (!plat_advance()) host_fatal("port_spin: nothing will ever happen");
}

/* ---- timers -------------------------------------------------------------- */

int osSetTimer(OSTimer *t, OSTime countdown, OSTime interval, OSMesgQueue *mq, OSMesg msg) {
    plat_event_remove(PEV_TIMER, t);
    t->next = t->prev = NULL;
    t->interval = interval;
    t->value = countdown != 0 ? countdown : interval;
    t->mq = mq;
    t->msg = msg;
    plat_event_add(plat_now + t->value, PEV_TIMER, t);
    return 0;
}

int osStopTimer(OSTimer *t) {
    plat_event_remove(PEV_TIMER, t);
    return 0;
}

void plat_timer_fire(OSTimer *t) {
    if (t->interval != 0) plat_event_add(plat_now + t->interval, PEV_TIMER, t);
    t->value = 0;
    if (t->mq != NULL) osSendMesg(t->mq, t->msg, OS_MESG_NOBLOCK);
}

/* ---- the host loop ------------------------------------------------------- */

void plat_run(void) {
    g_main_fiber = host_fiber_init();
    /* retraces at fixed multiples of the period from power-on (the clock may
     * start late: --boot-count) */
    plat_vi_count = (u32) (plat_now / PLAT_VI_PERIOD);
    plat_event_add((plat_now / PLAT_VI_PERIOD + 1) * PLAT_VI_PERIOD, PEV_VI, NULL);
    for (;;) {
        OSThread *next = pop(&g_runq);
        if (next != NULL) {
            HostThread *h = host_of(next);
            g_running = next;
            next->state = OS_STATE_RUNNING;
            plat_stats.thread_switches++;
            host_fiber_switch(h->fiber);
            /* back here: every game thread is blocked (or parked) */
            continue;
        }
        if (!plat_advance()) host_fatal("deadlock: no thread ready and no event pending");
    }
}
