#pragma once
#include <algorithm>
#include <array>
#include <functional>
#include <string>

namespace autobuy {
    struct Options {
        bool ct = false, keepWeapons = true;
        int pistol = 0, rifle = 0, sniper = 0, reserve = 0;
        int grenadeLimit = 4;
        std::array<bool, 3> equipment{};
        std::array<bool, 5> grenades{};
    };
    struct Inventory {
        int money = 0, armor = 0;
        bool helmet = false, defuser = false, primary = false, secondary = false;
        std::array<int, 70> items{};
    };

    inline std::string Build(const Options& options, const Inventory& inventory,
        const std::function<int(unsigned)>& price) {
        int budget = (std::max)(0, inventory.money - std::clamp(options.reserve, 0, 16000));
        std::string commands;
        const auto owns = [&](unsigned id) { return id < inventory.items.size() && inventory.items[id] > 0; };
        const auto add = [&](const char* aliases, unsigned id, unsigned alternate = 0, int fixedCost = -1) {
            if (owns(id) || (alternate && owns(alternate)))
                return false;
            const int cost = fixedCost >= 0 ? fixedCost : (std::max)(price(id), alternate ? price(alternate) : 0);
            // An unknown price cannot safely preserve the requested reserve.
            if (cost <= 0 || cost > budget)
                return false;
            commands += aliases;
            budget -= cost;
            return true;
        };
        if (!options.keepWeapons || !inventory.primary) {
            switch (options.sniper) {
            case 1: add("buy ssg08;", 40); break;
            case 2: add("buy awp;", 9); break;
            case 3: options.ct ? add("buy scar20;", 38) : add("buy g3sg1;", 11); break;
            default:
                switch (options.rifle) {
                case 1: options.ct ? add("buy famas;", 10) : add("buy galilar;", 13); break;
                case 2: options.ct ? add("buy m4a1;buy m4a1_silencer;", 16, 60) : add("buy ak47;", 7); break;
                case 3: options.ct ? add("buy aug;", 8) : add("buy sg556;", 39); break;
                }
                break;
            }
        }
        if (!options.keepWeapons || !inventory.secondary) {
            switch (options.pistol) {
            case 1: options.ct ? add("buy usp_silencer;buy hkp2000;", 61, 32) : add("buy glock;", 4); break;
            case 2: add("buy elite;", 2); break;
            case 3: add("buy p250;", 36); break;
            case 4: options.ct ? add("buy fiveseven;buy cz75a;", 3, 63) : add("buy tec9;buy cz75a;", 30, 63); break;
            case 5: add("buy deagle;buy revolver;", 1, 64); break;
            }
        }
        if (options.equipment[0]) {
            if (!inventory.helmet)
                add("buy vesthelm;", 0, 0, inventory.armor >= 100 ? 350 : 1000);
            else if (inventory.armor < 100)
                add("buy vest;", 0, 0, 650);
        }
        if (options.equipment[1]) add("buy taser;", 31);
        if (options.equipment[2] && options.ct && !inventory.defuser) add("buy defuser;", 0, 0, 400);
        int grenadeCount = 0;
        for (unsigned id = 43; id <= 48; ++id)
            grenadeCount += std::clamp(inventory.items[id], 0, 16);
        const auto grenade = [&](bool selected, const char* aliases, unsigned id) {
            if (selected && grenadeCount < std::clamp(options.grenadeLimit, 0, 16) && add(aliases, id))
                ++grenadeCount;
        };
        grenade(options.grenades[4], "buy smokegrenade;", 45);
        grenade(options.grenades[3], "buy hegrenade;", 44);
        grenade(options.grenades[0], options.ct ? "buy incgrenade;" : "buy molotov;", options.ct ? 48 : 46);
        grenade(options.grenades[2], "buy flashbang;", 43);
        grenade(options.grenades[1], "buy decoy;", 47);
        return commands;
    }
}
