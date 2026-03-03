#include "ConfigLoader.h"

namespace eclipse {

    //loads results from a file path
    ConfigLoadResult ConfigLoader::loadFromFile(const std::string& path) {
        ConfigLoadResult res;

        //opens file
        std::ifstream in(path);
        //if not open returns false and updatea error vector
        if (!in.is_open()) {
            res.ok = false;
            res.errors.push_back("ConfigLoader: Failed to open config file: " + path);
            return res;
        }

        //reads entire file into memory as string
        std::stringstream buffer;
        buffer << in.rdbuf();
        return loadFromString(buffer.str());
    }

    //The main parser
    ConfigLoadResult ConfigLoader::loadFromString(const std::string& text) {
        ConfigLoadResult res;

        //lambdas that turns config text into key value pairs as strings
        auto kv = parseKeyValue(text, res.warnings);

        //converts string to unit, assigns to config and warns if invalid
        setUInt(kv, "samplePeriodMs", res.config.samplePeriodMs, res.warnings);

        //Telemetry source block
        {
            std::string src;

            //if the key exsists then read it
            if (getString(kv, "telemetry.source", src)) {
                //case insensitive comparison for serial and sim
                if (equalsIgnoreCase(src, "serial")) res.config.telemetry.sourceType = TelemetrySourceType::Serial;
                else if (equalsIgnoreCase(src, "sim") || equalsIgnoreCase(src, "simulated"))
                    res.config.telemetry.sourceType = TelemetrySourceType::Simulated;
                //if both don't work, update warnings.
                else
                    res.warnings.push_back("Unknown telemetry.source value: " + src + " (keeping default)");
            }
        }

        //Serial telemetry - override defaults is present
        setString(kv, "telemetry.serial.port", res.config.telemetry.serial.portName, res.warnings);
        setUInt(kv, "telemetry.serial.baud", res.config.telemetry.serial.baudRate, res.warnings);

        //Sim telemetry - same idea
        setDouble(kv, "telemetry.sim.updateHz", res.config.telemetry.sim.updateHz, res.warnings);
        //Base simulated values
        setDouble(kv, "telemetry.sim.bmeTempC", res.config.telemetry.sim.bmeTempC, res.warnings);
        setDouble(kv, "telemetry.sim.bmeRhPct", res.config.telemetry.sim.bmeRhPct, res.warnings);
        setDouble(kv, "telemetry.sim.bmePressureHpa", res.config.telemetry.sim.bmePressureHpa, res.warnings);

        setDouble(kv, "telemetry.sim.scdCo2Ppm", res.config.telemetry.sim.scdCo2Ppm, res.warnings);
        setDouble(kv, "telemetry.sim.scdTempC", res.config.telemetry.sim.scdTempC, res.warnings);
        setDouble(kv, "telemetry.sim.scdRhPct", res.config.telemetry.sim.scdRhPct, res.warnings);

        setDouble(kv, "telemetry.sim.radCpm", res.config.telemetry.sim.radCpm, res.warnings);

        //Noise control
        setDouble(kv, "telemetry.sim.noiseStd", res.config.telemetry.sim.noiseStd, res.warnings);
        {
            //added for space / sim config.
            uint32_t spikes = res.config.telemetry.sim.enableSpikes ? 1u : 0u;
            setUInt(kv, "telemetry.sim.enableSpikes", spikes, res.warnings);
            res.config.telemetry.sim.enableSpikes = (spikes != 0u);
        }

        setDouble(kv, "telemetry.sim.spikeChance", res.config.telemetry.sim.spikeChance, res.warnings);
        setDouble(kv, "telemetry.sim.co2SpikePpm", res.config.telemetry.sim.co2SpikePpm, res.warnings);
        setDouble(kv, "telemetry.sim.radSpikeCpm", res.config.telemetry.sim.radSpikeCpm, res.warnings);


        //Threshold loader (helper lambda)
        auto loadBand = [&](const std::string& prefix, ThresholdBand& band) {
            //mode (optional)
            std::string mode;
            if (getString(kv, prefix + ".mode", mode)) {
                //ignoring case for error handline first
                if (equalsIgnoreCase(mode, "highonly")) band.mode = ThresholdMode::HighOnly;
                else if (equalsIgnoreCase(mode, "lowonly")) band.mode = ThresholdMode::LowOnly;
                else if (equalsIgnoreCase(mode, "range")) band.mode = ThresholdMode::Range;
                else res.warnings.push_back("Unknown " + prefix + ".mode: " + mode + " (keeping default)");
            }
            //sorts and sets modes
            setOptDouble(kv, prefix + ".lowWarn", band.lowWarn);
            setOptDouble(kv, prefix + ".lowCrit", band.lowCrit);
            setOptDouble(kv, prefix + ".highWarn", band.highWarn);
            setOptDouble(kv, prefix + ".highCrit", band.highCrit);
            };

        //loads each band individually.
        loadBand("threshold.temperatureC", res.config.thresholds.temperatureC);
        loadBand("threshold.humidityRH", res.config.thresholds.humidityRH);
        loadBand("threshold.pressureHpa", res.config.thresholds.pressureHpa);
        loadBand("threshold.co2Ppm", res.config.thresholds.co2Ppm);
        loadBand("threshold.radiationCpm", res.config.thresholds.radiationCpm);

        //Basic validation: warn if bands are incomplete for their mode
        validateThreshold("threshold.temperatureC", res.config.thresholds.temperatureC, res.warnings);
        validateThreshold("threshold.humidityRH", res.config.thresholds.humidityRH, res.warnings);
        validateThreshold("threshold.pressureHpa", res.config.thresholds.pressureHpa, res.warnings);
        validateThreshold("threshold.co2Ppm", res.config.thresholds.co2Ppm, res.warnings);
        validateThreshold("threshold.radiationCpm", res.config.thresholds.radiationCpm, res.warnings);

        //No hard-fail unless yhere are parsing errors
        res.ok = res.errors.empty();
        return res;
    }

    //splits file inot lines, rmeoves comments and extracts key-value pairs in a hash map.
    std::unordered_map<std::string, std::string> ConfigLoader::parseKeyValue(
        //takes enitre converted config file as one big string
        const std::string& text,
        //if malformed lines appear we push warnings into this vector.
        std::vector<std::string>& warnings
    ) {
        //create output map
        std::unordered_map<std::string, std::string> out;

        //converts text into stream so it can be read lien by line like a file
        std::istringstream in(text);
        //variables for loop, line stores current line and line no keeps track for error messages.
        std::string line;
        int lineNo = 0;

        //reads one line at a time, stops at a new line and returns false when no more lines ot sotp loop
        while (std::getline(in, line)) {
            lineNo++;

            //Strip comments (# or ;)
            line = stripComment(line);
            line = trim(line);
            if (line.empty()) continue;

            //if no equal sign them malformed add warning and continue
            auto eq = line.find('=');
            if (eq == std::string::npos) {
                warnings.push_back("ConfigLoader: ignoring malformed line " + std::to_string(lineNo) + " (no '=')");
                continue;
            }
            //key is before the eq sign, value is after.
            std::string key = trim(line.substr(0, eq));
            std::string val = trim(line.substr(eq + 1));

            //if no key, ignore the line
            if (key.empty()) {
                warnings.push_back("ConfigLoader: ignoring line " + std::to_string(lineNo) + " (empty key)");
                continue;
            }

            //push the line to the output map.
            out[key] = val;
        }

        return out;
    }

    //checks logic completeness, adds warning but does not crash if somehting is missing.
    void ConfigLoader::validateThreshold(const std::string& name, const ThresholdBand& t, std::vector<std::string>& warnings) {
        //
        auto needHigh = (t.mode == ThresholdMode::HighOnly || t.mode == ThresholdMode::Range);
        auto needLow = (t.mode == ThresholdMode::LowOnly || t.mode == ThresholdMode::Range);

        //if value is needed add warning 
        if (needHigh && (!t.highWarn.has_value() || !t.highCrit.has_value()))
            warnings.push_back(name + ": highWarn/highCrit not fully set for this mode");

        if (needLow && (!t.lowWarn.has_value() || !t.lowCrit.has_value()))
            warnings.push_back(name + ": lowWarn/lowCrit not fully set for this mode");
    }

    //cmpares two strings ignoring case 
    bool ConfigLoader::equalsIgnoreCase(const std::string& a, const std::string& b) {
        //first compare sizes, if not the same size they are alreayd not equal.
        if (a.size() != b.size()) return false;
        //then turn to lowercase and compare char by char. as soon as a character is not the same return false
        for (size_t i = 0; i < a.size(); i++) {
            if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i])) return false;
        }
        //otherwise return true.
        return true;
    }

    //strip comments from a string..
    std::string ConfigLoader::stripComment(const std::string& s) {
        //set chars to remove
        size_t p1 = s.find('#');
        size_t p2 = s.find(';');
        //if p1 is not found, use s.size() otherwise use p1
        size_t pos1 = (p1 == std::string::npos) ? s.size() : p1;
        size_t pos2 = (p2 == std::string::npos) ? s.size() : p2;
        //get the eraliers comment (smallest position)
        size_t p = std::min(pos1, pos2);
        //then get the string up to but not including that comment.
        return s.substr(0, p);
    }

    //classic trim funcion for leading or trailing whitespace, tabs etc.
    std::string ConfigLoader::trim(const std::string& s) {
        //start form left, pos 0
        size_t i = 0;
        //move forward and track i while the current character isspace. (unsigned prevent sissues wiht negative char values)
        while (i < s.size() && std::isspace((unsigned char)s[i])) i++;
        //then start from the end & repeat
        size_t j = s.size();
        while (j > i && std::isspace((unsigned char)s[j - 1])) j--;
        //return string wiht no whitespace
        return s.substr(i, j - i);
    }

    //bool fucntion that looks for a key  inside a map, copies the value into the  out
    bool ConfigLoader::getString(const std::unordered_map<std::string, std::string>& kv,
        // the key we want
        const std::string& key,
        //output param.
        std::string& out) {
        //search the map to find the key.
        auto it = kv.find(key);
        //if doesn't exsist, return false
        if (it == kv.end()) return false;
        //otherwise copy the value into the out file & return true.
        out = it->second;
        return true;
    }

    //looks up a key in the config map, assigns its value to target.
    void ConfigLoader::setString(const std::unordered_map<std::string, std::string>& kv,
        const std::string& key,
        //what we want
        std::string& target,
        std::vector<std::string>& warnings) {
        //temp variable to avoid modifying target.
        std::string v;
        //lookup using get string func then assign target to equal copy of value.
        if (getString(kv, key, v)) target = v;
        //ignores warnings for now
        (void)warnings;
    }

    //attempts to convert a string to an unsigned integer
    bool ConfigLoader::parseUInt(const std::string& s, uint32_t& out) {
        try {
            // tries to tuen v from a string to unsigned long int
            unsigned long v = std::stoul(s);
            //then casts to uint32 (32-bit int) 
            out = (uint32_t)v;
            return true;
        }
        catch (...) {
            return false;
        }
    }

    //Attempts to turn string to double.
    bool ConfigLoader::parseDouble(const std::string& s, double& out) {
        try {
            out = std::stod(s);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    //If key exsists, attempt parse, if invalid add warning otherwise assign 
    void ConfigLoader::setUInt(const std::unordered_map<std::string, std::string>& kv,
        const std::string& key,
        uint32_t& target,
        std::vector<std::string>& warnings) {
        auto it = kv.find(key);
        if (it == kv.end()) return;

        uint32_t v = 0;
        if (!parseUInt(it->second, v)) {
            warnings.push_back("ConfigLoader: invalid uint for " + key + " = " + it->second + " (keeping default)");
            return;
        }
        target = v;
    }

    //same logic as setUInt()
    void ConfigLoader::setDouble(const std::unordered_map<std::string, std::string>& kv,
        const std::string& key,
        double& target,
        std::vector<std::string>& warnings) {
        auto it = kv.find(key);
        if (it == kv.end()) return;

        double v = 0.0;
        if (!parseDouble(it->second, v)) {
            warnings.push_back("ConfigLoader: invalid double for " + key + " = " + it->second + " (keeping default)");
            return;
        }
        target = v;
    }

    //used for optional thresholds, if key exists and parses, assign value otherwise silently ignore.
    void ConfigLoader::setOptDouble(const std::unordered_map<std::string, std::string>& kv,
        const std::string& key,
        std::optional<double>& target) {
        auto it = kv.find(key);
        if (it == kv.end()) return;

        double v = 0.0;
        if (!parseDouble(it->second, v)) return; // silently ignore bad optional
        target = v;
    }

}