@echo off
set rt11dsk=C:\bin\rt11dsk

del x-ukncbtl\1.dsk
@if exist "x-ukncbtl\1.dsk" (
  echo.
  echo ####### FAILED to delete old disk image file #######
  exit /b
)
copy x-ukncbtl\sys1002ex.dsk 1.dsk
%rt11dsk% a 1.dsk 1.SAV
move 1.dsk x-ukncbtl\1.dsk

@if not exist "x-ukncbtl\1.dsk" (
  echo ####### ERROR disk image file not found #######
  exit /b
)

start x-ukncbtl\UKNCBTL.exe /boot
