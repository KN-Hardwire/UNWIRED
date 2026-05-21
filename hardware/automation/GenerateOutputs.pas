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
    WriteLn(LogFile, 'Script entry point reached at ' + DateTimeToStr(Now));

    Workspace := GetWorkspace;
    
    Project := Workspace.DM_FocusedProject;
    if Project = nil then
    begin
        WriteLn(LogFile, 'No focused project. Checking project count...');
        if Workspace.DM_ProjectCount > 0 then
        begin
            Project := Workspace.DM_Projects(0);
            WriteLn(LogFile, 'Using first project in workspace: ' + Project.DM_ProjectFileName);
        end;
    end;

    if Project = nil then 
    begin
        WriteLn(LogFile, 'Error: No project found in workspace. Ensure the .PrjPcb was opened.');
        CloseFile(LogFile);
        Exit;
    end;

    WriteLn(LogFile, 'Using project: ' + Project.DM_ProjectFileName);

    OutJobPath := Project.DM_ProjectDirectory + '\Default.OutJob';
    WriteLn(LogFile, 'Targeting OutJob: ' + OutJobPath);

    Document := Client.OpenDocument('OUTPUTJOB', OutJobPath);
    
    if Document <> nil then
    begin
        WriteLn(LogFile, 'OutJob opened. Starting generation...');
        Client.ShowDocument(Document);
        
        ResetParameters;
        AddStringParameter('ObjectKind', 'OutputBatch');
        AddStringParameter('Action', 'Run');
        RunProcess('WorkSpaceManager:GenerateOutputs');
        
        WriteLn(LogFile, 'Generation process triggered.');
        Client.CloseDocument(Document);
    end
    else
    begin
        WriteLn(LogFile, 'Error: Default.OutJob not found or could not be opened.');
    end;
    
    WriteLn(LogFile, 'Process complete. Terminating instance.');
    CloseFile(LogFile);
    Terminate;
end;
