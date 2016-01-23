REM
REM CLEAN script: makes sure a full rebuild is done after
REM               executing this script
REM

cd ..\src
del /S *.obj
del /S *.ncb
del /S *.pdb
del /S *.idb
del /S *.exe
del /S *.lib
del /S *.res
del /S *.dll
del /S *.mac
del /S *.exp
del /S *.log
REM del /S *.???_

