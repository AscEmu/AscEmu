/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <map>
#include <string>
#include "CommonTypes.hpp"

class SERVER_DECL ConfigFile
{
public:

    ConfigFile() = default;
    ~ConfigFile() = default;

    struct ConfigValueSetting
    {
        std::string asString;
        bool asBool;
        int asInt;
        float asFloat;
    };

    bool openAndLoadConfigFile(const std::string& configFileName);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Parser
    bool parseConfigValues(std::string fileBufferString);
    void removeSpacesInString(std::string& str);
    void removeAllSpacesInString(std::string& str);
    bool isComment(std::string& lineString, bool* isInMultilineComment);
    void applySettingToStore(std::string& str, ConfigValueSetting& setting);

    uint32_t getSettingHash(const std::string& settingString);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Get functions
    ConfigValueSetting* getSavedSetting(const std::string& sectionName, const std::string& confName);

    std::string getStringDefault(const std::string& sectionName, const std::string& confName, const std::string& defaultString);
    bool getBoolDefault(const std::string& sectionName, const std::string& confName, bool defaultBool);
    int getIntDefault(const std::string& sectionName, const std::string& confName, int defaultInt);
    float getFloatDefault(const std::string& sectionName, const std::string& confName, float defaultFloat);

    bool tryGetBool(const std::string& sectionName, const std::string& keyName, bool* b);
    bool tryGetFloat(const std::string& sectionName, const std::string& keyName, float* f);
    bool tryGetInt(const std::string& sectionName, const std::string& keyName, int* i);
    bool tryGetInt(const std::string& sectionName, const std::string& keyName, uint8_t* i);
    bool tryGetInt(const std::string& sectionName, const std::string& keyName, uint32_t* i);
    bool tryGetString(const std::string& sectionName, const std::string& keyName, std::string* s);

private:

    typedef std::map<uint32_t, ConfigValueSetting> ConfigSection;
    std::map<uint32_t, ConfigSection> mSettings;
};
