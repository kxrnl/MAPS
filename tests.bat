@echo off
setlocal
title MAPS V3 Menu Tests
cd /d "%~dp0"

set "MAPS=%~dp0build\maps-demo.exe"
set "SANDBOX=%~dp0test_sandbox"
set "OUT=%SANDBOX%\out.txt"
set /a PASS=0
set /a FAIL=0

if not exist "%MAPS%" (
	echo build\maps.exe not found. Build the V3 binary first.
	pause
	exit /b 1
)

if exist "%SANDBOX%" rmdir /s /q "%SANDBOX%"
mkdir "%SANDBOX%\home" "%SANDBOX%\external" "%SANDBOX%\newroot" "%SANDBOX%\home\Documents\proj"
set "HOME="
set "USERPROFILE=%SANDBOX%\home"

echo === M.A.P.S V3 Menu Tests ===

@REM [3] empty list (no prompt to open), [8] info, [4] templates, [9] help, [0] exit
(
  echo 3
  echo.
  echo 8
  echo.
  echo 4
  echo.
  echo 9
  echo.
  echo 0
) | "%MAPS%" > "%OUT%" 2>&1
call :check "empty list"      "No projects yet"
call :check "info credits"    "Taurus Software"
call :check "templates list"  "Available languages:"
call :check "help text"       "M.A.P.S - Modular Architecture Project Scaffolder"

@REM [1] create cpp project, verify files & .maps marker, then exit
(
  echo 1
  echo calc
  echo cpp
  echo.
  echo 0
) | "%MAPS%" > "%OUT%" 2>&1
call :check "new project"     "Created"

type "%USERPROFILE%\MAPS\C++\calc\src\main.cpp" > "%OUT%" 2>&1
call :check "cpp starter"     "Hello from calc"

dir /b /a "%USERPROFILE%\MAPS\C++\calc" > "%OUT%" 2>&1
call :check "marker file"     ".maps"

@REM [3] list projects: skip once with Enter, then pick #1 with MAPS_NO_EXPLORER=1
(
  echo 3
  echo.
  echo.
  echo 0
) | "%MAPS%" > "%OUT%" 2>&1
call :check "list shows calc" "calc"

set "MAPS_NO_EXPLORER=1"
(
  echo 3
  echo 1
  echo.
  echo 0
) | "%MAPS%" > "%OUT%" 2>&1
set "MAPS_NO_EXPLORER="
call :check "explorer dry-run" "Would open in Explorer:"

@REM [5] show root, [6] change root (y), [7] reset root
(
  echo 5
  echo.
  echo 6
  echo %SANDBOX%\newroot
  echo y
  echo.
  echo 7
  echo.
  echo 0
) | "%MAPS%" > "%OUT%" 2>&1
call :check "set root"        "Head folder set"
call :check "reset root"      "default again"

@REM [2] import external folder (y), block protected folder
mkdir "%SANDBOX%\external\rofind-react"
(
  echo 2
  echo %SANDBOX%\external\rofind-react
  echo cpp
  echo y
  echo.
  echo 2
  echo %SystemRoot%
  echo cpp
  echo.
  echo 0
) | "%MAPS%" > "%OUT%" 2>&1
call :check "import project"    "Imported"
call :check "protected folder"  "protected"

@REM invalid menu choice
(
  echo 99
  echo.
  echo 0
) | "%MAPS%" > "%OUT%" 2>&1
call :check "invalid option"  "Invalid option"

echo.
echo Passed: %PASS% 
echo Failed: %FAIL%
if /i not "%~1"=="nopause" pause
rmdir /s /q "%SANDBOX%"
exit /b %FAIL%

:check
findstr /i /l /c:"%~2" "%OUT%" >nul
if errorlevel 1 (
	set /a FAIL+=1
	echo [FAIL] %~1  ^(expected: %~2^)
) else (
	set /a PASS+=1
	echo [PASS] %~1
)
exit /b