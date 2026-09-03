# Albert

An emulator for the **Tatung Einstein TC-01** (Z80, 1984). Named for the
machine's namesake, on the grounds that a computer named after a person deserves
an emulator on first-name terms with it.

See `../EMULATOR-PLAN.md` for the milestone plan and `../CLAUDE.md` for the
hardware reference this is built against.

Current state: **M10 done — every milestone complete.** The emulation was
finished at M9; M10 turned the debugger shell into an application, gave it a
name, and made it something you can hand to somebody else. A release is
`albert.exe`, `SDL3.dll` and a `roms` folder, with nothing to install.

The emulation, as of M9: a real commercial
title (Oh Mummy, as an Extended CPC DSK) loads, runs and plays its music.
XtalDOS 1.31 and 2.02 boot from disc images to their CCP prompt, `DIR` and
`SAVE` work, MOS 1.2 keeps its real-time clock at `FB8C` ticking once per second
off the CTC2 -> CTC3 cascade, you can type at the prompt, the CPU passes zexdoc
and zexall, ROM/RAM banking works, and the TMS9129 is implemented in all four
modes plus sprites, with coincidence and the fifth-sprite flag. The Z80 PIO
drives a printer, the 8251 serial port runs at the baud rate the CTC actually
delivers, and the ADC0844 reads joystick axes. Every peripheral documented in
`MOSEQU.GEN` is now modelled, the 80-column card included.

**MOS 1.21 runs too**, and needed no code to do it: the second ROM in the archive
boots, banners, keeps its clock and loads XtalDOS 2.02 off a disc, against an
emulator written entirely against the 1.2 disassembly. Its font table moved from
`116D` to `117A` — byte for byte the same table — so the screen reader anchors on
the table rather than the address.

Emulation speed is slaved to the audio device's appetite, not to a timer: the
PSG emits exactly 48000 samples per emulated second, so keeping the device's
queue topped up *is* running at the right speed, and it self-corrects after a
slow frame. Without an audio device it falls back to a fixed frame budget.

The PSG clock is **2 MHz**, derived from the BEL handler rather than assumed --
see the table in `src/core/ay38910.h`. The beep rules out the 1.7897725 MHz this
chip usually runs on; the remaining uncertainty is one octave and lives in one
constant.

The PSG output goes through a **low-pass at 16 kHz** by default, 12 dB/octave.
A bare AY is nothing but square waves, and games that park a tone channel at a
short period -- or lean on the noise generator -- put energy far above anything
the Einstein's internal speaker could reproduce, which comes out as a
high-pitched whistle over the music. The filter runs at the 250 kHz step rate,
*ahead* of the box filter that decimates to 48 kHz, so it doubles as the
anti-alias stage: after the decimation it could only attenuate a whistle that
had already folded down to the wrong frequency.

16 kHz is the default because it is the light setting -- the whole audible band
comes through, including the harmonics that give a square wave its edge, and
what goes is the ultrasonic content that was folding down. A whistle that
really is at 15 kHz sits inside that and survives it. Options > Sound filter
offers Off and 3, 4.5, 6, 9, 12 and 16 kHz for those; the chip's musical range
tops out around 2-4 kHz of fundamental (a tone period of 32 is 3.9 kHz), so
even 6 kHz is above every note it plays and costs only brightness. The Sound
panel has the same control as a slider whose bottom end is off.

Discs: `--disk <path>` inserts into drive 0 (`--disk1` for drive 1), and a bare
path ending `.dsk`, `.cpm` or `.img` is taken as a disc rather than a ROM -- or
use Media > Drive 0 > Insert, or drop the file on the window. Three
formats are read -- plain sector images, the archive's SYSTEM*.CPM system tracks
padded out to a blank bootable disc, and CPC / Extended CPC DSK. Note the CCP
prompt is `0:`, not CP/M's `A>`.

```
build\src\app\albert.exe --disk tests\disks\OHMUMMY.DSK
```

The FD1770 model turns the disc: 300 rpm, a 4 ms index pulse, one byte every
128 T-states and the datasheet's six-revolution motor spin-up. That is why boot
takes ~2.8 seconds, as on real hardware, and it is required rather than
decorative -- MOS will not finish its RESTORE until it has seen an index pulse.
Reads are timed against each track's real physical sector order, so an
interleaved disc reads at the speed its interleave was chosen to give.

MOS drives a **Graphics II** bitmap screen and draws text as pixels — it never
uses the VDP's text mode. It also leaves VDP interrupts disabled (R1 bit 5 clear)
and never reads the status port. The text screen is **40 columns**: ZVOUT packs
four characters into three bytes, six pixels each, so characters do not line up
with VRAM cells and the screen can only be read back in rendered pixels.

The **80-column card** is off by default — it was an add-on, and MOS's detection
(read port `40`, write back the complement, read again) has to report none on a
base machine. `--col80` fits one, as does the checkbox in the machine panel. Its
2K of character RAM is addressed across the whole 16-bit port: `OUT (C),A` puts B
on A8-A15, so the page arrives in the low byte and the offset in the high. Port
`4C` bit 0 is an access window MOS waits on before every character, so the card
is clocked; without that the ROM spins there forever. The picture is not drawn —
the card's character generator ROM is not in this archive — so what it displays
is kept as character codes and read back as text, in the tests and in a panel.

The debugger single-steps against **Hohne's own commented sources**, not raw
mnemonics: `Mos12.zsm` and `UPPERMEM.ZSM` are mapped onto addresses by anchoring
on the `Lxxxx` labels (whose names are their addresses) and walking the source
in lockstep with the real instruction lengths. All 324 anchors agree, and the
ROM's own MCAL dispatch table is used as an independent check on where eleven
named labels landed. There are breakpoints on execute, read, write and port,
a VRAM inspector, and save states.

Interrupts run a real daisy chain: the CTC ahead of the Z80 PIO, with a device
in service holding IEO low so nothing below it can get in until its `RETI`. The
printer proves most of that path by itself -- MOS's ZPOUT writes a byte to PIO
port A and then spins until the port A interrupt handler clears `IFLAG` bit 0,
so a character that prints at all has been through the PIO, the vector, the
strobe, the chain and the handler.

The keyboard is an 8x8 matrix reached through the AY-3-8910 — register 14 selects
rows, register 15 returns columns, both active low — with shift, control and
graph on port `20`. MOS polls it; there is no keyboard interrupt. The key codes
come from the ROM's own table at `L10C5` (24 bytes per row: normal, shifted,
control), and `test_core` checks all 168 of them against the ROM image.

Two things that look like bugs and are not: MOS **boots with caps on**
(`KFLAG` = `80H`), so the unshifted `a` key gives `A`; and `LKEY` at `FB46` is
only valid while a key is held — it is cleared on release and during auto-repeat.

In the GUI, Caps Lock is ALPHA, Pause is BREAK and Alt is GRAPH.

**Booting a disc put in after the machine has started** is Machine > Boot disc,
or Media > Drive 0 > Boot it. A disc already in the drive at power-on boots by
itself, as on the real machine; one inserted afterwards needs Ctrl-BREAK, and
the menu performs that gesture properly -- CONTROL has to outlive BREAK, because
ZKSCAN reads the modifier keys only once BREAK has been released
(`Mos12.zsm:1336-1348`).

**Ctrl+Pause does not work on Windows and cannot be made to.** The Pause key
sends a different code when Ctrl is held -- it becomes Break -- and SDL reports
that as `SDL_SCANCODE_UNKNOWN`; see the `pending_E1_key_sequence` branch of
`SDL_windowsevents.c`, where the comment reads "Ctrl+ScrollLock == Break (no SDL
scancode?)". The keystroke never arrives, so there is nothing for the emulator
to act on. **Ctrl+End** does the same job, since End doubles for BREAK, and the
menu item sidesteps the question entirely.

## The interface

Two modes. **Emulator mode** is what you get: the Einstein screen is the window,
scaled to fit and centred, with a menu bar above it and a status bar below.
**Debugger mode** is the dockspace with all twelve panels, and it is summoned --
F12, or Machine > Debugger, or start with `--debugger`. Everything built for M8
is still there; it stopped being the first thing you see.

The screen is scaled by whole-window fit rather than a zoom factor, in both
modes -- the Screen panel fits its window the same way emulator mode fits the
client area. 256x192 is 4:3, the shape of the television it was drawn for, so
square pixels are already aspect-correct and there is no pixel aspect to fix.
Options > Video > Integer scaling restricts it to whole multiples if you would
rather have exact pixels than a filled window, and unticking "fit" in the Screen
panel pins it to the zoom factor instead.

Two things about that panel are worth knowing, because both were bugs. The
picture is drawn with **point sampling**, which has to be asked for explicitly:
the SDL3 ImGui backend rewrites the scale mode of every texture it draws and its
default is linear, which turns 256x192 pixel art to mush the moment it is
magnified. And the **debugger layout is saved only from debugger mode**.
Emulator mode never submits the panels, so letting ImGui write the file from
there records a layout with no docking in it at all -- which silently destroyed
the arrangement, and left every panel floating on top of the next one the following
time the debugger was opened. Entering the debugger also checks that the
dockspace really has a layout, and rebuilds the default if it does not.

A menu bar on Fuse's five headings: **File** (open, recent, snapshots, exit),
**Options** (sound, sound filter, zoom, integer scaling, fit, full screen, emulation speed,
joysticks, reset layout, save configuration), **Machine** (reset, run/pause, step, which MOS ROM,
the 80-column card, the debugger), **Media** (both drives, printer, serial) and
**Help** (keyboard map, about). The status bar shows running/paused, measured
speed, the ROM, drive activity, the ALPHA LED and the audio queue.

**The machine runs.** It is not left paused after a reset, or on a cold start
with no disc in the drive -- pausing is something you ask for (F9), not the
state you are handed. A breakpoint still stops it, which is the point of one.

**Joysticks** are under Options > Joysticks, two of them, with every direction
and button bound to a host key you choose. Joystick 1 defaults to the numeric
keypad, which is the one stretch of the host keyboard the Einstein's matrix does
not claim, so the default costs the machine no keys. Directions are analogue --
they drive an ADC0844 axis to one end and let it back to 128 at rest, because
that is what the hardware is and what a game reading it expects.

**There are two joystick interfaces and software uses one or the other**, so the
emulator drives both from the same bindings. Oh Mummy reads the **analogue**
sticks on the ADC0844. COBRA reads a **digital** stick on Z80 PIO port B (port
`32`) and touches nothing else at all -- not the ADC, not the keyboard, not port
`20` -- which is why it ignored everything until the PIO stick was modelled.

| PIO port B, active low | |
| --- | --- |
| bit 0 | down |
| bit 1 | left |
| bit 2 | up |
| bit 3 | right |
| bit 4 | fire |

Only joystick 1 has a known place there; five bits do not fit in the three that
are left, so joystick 2 drives the ADC only.

**Every orientation was measured, not assumed**, by playing Oh Mummy headlessly:
boot it, choose its own Joystick option, snapshot the machine, drive one thing,
and compare the screen against the identical state left alone.

| | |
| --- | --- |
| ADC channel 0 | X: `0` walks left, `255` walks right |
| ADC channel 1 | Y: **`0` walks down, `255` walks up** |
| Fire | **port `20` bit 0, active low** |

The Y axis runs the opposite way to a screen coordinate, which is exactly why it
was worth measuring -- the obvious guess is wrong and would have inverted every
game's controls.

Fire took finding. Nothing in the archive documents a port that *reads* a fire
button; there is an interrupt mask at port `25` and the vector `IFIRE` at
`FB0C`, MOS's handler for it is a stub, and the core does not generate that
vector. But waiting at Oh Mummy's `PRESS 'C'/'FIRE' TO CONTINUE`, the game reads
**ports `02` and `20` and nothing else** -- so fire is neither an ADC channel
nor a matrix position. Driving port `20` bit 0 low is what gets past that
screen, and that is what the emulator now does. A fire button can also press a
key on the matrix, for games that read a key instead. Joystick 2's fire is
assumed to be bit 1 by symmetry; that one is a guess.

Files come in through **native Windows dialogs** (SDL's own, so no extra
dependency): File > Open, Media > Drive 0 > Insert, snapshot save and load, and
saving what the printer or the serial port has been sent. You can also **drop a
file on the window**. One rule decides what an arriving path is, whether it came
from the command line, a dialog or a drop: a `.dsk`, `.cpm` or `.img` extension
means a disc, anything else is a ROM. File > Recent remembers the last eight.

Settings live in **`albert.cfg`** next to wherever you run it -- zoom, integer
scaling, sound, the sound filter cutoff (`lowpass=`, in Hz; `0` is off), speed,
which mode, the 80-column card and the recent list.
Written on exit unless you turn that off in Options, and anything named on the
command line overrides what is in the file. It is plain `key=value` text and is
meant to be editable; a UTF-8 BOM from Notepad is tolerated.

**Keyboard shortcuts are limited to F9, F11 and F12, and that is the hardware's
doing rather than a choice.** Ctrl is CONTROL, Alt is GRAPH, Caps Lock is ALPHA,
Pause is BREAK, and F1-F8 are the Einstein's own function keys; the arrows,
Enter, Space, Escape, Delete and Backspace are all in the matrix. F9-F12 is the
entire free budget. In particular **Alt does not open the menu** the way it
would in any other Windows application — it has to stay GRAPH, or software using
graphics characters would lose it.

| Key | |
| --- | --- |
| F9 | Run / pause |
| F11 | Full screen |
| F12 | Show or hide the debugger windows |

**The menu is mouse-driven**, and F10 does not open it. Settling the keyboard
question is what decided this: ImGui's keyboard navigation wants the arrows,
Enter, Space and Escape, every one of which is a key on the Einstein's matrix,
and it opens its menu layer with Alt, which is GRAPH. There is no arrangement
that shares them, so navigation is switched off entirely and the keyboard
belongs to the machine. F10 could only open a menu you would then have to
finish with the mouse, which is worse than no F10. It stays free.

While a menu is open ImGui holds the keyboard, so keystrokes do not leak through
to the machine behind it.

Port `24H` is a write strobe: any write toggles the ROM overlay and the data bus
is ignored. The derivation is in `src/core/machine.cpp` and `../EMULATOR-PLAN.md`.
Note that MOS alone never writes to port 24 — banking is only exercised once DOS
is resident (M5), so for now the semantics rest on the derivation plus unit tests.

## Known issues, and what is not done

Kept here deliberately: everything below is either unfinished or unverified,
and none of it is hidden in a commit message.

- **Some graphics do not render correctly.** Reported against a side-by-side
  comparison with fMSX, which drives the same TMS9918-family VDP, so the
  comparison is a fair one. Not yet diagnosed -- the screenshots were not to
  hand before this session ended. This is the most substantial open item, and
  the VDP is the place to start: `tms9129.cpp`, particularly the Graphics II
  colour-table banking and the sprite path.
- **Machine > Boot disc has not been watched working.** The Ctrl-BREAK sequence
  it performs is proven -- headlessly it reads 46 sectors and boots Oh Mummy --
  but the menu item driving it was never clicked by the author, because
  synthesised input cannot take foreground focus on Windows. If it does not
  work, the gesture timing in `input.cpp` is where to look, not the firmware.
- **Double Dragon's controls are unknown.** It loads and reaches its title
  screen, but a sweep of every input the machine has -- all 64 matrix keys, four
  ADC channels, eight port 20 bits, both PIO ports -- moved nothing. The sweep
  holds each input steady; the untested hypothesis is an input that registers on
  a press-and-release edge, which is how COBRA's fire turned out to behave.
- **ISLAND.dsk cannot be run.** It holds five `.XBS` Xtal BASIC programs and no
  `.COM`, and there is no BASIC in this archive to load them with.
- **Joystick 2 has no known place on the PIO.** Its fire bit is assumed to be
  port 20 bit 1 by symmetry with joystick 1, and its digital directions are not
  mapped at all; five bits do not fit in the three left over. Only the analogue
  axes are real for the second stick.
- **The frontend has no tests.** `test_core` links `ein_core` alone, and that
  separation is what makes the headless suite possible, so nothing in `src/app`
  is covered. Everything there was checked by building, running, and
  round-tripping files.
- **The ROM scan runs once at startup.** A ROM added to the folder while Albert
  is running needs File > Open or a restart.
- **Windows only.** Nothing in the core is platform-specific -- it is SDL and
  ImGui above it -- but no other platform has been built or tried.

## Build

Requires Visual Studio 2022 with the C++ desktop workload. CMake and Ninja come
bundled with it — nothing else to install, and no vcpkg.

```
build.cmd core    core library + M0 tests only (no network, no dependencies)
build.cmd app     everything, fetching SDL3 and Dear ImGui from source
build.cmd         same as "app"
```

The first `app` build clones and compiles SDL3, so it takes a few minutes; after
that it is incremental. Run the result with:

```
build\src\app\albert.exe [path-to-rom] [--disk <path>] [--col80] [--debugger] [--steps N]
```

**Which ROM it loads by default.** `einstein.rom` in the ROM folder, if it is
there. Otherwise the first `.rom` in that folder whose *contents* are a
recognised image -- identity comes from the MD5 (`rom.cpp`), never the filename,
so a dump keeps whatever name it arrived under and still works. Nothing
recognised at all and Albert starts with no machine, naming `einstein.rom` as
what it looked for.

Machine > ROM offers MOS 1.2 and MOS 1.21 whenever an image that really is one
is present, again whatever it is called; entries with no matching image are
greyed out. The scan runs once at startup, so a ROM added while Albert is
running wants File > Open.

## Releasing

```
cmake --build build --target dist
```

Assembles `build/dist/albert-windows-x64.zip` -- about 2.5 MB, and the whole
installation is unzip and run:

```
albert/
  albert.exe
  SDL3.dll
  READ-ME-FIRST.txt
  roms/                        the ROM, and the four sources the debugger reads
```

**There is nothing to install.** The C runtime is linked statically
(`CMAKE_MSVC_RUNTIME_LIBRARY` in the top-level `CMakeLists.txt`), so neither
`albert.exe` nor `SDL3.dll` needs the Visual C++ Redistributable -- checked with
`dumpbin /dependents`, which leaves only `KERNEL32`, `USER32`, `SHELL32` and
`IMM32`. The settings and layout files are written on first run rather than
shipped, so a release does not impose one person's window arrangement on
everyone.

**The ROM archive is found beside the executable.** `roms_dir()` in `files.cpp`
prefers a `roms` folder next to `albert.exe` and falls back to the path compiled
in at build time, which is what lets the build tree keep running against the
checkout. Before this the absolute build path was baked into the binary, and a
copy given to anyone else would have found no ROM, no ROM menu and no annotated
source.

`-DEIN_DIST_ROMS=OFF` omits the ROM images and the disassembly, leaving an empty
`roms/` for the user to fill. Worth thinking about: the MOS firmware is Tatung's
copyright and the commented sources are Ric Hohne's, so shipping them is a
decision rather than a default.

## Layout

`src/core` is the emulator proper and deliberately has **no SDL or ImGui
include anywhere**. That is what lets `build.cmd core` produce a dependency-free,
network-free build, and it is what the M1 zexdoc/zexall harness will link
against. Keep it that way.

`src/app` is the SDL3 + Dear ImGui frontend, split at M10 into `app.h` (shared
state and the seams), `main.cpp` (setup, command line, frame loop), `menu.cpp`,
`panels.cpp`, `files.cpp`, `input.cpp`, `screen.cpp` and `audio.cpp`. It was one
1738-line `main.cpp` until then, which the M8 notes had been promising to split
for two milestones.

`src/app/albert.rc` carries one resource, the application icon, as ID 1. That
number matters: Explorer shows the lowest-numbered icon as the file's icon, and
SDL3 does the same for the window class when no icon hint is set -- it walks
RT_GROUP_ICON and takes the first, "like in the Explorer" -- so one entry covers
Explorer, the taskbar and the title bar with no C++ involved.

`tools/make_icon.py` draws `albert.ico`: the Einstein caricature the emulator is
named after, laid out in a unit square and cut at 16, 20, 24, 32, 48, 64, 128
and 256. Each size is rendered from the geometry and supersampled rather than
resized down from the 256, because 16 pixels cannot afford the softness that
costs. The design is a silhouette test -- at 16 pixels all that survives is a
dark disc, a white flare of hair and a white bar of moustache, so those three
are what the colours are chosen to keep apart. Edit the script, not the `.ico`.

The About window shows the same face, and gets it by reading the icon back out
of the running executable's own resources (`load_app_icon()` in `screen.cpp`)
rather than embedding the pixels a second time -- so the two cannot drift apart,
and the binary carries one copy. That path is Windows-only and returns null
everywhere else; the About window simply drops the image if it does.

## Tests

`ctest` runs the acceptance suite, or run it directly for per-assertion output:

```
build\tests\test_core.exe ..\..\roms
```

It checks the MD5 implementation against the RFC 1321 vectors before trusting it
to identify ROM images, then a CPU smoke test, then verifies MOS 1.2 and MOS 1.21
identification, the 8193-byte `rom.bin` truncation case, the reset state, and that
the bus reads `21 00 80 11 01 80 18 38` at 0000. It then boots the real ROM and
checks the reset sequence, the VDP tables, the CTC cascade and clock, and the
keyboard end to end — down to finding the ROM's own font glyphs in the rendered
framebuffer after typing. It goes on through the disc formats, the daisy chain,
the disassembler, save states and a boot-screen hash, and finishes on the M9
work: sprite coincidence and the fifth-sprite flag, MOS 1.21 booting both from
ROM and from disc, the 80-column card present and absent, and the VDP access
spacing MOS actually uses.

Despite the `m0_acceptance` name it is the whole acceptance suite; the name is
from M0 and stayed.

The slow tests are the Z80 exercisers (about three minutes each) and a
ten-minute audio soak that plays a game's music and checks the sample rate never
sags. They are labelled `slow` and excluded from the default run:

```
ctest --test-dir build -L slow --output-on-failure
```

`tests/zex/zexdoc.com` and `zexall.com` are the standard CP/M exercisers, taken
from the `anotherlin/z80emu` test files.

## Running the ROM without clicking

```
albert.exe --steps 200000
```

Executes N instructions headlessly before opening the window. After 200k
instructions of MOS 1.2 you should see PC deep in ROM, `I=FB`, `IM 2` and
`SP` just under `FCFF` -- the documented reset sequence at `Mos12.zsm:255-300`.

## Licence and third-party material

Albert's own code -- everything under `emulator/` apart from the test data
listed below -- is MIT licensed. See `LICENSE` at the root of the repository.
That covers the emulator, the debugger and the build system, and nothing else
in here.

The rest is other people's work, included because an emulator is worth nothing
without the thing it is being faithful to:

- **`roms/*.rom`, `roms/*.bin`** -- Tatung's Xtal MOS firmware for the Einstein
  TC-01 (1983-84), still Tatung's copyright. The `dist` target ships it by
  default for convenience; configure with `-DEIN_DIST_ROMS=OFF` to build a
  release without it.
- **`roms/disassembled/Einstein/`** -- the commented MOS 1.2 and XtalDOS
  disassemblies by Ric Hohne (1986-87, minor corrections 2006). Albert is
  written against these rather than against guesswork, and the debugger reads
  them at run time so it can single-step through named, annotated source. They
  are the reason this emulator could be written at all.
- **`roms/rom.asm`, `romLBLD`, `rom.ref`, `romhex.TXT`** -- machine-generated
  dZ80 1.50 output (2001), kept for cross-reference.
- **`emulator/tests/zex/zexdoc.com`, `zexall.com`** -- Frank Cringle's Z80
  instruction set exercisers, taken from the `anotherlin/z80emu` test files.
  GPL. Used as test input; nothing links against them.
- **`emulator/tests/disks/OHMUMMY.DSK`** -- a commercial disc image (Oh Mummy),
  copyright its publisher. The acceptance suite reads it to check the disc
  geometry, the FD1770's rotational timing and the PSG against a disc a real
  Einstein actually shipped with, rather than against a synthetic image.
- **SDL3** (zlib licence) and **Dear ImGui** (MIT) are fetched at configure
  time by `cmake/deps.cmake`, not vendored here. A release ships `SDL3.dll`
  beside the executable.

If you redistribute this, the firmware and the disc image are the two items to
think about: neither of them is mine to relicense.
