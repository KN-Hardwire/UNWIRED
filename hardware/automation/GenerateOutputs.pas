procedure RunOutJob;
var
    Project : IProject;
    Document : IServerDocument;
    Workspace : IWorkspace;
    OutJobPath : String;
    LogFile : TextFile;
    LogPath : String;
begin
    LogPath := 'C:\altium_automation_log.txt';
    AssignFile(LogFile, LogPath);
    Rewrite(LogFile);
    WriteLn(LogFile, 'Script entry point reached.');

    Workspace := GetWorkspace;
    
    // Wait for project to load
    Sleep(5000);

    Project := Workspace.DM_FocusedProject;
    if Project = nil then
    begin
        WriteLn(LogFile, 'No focused project. Checking project count...');
        if Workspace.DM_ProjectCount > 0 then
        begin
            Project := Workspace.DM_Projects(0);
            WriteLn(LogFile, 'Found project in workspace: ' + Project.DM_ProjectFileName);
        end;
    end;

    if Project = nil then 
    begin
        WriteLn(LogFile, 'Error: No project found.');
        CloseFile(LogFile);
        Exit;
    end;

    OutJobPath := Project.DM_ProjectDirectory + '\Default.OutJob';
    WriteLn(LogFile, 'Opening OutJob: ' + OutJobPath);

    Document := Client.OpenDocument('OUTPUTJOB', OutJobPath);
    
    if Document <> nil then
    begin
        WriteLn(LogFile, 'OutJob opened. Starting generation...');
        Client.ShowDocument(Document);
        
        ResetParameters;
        AddStringParameter('ObjectKind', 'OutputBatch');
        AddStringParameter('Action', 'Run');
        RunProcess('WorkSpaceManager:GenerateOutputs');
        
        WriteLn(LogFile, 'Generation triggered.');
        Client.CloseDocument(Document);
    end
    else
    begin
        WriteLn(LogFile, 'Error: Could not open OutJob.');
    end;
    
    WriteLn(LogFile, 'Done.');
    CloseFile(LogFile);
    Terminate;
end;
