@echo off
REM
REM Reinstaller script: to use when testing changes to the driver,
REM                     driverUI or print processor
REM
REM NOTE: requires that you have installed ddkbuild.cmd
REM       (see docs\tech.txt)
REM

set EMFPRINTER_DIR="%CD%\.."


REM clean everything

CALL clean.bat
cd %EMFPRINTER_DIR%\distrib
CALL clean.bat


REM rebuild all

cd %EMFPRINTER_DIR%
CALL ddkbuild.cmd -WXP checked .


REM put everything in distrib\i386

cd %EMFPRINTER_DIR%\distrib
CALL copy-checked-build.bat


REM uninstall the system-wide stuff

REM make sure the spooler is running
net start spooler

cd %EMFPRINTER_DIR%\distrib\i386
emfprinter_inst.exe uninstall

net stop spooler
net start spooler


REM install the new stuff

cd %EMFPRINTER_DIR%\distrib\i386
emfprinter_inst.exe install

echo -----------------------------------------------------
echo Remember to use the ProcessExplorer to restart
echo EXPLORER so that new DLLs are loaded in RAM
echo -----------------------------------------------------

pause
