@echo off
setlocal
if not exist build\CMakeCache.txt call configure.bat
if not exist build\CMakeCache.txt exit /b 1
cmake --build build --config Release --target ascemu_launcher
if errorlevel 1 (echo [ERROR] Launcher build failed.& pause & exit /b 1)
echo.
echo [OK] Launcher built in build\bin\ascemu_launcher\Release
pause
