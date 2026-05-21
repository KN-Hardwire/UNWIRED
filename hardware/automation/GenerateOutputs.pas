procedure RunOutJob;
var
    Project : IProject;
    Document : IServerDocument;
    Workspace : IWorkspace;
    OutJobPath : String;
    LogFile : TextFile;
    LogPath : String;
begin
    // Use a universally writable location for the log
    LogPath := 'C:\Users\Public\altium_log.txt';
    AssignFile(LogFile, LogPath);
    Rewrite(LogFile);
    WriteLn(LogFile, 'START');

    Workspace := GetWorkspace;
    Sleep(5000);

    Project := Workspace.DM_FocusedProject;
    if Project = nil then
    begin
        if Workspace.DM_ProjectCount > 0 then
            Project := Workspace.DM_Projects(0);
    end;

    if Project = nil then 
    begin
        WriteLn(LogFile, 'ERROR: NO_PROJECT');
        CloseFile(LogFile);
        Exit;
    end;

    OutJobPath := Project.DM_ProjectDirectory + '\Default.OutJob';
    WriteLn(LogFile, 'Opening: ' + OutJobPath);
    Document := Client.OpenDocument('OUTPUTJOB', OutJobPath);
    
    if Document <> nil then
    begin
        Client.ShowDocument(Document);
        // Wait for the document to fully activate before running the command
        Sleep(2000); 
        
        WriteLn(LogFile, 'Triggering OutputBatch...');
        ResetParameters;
        AddStringParameter('ObjectKind', 'OutputBatch');
        AddStringParameter('Action', 'Run');
        RunProcess('WorkSpaceManager:GenerateOutputs');
        
        Client.CloseDocument(Document);
        WriteLn(LogFile, 'SUCCESS');
    end
    else
    begin
        WriteLn(LogFile, 'ERROR: NO_OUTJOB');
    end;
    
    WriteLn(LogFile, 'FINISH');
    CloseFile(LogFile);
    Terminate;
end;
