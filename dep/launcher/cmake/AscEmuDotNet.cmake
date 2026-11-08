find_program(DOTNET_EXECUTABLE NAMES dotnet)
if(NOT DOTNET_EXECUTABLE)
    message(FATAL_ERROR ".NET SDK was not found. Install the .NET 8 SDK and re-run CMake. Expected command: dotnet --info")
endif()

execute_process(
    COMMAND "${DOTNET_EXECUTABLE}" --version
    OUTPUT_VARIABLE DOTNET_SDK_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE DOTNET_VERSION_RESULT)

if(NOT DOTNET_VERSION_RESULT EQUAL 0)
    message(FATAL_ERROR "dotnet was found, but 'dotnet --version' failed.")
endif()

if(DOTNET_SDK_VERSION VERSION_LESS "8.0")
    message(FATAL_ERROR "AscEmu Launcher requires .NET SDK 8.0 or newer. Found ${DOTNET_SDK_VERSION}.")
endif()

message(STATUS "AscEmu Launcher .NET SDK: ${DOTNET_SDK_VERSION}")

function(ascemu_add_dotnet_launcher TARGET PROJECT_FILE)
    get_filename_component(PROJECT_DIR "${PROJECT_FILE}" DIRECTORY)
    set(BUILD_DIR "${CMAKE_BINARY_DIR}/bin/${TARGET}")
    set(PUBLISH_DIR "${CMAKE_BINARY_DIR}/publish/${TARGET}")

    add_custom_target(${TARGET} ALL
        COMMAND "${DOTNET_EXECUTABLE}" build "${PROJECT_FILE}"
                --configuration "$<IF:$<CONFIG:Debug>,Debug,Release>"
                --output "${BUILD_DIR}/$<IF:$<CONFIG:Debug>,Debug,Release>"
                --nologo
        WORKING_DIRECTORY "${PROJECT_DIR}"
        COMMENT "Building AscEmu Launcher"
        VERBATIM)

    set(PUBLISH_SELF_CONTAINED false)
    if(ASCEMU_LAUNCHER_SELF_CONTAINED)
        set(PUBLISH_SELF_CONTAINED true)
    endif()

    set(PUBLISH_SINGLE_FILE false)
    if(ASCEMU_LAUNCHER_SINGLE_FILE)
        set(PUBLISH_SINGLE_FILE true)
    endif()

    add_custom_target(${TARGET}_publish
        COMMAND "${DOTNET_EXECUTABLE}" publish "${PROJECT_FILE}"
                --configuration Release
                --runtime "${ASCEMU_LAUNCHER_RUNTIME}"
                --self-contained "${PUBLISH_SELF_CONTAINED}"
                --output "${PUBLISH_DIR}"
                -p:PublishSingleFile=${PUBLISH_SINGLE_FILE}
                -p:IncludeNativeLibrariesForSelfExtract=true
                -p:DebugType=None
                -p:DebugSymbols=false
                --nologo
        WORKING_DIRECTORY "${PROJECT_DIR}"
        COMMENT "Publishing AscEmu Launcher to ${PUBLISH_DIR}"
        VERBATIM)
endfunction()
