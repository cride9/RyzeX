//#include "skinchanger.h"
//#include "../../utilities.h"
//#include "../../Interface/interfaces.h"
//
//void SkinChanger::Dump( )
//{
//	const auto V_UCS2ToUTF8 = static_cast< int( * )( const wchar_t* ucs2, char* utf8, int len ) >( util::GetExportAddress( "vstdlib.dll", "V_UCS2ToUTF8" ) );
//	std::ifstream items = std::ifstream( "csgo/scripts/items/items_game_cdn.txt" );
//	std::string gameItems = std::string( std::istreambuf_iterator <char> { items }, std::istreambuf_iterator <char> { } );
//
//	if ( !items.is_open( ) )
//		return;
//
//	items.close( );
//
//	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
//
//	uintptr_t sig_address = util::FindSignature( "client.dll", "E8 ?? ?? ?? ?? FF 76 0C 8D 48 04 E8" );
//
//	// Skip the opcode, read rel32 address
//	int item_system_offset = *reinterpret_cast< int32_t* >( sig_address + 1 );
//
//	// Add the offset to the end of the instruction
//	auto item_system_fn = reinterpret_cast< IItemSystem * ( * )( ) >( sig_address + 5 + item_system_offset );
//
//	// Skip VTable, first member variable of ItemSystem is ItemSchema
//	m_pItemSchematic = reinterpret_cast< IItemSchema* >( uintptr_t( item_system_fn( ) ) + sizeof( void* ) );
//
//	std::vector<std::pair<short, EItemDefinitionIndex>> kitsWeapons;
//	kitsWeapons.reserve( 2000 );
//
//	for ( int i = 0; i < m_pItemSchematic->getLootListCount( ); ++i )
//	{
//		const auto& contents = m_pItemSchematic->getLootList( i )->getLootListContents( );
//
//		for ( int j = 0; j < contents.size; ++j ) {
//			if ( contents[ j ].paintKit != 0 )
//				kitsWeapons.emplace_back( contents[ j ].paintKit, contents[ j ].weaponId( ) );
//		}
//	}
//
//	for ( int i = 0; i < m_pItemSchematic->getItemSetCount( ); ++i )
//	{
//		const auto set = m_pItemSchematic->getItemSet( i );
//
//		for ( int j = 0; j < set->getItemCount( ); ++j ) {
//			const auto paintKit = set->getItemPaintKit( j );
//			if ( paintKit != 0 )
//				kitsWeapons.emplace_back( paintKit, set->getItemDef( j ) );
//		}
//	}
//
//	std::sort( kitsWeapons.begin( ), kitsWeapons.end( ), []( const auto& a, const auto& b ) { return a.first < b.first; } );
//
//	SkinKits.reserve( m_pItemSchematic->m_pPaintKits.lastAlloc );
//	GloveKits.reserve( m_pItemSchematic->m_pPaintKits.lastAlloc );
//
//	for ( int i = 0; i <= m_pItemSchematic->m_pPaintKits.lastAlloc; i++ )
//	{
//		const auto paintKit = m_pItemSchematic->m_pPaintKits.memory[ i ].value;
//
//		if ( paintKit->m_nID == 0 || paintKit->m_nID == 9001 ) // ignore workshop_default
//			continue;
//
//		if ( paintKit->m_nID >= 10000 )
//		{
//			const std::string_view gloveName{ paintKit->m_szName.data( ) };
//			std::wstring name;
//
//			if ( gloveName._Starts_with( "bloodhound" ) )
//				name = i::Localize->Find( "CSGO_Wearable_t_studdedgloves" );
//			else if ( gloveName._Starts_with( "motorcycle" ) )
//				name = i::Localize->Find( "CSGO_Wearable_v_motorcycle_glove" );
//			else if ( gloveName._Starts_with( "slick" ) )
//				name = i::Localize->Find( "CSGO_Wearable_v_slick_glove" );
//			else if ( gloveName._Starts_with( "sporty" ) )
//				name = i::Localize->Find( "CSGO_Wearable_v_sporty_glove" );
//			else if ( gloveName._Starts_with( "specialist" ) )
//				name = i::Localize->Find( "CSGO_Wearable_v_specialist_glove" );
//			else if ( gloveName._Starts_with( "operation10" ) )
//				name = i::Localize->Find( "CSGO_Wearable_t_studded_brokenfang_gloves" );
//			else if ( gloveName._Starts_with( "handwrap" ) )
//				name = i::Localize->Find( "CSGO_Wearable_v_leather_handwrap" );
//			else
//				assert( false );
//
//			name += L" | ";
//			name += i::Localize->Find( paintKit->m_szDescriptionTag.data( ) + 1 );
//
//			char nameStr[ 256 ];
//			V_UCS2ToUTF8( name.c_str( ), nameStr, sizeof( nameStr ) );
//
//			GloveKits.push_back( { paintKit->m_nID, nameStr, paintKit->m_szName.data( ), paintKit->m_nRarity,0 } );
//		}
//		else
//		{
//			std::unordered_set<EItemDefinitionIndex> weapons;
//
//			for ( auto it = std::lower_bound( kitsWeapons.begin( ), kitsWeapons.end( ), paintKit->m_nID, []( const auto& p, auto id ) { return p.first < id; } ); it != kitsWeapons.end( ) && it->first == paintKit->m_nID; ++it )
//			{
//				weapons.insert( it->second );
//			}
//
//			for ( EItemDefinitionIndex weapon : weapons )
//			{
//				const auto itemDef = m_pItemSchematic->getItemDefinitionInterface( weapon );
//				if ( !itemDef )
//					continue;
//
//				std::wstring name = i::Localize->Find( itemDef->getItemBaseName( ) );
//				name += L" | ";
//				name += i::Localize->Find( paintKit->m_szDescriptionTag.data( ) + 1 );
//
//				char nameStr[ 256 ];
//				V_UCS2ToUTF8( name.c_str( ), nameStr, sizeof( nameStr ) );
//
//				SkinKits.push_back( { paintKit->m_nID, nameStr, paintKit->m_szName.data( ) ,std::clamp( itemDef->getRarity( ) + paintKit->m_nRarity - 1, 0, ( paintKit->m_nRarity == 7 ) ? 7 : 6 ),( int )weapon } );
//			}
//
//			if ( weapons.empty( ) || weapons.size( ) > 1 ) // this paint kit fits more than one weapon
//			{
//				std::wstring name = i::Localize->Find( paintKit->m_szDescriptionTag.data( ) + 1 );
//
//				char nameStr[ 256 ];
//				V_UCS2ToUTF8( name.c_str( ), nameStr, sizeof( nameStr ) );
//
//				SkinKits.push_back( { paintKit->m_nID, nameStr,paintKit->m_szName.data( ), paintKit->m_nRarity, 0 } );
//			}
//		}
//	}
//
//	std::sort( SkinKits.begin( ), SkinKits.end( ) );
//	SkinKits.shrink_to_fit( );
//	std::sort( GloveKits.begin( ), GloveKits.end( ) );
//	GloveKits.shrink_to_fit( );
//
//	//Handle stickers
//	StickerKits.reserve( m_pItemSchematic->stickerKits.lastAlloc );
//	for ( int i = 0; i <= m_pItemSchematic->stickerKits.lastAlloc; i++ )
//	{
//		const auto stickerKit = m_pItemSchematic->stickerKits.memory[ i ].value;
//		if ( std::string_view name{ stickerKit->name.data( ) }; name._Starts_with( "spray" ) || name._Starts_with( "patch" ) )
//			continue;
//		std::wstring name = i::Localize->Find( stickerKit->id != 242 ? stickerKit->itemName.data( ) + 1 : "StickerKit_dhw2014_teamdignitas_gold" );
//
//		char nameStr[ 256 ];
//		V_UCS2ToUTF8( name.c_str( ), nameStr, sizeof( nameStr ) );
//
//		StickerKits.push_back( { stickerKit->id, std::move( nameStr ),stickerKit->inventoryImage.data( ), stickerKit->rarity,0 } );
//	}
//
//	StickerKits.insert( StickerKits.begin( ), { 0, "None" } );
//	std::sort( StickerKits.begin( ) + 1, StickerKits.end( ) );
//	StickerKits.shrink_to_fit( );
//
//	for ( size_t m{ 0 }; m < m_pItemSchematic->m_pPaintKits.lastAlloc; m++ )
//	{
//		PaintKit_t* m_pPaintKit = m_pItemSchematic->m_pPaintKits.memory[ m ].value;
//
//		SkinColors_t inf;
//		inf.m_colColor[ 0 ] = m_pPaintKit->m_rgbaColor[ 0 ];
//		inf.m_colColor[ 1 ] = m_pPaintKit->m_rgbaColor[ 1 ];
//		inf.m_colColor[ 2 ] = m_pPaintKit->m_rgbaColor[ 2 ];
//		inf.m_colColor[ 3 ] = m_pPaintKit->m_rgbaColor[ 3 ];
//		SkinColors.push_back( inf );
//	}
//}
//
//
////void c_skins::override_hud_icon(i_game_event* game_event)
////{
////	// get active weapon
////	const auto weapon = g_client.local->get_active_weapon();
////
////	// make sure its valid
////	if (!weapon || !weapon->is_knife())
////		return;
////
////	// get item definition index of current weapon
////	auto& item_definition_index = weapon->get_item_definition_index();
////
////	// get current skin config
////	const auto config = &g_skins.options.items[g_skins.is_knife(item_definition_index) ? weapon_knife : item_definition_index];
////
////	// check for the selected knife's index
////	if (const auto def = g_interfaces.itemsystem()->get_item_schema()->get_item_definition_interface(config->definition_override_index))
////	{
////		// get definition name
////		if (const auto defName = def->get_definition_name(); defName && std::string_view{ defName }.starts_with(XOR("weapon_")))
////		{
////			// override the icon!
////			game_event->set_string(XOR("weapon"), defName + 7);
////		}
////	}
////}
////void c_skins::run()
////{
////	// check for local player existence
////	if (!g_client.local)
////		return;
////
////	// get player info
////	player_info_t player_info;
////	if (!g_interfaces.engine->get_player_info(g_interfaces.engine->get_local_player(), &player_info))
////		return;
////
////	// check for update in the menu
////	this->check_update();
////
////	// if got any, update skins in a safe thread
////	if (this->force_update)
////		this->update_skins();
////
////	// check if we have a valid view model
////	const auto view_model = reinterpret_cast<base_view_model_t*>(g_interfaces.entitylist->get_client_entity_from_handle(g_client.local->get_view_model()));
////	if (!view_model)
////		return;
////
////	// check for weapon a handle in our view model
////	const auto view_model_weapon = reinterpret_cast<weapon_t*>(g_interfaces.entitylist->get_client_entity_from_handle(view_model->get_weapon_handle()));
////	if (!view_model_weapon)
////		return;
////
////	// check for the item definition index
////	const auto definition = g_interfaces.itemsystem()->get_item_schema()->get_item_definition_interface(view_model_weapon->get_item_definition_index());
////	if (!definition)
////		return;
////
////	// check for valid world model
////	const auto world_model = reinterpret_cast<weapon_t*>(g_interfaces.entitylist->get_client_entity_from_handle(view_model_weapon->get_world_model()));
////	if (!world_model)
////		return;
////
////	// loop through all weapons
////	for (auto& weapons = g_client.local->get_my_weapons(); auto weapon_handle : weapons)
////	{
////		// check for weapon handle
////		if (weapon_handle == -1)
////			break;
////
////		// get weapon from handle
////		const auto weapon = reinterpret_cast<base_attributable_item_t*>(g_interfaces.entitylist->get_client_entity_from_handle(weapon_handle));
////		if (!weapon)
////			continue;
////
////		// get weapon index
////		auto& item_definition_index = weapon->get_item_definition_index();
////
////		// get current setting
////		const auto config = &options.items[is_knife(item_definition_index) ? weapon_knife : item_definition_index];
////
////		// apply stuff we want to change about the current weapon
////		weapon->get_account_id() = player_info.xuid_low;
////		weapon->get_item_id_high() = -1;
////		weapon->get_fallback_wear() = config->wear;
////
////		if (config->seed)
////			weapon->get_fallback_seed() = config->seed;
////
////		if (config->stat_trak + 1 && config->enable_stat_track)
////			weapon->get_fallback_stat_trak() = config->stat_trak;
////
////		// apply paintkit for weapons & knives later
////		if (!this->is_knife(item_definition_index))
////		{
////			if (config->paint_kit_index)
////				weapon->get_fallback_paint_kit() = config->paint_kit_index;
////
////			// fix stat trak hud for weapons
////			if (config->stat_trak + 1 && config->enable_stat_track)
////				weapon->get_entity_quality() = 9;
////		}
////
////		// check for changed index
////		if (config->definition_override_index && config->definition_override_index != item_definition_index)
////		{
////			// apply new index
////			item_definition_index = config->definition_override_index;
////
////			if (const auto def = g_interfaces.itemsystem()->get_item_schema()->get_item_definition_interface(config->definition_override_index))
////			{
////				// we only wanna do this for a knife
////				if (this->is_knife(item_definition_index))
////				{
////					// override paintkit idk why do you have to do this in here but
////					// console spamming is because of the updating function
////					// when we update skins -> default knife model appears -> cheat can't override the skin of the default model
////					if (config->paint_kit_index)
////						weapon->get_fallback_paint_kit() = config->paint_kit_index;
////
////					// we wanna get that shitty star, fuck movement players
////					weapon->get_entity_quality() = 3;
////
////					// apply knife model index
////					weapon->set_model_index(g_interfaces.modelinfo->get_model_index(def->get_player_display_model()));
////
////					// update the weapon
////					weapon->pre_data_update(0);
////				}
////			}
////		}
////	}
////
////	// we need to see the knife in our hands
////	view_model->get_model_index() = g_interfaces.modelinfo->get_model_index(definition->get_player_display_model());
////
////	// we need to see the knife in third person
////	world_model->get_model_index() = g_interfaces.modelinfo->get_model_index(definition->get_world_display_model());
////}
////
////void c_skins::update_skins()
////{
////	// check for some shit
////	if (g_client.local->is_dormant() || !g_client.local->is_alive())
////		return;
////
////	// get hud weapons from CCSGO_HudWeaponSelection
////	if (auto hud_weapons = g_interfaces.findhudelement(g_interfaces.hud, XOR("CCSGO_HudWeaponSelection")) - 0x28)
////	{
////		// loop through weapons
////		for (int i = 0; i < *(hud_weapons + 32); i++)
////		{
////			// clear all hud weapons
////			i = g_interfaces.clearhudweapon(hud_weapons, i);
////		}
////	}
////
////	// get update hud func
////	typedef void(*fn) (void);
////	static fn update = (fn)g_signatures.update_hud;
////	update();
////
////	// lets update hud again to make sure user will have the correct skin name
////	static bool applied_update_time = false;
////	float update_time{};
////
////	// apply the timer
////	if (!update_time && !applied_update_time)
////	{
////		update_time = g_interfaces.globalvars->curtime + 10.f;
////		applied_update_time = true;
////	}
////
////	// check for the right time
////	if (g_interfaces.globalvars->curtime >= update_time)
////	{
////		// update the hud and exit from function
////		update();
////		update_time = 0;
////		applied_update_time = false;
////		this->force_update = false;
////	}
////}
////
////
////bool c_skins::apply_glove_model(base_attributable_item_t* glove, const char* model)
////{
////	// glove model apply function
////	*reinterpret_cast<int*>(uintptr_t(glove) + 0x64) = -1;
////	return true;
////}
////
////bool c_skins::apply_glove_skin(base_attributable_item_t* glove, int item_definition_index, int paint_kit, int model_index, int world_model_index, int entity_quality, float fallback_wear)
////{
////	glove->get_item_definition_index() = item_definition_index;
////	glove->get_fallback_paint_kit() = paint_kit;
////	glove->set_model_index(model_index);
////	glove->get_entity_quality() = entity_quality;
////	glove->get_fallback_wear() = fallback_wear;
////
////	return true;
////}
////
////void c_skins::glove_changer()
////{
////	// check for shits
////	if (!g_client.local || !g_interfaces.engine->is_in_game() && g_client.cmd == nullptr)
////		return;
////
////	// check for local player info
////	player_info_t player_info;
////	if (!g_interfaces.engine->get_player_info(g_interfaces.engine->get_local_player(), &player_info))
////		return;
////
////	if (g_vars.skins.glove_model != 0)
////	{
////		// get the list of glove models
////		std::string model_blood = XOR("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl");
////		std::string model_sport = XOR("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl");
////		std::string model_slick = XOR("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl");
////		std::string model_leath = XOR("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl");
////		std::string model_moto = XOR("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl");
////		std::string model_speci = XOR("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl");
////		std::string model_hydra = XOR("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl");
////		std::string model_fang = XOR("models/weapons/v_models/arms/anarchist/v_glove_anarchist.mdl");
////
////		// get the list of glove model indexes
////		auto index_blood = g_interfaces.modelinfo->get_model_index(XOR("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"));
////		auto index_sport = g_interfaces.modelinfo->get_model_index(XOR("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"));
////		auto index_slick = g_interfaces.modelinfo->get_model_index(XOR("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"));
////		auto index_leath = g_interfaces.modelinfo->get_model_index(XOR("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"));
////		auto index_moto = g_interfaces.modelinfo->get_model_index(XOR("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"));
////		auto index_speci = g_interfaces.modelinfo->get_model_index(XOR("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"));
////		auto index_hydra = g_interfaces.modelinfo->get_model_index(XOR("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl"));
////		auto index_fang = g_interfaces.modelinfo->get_model_index(XOR("models/weapons/v_models/arms/anarchist/v_glove_anarchist.mdl"));
////
////		const auto wearables = g_client.local->get_wearables();
////
////		if (!wearables)
////			return;
////
////		static UINT glove_handle = UINT(0);
////		auto glove = reinterpret_cast<base_attributable_item_t*>(g_interfaces.entitylist->get_client_entity_from_handle(wearables[0]));
////
////		if (!glove)
////		{
////			const auto our_glove = reinterpret_cast<base_attributable_item_t*>(g_interfaces.entitylist->get_client_entity_from_handle(glove_handle));
////
////			if (our_glove)
////			{
////				wearables[0] = glove_handle;
////				glove = our_glove;
////			}
////		}
////
////		if (!g_client.local->is_alive())
////		{
////			if (glove)
////			{
////				glove->set_destroyed_on_recreate_entities();
////				glove->release();
////			}
////			return;
////		}
////
////		if (!glove)
////		{
////			const auto entry = g_interfaces.entitylist->get_highest_entity_index() + 1;
////			const auto serial = rand() % 0x1000;
////			glove = make_glove(entry, serial);
////			wearables[0] = entry | serial << 16;
////			glove_handle = wearables[0];
////		}
////
////		if (glove)
////		{
////			float gloves_wear = g_vars.skins.glove_wear;
////
////			switch (g_vars.skins.glove_model)
////			{
////			case 0:
////				break;
////			case 1:
////				this->apply_glove_skin(glove, glove_studded_bloodhound, this->get_glove_skin(0, g_vars.skins.glove_skin_blood), index_blood, 0, 3, gloves_wear);
////				break;
////			case 2:
////				this->apply_glove_skin(glove, glove_sporty, this->get_glove_skin(1, g_vars.skins.glove_skin_sport), index_sport, 0, 3, gloves_wear);
////				break;
////			case 3:
////				this->apply_glove_skin(glove, glove_slick, this->get_glove_skin(2, g_vars.skins.glove_skin_driver), index_slick, 0, 3, gloves_wear);
////				break;
////			case 4:
////				this->apply_glove_skin(glove, glove_leather_wrap, this->get_glove_skin(3, g_vars.skins.glove_skin_handwar), index_leath, 0, 3, gloves_wear);
////				break;
////			case 5:
////				this->apply_glove_skin(glove, glove_motorcycle, this->get_glove_skin(4, g_vars.skins.glove_skin_motor), index_moto, 0, 3, gloves_wear);
////				break;
////			case 6:
////				this->apply_glove_skin(glove, glove_specialist, this->get_glove_skin(5, g_vars.skins.glove_skin_specialist), index_speci, 0, 3, gloves_wear);
////				break;
////			case 7:
////				this->apply_glove_skin(glove, glove_hydra, this->get_glove_skin(6, g_vars.skins.glove_skin_hydra), index_hydra, 0, 3, gloves_wear);
////				break;
////			case 8:
////				this->apply_glove_skin(glove, glove_studded_brokenfang, this->get_glove_skin(7, g_vars.skins.glove_skin_fang), index_fang, 0, 3, gloves_wear);
////				break;
////			}
////
////			switch (g_vars.skins.glove_model)
////			{
////			case 0:
////				break;
////			case 1:
////				this->apply_glove_model(glove, model_blood.c_str());
////				break;
////			case 2:
////				this->apply_glove_model(glove, model_sport.c_str());
////				break;
////			case 3:
////				this->apply_glove_model(glove, model_slick.c_str());
////				break;
////			case 4:
////				this->apply_glove_model(glove, model_leath.c_str());
////				break;
////			case 5:
////				this->apply_glove_model(glove, model_moto.c_str());
////				break;
////			case 6:
////				this->apply_glove_model(glove, model_speci.c_str());
////				break;
////			case 7:
////				this->apply_glove_model(glove, model_hydra.c_str());
////				break;
////			case 8:
////				this->apply_glove_model(glove, model_fang.c_str());
////				break;
////			}
////
////			glove->get_item_id_high() = -1;
////			glove->get_fallback_seed() = 0;
////			glove->get_fallback_stat_trak() = -1;
////			glove->pre_data_update(0);
////		}
////	}
////}
////
////bool compareFunction(std::string a, std::string b) { return a < b; }
////void c_skins::menu()
////{
////	static char search_bar[32];
////
////	if (g_interfaces.engine->is_in_game() && g_client.cmd != nullptr)
////		if (g_client.local->is_alive())
////			g_skins.get_current_weapon(&weapon_index_skins, &weapon_vector_index_skins, weapon_names_full);
////
////	auto& selected_entry = options.items[weapon_names_full[weapon_vector_index_skins].definition_index];
////	selected_entry.definition_index = weapon_index_skins;
////	selected_entry.definition_vector_index = weapon_vector_index_skins;
////	bool is_gloves = false;
////
////	ImGui::MenuChild(XOR("models"), ImVec2(g_menu.menu_size.x / 2 - (15 * 2) - 70, g_menu.menu_size.y / 2 - 55 + 53 + 6 + 10), ImVec2(165, 44));
////	{
////		const char* knifemodels[20] = { "default", "bayonet", "classic", "paracord", "survival", "nomad", "skeleton", "flip", "gut", "karambit", "m9 bayonet", "huntsman", "falchion", "bowie", "butterfly", "shadow daggers", "ursus", "navaja", "stiletto", "talon" };
////
////		if (selected_entry.definition_index == weapon_knife)
////		{
////			ImGui::Combo(XOR("knife model"), &g_vars.skins.knife_model, knifemodels, 20, 20, true);
////			selected_entry.definition_override_vector_index = g_vars.skins.knife_model;
////			selected_entry.definition_override_index = knife_names.at(selected_entry.definition_override_vector_index).definition_index;
////		}
////		else
////		{
////			ImGui::Combo(XOR("knife model"), &g_vars.skins.knife_model, knifemodels, 20, 20, true);
////		}
////
////		ImGui::Combo(XOR("glove model"), &g_vars.skins.glove_model, glovemodels, 9, 9, true);
////
////		switch (g_vars.skins.glove_model)
////		{
////		case 1:
////			ImGui::Combo(XOR("glove skin"), &g_vars.skins.glove_skin_blood, glove_skinz_blood, 4, 4, true);
////			break;
////		case 2:
////			ImGui::Combo(XOR("glove skin"), &g_vars.skins.glove_skin_sport, glove_skinz_sport, 8, 8, true);
////			break;
////		case 3:
////			ImGui::Combo(XOR("glove skin"), &g_vars.skins.glove_skin_driver, glove_skinz_driver, 8, 8, true);
////			break;
////		case 4:
////			ImGui::Combo(XOR("glove skin"), &g_vars.skins.glove_skin_handwar, glove_skinz_handwar, 8, 8, true);
////			break;
////		case 5:
////			ImGui::Combo(XOR("glove skin"), &g_vars.skins.glove_skin_motor, glove_skinz_motor, 8, 8, true);
////			break;
////		case 6:
////			ImGui::Combo(XOR("glove skin"), &g_vars.skins.glove_skin_specialist, glove_skinz_specialist, 8, 8, true);
////			break;
////		case 7:
////			ImGui::Combo(XOR("glove skin"), &g_vars.skins.glove_skin_hydra, glove_skinz_hydra, 4, 4, true);
////			break;
////		case 8:
////			ImGui::Combo(XOR("glove skin"), &g_vars.skins.glove_skin_fang, glove_skinz_fang, 4, 4, true);
////			break;
////		}
////
////		if (g_vars.skins.glove_model > 0)
////			if (ImGui::SliderFloat(XOR("glove wear"), &g_vars.skins.glove_wear, 0.f, 1.f, "%.1f"))
////				g_vars.skins.glove_wear = std::clamp(g_vars.skins.glove_wear, 0.f, 1.f);
////
////		if (ImGui::Button(XOR("apply skins"), ImVec2(ImGui::GetWindowWidth() - 26 - 7, 25)))
////		{
////			this->force_update = true;
////		}
////
////		//	ImGui::Separator();
////
////			//ImGui::Checkbox(XOR("agent changer"), &g_vars.skins.enable_player);
////
////			//if (g_vars.skins.enable_player)
////
////
////	};
////	ImGui::EndMenuChild();
////
////	ImGui::MenuChild(XOR("agent model"), ImVec2(g_menu.menu_size.x / 2 - (15 * 2) - 65 - 5, g_menu.menu_size.y / 2 - 45 - 24 - 24 + 4 - 8 - 6 - 10), ImVec2(165, 30 + 8 + 24 + 10 + 6 + 24 - 3 + g_menu.menu_size.y / 2));
////	{
////		//ImGui::Combo(XOR("counter-terrorist"), &g_vars.skins.player_models_ct, player_model_ct, ARRAYSIZE(player_model_ct), ARRAYSIZE(player_model_ct), true);
////		//ImGui::Combo(XOR("terrorist"), &g_vars.skins.player_models_t, player_model_t, ARRAYSIZE(player_model_t), ARRAYSIZE(player_model_t), true);
////	}
////	ImGui::EndMenuChild();
////
////	ImGui::MenuChild(XOR("skins"), ImVec2(g_menu.menu_size.x / 2 - (15 * 2) - 70, g_menu.menu_size.y - 60 + 2 - 1), ImVec2(g_menu.menu_size.x / 2 + 80, 44));
////	{
////		static bool filter_weapons = true;
////		static int val = 0;
////
////		ImGui::Checkbox(XOR("stat-trak"), &selected_entry.enable_stat_track);
////
////		if (selected_entry.enable_stat_track)
////		{
////			ImGui::SliderInt(XOR("kills"), &selected_entry.stat_trak, 0, 1000, "%d");
////			val = 3;
////		}
////		else
////		{
////			val = 45;
////		}
////
////		ImGui::SliderInt(XOR("weapon seed"), &selected_entry.seed, 1, 5555, "%.1f");
////
////		if (ImGui::SliderFloat(XOR("weapon wear"), &selected_entry.wear, 0.f, 1.f, "%.1f"))
////			selected_entry.wear = std::clamp(selected_entry.wear, 0.f, 1.f);
////
////		ImGui::Checkbox(XOR("filter by weapon"), &filter_weapons);
////
////		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.f, 3.5f));
////		ImGui::InputText(XOR("##search_bar_skins"), search_bar, sizeof(search_bar));
////		ImGui::PopStyleVar();
////
////		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(11.f, 8.f));
////
////		ImGui::ListBoxHeader(XOR("##paint_kits"), ImVec2(ImGui::GetWindowWidth() - 26 - 7 + 3, -54 + val + 24 - 3 - 6 - 106 + 365 + (ImGui::GetWindowHeight() - 463)));
////
////		if (ImGui::Selectable(XOR("default"), selected_entry.paint_kit_index == 0))
////			selected_entry.paint_kit_index = 0;
////
////
////		bool is_knife = selected_entry.definition_index == weapon_knife || selected_entry.definition_index == weapon_knife_t;
////		int defindex = (is_knife || selected_entry.definition_index == glove_t_side || selected_entry.definition_index == glove_ct_side) ? selected_entry.definition_override_index : selected_entry.definition_index;
////
////		std::string skin_name = g_skins.get_weapon_name_by_id(defindex);
////
////		if (skin_name.compare("") != 0 || !filter_weapons)
////		{
////			std::string filter = std::string(search_bar);
////
////			int current = -1;
////			int s = 0;
////
////			auto set = weapon_skins[skin_name];
////			std::vector<std::string> list(set.size());
////			std::vector<std::string> anti_repeat;
////
////
////			std::copy(set.begin(), set.end(), list.begin());
////			std::sort(list.begin(), list.end(), compareFunction);//sort the vector
////
////			if (!filter_weapons)
////			{
////				list = {};
////				for (auto [key, value] : weapon_skins)
////				{
////					for (auto skin : value)
////					{
////
////						list.push_back(skin);
////
////					}
////				}
////			}
////
////			auto anti_repeat_fn = [&](std::string tx) -> bool
////			{
////				auto ret = std::find(anti_repeat.begin(), anti_repeat.end(), tx) == anti_repeat.end();
////
////				if (ret)
////					anti_repeat.push_back(tx);
////
////				return ret;
////			};
////
////
////
////			for (auto skin : list)
////			{
////
////				int pk = skin_map[skin].paintkit;
////				if (pk == selected_entry.paint_kit_index)
////					current = s;
////
////				bool passed_filter = true;
////
////				if (!filter.empty())
////				{
////					std::string name = skin_names[skin_map[skin].tag_name];
////
////					std::string query = filter;
////
////					std::transform(name.begin(), name.end(), name.begin(), ::tolower);
////					std::transform(query.begin(), query.end(), query.begin(), ::tolower);
////					if (name.find(query) == std::string::npos)
////						passed_filter = false;
////				}
////
////				if (passed_filter && anti_repeat_fn(skin_names[skin_map[skin].tag_name]))
////				{
////					std::transform(skin_names[skin_map[skin].tag_name].begin(), skin_names[skin_map[skin].tag_name].end(), skin_names[skin_map[skin].tag_name].begin(), ::tolower);
////
////					if (!skin_names[skin_map[skin].tag_name].contains("doppler"))
////					{
////						if (ImGui::Selectable((skin_names[skin_map[skin].tag_name]).c_str(), pk == selected_entry.paint_kit_index))
////						{
////							selected_entry.paint_kit_index = pk;
////						}
////					}
////
////				}
////
////				s++;
////			}
////		}
////		if (!filter_weapons || g_vars.skins.knife_model > 0 && selected_entry.definition_index == weapon_knife)
////		{
////			// hey :D
////			if (std::string(search_bar) == ""
////				|| std::string(search_bar).starts_with(XOR("do"))
////				|| std::string(search_bar).starts_with(XOR("ga"))
////				|| std::string(search_bar).contains(XOR("dop")))
////			{
////				if (ImGui::Selectable(XOR("doppler ##1"), selected_entry.paint_kit_index == 415))
////					selected_entry.paint_kit_index = 415;
////
////				if (ImGui::Selectable(XOR("doppler ##2"), selected_entry.paint_kit_index == 416))
////					selected_entry.paint_kit_index = 416;
////
////				if (ImGui::Selectable(XOR("doppler ##3"), selected_entry.paint_kit_index == 417))
////					selected_entry.paint_kit_index = 417;
////
////				if (ImGui::Selectable(XOR("doppler ##4"), selected_entry.paint_kit_index == 418))
////					selected_entry.paint_kit_index = 418;
////
////				if (ImGui::Selectable(XOR("doppler ##5"), selected_entry.paint_kit_index == 419))
////					selected_entry.paint_kit_index = 419;
////
////				if (ImGui::Selectable(XOR("doppler ##6"), selected_entry.paint_kit_index == 420))
////					selected_entry.paint_kit_index = 420;
////
////				if (ImGui::Selectable(XOR("doppler ##7"), selected_entry.paint_kit_index == 421))
////					selected_entry.paint_kit_index = 421;
////
////				if (ImGui::Selectable(XOR("doppler ##8"), selected_entry.paint_kit_index == 617))
////					selected_entry.paint_kit_index = 617;
////
////				if (ImGui::Selectable(XOR("doppler ##9"), selected_entry.paint_kit_index == 618))
////					selected_entry.paint_kit_index = 618;
////
////				if (ImGui::Selectable(XOR("gamma doppler ##emerald"), selected_entry.paint_kit_index == 568))
////					selected_entry.paint_kit_index = 568;
////
////				if (ImGui::Selectable(XOR("gamma doppler ##1"), selected_entry.paint_kit_index == 569))
////					selected_entry.paint_kit_index = 569;
////
////				if (ImGui::Selectable(XOR("gamma doppler ##2"), selected_entry.paint_kit_index == 570))
////					selected_entry.paint_kit_index = 570;
////
////				if (ImGui::Selectable(XOR("gamma doppler ##3"), selected_entry.paint_kit_index == 571))
////					selected_entry.paint_kit_index = 571;
////
////				if (ImGui::Selectable(XOR("gamma doppler ##4"), selected_entry.paint_kit_index == 572))
////					selected_entry.paint_kit_index = 572;
////			}
////		}
////
////		ImGui::ListBoxFooter();
////		ImGui::PopStyleVar();
////	}
////	ImGui::EndMenuChild();
////
////	this->first_time_render = false;
////}
////
////int c_skins::remap_knife_animations(short index, const int sequence)
////{
////	enum sequence_t
////	{
////		sequence_default_draw = 0,
////		sequence_default_idle1 = 1,
////		sequence_default_idle2 = 2,
////		sequence_default_light_miss1 = 3,
////		sequence_default_light_miss2 = 4,
////		sequence_default_heavy_miss1 = 9,
////		sequence_default_heavy_hit1 = 10,
////		sequence_default_heavy_backstab = 11,
////		sequence_default_lookat01 = 12,
////
////		sequence_butterfly_draw = 0,
////		sequence_butterfly_draw2 = 1,
////		sequence_butterfly_lookat01 = 13,
////		sequence_butterfly_lookat03 = 15,
////
////		sequence_falchion_idle1 = 1,
////		sequence_falchion_heavy_miss1 = 8,
////		sequence_falchion_heavy_miss1_noflip = 9,
////		sequence_falchion_lookat01 = 12,
////		sequence_falchion_lookat02 = 13,
////
////		sequence_daggers_idle1 = 1,
////		sequence_daggers_light_miss1 = 2,
////		sequence_daggers_light_miss5 = 6,
////		sequence_daggers_heavy_miss2 = 11,
////		sequence_daggers_heavy_miss1 = 12,
////
////		sequence_bowie_idle1 = 1,
////	};
////
////	switch (index) {
////	case weapon_knife_butterfly:
////		switch (sequence) {
////		case sequence_default_draw:
////			return g_interfaces.random_int(sequence_butterfly_draw, sequence_butterfly_draw2);
////		case sequence_default_lookat01:
////			return g_interfaces.random_int(sequence_butterfly_lookat01, sequence_butterfly_lookat03);
////		default:
////			return sequence + 1;
////		}
////	case weapon_knife_falchion:
////		switch (sequence) {
////		case sequence_default_draw:
////		case sequence_default_idle1:
////		case sequence_default_heavy_hit1:
////		case sequence_default_heavy_backstab:
////			return sequence;
////		case sequence_default_heavy_miss1:
////			return g_interfaces.random_int(sequence_falchion_heavy_miss1, sequence_falchion_heavy_miss1_noflip);
////		case sequence_default_lookat01:
////			return g_interfaces.random_int(sequence_falchion_lookat01, sequence_falchion_lookat02);
////		default:
////			return sequence - 1;
////		}
////	case weapon_knife_push:
////		switch (sequence) {
////		case sequence_default_idle2:
////			return sequence_daggers_idle1;
////		case sequence_default_light_miss1:
////		case sequence_default_light_miss2:
////			return g_interfaces.random_int(sequence_daggers_light_miss1, sequence_daggers_light_miss5);
////		case sequence_default_heavy_miss1:
////			return g_interfaces.random_int(sequence_daggers_heavy_miss2, sequence_daggers_heavy_miss1);
////		case sequence_default_heavy_hit1:
////		case sequence_default_heavy_backstab:
////		case sequence_default_lookat01:
////			return sequence + 3;
////		case sequence_default_draw:
////		case sequence_default_idle1:
////			return sequence;
////		default:
////			return sequence + 2;
////		}
////	case weapon_knife_survival_bowie:
////		switch (sequence) {
////		case sequence_default_draw:
////		case sequence_default_idle1:
////			return sequence;
////		case sequence_default_idle2:
////			return sequence_bowie_idle1;
////		default:
////			return sequence - 1;
////		}
////	case weapon_knife_ursus:
////	case weapon_knife_skeleton:
////	case weapon_knife_outdoor:
////	case weapon_knife_cord:
////	case weapon_knife_canis:
////		switch (sequence) {
////		case sequence_default_draw:
////			return g_interfaces.random_int(sequence_butterfly_draw, sequence_butterfly_draw2);
////		case sequence_default_lookat01:
////			return g_interfaces.random_int(sequence_butterfly_lookat01, sequence_t(14));
////		default:
////			return sequence + 1;
////		}
////	case weapon_knife_stiletto:
////		switch (sequence) {
////		case sequence_default_lookat01:
////			return g_interfaces.random_int(12, 13);
////		}
////	case weapon_knife_widowmaker:
////		switch (sequence) {
////		case sequence_default_lookat01:
////			return g_interfaces.random_int(14, 15);
////		}
////	default:
////		return sequence;
////	}
////}
////
////void c_skins::check_update()
////{
////	static int knife = 0;
////	static int glovemodel = 0;
////	static int glove_blood = 0;
////	static int glove_sport = 0;
////	static int glove_driver = 0;
////	static int glove_handwar = 0;
////	static int glove_motor = 0;
////	static int glove_specialist = 0;
////	static int glove_hydra = 0;
////	static int glove_fang = 0;
////	if (glove_hydra != g_vars.skins.glove_skin_hydra || glove_fang != g_vars.skins.glove_skin_fang || glove_specialist != g_vars.skins.glove_skin_specialist || glove_motor != g_vars.skins.glove_skin_motor || glove_handwar != g_vars.skins.glove_skin_handwar || glove_driver != g_vars.skins.glove_skin_driver || glove_sport != g_vars.skins.glove_skin_sport || glove_blood != g_vars.skins.glove_skin_blood || glovemodel != g_vars.skins.glove_model || knife != g_vars.skins.knife_model)
////	{
////		if (g_interfaces.engine->is_in_game() && g_interfaces.engine->is_in_game() && g_client.local && g_client.local->is_alive() && g_client.local->get_active_weapon())
////		{
////			force_update = true;
////		}
////
////		glovemodel = g_vars.skins.glove_model;
////		glove_blood = g_vars.skins.glove_skin_blood;
////		glove_sport = g_vars.skins.glove_skin_sport;
////		glove_driver = g_vars.skins.glove_skin_driver;
////		glove_handwar = g_vars.skins.glove_skin_handwar;
////		glove_motor = g_vars.skins.glove_skin_motor;
////		glove_specialist = g_vars.skins.glove_skin_specialist;
////		glove_hydra = g_vars.skins.glove_skin_hydra;
////		glove_fang = g_vars.skins.glove_skin_fang;
////		knife = g_vars.skins.knife_model;
////	}
////
////	auto& selected_entry = options.items[weapon_names_full[weapon_vector_index_skins].definition_index];
////
////	static int paint_kint = 0;
////	static bool statrak_bool = false;
////	static int statrak = 0;
////	static int seed = 0;
////	static float wear = 0.0;
////	static float glove_wear = 0.0;
////
////	if (paint_kint != selected_entry.paint_kit_index || seed != selected_entry.seed || wear != selected_entry.wear || glove_wear != g_vars.skins.glove_wear || statrak != selected_entry.stat_trak || statrak_bool != selected_entry.enable_stat_track)
////	{
////		if (g_interfaces.engine->is_in_game() && g_client.local && g_client.local->is_alive() && g_client.local->get_active_weapon() && g_menu.opened)
////		{
////			force_update = true;
////		}
////		paint_kint = selected_entry.paint_kit_index;
////		seed = selected_entry.seed;
////		statrak_bool = selected_entry.enable_stat_track;
////		statrak = selected_entry.stat_trak;
////		wear = selected_entry.wear;
////		wear = selected_entry.wear;
////		glove_wear = g_vars.skins.glove_wear;
////	}
////}
////
////void c_skins::agent_changer()
////{
////	if (!g_client.local || !g_interfaces.engine->is_in_game())
////		return;
////
////	static auto backup_model_index = -1;
////	//if (g_vars.skins.enable_player)
////	{
////		const char** player_model_index = nullptr;
////		auto player_model = 0;
////
////		switch (g_client.local->get_team())
////		{
////		case 2:
////			player_model_index = player_model_index_t;
////			player_model = g_vars.skins.player_models_t;
////			break;
////		case 3:
////			player_model_index = player_model_index_ct;
////			player_model = g_vars.skins.player_models_ct;
////			break;
////		}
////
////		if (player_model)
////		{
////			if (!backup_model)
////			{
////				auto model = g_client.local->get_model();
////
////				if (model)
////				{
////					auto studio_model = g_interfaces.modelinfo->get_studio_model(model);
////
////					if (studio_model)
////					{
////						auto name = XOR("models/") + (std::string)studio_model->name;
////						backup_model_index = g_interfaces.modelinfo->get_model_index(name.c_str());
////					}
////				}
////			}
////
////			if (player_model_indexes.find(player_model_index[player_model - 1]) == player_model_indexes.end())
////				player_model_indexes.emplace(player_model_index[player_model - 1], g_interfaces.modelinfo->get_model_index(player_model_index[player_model - 1]));
////
////			g_client.local->set_model_index(player_model_indexes[player_model_index[player_model - 1]]);
////			backup_model = true;
////		}
////		else if (backup_model)
////		{
////			g_client.local->set_model_index(backup_model_index);
////			backup_model = false;
////		}
////	}
////}