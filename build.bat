@echo off

@REM  This a simple build file to simplify things :)
@REM  Shoutout claude thanks for writing most of the complicated things

@REM Main builder
cd /d "%~dp0"
if not exist "build" mkdir "build"
g++ -std=c++20 ./src/main.cpp -o ./build/maps-demo.exe
if errorlevel 1 (
	pause
	exit /b 1
)
cls

@REM Disabled templates in demo mode. Everything is hardcoded

@REM  if not exist "%APPDATA%\MAPS" mkdir "%APPDATA%\MAPS"
@REM  if exist "templates" (
@REM  	xcopy "templates" "%APPDATA%\MAPS\templates" /e /i /y /q >nul
@REM  	if errorlevel 1 echo Warning: could not copy the templates.
@REM  ) else (
@REM  	echo Warning: no templates folder next to build.bat.
@REM  )

@REM  Disabled run after build since it defaults to menu mode (different from v1 and v2)
@REM  .\build\maps.exe help