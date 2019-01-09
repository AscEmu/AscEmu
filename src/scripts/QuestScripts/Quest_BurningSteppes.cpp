/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2009 WhyScripts Team <http://www.whydb.org/>
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

#include "Setup.h"

class TabletOfTheSeven : public QuestScript
{
public:

    void OnGameObjectActivate(uint32 entry, Player* mTarget, QuestLogEntry* qLogEntry)
    {
        if (mTarget == nullptr || qLogEntry == nullptr || entry != 169294)
            return;

        if (mTarget->getItemInterface()->GetItemCount(11470) < qLogEntry->GetQuest()->required_itemcount[0])
            mTarget->castSpell(mTarget, 15065, false);
    }
};

void SetupBurningSteppes(ScriptMgr* mgr)
{
    mgr->register_quest_script(4296, new TabletOfTheSeven);
}
