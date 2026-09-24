/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#if defined(AE_FOREVER)
#include "version/Forever/Fields/ForeverUpdateFields.hpp"
namespace AscEmu::ModernFields = AscEmu::Version::Forever::Fields;
#endif
