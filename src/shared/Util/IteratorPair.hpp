/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <utility>

namespace AscEmu::Util
{
    /**
     * @class IteratorPair
     *
     * @brief Utility class to enable range for loop syntax for multimap.equal_range uses
     */
    template<class iterator>
    class IteratorPair
    {
    public:
        constexpr IteratorPair() : _iterators() { }
        constexpr IteratorPair(iterator first, iterator second) : _iterators(first, second) { }
        constexpr IteratorPair(std::pair<iterator, iterator> iterators) : _iterators(iterators) { }

        constexpr iterator begin() const { return _iterators.first; }
        constexpr iterator end() const { return _iterators.second; }

    private:
        std::pair<iterator, iterator> _iterators;
    };

    namespace Containers
    {
        template<class M>
        inline auto MapEqualRange(M& map, typename M::key_type const& key) -> IteratorPair<decltype(map.begin())>
        {
            return { map.equal_range(key) };
        }
    }
    //! namespace Containers
}
//! namespace AscEmu::Util
