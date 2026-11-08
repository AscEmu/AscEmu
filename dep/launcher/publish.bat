@echo off
setlocal
if not exist build\CMakeCache.txt call configure.bat
if not exist build\CMakeCache.txt exit /b 1
cmake --build build --config Release --target ascemu_launcher_publish
if errorlevel 1 (echo [ERROR] Launcher publish failed.& pause & exit /b 1)
echo.
echo [OK] Portable launcher published in build\publish\ascemu_launcher
pause
