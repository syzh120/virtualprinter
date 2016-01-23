REM
REM Prepares for the creation of a debug build of the installer
REM

set PREFIX=..\src
set SUFFIX=objchk_wxp_x86\i386
set EMFUTILS=..\..\EmfUtils
set IMAGEMAGICK=C:\Programmi\ImageMagick-6.4.1-Q16
set DEST=i386

copy %PREFIX%\driver\%SUFFIX%\emfprinter_drv.dll %DEST%
copy %PREFIX%\ui\%SUFFIX%\emfprinter_ui.dll %DEST%
copy %PREFIX%\installer\%SUFFIX%\emfprinter_inst.exe %DEST%

REM debug info for debugging
copy %PREFIX%\driver\%SUFFIX%\emfprinter_drv.pdb %DEST%
copy %PREFIX%\ui\%SUFFIX%\emfprinter_ui.pdb %DEST%


REM copy emfutils
copy %EMFUTILS%\emfutils.exe %DEST%

REM copy ImageMagick
copy %IMAGEMAGICK%\*.dll %DEST%\ImageMagick
copy %IMAGEMAGICK%\convert.exe %DEST%\ImageMagick
copy %IMAGEMAGICK%\imdisplay.exe %DEST%\ImageMagick


REM copy common stuff

copy ..\license\gnugpl.txt %DEST%
copy ..\Readme.html %DEST%
copy ..\docs\style.css %DEST%\docs
copy ..\docs\*.png %DEST%\docs
