// ReSharper disable CppMemberFunctionMayBeStatic
#include "SoundEvent.h"

#include "ConditionManager.h"
#include "Utils.h"

namespace PVE {

    SoundEvent::SoundEvent(const int chance, const float cooldown, const bool canBeOverridden, const bool forceOverrideOthers, const float delay, const float volume, const std::vector<std::pair<std::string, std::vector<std::string>>>& files, const bool isLipped) {
        this->chance = chance;
        this->cooldown = cooldown;
        this->canBeOverridden = canBeOverridden;
        this->forceOverrideOthers = forceOverrideOthers;
        this->delay = delay;
        this->volume = volume;
        this->audios = files;
        this->isLipped = isLipped;
    }

    bool SoundEvent::Play(const std::string& eventName) {
        if (!currentSound.has_value() || !currentSound->IsPlaying() || (currentSound->CanBeOverridden() || forceOverrideOthers)) {
            if (eventCooldownMap[eventName] == 0.0f) {
                if (Utils::GenerateRandomInt(0, 99) < chance) {
                    for (auto [condition, files] : this->audios) {
                        if (ConditionManager::EvaluateCondition(eventName, condition)) {
                            // we can safely remove the dynamic conditions for this event, since they have been evaluated to true already
                            ConditionManager::ResetDynamicConditions(eventName);
                            if (currentSound.has_value()) {
                                currentSound->Stop();
                            }
                            const auto file = files.at(Utils::GenerateRandomInt(0, static_cast<int>(files.size() - 1)));
                            RE::BSResource::ID id;
                            id.GenerateFromPath(std::format("Sound/PlayerVoiceEvents/SoundData/{}", file).c_str());
                            RE::BSAudioManager::GetSingleton()->BuildSoundDataFromFile(handle, id, 0x1A, 1);
                            handle.SetVolume(this->volume);
                            handle.SetObjectToFollow(RE::PlayerCharacter::GetSingleton()->GetCurrent3D());
                            handle.Play();
                            Utils::Log("Debug", std::format("Played sound '{}'", file));
                            currentSound.emplace(*this);
                            eventCooldownMap[eventName] = this->cooldown;
                            return true;
                        }
                    }
                }
            }
        }
        ConditionManager::ResetDynamicConditions(eventName);
        return false;
    }

    void SoundEvent::Stop() {
        if (handle.IsValid()) {
            handle.Stop();
        }
    }

    bool SoundEvent::IsPlaying() const {
        return handle.IsPlaying();
    }

    std::vector<std::pair<std::string, std::vector<std::string>>> SoundEvent::GetAudioFiles() {
        return this->audios;
    }

    RE::BSSoundHandle SoundEvent::GetHandle() {
        return this->handle;
    }

    int SoundEvent::GetChance() {
        return this->chance;
    }

    float SoundEvent::GetCooldown() {
        return this->cooldown;
    }

    bool SoundEvent::CanBeOverridden() {
        return this->canBeOverridden;
    }

    bool SoundEvent::IsForceOverrideOthers() {
        return this->forceOverrideOthers;
    }

    float SoundEvent::GetDelay() {
        return this->delay;
    }

    float SoundEvent::GetVolume() {
        return this->volume;
    }

    bool SoundEvent::IsLipped() {
        return this->isLipped;
    }
}