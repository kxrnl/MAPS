@REM  This a simple build file to simplify things :)
@REM  Shoutout claude thanks for writing most of the complicated things

@echo off
rem build and run MAPS
cd /d "%~dp0"
if not exist "build" mkdir "build"
g++ -std=c++20 main.cpp -o ./build/maps.exe
if errorlevel 1 (
	pause
	exit /b 1
)
cls
.\build\maps.exe -h