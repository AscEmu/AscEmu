

#include "Setup.h"
#include "Instance_VaultOfArchavon.h"
#include <Units/Creatures/Pet.h>

class VaultOfArchavonInstanceScript : public InstanceScript
{
public:

    explicit VaultOfArchavonInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new VaultOfArchavonInstanceScript(pMapMgr); }


};

void SetupVaultOfArchavon(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_VAULT_OF_ARCHAVON, &VaultOfArchavonInstanceScript::Create);
}
