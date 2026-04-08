@echo off

:: Get revision number. Can't use Heretic2R.rc for this because of UTF-16 encoding...
set QC_PATH=.\src\qcommon\qcommon.h

:: Look for '#define VERSIONDISP "R667"' line...
for /f "usebackq tokens=2,3" %%a in ("%QC_PATH%") do (
	if "%%a"=="VERSIONDISP" (
		set H2R_REV=%%~b
		goto :break_loop
	)
)

:break_loop

if "%H2R_REV%"=="" (
	echo Failed to find revision!
	pause
	exit
)

:: Now pack it.
set SEVENZIP=c:\Program Files\7-Zip\7z.exe
set ZIP_NAME=.\release\Heretic2R_%H2R_REV%.zip

if exist %ZIP_NAME% del /q %ZIP_NAME%

:: Default_H2R.cfg is added with incorrect file path to keep relative file path...
"%SEVENZIP%" a -tzip -mx=7 %ZIP_NAME% .\LICENSE .\README.md .\build\Heretic2R.exe .\build\Default_H2R.cfg .\build\*.dll -r