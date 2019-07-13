/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Common.hpp"
#include "Config.h"
#include "Log.hpp"
#include "Util.hpp"


ConfigFile::ConfigFile() {}

ConfigFile::~ConfigFile() {}

bool ConfigFile::openAndLoadConfigFile(std::string configFileName)
{
    mSettings.clear();

    if (!configFileName.empty())
    {
        const auto configFile = Util::readFileIntoString(configFileName);

        return parseConfigValues(configFile);
    }
    return false;
}

bool ConfigFile::parseConfigValues(std::string fileBufferString)
{
    std::string currentLine;

    std::string::size_type lineEnding;
    std::string::size_type lineOffset;

    bool isInMultilineComment = false;
    bool isInMultilineQuote = false;
    bool isInSectionBlock = false;

    std::string currentSection = "";
    std::string currentSettingVariable = "";
    std::string currentSettingValue = "";

    ConfigSection currentSectionMap;
    ConfigValueSetting currentValueSettingStruct;

    try
    {
        for (;;)
        {
            // grab current line
            lineEnding = fileBufferString.find('\n');
            if (lineEnding == std::string::npos)
            {
                if (fileBufferString.size() == 0)
                    break;

                currentLine = fileBufferString;
                fileBufferString.clear();

                goto parse;
            }

            currentLine = fileBufferString.substr(0, lineEnding);
            fileBufferString.erase(0, lineEnding + 1);  // eol size 1

            goto parse;

        parse:
            if (!currentLine.size())
                continue;

            // are we in a comment section?
            if (!isInMultilineComment && isComment(currentLine, &isInMultilineComment))
            {
                // our line is a comment
                if (!isInMultilineComment)
                {
                    // the entire line is a comment
                    continue;
                }
            }

            // handle our cases
            if (isInMultilineComment)
            {
                lineOffset = currentLine.find("*/", 0);
                if (lineOffset == std::string::npos)    // skip entire line
                    continue;

                lineOffset = currentLine.find("//", 0);
                if (lineOffset == std::string::npos)    // skip entire line
                    continue;

                lineOffset = currentLine.find("#", 0);
                if (lineOffset == std::string::npos)    // skip entire line
                    continue;

                // remove up to the end of the comment block
                currentLine.erase(0, lineOffset + 1);   // eol size 1
                isInMultilineComment = false;
            }

            if (isInSectionBlock)
            {
                // handle settings across multiple lines
                if (isInMultilineQuote)
                {
                    // find the end of the quote block
                    lineOffset = currentLine.find("\"");
                    if (lineOffset == std::string::npos)
                    {
                        // append the whole line to the quote
                        currentSettingValue += currentLine;
                        currentSettingValue += "\n";
                        continue;
                    }

                    // append part of the line to the setting
                    currentSettingValue.append(currentLine.c_str(), lineOffset + 1);
                    currentLine.erase(0, lineOffset + 1);

                    // append the setting to the config section
                    if (currentSection == "" || currentSettingVariable == "")
                    {
                        LogError("Quote without variable.");
                        return false;
                    }

                    // apply the setting
                    applySettingToStore(currentSettingValue, currentValueSettingStruct);

                    // the var is done, append to the current section
                    currentSectionMap[getSettingHash(currentSettingVariable)] = currentValueSettingStruct;

                    // no longer the var or in a quote
                    currentSettingValue = "";
                    currentSettingVariable = "";
                    isInMultilineQuote = false;
                }

                // remove spaces
                removeSpacesInString(currentLine);
                if (!currentLine.size())
                    continue;

                // looking for variable '=' is our seperator
                lineOffset = currentLine.find("=");
                if (lineOffset != std::string::npos)
                {
                    ASSERT(currentSettingVariable == "");
                    currentSettingVariable = currentLine.substr(0, lineOffset);

                    // remove spaces from the end of the variable
                    removeAllSpacesInString(currentSettingVariable);

                    // remove the dir and the '=' from the line
                    currentLine.erase(0, lineOffset + 1);
                }

                // look for opening quote. this signifies the start of a value
                lineOffset = currentLine.find("\"");
                if (lineOffset != std::string::npos)
                {
                    ASSERT(currentSettingValue == "");
                    ASSERT(currentSettingVariable != "");

                    // find the ending quote
                    lineEnding = currentLine.find("\"", lineOffset + 1);
                    if (lineEnding != std::string::npos)
                    {
                        // the closing quote is on the same line
                        currentSettingValue = currentLine.substr(lineOffset + 1, lineEnding - lineOffset - 1);

                        // erase to the end
                        currentLine.erase(0, lineEnding + 1);

                        // apply the value
                        applySettingToStore(currentSettingValue, currentValueSettingStruct);

                        // the var is done, append to the current block
                        currentSectionMap[getSettingHash(currentSettingVariable)] = currentValueSettingStruct;

                        // no longer the var or in a quote
                        currentSettingValue = "";
                        currentSettingVariable = "";
                        isInMultilineQuote = false;

                        // go find other definitions on this line
                        goto parse;
                    }
                    else
                    {
                        // closing quote is not on this line. We'll try to find it on the next
                        currentSettingValue.append(currentLine.c_str(), lineOffset);

                        // go to the next line.
                        isInMultilineQuote = true;
                        continue;
                    }
                }

                // check end of the section
                lineOffset = currentLine.find(">");
                if (lineOffset != std::string::npos)
                {
                    currentLine.erase(0, lineOffset + 1);

                    isInSectionBlock = false;

                    // assign this block to the section map
                    mSettings[getSettingHash(currentSection)] = currentSectionMap;

                    // cleanup for next parse
                    currentSectionMap.clear();
                    currentSettingValue = "";
                    currentSettingVariable = "";
                    currentSection = "";
                }
            }
            else
            {
                // check for start of a section since we are not in one
                lineOffset = currentLine.find("<");
                if (lineOffset != std::string::npos)
                {
                    isInSectionBlock = true;

                    currentLine.erase(0, lineOffset + 1);

                    // find the section name
                    lineOffset = currentLine.find(" ");
                    if (lineOffset != std::string::npos)
                    {
                        currentSection = currentLine.substr(0, lineOffset);
                        currentLine.erase(0, lineOffset + 1);
                    }
                    else
                    {
                        LogError("Found the beginning of a section < but the section has no name!");
                        return false;
                    }

                    // skip to parse next
                    goto parse;
                }
            }
        }

    }
    catch (...)
    {
        LogError("Exception in config parsing!");
        return false;
    }

    // check errors
    if (isInSectionBlock)
    {
        LogError("Unterminated section! Add > at the end of a section.");
        return false;
    }

    if (isInMultilineComment)
    {
        LogError("Unterminated multiline comment found! Add */ at the end of your multiline comment.");
        return false;
    }

    if (isInMultilineQuote)
    {
        LogError("Missing closing quote found! Add \" at the end of a definition.");
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Parser
void ConfigFile::removeSpacesInString(std::string& str)
{
    while (str.size() && (*str.begin() == ' ' || *str.begin() == '\t'))
        str.erase(str.begin());
}

void ConfigFile::removeAllSpacesInString(std::string& str)
{
    std::string::size_type off = str.find(" ");
    while (off != std::string::npos)
    {
        str.erase(off, 1);
        off = str.find(" ");
    }

    off = str.find("\t");
    while (off != std::string::npos)
    {
        str.erase(off, 1);
        off = str.find("\t");
    }
}

bool ConfigFile::isComment(std::string& lineString, bool* isInMultilineComment)
{
    std::string stemp = lineString;
    removeSpacesInString(stemp);

    if (stemp.length() == 0)
        return false;

    if (stemp[0] == '/')
    {
        if (stemp.length() < 2)
            return false;

        if (stemp[1] == '*')
        {
            *isInMultilineComment = true;
            return true;
        }
        else if (stemp[1] == '/')
        {
            return true;
        }
    }

    if (stemp[0] == '#')
        return true;

    return false;
}

void ConfigFile::applySettingToStore(std::string& str, ConfigValueSetting& setting)
{
    setting.asString = str;
    setting.asInt = atoi(str.c_str());
    setting.asBool = (setting.asInt > 0);
    setting.asFloat = (float)atof(str.c_str());

    // check for yes / no answers
    if (str.length() > 1)
    {
        // is it a bool?
        if (str.compare("yes") == 0)
        {
            setting.asBool = true;
            setting.asInt = 1;
        }
        else if (str.compare("no") == 0)
        {
            setting.asBool = false;
            setting.asInt = 0;
        }
    }
}

uint32_t ConfigFile::getSettingHash(std::string settingString)
{
    size_t stringLength = settingString.size();
    uint32_t returnHash = 0;

    for (size_t i = 0; i < stringLength; ++i)
        returnHash += 5 * returnHash + (tolower(settingString[i]));

    return returnHash;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get functions
ConfigFile::ConfigValueSetting* ConfigFile::getSavedSetting(std::string sectionName, std::string confName)
{
    uint32_t sectionHash = getSettingHash(sectionName);
    uint32_t configHash = getSettingHash(confName);

    std::map<uint32_t, ConfigSection>::iterator itr = mSettings.find(sectionHash);
    if (itr != mSettings.end())
    {
        ConfigSection::iterator it2 = itr->second.find(configHash);
        if (it2 != itr->second.end())
            return &(it2->second);
    }

    LOG_ERROR("Could not load config value: [%s].[%s]", sectionName.c_str(), confName.c_str());
    return nullptr;
}

std::string ConfigFile::getStringDefault(std::string sectionName, std::string confName, std::string defaultString)
{
    ConfigValueSetting* confSetting = getSavedSetting(sectionName, confName);
    if (confSetting == nullptr)
        return defaultString;

    return confSetting->asString;
}

bool ConfigFile::getBoolDefault(std::string sectionName, std::string confName, bool defaultBool)
{
    ConfigValueSetting* confSetting = getSavedSetting(sectionName, confName);
    if (confSetting == nullptr)
        return defaultBool;

    return confSetting->asBool;
}

int ConfigFile::getIntDefault(std::string sectionName, std::string confName, int defaultInt)
{
    ConfigValueSetting* confSetting = getSavedSetting(sectionName, confName);
    if (confSetting == nullptr)
        return defaultInt;

    return confSetting->asInt;
}

float ConfigFile::getFloatDefault(std::string sectionName, std::string confName, float defaultFloat)
{
    ConfigValueSetting* confSetting = getSavedSetting(sectionName, confName);
    if (confSetting == nullptr)
        return defaultFloat;

    return confSetting->asFloat;
}

bool ConfigFile::tryGetBool(std::string sectionName, std::string confName, bool * b)
{
    const auto setting = getSavedSetting(sectionName, confName);
    if (!setting)
    {
        return false;
    }

    *b = setting->asBool;
    return true;
}

bool ConfigFile::tryGetFloat(std::string sectionName, std::string confName, float* f)
{
    const auto setting = getSavedSetting(sectionName, confName);
    if (!setting)
    {
        return false;
    }

    *f = setting->asFloat;
    return true;
}

bool ConfigFile::tryGetInt(std::string sectionName, std::string confName, int* i)
{
    const auto setting = getSavedSetting(sectionName, confName);
    if (!setting)
    {
        return false;
    }

    *i = setting->asInt;
    return true;
}

bool ConfigFile::tryGetInt(std::string sectionName, std::string confName, uint8_t* i)
{
    const auto setting = getSavedSetting(sectionName, confName);
    if (!setting)
    {
        return false;
    }

    *i = setting->asInt;
    return true;
}

bool ConfigFile::tryGetInt(std::string sectionName, std::string confName, uint32_t* i)
{
    const auto setting = getSavedSetting(sectionName, confName);
    if (!setting)
    {
        return false;
    }

    *i = setting->asInt;
    return true;
}

bool ConfigFile::tryGetString(std::string sectionName, std::string confName, std::string* s)
{
    const auto setting = getSavedSetting(sectionName, confName);
    if (!setting)
    {
        return false;
    }

    *s = setting->asString;
    return true;
}
