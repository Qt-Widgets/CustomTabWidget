@echo off
rem --------------------------------------------------------------------------------------------------------------------------------------------
rem check some required environment variables
rem --------------------------------------------------------------------------------------------------------------------------------------------
set OUT_DIR=%CD%\temp
if not exist %OUT_DIR% mkdir %OUT_DIR%

IF "%QTDIR%" == "" GOTO :NO_QTDIR
IF "%VS140COMNTOOLS%" == "" GOTO :NO_VS140COMNTOOLS

rem --------------------------------------------------------------------------------------------------------------------------------------------
echo Running vcvarsall...
rem --------------------------------------------------------------------------------------------------------------------------------------------
call "%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat"

echo Generating solution from qmake files...
%QTDIR%\bin\qmake.exe -spec win32-msvc2015 -tp vc "source.pro" -recursive QMAKE_INCDIR_QT=%QTDIR%\include QMAKE_LIBDIR=%QTDIR%\lib QMAKE_MOC=$(%QTDIR%)\bin\moc.exe QMAKE_QMAKE=$(%QTDIR%)\bin\qmake.exe > %OUT_DIR%\generate_solution_output.txt
if %errorlevel% neq 0 goto :qmakeFail

cd ..

echo qmake complete
goto :end

:qmakeFail
echo ERROR: qmake failed! GLHF ¯\_(ツ)_/¯
goto :end

:NO_QTDIR
echo ERROR: Script needs environment variable QTDIR (example value: C:\Qt\Qt5.4.1\5.4\msvc2015)
GOTO :end

:NO_VS140COMNTOOLS
echo ERROR: Script needs environment variable VS140COMNTOOLS (example value: C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\).
GOTO :end

:end