@ECHO OFF
TITLE AscEmu Installation Tool
COLOR 0A

:menu
CLS
ECHO.
ECHO        .####.   #### .####. ##### ##   ## ##  ##
ECHO        ##  ## ##.    ##  ## ##    ### ### ##  ##
ECHO        ###### '####  ##     ####  ## # ## ##  ##
ECHO        ##  ##    '## ##  ## ##    ##   ## ##  ##
ECHO        ##  ## .org#  '####' ##### ##   ##  ####
ECHO                     A never ending place to work.
ECHO.
ECHO    1 - Import Maps.
ECHO    2 - Import Vmaps.
ECHO    3 - Import Mmaps.
ECHO.
ECHO    X - Exit this tool
ECHO.
SET /p v= Enter a char:
IF %v%==* GOTO error
IF %v%==1 GOTO import_maps
IF %v%==2 GOTO import_vmaps
IF %v%==3 GOTO import_mmaps
IF %v%==x GOTO exit
IF %v%==X GOTO exit
IF %v%=="" GOTO exit
GOTO error

:import_maps
mkdir maps
map_extractor.exe
pause
GOTO menu

:import_vmaps
@echo off
mkdir vmaps
vmap4_extractor.exe
vmap4_assembler Buildings vmaps
pause
GOTO menu

:import_mmaps
mkdir mmaps
mmaps_generator.exe
pause
GOTO menu

:error
ECHO	Please enter a correct character.
ECHO.
GOTO menu
pause

:exit
