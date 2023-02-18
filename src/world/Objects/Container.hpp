/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Item.hpp"
#include "Data/WoWContainer.hpp"

struct WoWContainer;

class SERVER_DECL Container : public Item
{
public:
    Container(uint32_t high, uint32_t low);
    ~Container();

    void create(uint32_t itemid, Player* owner);
    void loadFromDB(Field* fields);
    void saveToDB(int8_t slot, bool first, QueryBuffer* buf);

    bool addItem(int16_t slot, Item* item);
    bool addItemToFreeSlot(Item* item, uint32_t* r_slot);
    void forceCreationUpdate(Item* item);

    Item* getItem(int16_t slot);

    int8_t findFreeSlot();
    bool hasItems() const;

    void swapItems(int8_t SrcSlot, int8_t DstSlot);
    Item* safeRemoveAndRetreiveItemFromSlot(int16_t slot, bool destroy);
    bool safeFullRemoveItemFromSlot(int16_t slot);

    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
private:
    const WoWContainer* containerData() const { return reinterpret_cast<WoWContainer*>(wow_data); }

public:
    uint32_t getSlotCount() const;
    void setSlotCount(uint32_t num);

    uint64_t getSlot(uint16_t slot) const;
    void setSlot(uint16_t slot, uint64_t guid);

protected:
    Item** m_Slot = nullptr;
    uint32_t __fields[getSizeOfStructure(WoWContainer)] = {0};
};
