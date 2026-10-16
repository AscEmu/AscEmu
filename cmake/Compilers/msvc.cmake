# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# MSVC >= 19.29
set(MSVC_SUPPORTS_VERSION 19.29.30140.0)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MSVC_SUPPORTS_VERSION)
    message(FATAL_ERROR "AscEmu requires version ${MSVC_SUPPORTS_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else ()
    message(STATUS "Minimum version MSVC required is ${MSVC_SUPPORTS_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - success")
endif ()

message(STATUS "Applying settings for ${CMAKE_CXX_COMPILER_ID}")

# windows math include does not define constants by default.
# set this definition so it does.
# also set NOMINMAX so the min and max functions are not overwritten with macros.
add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    _USE_MATH_DEFINES
    NOMINMAX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set defines for MSVC
add_compile_options(
	/EHa 
	/MP 
	/bigobj 
	/permissive-
    /utf-8
)

# set build platform specific settings (x86/x64)
if (NOT IS_64BIT)
    add_link_options(/LARGEADDRESSAWARE)
endif ()

if (TREAT_WARNINGS_AS_ERRORS)
    add_compile_options(/WX)
endif ()

# enable/disable warnings
# dll warning 4251 disabled by default.
if (BUILD_WITH_WARNINGS)
    add_compile_options(
	/W4
	/external:anglebrackets
	/external:W3
	/wd4251 # Suppress DLL-interface warning
	/wd4820 # Suppress padding warning
	/wd5045 # Suppress Spectre mitigation warning
    /wd4702 # Suppress the warning from fmt
)
else ()
    add_compile_options(/W0)
endif ()

function(enable_clang_tidy target_name)
    if (MSVC)

        set(clang_tidy_checks
            "-*"                                               # Disable all default checks
            "bugprone-*"                                       # Enable all bugprone checks
            "-bugprone-easily-swappable-parameters"            # Ignore: triggers easily on consecutive parameters of the same type
            "-bugprone-unchecked-string-to-number-conversion"  # Ignore: legacy string-to-number conversions
            "-bugprone-narrowing-conversions"                  # Ignore: implicit integer narrowing in legacy code
            "-clang-diagnostic-ignored-attributes"             # Ignore: warnings about compiler attributes being ignored
            "-bugprone-reserved-identifier"                    # Ignore: identifiers/macros using reserved names or leading underscores
            "-bugprone-switch-missing-default-case"            # Ignore: switches missing a default branch
            "-clang-diagnostic-unused-field"                   # Ignore: unused class or struct member variables
            "-clang-diagnostic-unused-private-field"           # Ignore: unused private class or struct member variables
            "-clang-diagnostic-unused-const-variable"          # Ignore: unused const variables
        )

        string(JOIN "," clang_tidy_checks_str ${clang_tidy_checks})

        set_target_properties(${target_name} PROPERTIES
            VS_GLOBAL_RunCodeAnalysis "true"
            VS_GLOBAL_EnableMicrosoftCodeAnalysis "false"
            VS_GLOBAL_EnableClangTidyCodeAnalysis "true"
            VS_GLOBAL_ClangTidyTreatWarningAsError "false"
            VS_GLOBAL_ClangTidyChecks "${clang_tidy_checks_str}"
        )
    endif()
endfunction()
