#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {
    // Structure to hold our multipliers
    struct SkillSettings {
        std::map<RE::ActorValue, float> multipliers;

        static SkillSettings* GetSingleton() {
            static SkillSettings singleton;
            return &singleton;
        }

        void Load() {
            CSimpleIniA ini;
            ini.SetUnicode();

            // Path: Data/SKSE/Plugins/SimplyXP.ini
            const auto path = L"Data/SKSE/Plugins/SimplyXP.ini";
            SI_Error rc = ini.LoadFile(path);

            if (rc < 0) {
                SPDLOG_ERROR("Failed to load SimplyXP.ini. Using defaults (1.0)");
            }

            // List of all skills to check in the INI
            std::vector<std::pair<RE::ActorValue, std::string>> skills = {
                { RE::ActorValue::kOneHanded, "OneHanded" },
                { RE::ActorValue::kTwoHanded, "TwoHanded" },
                { RE::ActorValue::kArchery, "Archery" },
                { RE::ActorValue::kBlock, "Block" },
                { RE::ActorValue::kSmithing, "Smithing" },
                { RE::ActorValue::kHeavyArmor, "HeavyArmor" },
                { RE::ActorValue::kLightArmor, "LightArmor" },
                { RE::ActorValue::kPickpocket, "Pickpocket" },
                { RE::ActorValue::kLockpicking, "Lockpicking" },
                { RE::ActorValue::kSneak, "Sneak" },
                { RE::ActorValue::kAlchemy, "Alchemy" },
                { RE::ActorValue::kSpeech, "Speech" },
                { RE::ActorValue::kAlteration, "Alteration" },
                { RE::ActorValue::kConjuration, "Conjuration" },
                { RE::ActorValue::kDestruction, "Destruction" },
                { RE::ActorValue::kIllusion, "Illusion" },
                { RE::ActorValue::kRestoration, "Restoration" },
                { RE::ActorValue::kEnchanting, "Enchanting" }
            };

            for (const auto& [av, name] : skills) {
                // If the key doesn't exist, SimpleIni returns the default value (1.0)
                float val = (float)ini.GetDoubleValue("Multipliers", name.c_str(), 1.0);
                multipliers[av] = val;
                SPDLOG_INFO("Multiplier for {}: {}", name, val);
            }
        }

        float GetMultiplier(RE::ActorValue a_skill) {
            auto it = multipliers.find(a_skill);
            return (it != multipliers.end()) ? it->second : 1.0f;
        }
    };

    // Hooking logic
    struct Hook_AddSkillExperience {
        // Function: PlayerCharacter::AddSkillExperience
        // SE ID: 39413, AE ID: 40488
        static void thunk(RE::PlayerCharacter* a_this, RE::ActorValue a_skill, float a_experience) {
            float multiplier = SkillSettings::GetSingleton()->GetMultiplier(a_skill);
            float modifiedXP = a_experience * multiplier;

            // Optional: Only log if XP is actually being granted
            if (modifiedXP > 0.0f) {
                SPDLOG_DEBUG("Original XP: {} | Multiplier: {} | Final XP: {}", a_experience, multiplier, modifiedXP);
            }

            func(a_this, a_skill, modifiedXP);
        }

        static inline REL::Relocation<decltype(thunk)> func;

        static void Install() {
            REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(39413, 40488) };
            auto& trampoline = SKSE::GetTrampoline();
            func = trampoline.write_branch<5>(target.address(), thunk);
            SPDLOG_INFO("Hooked AddSkillExperience.");
        }
    };
}

// Standard Logging setup
void InitializeLogging() {
    auto path = log_directory();
    if (!path) return;
    *path /= "SimplyXP.log";

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

    spdlog::set_default_logger(std::move(log));
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);
}

// SKSE Plugin Load Entry Point
SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();

    auto* plugin = PluginDeclaration::GetSingleton();
    SPDLOG_INFO("{} {} is loading...", plugin->GetName(), plugin->GetVersion());

    Init(skse);

    // 1. Load the INI settings
    SkillSettings::GetSingleton()->Load();

    // 2. Allocate trampoline for the branch hook
    AllocTrampoline(64);

    // 3. Install the hook
    Hook_AddSkillExperience::Install();

    SPDLOG_INFO("Plugin loaded successfully.");
    return true;
}