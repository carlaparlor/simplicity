@echo off
setlocal
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\windows\setup.ps1" %*
exit /b %ERRORLEVEL%
