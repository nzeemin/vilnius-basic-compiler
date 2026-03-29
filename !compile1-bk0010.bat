@echo off

rem Define ESCchar to use in ANSI escape sequences
rem https://stackoverflow.com/questions/2048509/how-to-echo-with-different-colors-in-the-windows-command-line
for /F "delims=#" %%E in ('"prompt #$E# & for %%E in (1) do rem"') do set "ESCchar=%%E"

@if exist 1.MAC del 1.MAC
@if exist 1.lst del 1.lst
@if exist 1.obj del 1.obj
@if exist 1.bin del 1.bin
@if exist 1.SAV del 1.SAV

Debug\vibasc.exe --onefile --turbo8 --platform=BK0010 1.ASC
if errorlevel 1 (
	exit /b
)

x-tools\BKTurbo8_x64.exe -l CO 1.MAC 
if errorlevel 1 (
	exit /b
)
