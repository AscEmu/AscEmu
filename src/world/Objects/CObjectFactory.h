/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
 *
 */

#ifndef COBJECT_FACTORY_H
#define COBJECT_FACTORY_H

class Object;
class GameObject;

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief Factory class that instantiates and destroys all Objects
//////////////////////////////////////////////////////////////////////////////////////////
class CObjectFactory
{
    public:

        CObjectFactory() {}
        ~CObjectFactory() {}

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Creates an instance of the GameObject class.
        /// \param uint32 Id  -  Entry if of the GameObject
        /// \param uint32 LowGUID  -  Unique ID of this instance
        ///
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        GameObject* CreateGameObject(uint32 Id, uint32 LowGUID);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Disposes of the created Object
        /// \param none
        ///
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void DisposeOf(Object* obj);
};

#endif      //COBJECT_FACTORY_H
