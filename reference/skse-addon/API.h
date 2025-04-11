#pragma once
#include <REX/W32/KERNEL32.h>

#include <functional>
#include <string>
#include <variant>

/**
 * You can include this header in your Project and directly use these functions. They are relayed from the PVE Mod directly.
 *
 * There's nothing to worry about in terms of License or anything. Include it, define it in your CMakeLists, and be happy <3
 */
namespace PVE {
    class API {
    public:
        static void PlaySound(const std::string &eventName) {
            reinterpret_cast<void (*)(const std::string &)>(GetProcAddress(pveHandle, "PVE_PlaySound"))(eventName);
        }

        static void StopSound() {
            reinterpret_cast<void (*)()>(GetProcAddress(pveHandle, "PVE_StopSound"))();
        }

        static void Log(const std::string &prefix, const std::string &msg) {
            reinterpret_cast<void (*)(const std::string &, const std::string &)>(GetProcAddress(pveHandle, "PVE_Log"))(prefix, msg);
        }

        static std::string Replace(const std::string &text, const std::string &oldSeq, const std::string &newSeq) {
            return reinterpret_cast<std::string (*)(const std::string &, const std::string &, const std::string &)>(GetProcAddress(pveHandle, "PVE_Replace"))(text, oldSeq, newSeq);
        }

        static void RegisterDynamicCondition(const std::string &eventName, const std::string &conditionName, const std::function<std::variant<float, int, bool, std::string>()> &conditionFunction) {
            reinterpret_cast<void (*)(const std::string &, const std::string &, const std::function<std::variant<float, int, bool, std::string>()> &)>(
                GetProcAddress(pveHandle, "PVE_RegisterDynamicCondition"))(eventName, conditionName, conditionFunction);
        }

    private:
        static inline REX::W32::HMODULE pveHandle = ::REX::W32::GetModuleHandleW(L"PlayerVoiceEvents");
    };
}
