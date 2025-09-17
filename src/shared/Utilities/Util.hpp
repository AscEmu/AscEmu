/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <chrono>
#include <map>
#include <filesystem>
#include <locale>
#include <type_traits>

namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // WoW String functions

    /*! \brief Returns wow specific language string to id*/
    uint8_t getLanguagesIdFromString(const std::string& langstr);

    /*! \brief Returns wow specific language id to string*/
    std::string getLanguagesStringFromId(uint8_t id);

    /*! \brief Returns an uint32_t from a string between start/endcharacter */
    uint32_t getNumberFromStringByRange(const std::string& string, int startCharacter, int endCharacter);

    //////////////////////////////////////////////////////////////////////////////////////////
    // utf8String functions

    std::size_t max_consecutive(std::string_view name, bool case_insensitive = false, const std::locale& locale = std::locale());

    //////////////////////////////////////////////////////////////////////////////////////////
    // Time calculation/formatting

    /*! \brief Returns the current point in time */
    std::chrono::high_resolution_clock::time_point TimeNow();

    /*! \ brief Returns TimeNow() as time_t*/
    time_t getTimeNow();

    /*! \ brief Returns TimeNow() as uint32_t*/
    uint32_t getMSTime();

    /*! \brief Returns the difference between start_time and now in milliseconds */
    long long GetTimeDifferenceToNow(const std::chrono::high_resolution_clock::time_point& start_time);

    /*! \brief Returns the difference between start_time and end_time in milliseconds */
    long long GetTimeDifference(const std::chrono::high_resolution_clock::time_point& start_time, const std::chrono::high_resolution_clock::time_point& end_time);

    /*! \brief Returns the current Date Time as string */
    std::string GetCurrentDateTimeString();

    /*! \brief Returns the current Time as string */
    std::string GetCurrentTimeString();

    /*! \brief Returns Date Time as string from timestamp */
    std::string GetDateTimeStringFromTimeStamp(uint32_t timestamp);

    /*! \brief Returns years months days hours minutes seconds as string from seconds value */
    std::string GetDateStringFromSeconds(uint32_t seconds);

    /*! \brief Returns calculated time based on (second) values e.g. 5h will return 5 * 60 * 60 */
    uint32_t GetTimePeriodFromString(const char* str);

    /*! \brief Returns generated time value for client packets */
    uint32_t getGameTime();

    time_t getLocalHourTimestamp(time_t time, uint8_t hour, bool onlyAfterTime = true);

    std::string ByteArrayToHexString(uint8_t const* bytes, uint32_t arrayLength, bool reverseArray = false);


    //////////////////////////////////////////////////////////////////////////////////////////
    // C++17 filesystem dependent functions

    /*! \brief Returns map of directory file names. */
    std::map<uint32_t, std::string> getDirectoryContent(const std::string& pathName, const std::string& specialSuffix = "", bool withPath = false);

    /*! \brief Reads the file into a string based on the given path. */
    std::string readFileIntoString(std::filesystem::path path);

    /*! \brief Returns the first 8 chars of the file name as major version. */
    uint32_t readMajorVersionFromString(const std::string& fileName);

    uint32_t readMinorVersionFromString(const std::string& fileName);

    //////////////////////////////////////////////////////////////////////////////////////////
    // std::is_specialization_of_v implementation

    // Primary template - not a specialization
    template <template <typename, typename...> class Template, typename>
    struct is_specialization_of : std::false_type {};

    // Partial specialization for types with a second parameter as a pack
    template <template <typename, typename...> class Template, typename T1, typename... Args>
    struct is_specialization_of<Template, Template<T1, Args...>> : std::true_type {};

    // Helper variable template for convenience
    template <typename T, template <typename, typename...> class Template>
    concept is_specialization_of_v = is_specialization_of<Template, T>::value;

    //////////////////////////////////////////////////////////////////////////////////////////
    // std::is_specialization_of_v implementation for e.g. std::arrays

    // Primary template for types with size_t as the second parameter - not a specialization
    template <template <typename, size_t> class Template, typename>
    struct is_size_based_specialization_of : std::false_type {};

    // Partial specialization for types with size_t as the second parameter
    template <template <typename, size_t> class Template, typename T1, size_t N>
    struct is_size_based_specialization_of<Template, Template<T1, N>> : std::true_type {};

    // Helper variable template for types with size_t as the second parameter for convenience
    template <typename T, template <typename, size_t> class Template>
    concept is_size_based_specialization_of_v = is_size_based_specialization_of<Template, T>::value;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    unsigned int makeIP(std::string_view _str);

    bool parseCIDRBan(uint32_t _ip, uint32_t _mask, uint32_t _maskBits);

    template<class Factory>
    struct LazyInstanceCreator
    {
        /*!
        * \brief Converts a struct to a specified type using a lazy evaluation approach.
        *
        * This struct utilizes an internal lambda factory to generate an instance of the provided type
        * when a conversion is necessary. It is particularly useful when working with objects and
        * emplace/try_emplace.
        *
        * Important notes:
        *
        * - The internal lambda will only be invoked if a conversion is required, ensuring that the
        * evaluation is lazy and avoids unnecessary overhead. If i.e. emplace operation fails, no
        * conversion will take place, meaning that resources such as unique_ptr will not be allocated,
        * preventing even temporary unnecessary memory allocations.
        *
        * - Works with both emplace and try_emplace
        */
        constexpr LazyInstanceCreator(Factory&& factory) : m_factory(std::move(factory))
        {
        }

        constexpr operator auto() const noexcept(std::is_nothrow_invocable_v<const Factory&>)
        {
            return m_factory();
        }

        Factory m_factory;
    };
}
