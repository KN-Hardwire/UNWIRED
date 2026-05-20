Option Explicit

Dim Altium, ProjectPath, ScriptPath, Params, Shell, FSO, Workspace
Set Shell = CreateObject("WScript.Shell")
Set FSO = CreateObject("Scripting.FileSystemObject")

If WScript.Arguments.Count < 2 Then
    WScript.Echo "Error: Missing arguments."
    WScript.Quit 1
End If

ProjectPath = WScript.Arguments(0)
ScriptPath = WScript.Arguments(1)

If Not FSO.FileExists(ProjectPath) Or Not FSO.FileExists(ScriptPath) Then
    WScript.Echo "Error: Files not found."
    WScript.Quit 1
End If

On Error Resume Next
Set Altium = GetObject(, "Altium.Application")

If Altium Is Nothing Then
    WScript.Echo "Starting Altium Designer..."
    Shell.Run """C:\Program Files\Altium\AD26\X2.EXE""", 1, False
    
    Dim timeout, startTime
    timeout = 60
    startTime = Now
    Do While Altium Is Nothing
        WScript.Sleep 2000
        Set Altium = GetObject(, "Altium.Application")
        If DateDiff("s", startTime, Now) > timeout Then
            WScript.Echo "Error: Timeout."
            WScript.Quit 1
        End If
    Loop
End If

Do While Altium.Client Is Nothing
    WScript.Sleep 1000
Loop
On Error GoTo 0

WScript.Echo "Opening Project: " & ProjectPath
Altium.Client.SendMessage "WorkSpaceManager:OpenObject", "FileName=" & ProjectPath
WScript.Sleep 3000

WScript.Echo "Executing Automation Script..."
Params = "ProjectName='" & ProjectPath & "'|ScriptName='" & ScriptPath & "'|ProcName='GenerateOutputs.RunOutJob'"

Altium.Client.RunProcess "ScriptingSystem:RunScript", Params

WScript.Echo "Done."
WScript.Sleep 1000
