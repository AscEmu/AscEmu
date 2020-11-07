

#include "Setup.h"
#include "Raid_TrialOfTheCrusader.h"
#include <Units/Creatures/Pet.h>

class TrialOfTheCrusaderInstanceScript : public InstanceScript
{
public:

    explicit TrialOfTheCrusaderInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TrialOfTheCrusaderInstanceScript(pMapMgr); }


};

void SetupTrialOfTheCrusader(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TRIAL_OF_THE_CRUSADER, &TrialOfTheCrusaderInstanceScript::Create);
}
