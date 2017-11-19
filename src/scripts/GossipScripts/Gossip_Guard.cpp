/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/Gossip/Gossip.h"
#include "Objects/ObjectMgr.h"


class StormwindGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 114;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class DarnassusGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 122;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class UndercityGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 142;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class UndercityGuardOverseer : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 163;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class ThunderbluffGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 152;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class GoldshireGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 132;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class TeldrassilGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 172;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class SilvermoonGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 180;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class ExodarGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 191;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class OrgrimmarGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 724;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class BloodhoofGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 751;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class RazorHillGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 989;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class BrillGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1003;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class IronforgeGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1012;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class KharanosGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1035;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class FalconwingGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1047;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class AzureWatchGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1058;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class ShattrathGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1068;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

class DalaranGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1095;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
    }
};

void SetupGuardGossip(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* goldshireGuard = new GoldshireGuard();
    mgr->register_creature_gossip(1423, goldshireGuard);

    Arcemu::Gossip::Script* stormwindGuard = new StormwindGuard();
    mgr->register_creature_gossip(68, stormwindGuard);
    mgr->register_creature_gossip(1976, stormwindGuard);
    mgr->register_creature_gossip(29712, stormwindGuard);

    Arcemu::Gossip::Script* darnassusGuard = new DarnassusGuard();
    mgr->register_creature_gossip(4262, darnassusGuard);

    Arcemu::Gossip::Script* undercityGuard = new UndercityGuard();
    mgr->register_creature_gossip(5624, undercityGuard);

    Arcemu::Gossip::Script* undercityGuardOverseer = new UndercityGuardOverseer();
    mgr->register_creature_gossip(36213, undercityGuardOverseer);

    Arcemu::Gossip::Script* teldrassilGuard = new TeldrassilGuard();
    mgr->register_creature_gossip(3571, teldrassilGuard);

    Arcemu::Gossip::Script* silvermoonGuard = new SilvermoonGuard();
    mgr->register_creature_gossip(16222, silvermoonGuard);

    Arcemu::Gossip::Script* exodarGuard = new ExodarGuard();
    mgr->register_creature_gossip(16733, exodarGuard);
    mgr->register_creature_gossip(20674, exodarGuard);

    Arcemu::Gossip::Script* orgrimmarGuard = new OrgrimmarGuard();
    mgr->register_creature_gossip(3296, orgrimmarGuard);

    Arcemu::Gossip::Script* thunderbluffGuard = new ThunderbluffGuard();
    mgr->register_creature_gossip(3084, thunderbluffGuard);

    Arcemu::Gossip::Script* bloodhoofGuard = new BloodhoofGuard();
    mgr->register_creature_gossip(3222, bloodhoofGuard);
    mgr->register_creature_gossip(3224, bloodhoofGuard);
    mgr->register_creature_gossip(3220, bloodhoofGuard);
    mgr->register_creature_gossip(3219, bloodhoofGuard);
    mgr->register_creature_gossip(3217, bloodhoofGuard);
    mgr->register_creature_gossip(3215, bloodhoofGuard);
    mgr->register_creature_gossip(3218, bloodhoofGuard);
    mgr->register_creature_gossip(3221, bloodhoofGuard);
    mgr->register_creature_gossip(3223, bloodhoofGuard);
    mgr->register_creature_gossip(3212, bloodhoofGuard);

    Arcemu::Gossip::Script* razorHillGuard = new RazorHillGuard();
    mgr->register_creature_gossip(5953, razorHillGuard);

    Arcemu::Gossip::Script* brillGuard = new BrillGuard();
    mgr->register_creature_gossip(5725, brillGuard);
    mgr->register_creature_gossip(1738, brillGuard);
    mgr->register_creature_gossip(1652, brillGuard);
    mgr->register_creature_gossip(1746, brillGuard);
    mgr->register_creature_gossip(1745, brillGuard);
    mgr->register_creature_gossip(1743, brillGuard);
    mgr->register_creature_gossip(1744, brillGuard);
    mgr->register_creature_gossip(1496, brillGuard);
    mgr->register_creature_gossip(1742, brillGuard);

    Arcemu::Gossip::Script* ironforgeGuard = new IronforgeGuard();
    mgr->register_creature_gossip(5595, ironforgeGuard);

    Arcemu::Gossip::Script* kharanosGuard = new KharanosGuard();
    mgr->register_creature_gossip(727, kharanosGuard);

    Arcemu::Gossip::Script* falconwingGuard = new FalconwingGuard();
    mgr->register_creature_gossip(16221, falconwingGuard);

    Arcemu::Gossip::Script* azureWatchGuard = new AzureWatchGuard();
    mgr->register_creature_gossip(18038, azureWatchGuard);

    Arcemu::Gossip::Script* shattrathGuard = new ShattrathGuard();
    mgr->register_creature_gossip(19687, shattrathGuard);
    mgr->register_creature_gossip(18568, shattrathGuard);
    mgr->register_creature_gossip(18549, shattrathGuard);

    Arcemu::Gossip::Script* dalaranGuard = new DalaranGuard();
    mgr->register_creature_gossip(32675, dalaranGuard);
    mgr->register_creature_gossip(32676, dalaranGuard);
    mgr->register_creature_gossip(32677, dalaranGuard);
    mgr->register_creature_gossip(32678, dalaranGuard);
    mgr->register_creature_gossip(32679, dalaranGuard);
    mgr->register_creature_gossip(32680, dalaranGuard);
    mgr->register_creature_gossip(32681, dalaranGuard);
    mgr->register_creature_gossip(32683, dalaranGuard);
    mgr->register_creature_gossip(32684, dalaranGuard);
    mgr->register_creature_gossip(32685, dalaranGuard);
    mgr->register_creature_gossip(32686, dalaranGuard);
    mgr->register_creature_gossip(32687, dalaranGuard);
    mgr->register_creature_gossip(32688, dalaranGuard);
    mgr->register_creature_gossip(32689, dalaranGuard);
    mgr->register_creature_gossip(32690, dalaranGuard);
    mgr->register_creature_gossip(32691, dalaranGuard);
    mgr->register_creature_gossip(32692, dalaranGuard);
    mgr->register_creature_gossip(32693, dalaranGuard);
}
