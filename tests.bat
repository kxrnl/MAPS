@@REM  CLAUDE WROTE THIS

@@REM  MAPS test suite. Put it next to build.bat and run it after building.
@echo off
setlocal
title MAPS test suite
cd /d "%~dp0"

set "MAPS=%~dp0build\maps.exe"
set "SANDBOX=%~dp0test_sandbox"
set "OUT=%SANDBOX%\out.txt"
set /a PASS=0
set /a FAIL=0

if not exist "%MAPS%" (
	echo build\maps.exe not found. Run build.bat first.
	pause
	exit /b 1
)
if not exist "%~dp0templates" (
	echo templates folder not found next to this file.
	pause
	exit /b 1
)

@REM sandbox: MAPS reads these variables, so your real settings and folders are never touched
if exist "%SANDBOX%" rmdir /s /q "%SANDBOX%"
mkdir "%SANDBOX%\home" "%SANDBOX%\appdata" "%SANDBOX%\external"
set "HOME="
set "USERPROFILE=%SANDBOX%\home"
set "APPDATA=%SANDBOX%\appdata"
xcopy "%~dp0templates" "%APPDATA%\MAPS\templates" /e /i /y /q >nul

cls
echo ==========================================================
echo   M.A.P.S test suite
echo   %DATE% %TIME%
echo   Program : %MAPS%
echo   Sandbox : %SANDBOX%
echo ==========================================================

@REM ---------------- usage and basics ----------------
call :t "01" "maps   [no arguments]"
"%MAPS%" > "%OUT%" 2>&1
call :expect "expected an argument"

call :t "02" "maps -h"
"%MAPS%" -h > "%OUT%" 2>&1
call :expect "Usage:"

call :t "03" "maps help"
"%MAPS%" help > "%OUT%" 2>&1
call :expect "Usage:"

call :t "04" "maps ?"
"%MAPS%" ? > "%OUT%" 2>&1
call :expect "Usage:"

call :t "05" "maps info   [before anything exists]"
"%MAPS%" info > "%OUT%" 2>&1
call :expect "not made yet"

call :t "06" "maps list   [no projects yet]"
"%MAPS%" list > "%OUT%" 2>&1
call :expect "No projects yet"

call :t "07" "maps templates"
"%MAPS%" templates > "%OUT%" 2>&1
call :expect "Templates folder:"

call :t "08" "maps foo   [unknown command]"
"%MAPS%" foo > "%OUT%" 2>&1
call :expect "Unknown command"

@REM ---------------- new ----------------
call :t "09" "maps new calc cpp"
"%MAPS%" new calc cpp > "%OUT%" 2>&1
call :expect "Created"

call :t "10" "starter file has the project name"
type "%USERPROFILE%\MAPS\C++\calc\main.cpp" > "%OUT%" 2>&1
call :expect "Hello from calc"

call :t "11" "marker file .maps exists in the project"
dir /b /a "%USERPROFILE%\MAPS\C++\calc" > "%OUT%" 2>&1
call :expect ".maps"

call :t "12" "maps new calc2 c++   [alias]"
"%MAPS%" new calc2 c++ > "%OUT%" 2>&1
call :expect "C++\calc2"

call :t "13" "maps new site web"
"%MAPS%" new site web > "%OUT%" 2>&1
call :expect "Created"

call :t "14" "maps new snake python   [alias]"
"%MAPS%" new snake python > "%OUT%" 2>&1
call :expect "PY\snake"

call :t "15" "maps new tool cpp structured   [template type]"
"%MAPS%" new tool cpp structured > "%OUT%" 2>&1
call :expect "Created"

call :t "16" "structured type made the src folder"
dir /s /b "%USERPROFILE%\MAPS\C++\tool" > "%OUT%" 2>&1
call :expect "src\main.cpp"

call :t "17" "maps new calc cpp   [duplicate]"
"%MAPS%" new calc cpp > "%OUT%" 2>&1
call :expect "already exists"

call :t "18" "maps new bad:name cpp   [invalid name]"
"%MAPS%" new "bad:name" cpp > "%OUT%" 2>&1
call :expect "invalid name"

call :t "19" "maps new demo rust   [no template]"
"%MAPS%" new demo rust > "%OUT%" 2>&1
call :expect "no template"

call :t "20" "maps list   [shows marked projects only]"
mkdir "%USERPROFILE%\MAPS\Tools\SCRCPY"
"%MAPS%" list > "%OUT%" 2>&1
call :expect "calc"
call :expectno "SCRCPY"

@REM ---------------- root ----------------
call :t "21" "maps root"
"%MAPS%" root > "%OUT%" 2>&1
call :expect "MAPS"

call :t "22" "maps root relative   [needs the full path]"
"%MAPS%" root relative > "%OUT%" 2>&1
call :expect "full path"

call :t "23" "maps root C:\does\not\exist   [folder must exist]"
"%MAPS%" root C:\does\not\exist > "%OUT%" 2>&1
call :expect "does not exist"

call :t "24" "maps root C:\   [drive root]"
"%MAPS%" root %SystemDrive%\ > "%OUT%" 2>&1
call :expect "drive root"

mkdir "%SANDBOX%\newroot"
call :t "25" "maps root newroot   [answer n]"
echo n | "%MAPS%" root "%SANDBOX%\newroot" > "%OUT%" 2>&1
call :expect "Cancelled"

call :t "26" "maps root newroot   [answer y]"
echo y | "%MAPS%" root "%SANDBOX%\newroot" > "%OUT%" 2>&1
call :expect "Head folder set"

call :t "27" "maps root   [shows the new head folder]"
"%MAPS%" root > "%OUT%" 2>&1
call :expect "newroot"

call :t "28" "maps root reset"
"%MAPS%" root reset > "%OUT%" 2>&1
call :expect "default again"

@REM ---------------- import ----------------
mkdir "%SANDBOX%\external\rofind-react\src"
echo test > "%SANDBOX%\external\rofind-react\src\a.js"

call :t "29" "maps import rofind-react web   [answer n]"
echo n | "%MAPS%" import "%SANDBOX%\external\rofind-react" web > "%OUT%" 2>&1
call :expect "Cancelled"

call :t "30" "folder is still in the old place"
dir /b "%SANDBOX%\external" > "%OUT%" 2>&1
call :expect "rofind-react"

call :t "31" "maps import rofind-react web   [answer y]"
echo y | "%MAPS%" import "%SANDBOX%\external\rofind-react" web > "%OUT%" 2>&1
call :expect "Imported"

call :t "32" "maps list   [imported project shows]"
"%MAPS%" list > "%OUT%" 2>&1
call :expect "rofind-react"

mkdir "%SANDBOX%\external\rofind-react"
call :t "33" "maps import   [name already exists]"
echo n | "%MAPS%" import "%SANDBOX%\external\rofind-react" web > "%OUT%" 2>&1
call :expect "already exists"

call :t "34" "maps import   [folder does not exist]"
echo n | "%MAPS%" import "%SANDBOX%\external\nothere" web > "%OUT%" 2>&1
call :expect "does not exist"

call :t "35" "maps import   [invalid language]"
echo n | "%MAPS%" import "%SANDBOX%\external\rofind-react" "a:b" > "%OUT%" 2>&1
call :expect "invalid language"

mkdir "%USERPROFILE%\MAPS\PY\oldproj"
call :t "36" "maps import   [project already in place, adds the marker]"
"%MAPS%" import "%USERPROFILE%\MAPS\PY\oldproj" py > "%OUT%" 2>&1
call :expect "Added"

call :t "37" "maps list   [old project now shows]"
"%MAPS%" list > "%OUT%" 2>&1
call :expect "oldproj"

@REM ---------------- protected folders (answer n, so nothing can move) ----------------
mkdir "%USERPROFILE%\Documents\proj"
mkdir "%USERPROFILE%\.ssh"
mkdir "%USERPROFILE%\OneDrive\Documents"

call :t "38" "maps import C:\Users web   [top level folder]"
echo n | "%MAPS%" import "%SystemDrive%\Users" web > "%OUT%" 2>&1
call :expect "protected"

call :t "39" "maps import C:\Windows web   [system folder]"
echo n | "%MAPS%" import "%SystemRoot%" web > "%OUT%" 2>&1
call :expect "protected"

call :t "40" "maps import C:\Windows\System32 web   [inside a system folder]"
echo n | "%MAPS%" import "%SystemRoot%\System32" web > "%OUT%" 2>&1
call :expect "protected"

call :t "41" "maps import C:\Program Files web   [top level folder]"
echo n | "%MAPS%" import "%ProgramFiles%" web > "%OUT%" 2>&1
call :expect "protected"

call :t "42" "maps import C:\Program Files\Common Files web   [inside Program Files]"
echo n | "%MAPS%" import "%ProgramFiles%\Common Files" web > "%OUT%" 2>&1
call :expect "protected"

call :t "43" "maps import C:\ProgramData web   [top level folder]"
echo n | "%MAPS%" import "%ProgramData%" web > "%OUT%" 2>&1
call :expect "protected"

call :t "44" "maps import LocalAppData\Temp web   [inside AppData]"
echo n | "%MAPS%" import "%LOCALAPPDATA%\Temp" web > "%OUT%" 2>&1
call :expect "protected"

call :t "45" "maps import AppData web   [the MAPS templates live there]"
echo n | "%MAPS%" import "%APPDATA%" web > "%OUT%" 2>&1
call :expect "protected"

call :t "46" "maps import home folder web"
echo n | "%MAPS%" import "%USERPROFILE%" web > "%OUT%" 2>&1
call :expect "home folder"

call :t "47" "maps import Documents web   [user folder]"
echo n | "%MAPS%" import "%USERPROFILE%\Documents" web > "%OUT%" 2>&1
call :expect "user folder"

call :t "48" "maps import .ssh web   [settings folder]"
echo n | "%MAPS%" import "%USERPROFILE%\.ssh" web > "%OUT%" 2>&1
call :expect "settings folder"

call :t "49" "maps import OneDrive\Documents web   [user folder]"
echo n | "%MAPS%" import "%USERPROFILE%\OneDrive\Documents" web > "%OUT%" 2>&1
call :expect "user folder"

call :t "50" "maps import head folder web   [holds the head folder]"
echo n | "%MAPS%" import "%USERPROFILE%\MAPS" web > "%OUT%" 2>&1
call :expect "holds the head folder"

call :t "51" "maps import head folder\PY web   [language folder]"
echo n | "%MAPS%" import "%USERPROFILE%\MAPS\PY" web > "%OUT%" 2>&1
call :expect "language folder"

call :t "52" "maps import project folder web   [holds the program]"
echo n | "%MAPS%" import "%~dp0." web > "%OUT%" 2>&1
call :expect "holds the program"

call :t "53" "maps import Documents\proj web   [allowed, answer n]"
echo n | "%MAPS%" import "%USERPROFILE%\Documents\proj" web > "%OUT%" 2>&1
call :expect "Cancelled"
call :expectno "protected"

call :t "54" "maps info   [after setup]"
"%MAPS%" info > "%OUT%" 2>&1
call :expect "Head folder"
call :expectno "not made yet"

goto :summary

@REM ---------------- helpers ----------------
:t
echo.
echo [%~1] %~2
exit /b

:expect
type "%OUT%"
findstr /i /l /c:"%~1" "%OUT%" >nul
if errorlevel 1 (
	set /a FAIL+=1
	echo   [FAIL] expected: %~1
) else (
	set /a PASS+=1
	echo   [PASS]
)
exit /b

:expectno
findstr /i /l /c:"%~1" "%OUT%" >nul
if errorlevel 1 (
	set /a PASS+=1
	echo   [PASS] does not contain: %~1
) else (
	set /a FAIL+=1
	echo   [FAIL] must not contain: %~1
)
exit /b

:summary
echo.
echo ==========================================================
echo   RESULT: %PASS% passed, %FAIL% failed
echo ==========================================================
rmdir /s /q "%SANDBOX%"
if /i not "%~1"=="nopause" pause
exit /b %FAIL%