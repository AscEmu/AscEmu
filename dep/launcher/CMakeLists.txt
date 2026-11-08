cmake_minimum_required(VERSION 3.24)
project(AscEmuLauncher LANGUAGES NONE)

set(ASCEMU_LAUNCHER_RUNTIME "win-x64" CACHE STRING "Target .NET runtime identifier")
set(ASCEMU_LAUNCHER_CONFIGURATION "Release" CACHE STRING "Launcher configuration")
option(ASCEMU_LAUNCHER_SELF_CONTAINED "Publish a self-contained launcher" ON)
option(ASCEMU_LAUNCHER_SINGLE_FILE "Publish as a single executable" ON)

include(cmake/AscEmuDotNet.cmake)
add_subdirectory(src/AscEmu.Launcher)
