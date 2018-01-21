# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

# mark variables as advanced to not get them in gui
mark_as_advanced(
    git_commit
    git_tag
    git_time
    BUILD_HOSTNAME
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
  OUTPUT_VARIABLE git_tag
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND git log -1 --format=%ct
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE git_time
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(ascemu_tag ${git_tag})
site_name(BUILD_HOSTNAME)
set(BUILD_USERNAME $ENV{USERNAME})

configure_file(
  ${CMAKE_SOURCE_DIR}/src/shared/git_version.h.in
  ${CMAKE_SOURCE_DIR}/src/shared/git_version.h
)