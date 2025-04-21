/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

    // Returns item in tuple with result if failed to add item, nullptr on success
    std::tuple<bool, std::unique_ptr<Item>> addItem(int16_t slot, std::unique_ptr<Item> itemHolder);
    // Returns item in tuple with result if failed to add item, nullptr on success
    std::tuple<bool, std::unique_ptr<Item>> addItemToFreeSlot(std::unique_ptr<Item> itemHolder, uint32_t* r_slot);
    void forceCreationUpdate(Item* item);

    Item* getItem(int16_t slot);

    int8_t findFreeSlot();
    bool hasItems() const;

    void swapItems(int8_t SrcSlot, int8_t DstSlot);
    std::unique_ptr<Item> safeRemoveAndRetreiveItemFromSlot(int16_t slot, bool destroy);
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
    std::unique_ptr<std::unique_ptr<Item>[]> m_Slot;
    uint32_t __fields[getSizeOfStructure(WoWContainer)] = {0};
};
