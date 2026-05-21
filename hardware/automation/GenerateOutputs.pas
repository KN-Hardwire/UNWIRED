procedure RunOutJob;
var
    Project : IProject;
    Document : IServerDocument;
    Workspace : IWorkspace;
    OutJobPath : String;
begin
    Workspace := GetWorkspace;
    Project := Workspace.DM_FocusedProject;
    
    if Project = nil then
    begin
        if Workspace.DM_ProjectCount > 0 then
            Project := Workspace.DM_Projects(0);
    end;

    if Project = nil then Exit;

    OutJobPath := Project.DM_ProjectDirectory + '\Default.OutJob';
    Document := Client.OpenDocument('OUTPUTJOB', OutJobPath);
    
    if Document <> nil then
    begin
        Client.ShowDocument(Document);
        
        ResetParameters;
        AddStringParameter('ObjectKind', 'OutputBatch');
        AddStringParameter('Action', 'Run');
        RunProcess('WorkSpaceManager:GenerateOutputs');
        
        Client.CloseDocument(Document);
    end;
    
    Terminate;
end;
