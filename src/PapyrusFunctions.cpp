// ReSharper disable CppPassValueParameterByConstReference
#include "PapyrusFunctions.h"

#include "ConditionManager.h"
#include "SoundEvent.h"
#include "Utils.h"

namespace PVE {
    void PapyrusFunctions::PlaySound(RE::StaticFunctionTag *, std::string eventName) {
        Utils::PlaySound(eventName);
    }

    void PapyrusFunctions::StopSound(RE::StaticFunctionTag *) {
        if (currentSound && currentSound.has_value()) {
            currentSound->Stop();
        }
    }

    void PapyrusFunctions::Log(RE::StaticFunctionTag *, std::string msg) {
        Utils::Log("Papyrus", msg);
    }

    std::string PapyrusFunctions::Replace(RE::StaticFunctionTag *, std::string text, std::string oldSeq, std::string newSeq) {
        return Utils::Replace(text, oldSeq, newSeq);
    }

    void PapyrusFunctions::RegisterFloatCondition(RE::StaticFunctionTag *, std::string name, float value) {
        return ConditionManager::RegisterCondition(name, [value] {
            return value;
        });
    }

    void PapyrusFunctions::RegisterIntCondition(RE::StaticFunctionTag *, std::string name, int value) {
        return ConditionManager::RegisterCondition(name, [value] {
            return value;
        });
    }

    void PapyrusFunctions::RegisterBoolCondition(RE::StaticFunctionTag *, std::string name, bool value) {
        return ConditionManager::RegisterCondition(name, [value] {
            return value;
        });
    }

    void PapyrusFunctions::RegisterStringCondition(RE::StaticFunctionTag *, std::string name, std::string value) {
        return ConditionManager::RegisterCondition(name, [value] {
            return value;
        });
    }

    void PapyrusFunctions::RegisterDynamicFloatCondition(RE::StaticFunctionTag *, std::string eventName, std::string name, float value) {
        return ConditionManager::RegisterDynamicCondition(eventName, name, [value] {
            return value;
        });
    }

    void PapyrusFunctions::RegisterDynamicIntCondition(RE::StaticFunctionTag *, std::string eventName, std::string name, int value) {
        return ConditionManager::RegisterDynamicCondition(eventName, name, [value] {
            return value;
        });
    }

    void PapyrusFunctions::RegisterDynamicBoolCondition(RE::StaticFunctionTag *, std::string eventName, std::string name, bool value) {
        return ConditionManager::RegisterDynamicCondition(eventName, name, [value] {
            return value;
        });
    }

    void PapyrusFunctions::RegisterDynamicStringCondition(RE::StaticFunctionTag *, std::string eventName, std::string name, std::string value) {
        return ConditionManager::RegisterDynamicCondition(eventName, name, [value] {
            return value;
        });
    }

    bool PapyrusFunctions::Funcs(RE::BSScript::IVirtualMachine *vm) {
        vm->RegisterFunction("PlaySound", "LNTC_PVESKSEFunctions", PlaySound);
        vm->RegisterFunction("StopSound", "LNTC_PVESKSEFunctions", StopSound);
        vm->RegisterFunction("Log", "LNTC_PVESKSEFunctions", Log);
        vm->RegisterFunction("Replace", "LNTC_PVESKSEFunctions", Replace);
        vm->RegisterFunction("RegisterFloatCondition", "LNTC_PVESKSEFunctions", RegisterFloatCondition);
        vm->RegisterFunction("RegisterIntCondition", "LNTC_PVESKSEFunctions", RegisterIntCondition);
        vm->RegisterFunction("RegisterBoolCondition", "LNTC_PVESKSEFunctions", RegisterBoolCondition);
        vm->RegisterFunction("RegisterStringCondition", "LNTC_PVESKSEFunctions", RegisterStringCondition);
        vm->RegisterFunction("RegisterDynamicFloatCondition", "LNTC_PVESKSEFunctions", RegisterDynamicFloatCondition);
        vm->RegisterFunction("RegisterDynamicIntCondition", "LNTC_PVESKSEFunctions", RegisterDynamicIntCondition);
        vm->RegisterFunction("RegisterDynamicBoolCondition", "LNTC_PVESKSEFunctions", RegisterDynamicBoolCondition);
        vm->RegisterFunction("RegisterDynamicStringCondition", "LNTC_PVESKSEFunctions", RegisterDynamicStringCondition);
        return true;
    }

}