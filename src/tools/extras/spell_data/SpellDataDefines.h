#ifndef uint32
	typedef unsigned int uint32;
#endif
#ifndef int
	typedef int int32;
#endif

#ifndef ASSERT
	#define ASSERT assert
#endif

#define SERVER_DECL __declspec(dllexport)

#define SQL_INSERTS_PER_QUERY 1000

//this might change from 1 version to another of the DBC
#define SPELL_COLUMN_COUNT 234
//last column is "skip_this_for_sql"

const char sql_translation_table[SPELL_COLUMN_COUNT][3][300] = 
{
    { "uint32", "Id", "0" },     //0
    { "uint32", "Category", "0" },     //1
    { "uint32", "DispelType", "0" },     //2
    { "uint32", "MechanicsType", "0" },     //3
    { "uint32", "Attributes", "0" },     //4
    { "uint32", "AttributesEx", "0" },     //5
    { "uint32", "AttributesExB", "0" },     //6
    { "uint32", "AttributesExC", "0" },     //7
    { "uint32", "AttributesExD", "0" },     //8
    { "uint32", "AttributesExE", "0" },     //9
    { "uint32", "AttributesExF", "0" },     //10
    { "uint32", "AttributesExG", "0" },     //11
    { "uint32", "RequiredShapeShift", "0" },     //12
    { "uint32", "Unknown", "0" },     //13
    { "uint32", "ShapeshiftExclude", "0" },     //14
    { "uint32", "Unknown2", "0" },     //15
    { "uint32", "Targets", "0" },     //16
    { "uint32", "TargetCreatureType", "0" },     //17
    { "uint32", "RequiresSpellFocus", "0" },     //18
    { "uint32", "FacingCasterFlags", "0" },     //19
    { "uint32", "CasterAuraState", "0" },     //20
    { "uint32", "TargetAuraState", "0" },     //21
    { "uint32", "CasterAuraStateNot", "0" },     //22
    { "uint32", "TargetAuraStateNot", "0" },     //23
    { "uint32", "casterAuraSpell", "0" },     //24
    { "uint32", "targetAuraSpell", "0" },     //25
    { "uint32", "casterAuraSpellNot", "0" },     //26
    { "uint32", "targetAuraSpellNot", "0" },     //27
    { "uint32", "CastingTimeIndex", "0" },     //28
    { "uint32", "RecoveryTime", "0" },     //29
    { "uint32", "CategoryRecoveryTime", "0" },     //30
    { "uint32", "InterruptFlags", "0" },     //31
    { "uint32", "AuraInterruptFlags", "0" },     //32
    { "uint32", "ChannelInterruptFlags", "0" },     //33
    { "uint32", "procFlags", "0" },     //34
    { "uint32", "procChance", "0" },     //35
    { "uint32", "procges", "0" },     //36
    { "uint32", "maxLevel", "0" },     //37
    { "uint32", "baseLevel", "0" },     //38
    { "uint32", "spellLevel", "0" },     //39
    { "uint32", "DurationIndex", "0" },     //40
    { "int32", "powerType", "0" },     //41
    { "uint32", "manaCost", "0" },     //42
    { "uint32", "manaCostPerlevel", "0" },     //43
    { "uint32", "manaPerSecond", "0" },     //44
    { "uint32", "manaPerSecondPerLevel", "0" },     //45
    { "uint32", "rangeIndex", "0" },     //46
    { "float", "speed", "0" },     //47
    { "uint32", "modalNextSpell", "0" },     //48
    { "uint32", "maxstack", "0" },     //49
    { "uint32", "Totem_0", "0" },     //50
    { "uint32", "Totem_1", "0" },     //51
    { "uint32", "Reagent_0", "0" },     //52
    { "uint32", "Reagent_1", "0" },     //53
    { "uint32", "Reagent_2", "0" },     //54
    { "uint32", "Reagent_4", "0" },     //55
    { "uint32", "Reagent_5", "0" },     //56
    { "uint32", "Reagent_6", "0" },     //57
    { "uint32", "Reagent_7", "0" },     //58
    { "uint32", "Reagent_8", "0" },     //59
    { "uint32", "ReagentCount_0", "0" },     //60
    { "uint32", "ReagentCount_1", "0" },     //61
    { "uint32", "ReagentCount_2", "0" },     //62
    { "uint32", "ReagentCount_4", "0" },     //63
    { "uint32", "ReagentCount_5", "0" },     //64
    { "uint32", "ReagentCount_6", "0" },     //65
    { "uint32", "ReagentCount_7", "0" },     //66
    { "uint32", "ReagentCount_8", "0" },     //67
    { "int32", "EquippedItemClass", "0" },     //68
    { "uint32", "EquippedItemSubClass", "0" },     //69
    { "uint32", "RequiredItemFlags", "0" },     //70
    { "uint32", "Effect_0", "0" },     //71
    { "uint32", "Effect_1", "0" },     //72
    { "uint32", "Effect_2", "0" },     //73
    { "uint32", "EffectDieSides_0", "0" },     //74
    { "uint32", "EffectDieSides_1", "0" },     //75
    { "uint32", "EffectDieSides_2", "0" },     //76
    { "float", "EffectRealPointsPerLevel_0", "0" },     //77
    { "float", "EffectRealPointsPerLevel_1", "0" },     //78
    { "float", "EffectRealPointsPerLevel_2", "0" },     //79
    { "int32", "EffectBasePoints_0", "0" },     //80
    { "int32", "EffectBasePoints_1", "0" },     //81
    { "int32", "EffectBasePoints_2", "0" },     //82
    { "int32", "EffectMechanic_0", "0" },     //83
    { "int32", "EffectMechanic_1", "0" },     //84
    { "int32", "EffectMechanic_2", "0" },     //85
    { "uint32", "EffectImplicitTargetA_0", "0" },     //86
    { "uint32", "EffectImplicitTargetA_1", "0" },     //87
    { "uint32", "EffectImplicitTargetA_2", "0" },     //88
    { "uint32", "EffectImplicitTargetB_0", "0" },     //89
    { "uint32", "EffectImplicitTargetB_1", "0" },     //90
    { "uint32", "EffectImplicitTargetB_2", "0" },     //91
    { "uint32", "EffectRadiusIndex_0", "0" },     //92
    { "uint32", "EffectRadiusIndex_1", "0" },     //93
    { "uint32", "EffectRadiusIndex_2", "0" },     //94
    { "uint32", "EffectApplyAuraName_0", "0" },     //95
    { "uint32", "EffectApplyAuraName_1", "0" },     //96
    { "uint32", "EffectApplyAuraName_2", "0" },     //97
    { "uint32", "EffectAmplitude_0", "0" },     //98
    { "uint32", "EffectAmplitude_1", "0" },     //99
    { "uint32", "EffectAmplitude_2", "0" },     //100
    { "float", "EffectMultipleValue_0", "0" },     //101
    { "float", "EffectMultipleValue_1", "0" },     //102
    { "float", "EffectMultipleValue_2", "0" },     //103
    { "uint32", "EffectChainTarget_0", "0" },     //104
    { "uint32", "EffectChainTarget_1", "0" },     //105
    { "uint32", "EffectChainTarget_2", "0" },     //106
    { "uint32", "EffectItemType_0", "0" },     //107
    { "uint32", "EffectItemType_1", "0" },     //108
    { "uint32", "EffectItemType_2", "0" },     //109
    { "uint32", "EffectMiscValue_0", "0" },     //110
    { "uint32", "EffectMiscValue_1", "0" },     //111
    { "uint32", "EffectMiscValue_2", "0" },     //112
    { "uint32", "EffectMiscValueB_0", "0" },     //113
    { "uint32", "EffectMiscValueB_1", "0" },     //114
    { "uint32", "EffectMiscValueB_2", "0" },     //115
    { "uint32", "EffectTriggerSpell_0", "0" },     //116
    { "uint32", "EffectTriggerSpell_1", "0" },     //117
    { "uint32", "EffectTriggerSpell_2", "0" },     //118
    { "float", "EffectPointsPerComboPoint_0", "0" },     //119
    { "float", "EffectPointsPerComboPoint_1", "0" },     //120
    { "float", "EffectPointsPerComboPoint_2", "0" },     //121
    { "uint32", "EffectSpellClassMask_0_0", "0" },     //122
    { "uint32", "EffectSpellClassMask_0_1", "0" },     //123
    { "uint32", "EffectSpellClassMask_0_2", "0" },     //124
    { "uint32", "EffectSpellClassMask_1_0", "0" },     //125
    { "uint32", "EffectSpellClassMask_1_1", "0" },     //126
    { "uint32", "EffectSpellClassMask_1_2", "0" },     //127
    { "uint32", "EffectSpellClassMask_2_0", "0" },     //128
    { "uint32", "EffectSpellClassMask_2_1", "0" },     //129
    { "uint32", "EffectSpellClassMask_2_2", "0" },     //130
    { "uint32", "SpellVisual", "0" },     //131
    { "uint32", "field114", "0" },     //132
    { "uint32", "spellIconID", "0" },     //133
    { "uint32", "activeIconID", "0" },     //134
    { "uint32", "spellPriority", "0" },     //135
    { "str", "Name", "0" },     //136
    { "uint32", "NameAlt_1", "0" },     //137
    { "uint32", "NameAlt_2", "0" },     //138
    { "uint32", "NameAlt_3", "0" },     //139
    { "uint32", "NameAlt_4", "0" },     //140
    { "uint32", "NameAlt_5", "0" },     //141
    { "uint32", "NameAlt_6", "0" },     //142
    { "uint32", "NameAlt_7", "0" },     //143
    { "uint32", "NameAlt_8", "0" },     //144
    { "uint32", "NameAlt_9", "0" },     //145
    { "uint32", "NameAlt_10", "0" },     //146
    { "uint32", "NameAlt_11", "0" },     //147
    { "uint32", "NameAlt_12", "0" },     //148
    { "uint32", "NameAlt_13", "0" },     //149
    { "uint32", "NameAlt_14", "0" },     //150
    { "uint32", "NameAlt_15", "0" },     //151
    { "uint32", "NameFlags", "0" },     //152
    { "str", "Rank", "0" },     //153
    { "uint32", "RankAlt_1", "0" },     //154
    { "uint32", "RankAlt_2", "0" },     //155
    { "uint32", "RankAlt_3", "0" },     //156
    { "uint32", "RankAlt_4", "0" },     //157
    { "uint32", "RankAlt_5", "0" },     //158
    { "uint32", "RankAlt_6", "0" },     //159
    { "uint32", "RankAlt_7", "0" },     //160
    { "uint32", "RankAlt_8", "0" },     //161
    { "uint32", "RankAlt_9", "0" },     //162
    { "uint32", "RankAlt_10", "0" },     //163
    { "uint32", "RankAlt_11", "0" },     //164
    { "uint32", "RankAlt_12", "0" },     //165
    { "uint32", "RankAlt_13", "0" },     //166
    { "uint32", "RankAlt_14", "0" },     //167
    { "uint32", "RankAlt_15", "0" },     //168
    { "uint32", "RankFlags", "0" },     //169
    { "str", "Description", "0" },     //170
    { "uint32", "DescriptionAlt_1", "0" },     //171
    { "uint32", "DescriptionAlt_2", "0" },     //172
    { "uint32", "DescriptionAlt_3", "0" },     //173
    { "uint32", "DescriptionAlt_4", "0" },     //174
    { "uint32", "DescriptionAlt_5", "0" },     //175
    { "uint32", "DescriptionAlt_6", "0" },     //176
    { "uint32", "DescriptionAlt_7", "0" },     //177
    { "uint32", "DescriptionAlt_8", "0" },     //178
    { "uint32", "DescriptionAlt_9", "0" },     //179
    { "uint32", "DescriptionAlt_10", "0" },     //180
    { "uint32", "DescriptionAlt_11", "0" },     //181
    { "uint32", "DescriptionAlt_12", "0" },     //182
    { "uint32", "DescriptionAlt_13", "0" },     //183
    { "uint32", "DescriptionAlt_14", "0" },     //184
    { "uint32", "DescriptionAlt_15", "0" },     //185
    { "uint32", "DescriptionFlags", "0" },     //186
    { "str", "BuffDescription", "0" },     //187
    { "uint32", "BuffDescription_1", "0" },     //188
    { "uint32", "BuffDescription_2", "0" },     //189
    { "uint32", "BuffDescription_3", "0" },     //190
    { "uint32", "BuffDescription_4", "0" },     //191
    { "uint32", "BuffDescription_5", "0" },     //192
    { "uint32", "BuffDescription_6", "0" },     //193
    { "uint32", "BuffDescription_7", "0" },     //194
    { "uint32", "BuffDescription_8", "0" },     //195
    { "uint32", "BuffDescription_9", "0" },     //196
    { "uint32", "BuffDescription_10", "0" },     //197
    { "uint32", "BuffDescription_11", "0" },     //198
    { "uint32", "BuffDescription_12", "0" },     //199
    { "uint32", "BuffDescription_13", "0" },     //200
    { "uint32", "BuffDescription_14", "0" },     //201
    { "uint32", "BuffDescription_15", "0" },     //202
    { "uint32", "buffdescflags", "0" },     //203
    { "uint32", "ManaCostPercentage", "0" },     //204
    { "uint32", "StartRecoveryCategory", "0" },     //205
    { "uint32", "StartRecoveryTime", "0" },     //206
    { "uint32", "MaxTargetLevel", "0" },     //207
    { "uint32", "SpellFamilyName", "0" },     //208
    { "uint32", "SpellGroupType_0", "0" },     //209
    { "uint32", "SpellGroupType_1", "0" },     //210
    { "uint32", "SpellGroupType_2", "0" },     //211
    { "uint32", "MaxTargets", "0" },     //212
    { "uint32", "Spell_Dmg_Type", "0" },     //213
    { "uint32", "PreventionType", "0" },     //214
    { "int32", "StanceBarOrder", "0" },     //215
    { "float", "dmg_multiplier_0", "0" },     //216
    { "float", "dmg_multiplier_1", "0" },     //217
    { "float", "dmg_multiplier_2", "0" },     //218
    { "uint32", "MinFactionID", "0" },     //219
    { "uint32", "MinReputation", "0" },     //220
    { "uint32", "RequiredAuraVision", "0" },     //221
    { "uint32", "TotemCategory_0", "0" },     //222
    { "uint32", "TotemCategory_1", "0" },     //223
    { "int32", "RequiresAreaId", "0" },     //224
    { "uint32", "School", "0" },     //225
    { "uint32", "RuneCostID", "0" },     //226
    { "uint32", "SpellMissileID", "0" },     //227
    { "uint32", "PowerDisplayId", "0" },     //228
    { "float", "EffectBonusMultiplier_0", "0" },     //229
    { "float", "EffectBonusMultiplier_1", "0" },     //230
    { "float", "EffectBonusMultiplier_2", "0" },     //231
    { "uint32", "SpellDescriptionVariable", "0" },     //232
    { "uint32", "SpellDifficultyID", "0" },     //233
};
