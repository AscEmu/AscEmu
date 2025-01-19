# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME g3dlite)

set(SRC_G3DLITE
    AABox.cpp
    Any.cpp
    AnyTableReader.cpp
    BinaryFormat.cpp
    BinaryInput.cpp
    BinaryOutput.cpp
    Box.cpp
    Capsule.cpp
    CollisionDetection.cpp
    CoordinateFrame.cpp
    Crypto.cpp
    Cylinder.cpp
    debugAssert.cpp
    FileSystem.cpp
    fileutils.cpp
    format.cpp
    g3dfnmatch.cpp
    g3dmath.cpp
    GThread.cpp
    Line.cpp
    LineSegment.cpp
    Log.cpp
    Matrix3.cpp
    Matrix4.cpp
    MemoryManager.cpp
    PhysicsFrame.cpp
    Plane.cpp
    prompt.cpp
    Quat.cpp
    Random.cpp
    Ray.cpp
    RegistryUtil.cpp
    Sphere.cpp
    stringutils.cpp
    System.cpp
    TextInput.cpp
    TextOutput.cpp
    Triangle.cpp
    uint128.cpp
    UprightFrame.cpp
    Vector2.cpp
    Vector3.cpp
    Vector4.cpp
)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} STATIC ${SRC_G3DLITE})

target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ZLIB_INCLUDE_DIRS}
)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
)

unset(SRC_G3DLITE)
