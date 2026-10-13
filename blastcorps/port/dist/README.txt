Blast Corps - native Windows port (build @VERSION@)
===================================================

This is Blast Corps (Rare, 1997) running natively on Windows: the game's code was
decompiled back into C and compiled for the PC, with RT64 drawing the graphics and
the game's own sound microcode reproduced on the CPU. It is NOT an emulator, but it
plays the original game exactly: the game logic runs on the N64's 60 Hz clock,
frame for frame as on the console.

No game data is included. You need your own copy of the game:

    a ROM image of Blast Corps (USA) (Rev 1), also called v1.1,
    as a .z64, .v64 or .n64 file
    (SHA-1 of the .z64: 483f7161aea39de8b45c9fbc70a2c3883c4dea8c)

The first USA release (v1.0), the Japanese Blastdozer and the European version are
not supported (yet); the game tells you if you pick one of those.


Running it
----------
Double-click bc.exe. The first time, a file dialog asks for your ROM; the game
checks it and remembers where it is (in bc.ini). That's all.

Needs: Windows 10 or 11, a graphics card with Direct3D 12 or Vulkan support, and
the files of this folder kept together (bc.exe, SDL2.dll, dxcompiler.dll, dxil.dll).
The folder must be writable (settings, saves and the log are kept here).


Controls (default; change them in the settings menu or bc.ini)
------------------------------------------
  N64            Keyboard            Game controller (XInput/Xbox and others)
  Analog stick   Arrow keys          Left stick
  A              X                   A
  B              C                   B or X
  Z              Z or Space          Left or right trigger
  Start          Enter               Start
  L / R          A / S               Shoulder buttons
  C buttons      I J K L             Right stick
  D-pad          T F G H             D-pad

  F1            settings menu (also Back+Start on a game controller)
  Alt+Enter, F11  fullscreen on/off
  M             sound off/on;   - and =  volume down/up
  Esc           quit (in the settings menu: back to the game)

The keyboard stick is digital (full tilt); a game controller's stick is analog.


Settings menu
-------------
F1 (or Back+Start on a game controller) opens the settings menu over the game, which
waits while it is open. It works with the mouse, the keyboard (arrows, Enter) or a
game controller (D-pad, A, B). Changes apply at once and are written to bc.ini when
the menu closes (Esc, F1 or "Back to the game"). The graphics API changes on the
next start.


Settings: bc.ini
----------------
bc.ini is created next to bc.exe on the first start, with every setting explained in
it. The settings menu writes it too; you can also edit it with Notepad while the
game is closed. It holds:
  [game]       rom (the ROM's path), saves (the saves folder), controller_pak (0/1)
  [video]      api (d3d12 or vulkan), scale (window size 320x240 times this),
               fullscreen, vsync; optional enhancements (off by default):
               resolution (internal resolution: window, 1 = the N64's, 2, 3, 4 ...),
               msaa (0, 2, 4, 8), filtering (aa, linear, nearest),
               aspect (original = 4:3, 16:9, 16:10, 21:9, window: widescreen),
               framerate (original = the game's own 20/30 pictures a second,
               display = the display's refresh rate, or a number like 60, 120, 144:
               smoother motion from in-between pictures), interpolation (tagged,
               the default, or auto), fps_counter (off, on, detailed) and
               fps_counter_corner (top-left, top-right, bottom-left, bottom-right),
               texture_pack (an RT64 texture pack folder or .rtz) and
               texture_pack_enabled (0/1)
  [audio]      volume, mute
  [keyboard]   one line per N64 button: SDL key names, several separated by commas
  [controller] the same for a game controller, plus which stick is the analog stick,
               its dead zone and sensitivity, which controller plays (pad = 1, 2 ...
               or any) and the buttons that open the settings menu
Delete bc.ini to get the defaults back. If a line isn't understood, the game says so
when it starts and uses the default for it.

The window can be resized freely; the picture keeps the N64's 4:3 shape unless
widescreen is on (aspect): then the 3D world fills the wider picture, nothing is
stretched, and the HUD, menus and other 2D pictures stay 4:3 in the middle. With
widescreen the game also treats the wider view as visible (which parts of the level
it draws), so it can play out slightly differently from the N64.

A higher frame rate (framerate) only changes what you see: the game still runs at
its own pace, and the renderer adds pictures between the game's own ones (the camera
and moving vehicles and objects glide instead of stepping). It is capped at the
display's refresh rate (on a 60 Hz display, 60). Cuts (a new screen, the mission
intro, a respawn) are shown without blending. It also counts a slightly wider view
as visible, as widescreen does. With interpolation = tagged (the default) each
vehicle, building part, piece of debris, smoke puff and person is matched with
itself from one game frame to the next, new ones appear without smearing, and the
HUD is never blended; auto lets the renderer guess the pairs instead (to compare).

The frame-rate counter (F2, or fps_counter in the settings menu) shows in a corner
how many pictures a second reach the display and how many frames a second the game
itself makes (30 in play); F2 again adds frame times and a graph, a third time hides
it.

Command-line options (for shortcuts or a .bat file) override bc.ini, for example:
  bc.exe "D:\ROMs\Blast Corps (USA) (Rev 1).z64"   use this ROM (not remembered)
  bc.exe --api vulkan          bc.exe --fullscreen      bc.exe --no-vsync
  bc.exe --mute                bc.exe --volume 50       bc.exe --scale 4
  bc.exe --saves D:\BCsaves    bc.exe --config other.ini   bc.exe --no-pak
  bc.exe --set audio.volume=50   (any bc.ini setting as section.key=value, this run only)
  bc.exe --resolution 4        bc.exe --msaa 4          bc.exe --filtering nearest
  bc.exe --aspect 16:9         bc.exe --texture-pack D:\packs\mypack
  bc.exe --framerate 60        bc.exe --framerate display
  bc.exe --interpolation auto  bc.exe --fps-counter on


Saves
-----
The game saves to its EEPROM like the cartridge does: progress, times and medals are
kept in saves\blastcorps.eep next to bc.exe. Change the folder with "saves =" in
bc.ini. "controller_pak = 1" plugs a Controller Pak into the controller
(saves\blastcorps.mpk); as on the N64, the game then keeps its progress on the pak
and leaves the EEPROM alone, so switch it only if you want to play from a pak.

Both files use the same formats as the mupen64plus emulator (a 512-byte .eep; a
128 KB .mpk with four paks), so saves can be copied between the two: rename the
emulator's file to blastcorps.eep / blastcorps.mpk. Back them up like any save.


Frame rate and smoothness
-------------------------
The game runs at its original speed on any monitor: its logic is tied to a virtual
60 Hz video clock, not to your display. It draws a new picture 20-30 times a second
like on the N64 (with framerate = original, the default; see above). Each
1/60 s field is handed to the display on a steady 60 Hz timer, and with vsync on
(the default) shown at the display's next refresh, without tearing. On 60 Hz
displays that is one refresh per N64 field. On 120/144 Hz and other high-refresh
displays the speed is the same; a field then stays up for a whole number of
refreshes (2 or 3 in turn at 144 Hz), a slight unevenness the eye barely sees.
When a level starts, the first second can stutter briefly while the graphics
driver prepares new shaders; the game then catches up and runs smoothly.


If something goes wrong
-----------------------
Errors are shown in a message box. The log of the last run is in bc.log next to
bc.exe; please include it in a report. If the game crashes, it also writes
bc-crash-<date>-<time>.txt (what happened) and a .dmp file of the same name (part
of the program's memory, for debugging; it can include file paths and your Windows
user name) next to bc.log; please include both. Both can be deleted afterwards.
- "The renderer could not start": try the other graphics API (api = vulkan in
  bc.ini, or d3d12), and update the graphics driver.
- A missing SDL2.dll, dxcompiler.dll or dxil.dll: keep all files of this folder
  together.
- No sound: check the volume keys (- and =) and M; the log says which sound device
  was opened.
- The keyboard stops working: another window took the focus (for example the Xbox
  button's Game Bar, or a notification). Game controllers keep working; in
  fullscreen a controller button brings the game back to the front, otherwise click
  the game's window. bc.log records focus changes and controllers coming and going.


Known issues
------------
- Only Blast Corps (USA) (Rev 1) is supported.
- 64-bit (x86_64) program; the game's memory sits at the N64's addresses (by design).
- Some rare differences from the N64 remain under investigation (see the project's
  notes): a few vertex values in two attract demos, and when exactly a sound starts
  (by one audio frame) compared with the console.
- No frame interpolation yet (planned as an option; the default will stay the
  original behaviour).
- The analog stick from the keyboard is all-or-nothing.


Credits and licences
--------------------
Built from the Blast Corps decompilation project. Graphics: RT64 (MIT licence).
Windows, input and sound output: SDL2 (zlib licence). Shaders: Microsoft DirectX
Shader Compiler (dxcompiler.dll, dxil.dll). THIRD_PARTY_LICENSES.txt lists every
included component with its licence text.

Blast Corps is a trademark of its owners. This project is not affiliated with or
endorsed by Rare, Nintendo or Microsoft, and includes none of the game's data.
