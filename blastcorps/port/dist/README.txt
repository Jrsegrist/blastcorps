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


Controls (default; change them in bc.ini)
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

  F1            shows the keys in the window title (F1 again hides them)
  Alt+Enter, F11  fullscreen on/off
  M             sound off/on;   - and =  volume down/up
  Esc           quit

The keyboard stick is digital (full tilt); a game controller's stick is analog.


Settings: bc.ini
----------------
bc.ini is created next to bc.exe on the first start, with every setting explained in
it. Edit it with Notepad while the game is closed. It holds:
  [game]       rom (the ROM's path), saves (the saves folder), controller_pak
  [video]      api (d3d12 or vulkan), scale (window size 320x240 times this),
               fullscreen, vsync
  [audio]      volume, mute
  [keyboard]   one line per N64 button: SDL key names, several separated by commas
  [controller] the same for a game controller, plus which stick is the analog stick
               and its dead zone
Delete bc.ini to get the defaults back. If a line isn't understood, the game says so
when it starts and uses the default for it.

The window can be resized freely; the picture keeps the N64's 4:3 shape.

Command-line options (for shortcuts or a .bat file) override bc.ini, for example:
  bc.exe "D:\ROMs\Blast Corps (USA) (Rev 1).z64"   use this ROM (not remembered)
  bc.exe --api vulkan          bc.exe --fullscreen      bc.exe --no-vsync
  bc.exe --mute                bc.exe --volume 50       bc.exe --scale 4
  bc.exe --saves D:\BCsaves    bc.exe --config other.ini


Saves
-----
The game saves to its EEPROM like the cartridge does: progress, times and medals are
kept in saves\blastcorps.eep next to bc.exe. A Controller Pak is plugged in too
(saves\blastcorps.mpk), for the game's Controller Pak options. Change the folder with
"saves =" in bc.ini; "controller_pak = 0" unplugs the pak.

Both files use the same formats as the mupen64plus emulator (a 512-byte .eep; a
128 KB .mpk with four paks), so saves can be copied between the two: rename the
emulator's file to blastcorps.eep / blastcorps.mpk. Back them up like any save.


Frame rate and smoothness
-------------------------
The game runs at its original speed on any monitor: its logic is tied to a virtual
60 Hz video clock, not to your display. It draws a new picture 20-30 times a second
like on the N64; there is no frame interpolation or higher frame rate (yet). With
vsync on (the default) each picture is shown at the display's next refresh, without
tearing. On 60 Hz displays that is one refresh per N64 field; on 120/144 Hz and
other high-refresh displays the speed is the same, and pictures stay up for 2 or 3
refreshes in turn (at 144 Hz), which is barely visible.


If something goes wrong
-----------------------
Errors are shown in a message box. The log of the last run is in bc.log next to
bc.exe; please include it in a report.
- "The renderer could not start": try the other graphics API (api = vulkan in
  bc.ini, or d3d12), and update the graphics driver.
- A missing SDL2.dll, dxcompiler.dll or dxil.dll: keep all files of this folder
  together.
- No sound: check the volume keys (- and =) and M; the log says which sound device
  was opened.


Known issues
------------
- Only Blast Corps (USA) (Rev 1) is supported.
- 32-bit program (by design: the game's memory sits at the N64's addresses).
- Some rare differences from the N64 remain under investigation (see the project's
  notes): a few vertex values in two attract demos, and when exactly a sound starts
  (by one audio frame) compared with the console.
- No frame interpolation, widescreen or texture packs yet (planned as options; the
  default will stay the original behaviour).
- The analog stick from the keyboard is all-or-nothing.


Credits and licences
--------------------
Built from the Blast Corps decompilation project. Graphics: RT64 (MIT licence).
Windows, input and sound output: SDL2 (zlib licence). Shaders: Microsoft DirectX
Shader Compiler (dxcompiler.dll, dxil.dll). THIRD_PARTY_LICENSES.txt lists every
included component with its licence text.

Blast Corps is a trademark of its owners. This project is not affiliated with or
endorsed by Rare, Nintendo or Microsoft, and includes none of the game's data.
