#include "EventListener.h"

#include "ConditionManager.h"
#include "Utils.h"

namespace PVE {
    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESPlayerBowShotEvent *event, RE::BSTEventSource<RE::TESPlayerBowShotEvent> *) {
        float f = event->shotPower;
        ConditionManager::RegisterDynamicCondition("PVEAttackBow", "IsFullDrawn", [f] {
            return f == 1.0f;
        });
        Utils::PlaySound("PVEAttackBow");
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
                ConditionManager::RegisterDynamicCondition("PVEReceivedHit", "IsAttackBlocked", [event] {
                    return event->flags.any(RE::TESHitEvent::Flag::kHitBlocked);
                });
                ConditionManager::RegisterDynamicCondition("PVEReceivedHit", "IsBashAttack", [event] {
                    return event->flags.any(RE::TESHitEvent::Flag::kBashAttack);
                });
                ConditionManager::RegisterDynamicCondition("PVEReceivedHit", "IsPowerAttack", [event] {
                    return event->flags.any(RE::TESHitEvent::Flag::kPowerAttack);
                });
                ConditionManager::RegisterDynamicCondition("PVEReceivedHit", "IsSneakAttack", [event] {
                    return event->flags.any(RE::TESHitEvent::Flag::kSneakAttack);
                });
                ConditionManager::RegisterDynamicCondition("PVEReceivedHit", "IsHealingAttack", [cause, source] {
                    return cause && cause->As<RE::Actor>() && cause->As<RE::Actor>()->IsPlayerTeammate() && source && source->Is(RE::FormType::Spell) &&
                           Utils::FormHasKeywordString(source, "MagicRestoreHealth");
                });
                if (!cause || !cause->As<RE::Actor>() || !cause->As<RE::Actor>()->IsInKillMove()) {
                    Utils::PlaySound("PVEReceivedHit");
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
                    bool isPowerAttack = false;
                    if (const auto currentProcess = actor->GetActorRuntimeData().currentProcess) {
                        if (currentProcess->high && currentProcess->high->attackData) {
                            const auto flags = currentProcess->high->attackData->data.flags;
                            isPowerAttack = flags.any(RE::AttackData::AttackFlag::kPowerAttack);
                        }
                    }
                    ConditionManager::RegisterDynamicCondition("PVEAttackMelee", "IsPowerAttack", [isPowerAttack]() {
                        return isPowerAttack;
                    });
                    Utils::PlaySound("PVEAttackMelee");
                } else if (type == SKSE::ActionEvent::Type::kBeginDraw) {
                    static bool isOnCooldown = false;
                    if (source && source->GetFormType() != RE::FormType::Spell && !isOnCooldown) {
                        isOnCooldown = true;
                        ConditionManager::RegisterDynamicCondition("PVEUnsheatheWeapon", "WeaponFormID", [source] {
                            if (const auto weap = source->As<RE::TESObjectWEAP>()) return static_cast<int>(weap->GetFormID());
                            return -1;
                        });
                        ConditionManager::RegisterDynamicCondition("PVEUnsheatheWeapon", "WeaponType", [source] {
                            if (const auto weap = source->As<RE::TESObjectWEAP>()) return static_cast<int>(weap->GetWeaponType());
                            return -1;
                        });
                        ConditionManager::RegisterDynamicCondition("PVEUnsheatheWeapon", "WeaponKeywords", [source] {
                            return Utils::GetFormKeywordsFormatted(source);
                        });
                        Utils::PlaySound("PVEUnsheatheWeapon");
                        std::thread([]() {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            isOnCooldown = false;
                        }).detach();
                    }
                } else if (type == SKSE::ActionEvent::Type::kEndDraw) {
                    static bool isOnCooldown = false;
                    if (source && source->GetFormType() == RE::FormType::Spell && !isOnCooldown) {
                        isOnCooldown = true;
                        ConditionManager::RegisterDynamicCondition("PVEUnsheatheSpell", "SpellFormID", [source] {
                            if (const auto spell = source->As<RE::SpellItem>()) return static_cast<int>(spell->GetFormID());
                            return -1;
                        });
                        ConditionManager::RegisterDynamicCondition("PVEUnsheatheSpell", "SpellKeywords", [source] {
                            std::string s = "";
                            if (const auto spell = source->As<RE::SpellItem>()) {
                                for (auto mgef : spell->effects) {
                                    for (const auto keyword : mgef->baseEffect->GetKeywords()) {
                                        if (keyword && !s.contains(keyword->GetFormEditorID())) {
                                            s += keyword->GetFormEditorID();
                                            s += "|";
                                        }
                                    }
                                }
                            }
                            return s.empty() ? s : s.substr(0, s.size() - 1);
                        });
                        Utils::PlaySound("PVEUnsheatheSpell");
                        std::thread([]() {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            isOnCooldown = false;
                        }).detach();
                    }
                } else if (type == SKSE::ActionEvent::Type::kBeginSheathe) {
                    if (source && source->GetFormType() != RE::FormType::Spell) {
                        static bool isOnCooldown = false;
                        if (!isOnCooldown) {
                            isOnCooldown = true;
                            ConditionManager::RegisterDynamicCondition("PVESheatheWeapon", "WeaponFormID", [source] {
                                if (const auto weap = source->As<RE::TESObjectWEAP>()) return static_cast<int>(weap->GetFormID());
                                return -1;
                            });
                            ConditionManager::RegisterDynamicCondition("PVESheatheWeapon", "WeaponType", [source] {
                                if (const auto weap = source->As<RE::TESObjectWEAP>()) return static_cast<int>(weap->GetWeaponType());
                                return -1;
                            });
                            ConditionManager::RegisterDynamicCondition("PVESheatheWeapon", "WeaponKeywords", [source] {
                                return Utils::GetFormKeywordsFormatted(source);
                            });
                            Utils::PlaySound("PVESheatheWeapon");
                            std::thread([]() {
                                std::this_thread::sleep_for(std::chrono::seconds(1));
                                isOnCooldown = false;
                            }).detach();
                        }
                    }
                } else if (type == SKSE::ActionEvent::Type::kEndSheathe) {
                    if (source && source->GetFormType() == RE::FormType::Spell) {
                        static bool isOnCooldown = false;
                        if (!isOnCooldown) {
                            isOnCooldown = true;
                            ConditionManager::RegisterDynamicCondition("PVESheatheSpell", "SpellFormID", [source] {
                                if (const auto spell = source->As<RE::SpellItem>()) return static_cast<int>(spell->GetFormID());
                                return -1;
                            });
                            ConditionManager::RegisterDynamicCondition("PVESheatheSpell", "SpellKeywords", [source] {
                                std::string s = "";
                                if (const auto spell = source->As<RE::SpellItem>()) {
                                    for (auto mgef : spell->effects) {
                                        for (const auto keyword : mgef->baseEffect->GetKeywords()) {
                                            if (keyword && !s.contains(keyword->GetFormEditorID())) {
                                                s += keyword->GetFormEditorID();
                                                s += "|";
                                            }
                                        }
                                    }
                                }
                                return s.empty() ? s : s.substr(0, s.size() - 1);
                            });
                            Utils::PlaySound("PVESheatheSpell");
                            std::thread([]() {
                                std::this_thread::sleep_for(std::chrono::seconds(1));
                                isOnCooldown = false;
                            }).detach();
                        }
                    }
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESSleepStartEvent *, RE::BSTEventSource<RE::TESSleepStartEvent> *) {
        ConditionManager::RegisterDynamicCondition("PVESleep", "SleepState", [] {
            return 0;
        });
        Utils::PlaySound("PVESleep");
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESSleepStopEvent *, RE::BSTEventSource<RE::TESSleepStopEvent> *) {
        ConditionManager::RegisterDynamicCondition("PVESleep", "SleepState", [] {
            return 1;
        });
        Utils::PlaySound("PVESleep");
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESContainerChangedEvent *event, RE::BSTEventSource<RE::TESContainerChangedEvent> *) {
        const auto oldContainer = event->oldContainer;
        if (const auto newContainer = event->newContainer; newContainer == RE::PlayerCharacter::GetSingleton()->GetFormID()) {
            const auto form = RE::TESForm::LookupByID(event->baseObj);
            ConditionManager::RegisterDynamicCondition("PVEPickupItem", "ItemFormID", [form] {
                if (form) {
                    return static_cast<int>(form->GetFormID());
                }
                return -1;
            });
            ConditionManager::RegisterDynamicCondition("PVEPickupItem", "ItemFormType", [form] {
                if (form) {
                    return static_cast<int>(form->GetFormType());
                }
                return -1;
            });
            ConditionManager::RegisterDynamicCondition("PVEPickupItem", "ItemKeywords", [form] {
                if (form) {
                    return Utils::GetFormKeywordsFormatted(form);
                }
                return std::string("");
            });
            ConditionManager::RegisterDynamicCondition("PVEPickupItem", "SourceContainerFormID", [oldContainer] {
                return static_cast<int>(oldContainer);
            });
            Utils::PlaySound("PVEPickupItem");
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESQuestStageEvent *event, RE::BSTEventSource<RE::TESQuestStageEvent> *) {
        if (!event || event->formID == 0) {
            return RE::BSEventNotifyControl::kContinue;
        };
        int questID = static_cast<int>(event->formID);
        int questStage = event->stage;
        if (!std::ranges::contains(questStageCooldownMap[questID], questStage)) {
            questStageCooldownMap[questID].push_back(questStage);
            ConditionManager::RegisterDynamicCondition("PVEQuestStageCompleted", "QuestID", [questID] {
                return questID;
            });
            ConditionManager::RegisterDynamicCondition("PVEQuestStageCompleted", "QuestStage", [questStage] {
                return questStage;
            });
            Utils::PlaySound("PVEQuestStageCompleted");
            std::thread([questID, questStage] {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::erase(questStageCooldownMap[questID], questStage);
            }).detach();
        }
        return RE::BSEventNotifyControl::kContinue;
    }
    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESFurnitureEvent *event, RE::BSTEventSource<RE::TESFurnitureEvent> *) {
        if (event) {
            if (event->actor->IsPlayerRef()) {
                if (event->type == RE::TESFurnitureEvent::FurnitureEventType::kEnter) {
                    Utils::PlaySound("PVESit");
                } else if (event->type == RE::TESFurnitureEvent::FurnitureEventType::kExit) {
                    Utils::PlaySound("PVEGetUp");
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::TESSpellCastEvent *event, RE::BSTEventSource<RE::TESSpellCastEvent> *) {
        static std::vector<RE::SpellItem *> casting = {};
        static std::mutex castingMutex;
        if (const auto obj = event->object.get()) {
            if (auto spell = RE::TESForm::LookupByID<RE::SpellItem>(event->spell)) {
                if (const auto actor = obj->As<RE::Actor>()) {
                    std::lock_guard<std::mutex> lock(castingMutex);
                    // If the spell is not marked as casting
                    if (actor->IsPlayerRef() && std::ranges::find(casting, spell) == std::end(casting)) {
                        // Mark this spell as casting
                        casting.push_back(spell);
                        // Start new async thread
                        std::thread([actor, spell]() {
                            // Wait until the spell isn't cast anymore
                            while (actor->IsCasting(spell)) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(250));
                            }
                            auto start = std::chrono::steady_clock::now();
                            // Wait for one second. If during this wait the spell is cast again, reset the timer to avoid spamming events by spam-casting.
                            while (std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count() < 1.0f) {
                                if (actor->IsCasting(spell)) {
                                    start = std::chrono::steady_clock::now();
                                }
                            }
                            // Mark spell as not casting
                            std::lock_guard lock(castingMutex);
                            std::erase(casting, spell);
                        }).detach();
                        // Register Dynamic Conditions
                        ConditionManager::RegisterDynamicCondition("PVESpellCast", "SpellFormID", [spell]() {
                            return static_cast<int>(spell->GetFormID());
                        });
                        ConditionManager::RegisterDynamicCondition("PVESpellCast", "SpellKeywords", [spell]() {
                            std::string s = "";
                            for (auto mgef : spell->effects) {
                                for (const auto keyword : mgef->baseEffect->GetKeywords()) {
                                    if (keyword && !s.contains(keyword->GetFormEditorID())) {
                                        s += keyword->GetFormEditorID();
                                        s += "|";
                                    }
                                }
                            }
                            return s.empty() ? s : s.substr(0, s.size() - 1);
                        });
                        // Play Sound
                        Utils::PlaySound("PVESpellCast");
                    }
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DefaultEventSink::ProcessEvent(const RE::LocationDiscovery::Event *event, RE::BSTEventSource<RE::LocationDiscovery::Event> *) {
        if (event && event->mapMarkerData) {
            std::string locName = "";
            std::string worldspaceName = "";
            if (event->mapMarkerData) locName = event->mapMarkerData->locationName.GetFullName();
            if (event->worldspaceID) worldspaceName = event->worldspaceID;
            ConditionManager::RegisterDynamicCondition("PVELocationDiscover", "LocationName", [locName] {
                return locName;
            });
            ConditionManager::RegisterDynamicCondition("PVELocationDiscover", "WorldspaceName", [worldspaceName] {
                return worldspaceName;
            });
            Utils::PlaySound("PVELocationDiscover");
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DynamicEventSink::ProcessEvent(const SKSE::CameraEvent *event, RE::BSTEventSource<SKSE::CameraEvent> *) {
        if (event && event->oldState && event->newState) {
            if (auto player = RE::PlayerCharacter::GetSingleton()) {
                const auto newState = event->newState->id;
                const auto oldState = event->oldState->id;
                if (newState == RE::CameraState::kVATS) {
                    if (RE::PlayerCharacter::GetSingleton()->IsInKillMove()) {
                        ConditionManager::RegisterDynamicCondition("PVEFinisher", "FinisherState", [] {
                            return 0;
                        });
                        Utils::PlaySound("PVEFinisher");
                    }
                } else if (oldState == RE::CameraState::kVATS) {
                    ConditionManager::RegisterDynamicCondition("PVEFinisher", "FinisherState", [] {
                        return 1;
                    });
                    Utils::PlaySound("PVEFinisher");
                } else if (newState == RE::CameraState::kMount) {
                    ConditionManager::RegisterDynamicCondition("PVEMount", "MountType", [] {
                        return 0;
                    });
                    Utils::PlaySound("PVEMount");
                } else if (oldState == RE::CameraState::kMount) {
                    ConditionManager::RegisterDynamicCondition("PVEDismount", "MountType", [] {
                        return 0;
                    });
                    Utils::PlaySound("PVEDismount");
                } else if (newState == RE::CameraState::kBleedout) {
                    Utils::PlaySound("PVEDeath");
                } else if (newState == RE::CameraState::kDragon) {
                    ConditionManager::RegisterDynamicCondition("PVEMount", "MountType", [] {
                        return 1;
                    });
                    Utils::PlaySound("PVEMount");
                } else if (oldState == RE::CameraState::kDragon) {
                    ConditionManager::RegisterDynamicCondition("PVEDismount", "MountType", [] {
                        return 1;
                    });
                    Utils::PlaySound("PVEDismount");
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl DynamicEventSink::ProcessEvent(const RE::BGSActorCellEvent *event, RE::BSTEventSource<RE::BGSActorCellEvent> *) {
        if (event && event->actor && event->actor.get()->IsPlayerRef()) {
            if (RE::TESForm *form = RE::TESForm::LookupByID(event->cellID)) {
                if (RE::TESObjectCELL *cell = form->As<RE::TESObjectCELL>()) {
                    if (!event->flags.any(RE::BGSActorCellEvent::CellFlag::kLeave)) {
                        if (cell != currentCell) {
                            RE::TESObjectCELL *oldCell = currentCell.has_value() ? currentCell.value() : nullptr;
                            RE::TESObjectCELL *newCell = cell;
                            currentCell = cell;
                            std::thread([oldCell, newCell] {
                                while (!Utils::CanPlaySound()) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                }
                                ConditionManager::RegisterDynamicCondition("PVEChangeCell", "OldCellName", [oldCell] {
                                    return oldCell ? oldCell->GetFormEditorID() : "";
                                });
                                ConditionManager::RegisterDynamicCondition("PVEChangeCell", "OldCellFormID", [oldCell] {
                                    return oldCell ? static_cast<int>(oldCell->GetFormID()) : -1;
                                });
                                ConditionManager::RegisterDynamicCondition("PVEChangeCell", "NewCellName", [newCell] {
                                    return newCell ? newCell->GetFormEditorID() : "";
                                });
                                ConditionManager::RegisterDynamicCondition("PVEChangeCell", "NewCellFormID", [newCell] {
                                    return newCell ? static_cast<int>(newCell->GetFormID()) : -1;
                                });
                                Utils::PlaySound("PVEChangeCell");
                            }).detach();
                        }
                    }
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

}