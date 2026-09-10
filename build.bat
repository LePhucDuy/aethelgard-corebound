@echo off
setlocal enabledelayedexpansion

echo =======================================================
echo   Aethelgard: Corebound - C++17 Roguelike Build System
echo =======================================================

if exist "C:\msys64\ucrt64\bin" (
    set "PATH=C:\msys64\ucrt64\bin;%PATH%"
)

where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] Cannot find g++.exe! Please ensure MSYS2 ucrt64 is installed.
    exit /b 1
)

if not exist "bin" mkdir bin

rem Sao chep cac DLL can thiet neu chua co de game chay doc lap
if exist "C:\msys64\ucrt64\bin\libraylib.dll" if not exist "bin\libraylib.dll" copy "C:\msys64\ucrt64\bin\libraylib.dll" "bin\" >nul
if exist "C:\msys64\ucrt64\bin\libstdc++-6.dll" if not exist "bin\libstdc++-6.dll" copy "C:\msys64\ucrt64\bin\libstdc++-6.dll" "bin\" >nul
if exist "C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll" if not exist "bin\libgcc_s_seh-1.dll" copy "C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll" "bin\" >nul
if exist "C:\msys64\ucrt64\bin\libwinpthread-1.dll" if not exist "bin\libwinpthread-1.dll" copy "C:\msys64\ucrt64\bin\libwinpthread-1.dll" "bin\" >nul
if exist "C:\msys64\ucrt64\bin\glfw3.dll" if not exist "bin\glfw3.dll" copy "C:\msys64\ucrt64\bin\glfw3.dll" "bin\" >nul

echo [INFO] Collecting source files...
set SOURCES=
for /R src %%f in (*.cpp) do (
    set "SOURCES=!SOURCES! "%%f""
)

echo [INFO] Compiling C++17 with Raylib...
g++ -std=c++17 -O2 -Wall -Iinclude !SOURCES! -lraylib -lopengl32 -lgdi32 -lwinmm -o bin\aethelgard.exe

if %errorlevel% equ 0 (
    echo =======================================================
    echo [SUCCESS] Build completed successfully: bin\aethelgard.exe
    echo   Chay game bang lenh: .\bin\aethelgard.exe
    echo =======================================================
) else (
    echo =======================================================
    echo [FAILED] Compilation failed with error code %errorlevel%
    echo =======================================================
    exit /b %errorlevel%
)
