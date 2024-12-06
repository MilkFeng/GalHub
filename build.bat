@echo off

set p1=%1
set p2=%2
set p3=%3

set QT6_PATH=C:\Qt\6.5.3\Qt6.5.3-Windows-x86_64-VS2022-17.10.0-staticFull
set VC_VARS_BAT=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat

set ROOT_DIR=%cd%

set DIST_DIR=%ROOT_DIR%\dist
set DIST_BIN_DIR=%DIST_DIR%\Bin

:: DEBUG MODE =========================================

if "%p1%"=="debug" goto debug_if
if "%p2%"=="debug" goto debug_if
if "%p3%"=="debug" goto debug_if
goto debug_else

:debug_if
echo ^>^> Debug mode enabled
set BUILD_TYPE=Debug
set BUILD_DIR=%ROOT_DIR%\build-debug
echo.

goto debug_end

:debug_else
echo ^>^> Release mode enabled
set BUILD_TYPE=Release
set BUILD_DIR=%ROOT_DIR%\build-release
echo.

:debug_end

:: DEBUG MODE END =====================================

:: CLEAN ==============================================

if "%p1%"=="clean" goto clean_if
if "%p2%"=="clean" goto clean_if
if "%p3%"=="clean" goto clean_if
goto clean_end

:clean_if
echo ^>^> Clean build directory and dist directory
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
if exist "%DIST_BIN_DIR%" rmdir /s /q "%DIST_BIN_DIR%"
if exist "%DIST_DIR%\GalHub.exe" del /q "%DIST_DIR%\GalHub.exe"
echo.

:clean_end

:: CLEAN END ==========================================

if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"
if not exist "%DIST_BIN_DIR%" mkdir "%DIST_BIN_DIR%"

:: BUILD ==============================================


echo ^>^> Setup x86 environment and configure configure cmake project with x86
call "%VC_VARS_BAT%" x86
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G Ninja -S "%ROOT_DIR%" -B "%BUILD_DIR%/x86"
echo.

cd "%BUILD_DIR%/x86"

echo ^>^> Build Hook-x86.dll and Helper.exe
cmake --build . --config %BUILD_TYPE% -j 14 --target Hook Helper
echo.

echo ^>^> Install Hook-x86.dll and Helper.exe to dist directory
cmake --install .
echo.

cd %ROOT_DIR%


echo ^>^> Setup x64 environment and configure cmake project with x64
call "%VC_VARS_BAT%" x64
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G Ninja -S "%ROOT_DIR%" -B "%BUILD_DIR%/x64" -DGENERATE_QT:BOOL=0
echo.

cd "%BUILD_DIR%/x64"

echo ^>^> Build Hook-x64.dll
cmake --build . --config %BUILD_TYPE% -j 14 --target Hook
echo.

echo ^>^> Install Hook-x64.dll to dist directory
cmake --install .
echo.

cd %ROOT_DIR%


echo ^>^> Setup x64 environment and configure cmake project with x64-qt
call "%VC_VARS_BAT%" x64
if %BUILD_TYPE% == Debug (set QT6_ENABLE_CONSOLE=1) else (set QT6_ENABLE_CONSOLE=0)
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja -S "%ROOT_DIR%" -B "%BUILD_DIR%/x64-qt" -DGENERATE_QT:BOOL=1 -DQT6_ENABLE_CONSOLE:BOOL=%QT6_ENABLE_CONSOLE%
echo.

cd "%BUILD_DIR%/x64-qt"

echo ^>^> Build GalHub.exe
cmake --build . --config Release -j 14 --target GalHub
echo.

echo ^>^> Install GalHub.exe to dist directory
cmake --install .
echo.

cd %ROOT_DIR%


if "%p1%"=="run" goto run
if "%p2%"=="run" goto run
if "%p3%"=="run" goto run
exit /b 0

:run
echo ^>^> Run GalHub.exe
cd "%DIST_DIR%"
start GalHub.exe
echo.

exit /b 0