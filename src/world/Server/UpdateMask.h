/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#ifndef UPDATEMASK_H
#define UPDATEMASK_H

#include <cstring>
#include <memory>

class UpdateMask
{
    std::unique_ptr<uint32_t[]> mUpdateMask;
    uint32_t mCount; // in values
    uint32_t mBlocks; // in uint32_t blocks

    public:
        UpdateMask() : mUpdateMask(0), mCount(0), mBlocks(0) { }
        UpdateMask(const UpdateMask & mask) : mUpdateMask(0) { *this = mask; }

        ~UpdateMask() = default;

        void SetBit(const uint32_t index)
        {
            if (index < mCount)
                ((uint8_t*)mUpdateMask.get())[ index >> 3 ] |= 1 << (index & 0x7);
        }

        void UnsetBit(const uint32_t index)
        {
            if (index < mCount)
                ((uint8_t*)mUpdateMask.get())[ index >> 3 ] &= (0xff ^ (1 << (index & 0x7)));
        }

        bool GetBit(const uint32_t index) const
        {
            if (index < mCount)
                return (((uint8_t*)mUpdateMask.get())[index >> 3] & (1 << (index & 0x7))) != 0;
            return false;
        }

        uint32_t GetUpdateBlockCount() const
        {
            uint32_t x;
            for (x = mBlocks - 1; x; x--)
                if (mUpdateMask[x])break;
            return (x + 1);
        }
        inline uint32_t GetBlockCount() const {return mBlocks;}

        inline uint32_t GetLength() const { return (mBlocks * sizeof(uint32_t)); }
        inline uint32_t GetCount() const { return mCount; }
        inline const uint8_t* GetMask() const { return (uint8_t*)mUpdateMask.get(); }

        void SetCount(uint32_t valuesCount)
        {
            mCount = valuesCount;
            //mBlocks = valuesCount/32 + 1;
            //mBlocks = (valuesCount + 31) / 32;
            mBlocks = mCount >> 5;
            if (mCount & 31)
                ++mBlocks;

            mUpdateMask = std::make_unique<uint32_t[]>(mBlocks);
            memset(mUpdateMask.get(), 0, mBlocks * sizeof(uint32_t));
        }

        void Clear()
        {
            if (mUpdateMask)
                memset(mUpdateMask.get(), 0, mBlocks << 2);
        }

        UpdateMask & operator = (const UpdateMask & mask)
        {
            SetCount(mask.mCount);
            memcpy(mUpdateMask.get(), mask.mUpdateMask.get(), mBlocks << 2);

            return *this;
        }

        void operator &= (const UpdateMask & mask)
        {
            if (mask.mCount <= mCount)
                for (uint32_t i = 0; i < mBlocks; i++)
                    mUpdateMask[i] &= mask.mUpdateMask[i];
        }

        void operator |= (const UpdateMask & mask)
        {
            if (mask.mCount <= mCount)
                for (uint32_t i = 0; i < mBlocks; i++)
                    mUpdateMask[i] |= mask.mUpdateMask[i];
        }

        UpdateMask operator & (const UpdateMask & mask) const
        {
            if (mask.mCount <= mCount)
            {
                UpdateMask newmask;
                newmask = *this;
                newmask &= mask;

                return newmask;
            }
            return mask;
        }

        UpdateMask operator | (const UpdateMask & mask) const
        {
            if (mask.mCount <= mCount)
            {
                UpdateMask newmask;
                newmask = *this;
                newmask |= mask;

                return newmask;
            }
            return mask;
        }
};

#endif      //UPDATEMASK_H
