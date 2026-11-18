@REM  This a simple build file to simplify things :)
@REM  Shoutout claude thanks for writing most of the complicated things

@echo off
rem build and run MAPS
cd /d "%~dp0"
g++ -std=c++20 main.cpp -o maps
if errorlevel 1 (
	pause
	exit /b 1
)
cls
maps.exe -h