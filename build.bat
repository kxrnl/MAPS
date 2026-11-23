@echo off

@REM  This a simple build file to simplify things :)
@REM  Shoutout claude thanks for writing most of the complicated things

@REM Main builder
cd /d "%~dp0"
if not exist "build" mkdir "build"
g++ -std=c++20 -static ./src/main.cpp -o ./build/maps-demo.exe
if errorlevel 1 (
	pause
	exit /b 1
) else (
	cls
	echo [Build] Success.
	pause
)
cls
