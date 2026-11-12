# Main_Matrix_Schematics — AI Design Review (2026-11-12)

Reviewer models: GLM-5.3-Flash (netlist forensics), Claude Opus 5.5 (datasheet passes 1–2).
Evidence: ERC `--severity-all`, KiCad netlist export, root connectivity dump, S_Matrix template + Sector sheet full read, datasheets (CH446Q, MCP23S17, DAC8568, TL074, 74LVC2G34) in /tmp/opencode/verify/datasheets/.
User-confirmed design intent: DAC8568 **A/B grade** (0–2.5 V out), **5 V MCU** on J1, desired opamp stage output **−5…+5 V**.

## Verdict
Core topology is sound and matches the datasheets: CH446Q serial mode (P/−S, AX, AY grounded), unique 7-bit addressing per chip, per-chip STB with 10 k pull-downs, MCP23S17 hardware addresses strapped uniquely per sector (000/001/010), RESET pulled up, shared RST RC, VDD=+5 V / VEE=−5 V within limits, both custom symbols (CH446Q 44-pin, DAC8568 TSSOP-16) match their datasheets pin-for-pin.

## Findings (ordered by severity)

### CRITICAL
| ID | Location | Problem | Fix |
|----|----------|---------|-----|
| N2 | whole design | 61 refs collide across the 3 sector instances (48× "U23" CH446Q, U7, U6, C17–C21, R43/44, MP33…). Netlist export fails; MCP23S17 pins appear on +5V *and* GND; a PCB update would leave Sector2/3 without their 74LVC2G34 buffer. Likely cause: instance paths reference old root-sheet UUID (5b137818…) while the root file UUID is b45d5c0b…. | Eeschema → Tools → Annotate Schematic… → **Clear Existing Annotation** → entire schematic. Then re-run `kicad-cli sch export netlist` and verify: unique refs, 3× U6 present, 48 unique CH446Q refs. Also 4 ERC `different_unit_net` errors (Net-(R43-Pad1)…) should clear. |

### MAJOR
| ID | Location | Problem | Fix |
|----|----------|---------|-----|
| N1 | sector U6 (74LVC2G34) | DAT/CLK swapped: CH_CTR.DAT (J1.9) → U6.1A → 1Y → R43 → CH446Q **CS/CK** (pin 34); CH_CTR.CLK (J1.10) → U6.2A → 2Y → R44 → CH446Q **DAT** (pin 36). | Swap the two buffer inputs (or swap J1.9/J1.10 labels + firmware). |
| F1/F3 | /AUX_BUS0..7 | TL074 outputs (±12 V rails, fault swing ±10.5 V) drive X pins of 12 CH446Qs each, no series R. Normal ±5 V is inside ±5.5 V abs max but zero margin; a fault or firmware double-connect pushes ~40 mA through switches rated 15 mA. | Add **470 Ω** series resistor at each TL074 output (isolation + fault current ≈10 mA). Firmware rule: never connect two driven AUX lines to one Y/pad. |
| F2 | TL074 stages | Gain 2.5 / offset −VREF gives −2.5…+3.75 V, not the intended −5…+5 V. | Change feedback R5/R6/R11/R12/R17/R18/R23/R24 **10 k → 20 k**. With R1=10 k (VREF) and R2=20 k (GND) this gives exactly Vout = 4·Vdac − 5 V = −5…+5 V. |
| N3 | U6/U7 inputs | Resolved by user answer: 5 V MCU meets VIH 4.0 V. No level shift needed. | none |

### MINOR
| ID | Location | Problem | Fix |
|----|----------|---------|-----|
| N5 | shared RST | RC (100 k/100 n) too weak: 16 × 10 µA leakage may never reach VIH on slow +5 V ramp; power-off drives RST ≈5 V below GND; MCU cannot force a reset. | 10 k pull-down + diode clamp RST→GND (cathode at GND), or drive RST from a spare MCU/GPIO line. |
| F9 | FB1/2/3 | Beads sit between test pads and rails — not in the supply path; −5 V has no bead. | Move beads between J1 and rails; pad on load side. |
| F10 | J1 | One GND pin for everything; SCK (pin 2) adjacent to +12 V (pin 1) — mis-plug risk; no MISO. | Add GND pins + keying; route MCP23S17 SO out if readback wanted. |
| F11/F12 | /VREF | ~2 mA load → ~1.6 LSB cross-channel error; internal reference is OFF at power-up (must enable in firmware); C7 value unverified. | Buffer VREF if 16-bit accuracy matters; firmware: enable ref; C7 ≥ 100 nF. |
| F13 | AUX nets | Op-amps (noise gain 2.5→4 after F2 fix) drive ~12 CH446Q inputs — ringing risk. | Covered by the 470 Ω series R. |
| F14 | rails | No sequencing: ±12 V up before ±5 V injects current into unpowered CH446Qs. | Series R limits it; optionally Schottky clamps AUX→±5 V rails. |

### INFO (verified OK, no action)
- CH446Q symbol: all 44 pins match datasheet; DAC8568 symbol: all 16 match; ~LDAC to GND (valid), ~CLR to +5 V.
- CH446Q template wiring (P/−S, AX0-3/AY0-2 to GND, NC pins open, 100 nF per supply pin) — correct.
- TL074 on ±12 V within abs max; DAC 0–2.5 V inside TL074 common-mode range on ±12 V.
- X13/X14/X15 + CBUS1–3 are internal inter-sector trunks — deliberate but undocumented (document them).
- MCP23S17: SO/INTA/INTB unused (no readback — acceptable for write-only use). Firmware must set IOCON.HAEN=1 in a broadcast write before addressing sectors.
- R43/R44 22 Ω damping on shared CK/DAT (16 loads each) — good practice.
- Strobe hier-label electrical type "passive" (others "input") — cosmetic.

## Open items / not verified
- STB/CK timing vs CH446Q serial protocol (needs firmware-side review).
- CBUS1–3 root wiring traced by naming only.
- Visual sweep of rendered sheets skipped (netlist + text review covered connectivity; PNGs rendered in /tmp/opencode/verify/sheets/ if wanted).
- Bench bring-up remains the final gate: the 32 test points are the measurement path.
