@echo off
cd %1
if exist "workdir" goto after_workdir
echo Creating missing workdir
mkdir workdir
:after_workdir