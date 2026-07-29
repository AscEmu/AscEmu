/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Config.hpp"

#include "Debugging/Errors.hpp"
#include "Logging/Logger.hpp"
#include "Utilities/Util.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdlib>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace
{
    [[nodiscard]] std::string_view trimLeft(std::string_view value) noexcept
    {
        while (!value.empty())
        {
            const char ch = value.front();
            if (ch != ' ' && ch != '\t' && ch != '\r')
            {
                break;
            }

            value.remove_prefix(1);
        }

        return value;
    }

    [[nodiscard]] std::string_view trimRight(std::string_view value) noexcept
    {
        while (!value.empty())
        {
            const char ch = value.back();
            if (ch != ' ' && ch != '\t' && ch != '\r')
            {
                break;
            }

            value.remove_suffix(1);
        }

        return value;
    }

    [[nodiscard]] std::string_view trim(std::string_view value) noexcept
    {
        return trimRight(trimLeft(value));
    }

    [[nodiscard]] bool equalsExact(std::string_view lhs, std::string_view rhs) noexcept
    {
        return lhs == rhs;
    }

    [[nodiscard]] int parseIntRelaxed(std::string_view value) noexcept
    {
        value = trim(value);

        if (value.empty())
        {
            return 0;
        }

        int parsedValue = 0;
        const auto result = std::from_chars(value.data(), value.data() + value.size(), parsedValue);
        if (result.ec == std::errc{})
        {
            return parsedValue;
        }

        return 0;
    }

    [[nodiscard]] float parseFloatRelaxed(std::string_view value) noexcept
    {
        value = trim(value);

        if (value.empty())
        {
            return 0.0f;
        }

        std::string buffer(value);
        char* endPtr = nullptr;
        return std::strtof(buffer.c_str(), &endPtr);
    }

    [[nodiscard]] [[maybe_unused]] bool startsWith(std::string_view value, std::string_view prefix) noexcept
    {
        return value.size() >= prefix.size() && value.substr(0, prefix.size()) == prefix;
    }
}

bool ConfigFile::openAndLoadConfigFile(const std::string& configFileName)
{
    m_settings.clear();

    if (configFileName.empty())
    {
        return false;
    }

    const auto configFile = Util::readFileIntoString(configFileName);
    return parseConfigValues(configFile);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Parser
bool ConfigFile::parseConfigValues(std::string fileBufferString)
{
    std::string_view fileBuffer(fileBufferString);

    bool isInMultilineComment = false;
    bool isInMultilineQuote = false;
    bool isInSectionBlock = false;

    std::string currentSection;
    std::string currentSettingVariable;
    std::string currentSettingValue;

    ConfigSection currentSectionMap;
    ConfigValueSetting currentValueSetting;

    const auto finalizeCurrentSetting = [&]() -> bool
    {
        if (currentSection.empty() || currentSettingVariable.empty())
        {
            sLogger.failure("Config parser state invalid: missing section or variable for quoted value.");
            return false;
        }

        applySettingToStore(currentSettingValue, currentValueSetting);
        currentSectionMap[calculateSettingHash(currentSettingVariable)] = currentValueSetting;

        currentValueSetting = {};
        currentSettingVariable.clear();
        currentSettingValue.clear();
        return true;
    };

    const auto finalizeCurrentSection = [&]()
    {
        if (!currentSection.empty())
        {
            m_settings[calculateSettingHash(currentSection)] = std::move(currentSectionMap);
            currentSectionMap = ConfigSection{};
            currentSection.clear();
        }

        currentSettingVariable.clear();
        currentSettingValue.clear();
        currentValueSetting = {};
        isInSectionBlock = false;
    };

    try
    {
        while (!fileBuffer.empty())
        {
            const auto lineEnd = fileBuffer.find('\n');
            std::string_view currentLine;

            if (lineEnd == std::string_view::npos)
            {
                currentLine = fileBuffer;
                fileBuffer = std::string_view{};
            }
            else
            {
                currentLine = fileBuffer.substr(0, lineEnd);
                fileBuffer.remove_prefix(lineEnd + 1);
            }

            if (!currentLine.empty() && currentLine.back() == '\r')
            {
                currentLine.remove_suffix(1);
            }

            std::string_view remaining = currentLine;

            while (!remaining.empty())
            {
                if (isInMultilineQuote)
                {
                    const auto quoteEnd = remaining.find('"');
                    if (quoteEnd == std::string_view::npos)
                    {
                        currentSettingValue.append(remaining.data(), remaining.size());
                        currentSettingValue.push_back('\n');
                        break;
                    }

                    currentSettingValue.append(remaining.data(), quoteEnd);
                    remaining.remove_prefix(quoteEnd + 1);

                    if (!finalizeCurrentSetting())
                    {
                        return false;
                    }

                    isInMultilineQuote = false;
                    continue;
                }

                remaining = trimLeft(remaining);
                if (remaining.empty())
                {
                    break;
                }

                if (isInMultilineComment)
                {
                    const auto commentEnd = remaining.find("*/");
                    if (commentEnd == std::string_view::npos)
                    {
                        break;
                    }

                    remaining.remove_prefix(commentEnd + 2);
                    isInMultilineComment = false;
                    continue;
                }

                {
                    std::string commentProbe(remaining);
                    if (isComment(commentProbe, &isInMultilineComment))
                    {
                        if (!isInMultilineComment)
                        {
                            break;
                        }

                        const auto commentStart = remaining.find("/*");
                        if (commentStart == std::string_view::npos)
                        {
                            break;
                        }

                        remaining.remove_prefix(commentStart + 2);
                        continue;
                    }
                }

                if (!isInSectionBlock)
                {
                    const auto sectionStart = remaining.find('<');
                    if (sectionStart == std::string_view::npos)
                    {
                        break;
                    }

                    remaining.remove_prefix(sectionStart + 1);
                    remaining = trimLeft(remaining);

                    if (remaining.empty())
                    {
                        sLogger.failure("Found the beginning of a section < but the section has no name!");
                        return false;
                    }

                    size_t sectionNameEnd = 0;
                    while (sectionNameEnd < remaining.size())
                    {
                        const char ch = remaining[sectionNameEnd];
                        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '>')
                        {
                            break;
                        }

                        ++sectionNameEnd;
                    }

                    if (sectionNameEnd == 0)
                    {
                        sLogger.failure("Found the beginning of a section < but the section has no name!");
                        return false;
                    }

                    currentSection.assign(remaining.substr(0, sectionNameEnd));
                    remaining.remove_prefix(sectionNameEnd);
                    isInSectionBlock = true;
                    continue;
                }

                if (remaining.front() == '>')
                {
                    remaining.remove_prefix(1);
                    finalizeCurrentSection();
                    continue;
                }

                const auto separatorPos = remaining.find('=');
                if (separatorPos == std::string_view::npos)
                {
                    break;
                }

                if (!currentSettingVariable.empty())
                {
                    sLogger.failure("Config parser found a new variable before the previous value was completed.");
                    return false;
                }

                currentSettingVariable.assign(remaining.substr(0, separatorPos));
                removeAllSpacesInString(currentSettingVariable);

                remaining.remove_prefix(separatorPos + 1);
                remaining = trimLeft(remaining);

                const auto quoteStart = remaining.find('"');
                if (quoteStart == std::string_view::npos)
                {
                    break;
                }

                remaining.remove_prefix(quoteStart + 1);

                const auto quoteEnd = remaining.find('"');
                if (quoteEnd != std::string_view::npos)
                {
                    currentSettingValue.assign(remaining.substr(0, quoteEnd));
                    remaining.remove_prefix(quoteEnd + 1);

                    if (!finalizeCurrentSetting())
                    {
                        return false;
                    }

                    continue;
                }

                currentSettingValue.assign(remaining);
                currentSettingValue.push_back('\n');
                isInMultilineQuote = true;
                break;
            }
        }
    }
    catch (...)
    {
        sLogger.failure("Exception in config parsing!");
        return false;
    }

    if (isInSectionBlock)
    {
        sLogger.failure("Unterminated section! Add > at the end of a section.");
        return false;
    }

    if (isInMultilineComment)
    {
        sLogger.failure("Unterminated multiline comment found! Add */ at the end of your multiline comment.");
        return false;
    }

    if (isInMultilineQuote)
    {
        sLogger.failure("Missing closing quote found! Add \" at the end of a definition.");
        return false;
    }

    return true;
}

void ConfigFile::removeSpacesInString(std::string& str)
{
    const auto firstNonSpace = str.find_first_not_of(" \t");
    if (firstNonSpace == std::string::npos)
    {
        str.clear();
        return;
    }

    str.erase(0, firstNonSpace);
}

void ConfigFile::removeAllSpacesInString(std::string& str)
{
    str.erase(
        std::remove_if(
            str.begin(),
            str.end(),
            [](const char ch)
            {
                return ch == ' ' || ch == '\t';
            }),
        str.end());
}

bool ConfigFile::isComment(std::string& lineString, bool* isInMultilineComment)
{
    std::string trimmedLine = lineString;
    removeSpacesInString(trimmedLine);

    if (trimmedLine.empty())
    {
        return false;
    }

    if (trimmedLine[0] == '/')
    {
        if (trimmedLine.size() < 2)
        {
            return false;
        }

        if (trimmedLine[1] == '*')
        {
            if (isInMultilineComment != nullptr)
            {
                *isInMultilineComment = true;
            }

            return true;
        }

        if (trimmedLine[1] == '/')
        {
            return true;
        }
    }

    return trimmedLine[0] == '#';
}

void ConfigFile::applySettingToStore(std::string& str, ConfigValueSetting& setting)
{
    setting.asString = str;
    setting.asInt = parseIntRelaxed(str);
    setting.asBool = setting.asInt > 0;
    setting.asFloat = parseFloatRelaxed(str);

    if (str.length() > 1)
    {
        if (equalsExact(str, "yes"))
        {
            setting.asBool = true;
            setting.asInt = 1;
        }
        else if (equalsExact(str, "no"))
        {
            setting.asBool = false;
            setting.asInt = 0;
        }
    }
}

uint32_t ConfigFile::getSettingHash(const std::string& settingString)
{
    return calculateSettingHash(settingString);
}

uint32_t ConfigFile::calculateSettingHash(std::string_view settingString) const noexcept
{
    uint32_t hashValue = 0;

    for (const unsigned char ch : settingString)
    {
        hashValue += 5 * hashValue + static_cast<uint32_t>(std::tolower(ch));
    }

    return hashValue;
}

const ConfigFile::ConfigValueSetting* ConfigFile::findSavedSetting(std::string_view sectionName, std::string_view confName) const noexcept
{
    const auto sectionItr = m_settings.find(calculateSettingHash(sectionName));
    if (sectionItr == m_settings.end())
    {
        return nullptr;
    }

    const auto valueItr = sectionItr->second.find(calculateSettingHash(confName));
    if (valueItr == sectionItr->second.end())
    {
        return nullptr;
    }

    return &valueItr->second;
}

ConfigFile::ConfigValueSetting* ConfigFile::findSavedSetting(std::string_view sectionName, std::string_view confName) noexcept
{
    const auto sectionItr = m_settings.find(calculateSettingHash(sectionName));
    if (sectionItr == m_settings.end())
    {
        return nullptr;
    }

    const auto valueItr = sectionItr->second.find(calculateSettingHash(confName));
    if (valueItr == sectionItr->second.end())
    {
        return nullptr;
    }

    return &valueItr->second;
}

void ConfigFile::logMissingSetting(const std::string& sectionName, const std::string& confName) const
{
    sLogger.failure("Could not load config value: [{}].[{}]", sectionName, confName);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get functions
ConfigFile::ConfigValueSetting* ConfigFile::getSavedSetting(const std::string& sectionName, const std::string& confName)
{
    if (auto* setting = findSavedSetting(sectionName, confName))
    {
        return setting;
    }

    throw std::invalid_argument("Could not load config value: [" + sectionName + "].[" + confName + "]");
}

std::string ConfigFile::getStringDefault(const std::string& sectionName, const std::string& confName, const std::string& defaultString)
{
    if (const auto* setting = findSavedSetting(sectionName, confName))
    {
        return setting->asString;
    }

    return defaultString;
}

bool ConfigFile::getBoolDefault(const std::string& sectionName, const std::string& confName, bool defaultBool)
{
    if (const auto* setting = findSavedSetting(sectionName, confName))
    {
        return setting->asBool;
    }

    return defaultBool;
}

int ConfigFile::getIntDefault(const std::string& sectionName, const std::string& confName, int defaultInt)
{
    if (const auto* setting = findSavedSetting(sectionName, confName))
    {
        return setting->asInt;
    }

    return defaultInt;
}

float ConfigFile::getFloatDefault(const std::string& sectionName, const std::string& confName, float defaultFloat)
{
    if (const auto* setting = findSavedSetting(sectionName, confName))
    {
        return setting->asFloat;
    }

    return defaultFloat;
}

bool ConfigFile::tryGetBool(const std::string& sectionName, const std::string& confName, bool* b)
{
    if (b == nullptr)
    {
        return false;
    }

    if (const auto* setting = findSavedSetting(sectionName, confName))
    {
        *b = setting->asBool;
        return true;
    }

    logMissingSetting(sectionName, confName);
    ASSERT(false);
    return false;
}

bool ConfigFile::tryGetFloat(const std::string& sectionName, const std::string& confName, float* f)
{
    if (f == nullptr)
    {
        return false;
    }

    if (const auto* setting = findSavedSetting(sectionName, confName))
    {
        *f = setting->asFloat;
        return true;
    }

    logMissingSetting(sectionName, confName);
    ASSERT(false);
    return false;
}

bool ConfigFile::tryGetInt(const std::string& sectionName, const std::string& confName, int* i)
{
    if (i == nullptr)
    {
        return false;
    }

    if (const auto* setting = findSavedSetting(sectionName, confName))
    {
        *i = setting->asInt;
        return true;
    }

    logMissingSetting(sectionName, confName);
    ASSERT(false);
    return false;
}

bool ConfigFile::tryGetInt(const std::string& sectionName, const std::string& confName, uint8_t* i)
{
    if (i == nullptr)
    {
        return false;
    }

    if (const auto* setting = findSavedSetting(sectionName, confName))
    {
        *i = static_cast<uint8_t>(setting->asInt);
        return true;
    }

    logMissingSetting(sectionName, confName);
    ASSERT(false);
    return false;
}

bool ConfigFile::tryGetInt(const std::string& sectionName, const std::string& confName, uint32_t* i)
{
    if (i == nullptr)
    {
        return false;
    }

    if (const auto* setting = findSavedSetting(sectionName, confName))
    {
        *i = static_cast<uint32_t>(setting->asInt);
        return true;
    }

    logMissingSetting(sectionName, confName);
    ASSERT(false);
    return false;
}

bool ConfigFile::tryGetString(const std::string& sectionName, const std::string& confName, std::string* s)
{
    if (s == nullptr)
    {
        return false;
    }

    if (const auto* setting = findSavedSetting(sectionName, confName))
    {
        *s = setting->asString;
        return true;
    }

    logMissingSetting(sectionName, confName);
    ASSERT(false);
    return false;
}
