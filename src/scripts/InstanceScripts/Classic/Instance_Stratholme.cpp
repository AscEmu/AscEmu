

#include "Setup.h"
#include "Instance_Stratholme.h"
#include <Units/Creatures/Pet.h>

class StratholmeInstanceScript : public InstanceScript
{
public:

    explicit StratholmeInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new StratholmeInstanceScript(pMapMgr); }


};

void SetupStratholme(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_STRATHOLME, &StratholmeInstanceScript::Create);
}
