/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

class Flayer : public CreatureAIScript
{
public:

    explicit Flayer(Creature* pCreature) : CreatureAIScript(pCreature) { }
    static CreatureAIScript* Create(Creature* c) { return new Flayer(c); }

    void OnDied(Unit* mKiller)
    {
        if (!mKiller->isPlayer())
            return;

        Creature* creat = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(11064, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), true, false, 0, 0);
        if (creat)
            creat->Despawn(60000, 0);
    }

};

class Darrowshire_Spirit : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr)
    {
        plr->AddQuestKill(5211, 0, 0);

        Arcemu::Gossip::Menu::SendSimpleMenu(pObject->getGuid(), 3873, plr);

        Creature* Spirit = static_cast<Creature*>(pObject);

        Spirit->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        Spirit->Despawn(4000, 0);
    }
};

class ArajTheSummoner : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ArajTheSummoner);
    explicit ArajTheSummoner(Creature* pCreature) : CreatureAIScript(pCreature) { }

    void OnDied(Unit* mKiller)
    {
        if (!mKiller->isPlayer())
            return;

        GameObject* go = mKiller->GetMapMgr()->CreateAndSpawnGameObject(177241, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), 1);
        if (go != nullptr)
            go->Despawn(60 * 1000, 0);
    }
};

void SetupEasternPlaguelands(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* gs = new Darrowshire_Spirit();
    mgr->register_creature_gossip(11064, gs);

    mgr->register_creature_script(8532, &Flayer::Create);
    mgr->register_creature_script(8531, &Flayer::Create);
    mgr->register_creature_script(8530, &Flayer::Create);
    mgr->register_creature_script(1852, &ArajTheSummoner::Create);
}
