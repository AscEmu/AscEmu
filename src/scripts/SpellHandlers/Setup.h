/****************************************************************************
 *
 * SpellHandler Plugin
 * Copyright (c) 2007 Team Ascent
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to Creative Commons,
 * 543 Howard Street, 5th Floor, San Francisco, California, 94105, USA.
 *
 * EXCEPT TO THE EXTENT REQUIRED BY APPLICABLE LAW, IN NO EVENT WILL LICENSOR BE LIABLE TO YOU
 * ON ANY LEGAL THEORY FOR ANY SPECIAL, INCIDENTAL, CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES
 * ARISING OUT OF THIS LICENSE OR THE USE OF THE WORK, EVEN IF LICENSOR HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 *
 */

#pragma once

#include "Server/Script/ScriptMgr.h"
#include "WorldConf.h"

class ScriptMgr;

// Class spell handlers
#if VERSION_STRING >= WotLK
void setupDeathKnightSpells(ScriptMgr* mgr);
#endif
void setupDruidSpells(ScriptMgr* mgr);
void setupHunterSpells(ScriptMgr* mgr);
void setupMageSpells(ScriptMgr* mgr);
#if VERSION_STRING >= Mop
void setupMonkSpells(ScriptMgr* mgr);
#endif
void setupPaladinSpells(ScriptMgr* mgr);
void setupPriestSpells(ScriptMgr* mgr);
void setupRogueSpells(ScriptMgr* mgr);
void setupShamanSpells(ScriptMgr* mgr);
void setupWarlockSpells(ScriptMgr* mgr);
void setupWarriorSpells(ScriptMgr* mgr);

// Other spell handlers
void setupItemSpells(ScriptMgr* mgr);
void setupMiscSpells(ScriptMgr* mgr);
void setupPetSpells(ScriptMgr* mgr);
void setupQuestSpells(ScriptMgr* mgr);

// Legacy spell handlers
void SetupLegacyShamanSpells(ScriptMgr* mgr);
void SetupLegacyWarlockSpells(ScriptMgr* mgr);
void SetupLegacyWarriorSpells(ScriptMgr* mgr);
void SetupLegacyHunterSpells(ScriptMgr* mgr);
void SetupLegacyMageSpells(ScriptMgr* mgr);
void SetupLegacyPaladinSpells(ScriptMgr* mgr);
void SetupLegacyRogueSpells(ScriptMgr* mgr);
void SetupLegacyPriestSpells(ScriptMgr* mgr);
void SetupLegacyDruidSpells(ScriptMgr* mgr);
void SetupLegacyDeathKnightSpells(ScriptMgr* mgr);
void SetupLegacyPetAISpells(ScriptMgr* mgr);
void SetupLegacyQuestItems(ScriptMgr* mgr);
void SetupLegacyItemSpells_1(ScriptMgr* mgr);
void SetupLegacyMiscSpellhandlers(ScriptMgr* mgr);
