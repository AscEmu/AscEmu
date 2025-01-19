/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

/* Script Comment:
 * Everything before escort event is made to work as blizzlike as possible!
 * This is still in development and is tagged[WIP].
 * It is suggested to apply SQL updates to see this script fully working!
 * Have fun ~Azolex
 */

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Instance_OldHillsbradFoothills.h"

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/WorldStates.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class OldHilsbradInstance : public InstanceScript
{
private:
    int32_t m_numBarrel;
    uint32_t m_phaseData[OHF_END];

public:
    explicit OldHilsbradInstance(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        m_numBarrel = 0;

        for (uint8_t i = 0; i < OHF_END; ++i)
            m_phaseData[i] = OHF_DATA_NOT_STARTED;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new OldHilsbradInstance(pMapMgr); }

    void OnPlayerEnter(Player* pPlayer)
    {
        if (pPlayer->getGender() == 0)
            pPlayer->castSpell(pPlayer, 35482, true);   // Human Male illusion
        else
            pPlayer->castSpell(pPlayer, 35483, true);   // Human Female illusion
    }

    void SetData(uint32_t pIndex, uint32_t pData)
    {
        if (pIndex >= OHF_END)
            return;

        if (pIndex == OHF_PHASE_2)
            mInstance->getWorldStatesHandler().SetWorldStateForZone(2367, 0, WORLDSTATE_OLD_HILLSBRAD_BARRELS, 0);

        m_phaseData[pIndex] = pData;
    }

    uint32_t GetData(uint32_t pIndex)
    {
        if (pIndex >= OHF_END)
            return 0;

        return m_phaseData[pIndex];
    }

    void OnGameObjectActivate(GameObject* pGameObject, Player* /*pPlayer*/)
    {
        if (pGameObject->getEntry() != GO_LODGE_ABLAZE || GetData(OHF_PHASE_1) == OHF_DATA_DONE)
            return;

        pGameObject->despawn(1000, 0);
        m_numBarrel++;
        pGameObject->getWorldMap()->getWorldStatesHandler().SetWorldStateForZone(2367, 0, WORLDSTATE_OLD_HILLSBRAD_BARRELS, m_numBarrel);
        if (m_numBarrel != 5)
            return;

        SetData(OHF_PHASE_1, OHF_DATA_DONE);

        for (auto& itr : mInstance->getPlayers())
        {
            if (auto* player = itr.second)
            {
                if (auto* questLog = player->getQuestLogByQuestId(10283))
                {
                    questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
                    questLog->sendUpdateAddKill(0);
                    questLog->updatePlayerFields();
                }
            }
        }

        for (uint8_t i = 0; i < 21; ++i)
        {
            GameObject* pGO = spawnGameObject(GO_FIRE, Fires[i].x, Fires[i].y, Fires[i].z, Fires[i].o);
            if (pGO != nullptr)
                pGO->despawn(10 * 60 * 1000, 0);
        }

        spawnCreature(CN_LIEUTENANT_DRAKE, 2118.310303f, 89.565969f, 52.453037f, 2.027089f);
    }
};

void SetupOldHillsbradFoothills(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_OLD_HILSBRAD, &OldHilsbradInstance::Create);
}
