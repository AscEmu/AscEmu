# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
include_guard(GLOBAL)

# mark variables as advanced to not get them in gui
mark_as_advanced(
    git_commit
    git_branch
)

# extract git revision
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE git_commit
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE git_branch
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(ascemu_branch ${git_branch})
set(BUILD_USERNAME $ENV{USERNAME})

configure_file(
    ${CMAKE_SOURCE_DIR}/src/shared/git_version.hpp.in
    ${CMAKE_SOURCE_DIR}/src/shared/git_version.hpp
)
