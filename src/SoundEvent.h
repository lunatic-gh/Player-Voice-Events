#pragma once

namespace PVE {
    class SoundEvent {
    public:
        explicit SoundEvent() = default;

        explicit SoundEvent(int chance, float cooldown, bool canBeOverridden, bool forceOverrideOthers, float delay, float volume, const std::vector<std::pair<std::string, std::vector<std::string>>> &files, const bool isLipped);

        bool Play(const std::string &eventName);

        void Stop();

        [[nodiscard]] bool IsPlaying() const;

        [[nodiscard]] std::vector<std::pair<std::string, std::vector<std::string>>> GetAudioFiles();

        [[nodiscard]] RE::BSSoundHandle GetHandle();

        [[nodiscard]] int GetChance();

        [[nodiscard]] float GetCooldown();

        [[nodiscard]] bool CanBeOverridden();

        [[nodiscard]] bool IsForceOverrideOthers();

        [[nodiscard]] float GetDelay();

        [[nodiscard]] float GetVolume();

        [[nodiscard]] bool IsLipped();

    private:
        static inline RE::BSSoundHandle handle;
        int chance = 100;
        float cooldown = 0.0f;
        bool canBeOverridden = false;
        bool forceOverrideOthers = false;
        float delay = 0.0f;
        float volume = 1.0f;
        std::vector<std::pair<std::string, std::vector<std::string>>> audios = std::vector<std::pair<std::string, std::vector<std::string>>>();
        bool isLipped = true;
    };
}