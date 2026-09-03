# Tatung Einstein Emulator — Implementation Plan (C++)

## Context

This repository holds MOS 1.2 / 1.21 ROM images and Ric Hohne's commented Z80 disassemblies of the
MOS firmware and two XtalDOS releases. That material is a complete specification for the machine:
port map, interrupt vectors, scratchpad layout, and the exact reset sequence are all documented in
`roms/disassembled/Einstein/__MOS12/`.

The goal is a cycle-stepped Tatung Einstein TC-01 emulator for Windows, in C++, that boots the real
MOS 1.2 ROM and runs XtalDOS from disk images. The disassemblies are not just reference — they are
the test oracle. Every milestone below is defined by an observable behaviour of the real ROM,
traceable to a specific line in the sources.

## Success criteria

Four moments, in order. Each is unambiguous and either happens or doesn't.

1. **Banner.** The screen shows `TATUNG/Xtal MOS 1.2        (C) 1983 198?` and
   `Insert disc in drive 0 and press Ctrl-BREAK to load`.
2. **Clock.** The BCD real-time clock at `FB8C` advances exactly one second per wall-clock second.
3. **Boot.** Ctrl-BREAK loads XtalDOS from a disk image and reaches the CCP prompt.
   That prompt is `0:`, not CP/M's `A>` — XtalDOS prints the drive as a digit and a
   colon (`CCP202.ZSM:275-284`).
4. **Software.** A commercial Einstein disk loads and runs at correct speed with sound.

## Architecture decisions

**Cycle-stepped, not instruction-stepped.** One `Machine::tick()` advances every device by a single
4 MHz T-state. Instruction-stepped cores cannot express VDP access timing or CTC counter cascades
correctly, and retrofitting accuracy later is a rewrite. Decide this now.

**No virtual dispatch in the tick loop.** Devices are concrete members of a `Machine` struct, called
directly. Virtual interfaces are fine for the debugger and for disk-image back-ends, not for the
hot path.

**One owner for bus state.** `Machine` owns RAM, ROM, and every device; devices receive a `Machine&`
(or a narrow bus interface) rather than holding pointers to each other. This avoids the tangle that
kills most hobby emulators around the time the FDC lands.

**Interrupts via a real daisy chain.** Model the Z80 IEI/IEO chain explicitly as an ordered list of
devices, each answering `int_pending()`, `int_ack() -> vector`, and `reti()`. Do not special-case
the CTC. This is the single most common source of subtle breakage on this machine.

## Milestones

Each milestone is a working, demonstrable build. Do not start the next until "done when" holds.

### M0 — Skeleton — **DONE**

CMake project, SDL3 window, Dear ImGui docking layout, ROM loader. Debugger shell: memory hex view,
register panel, run/pause/step buttons.

Built in `emulator/`. Dependencies come from CMake `FetchContent` (SDL3 `release-3.4.14`, Dear ImGui
`v1.92.9b-docking`) rather than vcpkg, which is not installed here — `build.cmd` locates the VS 2022
toolchain via `vswhere` and uses the CMake and Ninja bundled with it. `build.cmd core` builds the
core plus tests with no network access and no dependencies at all.

**Done when:** `Tatung X-TAL MOS v1.2 (1983)(Tatung).rom` loads and its MD5 is verified at startup
against `a10c89a81d5bda7b96f5e948245d45ce`; the hex view shows `21 00 80 11 01 80 18 38` at 0000.

### M1 — Z80 core — **DONE**

Full instruction set including `ED`/`CB`/`DD`/`FD` prefixes, the undocumented `IXH`/`IXL` ops, the
`WZ`/MEMPTR register, and correct flag behaviour for `BIT n,(HL)`, `SCF`/`CCF` (Q register), and
block instructions. T-state accurate per M-cycle.

**Done when:** `zexdoc` passes fully and `zexall` passes. Run these headless — load the `.COM` at
`0100`, stub BDOS at `0005` handling functions 2 and 9, `RET` to `0000` to exit. Do not wait for
the disk system to run them.

Both pass: 67/67 groups OK on each, 5.76e9 instructions and 4.67e10 T-states per run, ~190 s each
(`ctest -L slow`). The harness runs `Machine` with the ROM overlay switched off, giving plain 64K
RAM, and traps BDOS by watching PC — so no bus abstraction was needed and `Machine` stays concrete.

Deviation from the architecture note above, worth knowing: execution is per-instruction, but every
bus access and internal delay calls `Machine::tick()`, so devices still advance at the correct
T-state offsets *within* an instruction. That satisfies what the VDP and CTC need without a full
CPU state machine; the CPU never needs suspending mid-instruction on this machine.

### M2 — Memory map and ROM/RAM banking — **DONE**

64K RAM; ROM overlays `0000-1FFF` after reset. Critical: MOS copies trampolines into `FC09-FC8B`
precisely because ROM cannot page itself out while executing, and after DOS loads, `RST 08` is
redirected there — it pages ROM in, dispatches, and pages back out (`UPPERMEM.ZSM:LFC22-LFC40`).

**Port 24H semantics — settled.** It is a *write strobe*: any write toggles the overlay and the
data bus is ignored. The decisive evidence is the trampoline at `UPPERMEM.ZSM:33`:

```
LFB14:  OUT (24H),A     ; switch
        LD  A,(HL)      ; A is overwritten with an arbitrary data byte
LFB17:  OUT (24H),A     ; switch back, writing that arbitrary byte
```

The second `OUT` sends whatever happened to be at `(HL)` and must still restore the previous state,
so only the strobe matters. The same "switch, work, switch back with unchanged A" shape appears in
the `LDIR`/`LDDR`/`CALL` helpers at `LFB1A`/`LFB21`/`LFB28` and in the `RST 08` redirect at
`LFC3B`/`LFC40`, which brackets `CALL 0FCAH` with two `OUT`s both holding the same MCAL number.

Note that Hohne's own comments are unreliable here: `BIOS202.ZSM:228` annotates one site
`;SELECT ROM` and `CCP202.ZSM:43` annotates the identical instruction `;SELECT RAM`. Both are
reboot paths where `A` is leftover garbage, so both comments are guesses. Trust the trampolines.

**Done when:** the reset sequence at `Mos12.zsm:255-300` executes to completion — RAM filled with
`FF`, `FB00-FFFF` cleared, the three ROM-to-RAM block copies done, RTC set to six `30H` bytes,
`SP=FCFF`, `I=FBH`, `IM 2` set.

All verified in `tests/test_core.cpp`, byte-for-byte against ROM content rather than by eye. The
sequence takes 67,073 instructions and 1,407,985 T-states — 0.35 s at 4 MHz, a plausible power-on
memory test. Three scratchpad variables are also written by the reset tail and are asserted:
`USERI` (`FB60`) = `FB`, `USERSP` (`FB65`) = `FCFF`, `DFLAG` (`FBB0`) = `01`.

**Coverage caveat.** Measured: MOS on its own performs *zero* writes to port 24 in the first three
million instructions — it runs entirely from ROM, and the trampolines that switch the overlay are
only called once DOS is resident. So the toggle semantics are currently proven by unit test against
the derivation above, not by firmware. M5 is the first milestone that exercises banking for real,
and it is the milestone where a wrong polarity would first bite.

Where MOS reaches with no devices: init completes, `IM 2` interrupts are enabled, and it settles
into its idle loop scanning the keyboard — `LD A,0E / OUT (02),A / LD A,E / CPL / OUT (03),A` at
`0F99-0FA1`, i.e. selecting AY-3-8910 register 14 and writing the row mask. Useful for M4: the
**keyboard row select is PSG port A (register 14), not a dedicated port**, and the matrix value is
passed in `E` (matching the table at `Einstein.zsm:352-381`).

### M3 — TMS9129 VDP — **DONE**

VRAM 16K, eight registers, the address/data port protocol on `08`/`09`, the status register with its
read-clears-interrupt-flag side effect. Table bases are in `Einstein.zsm:342-348`.

**Done when:** success criterion 1. The banner is on screen.

Achieved, and then some: MOS boots past the banner to its `Ready` prompt with a blinking cursor.

**Correction to this plan's assumption.** The milestone said "text mode first ... sprites and
Graphics II can wait". That was wrong: MOS never uses the VDP's text mode. It selects **Graphics
II** and renders text as pixels into the bitmap, which is why the name table holds the sequential
G2 index rather than character codes. Graphics II was therefore the *only* mode that mattered for
the banner. All four modes plus sprites are implemented.

Measured VDP configuration after boot — registers `02 C0 0E FF 03 76 03 F4`:

| | value | source |
| --- | --- | --- |
| mode | Graphics II | R0 bit 1 (M3) set |
| name table | `3800` | `VPNTBL`, `Einstein.zsm:345` |
| pattern bank | `0000` | `VPGTBL`, R4 bit 2 clear |
| colour bank | `2000` | `VPCTBL`, R3 bit 7 set |
| sprite patterns | `1800` | `VSPTBL`, `Einstein.zsm:343` |
| colours | white on dark blue | R7 = `F4` |

**Two findings that change later milestones.**

MOS never reads port 09. The dZ80 cross-reference has no `Input Port &0009` section at all, and all
four `OUT (09)` sites are inside the two address-setup routines at `0C8D`/`0C98`. So the banner
needs no vblank polling.

**MOS leaves VDP interrupts disabled** — R1 = `C0`, so bit 5 (IE) is clear. Combined with the
absence of any VDP entry in the vector table (`Einstein.zsm:40-49`), the question of how the VDP's
INT reaches the Z80 is moot for now. Do not invent a wiring for it at M4; the CTC is the interrupt
source that matters.

Note `Tms9129::tick` runs on every bus access, so it compares against a precomputed vblank
threshold rather than dividing per call.

### M4 — CTC, interrupt daisy chain, keyboard — **DONE**

Four CTC channels with timer and counter modes, prescalers 16/256, and channel 2's ZC/TO cascading
into channel 3's input. Z80 mode 2 acknowledge with `I=FBH` indexing the vector table at
`FB00-FB13`. Keyboard matrix per `Einstein.zsm:352-381`, interrupt masked by port `20`.

The arithmetic to hit, derived from `ZZTIME` at `Mos12.zsm:2717`:

| Step | Port | Value | Meaning |
| --- | --- | --- | --- |
| 1 | `2A` (CTC2) | `3F` | timer, prescale 256, TC follows, no interrupt |
| 2 | `2A` | `7D` | TC = 125, so 4 000 000 / 256 / 125 = **125 Hz** |
| 3 | `2B` (CTC3) | `DF` | counter, TC follows, **interrupt enabled** |
| 4 | `2B` | `7D` | TC = 125, so 125 / 125 = **1 Hz** |

**Done when:** success criterion 2 — the RTC at `FB8C` ticks once per real second — and typing at the
MOS prompt echoes correctly, including shifted keys and Ctrl.

Both hold. The unit test gets exactly 125 and 1 zero-counts from one second of ticks, and the
firmware agrees: booting MOS and running it for twelve seconds gives 1616 channel-2 counts,
12 channel-3 counts, 12 interrupts taken, and `FB8C` reading `000012`. Typing is verified end to
end — the key goes into the matrix, MOS reads it over the PSG, and the character appears on the
rendered screen as the ROM's own glyph.

**Bit 3 of the CTC2 control word — resolved.** The plan flagged "timer starts on external trigger;
confirm against hardware if the clock refuses to start". The CLK/TRG inputs of channels 0-2 run at
**2 MHz**, the 4 MHz system clock halved. That comes out of the baud table at `Mos12.zsm:686-697`:
the 9600 entry is counter mode with time constant 13, and MOS puts the 8251 in x16 mode (mode byte
`CE`), so F = 9600 × 16 × 13 = 1.997 MHz. Every other counter-mode entry agrees (1200 → 1201.92,
2400 → 2403.85, 4800 → 4807.69). So the start edge arrives within two T-states of the time constant
being loaded and the clock starts on its own.

**A real bug, found through this milestone.** `Z80::tick()` forwarded its T-states to
`Machine::tick()` but never added them to the CPU's own `ts_`, so `Z80::step()` and `Z80::irq()`
both returned **0**. Two consequences, one silent and one fatal:

- `Machine::step()` guarded its interrupt accounting with `if (t > 0)`, so `interrupts_taken` never
  moved. Interrupts were in fact being delivered — `irq()` had already pushed the PC and vectored —
  which is why the RTC advanced while the counter insisted no interrupt had ever been taken. That
  contradiction is what exposed it.
- The app's frame loop spends its 4 MHz/60 budget against the returned T-states. With a zero return
  the budget never advanced, so **running the GUI would have hung the render loop**. The M0-M3 tests
  never caught it because they measure `Machine::tstates`, which was always correct.

`tick()` now accumulates, and `test_cpu` asserts that what `step()` reports equals what it charged
to the machine. `Machine::step()`'s interrupt guard was also tightened to match `Z80::irq`'s own
precondition exactly, since `int_ack()` latches the channel in service — acknowledging an interrupt
the CPU then refuses would drop it silently.

**The keyboard is in the ROM, not just in the comments.** `L10C5` (`Mos12.zsm:2827`, "KBD CHARS
(NORM,SHIFT,CONT)") holds **24 bytes per matrix row** — eight normal, eight shifted, eight control —
for the seven rows `02`..`80`. Row `01` is absent because ZKSCAN decodes it inline
(`Mos12.zsm:1441-1465`): column 0 BREAK, 2 F0, 3 F7, 4 ALPHA, 5 ENTER, 6 SPACE, 7 ESC.
`keyboard.cpp` carries those 168 bytes and `test_core` checks every one against the ROM image, so a
transcription slip cannot survive. Control is the third plane — Ctrl-A is `01` from row `40`
column 6.

Scanning is pure polling; **no keyboard interrupt is involved**. `IKBD` at `FB08` points at the
default `EI`/`RETI` stub (`UPPERMEM.ZSM:22`). Rows are selected by writing `~mask` to PSG register
14 (port A) and columns read back from register 15 (port B), both active low; `ZPINIT` sets R7 to
`7F`, making port A output and port B input. Modifiers are read from port `20`, active low in
bits 5-7, and land in `KFLAG` bits 4 (graph), 5 (control), 6 (shift).

**Three firmware behaviours that will mislead you if you assume otherwise.**

1. **MOS boots with caps on.** `UPPERMEM.ZSM` presets `KFLAG` to `80H` and ZKSCAN preserves bit 7
   across every scan. With caps on, a lowercase table entry is looked up again in the shifted plane
   (`Mos12.zsm:1412-1428`), so the *unshifted* `a` key produces `A`. A test expecting `a` is wrong,
   not the emulator.
2. **`LKEY` (`FB46`) has to be sampled while the key is down.** `L0763` zeroes it the moment nothing
   is pressed, and the auto-repeat path at `L0708` zeroes it periodically *during* a hold so the
   key can be re-accepted. Sampling after release always reads `00`, which looks exactly like a
   keyboard that does not work at all.
3. **MOS boots the 40-column text screen**, not a 32-column one. `WIDTH` (`FB4F`) is 40, and
   ZVOUT's 40-column path (`Mos12.zsm:1888`) packs four characters into three bytes — **six pixels
   per character**, the top six bits of each font byte (`AND 0FCH`). Characters therefore never
   line up with VRAM pattern cells, so screen contents cannot be read back by comparing cells.
   Verification has to happen in rendered pixels: character *n* on a row occupies x = *n*×6 …
   *n*×6+5. This matters for M8's screenshot regression test.

The font itself is another usable oracle: `L116D` (`Mos12.zsm:2870`, "VDP CHARACTER SET"), eight
bytes per glyph starting at `20H`. The echo test finds the ROM's `A` and `Z` glyphs in the rendered
framebuffer, so it proves the whole path — matrix, PSG, ZKSCAN, table lookup, ZVOUT, VDP, renderer.
Holding a key produces repeats, as the real machine does, so the test asserts "at least one more
glyph", not exactly one.

The GUI now drives the matrix from host keys (Caps Lock is ALPHA, Pause is BREAK, Alt is GRAPH) and
has a Keyboard/CTC panel showing the matrix, port 20, the four channels and the BCD clock.

### M5 — FD1770 FDC and disk images — **DONE**

Type 1-4 commands and the status word per `Einstein.zsm:394-467`. Drive and side select on port `23`
(bits 0-3 drive, bit 4 side). Support a plain sector image format first; add flux-level or
sector-with-timing support later if protected disks matter. Model rotational position and index
pulses — timing-blind FDCs boot plain disks and fail everything with protection.

Both XtalDOS layouts must work: 1.31 loads CCP at `E100`, DOS at `EC00`, BIOS at `FA00`; 2.02 loads
CCP at `E100`, DOS at `EA00`, BIOS at `F800`. Do not hardcode one map.

**Done when:** success criterion 3 — Ctrl-BREAK boots to the CCP prompt and `DIR` lists a directory.

Both XtalDOS releases boot, both entry points reached, and a full session runs:

```
0:DIR
0k Size, 188k Free, 190k Total
0:SAVE 1 TEST.COM
0:DIR
0: TEST    .COM
2k Size, 186k Free, 190k Total
```

Nothing is hardcoded per release: FBOOT reads the start, end and entry addresses out of the boot
sector's seven-byte header (`Mos12.zsm:484-516`) and the test asserts the header it found — `E100`
/ `FB00` / `F800` for 2.02, `FA00` entry for 1.31.

**The prompt is `0:`, not `A>`.** Success criterion 3 above was written from generic CP/M. XtalDOS
prints the drive as a digit followed by a colon — `PRNPMT` at `CCP202.ZSM:275-284` does
`GETDRV / ADD A,30H / CONOUT` then a literal `':'`. Anything looking for `A>` will never find it.

**Geometry, derived from the driver and then confirmed by a real disc.** Nothing here was guessed:

| | value | where it comes from |
| --- | --- | --- |
| sector size | 512 | ZRSECT moves `SSIZE * 256`, `SCTSIZE` preset to 2 (`UPPERMEM.ZSM:70`) |
| sectors/track | 10 (20 double sided) | GNSECT, `Mos12.zsm:4091` |
| tracks | 40 | ZRBLK stops at 40, `Mos12.zsm:962` |
| sector ids | **from 0** | FBOOT reads sector 0 |
| side 1 ids | 10-19 | CALDSC sets the port 23 side bit when `HSTSEC >= 10` and still writes the raw number to the sector register (`Mos12.zsm:4079`) |

A commercial Oh Mummy disc agrees on every line: 40 tracks, one side, ten 512-byte sectors, ids 0-9.

**Three image formats are read.** Plain sector images (204800 / 409600 bytes); the archive's own
`SYSTEM131.CPM` / `SYSTEM202.CPM`, which are exactly the two system tracks (10240 bytes) and get
padded out into a blank bootable disc; and **CPC DSK / Extended CPC DSK**, which is what Einstein
software is actually traded in. That last one matters for more than convenience: it records the real
ID fields, so it also records the **physical sector order**. Oh Mummy is laid out `0 5 1 6 2 7 3 8
4 9` — a 2:1 interleave — and the controller times its reads against that order, so sequential ids
land two slots apart and the CPU gets the gap the interleave was chosen to give it. A model that
assumed id order would have made every sequential read wait almost a full revolution.

**Rotation is modelled**, as the milestone insisted. 300 rpm (800 000 T-states a revolution), a
4 ms index pulse, 250 kbit/s MFM giving one byte every 128 T-states, the datasheet's six-revolution
motor spin-up before a command with `h=0` executes, and a nine-revolution idle timeout. This is not
decoration — `ZHMDSC` will not finish a RESTORE until it has seen an index pulse (`CHKINX`,
`Mos12.zsm:4113`), and the boot sequence takes the 2.81 s it does because the motor has to spin up
first, exactly like the real machine.

**How MOS decides a drive is ready** is worth recording, because the FD1770 has no ready input at
all. The reset tail selects drive 0 and issues RESTORE **with the verify flag set**
(`Mos12.zsm:331`); with no disc there is no ID field to verify, so the chip reports Record Not Found
and `DELTST` fails its `AND 0BDH / CP 0A4H` test. That is the whole of the "is there a disc in the
drive" logic, and it is why a disc present at power-on boots without anyone pressing anything.

**Ctrl-BREAK, precisely.** ZKSCAN waits for BREAK to be *released* before it reads the modifiers
(`Mos12.zsm:1336-1348`), then jumps to FBOOT if control is still down. So the control key has to
outlive the BREAK key — which is what "Ctrl-BREAK" means in practice, but a test that releases both
together will hang in the wait loop and never boot.

**Writes are implemented**, which the plan had deferred to M9. Left out, the first thing DOS does in
normal use would silently corrupt the user's session, and there is no way to demonstrate `DIR`
listing anything without either writing a file or hand-building a CP/M directory. `SAVE 1 TEST.COM`
writes three sectors through the real firmware path and the test then finds the directory entry in
the disc image itself, at track 2 sector 0. Writes land in the in-memory image only — nothing is
written back to the file on disk yet.

**A screen reader, exact.** All 96 glyphs of the ROM font at `L116D` stay distinct under the
six-bit mask the 40-column path applies, so a rendered 6x8 cell can be matched back to its character
with no ambiguity. `read_screen()` turns the framebuffer into 24 lines of text, which is how the
tests assert on the sign-on banner, the prompt and the `DIR` output rather than on pixel counts. M8
should build its regression screenshots on this.

**Oh Mummy runs.** A real commercial title boots from its Extended DSK, reads 46 sectors with no
Record Not Found, pages the ROM out and drives the VDP itself — 39 335 of 49 152 pixels lit and
144 131 VRAM writes. The tests judge it on that rather than on text, since the game uses its own
patterns and not the MOS font. Success criterion 4 is now only waiting on sound.

The app takes `--disk <path>` (and `--disk1`), or a bare path ending `.dsk`/`.cpm`/`.img`, and
starts running so a game boots on its own. A Discs panel shows the controller state and swaps discs
at runtime.

The transfer in progress is tracked by track/side/sector rather than a raw pointer, so changing a
disc mid-read can never leave the controller reading freed memory.

### M6 — AY-3-8910 sound — **DONE**

Three tone channels, noise, envelopes, and the two I/O ports. Register access on `02`/`03`. Resample
to 48 kHz; drive frame pacing from the audio callback's consumption rate, not from a sleep loop, or
you will get clicks forever.

**Done when:** a game's music plays at correct pitch and tempo with no underruns over ten minutes.

Oh Mummy's music driver runs, and the parts of that criterion a headless test can settle are
settled: ten emulated minutes produce **28 800 000 samples for 28 800 000 expected**, worst
one-second window `+0`, nothing dropped (`ctest -L slow -R audio_soak`). Whether it sounds right to
a listener is still worth checking by ear, but pitch and tempo are now measured rather than assumed.

**The clock, derived rather than assumed.** Nothing in the archive states it, so it comes out of
CTRLG, the BEL handler at `Mos12.zsm:2679`, which plays a single tone with everything else muted:
period 142, amplitude 10, mixer `7E`. A tone is `f = clock / (16 * period)`:

| clock | BEL comes out at | nearest note |
| --- | --- | --- |
| 1.7897725 MHz (Spectrum/CPC family) | 787.75 Hz | G5, **0.48% flat** |
| 1 MHz | 440.14 Hz | A4, 0.03% |
| 2 MHz | 880.28 Hz | A5, 0.03% |
| 4 MHz | 1760.56 Hz | A6, 0.03% |

Landing that precisely on a note means the period was computed for one, which **rules out the
1.7897725 MHz crystal this chip normally runs on** and says the clock is a whole number of
megahertz. 4 MHz is past the part's rating, leaving 1 or 2 MHz — an octave apart, which the beep
alone cannot separate. **2 MHz** is the choice, because this machine is already known to distribute
2 MHz: the CTC's CLK/TRG inputs run at it, derived independently from the baud table at
`Mos12.zsm:686`. Feeding the PSG from the same line needs no extra divider. The residual
uncertainty is exactly one octave and lives in one constant, `Ay38910::kClockHz`.

Oh Mummy then confirms the arithmetic from the other end. The notes it plays land on equal
temperament — periods 358 and 956 are F4 and C3 to within **1 cent** — and across a minute's play
13 of the 15 distinct tone periods it uses are semitones. That cannot happen unless the tone formula
and the clock are right up to that octave.

**A real bug, found by the soak test.** Ten minutes came out 25 samples short, and the whole
shortfall landed inside a single second — a startup effect, not drift. The cause: `ZFDRST` pulses
the port `00` soft-reset strobe **32 times in a row** (`Mos12.zsm:4239`), and the handler was
calling a full `psg.reset()` each time, which threw away the resampler's mid-sample accumulator and
any queued audio. A chip reset must clear the chip, not the audio pipeline in front of it. Split
into `reset()` (power-on: chip, resampler phase, queue, statistics) and `soft_reset()` (the strobe:
chip only), and the ten-minute figure became exact.

**One thing that looks like a bug and is not.** With a channel's tone *and* noise both disabled in
R7, the channel does not go silent — it sits at a constant level set by its amplitude register.
That is the mechanism digitised sample playback uses on AY machines: the driver rewrites the volume
register at the sample rate. The test asserts a steady DC level rather than silence.

**Envelopes** are checked against the datasheet's shapes one step at a time, not by ear: `0C` and
`08` as repeating up and down ramps, `0E` and `0A` as triangles with the turning point repeated,
`09`/`0B`/`0D`/`0F` holding at the right end (0, 15, 15, 0 respectively), and all eight `CONT = 0`
shapes collapsing to a single pass followed by silence. Writing R13 retriggers even when the value
is unchanged, which is how music drivers restart a note. Getting the four held levels right needs
the mask applied *after* ALT flips it — `15 ^ mask`, not the level the next pass would start from.

**Pacing is slaved to the audio device**, as the milestone insisted. The PSG emits exactly 48 000
samples per emulated second, so "keep the device's queue topped up" and "run at the right speed" are
the same instruction — and it self-corrects, because a frame that runs short leaves the queue low
and the next frame runs longer to refill it. Target queue depth is 60 ms, hard ceiling 120 ms, with
a per-frame instruction cap so a stall cannot wedge the UI. With no audio device, or with sound
muted, it falls back to the old fixed T-state budget.

Synthesis is in the core with no SDL anywhere near it: the chip writes into a ring buffer and the
frontend drains it. Sound generation can be switched off entirely (`set_audio_enabled(false)`),
which is what the zexdoc/zexall runner does — those runs have no listener and synthesising billions
of samples for nobody costs real minutes.

### M7 — Remaining peripherals — **DONE**

Z80 PIO (printer on port A, interrupts at `FB10`/`FB12`), 8251 UART with the baud table at
`Mos12.zsm:686` (MOS initialises it to mode `CE`, command `27`, 9600 baud), ADC0844 for analogue
joysticks with its interrupt mask on `21`, the fire-button mask on `25`, and the ALPHA LED on `22`.
Optionally the 80-column card — ports `40-47` video RAM, 6845 at `48`/`49`, input at `4C`
(`MOSEQU.GEN:79-84`).

All but the 80-column card, which is left for M9 with the rest of the optional hardware.

**The daisy chain is now real**, which was the milestone this plan flagged as its biggest risk. Two
devices generate vectors — the CTC and the PIO — and the chain is walked in order, stopping at the
first device that is either requesting or still in service, because a device being serviced holds
IEO low and shuts out everything below it. `RETI` is routed to whichever device actually holds the
chain rather than being handed blindly to the CTC.

The order is CTC ahead of PIO. MOS gives the CTC vectors `00-06` and the PIO `10` and `12`
(`Mos12.zsm:291, 301`), and the vector table at `FB00` is laid out in that order
(`Einstein.zsm:40-49`).

The test does not settle for watching the right thing happen by accident: with the CTC's handler
running and the PIO still requesting, it forces interrupts fully back on and checks the CPU
executes the *next instruction of the CTC handler* rather than vectoring to the PIO. That fails if
the block is coming from `IFF1` timing instead of from IEO.

**The keyboard, ADC, fire and user vectors (`08`-`0E`) are deliberately not in the chain.** They
come from separate logic, not from either chip, and nothing in the archive says where that logic
sits in the priority order. Every one of their MOS handlers is the `EI`/`RETI` stub at `LFC4E`, so
there is nothing to be gained by guessing a position. The masks on ports `20`, `21` and `25` are
latched and readable; the ADC's conversion-complete flag is modelled and can be wired up the moment
something turns out to need it.

**The printer proves most of the chain by itself.** ZPOUT (`Mos12.zsm:2661`) writes a byte to PIO
port A and then spins on `IFLAG` bit 0, which nothing clears except the port A interrupt handler at
`LFC84`. So printing three characters through the real MCAL exercises the PIO's mode and vector
setup, the acknowledge strobe, the interrupt, the daisy chain and the handler — and if any of it is
wrong, ZPOUT simply never returns. The test runs `MCAL ZPOUT` three times and gets `Hi!` out of the
printer with three strobes and `IFLAG` clear.

**The serial bit rate is not configured in the 8251 at all.** It comes from CTC channels 0 and 1,
which BAUD (`Mos12.zsm:636`) programmes from the table at `Mos12.zsm:686`; the chip only divides its
clock input by the mode word's factor. That table's own arithmetic pins down two clocks at once:

| rates | CTC mode | table's formula |
| --- | --- | --- |
| 75-600 | timer | `BAUD = 1/(COUNT * 256 * 250ns)` |
| 1200-9600 | counter | `BAUD = 1/(2 * COUNT * 16 * 250ns)` |

250 ns is the 4 MHz CPU period. In timer mode the 256 is the CTC's own /16 prescaler times the
8251's x16 factor, so a timer counts the **CPU clock**. In counter mode there is no prescaler, so
the leftover 2 is the divider between the 4 MHz CPU clock and the **2 MHz** the CLK/TRG inputs run
at — the same 2 MHz derived independently in M4 from the 9600 entry, and the same 2 MHz the PSG
clock argument leans on in M6. Three separate routes to the same number.

Measured, with MOS's own reset-time setup left untouched: the transmit clock arrives at
**153 846 Hz**, which divided by the mode word's 16 is **9615.4 baud** against the table's stated
9615.38. A character then goes out through `MCAL ZSLOUT` and comes back through `MCAL ZSRLIN` with
TxD looped to RxD.

**The ADC0844** follows the datasheet rather than firmware, and is documented as such: MOS only ever
sets its interrupt mask (`Mos12.zsm:2388`) and never reads it, so games are the only consumer. A
write selects the multiplexer channel and starts a conversion, which takes about 40 us before the
result can be read. Axis values are settable from the Peripherals panel.

**Also worth knowing:** the Einstein wires A0 to the PIO's control/data select and A1 to the port
select, which is the opposite of the usual Z80 PIO layout. It is settled by how MOS addresses the
chip — `PIA+1` is port A control and `PIA+3` is port B control (`Mos12.zsm:301-308`) — so the map is
`30` port A data, `31` port A control, `32` port B data, `33` port B control.

### M8 — Debugger and quality of life — **DONE**

Disassembly view that loads Hohne's labels and comments so you single-step against annotated source
rather than raw mnemonics; breakpoints on execute/read/write/port; VDP register and VRAM inspector;
save states; a "boot to banner" regression screenshot test.

This is where the archive earns its keep — wire `Mos12.zsm` symbol names into the disassembler.

All five, and the archive did earn its keep.

**Mapping a commented source onto addresses without writing an assembler.** A `.zsm` file says what
the instructions are but not where they live. Two things make a cheap approach exact:

- Most of Hohne's labels are `Lxxxx`, where `xxxx` **is** the address — `L0763`, `L0FC2`, `LFB14`.
  Every one is an anchor, and so is every `ORG`.
- Between anchors the source is a faithful one-line-per-instruction disassembly, so walking it in
  lockstep with the *real instruction lengths taken from the ROM* gives every line its address.
  Data lines (`DEFB`/`DEFW`/`DEFM`/`DC`/`DEFS`) take their length from the source instead, since
  those bytes are not instructions.

The walk checks itself. On reaching the next anchor, the address it has arrived at either matches
that label or it does not, and mismatches re-anchor and are counted. **324 of 324 anchors agree —
100% — across 3798 mapped lines**, and 23 of 23 in `UPPERMEM.ZSM`. Every agreement is an independent
confirmation that the walk tracked the real instruction stream over the stretch before it.

**The ROM contains its own answer key**, which turns that into a real test rather than a plausible
one. `RST 08` dispatches through a table of routine addresses at `MOSITP` (`0282`), indexed by the
MCAL number with bit 7 stripped. So for every MCAL the ROM itself says where the routine lives, and
the source walk has to agree — checked at eleven labels spread right across the image
(`ZKEYIN`, `ZPOUT`, `ZSLOUT`, `ZSRLIN`, `ZRSECT`, `ZWSECT`, `ZMOUT`, `ZKSCAN`, `ZZTIME`, `ZPINIT`,
`ZVOUT`). This caught a mistake in the *test*, not the code: three "expected" addresses had been
taken from grep line numbers rather than addresses, and the walk was right where they were wrong.

`UPPERMEM.ZSM` is mapped too. Its code lives in RAM, but the bytes come from the ROM at the source
of each copy (`MEMORY.TXT`), so the walk is pointed there — which means single-stepping the banking
trampolines and the RTC handler shows Hohne's comments as well.

**One correctness trap worth recording:** `MOSEQU.GEN` defines `ZZTIME`, `ZMOUT` and friends as
*MCAL numbers*, and the same names are code labels in `Mos12.zsm`. Code labels and EQU definitions
are therefore kept in separate maps, with labels winning, or `ZZTIME` resolves to `00BC`.

**A real CPU bug, found by the disassembler.** Sweeping every opcode and comparing the disassembler's
length against what the CPU actually consumed turned up `DD 76` and `FD 76` — prefixed HALT. The
CPU parks PC on the HALT opcode so it re-fetches while halted, which is fine, but `Z80::irq` never
stepped past it. The address pushed on accepting an interrupt was therefore the HALT itself, so the
program would **halt again the moment the handler returned** — trapping forever anything that uses
`HALT` to wait for an interrupt, which is the standard way to sync to a frame. zexdoc and zexall
cannot catch this; they never take an interrupt. Fixed in `irq()` and `nmi()`, with a test that
halts, interrupts, and checks the instruction *after* the HALT actually runs.

**Breakpoints** on execute, read, write and port in/out. Execute stops *before* the instruction, so
the machine is shown as it was when it arrived, and continuing steps past it once rather than
firing again immediately. All of it sits behind a single `bp_any_` test so the cost when no
breakpoints are set is one bool. The debugger's own views read through `peek()`, which does not
trip anything — otherwise the hex dump would fire every read breakpoint on screen.

**Save states** hold RAM, the CPU and every device, and the test does not settle for comparing
bytes: it snapshots, runs on 500 000 instructions, restores, runs the same distance again, and
requires the rendered screen and the whole register set to agree. It then does it once more into a
*different* `Machine` object, which is the case that catches a pointer saved into a snapshot — and
there is one, the PSG's pointer to the keyboard, which the restore has to re-seat. Devices with no
owning members are written whole under a `static_assert` that they stay trivially copyable, so
adding a `std::string` to one fails the build rather than corrupting states quietly. Discs, queued
audio and serial bytes mid-character are deliberately excluded, and `state.h` says so.

**The screenshot gate** boots to the banner and hashes the framebuffer, alongside the text
assertions from the `read_screen()` reader built at M5. Both are asserted so a failure says which
way it moved rather than just that it did.

### M9 — Accuracy pass — **DONE**

MOS 1.21 support (`roms/einstein1.21.bin` is a different image; nothing here disassembles it).
Contended timing if it proves to matter, VDP sprite collision and the fifth-sprite flag, FDC write
support, and per-title compatibility fixes.

FDC writes had already been pulled forward into M5, for the reason given there. The rest are done,
and the 80-column card that M7 left behind is done with them. Every peripheral in `MOSEQU.GEN` is
now modelled.

**MOS 1.21 needed no code.** The second ROM in the archive boots, reaches `IM 2`, drives the VDP,
puts its own banner up (`TATUNG/Xtal MOS 1.21`), keeps its clock ticking, and loads XtalDOS 2.02
off a disc in 19 sectors with no Record Not Found — against an emulator written entirely against
the 1.2 disassembly. That is worth stating as a result rather than a shrug: it is the closest thing
to an independent check that the hardware model is a model of the *hardware* and not a set of
behaviours fitted to one firmware image. The font moved (1.2 keeps its glyph table at `116D`,
1.21 at `117A`, byte for byte the same table), so the screen reader anchors on the table it finds
rather than a constant, and `test_core` boots both images through the same assertions.

**Sprite coincidence and the fifth-sprite flag.** Both are per-line, both sticky until the status
register is read, and reading the status clears F, 5S and C together. Coincidence counts a sprite's
pattern bits wherever two overlap, whatever colour they are — a transparent sprite still collides,
which is the subtle one, and the test asserts it directly. The fifth-sprite flag carries the
offending sprite's *number* in the status low bits, and only the first four on a line are drawn:
five sprites on one line give status `C4`, naming sprite 4, with four of them rendered. That
dropped fifth is the flicker every TMS9918 game has.

**Contended timing: measured, and deliberately not enforced.** There is no shared-memory contention
of the Spectrum kind on this machine — the VDP has private VRAM behind a port — but the TMS9918
family does need roughly 8 us between VRAM accesses while the display is fetching. MOS knows it:
the character-set upload at `Mos12.zsm:2370` pads each `OUTI` with `PUSH AF` / `POP AF` for no
other reason. Measured, its tightest spacing is **31 T-states (7.75 us) against the datasheet's 32**
— one T-state inside, from firmware that would be spacing writes 16 T-states apart if it were
running flat out. So the constraint is real and the firmware sits right on it, which settles what to
do about it: enforcing it by dropping accesses would break the machine's own ROM. The VDP counts
accesses that come in under the limit and exposes the tightest gap instead, so if a title ever
misbehaves, `fast_accesses()` says whether this is why.

**The 80-column card**, deferred from M7 as the last unmodelled peripheral. 2K of character RAM, a
6845 CRTC on ports `48`/`49`, and a status input on `4C`. The addressing is the part worth writing
down: the output routine at `L0C13` writes the character with `OUT (C),A` where C holds
`40 + (addr >> 8 & 7)` and B holds the low byte — and `OUT (C),A` puts B on A8-A15, so the RAM
offset arrives split across the port, page in the low half and offset in the high. The port decode
therefore has to see all sixteen address lines, which is why `Machine::io_read` passes the full port
to this device and the truncated low byte to every other.

Two details will hang the machine if they are wrong. Port `4C` bit 0 is an access window that MOS
waits to go high and then low again before *every* character (`L0BEF`) — the anti-snow handshake a
real CRTC card needs — so the card has to be clocked or the ROM spins there forever; and bit 1 must
read high or `Z80INIT` reports a card failure. Bit 2 clear selects the 50 Hz parameter table, which
is the PAL machine this is. Detection (`L0E31`) reads port `40`, writes the complement back and
reads again: plain RAM satisfies it and an open bus reading `FF` does not, so **a machine with no
card fitted correctly reports none** — the default, since the card was an add-on. The test asserts
both halves: with no card MOS clears the 80-column bit in `PCFLAGS` and never touches the 6845;
with one fitted it programmes all sixteen registers, sets `PCFLAGS` bit 2, and its banner is
readable straight out of the card's RAM.

What is *not* modelled is the picture. The card carries its own character generator ROM, that ROM
is not in this archive, and inventing a font would be inventing hardware. Everything written to the
card is kept as character codes and read back as text — in the tests, and in a debugger panel. The
card is off by default and enabled with `--col80` or the checkbox in the machine panel.

### M10 — Native-feeling UI — **DONE**

A Windows application rather than a debugger shell: a menu bar across the top, modelled on Fuse's
File / Options / Machine / Media / Help, a status bar, native file dialogs, and the emulated screen
as the window rather than as one panel among twelve.

The menu is the small part. The shape change behind it is that today the screen is a docked panel
and all twelve panels are drawn unconditionally every frame, which makes the app a debugger that
contains an emulator. Fuse is the other way round — the screen *is* the window and the debugger is
summoned — so adopting its menu means adopting its shape: an **emulator mode** that is the default,
and a **debugger mode** that is today's dockspace, reached from Machine, which is where Fuse puts
it. Nothing built for M8 is thrown away; it stops being what you see first.

**There is no free modifier, and this is a hardware fact, not a preference.** Ctrl is the Einstein
CONTROL key, Alt is GRAPH, Caps Lock is ALPHA and Pause is BREAK; F1-F8 are the machine's own
function keys (`kKeyMap`, column 7 of every row); and the arrows, Enter, Space, Escape, Delete and
Backspace are all in the matrix. The whole accelerator budget is **F9-F12**. In particular **Alt
cannot open the menu**, which is the one visible break with Windows convention and is forced: any
title using graphics characters would lose GRAPH to the menu bar.

**Settled at phase 2, and it went further than the plan expected.** The hazard was that
`ImGuiConfigFlags_NavEnableKeyboard` was on, so ImGui's keyboard navigation ate the arrows, Enter,
Space and Escape — every one of them a matrix key — and the debugger shape merely hid it. The
resolution is that navigation is switched **off entirely**. There is no arrangement that shares
those keys, so the mouse drives the interface and the keyboard belongs to the machine, with ImGui
claiming keys only for a text field or an open menu. That also disposes of F10, which had looked
like the free consolation for losing Alt: opening a menu from the keyboard needs the nav layer, so
F10 could only ever open a menu you then had to finish with the mouse, which is worse than no F10.
It stays unbound, and the accelerators are F9, F11 and F12. On this machine a mouse-driven menu is
the honest answer rather than a shortfall.

Four phases, in order, each shippable:

1. **The shell.** The five menus wired to the actions that already exist, per-panel visibility so
   the debugger windows can be toggled, a status bar, a window title that names the media, and the
   F9 / F11 / F12 accelerators. No behaviour changes; everything is re-plumbing.
2. **Emulator mode.** Screen as the client area, aspect-correct scaling, full screen, the mode
   switch, the focus rule above, and a new default layout (the ini goes to `-v3`, so saved v2
   layouts do not fight it).
3. **Native dialogs.** `SDL_ShowOpenFileDialog` / `SDL_ShowSaveFileDialog` — present in the SDL
   3.4.14 this already builds, so no new dependency — replacing the type-a-path text field that
   discs are currently inserted through, plus drag-and-drop of a `.dsk` onto the window, a recent
   files list, and a config file for zoom, sound, speed and the card.
4. **The split.** `main.cpp` is 1127 lines and its own header comment says the panels "split out at
   M8", which they did not. This is the moment they do.

Two things deliberately **not** done. There is no "hard reset": `Machine` has one `reset()` and no
soft/hard distinction in the hardware model, so Fuse's pair of menu items would be one action twice.
And the menu stays ImGui's rather than a Win32 `HMENU` on the SDL window's HWND — native would give
real Alt behaviour and system accelerators, at the cost of platform-specific code, a `WM_COMMAND`
message hook, and a menu bar that clashes with the dark panels behind it. Fuse's own Windows build
does not use native menus either.

All four landed. What the plan did not anticipate:

**The machine runs.** It used to be left paused after a reset and on a cold start, which reads as a
broken emulator rather than a stopped one. Running is the default now and pausing is something you
ask for; a breakpoint still stops it, which is the one case where being handed a stopped machine is
the point.

**Joysticks, and a hardware fact the archive does not contain.** The sticks are analogue, so a
direction is an ADC0844 axis driven to one end and released back to 128 — not a switch. Bindings
are configurable per stick, and joystick 1 defaults to the numeric keypad, the one stretch of host
keyboard the matrix does not claim.

The orientations are **measured, not assumed**, and the method is worth recording because the
archive cannot answer any of it: boot Oh Mummy headlessly, drive its own Joystick option, snapshot
the machine, change one input, run the same distance, and compare the rendered screen against the
identical state left alone. Save states are what make the comparison fair. That gives channel 0 as
X (`0` left, `255` right) and channel 1 as Y — **`0` down, `255` up**, the opposite way round to
a screen coordinate, and the obvious guess would have inverted every game's controls.

**There are two joystick interfaces**, which the plan did not know either. Oh Mummy reads the
analogue sticks on the ADC0844; COBRA reads a **digital** stick on Z80 PIO port B (port `32`) and
reads nothing else whatsoever — not the ADC, not the keyboard, not port `20`. Its bits, found the
same way, are `0` down, `1` left, `2` up, `3` right and `4` fire, all active low; fire is the one
that made a bullet appear, eight white pixels that no other bit produced. Both interfaces are
driven from the same bindings, since they are separate hardware and a title reads whichever it was
written for. This is also a reminder that "the joystick works" is a claim about one title until it
has been tried on a second.

**Fire is port `20` bit 0, active low**, on the same port as SHIFT, CONTROL and GRAPH. This was not
known when the milestone was planned; the plan had fire bound to a matrix key on the grounds that
the archive documents no readable fire button, which is true — port `25` is an interrupt mask and
`IFIRE` at `FB0C` has an `EI`/`RETI` stub for a handler. But the question is answerable by
experiment even so: waiting at `PRESS 'C'/'FIRE' TO CONTINUE`, Oh Mummy reads **ports `02` and `20`
and nothing else**, which rules out both the ADC and the matrix; scanning the five bits of port 20
the modifiers do not use finds bit 0 gets past the screen. `Keyboard::set_port20_low()` now carries
it, and fire alone starts the game with no key pressed. Joystick 2's fire is assumed to be bit 1 by
symmetry, and that one is still a guess — flagged as such in the source.

**The split, at last.** `main.cpp` went from 1738 lines to 248 — SDL and ImGui setup, the command
line, and the frame loop. The rest is `app.h` (the shared state and the seams), `panels.cpp`,
`menu.cpp`, `files.cpp`, `input.cpp`, `screen.cpp` and `audio.cpp`. The anonymous namespace became
`albert`. This is the tidying the M8 write-up promised and did not do.

**Making it something to hand over.** Three things stood between a working build and a release, and
all three were invisible until looked for. The ROM directory was an **absolute path compiled into
the binary**, so a copy given to anyone else would have found no ROM, no ROM menu and no annotated
source; `roms_dir()` now prefers a `roms` folder beside the executable and falls back to the
compiled-in path, which is what keeps the build tree working. The **C runtime is linked statically**,
which removes `MSVCP140`, `VCRUNTIME140` and `VCRUNTIME140_1` from both `albert.exe` and `SDL3.dll`
-- checked with `dumpbin /dependents`, leaving only Windows' own DLLs, so there is no redistributable
to install. And the default ROM was found by an exact TOSEC filename; it is now `einstein.rom`,
falling back to **hashing every `.rom` in the folder** and taking the first that is genuinely an
Einstein image, because identity has always come from the MD5 and never the name. `cmake --build
build --target dist` assembles the lot into a 2.5 MB zip.

**Two interface bugs, both found by photographing the window rather than reasoning about it.**
Emulator mode was **destroying the saved dock layout**: it submits no panels, so ImGui wrote an ini
with no docking in it, and the next visit to the debugger came up with every panel floating on top
of the next. The layout is now written only from debugger mode. And the picture in the Screen panel
was **blurred by the ImGui backend**, which overwrites the scale mode of every texture it draws and
defaults to linear -- point sampling has to be asked for explicitly around the image. Neither was
diagnosable from the source alone; both took a screenshot, and the first two screenshots were
themselves misleading until the capture was made DPI-aware.

**Ctrl-BREAK cannot be typed on Windows.** Booting a disc inserted after startup needs Ctrl-BREAK,
and it simply never arrived. The cause is not in this project at all: the Pause key sends a
different code when Ctrl is held -- it becomes Break -- and SDL maps that to `SDL_SCANCODE_UNKNOWN`
(`SDL_windowsevents.c`, whose own comment reads "Ctrl+ScrollLock == Break (no SDL scancode?)"). The
keystroke never reaches the program, so no emulator-side work could have caught it. Machine > Boot
disc now performs the gesture on a timer, and Ctrl+End does it from the keyboard. Worth recording
because the emulation was correct throughout: the same sequence driven directly reads 46 sectors and
boots the disc.

**What has no test.** The core has an acceptance suite; the frontend has none, because `test_core`
links `ein_core` alone and that separation is what makes the headless tests possible in the first
place. Everything in this milestone was checked by building, running and round-tripping the config
file rather than asserted in CI, and the joystick's axis logic in particular is verified by reading
it, not by a test. If the frontend grows much further it wants a harness of its own.

### M11 — Sound filter, icon and About — **DONE**

Three small pieces of finishing, and one build bug that only showed itself because the work was
checked instead of trusted.

**A low-pass on the PSG, because some games whistle.** A bare AY is nothing but square waves, and a
game that parks a tone channel at a short period — or leans on the noise generator — puts energy far
above anything the Einstein's internal speaker could reproduce. `Ay38910::set_lowpass()` is two
cascaded one-pole sections, 12 dB/octave, and the placement is the part worth recording: it runs at
the **250 kHz step rate, ahead of the box filter** that decimates to 48 kHz. After the decimation it
could only attenuate a whistle that had already folded down to the wrong frequency; ahead of it, the
same filter is also the anti-alias stage. Options > Sound filter offers Off and 3, 4.5, 6, 9 and
16 kHz, `albert.cfg` carries `lowpass=` in Hz, and the cutoff survives `reset()` because it is a
preference and not chip state.

**16 kHz is the shipped default, and it is the light setting.** The chip's musical range tops out
around 2-4 kHz of fundamental — a tone period of 32 is 3.9 kHz — so every one of these cutoffs is
above every note the AY can play, and 6 kHz was the first default on that reasoning. It was raised
to 16 kHz on the grounds that the whole audible band should come through untouched by default, which
means the default mostly does the anti-alias half of the job: a whistle that genuinely is at 15 kHz
sits inside the cutoff and survives it. The shorter cutoffs are there for those, at the cost of some
brightness. Both facts are in the header so the choice can be revisited without re-deriving it.

**The filter broke two existing tests, correctly.** "Amplitude 0 is silence" and the steady-DC check
both measure the whole rendered buffer, and a register step change now reaches its new level over a
fraction of a millisecond instead of instantly. They skip a 5 ms lead-in rather than being relaxed —
the transient is the filter working. The new tests are pinned to an explicit 6 kHz rather than to
`kDefaultLowpassHz`, because what is on trial there is the DSP and not which cutoff ships; a
separate check asserts the *default* leaves a 3 kHz tone alone, which is the property that stops a
future default quietly dulling every game to fix a few.

**The icon is generated, not drawn.** `tools/make_icon.py` lays an Einstein caricature out in a unit
square and cuts eight sizes from 16 to 256, each **rendered from the geometry and supersampled
rather than resized down from the 256** — verified as a real difference, not a theoretical one, by
comparing the frames in the `.ico` against a naive downscale. The design is a silhouette test: at 16
pixels all that survives is a dark disc, a white flare of hair and a white bar of moustache, so the
palette exists to keep those three apart. It took three passes to stop looking wrong, and each
failure was a different lesson about small-format drawing: a full crown reads as a **judge's wig**;
spikes long enough to clear the hair mass only sprout where the outline is thin, giving **horns** at
the temples; and a row of them along the crown reads as a **tiara**. The spikes were removed
altogether in the end, and the hair mass put back to its pre-spike size so the silhouette did not
shrink with them.

**One resource entry, three consumers.** `src/app/albert.rc` declares the icon as ID **1**, and that
number is load-bearing twice: Explorer shows the lowest-numbered icon as the file's icon, and SDL3
does the same for the window class when no icon hint is set — it enumerates `RT_GROUP_ICON` and
takes the first, "like in the Explorer" (`SDL_windowsevents.c`, `WIN_RegisterApp`). So Explorer, the
taskbar and the title bar are covered with nothing in the C++ asking for it. The About window then
reads the icon **back out of the running executable's own resources** (`load_app_icon()` in
`screen.cpp`, `LoadImageW` + `GetIconInfo` + `GetDIBits`) rather than embedding the pixels a second
time, so the two copies cannot drift and the binary carries one. That path is Windows-only and
returns null elsewhere, which the About window tolerates by dropping the image.

**The build bug: `rc.exe` emits no depfile.** Regenerating the icon and rebuilding printed
`Build OK`, relinked, and produced an executable with **the previous icon in it** — the `.res` was
fourteen minutes stale and nothing had told Ninja that `albert.rc` reads `albert.ico`. This is the
worst shape a build bug can take: silent, and it looks like success. Fixed by naming the dependency,
`set_source_files_properties(albert.rc PROPERTIES OBJECT_DEPENDS .../albert.ico)`, and verified by
regenerating the icon and watching a plain `ninja` go from "no work to do" to rebuilding the RC
object and relinking. Without it every future icon edit would have needed a manual clean.

**What has no test.** Still nothing on the frontend, for the reason M10 gives. The icon was checked
by extracting it back out of the linked `albert.exe` rather than by looking at the source `.ico`,
and the GDI alpha path was checked by driving the identical call sequence against the built binary
from PowerShell — it returns 3,640 fully transparent pixels, 11,548 opaque and 1,196 partial, which
is a real alpha channel and means the AND-mask fallback in `load_app_icon()` is dead code kept for
the documented case where `GetDIBits` returns none. The About window's layout was confirmed by eye,
by running it.

## Repository layout

```
emulator/
  CMakeLists.txt
  src/
    main.cpp
    machine.h/.cpp         owns RAM/ROM/devices; tick(); daisy chain
    cpu/z80.h/.cpp
    video/tms9129.h/.cpp
    ay38910.h/.cpp         done at M6; tones, noise, envelopes, I/O ports
    z80ctc.h/.cpp          done at M4; counters, timers and the daisy chain
    z80pio.h/.cpp          done at M7; port A drives the printer
    i8251.h/.cpp           done at M7; clocked by CTC channels 0 and 1
    adc0844.h/.cpp         done at M7; analogue joystick axes
    keyboard.h/.cpp        8x8 matrix + the ROM key table at L10C5
    fd1770.h/.cpp          done at M5; rotation, index pulses, read and write
    dskimage.h/.cpp        plain images, SYSTEM*.CPM, CPC and Extended CPC DSK
    card80.h/.cpp          done at M9; 2K character RAM, 6845, ports 40-4C
    disasm.h/.cpp          done at M8; full Z80 including undocumented ops
    symbols.h/.cpp         done at M8; Hohne's sources mapped onto addresses
    state.h                done at M8; save-state serialisation
  src/app/                 the frontend, split at M10 phase 4
    app.h                  shared state and the seams between the files below
    main.cpp               SDL and ImGui setup, the command line, the frame loop
    menu.cpp               menu bar, status bar, About and the window title
    panels.cpp             the twelve debugger panels
    files.cpp              ROMs, discs, snapshots, native dialogs, albert.cfg
    input.cpp              host keyboard to the matrix, and the joysticks
    screen.cpp             the picture, as a panel and as the window, plus
                           load_app_icon() reading the icon out of the exe
    audio.cpp              audio output, which is what paces the emulation
    albert.rc              done at M11; the application icon as resource ID 1
    albert.ico            generated -- see tools/make_icon.py, do not hand-edit
  tools/
    make_icon.py           done at M11; draws albert.ico at eight sizes
  tests/
    zex/                   zexdoc/zexall harness (headless, stub BDOS)
    disks/OHMUMMY.DSK      a real commercial title, Extended CPC DSK
    soak_audio.cpp         ten emulated minutes of game music, checking the
                           sample rate never sags (ctest -L slow -R audio_soak)
    test_core.cpp          acceptance for M0-M4: ROM identification, reset sequence,
                           banking, VDP tables, the 125 Hz / 1 Hz CTC cascade, the
                           keyboard table against the ROM's own, and typing echo
roms/                      existing archive — test fixtures, do not modify
```

## Build and dependencies

MSVC or clang-cl, C++20, CMake, vcpkg. Dependencies: `sdl3` (window/input/audio), `imgui[docking]`
(debugger UI). Keep the emulator core free of SDL and ImGui includes so the headless test harness
links against the core alone — this is what makes M1 and the CTC test possible.

For chip cores, prefer writing your own Z80 (it is the educational core of the project and you need
to understand it to debug everything else) but read existing implementations first. Andre
Weissflog's `chips` library (zlib) is a good cycle-stepped model for the Z80, CTC, and PIO; MAME's
`tms9928a` and `wd_fdc` are the reference for the VDP and FDC. **Check per-file license headers
before copying anything** — MAME is a mix of GPL-2.0+ and BSD-3-Clause, and that choice propagates
to your project.

## Testing strategy

- **CPU:** zexdoc + zexall, headless, in CI. Non-negotiable before M3.
- **Timing:** a unit test asserting the CTC cascade produces exactly 1 Hz from a 4 MHz clock.
- **Boot:** an automated run to the banner with a screenshot hash, as a regression gate.
- **Oracle:** MAME has an existing Tatung Einstein driver. When the screen is blank and you cannot
  tell whether it is the VDP, the CTC, or `RETI`, run the same ROM in MAME, set the same breakpoint,
  and diff the register state. This will save days.

## Risks

**The interrupt daisy chain.** CTC and PIO both sit on it. Wrong priority or wrong `RETI` handling
produces software that hangs or silently drops keys rather than crashing — the worst failure mode to
diagnose. Model IEI/IEO properly from the start; do not fake it with a single pending flag.

*Discharged at M7.* The chain is an ordered walk that stops at the first device requesting or in
service, and `RETI` goes to whichever device actually holds it. The test forces interrupts back on
inside the CTC's handler and checks the PIO is still locked out, so a pass cannot come from `IFF1`
timing instead of IEO.

**Banking re-entrancy.** `RST 08` under DOS pages ROM in and out from code executing in high RAM
(`UPPERMEM.ZSM:LFC22`). A naive "swap a pointer" banking implementation that assumes the fetch and
the execute see the same map will fail here, and only after DOS loads — long after you thought
memory was finished.

**Audio pacing.** Emulator speed must be slaved to audio consumption. Deciding this late means
rewriting the frame loop.

## Appendix — hardware facts verified from this archive

All confirmed against the sources during planning, not assumed:

- **CPU clock 4 MHz** — from the 250 ns baud-table arithmetic, `Mos12.zsm:686`.
- **Joystick fire is port `20` bit 0, active low**, and the ADC's Y axis has `0` at the *bottom*.
  These two are the exception to the heading: they are not in the sources at all, and were measured
  against Oh Mummy at M10 by driving one input at a time out of a save state. See M10.
- **Interrupt mode 2, `I = FBH`**, vector table `FB00-FB13` — `Mos12.zsm:255-300`,
  `Einstein.zsm:40-49`.
- **RTC = CTC2 (125 Hz) cascaded into CTC3 (1 Hz)**, interrupt vector `FB06`, handler at `FC47` —
  `Mos12.zsm:2717`, `UPPERMEM.ZSM:LFC47`.
- **ROM/RAM overlay on `OUT (24H),A`** with RAM-resident trampolines at `FC09-FC8B` —
  `UPPERMEM.ZSM:75-115`.
- **MCAL convention:** `RST 08` + `DEFB n` (`80H`-`D4H`), dispatch at `L0FC2`; `HL/DE/BC/AF`
  destroyed — `MOSEQU.GEN`, `Mos12.zsm:196`.
- **Port map, keyboard matrix, FDC command/status encoding, 8251 word layouts** —
  `Einstein.zsm:7-37, 352-381, 394-467, 471-487`.
- **8251 init:** mode `CE`, command `27`, 9600 baud — `Mos12.zsm:293-300`.
