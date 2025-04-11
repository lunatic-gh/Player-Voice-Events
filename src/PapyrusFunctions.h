#pragma once

namespace PVE {
    class PapyrusFunctions {
    public:
        static bool Funcs(RE::BSScript::IVirtualMachine *vm);

    private:
        static void PlaySound(RE::StaticFunctionTag *, std::string eventName);

        static void StopSound(RE::StaticFunctionTag *);

        static void Log(RE::StaticFunctionTag *, std::string msg);

        static std::string Replace(RE::StaticFunctionTag *, std::string text, std::string oldSeq, std::string newSeq);

        static void RegisterFloatCondition(RE::StaticFunctionTag *, std::string name, float value);

        static void RegisterIntCondition(RE::StaticFunctionTag *, std::string name, int value);

        static void RegisterBoolCondition(RE::StaticFunctionTag *, std::string name, bool value);

        static void RegisterStringCondition(RE::StaticFunctionTag *, std::string name, std::string value);

        static void RegisterDynamicFloatCondition(RE::StaticFunctionTag *, std::string eventName, std::string name, float value);

        static void RegisterDynamicIntCondition(RE::StaticFunctionTag *, std::string eventName, std::string name, int value);

        static void RegisterDynamicBoolCondition(RE::StaticFunctionTag *, std::string eventName, std::string name, bool value);

        static void RegisterDynamicStringCondition(RE::StaticFunctionTag *, std::string eventName, std::string name, std::string value);
    };
}
