# Altium Automation: Team Guide

This project uses an automated CI/CD pipeline to generate manufacturing files (Gerbers, BOM, PDFs) whenever a Pull Request is opened or updated.

## How to Use the Automation

To ensure the automation works for your PCB project, you must follow these requirements:

### 1. The Output Job File
The automation script specifically looks for an Output Job file to execute.
*   **Filename:** Every Altium project (`.PrjPcb`) must contain an Output Job file named exactly `Default.OutJob`.
*   **Configuration:** Ensure this file is configured with all required outputs (Fabrication Outputs, Assembly Outputs, and Documentation).
*   **Location:** The file must be in the same directory as your `.PrjPcb` file.

### 2. Branching Workflow
Automation is only triggered on Pull Requests.
*   Work on your assigned hardware branch (e.g., `hw/microcontroller`, `hw/power_supply`).
*   When you are ready for a review, open a **Pull Request** to the `main` branch.

### 3. Reviewing Automated Outputs
Once a Pull Request is opened:
1.  The GitHub Action will initialize and run Altium on our dedicated runner.
2.  After 2-5 minutes, look at the **Actions** tab or the bottom of the PR for **Artifacts**.
3.  Download the `manufacturing-outputs` ZIP file.
4.  **Verify these files** in a Gerber viewer before marking your PR as ready for review.

## Technical Details
The underlying automation uses the files in this folder:
- `Automation.PrjScr`: The Altium Script Project.
- `GenerateOutputs.pas`: The script that automates the `Default.OutJob` execution.

*Note: If the automation fails, ensure your project compiles without errors and that the `Default.OutJob` is correctly named.*
