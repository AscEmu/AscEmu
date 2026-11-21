@echo off
set VSLANG=1033
setlocal
where cmake >nul 2>nul || (echo [ERROR] CMake was not found in PATH.& pause & exit /b 1)
where dotnet >nul 2>nul || (echo [ERROR] .NET 8 SDK was not found. Install Microsoft.DotNet.SDK.8.& pause & exit /b 1)
echo [AscEmu Launcher] .NET SDK:
dotnet --version
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
if errorlevel 1 (echo [ERROR] CMake configure failed.& pause & exit /b 1)
echo [OK] Configure complete.
pause
