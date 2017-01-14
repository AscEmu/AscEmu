/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
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
 */

#ifndef _DBC_STORES_H
#define _DBC_STORES_H

#include "DBCGlobals.hpp"
#include "Server/Definitions.h"

class Player;

#pragma pack(push,1)

struct Trainerspell
{
    uint32 Id;
    uint32 skilline1;
    uint32 skilline2;
    uint32 skilline3;
    uint32 maxlvl;
    uint32 charclass;
};

#pragma pack(pop)

inline float GetRadius(DBC::Structures::SpellRadiusEntry const* radius)
{
    if (radius == nullptr)
        return 0;

    return radius->radius_min;
}
inline uint32 GetCastTime(DBC::Structures::SpellCastTimesEntry const* time)
{
    if (time == nullptr)
        return 0;

    return time->CastTime;
}
inline float GetMaxRange(DBC::Structures::SpellRangeEntry const* range)
{
    if (range == nullptr)
        return 0;

    return range->maxRange;
}
inline float GetMinRange(DBC::Structures::SpellRangeEntry const* range)
{
    if (range == nullptr)
        return 0;

    return range->minRange;
}
inline uint32 GetDuration(DBC::Structures::SpellDurationEntry const* dur)
{
    if (dur == nullptr)
        return 0;
    return dur->Duration1;
}

#define SAFE_DBC_CODE_RETURNS       /// undefine this to make out of range/nulls return null. */

template<class T>
class SERVER_DECL DBCStorage
{
        T* m_heapBlock;
        T* m_firstEntry;

        T** m_entries;
        uint32 m_max;
        uint32 m_numrows;
        uint32 m_stringlength;
        char* m_stringData;

        uint32 rows;
        uint32 cols;
        uint32 useless_shit;
        uint32 header;

    public:

        class iterator
        {
            private:
                T* p;
            public:
                iterator(T* ip = 0) : p(ip) { }
                iterator & operator++() { ++p; return *this; }
                iterator & operator--() { --p; return *this; }
                bool operator!=(const iterator & i) { return (p != i.p); }
                T* operator*() { return p; }
        };

        iterator begin()
        {
            return iterator(&m_heapBlock[0]);
        }
        iterator end()
        {
            return iterator(&m_heapBlock[m_numrows]);
        }

        DBCStorage()
        {
            m_heapBlock = NULL;
            m_entries = NULL;
            m_firstEntry = NULL;
            m_max = 0;
            m_numrows = 0;
            m_stringlength = 0;
            m_stringData = NULL;
            rows = 0;
            cols = 0;
            useless_shit = 0;
            header = 0;
        }

        ~DBCStorage()
        {
            Cleanup();
        }

        void Cleanup()
        {
            if (m_heapBlock)
            {
                free(m_heapBlock);
                m_heapBlock = NULL;
            }
            if (m_entries)
            {
                free(m_entries);
                m_entries = NULL;
            }
            if (m_stringData != NULL)
            {
                free(m_stringData);
                m_stringData = NULL;
            }
        }

        bool Load(const char* filename, const char* format, bool load_indexed, bool load_strings)
        {
            uint32 i;
            uint32 string_length;
            int pos;

            FILE* f = fopen(filename, "rb");
            if (f == NULL)
                return false;

            // read the number of rows, and allocate our block on the heap
            if (fread(&header, 4, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            if (fread(&rows, 4, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            if (fread(&cols, 4, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            if (fread(&useless_shit, 4, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            if (fread(&string_length, 4, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            pos = ftell(f);

            if (load_strings)
            {
                if (fseek(f, 20 + (rows * cols * 4), SEEK_SET) != 0)
                {
                    fclose(f);
                    return false;
                }

                m_stringData = (char*)malloc(string_length);
                m_stringlength = string_length;
                if (m_stringData)
                {
                    if (fread(m_stringData, string_length, 1, f) != 1)
                    {
                        fclose(f);
                        return false;
                    }
                }
            }

            if (fseek(f, pos, SEEK_SET) != 0)
            {
                fclose(f);
                return false;
            }

            m_heapBlock = (T*)malloc(rows * sizeof(T));
            ASSERT(m_heapBlock);

            // read the data for each row
            for (i = 0; i < rows; ++i)
            {
                memset(&m_heapBlock[i], 0, sizeof(T));
                ReadEntry(f, &m_heapBlock[i], format, cols, filename);

                if (load_indexed)
                {
                    // all the time the first field in the dbc is our unique entry
                    if (*(uint32*)&m_heapBlock[i] > m_max)
                        m_max = *(uint32*)&m_heapBlock[i];
                }
            }

            if (load_indexed)
            {
                m_entries = (T**)malloc(sizeof(T*) * (m_max + 1));
                ASSERT(m_entries);

                memset(m_entries, 0, (sizeof(T*) * (m_max + 1)));
                for (i = 0; i < rows; ++i)
                {
                    if (m_firstEntry == NULL)
                        m_firstEntry = &m_heapBlock[i];

                    m_entries[*(uint32*)&m_heapBlock[i]] = &m_heapBlock[i];
                }
            }

            m_numrows = rows;

            fclose(f);
            return true;
        }

        void ReadEntry(FILE* f, T* dest, const char* format, uint32 cols, const char* filename)
        {
            const char* t = format;
            uint32* dest_ptr = (uint32*)dest;
            uint32 c = 0;
            uint32 val;
            size_t len = strlen(format);
            if (len != cols)
                Log.outError("!!! possible invalid format in file %s (us: %u, them: %u)", filename, len, cols);

            while (*t != 0)
            {
                if ((++c) > cols)
                {
                    ++t;
                    Log.outError("!!! Read buffer overflow in DBC reading of file %s", filename);
                    continue;
                }

                if (fread(&val, 4, 1, f) != 1)
                {
                    ++t;
                    continue;
                }

                if (*t == 'x')
                {
                    ++t;
                    continue;        // skip!
                }

                if ((*t == 's') || (*t == 'l'))
                {
                    char** new_ptr = (char**)dest_ptr;
                    static const char* null_str = "";
                    char* ptr;
                    // if t == 'lxxxxxxxxxxxxxxxx' use localized strings in case
                    // the english one is empty. *t ends at most on the locale flag
                    for (int count = (*t == 'l') ? 16 : 0 ;
                            val == 0 && count > 0 && *(t + 1) == 'x'; t++, count--)
                    {
                        fread(&val, 4, 1, f);

                    }
                    if (val < m_stringlength)
                        ptr = m_stringData + val;
                    else
                        ptr = (char*)null_str;

                    *new_ptr = ptr;
                    new_ptr++;
                    dest_ptr = (uint32*)new_ptr;
                }
                else
                {
                    *dest_ptr = val;
                    dest_ptr++;
                }

                ++t;
            }
        }

        inline uint32 GetNumRows()
        {
            return m_numrows;
        }

        T* LookupEntryForced(uint32 i)
        {
#if 0
            if (m_entries)
            {
                if (i > m_max || m_entries[i] == NULL)
                {
                    printf("LookupEntryForced failed for entry %u\n", i);
                    return NULL;
                }
                else
                    return m_entries[i];
            }
            else
            {
                if (i >= m_numrows)
                    return NULL;
                else
                    return &m_heapBlock[i];
            }
#else
            if (m_entries)
            {
                if (i > m_max || m_entries[i] == NULL)
                    return NULL;
                else
                    return m_entries[i];
            }
            else
            {
                if (i >= m_numrows)
                    return NULL;
                else
                    return &m_heapBlock[i];
            }
#endif
        }

        T* LookupRowForced(uint32 i)
        {
            if (i >= m_numrows)
                return NULL;
            else
                return &m_heapBlock[i];
        }

        T* CreateCopy(T* obj)
        {
            T* oCopy = (T*)malloc(sizeof(T));
            ASSERT(oCopy);
            memcpy(oCopy, obj, sizeof(T));
            return oCopy;
        }
        void SetRow(uint32 i, T* t)
        {
            if (i < m_max && m_entries)
                m_entries[i] = t;
        }

        T* LookupEntry(uint32 i)
        {
            if (m_entries)
            {
                if (i > m_max || m_entries[i] == NULL)
                    return m_firstEntry;
                else
                    return m_entries[i];
            }
            else
            {
                if (i >= m_numrows)
                    return &m_heapBlock[0];
                else
                    return &m_heapBlock[i];
            }
        }

        T* LookupRow(uint32 i)
        {
            if (i >= m_numrows)
                return &m_heapBlock[0];
            else
                return &m_heapBlock[i];
        }
};

extern SERVER_DECL DBC::DBCStorage<DBC::Structures::WorldMapOverlayEntry> sWorldMapOverlayStore;

#ifdef ENABLE_ACHIEVEMENTS
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementEntry> sAchievementStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementCriteriaEntry> sAchievementCriteriaStore;
#endif

extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CharTitlesEntry> sCharTitlesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CurrencyTypesEntry> sCurrencyTypesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::BarberShopStyleEntry> sBarberShopStyleStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GemPropertiesEntry> sGemPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphSlotEntry> sGlyphSlotStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemEntry> sItemStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemSetEntry> sItemSetStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LockEntry> sLockStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEntry_New> sSpellStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDifficultyEntry> sSpellDifficultyStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDurationEntry> sSpellDurationStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRangeEntry> sSpellRangeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellShapeshiftFormEntry> sSpellShapeshiftFormStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesTextEntry> sEmotesTextStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRadiusEntry> sSpellRadiusStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCastTimesEntry> sSpellCastTimesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaGroupEntry> sAreaGroupStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTableEntry> sAreaStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionTemplateEntry> sFactionTemplateStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionEntry> sFactionStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellItemEnchantmentEntry> sSpellItemEnchantmentStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineAbilityEntry> sSkillLineAbilityStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineEntry> sSkillLineStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiNodesEntry> sTaxiNodesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathEntry> sTaxiPathStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AuctionHouseEntry> sAuctionHouseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentEntry> sTalentStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentTabEntry> sTalentTabStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureSpellDataEntry> sCreatureSpellDataStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureFamilyEntry> sCreatureFamilyStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrClassesEntry> sChrClassesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrRacesEntry> sChrRacesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MapEntry> sMapStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::HolidaysEntry> sHolidaysStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRuneCostEntry> sSpellRuneCostStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemExtendedCostEntry> sItemExtendedCostStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomSuffixEntry> sItemRandomSuffixStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtCombatRatingsEntry> sGtCombatRatingsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChatChannelsEntry> sChatChannelsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityCostsEntry> sDurabilityCostsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityQualityEntry> sDurabilityQualityStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::BankBagSlotPrices> sBankBagSlotPricesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::StableSlotPrices> sStableSlotPricesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritEntry> sGtChanceToMeleeCritStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritEntry> sGtChanceToSpellCritStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenMPEntry> sGtOCTRegenMPStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenMPPerSptEntry> sGtRegenMPPerSptStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTriggerEntry> sAreaTriggerStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatValuesEntry> sScalingStatValuesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::QuestXP> sQuestXPStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MailTemplateEntry> sMailTemplateStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::WMOAreaTableEntry> sWMOAreaTableStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SummonPropertiesEntry> sSummonPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::NameGenEntry> sNameGenStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LFGDungeonEntry> sLFGDungeonStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LiquidTypeEntry> sLiquidTypeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleEntry> sVehicleStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleSeatEntry> sVehicleSeatStore;

bool LoadDBCs();

DBC::Structures::WMOAreaTableEntry const* GetWMOAreaTableEntryByTriple(int32 root_id, int32 adt_id, int32 group_id);

#endif // _DBC_STORES_H
