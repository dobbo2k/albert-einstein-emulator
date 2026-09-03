# Albert

An emulator for the **Tatung Einstein TC-01** (Z80, 1984), together with the
firmware archive it was written against. Named for the machine's namesake, on
the grounds that a computer named after a person deserves an emulator on
first-name terms with it.

Real commercial titles load, run and play their music. XtalDOS 1.31 and 2.02
boot from disc images to their CCP prompt, where `DIR` and `SAVE` work. The CPU
passes zexdoc and zexall. Every peripheral documented in the Einstein's own
firmware sources is modelled: TMS9129 video in all four modes with sprites,
coincidence and the fifth-sprite flag; AY-3-8910 sound; the FD1770 floppy
controller with real rotational timing, so an interleaved disc reads at the
speed its interleave was chosen to give; the Z80 CTC and PIO; the 8251 serial
port; the ADC0844; and the 80-column card. MOS 1.21 runs too, and needed no code
to do it — the emulator is written entirely against the 1.2 disassembly.

There is also a debugger: disassembly against named, annotated source, memory
and VDP inspectors, breakpoints on address and on I/O port, and save states.

## Run it

Download **[`albert-windows-x64.zip`](https://github.com/dobbo2k/albert-einstein-emulator/raw/main/dist/albert-windows-x64.zip)**
(2.5 MB, Windows x64), unzip it anywhere, run `albert.exe`.

That is the whole installation. No installer, no registry use, and no Visual C++
Redistributable to chase down — the C runtime is linked in, so the folder holds
the executable, `SDL3.dll` and a `roms` folder and needs nothing else.

```
albert.exe --disk mygame.dsk
```

Some discs boot straight into the game; others boot XtalDOS to a `0:` prompt
where you type the program's name. To boot a disc you insert *after* starting,
use **Machine > Boot disc** — MOS looks at the drive only once, at reset, so a
disc put in later has to be asked for. `READ-ME-FIRST.txt` inside the zip has
the keyboard map and the rest.

## What is in here

| | |
|---|---|
| [`emulator/`](emulator/) | Albert itself — emulator core, debugger, tests and build. Full documentation in [`emulator/README.md`](emulator/README.md). |
| [`roms/`](roms/) | Xtal MOS 1.2 and 1.21 ROM images, and Ric Hohne's commented disassemblies of MOS 1.2 and XtalDOS 1.31 / 2.02. |
| [`dist/`](dist/) | The current Windows release, prebuilt. |
| [`CLAUDE.md`](CLAUDE.md) | The hardware reference the emulator is built against: I/O port map, interrupt vectors, scratchpad layout, the MCAL calling convention, the keyboard matrix, and the FD1770 and 8251 bit layouts. |
| [`EMULATOR-PLAN.md`](EMULATOR-PLAN.md) | The milestone plan the project was built to, M0 to M10. |

The archive is the more durable half of this repository. Albert exists because
those disassemblies do — every timing constant and register contract in the
emulator is traceable to a line in them rather than to guesswork.

## Build

Visual Studio 2022 with the C++ desktop workload. CMake and Ninja come bundled
with it; there is nothing else to install and no vcpkg.

```
cd emulator
build.cmd core                      core library and tests only, no network
build.cmd app                       everything, then runs the acceptance suite
cmake --build build --target dist   assembles dist/albert-windows-x64.zip
```

SDL3 and Dear ImGui are fetched from source at configure time, so the first
`app` build takes a few minutes and is incremental after that. `build.cmd app`
finishes by running the acceptance suite, which boots the real ROM and checks
the reset sequence, the VDP tables, the CTC cascade, the keyboard end to end,
the disc formats and the disassembler.

## Licence

Albert's own code is MIT — see [`LICENSE`](LICENSE).

The rest is other people's work, here because an emulator is worth nothing
without the thing it is being faithful to: the ROM images are Tatung's
copyright, the commented disassemblies are Ric Hohne's (1986-87, corrected
2006), the Z80 exercisers are Frank Cringle's, and the disc image the test suite
reads belongs to its publisher. None of it is mine to relicense. The full
accounting is in
[`emulator/README.md`](emulator/README.md#licence-and-third-party-material).
