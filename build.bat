@echo off
setlocal

:: Setup Paths
set QT_PATH=C:\Qt\6.10.1\mingw_64
set MINGW_PATH=C:\Qt\Tools\mingw1310_64
set NINJA_PATH=C:\tmp\mingw64\bin
set CMAKE_PATH=C:\Qt\Tools\CMake_64\bin

:: Ensure MinGW is prioritized in PATH
set PATH=%MINGW_PATH%\bin;%QT_PATH%\bin;%NINJA_PATH%;%CMAKE_PATH%;%PATH%

set PROJECT_NAME=BouyomiSchedule
set BUILD_TYPE=Release
if "%1"=="debug" set BUILD_TYPE=Debug

echo --- Building %PROJECT_NAME% [%BUILD_TYPE%] ---

if not exist build mkdir build
cd build

:: Run CMake with explicit compiler specification
cmake -G "Ninja" ^
    -DCMAKE_CXX_COMPILER="%MINGW_PATH%\bin\g++.exe" ^
    -DCMAKE_C_COMPILER="%MINGW_PATH%\bin\gcc.exe" ^
    -DCMAKE_PREFIX_PATH="%QT_PATH%" ^
    -DCMAKE_MAKE_PROGRAM="%NINJA_PATH%\ninja.exe" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    ..

if %ERRORLEVEL% neq 0 (
    echo CMake failed.
    exit /b %ERRORLEVEL%
)

:: Run Ninja
ninja
if %ERRORLEVEL% neq 0 (
    echo Build failed.
    exit /b %ERRORLEVEL%
)

:: Deploy dependencies (Release only)
if "%BUILD_TYPE%"=="Release" (
    echo Deploying dependencies...
    windeployqt --release --no-translations --compiler-runtime ..\bin\%PROJECT_NAME%.exe
)

echo --- Build Successful ---
cd ..
