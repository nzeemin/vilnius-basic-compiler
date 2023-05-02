@echo off
FOR /d /r %%F IN (tests\\*.tmp) DO (
	RMDIR /S /Q %%F
)