Const MY_COMPUTER = &H11&
Const WINDOW_HANDLE = 0
Const OPTIONS = &H10&

Set objShell = CreateObject("Shell.Application")

strPath = objShell.Namespace(MY_COMPUTER).Self.Path
Set objShell = CreateObject("Shell.Application")
Set objFolder = objShell.BrowseForFolder(WINDOW_HANDLE, "Select a folder:", OPTIONS, strPath)

If objFolder Is Nothing Then
	Wscript.Quit
End If

Wscript.Echo objFolder.Self.Path
