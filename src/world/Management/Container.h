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

#ifndef CONTAINER_H
#define CONTAINER_H

#include "Management/Item.h"

struct WoWContainer;
class SERVER_DECL Container : public Item
{
    // MIT Start

    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
    const WoWContainer* containerData() const { return reinterpret_cast<WoWContainer*>(wow_data); }
public:

    uint32_t getSlotCount() const;
    void setSlotCount(uint32_t num);

    //\todo not used. is it really uint64_t (guid) or is it another value we want to send to the client?
    uint64_t getSlot(uint16_t slot) const;
    void setSlot(uint16_t slot, uint64_t guid);

    // MIT End

        friend class WorldSession;

        Container(uint32 high, uint32 low);
        ~Container();

        void Create(uint32 itemid, Player* owner);
        void LoadFromDB(Field* fields);

        bool AddItem(int16 slot, Item* item);
        bool AddItemToFreeSlot(Item* pItem, uint32* r_slot);
    Item* GetItem(int16 slot);

        int8 FindFreeSlot();
        bool HasItems();

        void SwapItems(int8 SrcSlot, int8 DstSlot);
        Item* SafeRemoveAndRetreiveItemFromSlot(int16 slot, bool destroy);  /// doesn't destroy item from memory
        bool SafeFullRemoveItemFromSlot(int16 slot);                        /// destroys item fully

        void SaveBagToDB(int8 slot, bool first, QueryBuffer* buf);

protected:

        Item** m_Slot;
        uint32 __fields[CONTAINER_END];
};

#endif // CONTAINER_H
