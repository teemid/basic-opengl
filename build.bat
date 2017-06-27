@echo off

SET PROJ_DIR=%~dp0
SET SRC_DIR=%PROJ_DIR%\source
set SOURCES=%SRC_DIR%\main.cpp
SET LIBS=user32.lib gdi32.lib opengl32.lib

IF EXIST build GOTO compile

mkdir build

:compile
    pushd build

        cl %SOURCES% /nologo /I"%PROJ_DIR%include" /EHsc /c /Zi

        link /NOLOGO /DEBUG *.obj %LIBS% /OUT:basic-opengl.exe

    popd
