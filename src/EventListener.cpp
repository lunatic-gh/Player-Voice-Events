#include "EventListener.h"

#include "ConditionParser.h"
#include "Utils.h"

namespace PVE {
    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESPlayerBowShotEvent *event, RE::BSTEventSource<RE::TESPlayerBowShotEvent> *) {
        Utils::PlaySound("PVEAttackBow", event->shotPower < 1.0f ? "PVEAttackBowLow" : "");
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESHitEvent *event, RE::BSTEventSource<RE::TESHitEvent> *) {
        const auto cause = event->cause;
        const auto target = event->target;
        const auto source = RE::TESForm::LookupByID(event->source);
        if (cause && cause->IsPlayerRef()) {
            return RE::BSEventNotifyControl::kContinue;
        }
        if (target && target->IsPlayerRef()) {
            if (!target->IsDead()) {
                if (target->As<RE::Actor>()->IsBlocking()) {
                    Utils::PlaySound("PVEBlockReceivedHit", event->flags.any(RE::TESHitEvent::Flag::kPowerAttack) ? "PVEBlockReceivedPowerHit" : "");
                } else {
                    if (cause && cause->As<RE::Actor>() && cause->As<RE::Actor>()->IsPlayerTeammate() && source && source->Is(RE::FormType::Spell) &&
                        Utils::FormHasKeywordString(source, "MagicRestoreHealth")) {
                        Utils::PlaySound("PVEReceivedFriendlyHeal");
                    } else {
                        if (!cause || (!cause->Is(RE::FormType::ActorCharacter) || cause->As<RE::Actor>()->IsInKillMove())) {
                            Utils::PlaySound("PVEReceivedHit", event->flags.any(RE::TESHitEvent::Flag::kPowerAttack) ? "PVEReceivedPowerHit" : "");
                        }
                    }
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DynamicEventSink::ProcessEvent(const RE::BSAnimationGraphEvent *event, RE::BSTEventSource<RE::BSAnimationGraphEvent> *) {
        if (event && event->holder) {
            const RE::TESObjectREFR *holder = event->holder;
            if (strcmp(event->tag.c_str(), "SoundPlay.NPCHumanCombatShieldBash") == 0) {
                if (holder->IsPlayerRef()) {
                    Utils::PlaySound("PVEBash");
                    return RE::BSEventNotifyControl::kContinue;
                }
            } else if (strcmp(event->tag.c_str(), "JumpUp") == 0) {
                Utils::PlaySound("PVEJump");
            } else if (strcmp(event->tag.c_str(), "FootSprintRight") == 0) {
                Utils::PlaySound("PVESprintIdle");
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const SKSE::ActionEvent *event, RE::BSTEventSource<SKSE::ActionEvent> *) {
        const auto type = event->type;
        const auto source = event->sourceForm;
        if (const auto actor = event->actor) {
            if (actor->IsPlayerRef()) {
                if (type == SKSE::ActionEvent::Type::kWeaponSwing) {
                    bool b = false;
                    if (const auto currentProcess = actor->GetActorRuntimeData().currentProcess) {
                        if (const auto high = currentProcess->high) {
                            if (const auto attackData = high->attackData) {
                                Utils::PlaySound(attackData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack) ? "PVEPowerAttackMelee" : "PVEAttackMelee");
                                return RE::BSEventNotifyControl::kContinue;
                            }
                        }
                    }
                    Utils::PlaySound("PVEAttackMelee");
                } else if (source && type == SKSE::ActionEvent::Type::kSpellCast) {
                    Utils::PlaySound("PVESpellCast", std::format("PVESpellCast{}", Utils::Replace(source->GetName(), " ", "")));
                } else if (source && type == SKSE::ActionEvent::Type::kSpellFire) {
                    Utils::PlaySound("PVESpellFire", std::format("PVESpellFire{}", Utils::Replace(source->GetName(), " ", "")));
                } else if (type == SKSE::ActionEvent::Type::kBeginDraw) {
                    if (source && Utils::FormHasKeywordString(source, "WeapTypeBow")) {
                        Utils::PlaySound("PVEUnsheathe", "PVEUnsheatheBow");
                    } else if (source && source->GetFormType() != RE::FormType::Spell) {
                        Utils::PlaySound("PVEUnsheathe", "PVEUnsheatheMelee");
                    } else {
                        Utils::PlaySound("PVEUnsheathe");
                    }
                } else if (type == SKSE::ActionEvent::Type::kEndDraw) {
                    if (source && source->GetFormType() == RE::FormType::Spell) {
                        Utils::PlaySound("PVEUnsheathe", "PVEUnsheatheSpell");
                    }
                } else if (type == SKSE::ActionEvent::Type::kBeginSheathe) {
                    if (source && Utils::FormHasKeywordString(source, "WeapTypeBow")) {
                        Utils::PlaySound("PVESheathe", "PVESheatheBow");
                    } else if (source && source->GetFormType() != RE::FormType::Spell) {
                        Utils::PlaySound("PVESheathe", "PVESheatheMelee");
                    } else {
                        Utils::PlaySound("PVESheathe");
                    }
                } else if (type == SKSE::ActionEvent::Type::kEndSheathe) {
                    if (source && source->GetFormType() == RE::FormType::Spell) {
                        Utils::PlaySound("PVESheathe", "PVESheatheSpell");
                    }
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESSleepStartEvent *, RE::BSTEventSource<RE::TESSleepStartEvent> *) {
        Utils::PlaySound("PVESleepStart");
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESSleepStopEvent *, RE::BSTEventSource<RE::TESSleepStopEvent> *) {
        Utils::PlaySound("PVESleepEnd");
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESContainerChangedEvent *event, RE::BSTEventSource<RE::TESContainerChangedEvent> *) {
        if (event->newContainer == RE::PlayerCharacter::GetSingleton()->GetFormID()) {
            switch (RE::TESForm::LookupByID(event->baseObj)->GetFormType()) {
                case RE::FormType::Armor:
                    Utils::PlaySound("PVEPickupItem", "PVEPickupItemArmor");
                    break;
                case RE::FormType::Book:
                    Utils::PlaySound("PVEPickupItem", "PVEPickupItemBook");
                    break;
                case RE::FormType::Weapon:
                    Utils::PlaySound("PVEPickupItem", "PVEPickupItemWeapon");
                    break;
                case RE::FormType::Ammo:
                    Utils::PlaySound("PVEPickupItem", "PVEPickupItemAmmo");
                    break;
                case RE::FormType::AlchemyItem:
                    Utils::PlaySound("PVEPickupItem", "PVEPickupItemPotion");
                    break;
                default:
                    Utils::PlaySound("PVEPickupItem");
                    break;
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESQuestStageEvent *event, RE::BSTEventSource<RE::TESQuestStageEvent> *) {
        if (event->formID != 0) {
            const auto stage = event->stage;
            for (auto questData : quests) {
                auto [fst, snd] = std::get<1>(questData);
                const auto refQuest = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(snd, fst);
                if (refQuest && refQuest->GetFormID() == event->formID) {
                    if (std::ranges::find(questStageCooldowns, stage) == questStageCooldowns.end()) {
                        questStageCooldowns.push_back(stage);
                        Utils::PlaySound(std::format("PVEQuestStageCompleted{}_{}", std::get<0>(questData), stage));
                        std::thread([stage] {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            std::erase(questStageCooldowns, stage);
                        }).detach();
                    }
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DynamicEventSink::ProcessEvent(const SKSE::CameraEvent *event, RE::BSTEventSource<SKSE::CameraEvent> *) {
        if (event && event->oldState && event->newState) {
            const auto newState = event->newState->id;
            const auto oldState = event->oldState->id;
            if (newState == RE::CameraState::kVATS) {
                if (RE::PlayerCharacter::GetSingleton()->IsInKillMove()) {
                    Utils::PlaySound("PVEFinisherStart");
                }
            } else if (oldState == RE::CameraState::kVATS) {
                Utils::PlaySound("PVEFinisherEnd");
            } else if (newState == RE::CameraState::kFurniture) {
                Utils::PlaySound("PVESit");
            } else if (oldState == RE::CameraState::kFurniture) {
                Utils::PlaySound("PVEGetUp");
            } else if (newState == RE::CameraState::kMount) {
                Utils::PlaySound("PVEMountHorse");
            } else if (oldState == RE::CameraState::kMount) {
                Utils::PlaySound("PVEDismountHorse");
            } else if (newState == RE::CameraState::kBleedout) {
                Utils::PlaySound("PVEDeath");
            } else if (newState == RE::CameraState::kDragon) {
                Utils::PlaySound("PVEMountDragon");
            } else if (oldState == RE::CameraState::kDragon) {
                Utils::PlaySound("PVEDismountDragon");
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DynamicEventSink::ProcessEvent(const RE::BGSActorCellEvent *event, RE::BSTEventSource<RE::BGSActorCellEvent> *) {
        if (event->actor && event->actor.get()->IsPlayerRef() && !event->flags.any(RE::BGSActorCellEvent::CellFlag::kLeave)) {
            auto actor = event->actor.get();
            std::thread([actor] {
                while (!actor->GetWorldspace()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                auto worldspace = actor->GetWorldspace();
                if (currentWorldspace.has_value() && currentWorldspace.value() == worldspace) {
                    return;
                }
                currentWorldspace.emplace(worldspace);
                const std::string s = actor->GetWorldspace()->GetName();
                if (s == "Whiterun" || s == "Solitude" || s == "Markarth" || s == "Windhelm" || s == "Riften") {
                    Utils::PlaySound("PVELocationEnterCity", std::format("PVELocationEnterCity{}", s));
                }
            }).detach();
        }
        return RE::BSEventNotifyControl::kContinue;
    }
}