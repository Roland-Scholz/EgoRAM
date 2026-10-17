@echo off

set ASPECQT=C:\jac\system\Atari800\Tools\SIO\AspecQt\aspeqt.exe
set RELEASE=EgoUridium-Setup
set SOURCE=%RELEASE%.asm
set TARGET=%RELEASE%.xex
..\..\bin\mads -o:%TARGET% -l -t %SOURCE%
if errorlevel 1 goto :error
%ASPECQT% %TARGET%
rem start %TARGET% 
rem pause
goto :eof

:error
echo ERROR: See messages above.
pause.
goto :eof