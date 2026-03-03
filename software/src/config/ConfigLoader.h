#pragma once

#include <cctype>
#include <cstdint>
#include <fstream> //file reading
#include <sstream> //text to stream
#include <string>
#include <unordered_map> //for storing key value pairs from my config file
#include <vector> // for warnings and errors.
#include <optional>

#include "Config.h"

namespace eclipse {

    //to store the results of the loaded config
    struct ConfigLoadResult {
        AppConfig config = AppConfig::defaults();
        bool ok = true;
        //storing issues
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
    };

    class ConfigLoader {
    public:
        //loads results from a file path
        static ConfigLoadResult loadFromFile(const std::string& path);

        //The main parser
        static ConfigLoadResult loadFromString(const std::string& text);

    private:
        //helper fucnitons

        //splits file inot lines, rmeoves comments and extracts key-value pairs in a hash map.
        static std::unordered_map<std::string, std::string> parseKeyValue(
            //takes enitre converted config file as one big string
            const std::string& text,
            //if malformed lines appear we push warnings into this vector.
            std::vector<std::string>& warnings
        );

        //checks logic completeness, adds warning but does not crash if somehting is missing.
        static void validateThreshold(const std::string& name, const ThresholdBand& t, std::vector<std::string>& warnings);

        //cmpares two strings ignoring case 
        static bool equalsIgnoreCase(const std::string& a, const std::string& b);

        //strip comments from a string..
        static std::string stripComment(const std::string& s);

        //classic trim funcion for leading or trailing whitespace, tabs etc.
        static std::string trim(const std::string& s);

        //bool fucntion that looks for a key  inside a map, copies the value into the  out
        static bool getString(const std::unordered_map<std::string, std::string>& kv,
            // the key we want
            const std::string& key,
            //output param.
            std::string& out);

        //looks up a key in the config map, assigns its value to target.
        static void setString(const std::unordered_map<std::string, std::string>& kv,
            const std::string& key,
            //what we want
            std::string& target,
            std::vector<std::string>& warnings);

        //attempts to convert a string to an unsigned integer
        static bool parseUInt(const std::string& s, uint32_t& out);

        //Attempts to turn string to double.
        static bool parseDouble(const std::string& s, double& out);

        //If key exsists, attempt parse, if invalid add warning otherwise assign 
        static void setUInt(const std::unordered_map<std::string, std::string>& kv,
            const std::string& key,
            uint32_t& target,
            std::vector<std::string>& warnings);

        //same logic as setUInt()
        static void setDouble(const std::unordered_map<std::string, std::string>& kv,
            const std::string& key,
            double& target,
            std::vector<std::string>& warnings);

        //used for optional thresholds, if key exists and parses, assign value otherwise silently ignore.
        static void setOptDouble(const std::unordered_map<std::string, std::string>& kv,
            const std::string& key,
            std::optional<double>& target);
    };

}