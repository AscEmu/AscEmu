/*
 * AscScripts for AscEmu Framework
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INSTANCE_AZJOL_NERUB_H
#define _INSTANCE_AZJOL_NERUB_H

enum CreatureEntry
{
    //Krikthir The Gatewatcher
    BOSS_KRIKTHIR       = 28684,

    //Hadronox
    BOSS_HADRONOX       = 28921,

    //Watcher Gashra
    CN_GASHRA           = 28730,

    //Watcher Narjil
    CN_NARJIL           = 28729,

    //Watcher Silthik
    CN_SILTHIK          = 28731,

    //Anub'ar Shadowcaster
    CN_ANUB_SHADOWCASTER = 28733

};

enum CreatureSpells
{
    //Krikthir The Gatewatcher
    KRIKTHIR_MINDFLAY           = 52586,
    KRIKTHIR_CURSEOFFATIGUE     = 52592,
    KRIKTHIR_ENRAGE             = 28747,    // Aura
    KRIKTHIR_MINDFLAY_HC        = 59367,    // Heroic
    KRIKTHIR_CURSEOFFATIGUE_HC  = 59368,    // Heroic

    //Hadronox
    HADRONOX_WEBGRAB            = 53406,
    HADRONOX_PIERCEARMOR        = 53418,
    HADRONOX_LEECHPOISON        = 53030,
    HADRONOX_ACIDCLOUD          = 53400,
    HADRONOX_WEBGRAB_HC         = 59421,    // Heroic
    HADRONOX_LEECHPOISON_HC     = 59417,    // Heroic
    HADRONOX_ACIDCLOUD_HC       = 59419,    // Heroic

    //Watcher Gashra.
    GASHRA_WEBWRAP              = 52086,
    GASHRA_INFECTEDBITE         = 52469,
    GASHRA_ENRAGE               = 52470,

    //Watcher Narjil
    NARJIL_WEBWRAP              = 52086,
    NARJIL_INFECTEDBITE         = 52469,
    NARJIL_BLINDINGWEBS         = 52524,

    //Watcher Silthik
    SILTHIK_WEBWRAP             = 52086,
    SILTHIK_INFECTEDBITE        = 52469,
    SILTHIK_POISONSPRAY         = 52493,

    //Anub'ar Shadowcaster
    SHADOWCASTER_SHADOWBOLT     = 52534,
    SHADOWCASTER_SHADOW_NOVA    = 52535
};

enum CreatureSay
{
};

#endif // _INSTANCE_AZJOL_NERUB_H
