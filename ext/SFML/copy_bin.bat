rem @echo off
echo Copying SFML bin files to working directory...
xcopy /D /Y "%~dp0bin\*" "%1"