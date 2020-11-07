

#include "Setup.h"
#include "Instance_TheOculus.h"
#include <Units/Creatures/Pet.h>

class TheOculusInstanceScript : public InstanceScript
{
public:

    explicit TheOculusInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheOculusInstanceScript(pMapMgr); }


};

void SetupTheOculus(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_OCULUS, &TheOculusInstanceScript::Create);
}
