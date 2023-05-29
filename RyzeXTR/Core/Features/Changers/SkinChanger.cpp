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

void CSkinChanger::ApplyStickers(CBaseCombatWeapon* pWeapon)
{
	const auto econ_item_interface_wrapper = reinterpret_cast<void*>(pWeapon->GetEconItemView());

	if (!detour::GetStickerAttributeBySlotIndexInt.IsHooked())
		h::HookTable(detour::GetStickerAttributeBySlotIndexInt, econ_item_interface_wrapper, 5, &h::hkGetStickerAttributeBySlotIndexInt);
		//detour::GetStickerAttributeBySlotIndexInt.Create(MEM::GetVFunc(reinterpret_cast<void*>(econ_item_interface_wrapper), 5), &H::hkGetStickerAttributeBySlotIndexInt);
}

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
		static auto set_abs_origin_addr = util::FindSignature("client.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8");
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

	if (!cfg::skin::iSkinId.at(36))
		return;

	static int originalIdx = 0;

	if (!g::pLocal)
	{
		originalIdx = 0;
		return;
	}

	if (const auto model = cfg::skin::vecSkinsImgPaths.at(36).c_str())
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
			glove->Release();
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
			U::GetGloveIdFromMenu(cfg::skin::iGloveModel), 
			cfg::skin::iSkinId.at(35),
			i::ModelInfo->GetModelIndex(mapGloveList.at(U::GetGloveIdFromMenu(cfg::skin::iGloveModel))), 
			0, 
			cfg::skin::flSkinWear.at(35));

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

	//wpn->CustomMaterialInitialized( ) = wpn->GetFallbackPaintKit( ) <= 0;
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

	if (int* hWeapons = &pLocal->GetWeaponsHandle().at(0); hWeapons != nullptr)
	{
		// -1 to prevent double active weapon
		for (int nIndex = MAX_WEAPONS - 1; hWeapons[nIndex]; nIndex--)
		{
			// get current weapon
			CBaseCombatWeapon* pCurrentWeapon = static_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(hWeapons[nIndex]));

			if (pCurrentWeapon == nullptr)
				continue;

			short& nDefinitionIndex = pCurrentWeapon->GetItemDefinitionIndex();

			CCSWeaponData* pWeaponData = reinterpret_cast<CCSWeaponData*>(pCurrentWeapon->GetCSWpnData());

			if (pWeaponData == nullptr || (pWeaponData->nWeaponType == WEAPONTYPE_GRENADE))
				continue;

			int menuId = U::GetMenuFromId(nDefinitionIndex);

			for (size_t m{ 0 }; m < m_pItemSchematic->m_pPaintKits.lastAlloc; m++)
			{
				PaintKit_t* m_pPaintKit = m_pItemSchematic->m_pPaintKits.memory[m].value;

				if (m_pPaintKit->m_nID == cfg::skin::iSkinId.at(menuId))
				{
					if (cfg::skin::bModifySkinColors.at(menuId))
					{
						// i dont even know what the hack is going on here
						// I guess its 4 std::vector<Color> ??
						// if yes, here's the code
						m_pPaintKit->m_rgbaColor[0] = cfg::skin::colSkins1.at(menuId);
						m_pPaintKit->m_rgbaColor[1] = cfg::skin::colSkins2.at(menuId);
						m_pPaintKit->m_rgbaColor[2] = cfg::skin::colSkins3.at(menuId);
						m_pPaintKit->m_rgbaColor[3] = cfg::skin::colSkins4.at(menuId);

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
				ApplyKnifeModel(pCurrentWeapon, mapItemList.at(U::GetIdFromMenu(0, cfg::skin::iKnifeModel)).szModel);

			// Apply knife skin.
			if (pCurrentWeapon->GetClientClass()->nClassID == EClassIndex::CKnife && cfg::skin::iKnifeModel > 0)
				ApplyKnifeSkin(pCurrentWeapon, mapItemList.at(U::GetIdFromMenu(0, cfg::skin::iKnifeModel)).szModel, U::GetKnifeDefinitionIndex(cfg::skin::iKnifeModel));

			if (!cfg::skin::szSkinNametag.at(menuId).empty())
				strcpy(pCurrentWeapon->GetCustomName(), cfg::skin::szSkinNametag.at(menuId).c_str());

			if (cfg::skin::iSkinStattrak.at(menuId))
			{
				pCurrentWeapon->GetFallbackStatTrak() = cfg::skin::iSkinStattrak.at(menuId);
				pCurrentWeapon->GetEntityQuality() = 9;
			}
			else
			{
				if (pCurrentWeapon->GetClientClass()->nClassID == EClassIndex::CKnife)
					pCurrentWeapon->GetEntityQuality() = 3;
				else
					pCurrentWeapon->GetEntityQuality() = 0;
			}

			pCurrentWeapon->GetFallbackPaintKit() = cfg::skin::iSkinId.at(menuId);
			pCurrentWeapon->GetFallbackWear() = cfg::skin::flSkinWear.at(menuId) / 100.f;

			pCurrentWeapon->GetOwnerXuidHigh() = 0;
			pCurrentWeapon->GetOwnerXuidLow() = 0;
			pCurrentWeapon->GetAccountID() = XUID;
			pCurrentWeapon->GetFallbackSeed() = 661;
			pCurrentWeapon->GetItemIDHigh() = -1;

			ApplyStickers(pCurrentWeapon);

			// weapon that we own
			pWeapons.push_back(pCurrentWeapon);
		}
	}

	//--------------------------------------------weapon-changer-------------------------------------------- 

	// we only want to force update every second rather than spam constant which can cause crash
	if (bshouldFullUpdate && i::GlobalVars->flCurrentTime >= flUpdateTime)
	{
		//for (auto& w : pWeapons)
		//	ForceItemUpdate(w);

		int menuId = U::GetMenuFromId(g::pLocal->GetWeapon()->GetItemDefinitionIndex());

		for (size_t m{ 0 }; m < m_pItemSchematic->m_pPaintKits.lastAlloc; m++)
		{
			PaintKit_t* m_pPaintKit = m_pItemSchematic->m_pPaintKits.memory[m].value;

			if (m_pPaintKit->m_nID == cfg::skin::iSkinId.at(menuId))
			{
				if (cfg::skin::bModifySkinColors.at(menuId))
				{
					// i dont even know what the hack is going on here
						// I guess its 4 std::vector<Color> ??
						// if yes, here's the code
					m_pPaintKit->m_rgbaColor[0] = cfg::skin::colSkins1.at(menuId);
					m_pPaintKit->m_rgbaColor[1] = cfg::skin::colSkins2.at(menuId);
					m_pPaintKit->m_rgbaColor[2] = cfg::skin::colSkins3.at(menuId);
					m_pPaintKit->m_rgbaColor[3] = cfg::skin::colSkins4.at(menuId);

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

	auto menuId = U::GetMenuFromId(defIndex);

	const std::string uNameHash = pEvent->GetName();

	/* update stattrak */
	if (uNameHash == "player_death")
	{
		CBaseEntity* pAttacker = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"))));

		if (pAttacker == g::pLocal)
		{
			CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"))));

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

				if (cfg::skin::iSkinStattrak.at(menuId))
				{
					cfg::skin::iSkinStattrak.at(menuId)++;
					ForceItemUpdate(pWeapon);
				}
			}
		}
	}

	/* reset update timer to 0 on round start */
	if (uNameHash == "round_start")
	{
		flUpdateTime = 0.0f;
	}
}