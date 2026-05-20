# Altium Automation Guide

This folder contains the necessary scripts and documentation to automate the generation of manufacturing files (Gerbers, BOM, PDFs) using GitHub Actions.

## Requirements

1.  **Self-Hosted Runner**: A Windows machine with Altium Designer installed and licensed.
2.  **GitHub Runner Software**: Installed on that machine and labeled with `altium`.
3.  **Output Job File**: Each project must contain an `.OutJob` file named `Default.OutJob`.

## Setup Instructions

### 1. Prepare Altium
Ensure that your project has a `Default.OutJob` file configured with all the outputs you want (Gerbers, NC Drill, BOM, etc.).

### 2. Set up the GitHub Runner
1.  Go to **Settings > Actions > Runners** in this repository.
2.  Follow the instructions to add a **New self-hosted runner** for **Windows**.
3.  During setup, add the label `altium` to the runner.
4.  Run the runner as a service so it's always available.

### 3. Scripts
The GitHub Action uses the script in this folder to trigger Altium. 
- `Automation.PrjScr`: The Altium Script Project.
- `GenerateOutputs.pas`: The DelphiScript that opens the active project and runs the `Default.OutJob`.

## Workflow Overview
When a Pull Request is opened or updated, the GitHub Action will:
1.  Initialize the self-hosted Windows runner.
2.  Execute Altium Designer.
3.  Run the `GenerateOutputs.pas` script.
4.  Upload the resulting `ProjectOutputs/` folder as a ZIP artifact for review.
