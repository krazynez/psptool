@echo off

SET compress=0
SET appname=PSP Tool
SET outpath=PSP\GAME

SET PARAM_SFO=src\PARAM.SFO
SET ICON0_PNG=src\data\images\ICON0.PNG
SET DATA_PSP=src\main.prx
SET DATA_PSAR=src\data\files.zip

del /f /q "%PARAM_SFO%"
del /f /q "%DATA_PSP%"
rmdir /s /q "%outpath%"
mkdir "%outpath%\%appname%"

src\tools\compiler.exe compress=%compress% appname="%appname%" outpath="%outpath%"

src\tools\mksfo.exe "%appname%" "%PARAM_SFO%"
src\tools\pack-pbp.exe "%outpath%\%appname%\EBOOT.PBP" "%PARAM_SFO%" "%ICON0_PNG%" NULL NULL NULL NULL "%DATA_PSP%" "%DATA_PSAR%"
src\tools\sha1add.exe "%outpath%\%appname%\EBOOT.PBP" "%outpath%\%appname%\EBOOT.PBP"

del /f /q "%PARAM_SFO%"
del /f /q "%DATA_PSP%"