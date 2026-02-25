#include "SKSE/SKSE.h"
#include "RE/Skyrim.h"

// The listener that waits for the game to finish loading mods
void OnMessage(SKSE::MessagingInterface::Message* a_message)
{
    // kDataLoaded fires exactly once, right after all ESM/ESP/ESL files have finished loading.
    if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {

        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) return;

        // Get the array of all Ingredient Base Forms in the game
        const auto& ingredients = dataHandler->GetFormArray<RE::IngredientItem>();
        size_t patchedCount = 0;

        // Iterate through all of them and overwrite the weight to 0.0
        for (auto* ingredient : ingredients) {
            if (ingredient) {
                // IngredientItem inherits from TESWeightForm
                ingredient->weight = 0.0f;
                patchedCount++;
            }
        }

        SKSE::log::info("Successfully set the weight of {} ingredients to 0.", patchedCount);
    }
}

// Standard SKSE NG entry point
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