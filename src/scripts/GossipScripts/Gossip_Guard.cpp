/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/Gossip/Gossip.h"
#include "Setup.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Objects/ObjectMgr.h"


class StormwindGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 114;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class DarnassusGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 122;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class UndercityGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 142;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class UndercityGuardOverseer : public GossipScript
{
public:

    uint32_t definedGossipMenu = 163;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class ThunderbluffGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 152;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class GoldshireGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 132;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class TeldrassilGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 172;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class SilvermoonGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 180;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class ExodarGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 191;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class OrgrimmarGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 724;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class BloodhoofGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 751;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class RazorHillGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 989;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class BrillGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1003;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class IronforgeGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1012;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class KharanosGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1035;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class FalconwingGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1047;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class AzureWatchGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1058;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class ShattrathGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1068;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

class DalaranGuard : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1095;
    void GossipHello(Object* object, Player* player)
    {
        objmgr.createGuardGossipMenuForPlayer(object->GetGUID(), definedGossipMenu, player);
    }

    void GossipSelectOption(Object* object, Player* player, uint32 Id, uint32 IntId, const char* Code, uint32_t gossipId)
    {
        if (IntId > 0)
        {
            if (gossipId != 0)
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, gossipId);
            else
                objmgr.createGuardGossipOptionAndSubMenu(object->GetGUID(), player, IntId, definedGossipMenu);
        }
        else
        {
            GossipHello(object, player);
        }
    }
};

void SetupGuardGossip(ScriptMgr* mgr)
{
    mgr->register_gossip_script(1423, new GoldshireGuard);
    mgr->register_gossip_script(68, new StormwindGuard);
    mgr->register_gossip_script(1976, new StormwindGuard);
    mgr->register_gossip_script(29712, new StormwindGuard);
    mgr->register_gossip_script(4262, new DarnassusGuard);
    mgr->register_gossip_script(5624, new UndercityGuard);
    mgr->register_gossip_script(36213, new UndercityGuardOverseer);
    mgr->register_gossip_script(3571, new TeldrassilGuard);
    mgr->register_gossip_script(16222, new SilvermoonGuard);
    mgr->register_gossip_script(16733, new ExodarGuard);
    mgr->register_gossip_script(20674, new ExodarGuard);
    mgr->register_gossip_script(3296, new OrgrimmarGuard);
    mgr->register_gossip_script(3084, new ThunderbluffGuard);
    mgr->register_gossip_script(3222, new BloodhoofGuard);
    mgr->register_gossip_script(3224, new BloodhoofGuard);
    mgr->register_gossip_script(3220, new BloodhoofGuard);
    mgr->register_gossip_script(3219, new BloodhoofGuard);
    mgr->register_gossip_script(3217, new BloodhoofGuard);
    mgr->register_gossip_script(3215, new BloodhoofGuard);
    mgr->register_gossip_script(3218, new BloodhoofGuard);
    mgr->register_gossip_script(3221, new BloodhoofGuard);
    mgr->register_gossip_script(3223, new BloodhoofGuard);
    mgr->register_gossip_script(3212, new BloodhoofGuard);
    mgr->register_gossip_script(5953, new RazorHillGuard);
    mgr->register_gossip_script(5725, new BrillGuard);
    mgr->register_gossip_script(1738, new BrillGuard);
    mgr->register_gossip_script(1652, new BrillGuard);
    mgr->register_gossip_script(1746, new BrillGuard);
    mgr->register_gossip_script(1745, new BrillGuard);
    mgr->register_gossip_script(1743, new BrillGuard);
    mgr->register_gossip_script(1744, new BrillGuard);
    mgr->register_gossip_script(1496, new BrillGuard);
    mgr->register_gossip_script(1742, new BrillGuard);
    mgr->register_gossip_script(5595, new IronforgeGuard);
    mgr->register_gossip_script(727, new KharanosGuard);
    mgr->register_gossip_script(16221, new FalconwingGuard);
    mgr->register_gossip_script(18038, new AzureWatchGuard);
    mgr->register_gossip_script(19687, new ShattrathGuard);
    mgr->register_gossip_script(18568, new ShattrathGuard);
    mgr->register_gossip_script(18549, new ShattrathGuard);
    mgr->register_gossip_script(32675, new DalaranGuard);
    mgr->register_gossip_script(32676, new DalaranGuard);
    mgr->register_gossip_script(32677, new DalaranGuard);
    mgr->register_gossip_script(32678, new DalaranGuard);
    mgr->register_gossip_script(32679, new DalaranGuard);
    mgr->register_gossip_script(32680, new DalaranGuard);
    mgr->register_gossip_script(32681, new DalaranGuard);
    mgr->register_gossip_script(32683, new DalaranGuard);
    mgr->register_gossip_script(32684, new DalaranGuard);
    mgr->register_gossip_script(32685, new DalaranGuard);
    mgr->register_gossip_script(32686, new DalaranGuard);
    mgr->register_gossip_script(32687, new DalaranGuard);
    mgr->register_gossip_script(32688, new DalaranGuard);
    mgr->register_gossip_script(32689, new DalaranGuard);
    mgr->register_gossip_script(32690, new DalaranGuard);
    mgr->register_gossip_script(32691, new DalaranGuard);
    mgr->register_gossip_script(32692, new DalaranGuard);
    mgr->register_gossip_script(32693, new DalaranGuard);
}
