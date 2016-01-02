/********************************************************
*				FrostwingCore Scripts					*
*														*
*				The Burning Crusade Trash				*
*Bosses already support heroic mode via there instance	*
*********************************************************/

// Wrath of Lich King
/********************************************************
*				FrostwingCore Scripts					*
*														*
*				Halls of Lightning						*
*Bosses already support heroic mode via there instance	*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/
class Heroic_HallsofLightning : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Heroic_HallsofLightning, MoonScriptCreatureAI);
	Heroic_HallsofLightning(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic()) // Heroic Difficulty
		{
			switch (_unit->GetEntry())
			{
			case 28580:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28578:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28579:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28583:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28584:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28965:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28838:
				_unit->SetMaxHealth(130330);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28961:
				_unit->SetMaxHealth(78198);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28836:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28582:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28835:
				_unit->SetMaxHealth(130330);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28837:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28920:
				_unit->SetMaxHealth(126000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 29240:
				_unit->SetMaxHealth(104264);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28826:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28547:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			}
		}
	}
};

/********************************************************
*				FrostwingCore Scripts					*
*														*
*					Gun'Drak							*
*Bosses already support heroic mode via there instance	*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/
class Heroic_Gundrak : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Heroic_Gundrak, MoonScriptCreatureAI);
	Heroic_Gundrak(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic()) // Heroic Difficulty
		{
			switch (_unit->GetEntry())
			{
			case 29713:
				_unit->SetMaxHealth(6517);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(464.0f);
				_unit->SetMaxDamage(604.0f);
				_unit->setLevel(81);
				break;
			case 29680:
				_unit->SetMaxHealth(6517);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(464.0f);
				_unit->SetMaxDamage(604.0f);
				_unit->setLevel(81);
				break;
			case 29774:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 29768:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 29820:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(81);
				break;
			case 29826:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(81);
				break;
			case 29832:
				_unit->SetMaxHealth(114039);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 29830:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 29829:
				_unit->SetMaxHealth(105893);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 29822:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(81);
				break;
			case 29834:
				_unit->SetMaxHealth(16291);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(464.0f);
				_unit->SetMaxDamage(604.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 29819:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 29874:
				_unit->SetMaxHealth(16291);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(464.0f);
				_unit->SetMaxDamage(604.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 29920:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 29982:
				_unit->SetMaxHealth(16291);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(464.0f);
				_unit->SetMaxDamage(604.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 29838:
				_unit->SetMaxHealth(130330);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 29836:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			}
		}
	}
};
/********************************************************
*				FrostwingCore Scripts					*
*														*
*					The Nexus							*
*Bosses already support heroic mode via there instance	*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/
class Heroic_Nexus : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Heroic_Nexus, MoonScriptCreatureAI);
	Heroic_Nexus(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic()) // Heroic Difficulty
		{
			switch (_unit->GetEntry())
			{
			case 26800:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5486.0f);
				_unit->SetMaxDamage(7618.0f);
				_unit->setLevel(80);
				break;
			case 26799:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5486.0f);
				_unit->SetMaxDamage(7618.0f);
				_unit->setLevel(80);
				break;
			case 26802:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26801:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26805:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26803:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26734:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5486.0f);
				_unit->SetMaxDamage(7618.0f);
				_unit->setLevel(80);
				break;
			case 26722:
				_unit->SetMaxHealth(130330);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(80);
				break;
			case 26735:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26716:
				_unit->SetMaxHealth(130330);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(80);
				break;
			case 26727:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26728:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26729:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5486.0f);
				_unit->SetMaxDamage(7618.0f);
				_unit->setLevel(80);
				break;
			case 26730:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26793:
				_unit->SetMaxHealth(5670);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(422.0f);
				_unit->SetMaxDamage(586.0f);
				_unit->setLevel(80);
				break;
			case 26782:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26792:
				_unit->SetMaxHealth(126000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5486.0f);
				_unit->SetMaxDamage(7618.0f);
				_unit->setLevel(80);
				break;
			case 28231:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26918:
				_unit->SetMaxHealth(28350);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26737:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				_unit->setLevel(80);
				break;
			case 26746:
				_unit->SetMaxHealth(9576);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(346.0f);
				_unit->SetMaxDamage(499.0f);
				_unit->setLevel(80);
				break;
			}
		}
	}
};
/********************************************************
*				FrostwingCore Scripts					*
*														*
*					Pit of Saron						*
*Bosses already support heroic mode via there instance	*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/
class Heroic_PitofSaron : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Heroic_PitofSaron, MoonScriptCreatureAI);
	Heroic_PitofSaron(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic()) // Heroic Difficulty
		{
			switch (_unit->GetEntry())
			{
			case 36788:
				_unit->SetMaxHealth(302400);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(4498.0f);
				_unit->SetMaxDamage(6487.0f);
				break;
			case 36881:
				_unit->SetMaxHealth(8013);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(422.0f);
				_unit->SetMaxDamage(586.0f);
				break;
			case 36830:
				_unit->SetMaxHealth(264600);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5486.0f);
				_unit->SetMaxDamage(7618.0f);
				break;
			case 37712:
				_unit->SetMaxHealth(105840);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(4498.0f);
				_unit->SetMaxDamage(6487.0f);
				break;
			case 37713:
				_unit->SetMaxHealth(105840);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(4498.0f);
				_unit->SetMaxDamage(6487.0f);
				break;
			case 37711:
				_unit->SetMaxHealth(132300);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5486.0f);
				_unit->SetMaxDamage(7618.0f);
				break;
			case 31260:
				_unit->SetMaxHealth(252000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5486.0f);
				_unit->SetMaxDamage(7618.0f);
				break;
			case 36891:
				_unit->SetMaxHealth(252000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5486.0f);
				_unit->SetMaxDamage(7618.0f);
				break;
			case 36886:
				_unit->SetMaxHealth(100800);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				break;
			case 36879:
				_unit->SetMaxHealth(252000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				break;
			case 36893:
				_unit->SetMaxHealth(119700);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				break;
			case 36892:
				_unit->SetMaxHealth(163800);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				break;
			case 36840:
				_unit->SetMaxHealth(119700);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				break;
			case 36841:
				_unit->SetMaxHealth(94500);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				break;
			case 36842:
				_unit->SetMaxHealth(132300);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				break;
			case 36877:
				_unit->SetMaxHealth(37800);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5421.0f);
				_unit->SetMaxDamage(7566.0f);
				break;
			}
		}
	}
};

/********************************************************
*				FrostwingCore Scripts					*
*														*
*					Violet Hold							*
*Bosses already support heroic mode via there instance	*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/
class Violet1 : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Violet1, MoonScriptCreatureAI);
	Violet1(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
				_unit->SetMaxHealth(162913);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7826.0f);
				_unit->setLevel(81);
				_unit->SetFaction(14);
		}
		ParentClass::OnLoad();
	}
};

class Violet2 : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Violet2, MoonScriptCreatureAI);
	Violet2(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(162913);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(6032.0f);
			_unit->SetMaxDamage(7852.0f);
			_unit->setLevel(81);
		}
		ParentClass::OnLoad();
	}
};

class Violet3 : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Violet3, MoonScriptCreatureAI);
	Violet3(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(6032.0f);
			_unit->SetMaxDamage(7852.0f);
			_unit->setLevel(81);
		}
		ParentClass::OnLoad();
	}
};

class Violet4 : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Violet4, MoonScriptCreatureAI);
	Violet4(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(6032.0f);
			_unit->SetMaxDamage(7852.0f);
			_unit->setLevel(81);
		}
		ParentClass::OnLoad();
	}
};

class Violet5 : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Violet5, MoonScriptCreatureAI);
	Violet5(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(6032.0f);
			_unit->SetMaxDamage(7852.0f);
			_unit->setLevel(81);
		}
		ParentClass::OnLoad();
	}
};

/********************************************************
*				FrostwingCore Scripts					*
*														*
*					Drak'Tharon Keep					*
*Bosses already support heroic mode via there instance	*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/
class RisenDrakkariSoulMage : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(RisenDrakkariSoulMage, MoonScriptCreatureAI);
	RisenDrakkariSoulMage(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class RisenDrakkariSoulWarrior : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(RisenDrakkariSoulWarrior, MoonScriptCreatureAI);
	RisenDrakkariSoulWarrior(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class ScourgeReanimator : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(ScourgeReanimator, MoonScriptCreatureAI);
	ScourgeReanimator(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(63000);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class FleshGhoul : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(FleshGhoul, MoonScriptCreatureAI);
	FleshGhoul(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class GhoulTorment : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(GhoulTorment, MoonScriptCreatureAI);
	GhoulTorment(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(63000);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class Belcher : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Belcher, MoonScriptCreatureAI);
	Belcher(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(130330);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariInvader : public MoonScriptCreatureAI // DAMAGE GUESSED
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariInvader, MoonScriptCreatureAI);
	DrakkariInvader(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(2000);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(200.0f);
			_unit->SetMaxDamage(325.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DarkwebHatch : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DarkwebHatch, MoonScriptCreatureAI);
	DarkwebHatch(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(12600);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(578.0f);
			_unit->SetMaxDamage(788.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DarkwebRecluse : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DarkwebRecluse, MoonScriptCreatureAI);
	DarkwebRecluse(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(6420.0f);
			_unit->SetMaxDamage(8753.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariGuardian : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariGuardian, MoonScriptCreatureAI);
	DrakkariGuardian(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariShaman : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariShaman, MoonScriptCreatureAI);
	DrakkariShaman(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(63000);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class CrystalHandler : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(CrystalHandler, MoonScriptCreatureAI);
	CrystalHandler(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(41704);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(4807.0f);
			_unit->SetMaxDamage(6567.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class FetidCorpse : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(FetidCorpse, MoonScriptCreatureAI);
	FetidCorpse(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(3780);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(578.0f);
			_unit->SetMaxDamage(788.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class HulkingCorpse : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(HulkingCorpse, MoonScriptCreatureAI);
	HulkingCorpse(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(25200);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(4807.0f);
			_unit->SetMaxDamage(5567.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class RisenShadowCaster : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(RisenShadowCaster, MoonScriptCreatureAI);
	RisenShadowCaster(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(3128);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(578.0f);
			_unit->SetMaxDamage(788.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariBat : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariBat, MoonScriptCreatureAI);
	DrakkariBat(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(12600);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(578.0f);
			_unit->SetMaxDamage(788.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariBatRider : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariBatRider, MoonScriptCreatureAI);
	DrakkariBatRider(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(63000);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariGutripper : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariGutripper, MoonScriptCreatureAI);
	DrakkariGutripper(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariMount : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariMount, MoonScriptCreatureAI);
	DrakkariMount(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(26066);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(725.0f);
			_unit->SetMaxDamage(1025.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariScytheclaw : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariScytheclaw, MoonScriptCreatureAI);
	DrakkariScytheclaw(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariHandler : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariHandler, MoonScriptCreatureAI);
	DrakkariHandler(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(63000);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariCommander : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariCommander, MoonScriptCreatureAI);
	DrakkariCommander(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class DrakkariDK : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(DrakkariDK, MoonScriptCreatureAI);
	DrakkariDK(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(100800);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};

class ScourgeBrute : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(ScourgeBrute, MoonScriptCreatureAI);
	ScourgeBrute(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(65165);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5525.0f);
			_unit->SetMaxDamage(7826.0f);
			_unit->setLevel(RandomUInt(80, 81));
		}
	}
};
/********************************************************
*				FrostwingCore Scripts					*
*	|													*
*	V				Utgarde Pinnacle					*
*Bosses already support heroic mode via there instance	*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/

class Heroic_UtgardePinnacle : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Heroic_UtgardePinnacle, MoonScriptCreatureAI);
	Heroic_UtgardePinnacle(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic()) // Normal Modes
		{
			switch (_unit->GetEntry())
			{
			case 26550:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 26553:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 26554:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 26536:
				_unit->SetMaxHealth(4725);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(556.0f);
				_unit->SetMaxDamage(758.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 26555:
				_unit->SetMaxHealth(130330);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 26672:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 26670:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 26669:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 26696:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 26694:
				_unit->SetMaxHealth(50400);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 28368:
				_unit->SetMaxHealth(52130);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			}
		}
	}
};

/********************************************************
*				FrostwingCore Scripts					*
*	|													*
*	V				Halls of Stone						*
*Bosses already support heroic mode via there instance	*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/

class Heroic_HallsofStone : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Heroic_HallsofStone, MoonScriptCreatureAI);
	Heroic_HallsofStone(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			switch (_unit->GetEntry())
			{
			case 27966:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5417.0f);
				_unit->SetMaxDamage(7561.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27962:
				_unit->SetMaxHealth(RandomUInt(63000, 65165));
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5417.0f);
				_unit->SetMaxDamage(7839.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27969:
				_unit->SetMaxHealth(130330);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5641.0f);
				_unit->SetMaxDamage(7839.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27964:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(4513.0f);
				_unit->SetMaxDamage(6271.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27965:
				_unit->SetMaxHealth(RandomUInt(63000, 65165));
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5417.0f);
				_unit->SetMaxDamage(7839.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27963:
				_unit->SetMaxHealth(RandomUInt(50400, 52430));
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(4499.0f);
				_unit->SetMaxDamage(6710.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27960:
				_unit->SetMaxHealth(RandomUInt(63000, 65165));
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5480.0f);
				_unit->SetMaxDamage(7905.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27961:
				_unit->SetMaxHealth(63000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5480.0f);
				_unit->SetMaxDamage(7624.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27149:
				_unit->SetMaxHealth(50400);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5417.0f);
				_unit->SetMaxDamage(7561.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27972:
				_unit->SetMaxHealth(RandomUInt(126000, 130330));
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(3611.0f);
				_unit->SetMaxDamage(5225.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27970:
				_unit->SetMaxHealth(126000);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5480.0f);
				_unit->SetMaxDamage(7624.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27971:
				_unit->SetMaxHealth(RandomUInt(126000, 130330));
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5480.0f);
				_unit->SetMaxDamage(7905.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27973:
				_unit->SetMaxHealth(25200);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(422.0f);
				_unit->SetMaxDamage(586.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27984:
				_unit->SetMaxHealth(RandomUInt(20160, 20972));
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(346.0f);
				_unit->SetMaxDamage(516.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			case 27985:
				_unit->SetMaxHealth(67405);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5951.0f);
				_unit->SetMaxDamage(8200.0f);
				_unit->setLevel(RandomUInt(80, 81));
				break;
			}
		}
	}
};

/********************************************************
*				FrostwingCore Scripts					*
*	|													*
*	V				Forge of Souls						*
*Bosses already support heroic mode via there instance	*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/

class WatchmanAI : public MoonScriptCreatureAI
{
	MOONSCRIPT_FACTORY_FUNCTION(WatchmanAI, MoonScriptCreatureAI);
	WatchmanAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	};

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(132300);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5486.0f);
			_unit->SetMaxDamage(7618.0f);
		}
		ParentClass::OnLoad();
	};
};

class AdeptAI : public MoonScriptCreatureAI
{
	MOONSCRIPT_FACTORY_FUNCTION(AdeptAI, MoonScriptCreatureAI);
	AdeptAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	};

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(105000);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5486.0f);
			_unit->SetMaxDamage(7618.0f);
		}
		ParentClass::OnLoad();
	};
};

class AnimatorAI : public MoonScriptCreatureAI
{
	MOONSCRIPT_FACTORY_FUNCTION(AnimatorAI, MoonScriptCreatureAI);
	AnimatorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	};

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(132000);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5486.0f);
			_unit->SetMaxDamage(7618.0f);
		}
		ParentClass::OnLoad();
	};
};

class BoneAI : public MoonScriptCreatureAI
{
	MOONSCRIPT_FACTORY_FUNCTION(BoneAI, MoonScriptCreatureAI);
	BoneAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	};

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(132000);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5486.0f);
			_unit->SetMaxDamage(7618.0f);
		}
		ParentClass::OnLoad();
	};
};

class ReaperAI : public MoonScriptCreatureAI
{
	MOONSCRIPT_FACTORY_FUNCTION(ReaperAI, MoonScriptCreatureAI);
	ReaperAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	};

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(132000);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5486.0f);
			_unit->SetMaxDamage(7618.0f);
		}
		ParentClass::OnLoad();
	};
};

class HorrorAI : public MoonScriptCreatureAI
{
	MOONSCRIPT_FACTORY_FUNCTION(HorrorAI, MoonScriptCreatureAI);
	HorrorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	};

	void OnLoad()
	{
		if (IsHeroic())
		{
			_unit->SetMaxHealth(94500);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5486.0f);
			_unit->SetMaxDamage(7618.0f);
		}
		ParentClass::OnLoad();
	};
};

class WardenAI : public MoonScriptCreatureAI
{
	MOONSCRIPT_FACTORY_FUNCTION(WardenAI, MoonScriptCreatureAI);
	WardenAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	};

	void OnLoad()
	{
		if (IsHeroic()) // Heroic Mode stats set until ascemu supports heroic modes.
		{
			_unit->SetMaxHealth(94500);
			_unit->SetHealth(_unit->GetMaxHealth());
			_unit->SetMinDamage(5486.0f);
			_unit->SetMaxDamage(7618.0f);
		}
		ParentClass::OnLoad();
	};
};

/********************************************************
*				FrostwingCore Scripts					*
*	|													*
*	V			Ahn'kahet: The Old Kingdom				*
*Bosses already support heroic mode via there instance	*
*Stats from wotlk.openwow.com & wowpedia				*
*********************************************************/
class Heroic_Ahnkahet : public MoonScriptCreatureAI
{
public:
	MOONSCRIPT_FACTORY_FUNCTION(Heroic_Ahnkahet, MoonScriptCreatureAI);
	Heroic_Ahnkahet(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
	}

	void OnLoad()
	{
		if (IsHeroic())
		{
			switch (_unit->GetEntry())
			{
			case 30338:
				_unit->SetMaxHealth(250);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(422.0f);
				_unit->SetMaxDamage(586.0f);
				_unit->setLevel(80);
				break;
			case 30114:
				_unit->SetMaxHealth(10080);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(346.0f);
				_unit->SetMaxDamage(499.0f);
				_unit->setLevel(80);
				break;
			case 30414:
				_unit->SetMaxHealth(130330);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30287:
				_unit->SetMaxHealth(18900);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(422.0f);
				_unit->SetMaxDamage(586.0f);
				_unit->setLevel(80);
				break;
			case 30276:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30278:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30329:
				_unit->SetMaxHealth(130330);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30416:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30419:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30418:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30319:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30179:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30111:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(5525.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30285:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30286:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30284:
				_unit->SetMaxHealth(130330);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30283:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30277:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			case 30279:
				_unit->SetMaxHealth(65165);
				_unit->SetHealth(_unit->GetMaxHealth());
				_unit->SetMinDamage(6032.0f);
				_unit->SetMaxDamage(7852.0f);
				_unit->setLevel(81);
				break;
			}
		}
	}
};


void SetupHeroicMode(ScriptMgr* mgr)
{
	// MISC
	// The Burning Crusade (Heroic Trash Support)
	// Wrath of Lich King (Heroic Trash Support)
	//Start Halls of Lightning
	mgr->register_creature_script(28580, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28578, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28579, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28583, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28584, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28965, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28838, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28961, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28836, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28582, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28835, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28837, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28920, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(29240, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28826, &Heroic_HallsofLightning::Create);
	mgr->register_creature_script(28547, &Heroic_HallsofLightning::Create);
	//End Halls of Lightning
	//Start Gun'drak
	mgr->register_creature_script(29713, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29680, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29774, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29768, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29820, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29826, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29832, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29830, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29829, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29822, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29834, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29819, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29874, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29920, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29982, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29838, &Heroic_Gundrak::Create);
	mgr->register_creature_script(29836, &Heroic_Gundrak::Create);
	//end Gun'drak
	//Start The Nexus
	mgr->register_creature_script(26800, &Heroic_Nexus::Create);
	mgr->register_creature_script(26799, &Heroic_Nexus::Create);
	mgr->register_creature_script(26802, &Heroic_Nexus::Create);
	mgr->register_creature_script(26801, &Heroic_Nexus::Create);
	mgr->register_creature_script(26805, &Heroic_Nexus::Create);
	mgr->register_creature_script(26803, &Heroic_Nexus::Create);
	mgr->register_creature_script(26734, &Heroic_Nexus::Create);
	mgr->register_creature_script(26722, &Heroic_Nexus::Create);
	mgr->register_creature_script(26735, &Heroic_Nexus::Create);
	mgr->register_creature_script(26716, &Heroic_Nexus::Create);
	mgr->register_creature_script(26727, &Heroic_Nexus::Create);
	mgr->register_creature_script(26728, &Heroic_Nexus::Create);
	mgr->register_creature_script(26729, &Heroic_Nexus::Create);
	mgr->register_creature_script(26730, &Heroic_Nexus::Create);
	mgr->register_creature_script(26793, &Heroic_Nexus::Create);
	mgr->register_creature_script(26782, &Heroic_Nexus::Create);
	mgr->register_creature_script(26792, &Heroic_Nexus::Create);
	mgr->register_creature_script(28231, &Heroic_Nexus::Create);
	mgr->register_creature_script(26918, &Heroic_Nexus::Create);
	mgr->register_creature_script(26737, &Heroic_Nexus::Create);
	mgr->register_creature_script(26746, &Heroic_Nexus::Create);
	//End The Nexus
	// Start Pit Of Saron
	mgr->register_creature_script(36788, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(36881, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(36830, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(37712, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(37713, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(37711, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(31260, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(36886, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(36879, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(36893, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(36892, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(36840, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(36841, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(36842, &Heroic_PitofSaron::Create);
	mgr->register_creature_script(36877, &Heroic_PitofSaron::Create);
	// End Pit of Saron
	//Start Ahn'Kahet
	mgr->register_creature_script(30338, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30114, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30278, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30418, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30285, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30277, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30414, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30329, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30319, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30286, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30279, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30287, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30416, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30179, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30284, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30276, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30419, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30111, &Heroic_Ahnkahet::Create);
	mgr->register_creature_script(30283, &Heroic_Ahnkahet::Create);
	//End Ahn'Kahet
	//Start Violet Hold
	mgr->register_creature_script(30893, &Violet1::Create);
	mgr->register_creature_script(30660, &Violet2::Create);
	mgr->register_creature_script(32191, &Violet3::Create);
	mgr->register_creature_script(30668, &Violet4::Create);
	mgr->register_creature_script(30666, &Violet5::Create);
	//End Violet Hold
	//Start Drak'tharon Keep
	mgr->register_creature_script(26636, &RisenDrakkariSoulMage::Create);
	mgr->register_creature_script(26635, &RisenDrakkariSoulWarrior::Create);
	mgr->register_creature_script(26626, &ScourgeReanimator::Create);
	mgr->register_creature_script(27871, &FleshGhoul::Create);
	mgr->register_creature_script(26621, &GhoulTorment::Create);
	mgr->register_creature_script(26624, &Belcher::Create);
	mgr->register_creature_script(26674, &DarkwebHatch::Create);
	mgr->register_creature_script(26625, &DarkwebRecluse::Create);
	mgr->register_creature_script(26620, &DrakkariGuardian::Create);
	mgr->register_creature_script(26639, &DrakkariShaman::Create);
	mgr->register_creature_script(26627, &CrystalHandler::Create);
	mgr->register_creature_script(27598, &FetidCorpse::Create);
	mgr->register_creature_script(27597, &HulkingCorpse::Create);
	mgr->register_creature_script(27600, &RisenShadowCaster::Create);
	mgr->register_creature_script(26622, &DrakkariBat::Create);
	mgr->register_creature_script(26638, &DrakkariBatRider::Create);
	mgr->register_creature_script(26641, &DrakkariGutripper::Create);
	mgr->register_creature_script(26628, &DrakkariScytheclaw::Create);
	mgr->register_creature_script(27753, &DrakkariInvader::Create);
	mgr->register_creature_script(26824, &DrakkariMount::Create);
	mgr->register_creature_script(26637, &DrakkariHandler::Create);
	mgr->register_creature_script(27431, &DrakkariCommander::Create);
	mgr->register_creature_script(26830, &DrakkariDK::Create);
	mgr->register_creature_script(26623, &ScourgeBrute::Create);
	//End Drak'tharon Keep
	//Start Utgarde Pinnacle
	mgr->register_creature_script(26550, &Heroic_UtgardePinnacle::Create);
	mgr->register_creature_script(26553, &Heroic_UtgardePinnacle::Create);
	mgr->register_creature_script(26554, &Heroic_UtgardePinnacle::Create);
	mgr->register_creature_script(26536, &Heroic_UtgardePinnacle::Create);
	mgr->register_creature_script(26555, &Heroic_UtgardePinnacle::Create);
	mgr->register_creature_script(26672, &Heroic_UtgardePinnacle::Create);
	mgr->register_creature_script(26670, &Heroic_UtgardePinnacle::Create);
	mgr->register_creature_script(26669, &Heroic_UtgardePinnacle::Create);
	mgr->register_creature_script(26696, &Heroic_UtgardePinnacle::Create);
	mgr->register_creature_script(26694, &Heroic_UtgardePinnacle::Create);
	mgr->register_creature_script(28368, &Heroic_UtgardePinnacle::Create);
	//End Utgarde Pinnacle
	//Start <<Halls of Stone>>
	mgr->register_creature_script(27973, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27966, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27962, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27969, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27964, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27965, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27963, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27960, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27961, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(28149, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27972, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27970, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27971, &Heroic_HallsofStone::Create);
	mgr->register_creature_script(27985, &Heroic_HallsofStone::Create);
	//End <<Halls of Stone>>
	// Start <<Forge of Souls>>
	mgr->register_creature_script(36478, &WatchmanAI::Create);
	mgr->register_creature_script(36620, &AdeptAI::Create);
	mgr->register_creature_script(36516, &AnimatorAI::Create);
	mgr->register_creature_script(36564, &BoneAI::Create);
	mgr->register_creature_script(36499, &ReaperAI::Create);
	mgr->register_creature_script(36522, &HorrorAI::Create);
	mgr->register_creature_script(36666, &WardenAI::Create);
	// End <<Forge of Souls>>
}