REM
REM Prepares for the creation of a release build of the installer
REM

set PREFIX=..\src
set SUFFIX=objfre_wnet_x86\i386
set DEST=i386

copy %PREFIX%\driver\%SUFFIX%\emfprinter_drv.dll %DEST%
copy %PREFIX%\ui\%SUFFIX%\emfprinter_ui.dll %DEST%

copy %PREFIX%\installer\%SUFFIX%\emfprinter_inst.exe %DEST%
