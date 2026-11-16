# Temporary CI settings — revert these

The Hardware Guard / ERC/DRC workflows contain deliberate softening that must
be undone as the designs mature. Each entry: what, where, when to revert.

## 1. `unconnected_items` = warning
- **File:** `hardware/connection_matrix/Main_Matrix_Schematics.kicad_pro`
  (`board.design_settings.rule_severities.unconnected_items`)
- **Why:** board is rough/unrouted; default severity `error` fails every DRC.
- **Revert when:** the matrix PCB routing is complete. Set back to `"error"`
  — it then acts as the "routing is finished" gate.

## 2. `invalid_outline` = warning
- **Files:** `modules/digital_multieffect/.../digital_multieffect.kicad_pro`,
  `modules/filter/.../filter.kicad_pro`,
  `modules/oscillator/.../oscillator.kicad_pro` (same severity table)
- **Why:** these are placeholder boards with nothing drawn on `Edge.Cuts`.
- **Revert when:** each board gets a real outline drawn; set back to `"error"`.

## 3. ERC filter for kicad#24409
- **Files:** `.github/workflows/kicad-erc-drc.yml` ("Filter known kicad-cli
  hierarchy bug" step) and `scripts/ci_local.sh`
- **Why:** `kicad-cli` cannot resolve repeated hierarchical sheet instances
  (KiCad 10 bug, https://gitlab.com/kicad/code/kicad/-/issues/24409); it
  reports `different_unit_net` across instances of `sector.kicad_sch` that
  the GUI (correctly) does not.
- **Revert when:** a KiCad 10.x patch release fixes instance resolution —
  delete the filter step and re-run; the matrix should still pass with 0
  violations (the schematic labels `CLK_BUF`/`DAT_BUF` are real fixes and stay).

## 4. `schematic-only` label
- **Where:** PR labels; honored in `kicad-erc-drc.yml` (DRC job condition).
- **Why:** allows schematic-sync PRs without DRC.
- **Risk to remember:** a schematic change that alters the netlist (moved
  parts, changed footprints) is not parity-checked against the PCB until the
  PCB changes or the label is removed. The PR template checklist is the
  human backstop.

## 5. `invalid_outline` = warning (microcontroller)
- **File:** `hardware/microcontroller/unwired.kicad_pro` (same severity table)
- **Why:** placeholder board without an Edge.Cuts outline (came in via PR #23).
- **Revert when:** the board gets a real outline; set back to `"error"`.
