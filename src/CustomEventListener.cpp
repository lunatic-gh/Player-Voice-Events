#include "CustomEventListener.h"

#include "ConditionManager.h"
#include "Main.h"
#include "Utils.h"

namespace PVE {
    void CustomEventListener::Register() {
        if (!markerMap.empty()) {
            loopManager.StartNew("Markers", []()  {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                auto player = RE::PlayerCharacter::GetSingleton();
                auto playerWorldspace = player->GetWorldspace();
                if (!playerWorldspace) {
                    if (currentLocMarker.has_value()) {
                        currentLocMarker.reset();
                    }
                    return;
                }
                const std::string worldName = playerWorldspace->GetName();
                RE::TESObjectREFR* newActiveMarker = nullptr;
                double activeMarkerRadius = 0.0;
                double closestDistance = std::numeric_limits<double>::max();
                for (const auto& [markerWorldspace, markers] : markerMap) {
                    if (markerWorldspace != worldName) continue;
                    for (const auto& [markerRef, markerRadius] : markers) {
                        double distance = player->GetPosition().GetDistance(markerRef->GetPosition());
                        if (distance <= markerRadius && distance < closestDistance) {
                            newActiveMarker = markerRef;
                            activeMarkerRadius = markerRadius;
                            closestDistance = distance;
                        }
                    }
                }
                if (!currentLocMarker.has_value() && newActiveMarker) {
                    currentLocMarker.emplace(newActiveMarker);
                    while (!Utils::CanPlaySound()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
                    auto markerLocName = newActiveMarker->extraList.GetByType<RE::ExtraMapMarker>()->mapData->locationName.GetFullName();
                    ConditionManager::RegisterDynamicCondition("PVEEnterMarker", "MarkerName", [markerLocName] {
                        return markerLocName;
                    });
                    ConditionManager::RegisterDynamicCondition("PVEEnterMarker", "MarkerFormID", [newActiveMarker] {
                        return static_cast<int>(newActiveMarker->GetFormID());
                    });
                    Utils::PlaySound(std::format("PVEEnterMarker", markerLocName));
                    return;
                }
                if (currentLocMarker.has_value() && !newActiveMarker) {
                    currentLocMarker.reset();
                    return;
                }
                if (currentLocMarker.has_value() && newActiveMarker && currentLocMarker.value() != newActiveMarker) {
                    currentLocMarker.emplace(newActiveMarker);
                    {
                        while (!Utils::CanPlaySound()) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        }
                        auto markerLocName = newActiveMarker->extraList.GetByType<RE::ExtraMapMarker>()->mapData->locationName.GetFullName();
                        ConditionManager::RegisterDynamicCondition("PVEEnterMarker", "MarkerName", [markerLocName] {
                            return markerLocName;
                        });
                        ConditionManager::RegisterDynamicCondition("PVEEnterMarker", "MarkerFormID", [newActiveMarker] {
                            return static_cast<int>(newActiveMarker->GetFormID());
                        });
                        Utils::PlaySound(std::format("PVEEnterMarker", markerLocName));
                    }
                    return;
                }
                if (currentLocMarker.has_value() && newActiveMarker && newActiveMarker == currentLocMarker.value()) {
                    double currentDistance = player->GetPosition().GetDistance(currentLocMarker.value()->GetPosition());
                    if (currentDistance >= activeMarkerRadius * 1.2) {
                        currentLocMarker.reset();
                    }
                }
            });
        }
    }
}
