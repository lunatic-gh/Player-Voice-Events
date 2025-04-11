// ReSharper disable once CppUnusedIncludeDirective
#include "Exports.h"
#include "Logger.h"
#include "Main.h"
#include "PapyrusFunctions.h"

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    PVE::Logger::Init();
    SKSE::GetPapyrusInterface()->Register(PVE::PapyrusFunctions::Funcs);
    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        switch (message->type) {
            case SKSE::MessagingInterface::kDataLoaded: {
                PVE::Main::Init();
                break;
            }
            case SKSE::MessagingInterface::kNewGame:
            case SKSE::MessagingInterface::kPostLoadGame: {
                PVE::Main::Load();
                break;
            }
            default:
                break;
        }
    });
    return true;
}