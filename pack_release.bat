@echo off

:: Get revision number.
set VER_PATH=.\src\qcommon\Version.h
set H2R_REV=0

:: Look for '#define VERSION_REVISION 667' line...
for /f "tokens=2,3" %%a in (%VER_PATH%) do (
	if %%a==VERSION_REVISION (
		set H2R_REV=%%b
		goto :break_loop
	)
)

:break_loop

if %H2R_REV%==0 (
	echo Failed to find revision!
	pause
	exit
)

:: Now pack it.
set SEVENZIP=c:\Program Files\7-Zip\7z.exe
set ZIP_NAME=.\release\Heretic2R_R%H2R_REV%.zip

if exist %ZIP_NAME% del /q %ZIP_NAME%

:: Default_H2R.cfg is added with incorrect file path to keep relative file path...
"%SEVENZIP%" a -tzip -mx=7 %ZIP_NAME% .\LICENSE .\README.md .\build\Heretic2R.exe .\build\Default_H2R.cfg .\build\*.dll -r