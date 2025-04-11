#pragma once

#include <nlohmann/json.hpp>

#include "SoundEvent.h"

namespace PVE {

    inline std::unordered_map<std::string, std::vector<std::pair<RE::TESObjectREFR *, float>>> markerMap;
    inline std::optional<RE::TESObjectCELL *> currentCell;
    inline std::optional<RE::TESObjectREFR *> currentLocMarker;
    inline std::unordered_map<std::string, SoundEvent> registeredSoundEvents;
    inline std::optional<SoundEvent> currentSound;
    inline std::unordered_map<std::string, float> eventCooldownMap;
    inline std::map<int, std::vector<int>> questStageCooldownMap;

    class Utils {
    public:
        static void LoadData();

        static void PlaySound(const std::string &eventName);

        static bool CanPlaySound();

        static void Log(const std::string &prefix, const std::string &msg);

        static std::string Replace(const std::string &text, const std::string &oldSeq, const std::string &newSeq);

        static int GenerateRandomInt(int minInclusive, int maxInclusive);

        static float GenerateRandomFloat(float minInclusive, float maxInclusive);

        static void RunConsoleCommand(const std::string &command);

        static bool FormHasKeywordString(RE::TESForm *form, const std::string &keyword);

        static std::string GetFormKeywordsFormatted(RE::TESForm *form);

        static float CalculateDistance(float x1, float y1, float x2, float y2);

        static std::vector<std::string> SplitByChar(const std::string &input, const char &delimiter);

        static std::string TrimString(const std::string &s);

    private:
        static void CompileAndRun(RE::Script *script, RE::TESObjectREFR *targetRef, RE::COMPILER_NAME name = RE::COMPILER_NAME::kSystemWindowCompiler);

        static void CompileAndRunImpl(RE::Script *script, RE::ScriptCompiler *compiler, RE::COMPILER_NAME name, RE::TESObjectREFR *targetRef);

        static RE::NiPointer<RE::TESObjectREFR> GetSelectedRef();

        static RE::ObjectRefHandle GetSelectedRefHandle();
    };
};