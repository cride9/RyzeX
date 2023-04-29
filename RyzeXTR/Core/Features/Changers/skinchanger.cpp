//#include "skinchanger.h"
//#include "../../globals.h"
//#include "../../SDK/math.h"
//
//uintptr_t GetItemSchema()
//{
//	static auto fnGetItemSchema
//		= reinterpret_cast<uintptr_t(__stdcall*)()>(
//			util::FindSignature("client.dll", "A1 ? ? ? ? 85 C0 75 53")
//			);
//	return fnGetItemSchema();
//}
//
//void c_skins::override_hud_icon(IGameEvent* game_event)
//{
//	// get active weapon
//	const auto weapon = g::pLocal->GetWeapon();
//
//	// make sure its valid
//	if (!weapon || !weapon->IsKnife())
//		return;
//
//	// get item definition index of current weapon
//	auto& item_definition_index = weapon->GetItemDefinitionIndex();
//
//	// get current skin config
//	const auto config = &g_skins.options.items[g_skins.is_knife(item_definition_index) ? WEAPON_KNIFE : item_definition_index];
//
//	// check for the selected knife's index
//	if (const auto def = g_interfaces.itemsystem()->get_item_schema()->get_item_definition_interface(config->definition_override_index))
//	{
//		// get definition name
//		if (const auto defName = def->get_definition_name(); defName && std::string_view{ defName }.starts_with(("weapon_")))
//		{
//			// override the icon!
//			game_event->SetString(("weapon"), defName + 7);
//		}
//	}
//}
//void c_skins::run()
//{
//	// check for local player existence
//	if (!g::pLocal)
//		return;
//
//	// get player info
//	PlayerInfo_t player_info;
//	if (!i::EngineClient->GetPlayerInfo(i::EngineClient->GetLocalPlayer(), &player_info))
//		return;
//
//	// check for update in the menu
//	this->check_update();
//
//	// if got any, update skins in a safe thread
//	if (this->force_update)
//		this->update_skins();
//
//	// check if we have a valid view model
//	const auto view_model = reinterpret_cast<CBaseViewModel*>(i::EntityList->GetClientEntityFromHandle(g::pLocal->GetViewModel()));
//	if (!view_model)
//		return;
//
//	// check for weapon a handle in our view model
//	const auto view_model_weapon = reinterpret_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(view_model->GetWeaponHandle()));
//	if (!view_model_weapon)
//		return;
//
//	// check for the item definition index
//	const auto definition = g_interfaces.itemsystem()->get_item_schema()->get_item_definition_interface(view_model_weapon->GetItemDefinitionIndex());
//	if (!definition)
//		return;
//
//	// check for valid world model
//	const auto world_model = reinterpret_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(view_model_weapon->GetWorldModelHandle()));
//	if (!world_model)
//		return;
//
//	// loop through all weapons
//	for (auto& weapons = g::pLocal->get_my_weapons(); auto weapon_handle : weapons)
//	{
//		// check for weapon handle
//		if (weapon_handle == -1)
//			break;
//
//		// get weapon from handle
//		const auto weapon = reinterpret_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(weapon_handle));
//		if (!weapon)
//			continue;
//
//		// get weapon index
//		auto& item_definition_index = weapon->GetItemDefinitionIndex();
//
//		// get current setting
//		const auto config = &options.items[is_knife(item_definition_index) ? WEAPON_KNIFE : item_definition_index];
//
//		// apply stuff we want to change about the current weapon
//		weapon->GetAccountID() = player_info.nXuidLow;
//		weapon->GetItemIDHigh() = -1;
//		weapon->GetFallbackWear() = config->wear;
//
//		if (config->seed)
//			weapon->GetFallbackSeed() = config->seed;
//
//		if (config->stat_trak + 1 && config->enable_stat_track)
//			weapon->GetFallbackStatTrak() = config->stat_trak;
//
//		// apply paintkit for weapons & knives later
//		if (!this->is_knife(item_definition_index))
//		{
//			if (config->paint_kit_index)
//				weapon->GetFallbackPaintKit() = config->paint_kit_index;
//
//			// fix stat trak hud for weapons
//			if (config->stat_trak + 1 && config->enable_stat_track)
//				weapon->GetEntityQuality() = 9;
//		}
//
//		// check for changed index
//		if (config->definition_override_index && config->definition_override_index != item_definition_index)
//		{
//			// apply new index
//			item_definition_index = config->definition_override_index;
//
//			if (const auto def = g_interfaces.itemsystem()->get_item_schema()->get_item_definition_interface(config->definition_override_index))
//			{
//				// we only wanna do this for a knife
//				if (this->is_knife(item_definition_index))
//				{
//					// override paintkit idk why do you have to do this in here but
//					// console spamming is because of the updating function
//					// when we update skins -> default knife model appears -> cheat can't override the skin of the default model
//					if (config->paint_kit_index)
//						weapon->GetFallbackPaintKit() = config->paint_kit_index;
//
//					// we wanna get that shitty star, fuck movement players
//					weapon->GetEntityQuality() = 3;
//
//					// apply knife model index
//					weapon->SetModelIndex(i::ModelInfo->GetModelIndex(def->get_player_display_model()));
//
//					// update the weapon
//					weapon->PreDataUpdate(DATA_UPDATE_CREATED);
//				}
//			}
//		}
//	}
//
//	// we need to see the knife in our hands
//	view_model->GetModelIndex() = i::ModelInfo->GetModelIndex(definition->get_player_display_model());
//
//	// we need to see the knife in third person
//	world_model->GetWorldModelIndex() = i::ModelInfo->GetModelIndex(definition->get_world_display_model());
//}
//
//void c_skins::update_skins()
//{
//	// check for some shit
//	if (g::pLocal->IsDormant() || !g::pLocal->IsAlive())
//		return;
//
//	// get hud weapons from CCSGO_HudWeaponSelection
//	if (auto hud_weapons = g_interfaces.findhudelement(g_interfaces.hud, ("CCSGO_HudWeaponSelection")) - 0x28)
//	{
//		// loop through weapons
//		for (int i = 0; i < *(hud_weapons + 32); i++)
//		{
//			// clear all hud weapons
//			i = g_interfaces.clearhudweapon(hud_weapons, i);
//		}
//	}
//
//	// get update hud func
//	typedef void(*fn) (void);
//	static fn update = (fn)g_signatures.update_hud;
//	update();
//
//	// lets update hud again to make sure user will have the correct skin name
//	static bool applied_update_time = false;
//	float update_time{};
//
//	// apply the timer
//	if (!update_time && !applied_update_time)
//	{
//		update_time = i::GlobalVars->flCurrentTime + 10.f;
//		applied_update_time = true;
//	}
//
//	// check for the right time
//	if (i::GlobalVars->flCurrentTime >= update_time)
//	{
//		// update the hud and exit from function
//		update();
//		update_time = 0;
//		applied_update_time = false;
//		this->force_update = false;
//	}
//}
//
//
//bool c_skins::apply_glove_model(CBaseCombatWeapon* glove, const char* model)
//{
//	// glove model apply function
//	*reinterpret_cast<int*>(uintptr_t(glove) + 0x64) = -1;
//	return true;
//}
//
//bool c_skins::apply_glove_skin(CBaseCombatWeapon* glove, int item_definition_index, int paint_kit, int model_index, int world_model_index, int entity_quality, float fallback_wear)
//{
//	glove->GetItemDefinitionIndex() = item_definition_index;
//	glove->GetFallbackPaintKit() = paint_kit;
//	glove->SetModelIndex(model_index);
//	glove->GetEntityQuality() = entity_quality;
//	glove->GetFallbackWear() = fallback_wear;
//
//	return true;
//}
//
//void c_skins::glove_changer()
//{
//	// check for shits
//	if (!g::pLocal || !i::EngineClient->IsInGame() && g::pCmd == nullptr)
//		return;
//
//	// check for local player info
//	PlayerInfo_t player_info;
//	if (!i::EngineClient->GetPlayerInfo(i::EngineClient->GetLocalPlayer(), &player_info))
//		return;
//
//	if (g_vars.skins.glove_model != 0)
//	{
//		// get the list of glove models
//		std::string model_blood = ("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl");
//		std::string model_sport = ("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl");
//		std::string model_slick = ("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl");
//		std::string model_leath = ("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl");
//		std::string model_moto = ("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl");
//		std::string model_speci = ("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl");
//		std::string model_hydra = ("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl");
//		std::string model_fang = ("models/weapons/v_models/arms/anarchist/v_glove_anarchist.mdl");
//
//		// get the list of glove model indexes
//		auto index_blood = i::ModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"));
//		auto index_sport = i::ModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"));
//		auto index_slick = i::ModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"));
//		auto index_leath = i::ModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"));
//		auto index_moto = i::ModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"));
//		auto index_speci = i::ModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"));
//		auto index_hydra = i::ModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl"));
//		auto index_fang = i::ModelInfo->GetModelIndex(("models/weapons/v_models/arms/anarchist/v_glove_anarchist.mdl"));
//
//		const auto wearables = g::pLocal->get_wearables();
//
//		if (!wearables)
//			return;
//
//		static UINT glove_handle = UINT(0);
//		auto glove = reinterpret_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(wearables[0]));
//
//		if (!glove)
//		{
//			const auto our_glove = reinterpret_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(glove_handle));
//
//			if (our_glove)
//			{
//				wearables[0] = glove_handle;
//				glove = our_glove;
//			}
//		}
//
//		if (!g::pLocal->IsAlive())
//		{
//			if (glove)
//			{
//				glove->SetDestroyedOnRecreateEntities();
//				glove->Release();
//			}
//			return;
//		}
//
//		if (!glove)
//		{
//			const auto entry = i::EntityList->GetHighestEntityIndex() + 1;
//			const auto serial = rand() % 0x1000;
//			glove = make_glove(entry, serial);
//			wearables[0] = entry | serial << 16;
//			glove_handle = wearables[0];
//		}
//
//		if (glove)
//		{
//			float gloves_wear = g_vars.skins.glove_wear;
//
//			switch (g_vars.skins.glove_model)
//			{
//			case 0:
//				break;
//			case 1:
//				this->apply_glove_skin(glove, GLOVE_STUDDED_BLOODHOUND, this->get_glove_skin(0, g_vars.skins.glove_skin_blood), index_blood, 0, 3, gloves_wear);
//				break;
//			case 2:
//				this->apply_glove_skin(glove, GLOVE_SPORTY, this->get_glove_skin(1, g_vars.skins.glove_skin_sport), index_sport, 0, 3, gloves_wear);
//				break;
//			case 3:
//				this->apply_glove_skin(glove, GLOVE_SLICK, this->get_glove_skin(2, g_vars.skins.glove_skin_driver), index_slick, 0, 3, gloves_wear);
//				break;
//			case 4:
//				this->apply_glove_skin(glove, GLOVE_LEATHER_HANDWRAPS, this->get_glove_skin(3, g_vars.skins.glove_skin_handwar), index_leath, 0, 3, gloves_wear);
//				break;
//			case 5:
//				this->apply_glove_skin(glove, GLOVE_MOTORCYCLE, this->get_glove_skin(4, g_vars.skins.glove_skin_motor), index_moto, 0, 3, gloves_wear);
//				break;
//			case 6:
//				this->apply_glove_skin(glove, GLOVE_SPECIALIST, this->get_glove_skin(5, g_vars.skins.glove_skin_specialist), index_speci, 0, 3, gloves_wear);
//				break;
//			case 7:
//				this->apply_glove_skin(glove, GLOVE_STUDDED_HYDRA, this->get_glove_skin(6, g_vars.skins.glove_skin_hydra), index_hydra, 0, 3, gloves_wear);
//				break;
//			case 8:
//				this->apply_glove_skin(glove, GLOVE_STUDDED_BROKENFANG, this->get_glove_skin(7, g_vars.skins.glove_skin_fang), index_fang, 0, 3, gloves_wear);
//				break;
//			}
//
//			switch (g_vars.skins.glove_model)
//			{
//			case 0:
//				break;
//			case 1:
//				this->apply_glove_model(glove, model_blood.c_str());
//				break;
//			case 2:
//				this->apply_glove_model(glove, model_sport.c_str());
//				break;
//			case 3:
//				this->apply_glove_model(glove, model_slick.c_str());
//				break;
//			case 4:
//				this->apply_glove_model(glove, model_leath.c_str());
//				break;
//			case 5:
//				this->apply_glove_model(glove, model_moto.c_str());
//				break;
//			case 6:
//				this->apply_glove_model(glove, model_speci.c_str());
//				break;
//			case 7:
//				this->apply_glove_model(glove, model_hydra.c_str());
//				break;
//			case 8:
//				this->apply_glove_model(glove, model_fang.c_str());
//				break;
//			}
//
//			glove->GetItemIDHigh() = -1;
//			glove->GetFallbackSeed() = 0;
//			glove->GetFallbackStatTrak() = -1;
//			glove->PreDataUpdate(DATA_UPDATE_CREATED);
//		}
//	}
//}
//
//int c_skins::remap_knife_animations(short index, const int sequence)
//{
//	enum sequence_t
//	{
//		sequence_default_draw = 0,
//		sequence_default_idle1 = 1,
//		sequence_default_idle2 = 2,
//		sequence_default_light_miss1 = 3,
//		sequence_default_light_miss2 = 4,
//		sequence_default_heavy_miss1 = 9,
//		sequence_default_heavy_hit1 = 10,
//		sequence_default_heavy_backstab = 11,
//		sequence_default_lookat01 = 12,
//
//		sequence_butterfly_draw = 0,
//		sequence_butterfly_draw2 = 1,
//		sequence_butterfly_lookat01 = 13,
//		sequence_butterfly_lookat03 = 15,
//
//		sequence_falchion_idle1 = 1,
//		sequence_falchion_heavy_miss1 = 8,
//		sequence_falchion_heavy_miss1_noflip = 9,
//		sequence_falchion_lookat01 = 12,
//		sequence_falchion_lookat02 = 13,
//
//		sequence_daggers_idle1 = 1,
//		sequence_daggers_light_miss1 = 2,
//		sequence_daggers_light_miss5 = 6,
//		sequence_daggers_heavy_miss2 = 11,
//		sequence_daggers_heavy_miss1 = 12,
//
//		sequence_bowie_idle1 = 1,
//	};
//
//	switch (index) {
//	case WEAPON_KNIFE_BUTTERFLY:
//		switch (sequence) {
//		case sequence_default_draw:
//			return M::RandomInt(sequence_butterfly_draw, sequence_butterfly_draw2);
//		case sequence_default_lookat01:
//			return M::RandomInt(sequence_butterfly_lookat01, sequence_butterfly_lookat03);
//		default:
//			return sequence + 1;
//		}
//	case WEAPON_KNIFE_FALCHION:
//		switch (sequence) {
//		case sequence_default_draw:
//		case sequence_default_idle1:
//		case sequence_default_heavy_hit1:
//		case sequence_default_heavy_backstab:
//			return sequence;
//		case sequence_default_heavy_miss1:
//			return M::RandomInt(sequence_falchion_heavy_miss1, sequence_falchion_heavy_miss1_noflip);
//		case sequence_default_lookat01:
//			return M::RandomInt(sequence_falchion_lookat01, sequence_falchion_lookat02);
//		default:
//			return sequence - 1;
//		}
//	case WEAPON_KNIFE_PUSH:
//		switch (sequence) {
//		case sequence_default_idle2:
//			return sequence_daggers_idle1;
//		case sequence_default_light_miss1:
//		case sequence_default_light_miss2:
//			return M::RandomInt(sequence_daggers_light_miss1, sequence_daggers_light_miss5);
//		case sequence_default_heavy_miss1:
//			return M::RandomInt(sequence_daggers_heavy_miss2, sequence_daggers_heavy_miss1);
//		case sequence_default_heavy_hit1:
//		case sequence_default_heavy_backstab:
//		case sequence_default_lookat01:
//			return sequence + 3;
//		case sequence_default_draw:
//		case sequence_default_idle1:
//			return sequence;
//		default:
//			return sequence + 2;
//		}
//	case WEAPON_KNIFE_SURVIVAL_BOWIE:
//		switch (sequence) {
//		case sequence_default_draw:
//		case sequence_default_idle1:
//			return sequence;
//		case sequence_default_idle2:
//			return sequence_bowie_idle1;
//		default:
//			return sequence - 1;
//		}
//	case WEAPON_KNIFE_URSUS:
//	case WEAPON_KNIFE_SKELETON:
//	case WEAPON_KNIFE_OUTDOOR:
//	case WEAPON_KNIFE_CORD:
//	case WEAPON_KNIFE_CANIS:
//		switch (sequence) {
//		case sequence_default_draw:
//			return M::RandomInt(sequence_butterfly_draw, sequence_butterfly_draw2);
//		case sequence_default_lookat01:
//			return M::RandomInt(sequence_butterfly_lookat01, sequence_t(14));
//		default:
//			return sequence + 1;
//		}
//	case WEAPON_KNIFE_STILETTO:
//		switch (sequence) {
//		case sequence_default_lookat01:
//			return M::RandomInt(12, 13);
//		}
//	case WEAPON_KNIFE_WIDOWMAKER:
//		switch (sequence) {
//		case sequence_default_lookat01:
//			return M::RandomInt(14, 15);
//		}
//	default:
//		return sequence;
//	}
//}
//
//void c_skins::check_update()
//{
//	static int knife = 0;
//	static int glovemodel = 0;
//	static int glove_blood = 0;
//	static int glove_sport = 0;
//	static int glove_driver = 0;
//	static int glove_handwar = 0;
//	static int glove_motor = 0;
//	static int glove_specialist = 0;
//	static int glove_hydra = 0;
//	static int glove_fang = 0;
//	if (glove_hydra != g_vars.skins.glove_skin_hydra || glove_fang != g_vars.skins.glove_skin_fang || glove_specialist != g_vars.skins.glove_skin_specialist || glove_motor != g_vars.skins.glove_skin_motor || glove_handwar != g_vars.skins.glove_skin_handwar || glove_driver != g_vars.skins.glove_skin_driver || glove_sport != g_vars.skins.glove_skin_sport || glove_blood != g_vars.skins.glove_skin_blood || glovemodel != g_vars.skins.glove_model || knife != g_vars.skins.knife_model)
//	{
//		if (i::EngineClient->IsInGame() && g::pLocal && g::pLocal->IsAlive() && g::pLocal->GetWeapon())
//		{
//			force_update = true;
//		}
//
//		glovemodel = g_vars.skins.glove_model;
//		glove_blood = g_vars.skins.glove_skin_blood;
//		glove_sport = g_vars.skins.glove_skin_sport;
//		glove_driver = g_vars.skins.glove_skin_driver;
//		glove_handwar = g_vars.skins.glove_skin_handwar;
//		glove_motor = g_vars.skins.glove_skin_motor;
//		glove_specialist = g_vars.skins.glove_skin_specialist;
//		glove_hydra = g_vars.skins.glove_skin_hydra;
//		glove_fang = g_vars.skins.glove_skin_fang;
//		knife = g_vars.skins.knife_model;
//	}
//
//	auto& selected_entry = options.items[weapon_names_full[weapon_vector_index_skins].definition_index];
//
//	static int paint_kint = 0;
//	static bool statrak_bool = false;
//	static int statrak = 0;
//	static int seed = 0;
//	static float wear = 0.0;
//	static float glove_wear = 0.0;
//
//	if (paint_kint != selected_entry.paint_kit_index || seed != selected_entry.seed || wear != selected_entry.wear || glove_wear != g_vars.skins.glove_wear || statrak != selected_entry.stat_trak || statrak_bool != selected_entry.enable_stat_track)
//	{
//		if (i::EngineClient->IsInGame() && g::pLocal && g::pLocal->IsAlive() && g::pLocal->GetWeapon())
//		{
//			force_update = true;
//		}
//		paint_kint = selected_entry.paint_kit_index;
//		seed = selected_entry.seed;
//		statrak_bool = selected_entry.enable_stat_track;
//		statrak = selected_entry.stat_trak;
//		wear = selected_entry.wear;
//		wear = selected_entry.wear;
//		glove_wear = g_vars.skins.glove_wear;
//	}
//}
//
//void c_skins::agent_changer()
//{
//	if (!g::pLocal || !i::EngineClient->IsInGame())
//		return;
//
//	static auto backup_model_index = -1;
//	//if (g_vars.skins.enable_player)
//	{
//		const char** player_model_index = nullptr;
//		auto player_model = 0;
//
//		switch (g::pLocal->GetTeam())
//		{
//		case 2:
//			player_model_index = player_model_index_t;
//			player_model = g_vars.skins.player_models_t;
//			break;
//		case 3:
//			player_model_index = player_model_index_ct;
//			player_model = g_vars.skins.player_models_ct;
//			break;
//		}
//
//		if (player_model)
//		{
//			if (!backup_model)
//			{
//				auto model = g::pLocal->GetModel();
//
//				if (model)
//				{
//					auto studio_model = i::ModelInfo->GetStudioModel(model);
//
//					if (studio_model)
//					{
//						auto name = ("models/") + (std::string)studio_model->szName;
//						backup_model_index = i::ModelInfo->GetModelIndex(name.c_str());
//					}
//				}
//			}
//
//			if (player_model_indexes.find(player_model_index[player_model - 1]) == player_model_indexes.end())
//				player_model_indexes.emplace(player_model_index[player_model - 1], i::ModelInfo->GetModelIndex(player_model_index[player_model - 1]));
//
//			//g::pLocal->set_model_index(player_model_indexes[player_model_index[player_model - 1]]);
//			backup_model = true;
//		}
//		else if (backup_model)
//		{
//			//g::pLocal->set_model_index(backup_model_index);
//			backup_model = false;
//		}
//	}
//}