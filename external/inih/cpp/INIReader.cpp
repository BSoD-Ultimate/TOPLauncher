// Read an INI file into easy-to-access name/value pairs.

// inih and INIReader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include "../ini.h"
#include "INIReader.h"

using std::string;

INIReader::INIReader(const string& filename)
{
    _error = ini_parse(filename.c_str(), ValueHandler, this);
}

int INIReader::ParseError() const
{
    return _error;
}

string INIReader::Get(const string& section, const string& name, const string& default_value) const
{
    string key = MakeKey(section, name);
    // Use _values.find() here instead of _values.at() to support pre C++11 compilers
    auto sectionIter = std::find_if(_sectionKeys.cbegin(), _sectionKeys.cend(), 
        [section](const std::pair<std::string, std::vector<KeyValuePair>>& pair)
    {
        return section == pair.first;
    });
    if (sectionIter != _sectionKeys.cend())
    {
        auto keyIter = std::find_if(sectionIter->second.cbegin(), sectionIter->second.cend(),
            [&name](const KeyValuePair& pair) 
        {
            return pair.first == name;
        });
        if (keyIter != sectionIter->second.cend())
        {
            return keyIter->second;
        }
    }
    return default_value;
}

long INIReader::GetInteger(const string& section, const string& name, long default_value) const
{
    string valstr = Get(section, name, "");
    const char* value = valstr.c_str();
    char* end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    long n = strtol(value, &end, 0);
    return end > value ? n : default_value;
}

double INIReader::GetReal(const string& section, const string& name, double default_value) const
{
    string valstr = Get(section, name, "");
    const char* value = valstr.c_str();
    char* end;
    double n = strtod(value, &end);
    return end > value ? n : default_value;
}

bool INIReader::GetBoolean(const string& section, const string& name, bool default_value) const
{
    string valstr = Get(section, name, "");
    // Convert to lower case to make string comparisons case-insensitive
    std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
    if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
        return true;
    else if (valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0")
        return false;
    else
        return default_value;
}

void INIReader::Set(const std::string & section, const std::string & name, const std::string & value)
{
    auto sectionIter = std::find_if(_sectionKeys.cbegin(), _sectionKeys.cend(),
        [section](const std::pair<std::string, std::vector<KeyValuePair>>& pair)
    {
        return section == pair.first;
    });

    if (sectionIter == _sectionKeys.cend())
    {
        _sectionKeys.emplace_back(section, std::vector<KeyValuePair>());
    }

    auto& kvPairs = std::find_if(_sectionKeys.begin(), _sectionKeys.end(),
        [section](const std::pair<std::string, std::vector<KeyValuePair>>& pair)
    {
        return section == pair.first;
    })->second;

    auto keyIter = std::find_if(kvPairs.begin(), kvPairs.end(),
        [&name](const KeyValuePair& pair)
    {
        return pair.first == name;
    });

    if (keyIter == kvPairs.end())
    {
        kvPairs.emplace_back(name, value);
    }
    else
    {
        keyIter->second = value;
    }
}

void INIReader::WriteINIString(std::string & output)
{
    output.clear();

    for (auto iterSection = _sectionKeys.cbegin(); iterSection != _sectionKeys.cend(); iterSection++)
    {
        std::string sectionString = "[" + iterSection->first + "]\r\n";
        output.append(sectionString);

        for (auto iterKeyValue = iterSection->second.cbegin(); iterKeyValue != iterSection->second.cend(); iterKeyValue++)
        {
            std::string kvString = iterKeyValue->first + " = " + iterKeyValue->second + "\r\n";
            output.append(kvString);
        }

        output.append("\r\n");
    }
}

void INIReader::WriteINIFile(const std::string & saveFileName)
{
    FILE* fp = fopen(saveFileName.c_str(), "wb");

    std::string iniContent;
    WriteINIString(iniContent);
    fwrite(iniContent.c_str(), 1, iniContent.length(), fp);

    fclose(fp);
}

string INIReader::MakeKey(const string& section, const string& name)
{
    string key = section + "=" + name;
    // Convert to lower case to make section/name lookups case-insensitive
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    return key;
}

int INIReader::ValueHandler(void* user, const char* section, const char* name,
                            const char* value)
{
    INIReader* reader = (INIReader*)user;

    auto sectionIter = std::find_if(reader->_sectionKeys.cbegin(), reader->_sectionKeys.cend(),
        [section](const std::pair<std::string, std::vector<KeyValuePair>>& pair)
    {
        return section == pair.first;
    });

    if (sectionIter == reader->_sectionKeys.cend())
    {
        reader->_sectionKeys.emplace_back(section, std::vector<KeyValuePair>());
    }

    auto& kvPairs = std::find_if(reader->_sectionKeys.begin(), reader->_sectionKeys.end(),
        [section](const std::pair<std::string, std::vector<KeyValuePair>>& pair)
    {
        return section == pair.first;
    })->second;

    auto keyIter = std::find_if(kvPairs.begin(), kvPairs.end(),
        [&name](const KeyValuePair& pair)
    {
        return pair.first == name;
    });

    if (keyIter == kvPairs.end())
    {
        kvPairs.emplace_back(name, value);
    }
    else
    {
        keyIter->second = value;
    }

    //if (reader->_values[key].size() > 0)
    //    reader->_values[key] += "\n";
    //reader->_values[key] += value;
    return 1;
}
