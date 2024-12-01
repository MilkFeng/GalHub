@echo off

set p1=%1
set p2=%2
set p3=%3

set QT6_PATH=C:\Qt\6.5.3\Qt6.5.3-Windows-x86_64-VS2022-17.10.0-staticFull
set VC_VARS_BAT=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat

set ROOT_DIR=%cd%

set BUILD_X64_DIR=%ROOT_DIR%\build-x64
set BUILD_X86_DIR=%ROOT_DIR%\build-x86

set DIST_DIR=%ROOT_DIR%\dist

:: DEBUG MODE =========================================

if "%p1%"=="debug" goto debug_if
if "%p2%"=="debug" goto debug_if
if "%p3%"=="debug" goto debug_if
goto debug_else

:debug_if
echo Debug MODE
set BUILD_TYPE=Debug
set BUILD_X64_DIR=%BUILD_X64_DIR%-debug
set BUILD_X86_DIR=%BUILD_X86_DIR%-debug
goto debug_end

:debug_else
echo Release MODE
set BUILD_TYPE=Release

:debug_end

:: DEBUG MODE END =====================================

:: CLEAN ==============================================

if "%p1%"=="clean" goto clean_if
if "%p2%"=="clean" goto clean_if
if "%p3%"=="clean" goto clean_if
goto clean_end

:clean_if
echo Clean
rmdir /s /q "%BUILD_X64_DIR%"
rmdir /s /q "%BUILD_X86_DIR%"
rmdir /s /q "%DIST_DIR%"

:clean_end

:: CLEAN END ==========================================

if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"

:: BUILD ==============================================

set DIST_BIN_DIR=%DIST_DIR%\Bin
if not exist "%DIST_BIN_DIR%" mkdir "%DIST_BIN_DIR%"



call "%VC_VARS_BAT%" x86

cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G Ninja -S "%ROOT_DIR%" -B "%BUILD_X86_DIR%"
cd "%BUILD_X86_DIR%"
cmake --build . --config %BUILD_TYPE% -j 14 --target Hook Helper

echo Copy Hook-x86.dll
copy "%BUILD_X86_DIR%\Hook.dll" "%DIST_BIN_DIR%\Hook-x86.dll" /b

echo Copy Helper.exe
copy "%BUILD_X86_DIR%\Helper.exe" "%DIST_BIN_DIR%\Helper.exe" /b

cd %ROOT_DIR%



call "%VC_VARS_BAT%" x64

cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G Ninja -S "%ROOT_DIR%" -B "%BUILD_X64_DIR%"
cd "%BUILD_X64_DIR%"
cmake --build . --config %BUILD_TYPE% -j 14 --target Hook GalHub

echo Copy Hook-x64.dll
copy "%BUILD_X64_DIR%\Hook.dll" "%DIST_BIN_DIR%\Hook-x64.dll" /b

echo Copy GalHub.exe
copy "%BUILD_X64_DIR%\GalHub.exe" "%DIST_DIR%\GalHub.exe" /b

cd %ROOT_DIR%



if "%p1%"=="run" goto run
if "%p2%"=="run" goto run
if "%p3%"=="run" goto run
exit /b 0

:run
echo Run
cd "%DIST_DIR%"
start GalHub.exe

exit /b 0