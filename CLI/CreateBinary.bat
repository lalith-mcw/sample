@echo off

:: Generate offcialRun.json
del ..\workloads\pezcore\workload.json > nul 2>&1
node.exe main.js -g -p ..\workloads
IF %ERRORLEVEL% NEQ 0 (
   goto ERROR
)

::Requirement - Install pkg using npm before running this script
move officialRun.json ..\
pkg package.json -o ..\SPECworkstation.exe && echo "Generated exe file"

:ERROR
ECHO "ERROR occurred in offficialRun.json creation."
ECHO "Aborting Binary Generation..."
