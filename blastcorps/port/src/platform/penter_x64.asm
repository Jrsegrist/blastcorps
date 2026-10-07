; MSVC x64 function-entry hook for the game code (port/CMakeLists.txt builds the
; game files with /Gh /GH).  The counterpart of gcc's -finstrument-functions /
; __cyg_profile_func_enter (os_thread.c): --sync's function-entry switch points
; and --calls.
;
; /Gh: every game function calls _penter at the end of its prologue (so its
; stack is set up and its arguments are still in rcx, rdx, r8, r9, xmm0-3).
; /GH: every game function calls _pexit before it returns; the hook does
; nothing, but its presence keeps the compiler from turning a call in tail
; position into a jump, so return addresses name their real callers, as the
; gcc builds' -fno-optimize-sibling-calls.
;
; _penter returns at once unless port_penter_on (os_thread.c) is set; then it
; saves every volatile register, aligns the stack and calls
;   port_penter_hook(key, rsp, rbp, saved)
;     key    the return address of the `call _penter` (inside the function)
;     rsp    the function's rsp at that call (for unwinding to its caller)
;     rbp    its rbp
;     saved  -> the function's r9, r8, rdx, rcx (in that order)
; which may switch fibers (a --sync switch point) before it returns.

EXTERN port_penter_hook:PROC
EXTERN port_penter_on:DWORD

_TEXT SEGMENT

_penter PROC
    cmp     dword ptr [port_penter_on], 0
    jne     penter_slow
    ret
_penter ENDP

penter_slow PROC FRAME
    push    rbp
    .pushreg rbp
    mov     rbp, rsp
    .setframe rbp, 0
    .endprolog
    push    rax                 ; [rbp-8]
    push    rcx                 ; [rbp-16]
    push    rdx                 ; [rbp-24]
    push    r8                  ; [rbp-32]
    push    r9                  ; [rbp-40]
    push    r10                 ; [rbp-48]
    push    r11                 ; [rbp-56]
    and     rsp, -16
    sub     rsp, 80h            ; home area + xmm0-5
    movdqa  xmmword ptr [rsp+20h], xmm0
    movdqa  xmmword ptr [rsp+30h], xmm1
    movdqa  xmmword ptr [rsp+40h], xmm2
    movdqa  xmmword ptr [rsp+50h], xmm3
    movdqa  xmmword ptr [rsp+60h], xmm4
    movdqa  xmmword ptr [rsp+70h], xmm5
    mov     rcx, [rbp+8]        ; key: our return address
    lea     rdx, [rbp+16]       ; the function's rsp at its call
    mov     r8, [rbp]           ; its rbp
    lea     r9, [rbp-40]        ; saved r9, r8, rdx, rcx
    call    port_penter_hook
    movdqa  xmm0, xmmword ptr [rsp+20h]
    movdqa  xmm1, xmmword ptr [rsp+30h]
    movdqa  xmm2, xmmword ptr [rsp+40h]
    movdqa  xmm3, xmmword ptr [rsp+50h]
    movdqa  xmm4, xmmword ptr [rsp+60h]
    movdqa  xmm5, xmmword ptr [rsp+70h]
    lea     rsp, [rbp-56]
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rdx
    pop     rcx
    pop     rax
    pop     rbp
    ret
penter_slow ENDP

_pexit PROC
    ret
_pexit ENDP

_TEXT ENDS
END
