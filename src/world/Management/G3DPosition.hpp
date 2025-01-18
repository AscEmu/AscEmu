/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LocationVector.h"
#include <G3D/Vector3.h>

inline G3D::Vector3 positionToVector3(LocationVector p) { return { p.x, p.y, p.z }; }
inline G3D::Vector3 positionToVector3(LocationVector const* p) { return { p->x, p->y, p->z }; }
inline LocationVector vector3ToPosition(G3D::Vector3 v) { return { v.x, v.y, v.z }; }