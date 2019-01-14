/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class DarnassusGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 122;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class UndercityGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 142;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class UndercityGuardOverseer : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 163;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class ThunderbluffGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 152;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class GoldshireGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 132;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class TeldrassilGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 172;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class SilvermoonGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 180;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class ExodarGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 191;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class OrgrimmarGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 724;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class BloodhoofGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 751;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class RazorHillGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 989;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class BrillGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1003;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class IronforgeGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1012;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class KharanosGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1035;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class FalconwingGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1047;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class AzureWatchGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1058;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class ShattrathGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1068;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

class DalaranGuard : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1095;
    void OnHello(Object* object, Player* player) override
    {
        objmgr.createGuardGossipMenuForPlayer(object->getGuid(), definedGossipMenu, player);
    }

    void OnSelectOption(Object* object, Player* player, uint32 IntId, const char* /*Code*/, uint32 gossipId) override
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->getGuid(), player, IntId, definedGossipMenu);
        }
    }
};

void SetupGuardGossip(ScriptMgr* mgr)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // GoldshireGuard
    mgr->register_creature_gossip(1423, new GoldshireGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // StormwindGuard
    mgr->register_creature_gossip(68, new StormwindGuard());
    mgr->register_creature_gossip(1976, new StormwindGuard());
    mgr->register_creature_gossip(29712, new StormwindGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // DarnassusGuard
    mgr->register_creature_gossip(4262, new DarnassusGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // UndercityGuard
    mgr->register_creature_gossip(5624, new UndercityGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // UndercityGuardOverseer
    mgr->register_creature_gossip(36213, new UndercityGuardOverseer());

    //////////////////////////////////////////////////////////////////////////////////////////
    // TeldrassilGuard
    mgr->register_creature_gossip(3571, new TeldrassilGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // SilvermoonGuard
    mgr->register_creature_gossip(16222, new SilvermoonGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // ExodarGuard
    mgr->register_creature_gossip(16733, new ExodarGuard());
    mgr->register_creature_gossip(20674, new ExodarGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // OrgrimmarGuard
    mgr->register_creature_gossip(3296, new OrgrimmarGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // ThunderbluffGuard
    mgr->register_creature_gossip(3084, new ThunderbluffGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // BloodhoofGuard
    mgr->register_creature_gossip(3222, new BloodhoofGuard());
    mgr->register_creature_gossip(3224, new BloodhoofGuard());
    mgr->register_creature_gossip(3220, new BloodhoofGuard());
    mgr->register_creature_gossip(3219, new BloodhoofGuard());
    mgr->register_creature_gossip(3217, new BloodhoofGuard());
    mgr->register_creature_gossip(3215, new BloodhoofGuard());
    mgr->register_creature_gossip(3218, new BloodhoofGuard());
    mgr->register_creature_gossip(3221, new BloodhoofGuard());
    mgr->register_creature_gossip(3223, new BloodhoofGuard());
    mgr->register_creature_gossip(3212, new BloodhoofGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // RazorHillGuard
    mgr->register_creature_gossip(5953, new RazorHillGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // BrillGuard
    mgr->register_creature_gossip(5725, new BrillGuard());
    mgr->register_creature_gossip(1738, new BrillGuard());
    mgr->register_creature_gossip(1652, new BrillGuard());
    mgr->register_creature_gossip(1746, new BrillGuard());
    mgr->register_creature_gossip(1745, new BrillGuard());
    mgr->register_creature_gossip(1743, new BrillGuard());
    mgr->register_creature_gossip(1744, new BrillGuard());
    mgr->register_creature_gossip(1496, new BrillGuard());
    mgr->register_creature_gossip(1742, new BrillGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // IronforgeGuard
    mgr->register_creature_gossip(5595, new IronforgeGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // KharanosGuard
    mgr->register_creature_gossip(727, new KharanosGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // FalconwingGuard
    mgr->register_creature_gossip(16221, new FalconwingGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // AzureWatchGuard
    mgr->register_creature_gossip(18038, new AzureWatchGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // ShattrathGuard
    mgr->register_creature_gossip(19687, new ShattrathGuard());
    mgr->register_creature_gossip(18568, new ShattrathGuard());
    mgr->register_creature_gossip(18549, new ShattrathGuard());

    //////////////////////////////////////////////////////////////////////////////////////////
    // DalaranGuard
    mgr->register_creature_gossip(32675, new DalaranGuard());
    mgr->register_creature_gossip(32676, new DalaranGuard());
    mgr->register_creature_gossip(32677, new DalaranGuard());
    mgr->register_creature_gossip(32678, new DalaranGuard());
    mgr->register_creature_gossip(32679, new DalaranGuard());
    mgr->register_creature_gossip(32680, new DalaranGuard());
    mgr->register_creature_gossip(32681, new DalaranGuard());
    mgr->register_creature_gossip(32683, new DalaranGuard());
    mgr->register_creature_gossip(32684, new DalaranGuard());
    mgr->register_creature_gossip(32685, new DalaranGuard());
    mgr->register_creature_gossip(32686, new DalaranGuard());
    mgr->register_creature_gossip(32687, new DalaranGuard());
    mgr->register_creature_gossip(32688, new DalaranGuard());
    mgr->register_creature_gossip(32689, new DalaranGuard());
    mgr->register_creature_gossip(32690, new DalaranGuard());
    mgr->register_creature_gossip(32691, new DalaranGuard());
    mgr->register_creature_gossip(32692, new DalaranGuard());
    mgr->register_creature_gossip(32693, new DalaranGuard());
}
