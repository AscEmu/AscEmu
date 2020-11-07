

#include "Setup.h"
#include "Instance_AhnKahetTheOldKingdom.h"
#include <Units/Creatures/Pet.h>

class AhnKahetTheOldKingdomInstanceScript : public InstanceScript
{
public:

    explicit AhnKahetTheOldKingdomInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new AhnKahetTheOldKingdomInstanceScript(pMapMgr); }


};

void SetupAhnKahetTheOldKingdom(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_AHN_KAHET, &AhnKahetTheOldKingdomInstanceScript::Create);
}
