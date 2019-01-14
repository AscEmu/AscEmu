/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CRITUAL_H
#define CRITUAL_H

#include <vector>

 //////////////////////////////////////////////////////////////////////////////////////////
 /// Class that implements Summoning Rituals
 //////////////////////////////////////////////////////////////////////////////////////////
class CRitual
{
    public:

        CRitual() {}

        CRitual(unsigned long MaxMembers) : Members(MaxMembers)
        {
            this->MaxMembers = MaxMembers;
            CurrentMembers = 0;
            CasterGUID = 0;
            TargetGUID = 0;
            SpellID = 0;
        }

        ~CRitual() {}

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Initializes the Ritual
        /// \param unsigned long CasterGUID - GUID of the caster Player
        /// \param unsigned long TargetGUID - GUID of the target Player
        /// \param unsigned long SpellID - ID of the channeled spell (visual)
        /// \returns none
        //////////////////////////////////////////////////////////////////////////////////////////
        void Setup(unsigned long CasterGUID, unsigned long TargetGUID, unsigned long SpellID);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Returns the GUID of the caster Player
        /// \param none
        /// \returns the GUID of the caster Player
        //////////////////////////////////////////////////////////////////////////////////////////
        unsigned long GetCasterGUID()
        {
            return CasterGUID;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Returns the GUID of the target Player
        /// \param none
        /// \returns the GUID of the target Player
        //////////////////////////////////////////////////////////////////////////////////////////
        unsigned long GetTargetGUID()
        {
            return TargetGUID;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Returns the ID of the channeled spell
        /// \param none
        /// \returns the ID of the channeled spell
        //////////////////////////////////////////////////////////////////////////////////////////
        unsigned long GetSpellID()
        {
            return SpellID;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Adds a member to the Ritual
        /// \param unsigned long GUID - GUID of the ritual member to add
        /// \returns true on success, false on failure.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool AddMember(unsigned long GUID);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Removes a member to the Ritual
        /// \param unsigned long GUID - GUID of the ritual member to remove
        /// \returns true on success, false on failure.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool RemoveMember(unsigned long GUID);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Checks if the Ritual has a member with this GUID
        /// \param unsigned long GUID - GUID to search for
        /// \returns true if a member with this GUID can be found and false otherwise
        //////////////////////////////////////////////////////////////////////////////////////////
        bool HasMember(unsigned long GUID);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Returns the GUID of the Ritual member in this slot
        /// \param unsigned long Slot - Slot ID
        /// \returns the GUID of the Ritual member in this slot
        //////////////////////////////////////////////////////////////////////////////////////////
        unsigned long GetMemberGUIDBySlot(unsigned long Slot)
        {
            return Members[Slot];
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells is the Ritual has free slots
        /// \param none
        /// \returns true if there are still free slots and false otherwise
        //////////////////////////////////////////////////////////////////////////////////////////
        bool HasFreeSlots()
        {
            if (CurrentMembers < MaxMembers)
                return true;
            else
                return false;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Returns the number of members this Ritual requires
        /// \param none
        /// \returns the number of members this Ritual requires
        //////////////////////////////////////////////////////////////////////////////////////////
        unsigned long GetMaxMembers()
        {
            return MaxMembers;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Finishes the ritual
        /// \param none           \returns none
        //////////////////////////////////////////////////////////////////////////////////////////
        void Finish()
        {
            SpellID = 0;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells if the ritual is finished
        /// \param none
        /// \returns true if the ritual is finished and false otherwise
        //////////////////////////////////////////////////////////////////////////////////////////
        bool IsFinished()
        {
            if (SpellID == 0)
                return true;
            else
                return false;
        }

    private:

        unsigned long CasterGUID;
        unsigned long TargetGUID;
        unsigned long SpellID;
        unsigned long CurrentMembers;
        unsigned long MaxMembers;

        std::vector<unsigned long> Members;
};

#endif // CRITUAL_H
