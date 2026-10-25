@echo off
setlocal
"%~dp0simplicity.exe"
set "result=%ERRORLEVEL%"
echo.
if not "%result%"=="0" echo Simplicity exited with error %result%.
echo This preview is a headless engine test. No DAW editor or audio playback yet.
pause
exit /b %result%
