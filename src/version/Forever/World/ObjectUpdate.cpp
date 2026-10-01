/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ObjectUpdate.hpp"

// The Forever object-update implementation is intentionally split by wire
// responsibility under World/ObjectUpdate/. Keep this translation unit as the
// public module anchor; serializers and packet builders live in the focused
// implementation files next to it.
