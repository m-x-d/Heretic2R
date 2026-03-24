@echo off
set DATESTAMP=%DATE:~10,4%-%DATE:~4,2%-%DATE:~7,2%
set SEVENZIP=c:\Program Files\7-Zip\7z.exe
set ZIP_NAME=.\release\Heretic2R_%DATESTAMP%.zip

if exist %ZIP_NAME% del /q %ZIP_NAME%

:: Default_H2R.cfg is added with incorrect file path to keep relative file path...
"%SEVENZIP%" a -tzip -mx=7 %ZIP_NAME% .\LICENSE .\README.md .\build\Heretic2R.exe .\build\Default_H2R.cfg .\build\*.dll -r