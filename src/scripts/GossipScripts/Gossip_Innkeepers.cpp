/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/QuestMgr.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

enum
{
    SPELL_TRICK_OR_TREATED = 24755,
    SPELL_TREAT = 24715,
};

class InnkeeperGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* Plr) override;
    void onSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* Code, uint32_t gossipId) override;
    void destroy() override { delete this; }
};

void InnkeeperGossip::onHello(Object* pObject, Player* Plr)
{
    Creature* pCreature = pObject->isCreature() ? static_cast<Creature*>(pObject) : nullptr;
    if (pCreature == nullptr)
    {
        return;
    }

    uint32_t TextID = 820;
    uint32_t Text = sMySQLStore.getGossipTextIdForNpc(pCreature->getEntry());
    if (Text != 0)
    {
        MySQLStructure::NpcGossipText const* text = sMySQLStore.getNpcGossipText(Text);
        if (text != nullptr)
        {
            TextID = Text;
        }
    }
    GossipMenu menu(pCreature->getGuid(), TextID, 0);

    // Halow's End started?
    auto _now = std::chrono::system_clock::now();
    auto _time_now = std::chrono::system_clock::to_time_t(_now);
    tm * ct = std::localtime(&_time_now);
    if (ct->tm_mon == 9 && (ct->tm_mday > 17 && ct->tm_mday <= 31))
    {
        if (!Plr->hasAurasWithId(SPELL_TRICK_OR_TREATED))
        {
            menu.addItem(GOSSIP_ICON_CHAT, GI_TRICK_TREAT, 4);
        }
    }

    if (pCreature->isVendor())
    {
        menu.addItem(GOSSIP_ICON_VENDOR, VENDOR, 1);
    }

    menu.addItem(GOSSIP_ICON_CHAT, INNKEEPER, 2);
    menu.addItem(GOSSIP_ICON_CHAT, INNKEEPERASK, 3);
    sQuestMgr.FillQuestMenu(pCreature, Plr, menu);
    menu.sendGossipPacket(Plr);
}

void InnkeeperGossip::onSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/)
{
    Creature* pCreature = pObject->isCreature() ? static_cast<Creature*>(pObject) : nullptr;

    if (pCreature == nullptr)
        return;

    switch (Id)
    {
        case 1: // Vendor
        {
            Plr->getSession()->sendInventoryList(pCreature);
        } break;
        case 2: // Binder
        {
            Plr->getSession()->sendInnkeeperBind(pCreature);
        } break;
        case 3: // What can i do?
        {
            // Prepare second menu
            GossipMenu::sendQuickMenu(pCreature->getGuid(), 1853, Plr, 2, GOSSIP_ICON_CHAT, Plr->getSession()->LocalizedGossipOption(INNKEEPER));
        } break;
        case 4: // Event of halloween
        {
            if (!Plr->hasAurasWithId(SPELL_TRICK_OR_TREATED))
            {
                pCreature->castSpell(Plr, SPELL_TRICK_OR_TREATED, true);

                // either trick or treat, 50% chance
                if (Util::getRandomUInt(1))
                {
                    Plr->castSpell(Plr, SPELL_TREAT, true);
                }
                else
                {
                    int32_t trickspell = 0;
                    switch (Util::getRandomUInt(5))
                    {
                        case 0:
                        {
                            trickspell = 24753;                     // cannot cast, random 30sec
                        } break;
                        case 1:
                        {
                            trickspell = 24713;                     // lepper gnome costume
                        } break;
                        case 2:
                        {
                            if (Plr->getGender() == 0)
                            {
                                trickspell = 24735;                 // male ghost costume
                            }
                            else
                            {
                                trickspell = 24736;                 // female ghostcostume
                            }
                        } break;
                        case 3:
                        {
                            if (Plr->getGender() == 0)
                            {
                                trickspell = 24711;                 // male ninja costume
                            }
                            else
                            {
                                trickspell = 24710;                 // female ninja costume
                            }
                        } break;
                        case 4:
                        {
                            if (Plr->getGender() == 0)
                            {
                                trickspell = 24708;                 // male pirate costume
                            }
                            else
                            {
                                trickspell = 24709;                 // female pirate costume
                            }
                        } break;
                        case 5:
                        {
                            trickspell = 24723;                     // skeleton costume
                        } break;
                        default:
                        {
                            return;
                        }
                    }
                    pCreature->castSpell(Plr, trickspell, true);
                }
            }
            GossipMenu::senGossipComplete(Plr);
        } break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Innkeeper List 
void SetupInnkeepers(ScriptMgr* mgr)
{
    GossipScript* gs = new InnkeeperGossip();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Eastern Kingdoms
    mgr->register_creature_gossip(9501, gs);      // Innkeeper Adegwa
    mgr->register_creature_gossip(6740, gs);      // Innkeeper Allison
    mgr->register_creature_gossip(6739, gs);      // Innkeeper Bates
    mgr->register_creature_gossip(6727, gs);      // Innkeeper Brianna
    mgr->register_creature_gossip(1247, gs);      // Innkeeper Belm
    mgr->register_creature_gossip(295, gs);       // Innkeeper Farley
    mgr->register_creature_gossip(5111, gs);      // Innkeeper Firebrew
    mgr->register_creature_gossip(6734, gs);      // Innkeeper Hearthstove
    mgr->register_creature_gossip(8931, gs);      // Innkeeper Heather
    mgr->register_creature_gossip(1464, gs);      // Innkeeper Helbrek
    mgr->register_creature_gossip(17630, gs);     // Innkeeper Jovia
    mgr->register_creature_gossip(16542, gs);     // Innkeeper Kalarin
    mgr->register_creature_gossip(6930, gs);      // Innkeeper Karakul
    mgr->register_creature_gossip(6741, gs);      // Innkeeper Norman
    mgr->register_creature_gossip(5688, gs);      // Innkeeper Renee
    mgr->register_creature_gossip(2388, gs);      // Innkeeper Shay
    mgr->register_creature_gossip(9356, gs);      // Innkeeper Shul'kar
    mgr->register_creature_gossip(6807, gs);      // Innkeeper Skindle
    mgr->register_creature_gossip(5814, gs);      // Innkeeper Thulbek
    mgr->register_creature_gossip(7744, gs);      // Innkeeper Thulfram
    mgr->register_creature_gossip(6790, gs);      // Innkeeper Trelayne
    mgr->register_creature_gossip(16618, gs);     // Innkeeper Velandra
    mgr->register_creature_gossip(16256, gs);     // Jessica Chambers
    mgr->register_creature_gossip(15397, gs);     // Marniel Amberlight
    mgr->register_creature_gossip(25036, gs);     // Caregiver Inaara
    mgr->register_creature_gossip(44237, gs);     // Maegan Tillman <Innkeeper>
    mgr->register_creature_gossip(44235, gs);     // Thaegra Tillstone <Innkeeper>
    mgr->register_creature_gossip(3628, gs);      // Steven Lohan <Innkeeper>
    mgr->register_creature_gossip(38791, gs);     // Willa Arnes <Innkeeper>
    mgr->register_creature_gossip(1156, gs);      // Vyrin Swiftwind <Innkeeper>
    mgr->register_creature_gossip(44006, gs);     // Innkeeper Daughny <Innkeeper>
    mgr->register_creature_gossip(43993, gs);     // Innkeeper Larisal <Innkeeper>
    mgr->register_creature_gossip(2808, gs);      // Vikki Lonsav <Innkeeper>
    mgr->register_creature_gossip(44019, gs);     // Livingston Marshal <Innkeeper>
    mgr->register_creature_gossip(43699, gs);     // Innkeeper Keirnan <Innkeeper>
    mgr->register_creature_gossip(46269, gs);     // Mother Matterly <Innkeeper>
    mgr->register_creature_gossip(48093, gs);     // Ivan Zypher <Innkeeper>
    mgr->register_creature_gossip(47367, gs);     // Verad <Innkeeper>
    mgr->register_creature_gossip(44325, gs);     // Mama Morton <Innkeeper>
    mgr->register_creature_gossip(44334, gs);     // Donna Berrymore <Innkeeper>
    mgr->register_creature_gossip(42873, gs);     // Anissa Matherly <Innkeeper>
    mgr->register_creature_gossip(49688, gs);     // Innkeeper Francis <Innkeeper>
    mgr->register_creature_gossip(49686, gs);     // Innkeeper Teresa <Innkeeper>
    mgr->register_creature_gossip(49795, gs);     // Innkeeper Corlin <Innkeeper>
    mgr->register_creature_gossip(49591, gs);     // Naveen Tendernose <Innkeeper>
    mgr->register_creature_gossip(49574, gs);     // Vaughn Blusterbeard <Innkeeper>
    mgr->register_creature_gossip(49599, gs);     // Ben Mora <Innkeeper>
    mgr->register_creature_gossip(46271, gs);     // Provisioner Elda <Innkeeper>
    mgr->register_creature_gossip(45496, gs);     // Commander Hickley <Innkeeper>
    mgr->register_creature_gossip(49394, gs);     // Innkeeper Hershberg <Innkeeper>
    mgr->register_creature_gossip(39878, gs);     // Caretaker Movra <The Earthen Ring>
    mgr->register_creature_gossip(49430, gs);     // Innkeeper Durgens <Innkeeper>
    mgr->register_creature_gossip(44190, gs);     // Innkeeper Draxle <Innkeeper>
    mgr->register_creature_gossip(43739, gs);     // Bitsy <Innkeeper>
    mgr->register_creature_gossip(47857, gs);     // Roman Garner <Innkeeper>
    mgr->register_creature_gossip(44309, gs);     // Innkeeper Grak <Innkeeper>
    mgr->register_creature_gossip(42908, gs);     // Zun'ja <Innkeeper>
    mgr->register_creature_gossip(43141, gs);     // Innkeeper Nerius <Innkeeper>
    mgr->register_creature_gossip(49498, gs);     // Innkeeper Lutz <Innkeeper>
    mgr->register_creature_gossip(49783, gs);     // Innkeeper Geno <Innkeeper>
    mgr->register_creature_gossip(49762, gs);     // Innkeeper Turk <Innkeeper>
    mgr->register_creature_gossip(49747, gs);     // Innkeeper Krum <Innkeeper>
    mgr->register_creature_gossip(49934, gs);     // The Great Pisani <Mayor of Fuselight-by-the-Sea>
    mgr->register_creature_gossip(48054, gs);     // Sally Gearwell <Innkeeper>
    mgr->register_creature_gossip(47942, gs);     // Velma Rockslide <Innkeeper>
    mgr->register_creature_gossip(47334, gs);     // Cap'n Geech <Innkeeper>
    mgr->register_creature_gossip(2352, gs);      // Innkeeper Anderson
    mgr->register_creature_gossip(15433, gs);     // Innkeeper Delaniel
    mgr->register_creature_gossip(6737, gs);      // Innkeeper Shaussiy
    mgr->register_creature_gossip(7736, gs);      // Innkeeper Shyria
    mgr->register_creature_gossip(14731, gs);     // Lard
    mgr->register_creature_gossip(6778, gs);      // Melika Isenstrider
    mgr->register_creature_gossip(6806, gs);      // Tannok Frosthammer

    //////////////////////////////////////////////////////////////////////////////////////////
    // Kalimdor
    mgr->register_creature_gossip(16553, gs);     // Caregiver Chellan
    mgr->register_creature_gossip(16739, gs);     // Caregiver Breel
    mgr->register_creature_gossip(6736, gs);      // Innkeeper Keldamyr
    mgr->register_creature_gossip(6735, gs);      // Innkeeper Saelienne
    mgr->register_creature_gossip(17553, gs);     // Caregiver Topher Loaal
    mgr->register_creature_gossip(43420, gs);     // Innkeeper Kyteran <Innkeeper>
    mgr->register_creature_gossip(40898, gs);     // Alithia Fallowmere <Innkeeper>
    mgr->register_creature_gossip(41286, gs);     // Lyanath <Innkeeper> 
    mgr->register_creature_gossip(41491, gs);     // Valos Shadowrest <Innkeeper>
    mgr->register_creature_gossip(44177, gs);     // Innkeeper Bernice <Innkeeper>
    mgr->register_creature_gossip(6738, gs);      // Innkeeper Kimlya
    mgr->register_creature_gossip(11103, gs);     // Innkeeper Lyshaerya
    mgr->register_creature_gossip(44219, gs);     // Logistics Officer Renaldo <Innkeeper>
    mgr->register_creature_gossip(44268, gs);     // Keep Watcher Kerry <Innkeeper>
    mgr->register_creature_gossip(44267, gs);     // Logistics Officer Salista <Innkeeper>
    mgr->register_creature_gossip(6272, gs);      // Innkeeper Janene
    mgr->register_creature_gossip(40968, gs);     // Andoril <Innkeeper>
    mgr->register_creature_gossip(44391, gs);     // Innkeeper Shyria <Innkeeper>
    mgr->register_creature_gossip(47931, gs);     // Denmother Ulrica <Innkeeper>
    mgr->register_creature_gossip(6928, gs);      // Innkeeper Grosk
    mgr->register_creature_gossip(46642, gs);     // Innkeeper Nufa <Innkeeper>
    mgr->register_creature_gossip(44785, gs);     // Miwana <Innkeeper>
    mgr->register_creature_gossip(45563, gs);     // Tinza Silvermug <Innkeeper>
    mgr->register_creature_gossip(45086, gs);     // Sijambi <Innkeeper>
    mgr->register_creature_gossip(6747, gs);      // Innkeeper Kauth
    mgr->register_creature_gossip(6746, gs);      // Innkeeper Pala
    mgr->register_creature_gossip(43946, gs);     // Innkeeper Kerntis <Innkeeper>
    mgr->register_creature_gossip(3934, gs);      // Innkeeper Boorand Plainswind
    mgr->register_creature_gossip(43945, gs);     // Innkeeper Kritzle <Innkeeper>
    mgr->register_creature_gossip(43771, gs);     // Mixi <Innkeeper>
    mgr->register_creature_gossip(12196, gs);     // Innkeeper Kaylisk
    mgr->register_creature_gossip(43633, gs);     // Innkeeper Chin'toka <Innkeeper>
    mgr->register_creature_gossip(43606, gs);     // Innkeeper Duras <Innkeeper>
    mgr->register_creature_gossip(43624, gs);     // Innkeeper Linkasa <Innkeeper>
    mgr->register_creature_gossip(41892, gs);     // Felonius Stark <Innkeeper>
    mgr->register_creature_gossip(7731, gs);      // Innkeeper Jayka
    mgr->register_creature_gossip(11106, gs);     // Innkeeper Sikewa
    mgr->register_creature_gossip(44270, gs);     // Innkeeper Hurnahet <Innkeeper>
    mgr->register_creature_gossip(44276, gs);     // Innkeeper Lhakadd <Innkeeper>
    mgr->register_creature_gossip(24208, gs);     // "Little" Logok <Innkeeper>
    mgr->register_creature_gossip(40467, gs);     // Adene Treetotem <Innkeeper>
    mgr->register_creature_gossip(44376, gs);     // Chonk <Innkeeper>
    mgr->register_creature_gossip(7737, gs);      // Innkeeper Greul
    mgr->register_creature_gossip(6791, gs);      // Innkeeper Wiley
    mgr->register_creature_gossip(43872, gs);     // Innkeeper Dessina <Innkeeper>
    mgr->register_creature_gossip(23995, gs);     // Axle <Innkeeper>
    mgr->register_creature_gossip(4507,  gs);     // Daisy <Race Starter Girl>
    mgr->register_creature_gossip(48599, gs);     // Innkeeper Teenycaugh <Innkeeper>
    mgr->register_creature_gossip(48215, gs);     // Innkeeper Wylaria <Innkeeper>
    mgr->register_creature_gossip(7733, gs);      // Innkeeper Fizzgrimble
    mgr->register_creature_gossip(38714, gs);     // Carmen Ibanozzle <Innkeeper>
    mgr->register_creature_gossip(38488, gs);     // Innkeeper Dreedle <Marshal Expeditions>
    mgr->register_creature_gossip(11118, gs);     // Innkeeper Vizzie   
    mgr->register_creature_gossip(15174, gs);     // Calandrath
    mgr->register_creature_gossip(40843, gs);     // Sebelia <Innkeeper>
    mgr->register_creature_gossip(43487, gs);     // Isara Riverstride <Innkeeper>
    mgr->register_creature_gossip(43378, gs);     // Salirn Moonbear <Innkeeper>
    mgr->register_creature_gossip(49528, gs);     // Arcane Guest Registry
    mgr->register_creature_gossip(49406, gs);     // Yasmin <Innkeeper>
    mgr->register_creature_gossip(11116, gs);     // Innkeeper Abeqwa
    mgr->register_creature_gossip(7714, gs);      // Innkeeper Byula
    mgr->register_creature_gossip(16458, gs);     // Innkeeper Faralia
    mgr->register_creature_gossip(6929, gs);      // Innkeeper Gryshka

    //////////////////////////////////////////////////////////////////////////////////////////
    // Outland
    mgr->register_creature_gossip(16826, gs);     // Sid Limbardi
    mgr->register_creature_gossip(18906, gs);     // Caregiver Ophera Windfury
    mgr->register_creature_gossip(18251, gs);     // Caregiver Abidaar
    mgr->register_creature_gossip(18908, gs);     // Innkeeper Kerp
    mgr->register_creature_gossip(19296, gs);     // Innkeeper Biribi
    mgr->register_creature_gossip(18914, gs);     // Caregiver Isel
    mgr->register_creature_gossip(19495, gs);     // Innkeeper Shaunessy
    mgr->register_creature_gossip(21110, gs);     // Fizit "Doc" Clocktock
    mgr->register_creature_gossip(19352, gs);     // Dreg Cloudsweeper
    mgr->register_creature_gossip(16602, gs);     // Floyd Pinkus
    mgr->register_creature_gossip(18905, gs);     // Innkeeper Bazil Olof'tazun
    mgr->register_creature_gossip(18245, gs);     // Merajit
    mgr->register_creature_gossip(18957, gs);     // Innkeeper Grilka
    mgr->register_creature_gossip(18913, gs);     // Matron Tikkit
    mgr->register_creature_gossip(19470, gs);     // Gholah
    mgr->register_creature_gossip(21088, gs);     // Matron Varah
    mgr->register_creature_gossip(19319, gs);     // Innkeeper Darg Bloodclaw
    mgr->register_creature_gossip(18907, gs);     // Innkeeper Coryth Stoktron
    mgr->register_creature_gossip(19046, gs);     // Minalei
    mgr->register_creature_gossip(19232, gs);     // Innkeeper Haelthol
    mgr->register_creature_gossip(22922, gs);     // Innkeeper Aelerya
    mgr->register_creature_gossip(19571, gs);     // Innkeeper Remi Dodoso
    mgr->register_creature_gossip(19531, gs);     // Eyonix
    mgr->register_creature_gossip(21746, gs);     // Caretaker Aluuro
    mgr->register_creature_gossip(21744, gs);     // Roldemar
    mgr->register_creature_gossip(23143, gs);     // Horus <Innkeeper>

    //////////////////////////////////////////////////////////////////////////////////////////
    // Northrend
    mgr->register_creature_gossip(25245, gs);     // James Deacon <Innkeeper>
    mgr->register_creature_gossip(26596, gs);     // "Charlie" Northtop <Innkeeper>
    mgr->register_creature_gossip(23731, gs);     // Innkeeper Hazel Lagras <Innkeeper>
    mgr->register_creature_gossip(23937, gs);     // Innkeeper Celeste Goodhutch <Innkeeper>
    mgr->register_creature_gossip(24057, gs);     // Christina Daniels <Innkeeper>
    mgr->register_creature_gossip(27042, gs);     // Illusia Lune <Innkeeper>
    mgr->register_creature_gossip(27052, gs);     // Naohain <Innkeeper>
    mgr->register_creature_gossip(27066, gs);     // Jennifer Bell <Innkeeper>
    mgr->register_creature_gossip(26375, gs);     // Quartermaster McCarty <Innkeeper>
    mgr->register_creature_gossip(28686, gs);     // Caliel Brightwillow <Assistant Innkeeper>
    mgr->register_creature_gossip(26709, gs);     // Pahu Frosthoof <Innkeeper>
    mgr->register_creature_gossip(27069, gs);     // Matron Magah <Innkeeper>
    mgr->register_creature_gossip(24342, gs);     // Timothy Holland <Innkeeper>
    mgr->register_creature_gossip(24149, gs);     // Basil Osgood <Innkeeper>
    mgr->register_creature_gossip(24033, gs);     // Bori Wintertotem <Innkeeper>
    mgr->register_creature_gossip(27027, gs);     // Mrs. Winterby <Innkeeper>
    mgr->register_creature_gossip(26985, gs);     // Barracks Master Harga <Innkeeper>
    mgr->register_creature_gossip(27125, gs);     // Barracks Master Rhekku <Innkeeper>
    mgr->register_creature_gossip(26680, gs);     // Aiyan Coldwind <Innkeeper>
    mgr->register_creature_gossip(32418, gs);     // Abohba <Assistant Innkeeper>
    mgr->register_creature_gossip(29971, gs);     // Wabada Whiteflower <Innkeeper>
    mgr->register_creature_gossip(29944, gs);     // Peon Gakra <Innkeeper & Supplies>
    mgr->register_creature_gossip(33971, gs);     // Jarin Dawnglow <Innkeeper>
    mgr->register_creature_gossip(27187, gs);     // Caregiver Poallu <Innkeeper>
    mgr->register_creature_gossip(27148, gs);     // Caregiver Iqniq <Innkeeper>
    mgr->register_creature_gossip(27174, gs);     // Caregiver Mumik <Innkeeper>
    mgr->register_creature_gossip(27950, gs);     // Demestrasz <Innkeeper>
    mgr->register_creature_gossip(28687, gs);     // Amisi Azuregaze <Innkeeper>
    mgr->register_creature_gossip(32411, gs);     // Afsaneh Asrar <Assistant Innkeeper>
    mgr->register_creature_gossip(29532, gs);     // Ajay Green <Innkeeper>
    mgr->register_creature_gossip(28791, gs);     // Marissa Everwatch <Innkeeper>
    mgr->register_creature_gossip(29583, gs);     // Pan'ya <Innkeeper>
    mgr->register_creature_gossip(28038, gs);     // Purser Boulian <Innkeeper>
    mgr->register_creature_gossip(29904, gs);     // Smilin' Slirk Brassknob <Innkeeper>
    mgr->register_creature_gossip(30005, gs);     // Lodge-Matron Embla <Innkeeper>
    mgr->register_creature_gossip(29963, gs);     // Magorn <Innkeeper>
    mgr->register_creature_gossip(30308, gs);     // Initiate Brenners <Innkeeper>

    //////////////////////////////////////////////////////////////////////////////////////////
    // The Maelstorm
    mgr->register_creature_gossip(57619, gs);     // Cheng Dawnscrive <Temple Scholar>
    mgr->register_creature_gossip(65046, gs);     // Lao Ma Liang <Innkeeper>
    mgr->register_creature_gossip(98945, gs);     // Lao Shu <Innkeeper>

    //////////////////////////////////////////////////////////////////////////////////////////
    // Pandaria
    mgr->register_creature_gossip(65907, gs);     // Jiayi Applebloom <Innkeeper>
    mgr->register_creature_gossip(61599, gs);     // Cheerful Jessu <Innkeeper>
    mgr->register_creature_gossip(69088, gs);     // Keeper Jaril <Innkeeper>
    mgr->register_creature_gossip(62882, gs);     // Kai the Restless <Innkeeper>
    mgr->register_creature_gossip(64149, gs);     // Matron Vi Vinh <Innkeeper>
    mgr->register_creature_gossip(66236, gs);     // Brewmother Kiki <Innkeeper>
    mgr->register_creature_gossip(62975, gs);     // Grookin Bed-Haver
    mgr->register_creature_gossip(58184, gs);     // Malaya Dawnchaser <Innkeeper>
    mgr->register_creature_gossip(62967, gs);     // Aizra Dawnchaser <Innkeeper>
    mgr->register_creature_gossip(62883, gs);     // Mai the Sleepy <Innkeeper>
    mgr->register_creature_gossip(62996, gs);     // Madam Vee Luo <Innkeeper>
    mgr->register_creature_gossip(63008, gs);     // Brewmaster Skye <Innkeeper>
    mgr->register_creature_gossip(55809, gs);     // Peiji Goldendraft <Brewmaster>
    mgr->register_creature_gossip(55233, gs);     // Lin Windfur <Innkeeper>
    mgr->register_creature_gossip(57313, gs);     // Fela Woodear <Innkeeper>
    mgr->register_creature_gossip(62322, gs);     // Graceful Swan <Innkeeper>
    mgr->register_creature_gossip(62867, gs);     // Bolo the Elder <Innkeeper and Kegkeeper>
    mgr->register_creature_gossip(62868, gs);     // Lana the Sea Breeze <Innkeeper>
    mgr->register_creature_gossip(65528, gs);     // Nan Thunderfoot <Innkeeper>
    mgr->register_creature_gossip(59582, gs);     // Innkeeper Lei Lan
    mgr->register_creature_gossip(62878, gs);     // Nan the Mason Mug <Innkeeper>
    mgr->register_creature_gossip(70585, gs);     // Farmer's Journal
    mgr->register_creature_gossip(62879, gs);     // Rude Sho <Innkeeper>
    mgr->register_creature_gossip(62869, gs);     // Ni the Merciful <Innkeeper>
    mgr->register_creature_gossip(62872, gs);     // Cranfur the Noodler <Innkeeper>
    mgr->register_creature_gossip(62871, gs);     // Puli the Even Handed <Innkeeper>
    mgr->register_creature_gossip(59405, gs);     // Li Goldendraft <Innkeeper>
    mgr->register_creature_gossip(61651, gs);     // Master Lao
    mgr->register_creature_gossip(59688, gs);     // Chiyo Mistpaw <Innkeeper>
    mgr->register_creature_gossip(60420, gs);     // Clover Keeper <Innkeeper>
    mgr->register_creature_gossip(62877, gs);     // Stained Mug <Innkeeper>
    mgr->register_creature_gossip(60605, gs);     // Liu Ze <Innkeeper>
    mgr->register_creature_gossip(62873, gs);     // Saito the Sleeping Shadow <Innkeeper>
    mgr->register_creature_gossip(62875, gs);     // Kim the Quiet <Innkeeper>
    mgr->register_creature_gossip(62874, gs);     // Kali the Night Watcher <Innkeeper>
    mgr->register_creature_gossip(65220, gs);     // Zit'tix <Innkeeper>
    mgr->register_creature_gossip(63016, gs);     // San the Sea Calmer <Innkeeper>
    mgr->register_creature_gossip(58691, gs);     // Bartender Tomro <Innkeeper>
}
