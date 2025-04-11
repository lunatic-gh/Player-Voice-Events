#include "Utils.h"

#include <nlohmann/json.hpp>

#include "ConditionManager.h"
#include "ConfigParser.h"
#include "Logger.h"
#include "SoundEvent.h"

namespace PVE {
    // TODO: Add more specific error logging
    void Utils::LoadData() {
        // Load Location Data
        if (auto dataHandler = RE::TESDataHandler::GetSingleton()) {
            for (auto form : dataHandler->GetFormArray(RE::FormType::WorldSpace)) {
                if (auto worldspace = form->As<RE::TESWorldSpace>()) {
                    const char *wsName = worldspace->GetName();
                    if (wsName && *wsName != '\0') {
                        std::vector<std::pair<RE::TESObjectREFR *, float>> markerList;
                        if (auto cell = worldspace->persistentCell) {
                            // ForEachReferences crashes on AE it seems, so we do it manually
                            for (auto &ref : cell->GetRuntimeData().references) {
                                if (auto marker = ref->extraList.GetByType<RE::ExtraMapMarker>()) {
                                    if (marker->mapData) {
                                        float radius = 1000.0f;
                                        if (auto markerRadius = ref->extraList.GetByType<RE::ExtraRadius>()) {
                                            radius = markerRadius->radius;
                                        }
                                        markerList.push_back(std::make_pair(ref.get(), radius));
                                    }
                                }
                            }
                        }
                        if (!markerList.empty()) {
                            markerMap[wsName] = std::move(markerList);
                        }
                    }
                }
            }
        }
        const std::string markerDataOverridePath = "Data/Sound/PlayerVoiceEvents/MarkerOverrides";
        for (const auto &entry : std::filesystem::directory_iterator(markerDataOverridePath)) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != ".json") continue;
            std::ifstream markerDataFile(entry.path());
            if (!markerDataFile.is_open()) {
                Log("Error", std::format("Warning: Could not open File '{}'", entry.path().string()));
                continue;
            }
            nlohmann::json json;
            try {
                markerDataFile >> json;
            } catch (const nlohmann::json::parse_error &) {
                Log("Error", std::format("Warning: Could not open File '{}' - Please make sure it is in proper json-format.", entry.path().string()));
                continue;
            }
            if (!json.contains("worldspace") || !json["worldspace"].is_string() || !json.contains("markerID") || !json["markerID"].is_string()) {
                Log("Error", std::format("Warning: Ignoring Data File '{}', because it has missing or corrupt data. Please check it for errors.", entry.path().string()));
                continue;
            }

            auto worldspaceName = json["worldspace"].get<std::string>();
            auto markerIDStr = SplitByChar(json["markerID"].get<std::string>(), '|');
            std::string markerPlugin = "";
            int markerID = -1;
            try {
                markerPlugin = markerIDStr[0];
                markerID = std::stoi(markerIDStr[1], nullptr, 16);
            } catch (std::invalid_argument) {
                Log("Error", std::format("Warning: Ignoring Data File '{}', because it has missing or corrupt data. Please check it for errors.", entry.path().string()));
            } catch (std::out_of_range) {
                Log("Error", std::format("Warning: Ignoring Data File '{}', because it has missing or corrupt data. Please check it for errors.", entry.path().string()));
            }

            if (!markerMap.contains(worldspaceName)) continue;
            bool enabled = json.value("enabled", true);
            auto &markerList = markerMap[worldspaceName];
            if (!enabled) {
                std::erase_if(markerList, [markerPlugin, markerID](const std::pair<RE::TESObjectREFR *, float> &pair) {
                    return pair.first && pair.first->GetFile()->fileName == markerPlugin && static_cast<int>(pair.first->GetFormID()) == markerID;
                });
            } else if (json.contains("radius") && json["radius"].is_number_float()) {
                for (auto &pair : markerList) {
                    if (pair.first && pair.first->GetFile()->fileName == markerPlugin && static_cast<int>(pair.first->GetFormID()) == markerID) {
                        float oldRadius = pair.second;
                        float newRadius = json["radius"].get<float>();
                        pair.second = newRadius;
                    }
                }
            }
        }
        Log("Info", "Loaded MapMarker-Data");

        ConfigParser configParser;
        configParser.LoadSoundData();
    }

    void Utils::PlaySound(const std::string &eventName) {
        if (!CanPlaySound()) {
            ConditionManager::ResetDynamicConditions(eventName);
            return;
        }
        const auto eventIt = registeredSoundEvents.find(eventName);
        Log("Debug", std::format("Received Event '{}'", eventName));
        SoundEvent event;
        if (eventIt != registeredSoundEvents.end()) {
            event = eventIt->second;
        } else {
            ConditionManager::ResetDynamicConditions(eventName);
            return;
        }
        std::thread([event, eventName]() mutable {
            float delaySeconds = event.GetDelay();
            while (delaySeconds > 0.0f) {
                if (RE::UI::GetSingleton() && !RE::UI::GetSingleton()->GameIsPaused() && RE::PlayerCharacter::GetSingleton() && RE::PlayerCharacter::GetSingleton()->Is3DLoaded()) {
                    delaySeconds -= 0.05f;
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }
            event.Play(eventName);
        }).detach();
    }

    bool Utils::CanPlaySound() {
        const auto player = RE::PlayerCharacter::GetSingleton();
        const auto ui = RE::UI::GetSingleton();
        if (!player || !ui) {
            return false;
        }
        if (!player->Is3DLoaded() || ui->IsMenuOpen("Loading Menu")) {
            return false;
        }
        return true;
    }

    void Utils::Log(const std::string &prefix, const std::string &msg) {
        logger::info("[{}] {}", prefix, msg);
    }

    std::string Utils::Replace(const std::string &text, const std::string &oldSeq, const std::string &newSeq) {
        if (oldSeq.empty()) {
            return text;
        }
        std::string result = text;
        size_t pos = 0;
        while ((pos = result.find(oldSeq, pos)) != std::string::npos) {
            result.replace(pos, oldSeq.length(), newSeq);
            pos += newSeq.length();
        }
        return result;
    }

    int Utils::GenerateRandomInt(const int minInclusive, const int maxInclusive) {
        std::random_device dev;
        std::mt19937 gen(dev());
        std::uniform_int_distribution range(minInclusive, maxInclusive);
        return range(gen);
    }
    float Utils::GenerateRandomFloat(float minInclusive, float maxInclusive) {
        std::random_device dev;
        std::mt19937 gen(dev());
        std::uniform_real_distribution range(minInclusive, maxInclusive);
        return range(gen);
    }

    void Utils::RunConsoleCommand(const std::string &command) {
        const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
        if (const auto script = scriptFactory ? scriptFactory->Create() : nullptr) {
            script->SetCommand(command);
            CompileAndRun(script, GetSelectedRef().get());
            delete script;
        }
    }

    bool Utils::FormHasKeywordString(RE::TESForm *form, const std::string &keyword) {
        if (form) {
            if (const auto keywordForm = form->As<RE::BGSKeywordForm>()) {
                return keywordForm->HasKeywordString(keyword);
            }
        }
        return false;
    }
    std::string Utils::GetFormKeywordsFormatted(RE::TESForm *form) {
        std::string s = "";
        if (form) {
            if (const auto keywordForm = form->As<RE::BGSKeywordForm>()) {
                auto keywords = keywordForm->GetKeywords();
                for (int i = 0; i < keywords.size(); ++i) {
                    s += keywords[i]->GetFormEditorID();
                    if (i != keywords.size() - 1) {
                        s += "|";
                    }
                }
            }
        }
        return s;
    }

    float Utils::CalculateDistance(const float x1, const float y1, const float x2, const float y2) {
        return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    }

    std::vector<std::string> Utils::SplitByChar(const std::string &input, const char &delimiter) {
        std::vector<std::string> result;
        std::stringstream ss(input);
        std::string s;
        while (std::getline(ss, s, delimiter))
            result.push_back(s);
        return result;
    }
    std::string Utils::TrimString(const std::string &s) {
        auto view = std::string_view(s);
        view.remove_prefix(std::min(s.find_first_not_of(" \t\r\v\n"), s.size()));
        view.remove_suffix(std::min(s.size() - s.find_last_not_of(" \t\r\v\n") - 1, s.size()));
        return std::string(view);
    }

    // ReSharper disable once CppDFAConstantParameter
    void Utils::CompileAndRun(RE::Script *script, RE::TESObjectREFR *targetRef, const RE::COMPILER_NAME name) {
        RE::ScriptCompiler compiler;
        CompileAndRunImpl(script, &compiler, name, targetRef);
    }

    void Utils::CompileAndRunImpl(RE::Script *script, RE::ScriptCompiler *compiler, RE::COMPILER_NAME name, RE::TESObjectREFR *targetRef) {
        using func_t = decltype(CompileAndRunImpl);
        const REL::Relocation<func_t> func{RELOCATION_ID(21416, REL::Module::get().version().patch() < 1130 ? 21890 : 441582)};
        return func(script, compiler, name, targetRef);
    }

    RE::NiPointer<RE::TESObjectREFR> Utils::GetSelectedRef() {
        const auto handle = GetSelectedRefHandle();
        return handle.get();
    }

    RE::ObjectRefHandle Utils::GetSelectedRefHandle() {
        const REL::Relocation<RE::ObjectRefHandle *> selectedRef{RELOCATION_ID(519394, REL::Module::get().version().patch() < 1130 ? 405935 : 504099)};
        return *selectedRef;
    }
}