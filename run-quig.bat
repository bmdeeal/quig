@echo off
echo 2022 B.M.Deeal, part of quig for Windows
echo This .bat file will display a file open dialog so you can select a .quig game to run.
echo This script must be in the same folder as both run-quig.ps1 and quig-for-windows.exe, and Microsoft PowerShell must be installed.

echo.
echo -----------------
echo showing UI.......
echo -----------------
echo.

rem : Doing this ignores a baffling, easily ignored default policy setting on Windows, where you cannot run PowerShell scripts directly from a shell by default on a non-server version of Windows.
rem : In addition, double-clicking a .ps1 script will just open it in Notepad, you have to right-click it and select 'Run with PowerShell' to start it, while double-clicking a .bat file will just run it.
rem : This script has neither of those problems.
type .\run-quig.ps1 | powershell