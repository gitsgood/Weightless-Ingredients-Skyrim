#include "SKSE/SKSE.h"
#include "RE/Skyrim.h"

void OnMessage(SKSE::MessagingInterface::Message* a_message)
{
    // kDataLoaded fires after Survival Mode has applied its 0.1 weights
    if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {

        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) return;

        const auto& ammoArray = dataHandler->GetFormArray<RE::TESAmmo>();
        size_t patchedCount = 0;

        for (auto* ammo : ammoArray) {
            if (ammo) {
                // REVERSE ENGINEERING HACK:
                // We know TESWeightForm is located exactly 176 bytes (0xB0) into the TESAmmo object.
                // We forcefully cast that exact memory address into a WeightForm.
                auto* weightForm = reinterpret_cast<RE::TESWeightForm*>(reinterpret_cast<uintptr_t>(ammo) + 0xB0);

                // Now we have direct write access to the weight!
                weightForm->weight = 0.0f;
                patchedCount++;
            }
        }

        SKSE::log::info("Successfully forcefully hacked the weight of {} arrows and bolts to 0.", patchedCount);
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    auto messaging = SKSE::GetMessagingInterface();
    if (messaging) {
        messaging->RegisterListener(OnMessage);
    }
    return true;
}