@echo off
title Fix DLL Issues
cls

echo Fixing DLL problems...
echo.

cd /d "%~dp0"

:: Method 1: Copy DLLs to Windows directory (requires admin)
echo Method 1: Copying DLLs to system directory...
copy sfml-*.dll %windir%\system32\ 2>nul
copy openal32.dll %windir%\system32\ 2>nul

:: Method 2: Register DLLs
echo Method 2: Registering DLLs...
regsvr32 sfml-*.dll /s 2>nul

echo.
echo Fix applied!
echo Now try running the game again.
pause
