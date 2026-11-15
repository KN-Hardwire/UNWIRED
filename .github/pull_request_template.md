## Description
Describe the changes made in this PR. Include motivation and context.

## Linked Issues
Closes # (issue number)

## Type of Change
- [ ] Hardware Fix (PCB/Schematic)
- [ ] Hardware Improvement / New Feature
- [ ] Documentation Update

## Checklist

### 🛡️ Hardware Guard Requirements
- [ ] **PDF Updated:** If you changed a `.kicad_sch`, you MUST update the `schematic.pdf` in the same folder.
- [ ] **PDF Updated:** If you changed a `.kicad_pcb`, you MUST update the `pcb.pdf` in the same folder.
- [ ] **No Absolute Paths:** Checked that no absolute paths (e.g. `C:\Users\...` or `/home/...`) are in the `.kicad_pro` file.

### 📸 Visual Review (Library Changes)
*If you modified or added a Footprint (`.pretty`) or Symbol, you must paste a screenshot of the change below:*
> [ PASTE CLIPBOARD IMAGE HERE ]

### General Review
- [ ] **DRC/ERC Passed:** CI jobs `KiCad ERC/DRC` are green (ERC: errors+warnings fail; DRC: errors fail).
- [ ] **3D Models:** All new components have correct 3D footprints.
- [ ] **BOM Updated:** Bill of Materials is updated and parts are available.

## Screenshots / Attachments
Add any relevant screenshots (3D PCB view, scope captures, logic analyzer logs).
