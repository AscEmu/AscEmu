/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef WOWSERVER_QUEST_LOG_ENTRY_HPP
#define WOWSERVER_QUEST_LOG_ENTRY_HPP

#include <Database/Field.h>
#include "CommonDefines.hpp"
#include "Server/EventableObject.h"
#include "Management/Quest.h"
#include "Units/Players/Player.h"
#include "Server/Script/ScriptMgr.h"

class SERVER_DECL QuestLogEntry : public EventableObject
{
	friend class QuestMgr;

	public:

		QuestLogEntry();
		~QuestLogEntry();

		inline QuestProperties const* GetQuest() { return m_quest; };
		void Init(QuestProperties const* quest, Player* plr, uint16 slot);

		bool CanBeFinished();
		void Complete();
		void SaveToDB(QueryBuffer* buf);
		bool LoadFromDB(Field* fields);
		void UpdatePlayerFields();

		void SetTrigger(uint32 i);
		void SetMobCount(uint32 i, uint32 count);
		void IncrementMobCount(uint32 i);

		bool IsUnitAffected(Unit* target);
		inline bool IsCastQuest() { return iscastquest; }
		inline bool IsEmoteQuest() { return isemotequest; }
		void AddAffectedUnit(Unit* target);
		void ClearAffectedUnits();

		void SetSlot(uint16 i);
		void Finish();


        //////////////////////////////////////////////////////////////////////////////////////////
		/// void Fail(bool timerexpired)
		/// Marks the quest as failed
        ///
        /// \param bool timerexpired  -  true if the reason for failure is timer expiration.
        ///
        /// \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
		void Fail(bool timerexpired);


        //////////////////////////////////////////////////////////////////////////////////////////
		/// bool HasFailed()
		/// Tells if the Quest has failed.
		///
		/// \param none
		///
		/// \return true if the quest has failed, false otherwise.
		///
        //////////////////////////////////////////////////////////////////////////////////////////
        bool HasFailed();


        void SendQuestComplete();
		void SendUpdateAddKill(uint32 i);
		inline uint32 GetMobCount(uint32 i) { return m_mobcount[i]; }
		inline uint32 GetExploredAreas(uint32 i) { return m_explored_areas[i]; }

		uint16 GetBaseField(uint16 slot)
		{
			return PLAYER_QUEST_LOG_1_1 + (slot * 5);
		}
		uint16 GetSlot() { return m_slot; }

	private:

		uint32 completed;

		bool mInitialized;
		bool mDirty;

        QuestProperties const* m_quest;
		Player* m_plr;

		uint32 m_mobcount[4];
		uint32 m_explored_areas[4];

		std::set<uint64> m_affected_units;
		bool iscastquest;
		bool isemotequest;

		uint32 expirytime;
		uint16 m_slot;
};
#define CALL_QUESTSCRIPT_EVENT(obj, func) if (static_cast<QuestLogEntry*>(obj)->GetQuest()->pQuestScript != NULL) static_cast<QuestLogEntry*>(obj)->GetQuest()->pQuestScript->func

#endif // WOWSERVER_QUEST_HPP
