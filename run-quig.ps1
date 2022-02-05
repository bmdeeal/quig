#2022 B.M.Deeal, part of quig for Windows
#This PowerShell script simply displays a file open dialog for a user to select a .quig script to run.
#To run this script, either open run-quig.bat, or right-click this file and select 'Run with PowerShell'.
#This script will only work when run in the same folder as quig-for-windows.exe.

Add-Type -AssemblyName System.Windows.Forms
$FileBrowser = New-Object System.Windows.Forms.OpenFileDialog
$FileBrowser.Filter = '.quig Applications |*.quig'
$FileBrowser.Title = 'Select a .quig file to run...'
$result = $FileBrowser.ShowDialog()
if ($result -eq 'OK') { .\quig-for-windows.exe $FileBrowser.FileName }
