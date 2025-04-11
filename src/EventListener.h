#pragma once

namespace PVE {
    // Those can be registered once per game start, and they'll stay registered.
    class DefaultEventSink final : public RE::BSTEventSink<RE::TESPlayerBowShotEvent>,
                                   public RE::BSTEventSink<RE::TESHitEvent>,
                                   public RE::BSTEventSink<SKSE::ActionEvent>,
                                   public RE::BSTEventSink<RE::TESSleepStartEvent>,
                                   public RE::BSTEventSink<RE::TESSleepStopEvent>,
                                   public RE::BSTEventSink<RE::TESContainerChangedEvent>,
                                   public RE::BSTEventSink<RE::TESQuestStageEvent>,
                                   public RE::BSTEventSink<RE::TESFurnitureEvent>,
                                   public RE::BSTEventSink<RE::TESSpellCastEvent>,
                                   public RE::BSTEventSink<RE::LocationDiscovery::Event> {
    public:
        RE::BSEventNotifyControl ProcessEvent(const RE::TESPlayerBowShotEvent *event, RE::BSTEventSource<RE::TESPlayerBowShotEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent *event, RE::BSTEventSource<RE::TESHitEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const SKSE::ActionEvent *event, RE::BSTEventSource<SKSE::ActionEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const RE::TESSleepStartEvent *event, RE::BSTEventSource<RE::TESSleepStartEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const RE::TESSleepStopEvent *event, RE::BSTEventSource<RE::TESSleepStopEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent *event, RE::BSTEventSource<RE::TESContainerChangedEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const RE::TESQuestStageEvent *event, RE::BSTEventSource<RE::TESQuestStageEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const RE::TESFurnitureEvent *event, RE::BSTEventSource<RE::TESFurnitureEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const RE::TESSpellCastEvent *event, RE::BSTEventSource<RE::TESSpellCastEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const RE::LocationDiscovery::Event *event, RE::BSTEventSource<RE::LocationDiscovery::Event> *) override;

        static void Register() {
            static DefaultEventSink sink;
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESPlayerBowShotEvent>(&sink);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESHitEvent>(&sink);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESSleepStartEvent>(&sink);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESSleepStopEvent>(&sink);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESContainerChangedEvent>(&sink);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESQuestStageEvent>(&sink);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESFurnitureEvent>(&sink);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESSpellCastEvent>(&sink);
            RE::LocationDiscovery::GetEventSource()->AddEventSink<RE::LocationDiscovery::Event>(&sink);
            SKSE::GetActionEventSource()->AddEventSink(&sink);
        }
    };

    // Those need to be registered every time a save is loaded.
    class DynamicEventSink final : public RE::BSTEventSink<RE::BSAnimationGraphEvent>, public RE::BSTEventSink<SKSE::CameraEvent>, public RE::BSTEventSink<RE::BGSActorCellEvent> {
    public:
        RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent *event, RE::BSTEventSource<RE::BSAnimationGraphEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const SKSE::CameraEvent *event, RE::BSTEventSource<SKSE::CameraEvent> *) override;

        RE::BSEventNotifyControl ProcessEvent(const RE::BGSActorCellEvent *event, RE::BSTEventSource<RE::BGSActorCellEvent> *) override;

        static void Register() {
            static DynamicEventSink sink;
            RE::PlayerCharacter::GetSingleton()->RemoveAnimationGraphEventSink(&sink);
            SKSE::GetCameraEventSource()->RemoveEventSink(&sink);
            RE::PlayerCharacter::GetSingleton()->AsBGSActorCellEventSource()->RemoveEventSink(&sink);
            RE::PlayerCharacter::GetSingleton()->AddAnimationGraphEventSink(&sink);
            SKSE::GetCameraEventSource()->AddEventSink(&sink);
            RE::PlayerCharacter::GetSingleton()->AsBGSActorCellEventSource()->AddEventSink(&sink);
        }
    };
}
