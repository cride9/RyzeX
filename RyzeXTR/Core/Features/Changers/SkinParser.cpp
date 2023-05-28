#include "SkinParser.h"

void SkinChanger::Dump()
{
	const auto V_UCS2ToUTF8 = static_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(util::GetExportAddress("vstdlib.dll", "V_UCS2ToUTF8"));
	std::ifstream items = std::ifstream("csgo/scripts/items/items_game_cdn.txt");
	std::string gameItems = std::string(std::istreambuf_iterator <char> { items }, std::istreambuf_iterator <char> { });

	if (!items.is_open())
		return;

	items.close();

	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	uintptr_t sig_address = util::FindSignature("client.dll", "E8 ?? ?? ?? ?? FF 76 0C 8D 48 04 E8");

	// Skip the opcode, read rel32 address
	int item_system_offset = *reinterpret_cast<int32_t*>(sig_address + 1);

	// Add the offset to the end of the instruction
	auto item_system_fn = reinterpret_cast<IItemSystem * (*)()>(sig_address + 5 + item_system_offset);

	// Skip VTable, first member variable of ItemSystem is ItemSchema
	ItemSchema_t* m_pItemSchematic = reinterpret_cast<ItemSchema_t*>(uintptr_t(item_system_fn()) + sizeof(void*));

	std::vector<std::pair<short, EItemDefinitionIndex>> kitsWeapons;
	kitsWeapons.reserve(2000);

	for (int i = 0; i < m_pItemSchematic->getLootListCount(); ++i)
	{
		const auto& contents = m_pItemSchematic->getLootList(i)->getLootListContents();

		for (int j = 0; j < contents.size; ++j) {
			if (contents[j].paintKit != 0)
				kitsWeapons.emplace_back(contents[j].paintKit, contents[j].weaponId());
		}
	}

	for (int i = 0; i < m_pItemSchematic->getItemSetCount(); ++i)
	{
		const auto set = m_pItemSchematic->getItemSet(i);

		for (int j = 0; j < set->getItemCount(); ++j) {
			const auto paintKit = set->getItemPaintKit(j);
			if (paintKit != 0)
				kitsWeapons.emplace_back(paintKit, set->getItemDef(j));
		}
	}

	std::sort(kitsWeapons.begin(), kitsWeapons.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

	SkinKits.reserve(m_pItemSchematic->m_pPaintKits.lastAlloc);
	GloveKits.reserve(m_pItemSchematic->m_pPaintKits.lastAlloc);

	for (int i = 0; i <= m_pItemSchematic->m_pPaintKits.lastAlloc; i++)
	{
		const auto paintKit = m_pItemSchematic->m_pPaintKits.memory[i].value;

		if (paintKit->m_nID == 0 || paintKit->m_nID == 9001) // ignore workshop_default
			continue;

		if (paintKit->m_nID >= 10000)
		{
			const std::string_view gloveName{ paintKit->m_szName.data() };
			std::wstring name;

			if (gloveName._Starts_with("bloodhound"))
				name = i::Localize->Find("CSGO_Wearable_t_studdedgloves");
			else if (gloveName._Starts_with("motorcycle"))
				name = i::Localize->Find("CSGO_Wearable_v_motorcycle_glove");
			else if (gloveName._Starts_with("slick"))
				name = i::Localize->Find("CSGO_Wearable_v_slick_glove");
			else if (gloveName._Starts_with("sporty"))
				name = i::Localize->Find("CSGO_Wearable_v_sporty_glove");
			else if (gloveName._Starts_with("specialist"))
				name = i::Localize->Find("CSGO_Wearable_v_specialist_glove");
			else if (gloveName._Starts_with("operation10"))
				name = i::Localize->Find("CSGO_Wearable_t_studded_brokenfang_gloves");
			else if (gloveName._Starts_with("handwrap"))
				name = i::Localize->Find("CSGO_Wearable_v_leather_handwrap");
			else
				assert(false);

			name += L" | ";
			name += i::Localize->Find(paintKit->m_szDescriptionTag.data() + 1);

			char nameStr[256];
			V_UCS2ToUTF8(name.c_str(), nameStr, sizeof(nameStr));

			GloveKits.push_back({ paintKit->m_nID, nameStr, paintKit->m_szName.data(), paintKit->m_nRarity,0 });
		}
		else
		{
			std::unordered_set<EItemDefinitionIndex> weapons;

			for (auto it = std::lower_bound(kitsWeapons.begin(), kitsWeapons.end(), paintKit->m_nID, [](const auto& p, auto id) { return p.first < id; }); it != kitsWeapons.end() && it->first == paintKit->m_nID; ++it)
			{
				weapons.insert(it->second);
			}

			for (EItemDefinitionIndex weapon : weapons)
			{
				const auto itemDef = m_pItemSchematic->getItemDefinitionInterface(weapon);
				if (!itemDef)
					continue;

				std::wstring name = i::Localize->Find(itemDef->getItemBaseName());
				name += L" | ";
				name += i::Localize->Find(paintKit->m_szDescriptionTag.data() + 1);

				char nameStr[256];
				V_UCS2ToUTF8(name.c_str(), nameStr, sizeof(nameStr));

				SkinKits.push_back({ paintKit->m_nID, nameStr, paintKit->m_szName.data() ,std::clamp(itemDef->getRarity() + paintKit->m_nRarity - 1, 0, (paintKit->m_nRarity == 7) ? 7 : 6),(int)weapon });
			}

			if (weapons.empty() || weapons.size() > 1) // this paint kit fits more than one weapon
			{
				std::wstring name = i::Localize->Find(paintKit->m_szDescriptionTag.data() + 1);

				char nameStr[256];
				V_UCS2ToUTF8(name.c_str(), nameStr, sizeof(nameStr));

				SkinKits.push_back({ paintKit->m_nID, nameStr,paintKit->m_szName.data(), paintKit->m_nRarity, 0 });
			}
		}
	}

	std::sort(SkinKits.begin(), SkinKits.end());
	SkinKits.shrink_to_fit();
	std::sort(GloveKits.begin(), GloveKits.end());
	GloveKits.shrink_to_fit();

	//Handle stickers
	StickerKits.reserve(m_pItemSchematic->stickerKits.lastAlloc);
	for (int i = 0; i <= m_pItemSchematic->stickerKits.lastAlloc; i++)
	{
		const auto stickerKit = m_pItemSchematic->stickerKits.memory[i].value;
		if (std::string_view name{ stickerKit->name.data() }; name._Starts_with("spray") || name._Starts_with("patch"))
			continue;
		std::wstring name = i::Localize->Find(stickerKit->id != 242 ? stickerKit->itemName.data() + 1 : "StickerKit_dhw2014_teamdignitas_gold");

		char nameStr[256];
		V_UCS2ToUTF8(name.c_str(), nameStr, sizeof(nameStr));

		StickerKits.push_back({ stickerKit->id, std::move(nameStr),stickerKit->inventoryImage.data(), stickerKit->rarity,0 });
	}

	StickerKits.insert(StickerKits.begin(), { 0, "None" });
	std::sort(StickerKits.begin() + 1, StickerKits.end());
	StickerKits.shrink_to_fit();

	for (size_t m{ 0 }; m < m_pItemSchematic->m_pPaintKits.lastAlloc; m++)
	{
		PaintKit_t* m_pPaintKit = m_pItemSchematic->m_pPaintKits.memory[m].value;

		SkinColors_t inf;
		inf.m_colColor[0] = m_pPaintKit->m_rgbaColor[0];
		inf.m_colColor[1] = m_pPaintKit->m_rgbaColor[1];
		inf.m_colColor[2] = m_pPaintKit->m_rgbaColor[2];
		inf.m_colColor[3] = m_pPaintKit->m_rgbaColor[3];
		SkinColors.push_back(inf);
	}
}