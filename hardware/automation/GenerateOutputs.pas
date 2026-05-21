procedure RunOutJob;
var
    Project : IProject;
    Document : IServerDocument;
    Workspace : IWorkspace;
    OutJobPath : String;
    LogFile : TextFile;
    LogPath : String;
begin
    LogPath := 'C:\Users\Omen\Desktop\altium_automation_log.txt';
    AssignFile(LogFile, LogPath);
    Rewrite(LogFile);
    WriteLn(LogFile, 'Script started at ' + DateTimeToStr(Now));

    Workspace := GetWorkspace;
    
    // Try to find project
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
        WriteLn(LogFile, 'Error: No project found. Exiting.');
        CloseFile(LogFile);
        Exit;
    end;

    WriteLn(LogFile, 'Using project: ' + Project.DM_ProjectFileName);

    OutJobPath := Project.DM_ProjectDirectory + '\Default.OutJob';
    WriteLn(LogFile, 'Looking for OutJob: ' + OutJobPath);

    Document := Client.OpenDocument('OUTPUTJOB', OutJobPath);
    
    if Document <> nil then
    begin
        WriteLn(LogFile, 'OutJob opened successfully.');
        Client.ShowDocument(Document);
        
        WriteLn(LogFile, 'Starting generation...');
        ResetParameters;
        AddStringParameter('ObjectKind', 'OutputBatch');
        AddStringParameter('Action', 'Run');
        RunProcess('WorkSpaceManager:GenerateOutputs');
        
        WriteLn(LogFile, 'Generation finished.');
        Client.CloseDocument(Document);
    end
    else
    begin
        WriteLn(LogFile, 'Error: Could not open OutJob file.');
    end;
    
    WriteLn(LogFile, 'Terminating Altium.');
    CloseFile(LogFile);
    Terminate;
end;
