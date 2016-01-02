/********************************************************
*				AscEmu scripts by Cronic					*
*	|													*
*	V			Ahn'kahet: The Old Kingdom				*
*														*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/
//Dungeon Issues:
/*Boss: Elder Nadox
- Need better handling for creature spawning.

*Boss: Prince Taldaram
- Need to improve the checking system of the nerubian devices somehow to devices changes states.
- Need to implement the flame sphere phases.
- vanish/embrace phases may need improvements.

*Boss: Jedoga Shadowseeker
- Achievement is bugging out need to investigate.
- needs improvement overall for AI.

*Boss: Herald Volazj
Too Easy..
- Missing insanity phase, needs to be implemented but maybe hard due to phasing issues.
*/


enum HeraldVolazjData
{
	SPELL_MIND_FLAY = 57941,
	SPELL_SHADOW_BOLT_VOLLEY = 57942,
	SPELL_SHIVER = 57949
};

class HeraldVolazj : public MoonScriptBossAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(HeraldVolazj, MoonScriptBossAI);
	HeraldVolazj(Creature* pCreature) : MoonScriptBossAI(pCreature)
	{
		AddSpell(SPELL_MIND_FLAY, Target_Current, 100, 3, 20);
		AddSpell(SPELL_SHADOW_BOLT_VOLLEY, Target_Current, 50, 0, 5);
		AddSpell(SPELL_SHIVER, Target_Current, 50, 0, 15);

		AddEmote(Event_OnTargetDied, "Ywaq puul skshgn: on'ma yeh'glu zuq.", Text_Yell, 14045);
		AddEmote(Event_OnTargetDied, "Ywaq ma phgwa'cul hnakf.", Text_Yell, 14046);
		AddEmote(Event_OnTargetDied, "Ywaq maq oou; ywaq maq ssaggh. Ywaq ma shg'fhn.", Text_Yell, 14047);
	}

	void OnCombatStart(Unit* mTarget)
	{
		Emote("Shgla'yos plahf mh'naus.", Text_Yell, 14043);
		AchievementTimer = AddTimer(120000);
		AchievementStatus = true;
		ParentClass::OnCombatStart(mTarget);
	}

	void OnDied(Unit* mKiller)
	{
		Emote("Iilth vwah, uhn'agth fhssh za.", Text_Yell, 14048);
		if (AchievementStatus == true && IsHeroic())
		{
			_unit->GetMapMgr()->SendAchievementToAllPlayers(1862);
		}
		ParentClass::OnDied(mKiller);
	}

	void AIUpdate()
	{
		if (IsTimerFinished(AchievementTimer))
		{
			AchievementStatus = false; // If timer finished we failed the achievement.
			RemoveTimer(AchievementTimer);
		}
		ParentClass::AIUpdate();
	}

	void OnLoad()
	{
		if (IsHeroic()) // Heroic Difficulty
		{
			_unit->SetMaxHealth(431392);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(6344.0f);
			_unit->SetMaxDamage(8346.0f);
			_unit->setLevel(82);
		}
		ParentClass::OnLoad();
	}
protected:
	int32 AchievementTimer;
	bool AchievementStatus;
};

enum JedogaData
{
	SPELL_SPHERE_VISUAL = 56075,
	SPELL_GIFT_OF_THE_HERALD = 56219,
	SPELL_CYCLONE_STRIKE = 56855,
	SPELL_LIGHTNING_BOLT = 56891,
	SPELL_THUNDERSHOCK = 56926
};

class Jedoga : public MoonScriptBossAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Jedoga, MoonScriptBossAI);
	Jedoga(Creature* pCreature) : MoonScriptBossAI(pCreature)
	{
		AddSpell(SPELL_CYCLONE_STRIKE, Target_Self, 100, 0, 20);
		AddSpell(SPELL_LIGHTNING_BOLT, Target_RandomPlayer, 70, 0, 20, 0.0f, 40.0f, true);
		AddSpell(SPELL_THUNDERSHOCK, Target_RandomPlayer, 70, 0, 20, 0.0f, 30.0f, true);
	}

	void OnLoad()
	{
		if (IsHeroic()) // Heroic Difficulty
		{
			_unit->SetMaxHealth(431392);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(6019.0f);
			_unit->SetMaxDamage(8320.0f);
			_unit->setLevel(82);
			RegisterAIUpdateEvent(1000);
		}
		ParentClass::OnLoad();
	}

	void OnDied(Unit* mKiller) // Achievement: Volunteer Work - Defeat Jedoga Shadowseeker in Ahn'kahet on Heroic Difficulty without killing any Twilight Volunteers.
	{
		Creature* Volunteer = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), 30385);
		if (Volunteer)
		{
			if (Volunteer->IsDead() == false && Volunteer->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_ATTACKABLE_ALL) && IsHeroic())
			{
				_unit->GetMapMgr()->SendAchievementToAllPlayers(2056);
			}
			else
			{
				return;
			}
		}
		Emote("Do not expect your sacrilege... to go unpunished.", Text_Yell, 14351);
		ParentClass::OnDied(mKiller);
	}

	void OnCombatStart(Unit* mTarget)
	{
		Emote("These are sacred halls! Your intrusion will be met with death.", Text_Yell, 14343);
		SpawnCreature(30385, 362.45f, -714.16f, -16.096f, -2.3f);
		SpawnCreature(30385, 364.93f, -716.10f, -16.096f, -2.54f);
		SpawnCreature(30385, 362.01f, -719.82f, -16.096f, -2.52f);
		SpawnCreature(30385, 368.15f, -719.76f, -16.096f, -2.86f);
		SpawnCreature(30385, 368.78f, -713.93f, -16.096f, -2.75f);
		SpawnCreature(30385, 375.399994f, -711.434021f, -16.096399f, 2.679116f);
		SpawnCreature(30385, 379.049011f, -712.898987f, -16.096399f, 2.419089f);
		SpawnCreature(30385, 379.204010f, -716.697021f, -16.096399f, 2.599783f);
		SpawnCreature(30385, 378.424011f, -708.388000f, -16.096399f, 2.042742f);
		SpawnCreature(30385, 382.583008f, -711.713013f, -16.096399f, 2.131312f);
		SpawnCreature(30385, 385.692993f, -694.375977f, -16.096399f, 0.886445f);
		SpawnCreature(30385, 383.812012f, -700.409973f, -16.096399f, 1.169775f);
		SpawnCreature(30385, 387.223999f, -698.005981f, -16.096399f, 1.116557f);
		SpawnCreature(30385, 392.276001f, -695.895020f, -16.096399f, 1.131077f);
		SpawnCreature(30385, 389.626007f, -702.299988f, -16.096399f, 1.400275f);
		ParentClass::OnCombatStart(mTarget);
	}

	void OnReachWP(uint32 iWaypointId, bool bForwards)
	{
		if (iWaypointId == 1)
		{
			_unit->Root();
		}
	}

	void DoAirPhase()
	{
		sEAS.CreateCustomWaypointMap(_unit);
		sEAS.WaypointCreate(_unit, 372.330994f, -705.278015f, -0.624178f, 5.427970f, 1000, 256, 0);
		sEAS.EnableWaypoints(_unit);
		_unit->MoveToWaypoint(1);
		_unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		_unit->CastSpell(_unit, SPELL_SPHERE_VISUAL, true);
		switch (RandomUInt(1, 15))
		{
		case 1:
			Emote("Who among you is devoted?", Text_Yell, 14344);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(362.45f, -714.16f, -16.096f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(362.45f, -714.16f, -16.096f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 2:
			Emote("You there! Step forward!", Text_Yell, 14345);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(364.93f, -716.10f, -16.096f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(364.93f, -716.10f, -16.096f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 3:
			Emote("Who among you is devoted?", Text_Yell, 14344);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(362.01f, -719.82f, -16.096f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(362.01f, -719.82f, -16.096f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 4:
			Emote("You there! Step forward!", Text_Yell, 14345);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(368.15f, -719.76f, -16.096f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(368.15f, -719.76f, -16.096f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 5:
			Emote("Who among you is devoted?", Text_Yell, 14344);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(368.78f, -713.93f, -16.096f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(368.78f, -713.93f, -16.096f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 6:
			Emote("You there! Step forward!", Text_Yell, 14345);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(375.399994f, -711.434021f, -16.096399f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(375.399994f, -711.434021f, -16.096399f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 7:
			Emote("You there! Step forward!", Text_Yell, 14345);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(379.049011f, -712.898987f, -16.096399f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(379.049011f, -712.898987f, -16.096399f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 8:
			Emote("Who among you is devoted?", Text_Yell, 14344);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(379.204010f, -716.697021f, -16.096399f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(379.204010f, -716.697021f, -16.096399f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 9:
			Emote("You there! Step forward!", Text_Yell, 14345);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(378.424011f, -708.388000f, -16.096399f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(378.424011f, -708.388000f, -16.096399f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 10:
			Emote("Who among you is devoted?", Text_Yell, 14344);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(382.583008f, -711.713013f, -16.096399f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(382.583008f, -711.713013f, -16.096399f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 11:
			Emote("You there! Step forward!", Text_Yell, 14345);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(385.692993f, -694.375977f, -16.096399f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(385.692993f, -694.375977f, -16.096399f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 12:
			Emote("Who among you is devoted?", Text_Yell, 14344);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(383.812012f, -700.409973f, -16.096399f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(383.812012f, -700.409973f, -16.096399f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 13:
			Emote("You there! Step forward!", Text_Yell, 14345);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(387.223999f, -698.005981f, -16.096399f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(387.223999f, -698.005981f, -16.096399f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 14:
			Emote("Who among you is devoted?", Text_Yell, 14344);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(392.276001f, -695.895020f, -16.096399f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(392.276001f, -695.895020f, -16.096399f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		case 15:
			Emote("Who among you is devoted?", Text_Yell, 14344);
			SpawnCreature(30181, 372.330994f, -705.278015f, -16.179716f); // Jedoga controller summon circle visual.
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(389.626007f, -702.299988f, -16.096399f, 30385)->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(389.626007f, -702.299988f, -16.096399f, 30385)->GetAIInterface()->SetAllowedToEnterCombat(false);
			break;
		}
	}

	void AIUpdate()
	{
		if (_unit->GetHealthPct() <= 50 && GetPhase() == 1)
		{
			DoAirPhase();
			_unit->DisableAI();
			SetPhase(2);
		}
		ParentClass::AIUpdate();
	}
protected:
	int32 SacraficePhase;
	bool AirPhase;
};

class JedogaController : public MoonScriptBossAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(JedogaController, MoonScriptBossAI);
	JedogaController(Creature* pCreature) : MoonScriptBossAI(pCreature)
	{
	}

	void OnLoad()
	{
		_unit->CastSpell(_unit, 46172, true);
		ParentClass::OnLoad();
	}
};

class TwilightVolunteer : public MoonScriptBossAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(TwilightVolunteer, MoonScriptBossAI);
	TwilightVolunteer(Creature* pCreature) : MoonScriptBossAI(pCreature)
	{
	}

	void OnLoad()
	{
		_unit->GetAIInterface()->StopFlying();
		_unit->SetDisplayId(11686);
		_unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		_unit->GetAIInterface()->SetWalk();
		appear = AddTimer(RandomUInt(3000, 7000));
		RegisterAIUpdateEvent(1000);
		ParentClass::OnLoad();
	}

	void OnDied(Unit* mKiller)
	{
		Jedoga->RemoveAura(56075);
		Jedoga->EnableAI();
		Jedoga->GetAIInterface()->MoveCharge(372.330994f, -705.278015f, -16.179716f);
		Jedoga->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
		Jedoga->Unroot();
		ParentClass::OnDied(mKiller);
	}

	void AIUpdate()
	{
		if (_unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_ATTACKABLE_ALL)) // Selected
		{
			_unit->GetAIInterface()->MoveTo(372.330994f, -705.278015f, -16.179716f, 5.4f);
		}
		if (Jedoga)
		{
			if (Jedoga->IsDead())
			{
				_unit->Despawn(1000, 0);
			}
			else if (Jedoga->GetSpawnX(), Jedoga->GetSpawnY(), Jedoga->GetSpawnZ() && Jedoga->CombatStatus.IsInCombat() == false)
			{
				_unit->Despawn(1000, 0);
			}
		}
		if (_unit->GetPositionX() == 372.330994f && _unit->GetPositionY() == -705.278015f && _unit->GetPositionZ() == -16.179716f && sacraficed == false)
		{
			_unit->CastSpell(_unit, 56219, true);
			_unit->GetAIInterface()->SetAllowedToEnterCombat(true);
			sacraficed = true;
		}
		if (IsTimerFinished(appear))
		{
			_unit->SetDisplayId(_unit->GetNativeDisplayId());
			_unit->CastSpell(_unit, 48695, true);
			_unit->SetStandState(STANDSTATE_KNEEL);
			RemoveTimer(appear);
		}
		ParentClass::AIUpdate();
	}
protected:
	int32 appear;
	bool sacraficed;
	Creature* Jedoga = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(372.330994f, -705.278015f, -0.624178f, 29310);
};

enum PrinceTaldaramData
{
	SPELL_BLOODTHIRST = 55968, // Trigger Spell + add aura
	SPELL_EMBRACE_OF_THE_VAMPYR = 55959,
	// Flame Sphere spell needs to have better fuctioning before we attempt it.
	/* 
	SPELL_CONJURE_FLAME_SPHERE = 55931,
	SPELL_FLAME_SPHERE_SUMMON_1 = 55895, // 1x 30106
	SPELL_FLAME_SPHERE_SUMMON_2 = 59511, // 1x 31686
	SPELL_FLAME_SPHERE_SUMMON_3 = 59512, // 1x 31687
	SPELL_FLAME_SPHERE_SPAWN_EFFECT = 55891,
	SPELL_FLAME_SPHERE_VISUAL = 55928,
	SPELL_FLAME_SPHERE_PERIODIC = 55926,
	SPELL_FLAME_SPHERE_DEATH_EFFECT = 55947,
	NPC_FLAME_SPHERE_1 = 30106,
	NPC_FLAME_SPHERE_2 = 31686,
	NPC_FLAME_SPHERE_3 = 31687,
	*/
	SPELL_VANISH = 55964
};
class PrinceTaldaram : public MoonScriptBossAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(PrinceTaldaram, MoonScriptBossAI);
	PrinceTaldaram(Creature* pCreature) : MoonScriptBossAI(pCreature)
	{
		AddSpell(SPELL_BLOODTHIRST, Target_Self, 100, 0, 10);

		AddEmote(Event_OnTargetDied, "I will drink no blood before it's time.", Text_Yell, 14366);
		AddEmote(Event_OnTargetDied, "One final embrace.", Text_Yell, 14367);
	}

	void OnCombatStart(Unit* mTarget)
	{
		Emote("I will feast on your remains.", Text_Yell, 14360);
		VanishTimer = AddTimer(RandomUInt(25000, 35000));
		Intro = true;
		ParentClass::OnCombatStart(mTarget);
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(458354);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(6439.0f);
			_unit->SetMaxDamage(8916.0f);
			_unit->setLevel(82);
			RegisterAIUpdateEvent(1000);
		}
		ParentClass::OnLoad();
	}

	void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
	{
		if (_unit->HasAura(SPELL_EMBRACE_OF_THE_VAMPYR))
		{
			int32 BreakChance;
			BreakChance = rand() % 100;
			if (BreakChance <= 4) // "Damage to Prince Taldaram may break this effect."
			{
				_unit->InterruptSpell();
				ResetTimer(VanishTimer, RandomUInt(25000, 35000));
			}
		}
		ParentClass::OnDamageTaken(mAttacker, fAmount);
	}

	void AIUpdate()
	{
		GameObject* NerubianDevice1 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(692.535f, -785.49f, 18.99f, 193094);
		GameObject* NerubianDevice2 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(654.167f, -719.84f, 17.97f, 193093);
		GameObject* BlueAura = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(529.52f, -847.95f, 11.30f, 193564);
		if (NerubianDevice1 && NerubianDevice1->GetState() == GAMEOBJECT_STATE_OPEN && NerubianDevice2 && NerubianDevice2->GetState() == GAMEOBJECT_STATE_OPEN && Intro == false)
		{
			Emote("Who dares enter the old kingdom? you outsiders will die!", Text_Yell, 0); // need to find the real text.
			_unit->GetAIInterface()->MoveTo(529.52f, -847.95f, 11.30f, 1.32f);
			_unit->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			Intro = true;
			if (BlueAura)
			{
				BlueAura->Despawn(1000, 0);
			}
		}
		if (IsTimerFinished(VanishTimer))
		{
			//This is hacked very badly but we need it since if we use the real vanish spell it will take boss out of combat.
			DoVanishPhase();
		}
		if (IsTimerFinished(VanishReturn))
		{
			_unit->SetDisplayId(27406);
			_unit->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
			_unit->CastSpell(TargetGen_RandomPlayer, SPELL_EMBRACE_OF_THE_VAMPYR, false);
			ResetTimer(VanishTimer, RandomUInt(25000, 35000));
		}
		ParentClass::AIUpdate();
	}

	void DoVanishPhase()
	{
		_unit->SetDisplayId(11686); // Our hacked version of "vanish"
		_unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		VanishReturn = AddTimer(2500);
	}

	void OnDied(Unit* mKiller)
	{
		Emote("Still I hunger, still I thirst.", Text_Yell, 13618);
		GameObject* WebDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(443.06f, -325.156f, 104.023f, 192174);
		if (WebDoor)
		WebDoor->Despawn(1000, 0);
		ParentClass::OnDied(mKiller);
	}
protected:
	bool Intro;
	int32 VanishTimer; int32 VanishReturn;
};

enum NadoxSpells
{
	// Elder Nadox
	SPELL_BROOD_PLAGUE = 56130,
	H_SPELL_BROOD_RAGE = 59465,
	SPELL_ENRAGE = 26662,
	SPELL_SUMMON_SWARMERS = 56119,
	SPELL_SUMMON_SWARM_GUARD = 56120
};

class ElderNadox : public MoonScriptBossAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(ElderNadox, MoonScriptBossAI);
	ElderNadox(Creature* pCreature) : MoonScriptBossAI(pCreature)
	{
		AddSpell(SPELL_BROOD_PLAGUE, Target_RandomPlayer, 100, 0, 13);
		if (IsHeroic())
		{
			AddSpell(H_SPELL_BROOD_RAGE, Target_Self, 100, 0, RandomUInt(13, 50));
		}

		AddEmote(Event_OnTargetDied, "Sleep now, in the cold dark.", Text_Yell, 14036);
		AddEmote(Event_OnTargetDied, "For the Lich King!.", Text_Yell, 14037);
		AddEmote(Event_OnTargetDied, "Perhaps we will be allies soon.", Text_Yell, 14038);
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(431392);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(6019.0f);
			_unit->SetMaxDamage(8320.0f);
			_unit->setLevel(82);
		}
		ParentClass::OnLoad();
	}

	void OnCombatStart(Unit* mTarget)
	{
		Emote("The secrets of the deep shall remain hidden.", Text_Yell, 14033);
		SwarmersTimer = AddTimer(10000);
		ParentClass::OnCombatStart(mTarget);
	}

	void OnDied(Unit* mKiller)
	{
		//Achievement 2038 (Respect Your Elders)
		//Defeat Elder Nadox while a Ah'Kahar Guardian is alive on heroic mode.
		Creature* NeruGuard = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), 30176);
		if (NeruGuard && NeruGuard->IsDead() == false && IsHeroic())
		{
			_unit->GetMapMgr()->SendAchievementToAllPlayers(2038);
		}
		else
		{
			return;
		}
		Emote("Master, is my service complete?", Text_Yell, 14039);
		ParentClass::OnDied(mKiller);
	}

	void AIUpdate()
	{
		if (IsTimerFinished(SwarmersTimer))
		{
			_unit->CastSpell(_unit, SPELL_SUMMON_SWARMERS, true);
			switch (RandomUInt(1, 4))
			{
			case 1:
				break;
			case 2:
				Emote("The young must not grow hungry...", Text_Yell, 14034);
				break;
			case 3:
				break;
			case 4:
				Emote("Shhhad ak kereeesshh chak-k-k!", Text_Yell, 14035);
				break;
			}
			ResetTimer(SwarmersTimer, 10000);
		}

		if (_unit->GetPositionZ() < 24.0f)
		{
			if (_unit->HasAura(SPELL_ENRAGE))
			{
				return;
			}
			_unit->CastSpell(_unit, SPELL_ENRAGE, true);
		}

		if (_unit->GetHealthPct() <= 50 && GuardianSummon == false)
		{
			_unit->SendChatMessage(42, 0, "An Ahn'kahar Guardian hatches!");
			_unit->CastSpell(_unit, SPELL_SUMMON_SWARM_GUARD, true);
			GuardianSummon = true;
		}
			ParentClass::AIUpdate();
	}
protected:
	int32 SwarmersTimer;
	bool GuardianSummon;
};

void SetupAhnkahet(ScriptMgr* mgr)
{
	//Bosses
	mgr->register_creature_script(29309, &ElderNadox::Create);
	mgr->register_creature_script(29310, &Jedoga::Create);
	mgr->register_creature_script(30385, &TwilightVolunteer::Create);
	mgr->register_creature_script(29308, &PrinceTaldaram::Create);
	mgr->register_creature_script(29311, &HeraldVolazj::Create);
};