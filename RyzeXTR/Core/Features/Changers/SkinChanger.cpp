#include "SkinChanger.h"
#include <codecvt>
#include <unordered_set>
#include "../../Hooks/hooks.h"
#include "../../SDK/NetVar/netvar.h"

class CCStrike15ItemSchema;
class CCStrike15ItemSystem;

template <typename Key, typename Value>
struct Node_t
{
	int previous_id;		//0x0000
	int next_id;			//0x0004
	void* _unknown_ptr;		//0x0008
	int _unknown;			//0x000C
	Key key;				//0x0010
	Value value;			//0x0014
};

template <typename Key, typename Value>
struct Head_t
{
	Node_t<Key, Value>* memory;		//0x0000
	int allocation_count;			//0x0004
	int grow_size;					//0x0008
	int start_element;				//0x000C
	int next_available;				//0x0010
	int _unknown;					//0x0014
	int last_element;				//0x0018
}; //Size=0x001C

// could use CUtlString but this is just easier and CUtlString isn't needed anywhere else
struct String_t
{
	char* buffer;	//0x0000
	int capacity;	//0x0004
	int grow_size;	//0x0008
	int length;		//0x000C
}; //Size=0x0010

#if 0
void CSkinChanger::ApplyStickers(CBaseCombatWeapon* pWeapon)
{
	const auto econ_item_interface_wrapper = reinterpret_cast<void*>(pWeapon->GetEconItemView());

	if (!detour::GetStickerAttributeBySlotIndexInt.IsHooked())
		h::HookTable(detour::GetStickerAttributeBySlotIndexInt, econ_item_interface_wrapper, 5, &h::hkGetStickerAttributeBySlotIndexInt);
		//detour::GetStickerAttributeBySlotIndexInt.Create(MEM::GetVFunc(reinterpret_cast<void*>(econ_item_interface_wrapper), 5), &H::hkGetStickerAttributeBySlotIndexInt);
}
#endif

CreateClientClassFn GetWearableCreateFn()
{
	auto client_class = i::ClientDll->GetAllClasses();
	for (client_class = i::ClientDll->GetAllClasses();
		client_class; client_class = client_class->pNext)
	{
		if (client_class->nClassID == EClassIndex::CEconWearable)
			return client_class->pCreateFn;
	}
}

CBaseCombatWeapon* MakeGlove(int entry, int serial)
{
	static CreateClientClassFn WearableCreateFn = GetWearableCreateFn();
	WearableCreateFn(entry, serial);

	const auto glove = static_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntity(entry));
	assert(glove);
	{
		static auto set_abs_origin_addr = MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8"));
		const auto set_abs_origin_fn = reinterpret_cast<void(__thiscall*)(void*, const std::array<float, 3>&)>(set_abs_origin_addr);
		static constexpr std::array<float, 3> new_pos = { 10000.f, 10000.f, 10000.f };
		set_abs_origin_fn(glove, new_pos);
	}
	return glove;
}

bool ApplyGlove(CBaseCombatWeapon* glove, short item_definition_index, int paint_kit, int model_index, int entity_quality, float fallback_wear) {
	auto oldIndex = glove->GetItemDefinitionIndex();

	if (oldIndex != item_definition_index)
		glove->GetItemDefinitionIndex() = item_definition_index;

	glove->GetFallbackPaintKit() = paint_kit;
	glove->GetModelIndex() = model_index;
	glove->GetEntityQuality() = entity_quality;
	glove->GetFallbackWear() = fallback_wear;

	return true;
}

void CSkinChanger::AgentChanger(EStage stage)
{
	if (!cfg::skin::bEnableSkinChagner)
		return;

	if ( !cfg::skin::iSkinId[ 36 ] )
		return;

	static int originalIdx = 0;

	if (!g::pLocal)
	{
		originalIdx = 0;
		return;
	}

	if (const auto model = cfg::skin::szAgentModel )
	{
		if (stage == FRAME_RENDER_START)
			originalIdx = i::ModelInfo->GetModelIndex(g::pLocal->GetModel()->szName);

		const auto idx = stage == FRAME_RENDER_END && originalIdx ? originalIdx : i::ModelInfo->GetModelIndex(model);
		g::pLocal->SetModelIndex(idx);
	}
}

void HandleGloves()
{
	if (!cfg::skin::iGloveModel)
		return;

	if (g::pLocal == nullptr)
		return;

	CBaseHandle* wearables = g::pLocal->GetWearablesHandle();
	if (!wearables)
		return;

	static uintptr_t glove_handle = uintptr_t(0);

	CBaseCombatWeapon* glove = reinterpret_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(wearables[0]));

	if (!glove)
	{
		auto our_glove = reinterpret_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(glove_handle));
		if (our_glove)
		{
			wearables[0] = glove_handle;
			glove = our_glove;
		}
	}

	if (!g::pLocal->IsAlive())
	{
		if (glove)
		{
			glove->SetDestroyedOnRecreateEntities();

			// the compiler does not know which release should it use
			// if it's refcounted: ((CRefCounted*)(glove))->Release();
			// if it's material: ((IMaterial*)(glove))->Release();
			// @Note: I renamed the function to ReleaseNetworkable to avoid confusion. ( it was neither material or CRef )
			glove->ReleaseNetworkable();
		}
		return;
	}

	if (!glove)
	{
		const int highestEntityIndex = i::EntityList->GetHighestEntityIndex();
		const int entry = highestEntityIndex + 1;
		const int serial = rand() % 0x1000;
		glove = MakeGlove(entry, serial);   // He he
		wearables[0] = entry | serial << 16;
		glove_handle = wearables[0]; // Let's store it in case we somehow lose it.
	}

	if (glove)
	{
		ApplyGlove(glove, 
			skinChanger.GetGloveIdFromMenu(cfg::skin::iGloveModel),
			skinChanger.SkinKits.at(cfg::skin::iSkinId[35]).m_nID,
			i::ModelInfo->GetModelIndex(mapGloveList.at( skinChanger.GetGloveIdFromMenu(cfg::skin::iGloveModel))),
			0, 
			cfg::skin::flSkinWear[ 35 ] );

		glove->GetItemIDHigh() = -1;
		glove->GetFallbackStatTrak() = -1;

		glove->GetClientNetworkable()->PreDataUpdate(DATA_UPDATE_CREATED);
	}
}

bool CSkinChanger::ApplyKnifeModel(CBaseCombatWeapon* pWeapon, const char* szModel)
{
	if (g::pLocal == nullptr)
		return false;

	CBaseViewModel* pViewmodel = (CBaseViewModel*)i::EntityList->GetClientEntityFromHandle(g::pLocal->GetViewModel());
	if (!pViewmodel)
		return false;

	CBaseHandle pWeaponHandle = pViewmodel->GetWeaponHandle();
	if (!pWeaponHandle)
		return false;

	CBaseCombatWeapon* pViewmodelWeapon = (CBaseCombatWeapon*)(i::EntityList->GetClientEntityFromHandle(pWeaponHandle));
	if (pViewmodelWeapon != pWeapon)
		return false;

	pViewmodel->GetModelIndex() = i::ModelInfo->GetModelIndex(szModel);

	return true;
}

bool CSkinChanger::ApplyKnifeSkin(CBaseCombatWeapon* pWeapon, const char* szModel, short iItemDefIndex)
{
	if (g::pLocal == nullptr)
		return false;

	pWeapon->GetItemDefinitionIndex() = iItemDefIndex;
	//pWeapon->m_iEntityQuality() = 3;
	pWeapon->GetModelIndex() = i::ModelInfo->GetModelIndex(szModel);

	CBaseHandle pWorldModelHandle = pWeapon->GetWorldModelHandle();
	if (!pWorldModelHandle)
		return false;

	CBaseCombatWeapon* pWorldModel = (CBaseCombatWeapon*)(i::EntityList->GetClientEntityFromHandle(pWorldModelHandle));
	if (!pWorldModel)
		return false;

	pWorldModel->GetModelIndex() = i::ModelInfo->GetModelIndex(szModel) + 1;

	return true;
}

void ForceItemUpdate(CBaseCombatWeapon* m_pWeapon)
{
	if (!m_pWeapon || !m_pWeapon->IsWeapon())
		return;

	if (i::ClientState->iDeltaTick == -1)
		return;

	m_pWeapon->CustomMaterialInitialized( ) = m_pWeapon->GetFallbackPaintKit( ) <= 0;
	m_pWeapon->CustomMaterialInitialized() = false;

	m_pWeapon->GetCustomMaterials().RemoveAll();
	m_pWeapon->GetCustomMaterials2().RemoveAll();

	size_t count = m_pWeapon->GetVisualsDataProcessors().Count();
	for (size_t i{ }; i < count; ++i)
	{
		auto& elem = m_pWeapon->GetVisualsDataProcessors()[i];
		if (elem)
		{
			elem->Release();
			elem = nullptr;
		}
	}

	m_pWeapon->GetVisualsDataProcessors().RemoveAll();

	m_pWeapon->PostDataUpdate(DATA_UPDATE_CREATED);
	m_pWeapon->OnDataChanged(DATA_UPDATE_CREATED);

	i::ClientState->iDeltaTick = -1;

	util::ForceFullUpdate();
}

void CSkinChanger::Run(CBaseEntity* pLocal)
{
	if (!cfg::skin::bEnableSkinChagner)
		return;

	if (pLocal == nullptr)
		return;

	HandleGloves();

	if (!pLocal->IsAlive())
		return;

	PlayerInfo_t pInfo;
	if (!i::EngineClient->GetPlayerInfo(i::EngineClient->GetLocalPlayer(), &pInfo))
		return;

	auto pWeapon = pLocal->GetWeapon();

	if (!pWeapon)
		return;

	std::vector< CBaseCombatWeapon* > pWeapons{ };

	//--------------------------------------------definitions-------------------------------------------- 

	int index_knifeCT = i::ModelInfo->GetModelIndex(mapItemList.at(WEAPON_KNIFE).szModel);
	int index_knifeT = i::ModelInfo->GetModelIndex(mapItemList.at(WEAPON_KNIFE_T).szModel);

	int XUID = pInfo.nXuidLow;

	//--------------------------------------------definitions-------------------------------------------- 

	//--------------------------------------------glove-changer--------------------------------------------

	//--------------------------------------------weapon-changer-------------------------------------------- 

	// -1 to prevent double active weapon
	for (auto nIndex : pLocal->GetWeaponsHandle())
	{
		// get current weapon
		CBaseCombatWeapon* pCurrentWeapon = static_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(nIndex));

		if (pCurrentWeapon == nullptr)
			continue;

		short& nDefinitionIndex = pCurrentWeapon->GetItemDefinitionIndex();

		CCSWeaponData* pWeaponData = reinterpret_cast<CCSWeaponData*>(pCurrentWeapon->GetCSWpnData());

		if (pWeaponData == nullptr || (pWeaponData->nWeaponType == WEAPONTYPE_GRENADE))
			continue;

		int menuId = GetRemappedWeaponIndex(nDefinitionIndex);

		for (size_t m{ 0 }; m < m_pItemSchematic->m_pPaintKits.lastAlloc; m++)
		{
			PaintKit_t* m_pPaintKit = m_pItemSchematic->m_pPaintKits.memory[m].value;

			if (m_pPaintKit->m_nID == cfg::skin::iSkinId[ menuId ])
			{
				if (cfg::skin::bModifySkinColors[ menuId ] )
				{
					// i dont even know what the hack is going on here
					// I guess its 4 std::vector<Color> ??
					// if yes, here's the code
					m_pPaintKit->m_rgbaColor[0] = cfg::skin::colSkins1[ menuId ];
					m_pPaintKit->m_rgbaColor[1] = cfg::skin::colSkins2[ menuId ];
					m_pPaintKit->m_rgbaColor[2] = cfg::skin::colSkins3[ menuId ];
					m_pPaintKit->m_rgbaColor[3] = cfg::skin::colSkins4[ menuId ];

					// original here
					/*m_pPaintKit->m_rgbaColor[0] = C::Get<std::vector<Color>>(Vars.colSkins1).at(menuId);
					m_pPaintKit->m_rgbaColor[1] = C::Get<std::vector<Color>>(Vars.colSkins2).at(menuId);
					m_pPaintKit->m_rgbaColor[2] = C::Get<std::vector<Color>>(Vars.colSkins3).at(menuId);
					m_pPaintKit->m_rgbaColor[3] = C::Get<std::vector<Color>>(Vars.colSkins4).at(menuId);*/
				}
				else
				{
					m_pPaintKit->m_rgbaColor[0] = SkinColors[m].m_colColor[0];
					m_pPaintKit->m_rgbaColor[1] = SkinColors[m].m_colColor[1];
					m_pPaintKit->m_rgbaColor[2] = SkinColors[m].m_colColor[2];
					m_pPaintKit->m_rgbaColor[3] = SkinColors[m].m_colColor[3];
				}
			}
		}

		// Change knife model.
		if (pWeapon->GetClientClass()->nClassID == EClassIndex::CKnife && cfg::skin::iKnifeModel > 0)
			ApplyKnifeModel(pCurrentWeapon, mapItemList.at(GetKnifeDefinitionIndex( cfg::skin::iKnifeModel ) ).szModel );

		// Apply knife skin.
		if (pCurrentWeapon->GetClientClass()->nClassID == EClassIndex::CKnife && cfg::skin::iKnifeModel > 0)
			ApplyKnifeSkin(pCurrentWeapon, mapItemList.at(GetKnifeDefinitionIndex( cfg::skin::iKnifeModel ) ).szModel, GetKnifeDefinitionIndex(cfg::skin::iKnifeModel));

		if (!cfg::skin::szSkinNametag[ menuId ].empty())
			strcpy(pCurrentWeapon->GetCustomName(), cfg::skin::szSkinNametag[ menuId ].c_str());

		if (cfg::skin::iSkinStattrak[ menuId ] )
		{
			pCurrentWeapon->GetFallbackStatTrak() = cfg::skin::iSkinStattrak[ menuId ];
			pCurrentWeapon->GetEntityQuality() = 9;
		}
		else
		{
			if (pCurrentWeapon->GetClientClass()->nClassID == EClassIndex::CKnife)
				pCurrentWeapon->GetEntityQuality() = 3;
			else
				pCurrentWeapon->GetEntityQuality() = 0;
		}

		pCurrentWeapon->GetFallbackPaintKit() = SkinKits.at(cfg::skin::iSkinId[ menuId ]).m_nID;
		pCurrentWeapon->GetFallbackWear() = cfg::skin::flSkinWear[ menuId ] / 100.f;

		pCurrentWeapon->GetOwnerXuidHigh() = 0;
		pCurrentWeapon->GetOwnerXuidLow() = 0;
		pCurrentWeapon->GetAccountID() = XUID;
		pCurrentWeapon->GetFallbackSeed() = 661;
		pCurrentWeapon->GetItemIDHigh() = -1;

		//ApplyStickers(pCurrentWeapon);

		// weapon that we own
		pWeapons.push_back(pCurrentWeapon);
	}

	//--------------------------------------------weapon-changer-------------------------------------------- 

	// we only want to force update every second rather than spam constant which can cause crash
	if (bshouldFullUpdate && i::GlobalVars->flCurrentTime >= flUpdateTime)
	{
		//for (auto& w : pWeapons)
		//	ForceItemUpdate(w);

		int menuId = GetRemappedWeaponIndex(g::pLocal->GetWeapon()->GetItemDefinitionIndex());

		for (size_t m{ 0 }; m < m_pItemSchematic->m_pPaintKits.lastAlloc; m++)
		{
			PaintKit_t* m_pPaintKit = m_pItemSchematic->m_pPaintKits.memory[m].value;

			if (m_pPaintKit->m_nID == cfg::skin::iSkinId[ menuId ] )
			{
				if (cfg::skin::bModifySkinColors[ menuId ] )
				{
					// i dont even know what the hack is going on here
						// I guess its 4 std::vector<Color> ??
						// if yes, here's the code
					m_pPaintKit->m_rgbaColor[0] = cfg::skin::colSkins1[ menuId ];
					m_pPaintKit->m_rgbaColor[1] = cfg::skin::colSkins2[ menuId ];
					m_pPaintKit->m_rgbaColor[2] = cfg::skin::colSkins3[ menuId ];
					m_pPaintKit->m_rgbaColor[3] = cfg::skin::colSkins4[ menuId ];

					// original here
					/*m_pPaintKit->m_rgbaColor[0] = C::Get<std::vector<Color>>(Vars.colSkins1).at(menuId);
					m_pPaintKit->m_rgbaColor[1] = C::Get<std::vector<Color>>(Vars.colSkins2).at(menuId);
					m_pPaintKit->m_rgbaColor[2] = C::Get<std::vector<Color>>(Vars.colSkins3).at(menuId);
					m_pPaintKit->m_rgbaColor[3] = C::Get<std::vector<Color>>(Vars.colSkins4).at(menuId);*/
				}
				else
				{
					m_pPaintKit->m_rgbaColor[0] = SkinColors[m].m_colColor[0];
					m_pPaintKit->m_rgbaColor[1] = SkinColors[m].m_colColor[1];
					m_pPaintKit->m_rgbaColor[2] = SkinColors[m].m_colColor[2];
					m_pPaintKit->m_rgbaColor[3] = SkinColors[m].m_colColor[3];
				}
			}
		}

		//i::ClientState->m_iDeltaTick = -1;
		if (g::pLocal != nullptr && g::pLocal->GetWeapon() != nullptr)
			ForceItemUpdate(g::pLocal->GetWeapon());

		bshouldFullUpdate = false;
		flUpdateTime = i::GlobalVars->flCurrentTime + 1.f;
	}

	// it will help you here my sweetest candy <3
	// https://www.unknowncheats.me/wiki/Counter_Strike_Global_Offensive:Skin_Changer
}

void CSkinChanger::Event(IGameEvent* pEvent)
{
	if (!cfg::skin::bEnableSkinChagner)
		return;

	if (pEvent == nullptr || !i::EngineClient->IsInGame())
		return;

	if (g::pLocal == nullptr || !g::pLocal->IsAlive())
		return;

	auto pWeapon = g::pLocal->GetWeapon();

	if (!pWeapon)
		return;

	auto defIndex = pWeapon->GetItemDefinitionIndex();

	auto menuId = GetRemappedWeaponIndex(defIndex);

	const std::string uNameHash = pEvent->GetName();

	/* update stattrak */
	if (uNameHash == cachedEvents::playerDeath)
	{
		CBaseEntity* pAttacker = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i::EngineClient->GetPlayerForUserID(pEvent->GetInt(XorStr("attacker")))));

		if (pAttacker == g::pLocal)
		{
			CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i::EngineClient->GetPlayerForUserID(pEvent->GetInt(XorStr("userid")))));

			if (pEntity != nullptr && pEntity != g::pLocal)
			{
				if (!IsKnife(defIndex))
					return;

				CCSWeaponData* pWeaponData = reinterpret_cast<CCSWeaponData*>(i::WeaponSystem->GetWpnData(g::pLocal->GetWeapon()->GetItemDefinitionIndex()));
				if (!pWeaponData)
					return;

				if (pEvent->GetString("weapon", "knife") && pWeaponData->nWeaponType == WEAPONTYPE_KNIFE)
				{
					switch (cfg::skin::iKnifeModel)
					{
					case 0:
						pEvent->SetString("weapon", "knife");
						break;
					case 1:
						pEvent->SetString("weapon", "bayonet");
						break;
					case 2:
						pEvent->SetString("weapon", "knife_m9_bayonet");
						break;
					case 3:
						pEvent->SetString("weapon", "knife_karambit");
						break;
					case 4:
						pEvent->SetString("weapon", "knife_survival_bowie");
						break;
					case 5:
						pEvent->SetString("weapon", "knife_butterfly");
						break;
					case 6:
						pEvent->SetString("weapon", "knife_falchion");
						break;
					case 7:
						pEvent->SetString("weapon", "knife_flip");
						break;
					case 8:
						pEvent->SetString("weapon", "knife_gut");
						break;
					case 9:
						pEvent->SetString("weapon", "knife_tactical");
						break;
					case 10:
						pEvent->SetString("weapon", "knife_push");
						break;
					case 11:
						pEvent->SetString("weapon", "knife_gypsy_jackknife");
						break;
					case 12:
						pEvent->SetString("weapon", "knife_stiletto");
						break;
					case 13:
						pEvent->SetString("weapon", "knife_widowmaker");
						break;
					case 14:
						pEvent->SetString("weapon", "knife_ursus");
						break;
					case 15:
						pEvent->SetString("weapon", "knife_cord");
						break;
					case 16:
						pEvent->SetString("weapon", "knife_canis");
						break;
					case 17:
						pEvent->SetString("weapon", "knife_outdoor");
						break;
					case 18:
						pEvent->SetString("weapon", "knife_skeleton");
						break;
					case 19:
						pEvent->SetString("weapon", "knife_css");
						break;
					}
				}

				if (cfg::skin::iSkinStattrak[ menuId ] )
				{
					cfg::skin::iSkinStattrak[ menuId ]++;
					ForceItemUpdate(pWeapon);
				}
			}
		}
	}

	/* reset update timer to 0 on round start */
	if (uNameHash == cachedEvents::roundStart)
	{
		flUpdateTime = 0.0f;
	}
}

void CSkinChanger::Dump()
{
	const auto V_UCS2ToUTF8 = static_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(reinterpret_cast<void*>(util::GetExportAddress(util::GetModuleBaseHandle(XorStr("vstdlib.dll")), XorStr("V_UCS2ToUTF8"))));
	std::ifstream items = std::ifstream("csgo/scripts/items/items_game_cdn.txt");
	std::string gameItems = std::string(std::istreambuf_iterator <char> { items }, std::istreambuf_iterator <char> { });

	if (!items.is_open())
		return;

	items.close();

	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	uintptr_t sig_address = (uintptr_t)MEM::FindPattern(CLIENT_DLL, XorStr("E8 ?? ?? ?? ?? FF 76 0C 8D 48 04 E8"));

	// Skip the opcode, read rel32 address
	int item_system_offset = *reinterpret_cast<int32_t*>(sig_address + 1);

	// Add the offset to the end of the instruction
	auto item_system_fn = reinterpret_cast<IItemSystem * (*)()>(sig_address + 5 + item_system_offset);

	// Skip VTable, first member variable of ItemSystem is ItemSchema
	m_pItemSchematic = reinterpret_cast<ItemSchema_t*>(uintptr_t(item_system_fn()) + sizeof(void*));

	std::vector<std::pair<short, short>> kitsWeapons;
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
			std::unordered_set<short> weapons;

			for (auto it = std::lower_bound(kitsWeapons.begin(), kitsWeapons.end(), paintKit->m_nID, [](const auto& p, auto id) { return p.first < id; }); it != kitsWeapons.end() && it->first == paintKit->m_nID; ++it)
			{
				weapons.insert(it->second);
			}

			for (short weapon : weapons)
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
	util::Print(XorStr("Skins dumped"));
}