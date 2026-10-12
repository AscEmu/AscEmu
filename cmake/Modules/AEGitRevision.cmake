# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

find_package(Git QUIET)

set(git_commit "unknown")
set(git_branch "unknown")
set(git_commit_timestamp "unknown")
set(git_build_user "$ENV{USERNAME}")
set(git_build_host "$ENV{COMPUTERNAME}")

if(UNIX)
    if(NOT git_build_user)
        set(git_build_user "$ENV{USER}")
    endif()
    if(NOT git_build_host)
        execute_process(
            COMMAND hostname
            OUTPUT_VARIABLE git_build_host
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
endif()

if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short=7 HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE git_commit
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE git_branch
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" log -1 --format=%cI
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE git_commit_timestamp
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

set(ascemu_branch ${git_branch})
