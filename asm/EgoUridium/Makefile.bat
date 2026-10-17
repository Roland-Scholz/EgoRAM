@rem Run via launch configuration from within WUDSN IDE to see the full logs.
@echo off

call Make-Settings.bat

cd %BASE_DIR%

rem Make main file
call :make

echo Creating disk image.
set ATR=atr\out\EURIDIUM.atr
%DIR2ATR% -d -m -b MyPicoDos406 %ATR% atr\files
if ERRORLEVEL 1 goto :dir2atr_error
echo Done.

goto :eof

:make

echo Compiling loader.
set EXECUTABLE=atr\files\AUTORUN.AR0
if exist %EXECUTABLE% del %EXECUTABLE% 
%MADS% -s EgoUridium-Loader.asm -o:%EXECUTABLE%
if ERRORLEVEL 1 goto :mads_error

echo Compiling setup.
set EXECUTABLE=atr\files\EUSETUP.XEX
if exist %EXECUTABLE% del %EXECUTABLE% 
%MADS% -s EgoUridium-Setup.asm -o:%EXECUTABLE%
if ERRORLEVEL 1 goto :mads_error

echo Compiling main.
set EXECUTABLE=atr\files\EUMAIN.XEX
%MADS% -s EgoUridium.asm -l  -o:%EXECUTABLE%
if ERRORLEVEL 1 goto :mads_error

goto :eof

:mads_error
echo ERROR: MADS compilation errors occurred. Check error messages above.
exit

:dir2atr_error
echo ERROR: DIR2ATR errors occurred. Check error messages above.
exit
