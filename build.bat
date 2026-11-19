@echo off

@REM  This a simple build file to simplify things :)
@REM  Shoutout claude thanks for writing most of the complicated things

@REM Main builder
cd /d "%~dp0"
if not exist "build" mkdir "build"
g++ -std=c++20 ./src/main.cpp -o ./build/maps.exe
if errorlevel 1 (
	pause
	exit /b 1
)
cls

@REM copy templates to the fixed folder the program reads
if not exist "%APPDATA%\MAPS" mkdir "%APPDATA%\MAPS"
if exist "templates" (
	xcopy "templates" "%APPDATA%\MAPS\templates" /e /i /y /q >nul
	if errorlevel 1 echo Warning: could not copy the templates.
) else (
	echo Warning: no templates folder next to build.bat.
)

@REM RUN
.\build\maps.exe help