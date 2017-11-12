/*
 * Moon++ Scripts for Ascent MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BASE_H
#define _BASE_H

#include "Units/Creatures/AIInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include <Management/QuestLogEntry.hpp>
#include "Map/MapScriptInterface.h"
#include <Spell/Customization/SpellCustomizations.hpp>
#include "Map/WorldCreatorDefines.hpp"

#define MOONSCRIPT_FACTORY_FUNCTION(ClassName, ParentClassName)\
public:\
    ADD_CREATURE_FACTORY_FUNCTION(ClassName);\
    typedef ParentClassName ParentClass;


//////////////////////////////////////////////////////////////////////////////////////////
//Class MoonScriptCreatureAI
class MoonScriptCreatureAI : public CreatureAIScript
{
    public:
        MoonScriptCreatureAI(Creature* pCreature);
        virtual ~MoonScriptCreatureAI();
};

#endif // _BASE_H
