#pragma once
#include <yaml-cpp/yaml.h>

/**
 * Parser for the Pack configuration
 */
namespace PVE {
    class ConfigParser {
    public:
        ConfigParser() = default;

        void LoadSoundData() {
            std::filesystem::path configPath = "Data/Sound/PlayerVoiceEvents/SoundData/config.pve";
            if (exists(configPath) && is_regular_file(configPath)) {
                int result = ParseJson(configPath);
                if (result == 1) {
                    result = ParseYaml(configPath);
                }
                if (result == 0) {
                    Utils::Log("Info", "Sound-Data Config loaded.");
                    return;
                }
                if (result == 1) {
                    Utils::Log("Error", "Could not load Sound-Data Config: No valid JSON or YAML syntax.");
                    return;
                }
            }
            Utils::Log("Info", std::format("Could not load Sound-Data Config: File '{}' not found", configPath.string()));
        }

    private:
        int ParseJson(const std::filesystem::path &path) {
            std::ifstream packConfigFile(path);
            if (!packConfigFile.is_open()) {
                return 2;
            }
            nlohmann::json json;
            try {
                packConfigFile >> json;
            } catch (const nlohmann::json::parse_error &) {
                return 1;
            }
            for (const auto &[key, value] : json["sounds"].items()) {
                std::vector<std::pair<std::string, std::vector<std::string>>> audios;
                if (value.contains("audio") && value["audio"].is_array()) {
                    for (auto audio : value["audio"].get<std::vector<nlohmann::basic_json<>>>()) {
                        std::string condition;
                        std::vector<std::string> files;
                        if (!audio.contains("files") || !audio["files"].is_array() || audio["files"].empty()) {
                            continue;
                        }
                        files = audio["files"].get<std::vector<std::string>>();
                        if (audio.contains("condition") && audio["condition"].is_string() && !audio["condition"].get<std::string>().empty()) {
                            condition = audio["condition"].get<std::string>();
                        }
                        audios.emplace_back(condition, files);
                    }
                }
                if (audios.empty()) {
                    continue;
                }
                SoundEvent soundEvent(value.contains("chance") && value["chance"].is_number_integer() ? value["chance"].get<int>() : 100,
                                      value.contains("cooldown") && value["cooldown"].is_number_float() ? value["cooldown"].get<float>() : 0.0f,
                                      value.contains("canBeOverridden") && value["canBeOverridden"].is_boolean() ? value["canBeOverridden"].get<bool>() : false,
                                      value.contains("forceOverrideOthers") && value["forceOverrideOthers"].is_boolean() ? value["forceOverrideOthers"].get<bool>() : false,
                                      value.contains("delay") && value["delay"].is_number_float() ? value["delay"].get<float>() : 0.0f,
                                      value.contains("volume") && value["volume"].is_number_float() ? value["volume"].get<float>() : 1.0f,
                                      audios,
                                      value.contains("lipped") && value["lipped"].is_boolean() ? value["lipped"].get<bool>() : true);
                registeredSoundEvents[key] = soundEvent;
                eventCooldownMap[key] = false;
            }
            packConfigFile.close();
            return 0;
        }

        int ParseYaml(const std::filesystem::path &path) {
            YAML::Node yaml;
            try {
                yaml = YAML::LoadFile(path.string());
            } catch (YAML::BadFile) {
                return 2;
            } catch (const YAML::ParserException) {
                return 1;
            }

            if (yaml["sounds"]) {
                for (const auto &entry : yaml["sounds"]) {
                    std::string key = entry.first.as<std::string>();  // Get the key name
                    const YAML::Node &value = entry.second;

                    std::vector<std::pair<std::string, std::vector<std::string>>> audios;
                    if (value["audio"] && value["audio"].IsSequence()) {
                        for (const auto &audio : value["audio"]) {
                            std::string condition;
                            std::vector<std::string> files;

                            if (audio["files"] && audio["files"].IsSequence()) {
                                for (const auto &file : audio["files"]) {
                                    files.push_back(file.as<std::string>());
                                }
                            }
                            condition = audio["condition"] && audio["condition"].IsScalar() ? audio["condition"].as<std::string>() : "";
                            audios.emplace_back(condition, files);
                        }
                    }

                    if (audios.empty()) {
                        continue;
                    }

                    SoundEvent soundEvent(value["chance"] && value["chance"].IsScalar() ? value["chance"].as<int>() : 100,
                                          value["cooldown"] && value["cooldown"].IsScalar() ? value["cooldown"].as<float>() : 0.0f,
                                          value["canBeOverridden"] && value["canBeOverridden"].IsScalar() ? value["canBeOverridden"].as<bool>() : false,
                                          value["forceOverrideOthers"] && value["forceOverrideOthers"].IsScalar() ? value["forceOverrideOthers"].as<bool>() : false,
                                          value["delay"] && value["delay"].IsScalar() ? value["delay"].as<float>() : 0.0f,
                                          value["volume"] && value["volume"].IsScalar() ? value["volume"].as<float>() : 1.0f,
                                          audios,
                                          value["lipped"] && value["lipped"].IsScalar() ? value["lipped"].as<bool>() : true);

                    registeredSoundEvents[key] = soundEvent;
                    eventCooldownMap[key] = false;
                }
            }
            return 0;
        }
    };
}