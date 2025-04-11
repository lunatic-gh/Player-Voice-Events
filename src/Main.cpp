#include "Main.h"

#include "ConditionManager.h"
#include "CustomEventListener.h"
#include "EventListener.h"

namespace PVE {
    void Main::Init() {
        Utils::LoadData();
        Utils::Log("Info", "Registering Events...");
        DefaultEventSink::Register();
        CustomEventListener::Register();
        ConditionManager::Init();
        Utils::Log("Info", "Done.");
        Utils::Log("Info", "Starting Loops...");
        loopManager.StartNew("Cooldowns", [] {
            while (eventCooldownMap.empty()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            for (auto& cd : eventCooldownMap | std::views::values) {
                if (cd == 0.0f || RE::UI::GetSingleton()->GameIsPaused() || !RE::PlayerCharacter::GetSingleton()->Is3DLoaded()) {
                    continue;
                }
                cd = std::max(0.0f, cd - 0.020f);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        });
        loopManager.StartNew("LipHandler", [] {
            static auto lastLipPlayTime = std::chrono::steady_clock::now();
            static bool isLipPlaying = false;

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (const auto player = RE::PlayerCharacter::GetSingleton()) {
                if (currentSound && currentSound.has_value()) {
                    if (auto sound = currentSound.value(); sound.IsLipped() && sound.GetHandle().IsPlaying()) {
                        if (const auto currentTime = std::chrono::steady_clock::now();
                            std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastLipPlayTime).count() >= 300 || !isLipPlaying) {
                            Utils::RunConsoleCommand("player.speaksound \"PlayerVoiceEvents/SoundData/PVE/lip_dummy.lip\"");
                            lastLipPlayTime = currentTime;
                            isLipPlaying = true;
                        }
                    } else if (isLipPlaying) {
                        player->EndDialogue();
                        isLipPlaying = false;
                    }
                }
            }
        });
        Utils::Log("Info", "Done.");
    }

    void Main::Load() {
        loopManager.StartNew("DynamicSink", [] {
            while (!RE::PlayerCharacter::GetSingleton()->Is3DLoaded()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            Utils::Log("Info", "Registering Dynamic Events...");
            DynamicEventSink::Register();
            Utils::Log("Info", "Done.");
            loopManager.Stop("DynamicSink");
        });
    }
}
