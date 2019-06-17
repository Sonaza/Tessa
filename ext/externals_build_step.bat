@echo off
rem Externals build step, like for copying stuff and things 

set CONFIGURATION=%1
set WORKING_DIRECTORY=%2

if /I "%CONFIGURATION%" == "debug" (
	call "%~dp0vld\copy_bin.bat" %WORKING_DIRECTORY%
) else (
	echo Skipping Visual Leak Detector copy in %CONFIGURATION% build.
)

