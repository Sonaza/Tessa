@echo off
rem %~dp0

if exist "C:\Program Files\7-Zip\7z.exe" (
	set ZIPEXE="C:\Program Files\7-Zip\7z.exe"
	goto extract
)
if exist "C:\Program Files (x86)\7-Zip\7z.exe" (
	set ZIPEXE="C:\Program Files (x86)\7-Zip\7z.exe"
	goto extract
)
goto exe_not_found

:extract
echo Extracting with %ZIPEXE%

for /R %%I in (*.7z) do (
	call %ZIPEXE% x "%%I" -o"%%~dpI" -aoa
)
rem echo %%~dpI %%I

echo.
goto end

:exe_not_found
echo Error: Can't extract, 7zip executable 7z.exe not found.
echo.

:end