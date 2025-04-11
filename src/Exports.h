// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
#pragma once
#pragma warning(disable : 4190)

#include "ConditionManager.h"
#include "Utils.h"

using Value = std::variant<float, int, bool, std::string>;

extern "C" {
__declspec(dllexport) void PVE_PlaySound(const std::string &eventName) {
    PVE::Utils::PlaySound(eventName);
}

__declspec(dllexport) void PVE_StopSound() {
    if (PVE::currentSound && PVE::currentSound.has_value()) PVE::currentSound->Stop();
}

__declspec(dllexport) void PVE_Log(const std::string &prefix, const std::string &msg) {
    PVE::Utils::Log(prefix, msg);
}

__declspec(dllexport) std::string PVE_Replace(const std::string &text, const std::string &oldSeq, const std::string &newSeq) {
    return PVE::Utils::Replace(text, oldSeq, newSeq);
}

__declspec(dllexport) void PVE_RegisterCondition(const std::string &conditionName, const std::function<Value()> &conditionFunction) {
    PVE::ConditionManager::RegisterCondition(conditionName, conditionFunction);
}

__declspec(dllexport) void PVE_RegisterDynamicCondition(const std::string &eventName, const std::string &conditionName, const std::function<Value()> &conditionFunction) {
    PVE::ConditionManager::RegisterDynamicCondition(eventName, conditionName, conditionFunction);
}
}