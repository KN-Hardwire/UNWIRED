procedure RunOutJob;
var
    Project : IProject;
    Document : IServerDocument;
    Workspace : IWorkspace;
begin
    Workspace := GetWorkspace;
    Project := Workspace.DM_FocusedProject;
    
    if Project = nil then
    begin
        // If no project is focused, try to find the first one in the workspace
        if Workspace.DM_ProjectCount > 0 then
            Project := Workspace.DM_Projects(0);
    end;

    if Project = nil then Exit;

    // Open the Default.OutJob file
    // Assumes the file is named 'Default.OutJob' and is in the project directory
    Document := Client.OpenDocument('OUTPUTJOB', Project.DM_ProjectDirectory + '\Default.OutJob');
    
    if Document <> nil then
    begin
        Client.ShowDocument(Document);
        
        // Trigger the generation process
        ResetParameters;
        AddStringParameter('ObjectKind', 'OutputBatch');
        AddStringParameter('Action', 'Run');
        RunProcess('WorkSpaceManager:GenerateOutputs');
    end;
end;
