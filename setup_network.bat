@echo off
:: Auto-elevation to Administrator script
:init
setlocal DisableDelayedExpansion
set "batchPath=%~0"
for %%k in (%0) do set batchName=%%~nk
set "vbsGetAdmin=%temp%\OEgetAdmin_%batchName%.vbs"
setlocal EnableDelayedExpansion

:checkPrivileges
NET FILE 1>NUL 2>NUL
if '%errorlevel%' == '0' ( goto gotPrivileges ) else ( goto getPrivileges )

:getPrivileges
if '%1'=='ELEV' (echo ELEV ^& shift ^& goto gotPrivileges)
echo Set UAC = CreateObject^("Shell.Application"^) > "%vbsGetAdmin%"
echo UAC.ShellExecute "!batchPath!", "ELEV", "", "runas", 1 >> "%vbsGetAdmin%"
"%SystemRoot%\System32\Wscript.exe" "%vbsGetAdmin%"
exit /B

:gotPrivileges
if exist "%vbsGetAdmin%" ( del "%vbsGetAdmin%" )
pushd "%CD%"
CD /D "%~dp0"

:: Run the power network setup script bypassing policy restrictions
powershell -ExecutionPolicy Bypass -File "%~dp0setup_network.ps1"
