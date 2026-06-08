@echo off

set RELEASE=EgoRAM
set SOURCE=%RELEASE%.asm
set TARGET=%RELEASE%.xex
..\bin\mads -o:%TARGET% -l -t %SOURCE%
if errorlevel 1 goto :error
start %TARGET% 
goto :eof

:error
echo ERROR: See messages above.
pause.
goto :eof