@ECHO OFF
%external%/make/win/bin/make.exe -f Makefile.patch
IF %ERRORLEVEL% NEQ 0 (
   	exit /b 1
)
