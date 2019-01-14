/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"

enum
{
    CN_DTFLAMESCALE = 12463,
    FLAME_SHOCK = 22423, // 1 target, ranged like

    CN_DTWYRMKIN = 12465,
    FIREBALL_VOLLEY = 36742, // various targets

    CN_TECHNICIAN = 13996,
    GRANADE = 30217,

    CN_BLACK_WARLOCK = 12459,
    RAIN_OF_FIRE = 19717,
    BLACK_WARLOCK_AI_SHADOW_BOLT = 36986,

    CN_LASHLAYER = 12017,
    FIRE_NOVA_LASH = 39001, // Blast wave
    //MORTAL_STRIKE = 9347,
    KNOCK_BACK = 20686,

    CN_FIREMAW = 11983,
    WING_BUFFET = 37319,
    FLAME_BUFFET = 23341,

    CN_EBONROC = 14601,
    SHADOW_OF_EBONROC = 23340,

    CN_FLAMEGOR = 11981,
    FIRE_NOVA = 23462,

    CN_VAELASTRASZ = 13020,
    ESSENCE_OF_THE_RED = 23513,
    FLAME_BREATH = 18435,
    BURNING_ADRENALINE = 18173,

    CN_DTCAPTAIN = 12467,
    MARK_OF_FLAMES = 25050, // 1 target
    MARK_OF_DETONATION = 22438, // 1 target
};

class DTcaptainAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DTcaptainAI);
    explicit DTcaptainAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto markOfFlames = addAISpell(MARK_OF_FLAMES, 15.0f, TARGET_ATTACKING);
        markOfFlames->setAttackStopTimer(1000);

        auto markOfDetonation = addAISpell(MARK_OF_DETONATION, 4.0f, TARGET_ATTACKING);
        markOfDetonation->setAttackStopTimer(2000);
    }
};


/*
Each Pack contains exactly one captain.
The Captain is a very powerful, well-armored lvl 62+ mob with ~240k HP.
They cast a debuff called "Mark of Flames" to random members of the raid, increasing fire damage taken by 1000 for 2 min.
Also they apply a magic debuff called "Mark of Detonation" to their current target, inflicting ~700 fire damage to ALL allies within 30 yards of the afflicted player each time he is struck by a melee attack.
They cannot be slept or otherwise crowd controlled, and must be tanked instead.
The Captain can also dispel any Hibernate effect cast on Death Talon Wyrmkin.
*/

class DTflamescaleAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DTflamescaleAI);
    explicit DTflamescaleAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto flameShock = addAISpell(FLAME_SHOCK, 15.0f, TARGET_ATTACKING);
        flameShock->setAttackStopTimer(1000);
    }
};

/*
Each Pack contains one or two of these, randomly determined for each pack.
Flamescales are moderately tough lvl 62+ mobs with ~120k HP.
They have a periodic ranged attack called "Flame Shock" which inflicts ~2000 fire damage and
applies a magic DoT that inflicts an additional ~350 fire damage every 3 sec for 12 sec.
Combined with the Mark of Flames, this attack and DoT can become quite dangerous.
They also have a randomly targeted charge that knocks back those affected.
They cannot be slept or otherwise crowdcontrolled, and must be tanked instead.
*/

// Death Talon Seether AI

/*
Each Pack contains one or two.
Seethers are moderately tough level 62+ mobs with ~110k HP.
They go into a frenzy state to inflict additional melee damage (can be removed with tranquilizing shot!), and have a frontal cleave attack.
They cannot be slept or otherwise crowdcontrolled, and must be tanked instead.
As of patch 1.12 Seethers now have a Flame Buffet ability that deals roughly 1k damage per stack. Be aware of this if you kite these.
*/

class DTwyrmkinAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DTwyrmkinAI);
    explicit DTwyrmkinAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto fireballVolley = addAISpell(FIREBALL_VOLLEY, 40.0f, TARGET_VARIOUS);
        fireballVolley->setAttackStopTimer(1000);
    }
};

/*
Each Pack contains one or two. Wyrmkin are relatively fragile level 61+ mobs with ~90k HP.
Their primary form of attack is a "Fireball Volley" that they use frequently, sending out powerful fireballs
doing ~1100 damage apiece to all targets within LoS of the Wyrmkin.
You can break LoS with any active Wyrmkin to avoid being hit by this attack.
Although physically weak, Wyrmkin are a large threat to the raid if allowed to use their Volley attack unchecked.
They can be put to sleep by a druid's hibernate ability, but due to their high level they will tend to resist or break free of this effect fairly often.
"Fireball Volley" can be prevented by chain fearing or chain stunning the mob while the rest of the raid nukes it down.
*/

class TechnicianAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TechnicianAI);
    explicit TechnicianAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto granade = addAISpell(GRANADE, 25.0f, TARGET_ATTACKING);
        granade->setAttackStopTimer(1000);
    }
};

//Relatively low HP. Has a ranged aoe grenade that inflicts moderate damage to anyone in the area of effect.

class BlackWarlockAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BlackWarlockAI);
    explicit BlackWarlockAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto rainOfFire = addAISpell(RAIN_OF_FIRE, 10.0f, TARGET_DESTINATION);
        rainOfFire->setAttackStopTimer(1000);

        auto shadowBolt = addAISpell(BLACK_WARLOCK_AI_SHADOW_BOLT, 15.0f, TARGET_DESTINATION, 0, 0, false, true);
        shadowBolt->setAttackStopTimer(1000);
    }
};

/*
Casts a Rain of Fire that does 925 to 1075 damage per tick, also summons Enraged Felguards.
Their summon spell, 'Demon Portal' takes 30 seconds to generate a felguard once it's been cast.
Summoned creatures must be kept banished while everyone else needs to run out of the rain of fire quickly.
They are immune to polymorph and have roughly 100k health.
The summonables should remain banished until the rest of the lab pack has been killed, at which point a tank can pick it up.
Felguards can also be Feared if there arent enough Banishes to go around, or if the raid is just that ballsy.
The Warlocks also cast a 1700 damage shadow bolt.
*/

// Death Talon Overseer AI

/*
Large dragonkins that are resistant to most schools of magic, and highly vulnerable to one other school.
Although tough, they will go down quickly once their weakness is deduced. It can be one of the following classes: Shadow, Nature, Arcane, Fire, Frost.
They have three main attacks. They hit for 800-1000 every 2 seconds, they cleave for 1k every 10ish, and do a fire blast attack for 1k fire damage every 10 seconds.
As these do only targetted damage (minus cleave) they can be easily held for last in these packs.
*/

// Blackwing Spellbinder AI

/*
Immune to magical attacks, can only be meleed.
The hit for something like 400 dps. They cast greater polymorphs and drop flamestrikes that hit for 1600-1900 damage and have a DoT tick for 400 damage.
The flamestrikes have a 5 yd aoe, so melee dps wants to stand at the very edge of the mobs hit box to avoid being affected.
Typically dealt with in tandem with Overseers.
There are rumors that the polymorph is only targetted against people that are not on the spellbinders aggro table.
That said, as of patch 2.0, Ive been polyd while dpsing these.
It would appear more accurate to say that the polymorph is targetted at people who do not have the spellbinders aggro.
*/

class LashlayerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LashlayerAI);
    explicit LashlayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto fireNovaLash = addAISpell(FIRE_NOVA_LASH, 15.0f, TARGET_VARIOUS);
        fireNovaLash->setAttackStopTimer(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(2287);     // None of your kind should be here. You have doomed only yourselves!
    }
};

class FiremawAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FiremawAI);
    explicit FiremawAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto wingBuffet = addAISpell(WING_BUFFET, 10.0f, TARGET_VARIOUS);
        wingBuffet->setAttackStopTimer(1000);

        auto flameBuffet = addAISpell(FLAME_BUFFET, 15.0f, TARGET_VARIOUS);
        flameBuffet->setAttackStopTimer(1000);
    }
};

class EbonrocAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EbonrocAI);
    explicit EbonrocAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto wingBuffet = addAISpell(WING_BUFFET, 10.0f, TARGET_VARIOUS);
        wingBuffet->setAttackStopTimer(1000);

        auto shadowOfEbonroc = addAISpell(SHADOW_OF_EBONROC, 15.0f, TARGET_ATTACKING);
        shadowOfEbonroc->setAttackStopTimer(1000);
    }
};

class FlamegorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FlamegorAI);
    explicit FlamegorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto wingBuffet = addAISpell(WING_BUFFET, 10.0f, TARGET_VARIOUS);
        wingBuffet->setAttackStopTimer(1000);

        auto fireNova = addAISpell(FIRE_NOVA, 20.0f, TARGET_VARIOUS, 0, 0, false, true);
        fireNova->setAttackStopTimer(1000);
    }
};

/*
He does a Blastwave / AoE Knockback similar to the Scholomance Handlers (in the undead dragonkin room).
It is fire based and does quite a hefty amount of damage.

Like most nasty bosses, he has a cleave that your cloth wearers are going to want to avoid.
While not as nasty as Vael's, it's still pretty nasty.

Possibly the most problematic attack that Broodlord has is an ungodly Mortal Strike that he will do frequently on your Main Tank.
This can crit for up to 8000 damage on plate and 15000+ on cloth. It also leaves the MS debuff that reduces healing, which is Not Good.
It is recommended that priests power word shield the tank when he applies the MS debuff.

The trickiest part of Broodlord is a single target knockback that he will do on whomever is highest on his threat list.
After he knocks a target back, that targets aggro is reduced by 50%.
Over the course of this fight, he will do this repeatedly on all of your tanks.
*/

// QUOTE -> "None of your kind should be here. You have doomed only yourselves!"
// sound A_BroodlordLashlayerAggro -> 8286
// sound A_BroodlordLashlayerLeashTrigge -> 8287

class VaelastraszAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VaelastraszAI);
    explicit VaelastraszAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        essenceOfTheRed = addAISpell(ESSENCE_OF_THE_RED, 0.0f, TARGET_VARIOUS);
        essenceOfTheRed->setAttackStopTimer(1000);

        auto flameBreath = addAISpell(FLAME_BREATH, 15.0f, TARGET_VARIOUS);
        flameBreath->setAttackStopTimer(3000);

        auto burningAdrenaline = addAISpell(BURNING_ADRENALINE, 3.0f, TARGET_ATTACKING);
        burningAdrenaline->setAttackStopTimer(2000);

        getCreature()->setHealth((uint32)(getCreature()->getMaxHealth() * 0.3f));
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->castSpell(getCreature(), essenceOfTheRed->mSpellInfo, essenceOfTheRed->mIsTriggered);
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(2296);     // Forgive me, your death only adds to my failure.
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "At last the agony ends. I have failed you my Queen... I have failed us all...");
    }

    void AIUpdate() override
    {
        if (getCreature()->getHealthPct() <= 15 && getScriptPhase() == 1)
        {
            sendDBChatMessage(2295);     // Nefarius' hate has made me stronger than ever before. You should have fled, while you could, mortals! The fury of Blackrock courses through my veins!
            setScriptPhase(2);
        }
    }

protected:

    CreatureAISpells* essenceOfTheRed;
};

class VaelastraszGossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* Plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 9903, 0);
        menu.Send(Plr);
    }
};


/*
Essence of the Red [1]
    This debuff is cast on the raid at the beginning of the encounter and lasts for 3 minutes.
        Restores 500 Mana per second to mana users. Restores 50 Energy per second to Rogues and Cat Form Druids.
        Generates 20 Rage per second for Warriors and Bear Form Druids. It essentially means players have infinite mana/rage/energy for the fight.
        It is not dispellable, but can be removed by ice block or divine shield (which is not advised).

Fire Nova
    Inflicts 555-645 Fire damage to nearby enemies.
    Fire resistance is a must in order for healers to keep the raid alive.
    Player should wear as much quality FR gear as possible, but as a general rule, casters should have a little over 150, melee DPS over 200, and all Main Tanks the full 315 fire Resistance.

Flame Breath
    Inflicts 3063 to 3937 Fire damage to enemies in a cone in front of the caster.
    Must be directed away from the raid by the MT. Every flame breath applies a stacking debuff (also called flame breath) that ticks for 1000ish fire damage every few seconds.
    It's maximum level, which is always reached by the time the MT is burning, ticks for 4600 damage. This debuff plays a huge role. Early in the rotation, the MT takes only 1200dps or so. By the time they are burning, they're taking more like 2000dps.
    The difference is all in this attack.

Burning Adrenaline [2]
    Damage done increased by 100%. Attack speed increased by 100%. Spells become instant cast.
        Reduces max health by 5% every second; eventually causes player to die.

    Vael only casts Burning Adrenaline in two scenarios: He will cast it on random mana users throughout the fight and he will cast it on the current tank every 45 seconds.
    1 tank killed every 45 seconds for 3 minutes --> 3 / 0.75 = 4 tanks.

Cleave
    Cleave attack that hits for 2k. This is a chain cleave, so if positioning is poor it can chain to the entire raid, even to behind him.
*/

/*Lord Victor Nefarius (as he is corrupting Vaelastrasz)

    "Ah, the Heroes, you are persistent aren't you? Your Ally here attempted to match his power against mine....and paid the price. Now he shall serve me. By slaughtering you."
    "Get up little Red Wyrm, and destroy them!"
*/

// sound A_LordVictorNefariusAtStart -> 8279

/*Vaelastrasz (as the corruption slowly takes over)
    "Too late...friends. Nefarius' corruption has taken hold. I cannot...control myself. I beg you Mortals, flee! Flee before I lose all sense of control. The Black Fire rages within my heart. I must release it! FLAME! DEATH! DESTRUCTION! COWER MORTALS BEFORE THE WRATH OF LORD....NO! I MUST FIGHT THIS!"
*/

// sound A_VaelastraszLine1 -> 8281
// sound A_VaelastraszLine2 -> 8282
// sound A_VaelastraszLine2 -> 8283

void SetupBlackwingLair(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_DTCAPTAIN, &DTcaptainAI::Create);
    mgr->register_creature_script(CN_DTFLAMESCALE, &DTflamescaleAI::Create);
    mgr->register_creature_script(CN_DTWYRMKIN, &DTwyrmkinAI::Create);
    mgr->register_creature_script(CN_TECHNICIAN, &TechnicianAI::Create);
    mgr->register_creature_script(CN_BLACK_WARLOCK, &BlackWarlockAI::Create);
    mgr->register_creature_script(CN_LASHLAYER, &LashlayerAI::Create);
    mgr->register_creature_script(CN_FIREMAW, &FiremawAI::Create);
    mgr->register_creature_script(CN_EBONROC, &EbonrocAI::Create);
    mgr->register_creature_script(CN_FLAMEGOR, &FlamegorAI::Create);
    mgr->register_creature_script(CN_VAELASTRASZ, &VaelastraszAI::Create);

    mgr->register_creature_gossip(CN_VAELASTRASZ, new VaelastraszGossip()); //\todo  Vael Gossip change the flag to agressive
}
