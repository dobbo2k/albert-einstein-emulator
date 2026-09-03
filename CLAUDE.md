# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

An archive of Tatung Einstein (Z80, 1984) ROM images and Z80 disassemblies — firmware reference
material, not a software project. There is no build system, no tests, no lint, no package manifest,
and no source in any modern language. Nothing here compiles; don't go looking for a toolchain.

Treat it as a hardware and firmware specification for emulator, loader, and disk-tool work.

## Start here: the hardware reference

`roms/disassembled/Einstein/__MOS12/Einstein.zsm` is the single most useful file. 491 lines, pure
equates and commented tables, no code. It documents:

- **I/O port map** (`Einstein.zsm:7-37`) — PSG `00-07` (AY-3-8910: `02` address latch / read,
  `03` write), VDP `08-0F` (TMS9129: `08` VRAM, `09` register write / status read),
  PCI `10-17` (8251), FDC `18-1F` (FD1770: `18` cmd+status, `19` track, `1A` sector, `1B` data),
  `20` keyboard int mask, `21` ADC int mask, `22` ALPHA LED toggle, `23` drive select,
  `24` RAM/ROM select, `25` fire-button int mask, CTC `28-2F`, PIO `30-37`, ADC0844 `38-3F`.
- **Interrupt vectors** at `FB00-FB13` — CTC0-3 (CTC3 is the real-time clock), keyboard, ADC,
  fire button, user/external, PIA A (printer), PIA B.
- **Scratchpad layout** from `FB00` — cursor state, key flags, `PCFLAGS` bit meanings, host disk
  parameters `HSTDSC`/`HSTTRK`/`HSTSEC`/`HSTDMA`/`RWFLAG` at `FB50-FB55`, BCD clock at `FB8C`,
  step rate and disk-parameter-block pointer at `FBB0-FBB3`.
- **VDP table bases** — pattern generator `0000`, sprite/text pattern `1800`, pattern colour
  `2000`, pattern name `3800`, sprite attribute `3B00`, function keys `3B80`, text position `3C00`.
- **Full 8x8 keyboard matrix**, shifted and unshifted, plus the port-`23` drive-select/side-select
  bit assignment.
- **FD1770 command encoding and status-word bits**, broken down per command type.
- **8251A** status, command, and mode word bit layouts.

Supporting references:

- `roms/disassembled/Einstein/_EINSTEIN CHIPS.txt` — chip list with datasheet URLs.
- `roms/disassembled/Einstein/__MOS12/MOSEQU.GEN` — MCAL number to function table (`80H`-`D4H`),
  plus the 80-column card ports (`40-47` video RAM, `48`/`49` 6845 address/data, `4C` input).
- `roms/disassembled/Einstein/__MOS12/MEMORY.TXT` — memory map, MOS alone and with DOS loaded.

## The MCAL calling convention

The firmware's system-call mechanism. Needed to read almost any Einstein binary:

```
RST 8
DEFB <mcal-number>      ; 80H..D4H — see MOSEQU.GEN
```

`HL`, `DE`, `BC`, `AF` are generally destroyed. `RST 8` dispatches via `L0FC2`
(`roms/disassembled/Einstein/__MOS12/Mos12.zsm:196`).

The other RSTs are direct entry points, not MCALs:

| RST | Effect |
| --- | --- |
| `10H` | Output the character in `A` |
| `18H` | Print the message inline after the call, terminated by a byte with bit 7 set |
| `20H` | Set VRAM address from `BC` for a write |
| `28H` | Read the VRAM byte at `BC` into `A` |

Register contracts for all of them are commented at `__MOS12/Mos12.zsm:187-245`.

## Binary inventory — which images are the same

Verified by MD5; don't re-derive this.

- `roms/Tatung X-TAL MOS v1.2 (1983)(Tatung).rom`, `roms/disassembled/Einstein/__MOS12/MOS12.OBJ`,
  and the **first 8192 bytes** of `roms/rom.bin` are byte-identical (`a10c89a8…`). This is MOS 1.2;
  banner reads `TATUNG/Xtal MOS 1.2        (C) 1983 198?`.
- `roms/rom.bin` is 8193 bytes — the 8 KB ROM plus one stray trailing `0x21`. That extra byte is
  why `rom.asm` and `romLBLD` both end in a bogus `LD HL,&0000` at offset `2000`, and why
  `romhex.TXT` is 16386 characters. Truncate to 8192 before using it as a ROM image.
- `roms/einstein1.21.bin` is a **different, later** image — MOS 1.21 (`TATUNG/Xtal MOS 1.21`).
  Nothing here disassembles it; every commented source targets MOS 1.2.
- `roms/disassembled/Einstein/__XTALDOS131/SYSTEM131.CPM` and `__XTALDOS202/SYSTEM202.CPM` are
  10 KB system images matching the DOS 1.31 and 2.02 source trees.

## Two independent disassemblies of the same ROM

- **Machine-generated** (dZ80 1.50, Aug 2001), in `roms/` — `rom.asm` (address + hex + mnemonic),
  `romLBLD` (assembler-ready, `Lxxxx` labels, `ORG &0000`), `rom.ref` (cross-reference: input
  ports, output ports, direct and indirect address references), `romhex.TXT` (raw hex). Uses
  `&`-prefixed hex. No comments and no code/data separation — use it for cross-reference lookups.
- **Hand-commented by Ric Hohne** (1986-87, minor corrections Dec 2006), in
  `roms/disassembled/Einstein/` — this is the one to read. Data areas are identified, MCALs
  documented, register contracts stated. `roms/disassembled/ToChris.TxT` is Hohne's note confirming
  the sources reassembled to the original binaries when checked.

## Source tree

```
roms/disassembled/Einstein/
  Einstein.zsm, MOSEQU.GEN     equates / hardware reference (no code)
  __MOS12/  Mos12.zsm          MOS 1.2 ROM, ORG 0000H-1FFFH (4468 lines)
            UPPERMEM.ZSM       RAM images MOS copies out of ROM (ranges per MEMORY.TXT:
                               0000-000A, FB00-FB45, FC09-FC8C)
  __XTALDOS131/                CCP131 @ E100, DOS131 @ EC00, BIOS131 @ FA00
  __XTALDOS202/                CCP202 @ E100, DOS202 @ EA00, BIOS202 @ F800
  EINBIOS.GEN                  earlier BIOS 1.11 disassembly (1985, P.B.)
  ALT_*                        third-party CP/M 2.2 replacements, NOT Einstein disassemblies:
                               BDOSZ 1.4, CCPZ 4.1/4.2, ZCPR 1.0, EINCPR. Different authors and
                               provenance from everything else here.
```

The ORG addresses differ between DOS 1.31 and 2.02 — a loader must not assume one layout.

## Assembler dialect

Period Z80 assembler, not modern syntax. Directives in use: `DEFB`/`DEFW`/`DEFS`/`DEFM` in the
Hohne files versus `DB`/`DW`/`DS` in the MACRO-80-targeted `ALT_*` files; plus `EQU`, `ORG`,
`IF`/`ENDIF`, `MACRO`/`ENDM`, `TITLE`/`SUBTTL`, and the non-standard `*LIST OFF`/`*LIST ON` and
`*INCLUDE A:MOSEQU.GEN` (`EINBIOS.GEN:24`). Hex is `0FB00H`-style in the Hohne sources and
`&FB00`-style in the dZ80 output.

The Hohne `__MOS12` and `__XTALDOS*` files have no `END` directive; the `ALT_*` files do.
`Einstein.zsm` is a standalone equates superset (186 equates), while `Mos12.zsm` carries its own
inline copy (142) rather than including it — the two can drift, so check both when an address
looks wrong.

No assembler ships with this repo.

## Verification commands

- Identify a ROM image: `md5sum` on the `.rom`/`.bin`/`.OBJ`, or
  `head -c 8192 roms/rom.bin | md5sum` to skip the stray trailing byte.
- Extract banner/version strings — Git Bash here has no `strings`, so use PowerShell: read the
  bytes, ASCII-decode, then `[regex]::Matches($s,'[\x20-\x7E]{8,}')`.
