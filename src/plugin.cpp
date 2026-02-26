#include "SKSE/SKSE.h"
#include "RE/Skyrim.h"
#include <REL/Relocation.h>

namespace Hooks
{
    // We are now hooking Actor::GetWeight (ID 36463)
    // This is the function the UI calls to draw the bar.
    using GetActorWeight_t = float (*)(RE::Actor* a_actor);
    static REL::Relocation<GetActorWeight_t> _OriginalGetActorWeight;

    float Hooked_GetActorWeight(RE::Actor* a_actor)
    {
        // 1. SAFETY: If it's not the player, use vanilla logic.
        // We don't want to break NPC encumbrance or carry weight logic.
        if (!a_actor || !a_actor->IsPlayerRef()) {
            return _OriginalGetActorWeight(a_actor);
        }

        // 2. GET INVENTORY: The Actor has a helper to get InventoryChanges
        auto* changes = a_actor->GetInventoryChanges();
        if (!changes || !changes->entryList) {
            return 0.0f; // Empty inventory weighs nothing
        }

        float calculatedWeight = 0.0f;

        // 3. ITERATE: Manually sum the weight based on your rules
        for (auto* entry : *changes->entryList) {
            if (!entry || !entry->object) continue;

            float itemWeight = entry->object->GetWeight();
            if (itemWeight <= 0.0f) continue;

            // --- SOULSLIKE LOGIC ---

            bool isGear = entry->object->IsWeapon() || entry->object->IsArmor();

            if (!isGear) {
                // Potions/Misc: Add Full Stack Weight
                calculatedWeight += (itemWeight * entry->countDelta);
            }
            else {
                // Gear: Only add weight if EQUIPPED
                int equippedCount = 0;

                if (entry->extraLists) {
                    for (auto* extraList : *entry->extraLists) {
                        if (extraList) {
                            if (extraList->HasType(RE::ExtraDataType::kWorn) ||
                                extraList->HasType(RE::ExtraDataType::kWornLeft)) {

                                auto* countExtra = extraList->GetByType<RE::ExtraCount>();
                                equippedCount += (countExtra ? countExtra->count : 1);
                            }
                        }
                    }
                }

                if (equippedCount > 0) {
                    calculatedWeight += (itemWeight * equippedCount);
                }
                // Unequipped Gear is IGNORED here.
            }
        }

        return calculatedWeight;
    }

    void Install()
    {
        // ID 36463 is "Actor::GetWeight".
        // This function is the authority on how heavy an actor is.
        REL::Relocation<std::uintptr_t> target{ REL::ID(36463) };

        SKSE::log::info("Hooking Actor::GetWeight (ID 36463) at address: {:x}", target.address());

        auto& trampoline = SKSE::GetTrampoline();
        _OriginalGetActorWeight = trampoline.write_branch<5>(target.address(), Hooked_GetActorWeight);
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    SKSE::AllocTrampoline(64);
    Hooks::Install();
    return true;
}