@echo off
echo Running Post Build Script...

set CONFIGURATION=%1
set ROOT_DIR=%~dp0..\..

set WORKING_DIRECTORY_PATH=%ROOT_DIR%\workdir
set EXTERNALS_DIRECTORY_PATH=%ROOT_DIR%\ext

rem Create work directory if it doesn't exist
if NOT exist "%WORKING_DIRECTORY_PATH%" (
	echo Creating missing working directory...
	mkdir %WORKING_DIRECTORY_PATH%
	if %errorlevel% NEQ 0 goto ERROR
) else (
	echo Working directory OK.
)

rem Run externals build step
echo Running externals build step...
call "%EXTERNALS_DIRECTORY_PATH%\externals_build_step.bat" %CONFIGURATION% %WORKING_DIRECTORY_PATH%
if %errorlevel% NEQ 0 goto ERROR

if /I "%CONFIGURATION%" == "release" goto COPY_EXE
if /I "%CONFIGURATION%" == "finalrelease" goto COPY_EXE
goto FINISH

:COPY_EXE
echo Copying %ROOT_DIR%\builds\%CONFIGURATION%\Ivie.exe to %WORKING_DIRECTORY_PATH%\Ivie.exe
copy %ROOT_DIR%\builds\%CONFIGURATION%\Ivie.exe %WORKING_DIRECTORY_PATH%\Ivie.exe

:FINISH

echo Post Build Script complete!

exit 0

:ERROR
echo An error was encountered during Post Build Script. Check logs.
exit 1