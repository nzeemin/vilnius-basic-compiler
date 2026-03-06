@echo off
if exist tests.temp (
  FOR /d /r %%F IN (tests.temp\\*) DO (
    RMDIR /S /Q %%F
  )
  RMDIR /S /Q tests.temp
)