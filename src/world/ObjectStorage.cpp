/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

SERVER_DECL std::set<std::string> ExtraMapCreatureTables;
SERVER_DECL std::set<std::string> ExtraMapGameObjectTables;

void ObjectMgr::LoadProfessionDiscoveries()
{
    QueryResult* result = WorldDatabase.Query("SELECT * from professiondiscoveries");
    if (result != NULL)
    {
        do
        {
            Field* f = result->Fetch();
            ProfessionDiscovery* pf = new ProfessionDiscovery;
            pf->SpellId = f[0].GetUInt32();
            pf->SpellToDiscover = f[1].GetUInt32();
            pf->SkillValue = f[2].GetUInt32();
            pf->Chance = f[3].GetFloat();
            ProfessionDiscoveryTable.insert(pf);
        } while (result->NextRow());
        delete result;
    }
}

void ObjectMgr::LoadExtraCreatureProtoStuff()
{
    // Load creature_initiale_equip
    Log.Notice("ObjectStorage", "Loading creature_initial_equip...");
    {
        QueryResult* result = WorldDatabase.Query("SELECT creature_entry, itemslot_1, itemslot_2, itemslot_3 FROM creature_initial_equip;");

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 entry = fields[0].GetUInt32();
                CreatureProto const* creature_proto = sMySQLStore.GetCreatureProto(entry);
                if (creature_proto == nullptr)
                {
                    Log.Error("ObjectStorage", "Invalid creature_entry %u in table creature_initial_equip!", entry);
                    continue;
                }

                const_cast<CreatureProto*>(creature_proto)->itemslot_1 = fields[1].GetUInt32();
                const_cast<CreatureProto*>(creature_proto)->itemslot_2 = fields[2].GetUInt32();
                const_cast<CreatureProto*>(creature_proto)->itemslot_3 = fields[3].GetUInt32();

            } while (result->NextRow());

            delete result;
        }
    }

    // Load AI Agents
    if (Config.MainConfig.GetBoolDefault("Server", "LoadAIAgents", true))
    {
        QueryResult* result = WorldDatabase.Query("SELECT * FROM ai_agents");
        CreatureProto const* cn;

        if (result != NULL)
        {
            AI_Spell* sp;
            SpellEntry* spe;
            uint32 entry;

            do
            {
                Field* fields = result->Fetch();
                entry = fields[0].GetUInt32();
                cn = sMySQLStore.GetCreatureProto(entry);
                spe = dbcSpell.LookupEntryForced(fields[6].GetUInt32());
                if (spe == NULL)
                {
                    Log.Error("AIAgent", "For %u has nonexistent spell %u.", fields[0].GetUInt32(), fields[6].GetUInt32());
                    continue;
                }
                if (!cn)
                    continue;

                sp = new AI_Spell;
                sp->entryId = fields[0].GetUInt32();
                sp->instance_mode = fields[1].GetUInt32();
                sp->agent = fields[2].GetUInt16();
                sp->procChance = fields[4].GetUInt32();
                sp->procCount = fields[5].GetUInt32();
                sp->spell = spe;
                sp->spellType = static_cast<uint8>(fields[7].GetUInt32());

                int32  targettype = fields[8].GetInt32();
                if (targettype == -1)
                    sp->spelltargetType = static_cast<uint8>(GetAiTargetType(spe));
                else sp->spelltargetType = static_cast<uint8>(targettype);

                sp->cooldown = fields[9].GetInt32();
                sp->floatMisc1 = fields[10].GetFloat();
                sp->autocast_type = (uint32)-1;
                sp->cooldowntime = getMSTime();
                sp->procCounter = 0;
                sp->Misc2 = fields[11].GetUInt32();
                if (sp->agent == AGENT_SPELL)
                {
                    if (!sp->spell)
                    {
                        LOG_DEBUG("SpellId %u in ai_agent for %u is invalid.", (unsigned int)fields[6].GetUInt32(), (unsigned int)sp->entryId);
                        delete sp;
                        sp = NULL;
                        continue;
                    }

                    if (sp->spell->Effect[0] == SPELL_EFFECT_LEARN_SPELL || sp->spell->Effect[1] == SPELL_EFFECT_LEARN_SPELL ||
                        sp->spell->Effect[2] == SPELL_EFFECT_LEARN_SPELL)
                    {
                        LOG_DEBUG("Teaching spell %u in ai_agent for %u", (unsigned int)fields[6].GetUInt32(), (unsigned int)sp->entryId);
                        delete sp;
                        sp = NULL;
                        continue;
                    }

                    sp->minrange = GetMinRange(sSpellRangeStore.LookupEntry(sp->spell->rangeIndex));
                    sp->maxrange = GetMaxRange(sSpellRangeStore.LookupEntry(sp->spell->rangeIndex));

                    //omg the poor darling has no clue about making ai_agents
                    if (sp->cooldown == (uint32)-1)
                    {
                        //now this will not be exact cooldown but maybe a bigger one to not make him spam spells to often
                        int cooldown;
                        auto spell_duration = sSpellDurationStore.LookupEntry(sp->spell->DurationIndex);
                        int Dur = 0;
                        int Casttime = 0; //most of the time 0
                        int RecoveryTime = sp->spell->RecoveryTime;
                        if (sp->spell->DurationIndex)
                            Dur = ::GetDuration(spell_duration);
                        Casttime = GetCastTime(sSpellCastTimesStore.LookupEntry(sp->spell->CastingTimeIndex));
                        cooldown = Dur + Casttime + RecoveryTime;
                        if (cooldown < 0)
                            sp->cooldown = 2000; //huge value that should not loop while adding some timestamp to it
                        else sp->cooldown = cooldown;
                    }

                    /*
                    //now apply the moron filter
                    if (sp->procChance== 0)
                    {
                        //printf("SpellId %u in ai_agent for %u is invalid.\n", (unsigned int)fields[5].GetUInt32(), (unsigned int)sp->entryId);
                        delete sp;
                        sp = NULL;
                        continue;
                    }
                    if (sp->spellType== 0)
                    {
                        //right now only these 2 are used
                        if (IsBeneficSpell(sp->spell))
                            sp->spellType==STYPE_HEAL;
                        else sp->spellType==STYPE_BUFF;
                    }
                    if (sp->spelltargetType== 0)
                        sp->spelltargetType = RecommandAISpellTargetType(sp->spell);
                        */
                }

                if (sp->agent == AGENT_RANGED)
                {
                    const_cast<CreatureProto*>(cn)->m_canRangedAttack = true;
                    delete sp;
                    sp = NULL;
                }
                else if (sp->agent == AGENT_FLEE)
                {
                    const_cast<CreatureProto*>(cn)->m_canFlee = true;
                    if (sp->floatMisc1)
                        const_cast<CreatureProto*>(cn)->m_canFlee = (sp->floatMisc1 > 0.0f ? true : false);
                    else
                        const_cast<CreatureProto*>(cn)->m_fleeHealth = 0.2f;

                    if (sp->Misc2)
                        const_cast<CreatureProto*>(cn)->m_fleeDuration = sp->Misc2;
                    else
                        const_cast<CreatureProto*>(cn)->m_fleeDuration = 10000;

                    delete sp;
                    sp = NULL;
                }
                else if (sp->agent == AGENT_CALLFORHELP)
                {
                    const_cast<CreatureProto*>(cn)->m_canCallForHelp = true;
                    if (sp->floatMisc1)
                        const_cast<CreatureProto*>(cn)->m_callForHelpHealth = 0.2f;
                    delete sp;
                    sp = NULL;
                }
                else
                {
                    const_cast<CreatureProto*>(cn)->spells.push_back(sp);
                }

            } while (result->NextRow());

            delete result;
        }
    }
}

#define make_task(storage, itype, storagetype, tablename, format) tl.AddTask(new Task(\
    new CallbackP2< SQLStorage< itype, storagetype< itype > >, const char *, const char *> \
    (&storage, &SQLStorage< itype, storagetype< itype > >::Load, tablename, format)))


std::vector<std::pair<std::string, std::string> > additionalTables;

bool LoadAdditionalTable(const char* TableName, const char* SecondName, bool firstLoad = false)
{
    if (!stricmp(TableName, "creature_spawns"))
    {
        ExtraMapCreatureTables.insert(std::string(SecondName));
        return false;
    }
    else if (!stricmp(TableName, "gameobject_spawns"))
    {
        ExtraMapGameObjectTables.insert(std::string(SecondName));
        return false;
    }
    else
        return false;

    return true;
}

void Storage_LoadAdditionalTables()
{
    ExtraMapCreatureTables.insert(std::string("creature_spawns"));
    ExtraMapGameObjectTables.insert(std::string("gameobject_spawns"));

    std::string strData = Config.MainConfig.GetStringDefault("Startup", "LoadAdditionalTables", "");
    if (strData.empty())
        return;

    std::vector<std::string> strs = StrSplit(strData, ",");
    if (strs.empty())
        return;

    for (std::vector<std::string>::iterator itr = strs.begin(); itr != strs.end(); ++itr)
    {
        char s1[200];
        char s2[200];
        if (sscanf((*itr).c_str(), "%s %s", s1, s2) != 2)
            continue;

        if (LoadAdditionalTable(s2, s1, true))
        {
            std::pair<std::string, std::string> tmppair;
            tmppair.first = std::string(s1);
            tmppair.second = std::string(s2);
            additionalTables.push_back(tmppair);
        }
    }
}

void ObjectMgr::StoreBroadCastGroupKey()
{
    if (!sWorld.BCSystemEnable)
    {
        Log.Notice("ObjectMgr", "BCSystem Disabled.");
        return;
    }

    std::vector<std::string> keyGroup;
    QueryResult* result = WorldDatabase.Query("SELECT DISTINCT percent FROM `worldbroadcast` ORDER BY percent DESC");
    if (result != NULL)
    {
        do
        {
            Field* f = result->Fetch();
            keyGroup.push_back(std::string(f[0].GetString()));
        } while (result->NextRow());
        delete result;
        result = NULL;
    }

    if (keyGroup.empty())
    {
        Log.Notice("ObjectMgr", "BCSystem error! worldbroadcast empty? fill it first!");
        sWorld.BCSystemEnable = false;
        return;
    }
    else
    {
        Log.Notice("ObjectMgr", "BCSystem Enabled with %u KeyGroups.", keyGroup.size());
    }

    for (std::vector<std::string>::iterator itr = keyGroup.begin(); itr != keyGroup.end(); ++itr)
    {
        std::string curKey = (*itr);
        char szSQL[512];
        memset(szSQL, 0, sizeof(szSQL));
        sprintf(szSQL, "SELECT entry,percent FROM `worldbroadcast` WHERE percent='%s' ", curKey.c_str());
        result = WorldDatabase.Query(szSQL);
        if (result != NULL)
        {
            do
            {
                Field* f = result->Fetch();
                m_BCEntryStorage.insert(std::pair<uint32, uint32>(uint32(atoi(curKey.c_str())), f[0].GetUInt32()));
            } while (result->NextRow());
            delete result;
        }
    }
}
