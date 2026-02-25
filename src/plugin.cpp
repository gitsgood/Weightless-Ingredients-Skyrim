#include "SKSE/SKSE.h"
#include "RE/Skyrim.h"

void OnMessage(SKSE::MessagingInterface::Message* a_message)
{
    // Wait until all ESM/ESP/ESL mods have finished loading
    if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {

        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) return;

        // Fetch the array of every Soul Gem loaded by the game and mods
        const auto& soulGems = dataHandler->GetFormArray<RE::TESSoulGem>();
        size_t patchedCount = 0;

        // Iterate through and set the weight to 0
        for (auto* gem : soulGems) {
            if (gem) {
                // TESSoulGem inherits from TESWeightForm, giving us access to weight
                gem->weight = 0.0f;
                patchedCount++;
            }
        }

        SKSE::log::info("Successfully set the weight of {} soul gems to 0.", patchedCount);
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    // Initialize SKSE
    SKSE::Init(skse);

    // Register our event listener
    auto messaging = SKSE::GetMessagingInterface();
    if (messaging) {
        messaging->RegisterListener(OnMessage);
    }

    return true;
}