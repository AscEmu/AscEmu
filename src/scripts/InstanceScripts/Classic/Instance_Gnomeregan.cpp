

#include "Setup.h"
#include "Instance_Gnomeregan.h"
#include <Units/Creatures/Pet.h>

class GnomereganInstanceScript : public InstanceScript
{
public:

    explicit GnomereganInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new GnomereganInstanceScript(pMapMgr); }


};

void SetupGnomeregan(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GNOMEREGAN, &GnomereganInstanceScript::Create);
}
