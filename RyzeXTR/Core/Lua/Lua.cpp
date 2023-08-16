#include "Lua.h"
#include "../Interface/interfaces.h"

#include "../SDK/Entity.h"
#include "../SDK/NetVar/Netvar.h"

#include "../Features/Misc/Playerlist.h"
#include "../Features/Visuals/drawlist.h"
#include "../Features/Rage/aimbot.h"

#include "../../Dependecies/ImGui/imgui_internal.h"

namespace ExloError
{
	template <typename T>
	inline void ParseError( T* Value, std::string szReason = XorStr("") )
	{
		if (Value == nullptr)
		{
			LuaImplementation::PrintError( std::vformat(XorStr( "Data was null when trying to get: {}" ), std::make_format_args( szReason ) ) );
			return;
		}
		
		LuaImplementation::PrintError( XorStr( "Unknown error" ) );
	}

};

void LuaPanic( sol::optional< std::string > message )
{
	if (!message)
		return;

	LuaImplementation::PrintError( message.value_or( XorStr( "unknown" ) ) );
}

// lua_State helpers
namespace helpers {
	// package.path: used when doing "require("something.lua")"
	std::string szLibrariesPath = "";

	// https://www.lua.org/manual/5.1/manual.html#3.8

	// btw, both work
#if 0
	int GetCurrentLine( lua_State* L, int level = 1 ) {
		lua_Debug ar;
		lua_getstack( L, level, &ar );
		lua_getinfo( L, XorStr( "l" ), &ar );
		return ar.currentline;
	}

	std::string GetCurrentLuaFilename( lua_State* L, int level = 1 ) {
		lua_Debug ar;
		lua_getstack( L, level, &ar );
		lua_getinfo( L, XorStr( "S" ), &ar );
		std::string source = ar.source;
		std::string filename = std::filesystem::path( source.substr( 1 ) ).filename( ).string( );
		return filename;
	}
#else
	int GetCurrentLine( sol::this_state L )
	{
		sol::state_view lua_state( L );
		sol::table rs = lua_state[ XorStr( "debug" ) ][ XorStr( "getinfo" ) ]( 2, XorStr( "l" ) );
		int currentline = rs[ XorStr( "currentline" ) ];

		return currentline;
	}

	std::string GetCurrentLuaFilename( sol::this_state L )
	{
		sol::state_view lua_state( L );
		sol::table rs = lua_state[ XorStr( "debug" ) ][ XorStr( "getinfo" ) ]( 2, XorStr( "S" ) );
		std::string source = rs[ XorStr( "source" ) ];
		std::string filename = std::filesystem::path( source.substr( 1 ) ).filename( ).string( );

		return filename;
	}
#endif
}

namespace LUAClasses 
{
	class LuaPlayerInfo 
	{
	public:
		LuaPlayerInfo( ) 
		{
			this->steamId64 = ( int64_t )0;
			this->name = "";
			this->userid = 0;
			this->szSteamId = "";
			this->bot = false;
		}

		LuaPlayerInfo( PlayerInfo_t pl_info ) 
		{
			this->steamId64 = pl_info.ullXuid;
			this->name = pl_info.szName;
			this->userid = pl_info.nUserID;
			this->szSteamId = pl_info.szSteamID;
			this->bot = pl_info.bFakePlayer;
		}

		int64_t steamId64;
		std::string name;
		int userid;
		std::string szSteamId;
		bool bot;

		sol::object _lua_get( sol::stack_object key, sol::this_state L ) 
		{
			// we use stack_object for the arguments because we
			// know the values from Lua will remain on Lua's stack,
			// so long we we don't mess with it
			auto maybe_string_key
				= key.as< sol::optional< std::string > >( );
			if (maybe_string_key) {
				const std::string& k = *maybe_string_key;
				if (k == XorStr( "steamId64" )) { return sol::object( L, sol::in_place, this->steamId64 ); }
				else if (k == XorStr( "name" )) { return sol::object( L, sol::in_place, this->name ); }
				else if (k == XorStr( "userid" )) { return sol::object( L, sol::in_place, this->userid ); }
				else if (k == XorStr( "szSteamId" )) { return sol::object( L, sol::in_place, this->szSteamId ); }
				else if (k == XorStr( "bot" )) { return sol::object( L, sol::in_place, this->bot ); }
			}

			// No valid key: push nil
			return sol::object( L, sol::in_place, sol::lua_nil );
		}
	};

	class LuaPlayer;

	class LuaEntity : public CBaseEntity
	{
	public:

		Vector GetEyePosition(bool bShouldCorrent = false)
		{
			return this->GetEyePosition(bShouldCorrent);
		}

		LuaPlayer* ToPlayer( ) 
		{
			return ( LuaPlayer* )this;
		}
			
		uintptr_t GetAddress( ) 
		{
			return ( uintptr_t )this;
		}

		int GetIndex( ) 
		{
			return this->GetIndex( );
		}

		bool IsDormant( ) 
		{
			return this->IsDormant( );
		}

		int GetPropInt( int offset ) 
		{
			return this->get< int >( offset );
		}

		float GetPropFloat( int offset ) 
		{
			return this->get< float >( offset );
		}

		short GetPropShort( int offset ) 
		{
			return this->get< short >( offset );
		}

		double GetPropDouble( int offset ) 
		{
			return this->get< double >( offset );
		}

		bool GetPropBool( int offset ) 
		{
			return this->get< bool >( offset );
		}

		Vector GetPropVector( int offset ) 
		{
			return this->get< Vector >( offset );
		}

		void SetPropInt( int offset, int value ) 
		{
			this->set< int >( offset, value );
		}

		void SetPropFloat( int offset, float value ) 
		{
			this->set< float >( offset, value );
		}

		void SetPropShort( int offset, short value ) 
		{
			this->set< short >( offset, value );
		}

		void SetPropDouble( int offset, double value ) 
		{
			this->set< double >( offset, value );
		}

		void SetPropBool( int offset, bool value ) 
		{
			this->set< bool >( offset, value );
		}

		void SetPropVector( int offset, Vector value ) 
		{
			this->set< Vector >( offset, value );
		}
	};

	class LuaPlayer : public LuaEntity 
	{
	public:
		bool IsEnemyOf( LuaPlayer* player ) 
		{
			if (!player)
				return false;

			return this->IsEnemy( player );
		}

		Vector GetAbsOrigin( ) 
		{
			// had to cast..
			return this->GetAbsOrigin( );
		}

		Vector GetAbsAngles( ) 
		{
			// had to cast..
			return this->GetAbsAngles( );
		}
	};

	class LuaPlayerList_PlayerSettings 
	{
		playerSettings_t* pSettings;
	public:
		LuaPlayerList_PlayerSettings( ) : pSettings{ nullptr } { }
		LuaPlayerList_PlayerSettings( playerSettings_t* pSettings ) : pSettings{ pSettings } { }

		// Ragebot whitelist
		bool IsRagebotWhitelisted( ) {
			if (!pSettings)
				return false;

			return pSettings->bWhiteList;
		}

		void SetRagebotWhitelist( bool value ) {
			if (!pSettings)
				return;

			pSettings->bWhiteList = value;
		}

		bool IsOverridingResolver( ) {
			if (!pSettings)
				return false;

			return pSettings->bOverrideResolver;
		}

		void ToggleResolverOverride( bool value ) {
			if (!pSettings)
				return;

			pSettings->bOverrideResolver = value;
		}

		void SetCustomResolveYaw( float value ) {
			if (!pSettings)
				return;

			pSettings->flOverrideYaw = value;
		}
	};

	class RageBot_CachedData
	{
		rageBotData_t* pData = &aimbot.GetHitLogData( );
	public:
		RageBot_CachedData( ) : pData{ nullptr } { }
		RageBot_CachedData( rageBotData_t* pData ) : pData{ pData } { }

		LuaPlayer* GetTarget( ) {
			if (!pData || pData->pAimbotTarget)
			{
				ExloError::ParseError( pData, XorStr( "GetTarget" ) );
				return nullptr;
			}
			
			return reinterpret_cast<LuaPlayer*>( pData->pAimbotTarget );
		}

		/*matrix3x4_t* GetMatrix( )
		{
			if ( !pData || !pData->pTargetMatrix )
				return nullptr;

			return pData->pTargetMatrix;
		}*/

		Vector GetLocalShootPosition( )
		{
			if (!pData || pData->vecLocalShootPosition.IsZero( ))
			{
				ExloError::ParseError( pData, XorStr( "GetLocalShootPosition" ) );
				return Vector( 0, 0, 0 );
			}
				

			return pData->vecLocalShootPosition;
		}

		int GetHitbox( )
		{
			if (!pData || pData->iHitbox <= 0)
			{
				ExloError::ParseError( pData, XorStr( "GetHitbox" ) );
				return 0;
			}

			return pData->iHitbox;
		}

		int GetBacktrack( )
		{
			if (!pData || pData->iTickcount <= 0 || pData->flTargetSimulation <= 0.0f)
			{
				ExloError::ParseError( pData, XorStr( "GetBacktrack" ) );
				return 0;
			}

			return ( pData->iTickcount - TIME_TO_TICKS( pData->pRecord->flSimulationTime ) );
		}

		float GetDamage( )
		{
			if (!pData || pData->flDamage <= 0.0f)
			{
				ExloError::ParseError( pData, XorStr( "GetDamage" ) );
				return 0.0f;
			}

			return pData->flDamage;
		}

		float GetHitChance( )
		{
			if (!pData || pData->flHitchance <= 0.0f)
			{
				ExloError::ParseError( pData, XorStr( "GetHitChance" ) );
				return 0.0f;
			}

			return pData->flHitchance;
		}

		float GetTargetSimulationTime( )
		{
			if (!pData || pData->flTargetSimulation <= 0.0f)
			{
				ExloError::ParseError( pData, XorStr( "GetTargetSimulationTime" ) );
				return 0.0f;
			}

			return pData->flTargetSimulation;
		}

		bool CanShoot( )
		{
			if (!pData)
			{
				ExloError::ParseError( pData, XorStr( "CanShoot" ) );
				return false;
			}

			return pData->bCanShoot;
		}
	};
}

std::vector< std::string > vecGameEventList =
{
	XorStr( "player_death" ),
	XorStr( "other_death" ),
	XorStr( "player_hurt" ),
	XorStr( "item_purchase" ),
	XorStr( "bomb_beginplant" ),
	XorStr( "bomb_abortplant" ),
	XorStr( "bomb_planted" ),
	XorStr( "bomb_defused" ),
	XorStr( "bomb_exploded" ),
	XorStr( "bomb_dropped" ),
	XorStr( "bomb_pickup" ),
	XorStr( "defuser_dropped" ),
	XorStr( "defuser_pickup" ),
	XorStr( "announce_phase_end" ),
	XorStr( "cs_intermission" ),
	XorStr( "bomb_begindefuse" ),
	XorStr( "bomb_abortdefuse" ),
	XorStr( "hostage_follows" ),
	XorStr( "hostage_hurt" ),
	XorStr( "hostage_killed" ),
	XorStr( "hostage_rescued" ),
	XorStr( "hostage_stops_following" ),
	XorStr( "hostage_rescued_all" ),
	XorStr( "hostage_call_for_help" ),
	XorStr( "vip_escaped" ),
	XorStr( "vip_killed" ),
	XorStr( "player_radio" ),
	XorStr( "bomb_beep" ),
	XorStr( "weapon_fire" ),
	XorStr( "weapon_fire_on_empty" ),
	XorStr( "grenade_thrown" ),
	XorStr( "weapon_outofammo" ),
	XorStr( "weapon_reload" ),
	XorStr( "weapon_zoom" ),
	XorStr( "silencer_detach" ),
	XorStr( "inspect_weapon" ),
	XorStr( "weapon_zoom_rifle" ),
	XorStr( "player_spawned" ),
	XorStr( "item_pickup" ),
	XorStr( "item_pickup_failed" ),
	XorStr( "item_remove" ),
	XorStr( "ammo_pickup" ),
	XorStr( "item_equip" ),
	XorStr( "enter_buyzone" ),
	XorStr( "exit_buyzone" ),
	XorStr( "buytime_ended" ),
	XorStr( "enter_bombzone" ),
	XorStr( "exit_bombzone" ),
	XorStr( "enter_rescue_zone" ),
	XorStr( "exit_rescue_zone" ),
	XorStr( "silencer_off" ),
	XorStr( "silencer_on" ),
	XorStr( "buymenu_open" ),
	XorStr( "buymenu_close" ),
	XorStr( "round_announce_warmup" ),
	XorStr( "round_announce_match_start" ),
	XorStr( "round_prestart" ),
	XorStr( "round_poststart" ),
	XorStr( "round_start" ),
	XorStr( "round_end" ),
	XorStr( "grenade_bounce" ),
	XorStr( "hegrenade_detonate" ),
	XorStr( "flashbang_detonate" ),
	XorStr( "smokegrenade_detonate" ),
	XorStr( "smokegrenade_expired" ),
	XorStr( "molotov_detonate" ),
	XorStr( "decoy_detonate" ),
	XorStr( "decoy_started" ),
	XorStr( "tagrenade_detonate" ),
	XorStr( "inferno_startburn" ),
	XorStr( "inferno_expire" ),
	XorStr( "inferno_extinguish" ),
	XorStr( "decoy_firing" ),
	XorStr( "bullet_impact" ),
	XorStr( "player_footstep" ),
	XorStr( "player_jump" ),
	XorStr( "player_blind" ),
	XorStr( "player_falldamage" ),
	XorStr( "door_moving" ),
	XorStr( "round_freeze_end" ),
	XorStr( "mb_input_lock_success" ),
	XorStr( "mb_input_lock_cancel" ),
	XorStr( "nav_blocked" ),
	XorStr( "nav_generate" ),
	XorStr( "player_stats_updated" ),
	XorStr( "achievement_info_loaded" ),
	XorStr( "spec_target_updated" ),
	XorStr( "spec_mode_updated" ),
	XorStr( "hltv_changed_mode" ),
	XorStr( "cs_game_disconnected" ),
	XorStr( "cs_win_panel_round" ),
	XorStr( "cs_win_panel_match" ),
	XorStr( "cs_match_end_restart" ),
	XorStr( "cs_pre_restart" ),
	XorStr( "show_freezepanel" ),
	XorStr( "hide_freezepanel" ),
	XorStr( "freezecam_started" ),
	XorStr( "player_avenged_teammate" ),
	XorStr( "achievement_earned" ),
	XorStr( "achievement_earned_local" ),
	XorStr( "item_found" ),
	XorStr( "items_gifted" ),
	XorStr( "repost_xbox_achievements" ),
	XorStr( "match_end_conditions" ),
	XorStr( "round_mvp" ),
	XorStr( "player_decal" ),
	XorStr( "teamplay_round_start" ),
	XorStr( "client_disconnect" ),
	XorStr( "gg_player_levelup" ),
	XorStr( "ggtr_player_levelup" ),
	XorStr( "assassination_target_killed" ),
	XorStr( "ggprogressive_player_levelup" ),
	XorStr( "gg_killed_enemy" ),
	XorStr( "gg_final_weapon_achieved" ),
	XorStr( "gg_bonus_grenade_achieved" ),
	XorStr( "switch_team" ),
	XorStr( "gg_leader" ),
	XorStr( "gg_team_leader" ),
	XorStr( "gg_player_impending_upgrade" ),
	XorStr( "write_profile_data" ),
	XorStr( "trial_time_expired" ),
	XorStr( "update_matchmaking_stats" ),
	XorStr( "player_reset_vote" ),
	XorStr( "enable_restart_voting" ),
	XorStr( "sfuievent" ),
	XorStr( "start_vote" ),
	XorStr( "player_given_c4" ),
	XorStr( "player_become_ghost" ),
	XorStr( "gg_reset_round_start_sounds" ),
	XorStr( "tr_player_flashbanged" ),
	XorStr( "tr_highlight_ammo" ),
	XorStr( "tr_mark_complete" ),
	XorStr( "tr_mark_best_time" ),
	XorStr( "tr_exit_hint_trigger" ),
	XorStr( "bot_takeover" ),
	XorStr( "tr_show_finish_msgbox" ),
	XorStr( "tr_show_exit_msgbox" ),
	XorStr( "reset_player_controls" ),
	XorStr( "jointeam_failed" ),
	XorStr( "teamchange_pending" ),
	XorStr( "material_default_complete" ),
	XorStr( "cs_prev_next_spectator" ),
	XorStr( "cs_handle_ime_event" ),
	XorStr( "nextlevel_changed" ),
	XorStr( "seasoncoin_levelup" ),
	XorStr( "tournament_reward" ),
	XorStr( "start_halftime" ),
	XorStr( "ammo_refill" ),
	XorStr( "parachute_pickup" ),
	XorStr( "parachute_deploy" ),
	XorStr( "dronegun_attack" ),
	XorStr( "drone_dispatched" ),
	XorStr( "loot_crate_visible" ),
	XorStr( "loot_crate_opened" ),
	XorStr( "open_crate_instr" ),
	XorStr( "smoke_beacon_paradrop" ),
	XorStr( "drone_cargo_detached" ),
	XorStr( "choppers_incoming_warning" ),
	XorStr( "firstbombs_incoming_warning" ),
	XorStr( "dz_item_interaction" ),
	XorStr( "snowball_hit_player_face" ),

	// somehow isn't on the alliedmods' wiki
	XorStr( "player_say" )
};


// Modules
namespace LUAModules 
{
	namespace CallBacks 
	{
		void Add( std::string szName, sol::function pFunction, sol::this_state L ) {
			std::string current_script_filename = helpers::GetCurrentLuaFilename( L );
			int current_line = helpers::GetCurrentLine( L );

			auto script = LuaImplementation::FindScriptByName( current_script_filename );
			if (!script) {

				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[Callbacks.Add] Failed to find script " ) + current_script_filename );
				return;
			}

			// not valid callback
			if (std::find( LuaImplementation::vecCallbackList.begin( ), LuaImplementation::vecCallbackList.end( ), szName ) == LuaImplementation::vecCallbackList.end( )
				&& std::find( vecGameEventList.begin( ), vecGameEventList.end( ), szName ) == vecGameEventList.end( )) {

				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[Callbacks.Add] " ) + szName + XorStr( " is not a valid callback ( line: " ) + std::to_string( current_line ) + XorStr( " in " ) + current_script_filename + XorStr( " )" ) );

				script->Unload( );
				return;
			}

			script->AddCallback( szName, pFunction );
		}
	}

	namespace GlobalVars {
		float GetRealtime( )
		{
			return i::GlobalVars->flRealTime;
		}

		int GetFrameCount( )
		{
			return i::GlobalVars->iFrameCount;
		}

		float GetAbsFrametime( )
		{
			return i::GlobalVars->flAbsFrameTime;
		}

		float GetCurtime( )
		{
			return i::GlobalVars->flCurrentTime;
		}

		float GetFrametime( )
		{
			return i::GlobalVars->flFrameTime;
		}

		int GetTickcount( )
		{
			return i::GlobalVars->iTickCount;
		}

		float GetIntervalPerTick( )
		{
			return i::GlobalVars->flIntervalPerTick;
		}

		float GetInterpolationAmount( )
		{
			return i::GlobalVars->flInterpolationAmount;
		}
	}

	namespace ClientState
	{
		int GetLastOutgoingCommand( )
		{
			return i::ClientState->iLastOutgoingCommand;
		}

		int GetChokedCommands( )
		{
			return i::ClientState->nChokedCommands;
		}

		int GetLastCommandAck( )
		{
			return i::ClientState->iLastCommandAck;
		}

		int GetCommandAck( )
		{
			return i::ClientState->iCommandAck;
		}
	}

	namespace Engine 
	{
		Vector GetViewAngles( ) 
		{
			Vector ret;
			i::EngineClient->GetViewAngles( ret );
			return ret;
		}

		void SetViewAngles( Vector vecAngles )
		{
			i::EngineClient->SetViewAngles( vecAngles );
		}

		void ExecuteClientCmd( std::string szCommand ) 
		{
			i::EngineClient->ExecuteClientCmd( szCommand.c_str( ) );
		}

		INetChannelInfo* GetNetChannelInfo( ) 
		{
			return i::EngineClient->GetNetChannelInfo( );
		}

		LUAClasses::LuaPlayerInfo GetPlayerInfo( int iIndex ) 
		{
			PlayerInfo_t ret;
			i::EngineClient->GetPlayerInfo( iIndex, &ret );
			return LUAClasses::LuaPlayerInfo( ret );
		}

		int GetLocalPlayer( ) 
		{
			return i::EngineClient->GetLocalPlayer( );
		}

		int GetPlayerForUserID( int uid ) 
		{
			return i::EngineClient->GetPlayerForUserID( uid );
		}

		bool IsInGame( ) 
		{
			return i::EngineClient->IsInGame( );
		}

		bool IsConnected( ) 
		{
			return i::EngineClient->IsConnected( );
		}

		bool IsVoiceRecording( ) 
		{
			return i::EngineClient->IsVoiceRecording( );
		}

		Vector2D GetScreenSize( )
		{
			int x, y = 0;
			i::EngineClient->GetScreenSize( x, y );			
			return Vector2D( x, y );
		}
	}

	namespace Render 
	{
		//LuaImplementation::ScriptFont_t* CreateFont_( std::string szFontName, int height, int weight, sol::optional< int > flags, sol::this_state L ) {
		//	std::string current_script_filename = helpers::GetCurrentLuaFilename( L );
		//	//int current_line = helpers::GetCurrentLine( L );

		//	auto script = LuaImplementation::FindScriptByName( current_script_filename );
		//	if (!script) {

		//		// formatting could be better here
		//		LuaImplementation::PrintError( XorStr( "[CreateFont] Failed to find script " ) + current_script_filename );
		//		return nullptr;
		//	}

		//	LuaImplementation::ScriptFont_t& font = script->vecFonts.emplace_back( );
		//	font.m_szWindowsFontName = szFontName;
		//	font.m_iHeight = height;
		//	font.m_iWeight = weight;
		//	font.m_Flags = flags.value_or( 0 );
		//	font.Create( );

		//	return &font;
		//}

		void DrawFilledRect( Vector2D vecPos, Vector2D vecSize, Color col ) 
		{	
			drawlist::AddRect( vecPos, vecSize, DRAWFLAGS::DRAWFLAGS_FILLED, col );
		}

		void DrawOutlinedRect( Vector2D vecPos, Vector2D vecSize, Color col ) 
		{
			drawlist::AddRect( vecPos, vecSize, DRAWFLAGS::DRAWFLAGS_OUTLINE, col );
		}

		//void DrawGradientRect( vec2_t vecPos, vec2_t vecSize, color cCol1, color cCol2, sol::optional< bool > horizontal ) {
		//	bool bHorizontal = horizontal.value_or( false );
		//	g_SurfaceRenderer.DrawGradientRect( renderer::rect_t( vecPos.x, vecPos.y, vecSize.x, vecSize.y ), cCol1, cCol2, bHorizontal );
		//}

		void DrawLine( Vector2D vecPos1, Vector2D vecPos2, Color col ) 
		{
			drawlist::AddLine( vecPos1, vecPos2, col );
		}

		void DrawString( Vector2D vecPosition, bool bCenter, Color col, std::string szText )
		{
			drawlist::AddText( vecPosition, g::fonts::SkeetFont, bCenter, col, szText );
		}

		//void DrawFilledTriangle( vec2_t vecPos1, vec2_t vecPos2, vec2_t vecPos3, color sColor ) {
		//	g_SurfaceRenderer.DrawFilledTriangle(
		//		vecPos1.x, vecPos1.y,
		//		vecPos2.x, vecPos2.y,
		//		vecPos3.x, vecPos3.y,
		//		sColor );
		//}

		//void DrawOutlinedTriangle( vec2_t vecPos1, vec2_t vecPos2, vec2_t vecPos3, color sColor ) {
		//	g_SurfaceRenderer.DrawOutlinedTriangle(
		//		vecPos1.x, vecPos1.y,
		//		vecPos2.x, vecPos2.y,
		//		vecPos3.x, vecPos3.y,
		//		sColor );
		//}

		//void DrawFilledCircle( vec2_t vecPos, float flRadius, color sColor ) {
		//	g_SurfaceRenderer.DrawFilledCircle(
		//		vecPos.x, vecPos.y,
		//		flRadius,
		//		sColor
		//	);
		//}

		//void DrawOutlinedCircle( vec2_t vecPos, float flRadius, color sColor ) {
		//	g_SurfaceRenderer.DrawOutlinedCircle(
		//		vecPos.x, vecPos.y,
		//		flRadius,
		//		sColor
		//	);
		//}

		//void DrawCircle3D( vec3_t vecPos, float flRadius, color sColor, sol::optional< bool > outline ) {
		//	g_SurfaceRenderer.DrawWorldCircle(
		//		vecPos.x,
		//		vecPos.y,
		//		vecPos.z,
		//		flRadius,
		//		sColor,
		//		outline.value_or( false )
		//	);
		//}
	}

	namespace Print
	{
		void PrintToConsole( std::string szText )
		{
			misc::Print( szText );
		}
	}

	namespace NetVars 
	{
		int FindOffset( std::string szTableProp ) 
		{
			int nOffset = n::netvars[ fnv::HashConst( szTableProp.c_str( ) ) ].uOffset;
			if (nOffset == 0)
			{
				ExloError::ParseError( &nOffset, XorStr( "FindOffset [ " ) + szTableProp + XorStr( " ]" ) );
				return 0;
			}
			
			return nOffset;
		}
	}

	namespace EntityList 
	{
		LUAClasses::LuaEntity* GetClientEntity( int iIndex )
		{
			return reinterpret_cast< LUAClasses::LuaEntity* >( i::EntityList->GetClientEntity( iIndex ) );
		}

		LUAClasses::LuaEntity* GetClientEntityFromHandle( unsigned long lHandle ) 
		{
			return reinterpret_cast< LUAClasses::LuaEntity* >( i::EntityList->GetClientEntityFromHandle( lHandle ) );
		}

		int GetHighestEntityIndex( ) 
		{
			return i::EntityList->GetHighestEntityIndex( );
		}
	}

	namespace Menu 
	{
		bool IsOpened( ) 
		{
			return menu::open;
		}

		/*Vector2D GetMenuPosition( ) 
		{
			return Vector2D( menu::m_vecLastKnownMenuPosition.x, menu::m_vecLastKnownMenuPosition.y );
		}*/

		LuaImplementation::CheckboxMenuItem_t* AddCheckbox( std::string szLabel, std::string szVarName, sol::optional< bool > bDefaultValue, sol::this_state L )
		{
			std::string szCurrentScriptName = helpers::GetCurrentLuaFilename( L );
			int current_line = helpers::GetCurrentLine( L );

			auto script = LuaImplementation::FindScriptByName( szCurrentScriptName );
			if (!script) {

				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddCheckbox] Failed to find script " ) + szCurrentScriptName );
				return nullptr;
			}

			// make sure variable doesn't exist yet
			if (script->FindMenuItem( szVarName ) != nullptr) {
				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddCheckbox] Variable already exists (line: " ) + std::to_string( current_line ) + XorStr( " in " ) + szCurrentScriptName + XorStr( ")" ) );
				script->Unload( );

				return nullptr;
			}

			// add menu item and get reference to it
			LuaImplementation::CheckboxMenuItem_t& pCheckbox = reinterpret_cast< LuaImplementation::CheckboxMenuItem_t& >( script->vecMenuItems.emplace_back( ) );
			pCheckbox.szPathToScript = script->szPath;
			pCheckbox.iType = LuaImplementation::MENUITEM_CHECKBOX;
			pCheckbox.bVisible = true;
			pCheckbox.szLabel = szLabel;
			pCheckbox.szVarName = szVarName;
			pCheckbox.bValue = bDefaultValue.value_or( false );

			return &pCheckbox;
		}

		LuaImplementation::SliderIntMenuItem_t* AddSliderInt( std::string szLabel, std::string szVarName, int iMinValue, int iMaxValue, sol::optional< int > iDefaultValue, sol::optional< std::string > szFormat, sol::this_state L )
		{
			std::string szCurrentScriptName = helpers::GetCurrentLuaFilename( L );
			int current_line = helpers::GetCurrentLine( L );

			auto script = LuaImplementation::FindScriptByName( szCurrentScriptName );
			if (!script) {

				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddSliderInt] Failed to find script " ) + szCurrentScriptName );
				return nullptr;
			}

			// make sure variable doesn't exist yet
			if (script->FindMenuItem( szVarName ) != nullptr) {
				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddSliderInt] Variable already exists (line: " ) + std::to_string( current_line ) + XorStr( " in " ) + szCurrentScriptName + XorStr( ")" ) );
				script->Unload( );

				return nullptr;
			}

			int iMin = iMinValue;
			int iMax = iMaxValue;

			// opposite
			if (iMin > iMax) {
				ImSwap( iMin, iMax );
			}

			// add menu item and get reference to it
			LuaImplementation::SliderIntMenuItem_t& pSlider = reinterpret_cast< LuaImplementation::SliderIntMenuItem_t& >( script->vecMenuItems.emplace_back( ) );
			pSlider.szPathToScript = script->szPath;
			pSlider.iType = LuaImplementation::MENUITEM_SLIDERINT;
			pSlider.bVisible = true;
			pSlider.szLabel = szLabel;
			pSlider.szVarName = szVarName;
			pSlider.iValue = iDefaultValue.value_or( iMin );
			pSlider.iMinValue = iMin;
			pSlider.iMaxValue = iMax;
			pSlider.szFormat = szFormat.value_or( XorStr( "%d" ) );

			return &pSlider;
		}

		LuaImplementation::SliderFloatMenuItem_t* AddSliderFloat( std::string szLabel, std::string szVarName, float flMinValue, float flMaxValue, sol::optional< float > flDefaultValue, sol::optional< std::string > szFormat, sol::this_state L )
		{
			std::string szCurrentScriptName = helpers::GetCurrentLuaFilename( L );
			int current_line = helpers::GetCurrentLine( L );

			auto script = LuaImplementation::FindScriptByName( szCurrentScriptName );
			if (!script) {

				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddSliderFloat] Failed to find script " ) + szCurrentScriptName );
				return nullptr;
			}

			// make sure variable doesn't exist yet
			if (script->FindMenuItem( szVarName ) != nullptr) {
				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddSliderFloat] Variable already exists (line: " ) + std::to_string( current_line ) + XorStr( " in " ) + szCurrentScriptName + XorStr( ")" ) );
				script->Unload( );

				return nullptr;
			}

			float flMin = flMinValue;
			float flMax = flMaxValue;

			// opposite
			if (flMin > flMax) {
				ImSwap( flMin, flMax );
			}

			// add menu item and get reference to it
			LuaImplementation::SliderFloatMenuItem_t& pSlider = reinterpret_cast< LuaImplementation::SliderFloatMenuItem_t& >( script->vecMenuItems.emplace_back( ) );
			pSlider.szPathToScript = script->szPath;
			pSlider.iType = LuaImplementation::MENUITEM_SLIDERFLOAT;
			pSlider.bVisible = true;
			pSlider.szLabel = szLabel;
			pSlider.szVarName = szVarName;
			pSlider.flValue = flDefaultValue.value_or( flMin );
			pSlider.flMinValue = flMin;
			pSlider.flMaxValue = flMax;
			pSlider.szFormat = szFormat.value_or( XorStr( "%.2f" ) );

			return &pSlider;
		}

		LuaImplementation::KeybindMenuItem_t* AddKeybind( std::string szLabel, std::string szVarName, sol::optional< int > iDefaultKey, sol::optional< int > iDefaultKeyMode, sol::this_state L )
		{
			std::string szCurrentScriptName = helpers::GetCurrentLuaFilename( L );
			int current_line = helpers::GetCurrentLine( L );

			auto script = LuaImplementation::FindScriptByName( szCurrentScriptName );
			if (!script) {

				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddKeybind] Failed to find script " ) + szCurrentScriptName );
				return nullptr;
			}

			// make sure variable doesn't exist yet
			if (script->FindMenuItem( szVarName ) != nullptr) {
				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddKeybind] Variable already exists (line: " ) + std::to_string( current_line ) + XorStr( " in " ) + szCurrentScriptName + XorStr( ")" ) );
				script->Unload( );

				return nullptr;
			}

			// add menu item and get reference to it
			LuaImplementation::KeybindMenuItem_t& pKeybind = reinterpret_cast< LuaImplementation::KeybindMenuItem_t& >( script->vecMenuItems.emplace_back( ) );
			pKeybind.szPathToScript = script->szPath;
			pKeybind.iType = LuaImplementation::MENUITEM_KEYBIND;
			pKeybind.bVisible = true;
			pKeybind.szLabel = szLabel;
			pKeybind.szVarName = szVarName;
			pKeybind.iValue = iDefaultKey.value_or( 0 );
			pKeybind.iKeyMode = iDefaultKeyMode.value_or( 0 );

			return &pKeybind;
		}

		LuaImplementation::ComboMenuItem_t* AddCombo( std::string szLabel, std::string szVarName, std::vector< std::string > vecItems, sol::optional< int > iDefaultValue, sol::this_state L )
		{
			std::string szCurrentScriptName = helpers::GetCurrentLuaFilename( L );
			int current_line = helpers::GetCurrentLine( L );

			auto script = LuaImplementation::FindScriptByName( szCurrentScriptName );
			if (!script) {

				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddCombo] Failed to find script " ) + szCurrentScriptName );
				return nullptr;
			}

			if (vecItems.empty( ) || ( iDefaultValue.has_value( ) && ( iDefaultValue.value( ) < 0 || iDefaultValue.value( ) >= vecItems.size( ) ) ))
			{
				LuaImplementation::PrintError( XorStr( "[AddCombo] Invalid arguments (line: " ) + std::to_string( current_line ) + XorStr( " in " ) + szCurrentScriptName + XorStr( ")" ) );
				script->Unload( );
				return nullptr;
			}

			// make sure variable doesn't exist yet
			if (script->FindMenuItem( szVarName ) != nullptr) {

				LuaImplementation::PrintError( XorStr( "[AddCombo] Variable already exists (line: " ) + std::to_string( current_line ) + XorStr( " in " ) + szCurrentScriptName + XorStr( ")" ) );
				script->Unload( );
				return nullptr;
			}

			// add menu item and get reference to it
			LuaImplementation::ComboMenuItem_t& pCombo = reinterpret_cast< LuaImplementation::ComboMenuItem_t& >( script->vecMenuItems.emplace_back( ) );
			pCombo.szPathToScript = script->szPath;
			pCombo.iType = LuaImplementation::MENUITEM_COMBOBOX;
			pCombo.bVisible = true;
			pCombo.szLabel = szLabel;
			pCombo.szVarName = szVarName;
			pCombo.iValue = iDefaultValue.value_or( 0 );
			pCombo.vecComboboxLabels = vecItems;

			return &pCombo;
		}

		LuaImplementation::MultiComboMenuItem_t* AddMultiCombo( std::string szLabel, std::string szVarName, std::vector< std::string > vecItems, sol::optional< std::deque< bool > > vecDefaultValues, sol::this_state L )
		{
			std::string szCurrentScriptName = helpers::GetCurrentLuaFilename( L );
			int current_line = helpers::GetCurrentLine( L );

			auto script = LuaImplementation::FindScriptByName( szCurrentScriptName );
			if (!script) {

				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddMultiCombo] Failed to find script " ) + szCurrentScriptName );
				return nullptr;
			}

			if (vecItems.empty( ) || ( vecDefaultValues.has_value( ) && vecDefaultValues.value( ).size( ) != vecItems.size( ) ))
			{
				LuaImplementation::PrintError( XorStr( "[AddMultiCombo] Invalid arguments (line: " ) + std::to_string( current_line ) + XorStr( " in " ) + szCurrentScriptName + XorStr( ")" ) );
				script->Unload( );
				return nullptr;
			}

			// make sure variable doesn't exist yet
			if (script->FindMenuItem( szVarName ) != nullptr) {

				LuaImplementation::PrintError( XorStr( "[AddMultiCombo] Variable already exists (line: " ) + std::to_string( current_line ) + XorStr( " in " ) + szCurrentScriptName + XorStr( ")" ) );
				script->Unload( );
				return nullptr;
			}

			// add menu item and get reference to it
			LuaImplementation::MultiComboMenuItem_t& pMultiCombo = reinterpret_cast< LuaImplementation::MultiComboMenuItem_t& >( script->vecMenuItems.emplace_back( ) );
			pMultiCombo.szPathToScript = script->szPath;
			pMultiCombo.iType = LuaImplementation::MENUITEM_MULTICOMBOBOX;
			pMultiCombo.bVisible = true;
			pMultiCombo.szLabel = szLabel;
			pMultiCombo.szVarName = szVarName;
			pMultiCombo.vecComboboxLabels = vecItems;

			for (auto& i : vecItems) {
				pMultiCombo.vecMultiComboValues.push_back( false );
			}

			if (vecDefaultValues.has_value( )) {

				// https://github.com/vinniefalco/LuaBridge/issues/163
				int i = 0;
				for (auto& val : vecDefaultValues.value( )) {
					if (i < pMultiCombo.vecMultiComboValues.size( )) {
						pMultiCombo.vecMultiComboValues[ i ] = val;
					}

					i++;
				}
			}

			return &pMultiCombo;
		}

		LuaImplementation::ColorPickerMenuItem_t* AddColorPicker( std::string szLabel, std::string szVarName, sol::optional< Color > cDefaultValue, sol::this_state L )
		{
			std::string szCurrentScriptName = helpers::GetCurrentLuaFilename( L );
			int current_line = helpers::GetCurrentLine( L );

			auto script = LuaImplementation::FindScriptByName( szCurrentScriptName );
			if (!script) {

				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddColorPicker] Failed to find script " ) + szCurrentScriptName );
				return nullptr;
			}

			// make sure variable doesn't exist yet
			if (script->FindMenuItem( szVarName ) != nullptr) {

				LuaImplementation::PrintError( XorStr( "[AddColorPicker] Variable already exists (line: " ) + std::to_string( current_line ) + XorStr( " in " ) + szCurrentScriptName + XorStr( ")" ) );
				script->Unload( );
				return nullptr;
			}

			// add menu item and get reference to it
			LuaImplementation::ColorPickerMenuItem_t& pColorPicker = reinterpret_cast< LuaImplementation::ColorPickerMenuItem_t& >( script->vecMenuItems.emplace_back( ) );
			pColorPicker.szPathToScript = script->szPath;
			pColorPicker.iType = LuaImplementation::MENUITEM_COLORPICKER;
			pColorPicker.bVisible = true;
			pColorPicker.szLabel = szLabel;
			pColorPicker.szVarName = szVarName;
			pColorPicker.cValue = cDefaultValue.value_or( Color(255, 255, 255, 255) );

			return &pColorPicker;
		}

		LuaImplementation::ButtonMenuItem_t* AddButton( std::string szLabel, sol::optional< sol::function > pFunction, sol::this_state L ) {
			std::string szCurrentScriptName = helpers::GetCurrentLuaFilename( L );
			int current_line = helpers::GetCurrentLine( L );

			auto script = LuaImplementation::FindScriptByName( szCurrentScriptName );
			if (!script) {

				// formatting could be better here
				LuaImplementation::PrintError( XorStr( "[AddButton] Failed to find script " ) + szCurrentScriptName );
				return nullptr;
			}

			std::string label = szLabel;

			// add menu item and get reference to it
			LuaImplementation::ButtonMenuItem_t& pButton = reinterpret_cast< LuaImplementation::ButtonMenuItem_t& >( script->vecMenuItems.emplace_back( ) );
			pButton.szPathToScript = script->szPath;
			pButton.iType = LuaImplementation::MENUITEM_BUTTON;
			pButton.bVisible = true;
			pButton.szLabel = label;
			pButton.szVarName = XorStr( "#button" );

			pButton.vecFunctions.clear( );
			if (pFunction.has_value( )) {
				pButton.vecFunctions.push_back( pFunction.value( ) );
			}

			return &pButton;
		}
	}

	namespace Variables 
	{
		bool& GetBool(std::string szVar, sol::this_state L)
		{
			return Config2->FindBool(szVar);
		}

		float& GetFloat(std::string szVar, sol::this_state L)
		{
			return Config2->FindFloat(szVar);
		}

		int& GetInt(std::string szVar, sol::this_state L)
		{
			return Config2->FindInt(szVar);
		}

		std::string& GetString(std::string szVar, sol::this_state L)
		{
			return Config2->FindString(szVar);
		}

		void SetBool(std::string szVar, bool bValue, sol::this_state L)
		{
			Config2->FindBool(szVar) = bValue;
		}

		void SetFloat(std::string szVar, float flValue, sol::this_state L)
		{
			Config2->FindFloat(szVar) = flValue;
		}

		void SetInt(std::string szVar, int iValue, sol::this_state L)
		{
			Config2->FindInt(szVar) = iValue;
		}

		void SetString(std::string szVar, std::string szValue, sol::this_state L)
		{
			Config2->FindString(szVar) = szValue;
		}
	}

	namespace CVars 
	{
		CConVar* FindVar( std::string szVar, sol::this_state L ) 
		{
			return i::ConVar->FindVar( szVar.c_str( ) );
		}
	}

	// add if you want
	namespace Input 
	{
		/*bool PressedKey( int key, sol::optional< bool > bOverrideInput )
		{
			return input::pressed( key, bOverrideInput.value_or( false ) );
		}

		bool HoldingKey( int key, sol::optional< bool > bOverrideInput )
		{
			return input::down( key, bOverrideInput.value_or( false ) );
		}

		bool ReleasedKey( int key, sol::optional< bool > bOverrideInput )
		{
			return input::released( key, bOverrideInput.value_or( false ) );
		}

		vec2_t GetCursorPos( )
		{
			auto cursor_pos = input::get_cursor_position( );
			return vec2_t( cursor_pos.x, cursor_pos.y );
		}*/
	}

	namespace Utilities 
	{
		class CInterfaceReg
		{
		private:
			using instantiate_interface_fn = void* ( * )( );

		public:
			instantiate_interface_fn create_fn;
			const char* name;

			CInterfaceReg* next;
		};

		CInterfaceReg* GetInterfaceList( const char* mod ) {
			DWORD interface_fn = reinterpret_cast< DWORD >( GetProcAddress( GetModuleHandleA( mod ), XorStr( "CreateInterface" ) ) ); // we need to use util::modules for this but ok
			if (!interface_fn)
				return nullptr;

			unsigned int jmp_start = ( unsigned int )( interface_fn )+4;
			unsigned int jmp_target = jmp_start + *( unsigned int* )( jmp_start + 1 ) + 5;
			CInterfaceReg* interface_reg_list = **reinterpret_cast< CInterfaceReg*** >( jmp_target + 6 );

			return interface_reg_list;
		}

		template < typename t >
		t _CreateInterface( const char* mod, const char* iname ) {
			t ret = ( t ) nullptr;
			CInterfaceReg* interface_reg_list = GetInterfaceList( mod );

			for (CInterfaceReg* cur = interface_reg_list; cur; cur = cur->next) {
				if (strcmp( cur->name, iname ) == 0) {
					ret = reinterpret_cast< t >( cur->create_fn( ) );

					//util::console::set_prefix( util::console::SDK );
					//util::console::print( "found %s at: 0x%p\n" , cur->name , ret );
					//util::console::set_prefix( util::console::NONE );
				}
			}

			return reinterpret_cast< t >( ret );
		}

		void* CreateInterface( std::string szModuleName, std::string szInterfaceVer ) 
		{
			return _CreateInterface< void* >( szModuleName.c_str( ), szInterfaceVer.c_str( ) );
		}

		void* FindSignature( std::string szModuleName, std::string szSignature ) 
		{
			return reinterpret_cast<void*>( MEM::FindPattern( std::wstring( szModuleName.begin( ), szModuleName.end( ) ).c_str(), szSignature.c_str( ) ) );
		}

		Vector2D WorldToScreen( Vector vecOrigin, sol::this_state L ) 
		{
			Vector2D ret;
			if (M::WorldToScreen( vecOrigin,ret )) 
				return ret;

			return Vector2D( 0, 0 );
		}
	}

	namespace Trace 
	{
		Trace_t* TraceLine( int iSkipEntIndex, int iMask, Vector vecStart, Vector vecEnd ) 
		{
			Trace_t output = Trace_t();
			CTraceFilter filter( i::EntityList->GetClientEntity( iSkipEntIndex ) );

			Ray_t ray( vecStart, vecEnd );
			i::EngineTrace->TraceRay( ray, iMask, &filter, &output );
			return &output;
		}
	}

	namespace PlayerList 
	{
		LUAClasses::LuaPlayerList_PlayerSettings GetPlayerSettings( int iIndex ) 
		{
			playerSettings_t* pSettings = &playerList::arrPlayers.at( iIndex );
			if (!pSettings) 
			{
				return LUAClasses::LuaPlayerList_PlayerSettings( );
			}

			return LUAClasses::LuaPlayerList_PlayerSettings( pSettings );
		}
	}
}

void MyLuaPrint( std::string szText, sol::this_state L )
{
	misc::Print( szText );
}

//vec2_t LuaImplementation::ScriptFont_t::GetTextSize_( std::string szText, sol::this_state L )
//{
//	auto text_size = this->GetTextSize( szText );
//	return vec2_t( text_size.w, text_size.h );
//}
//
//void LuaImplementation::ScriptFont_t::DrawText_( vec2_t vecPos, std::string szText, color cColor, sol::this_state L )
//{
//	this->DrawPrintText( vecPos.x, vecPos.y, szText, cColor );
//}

void LuaImplementation::PrintError( std::string szError ) 
{
	misc::Print( XorStr( "[Lua error] " ) + szError );
}

void LuaImplementation::DisableTables( sol::state& lua )
{
#ifndef DEV
	// thx panza. 
	lua[ XorStr( "getfenv" ) ] = sol::nil;
	lua[ XorStr( "setfenv" ) ] = sol::nil;
	lua[ XorStr( "pcall" ) ] = sol::nil;
	lua[ XorStr( "xpcall" ) ] = sol::nil;
	lua[ XorStr( "load" ) ] = sol::nil;
	lua[ XorStr( "loadstring" ) ] = sol::nil;
	lua[ XorStr( "dofile" ) ] = sol::nil;
	lua[ XorStr( "gcinfo" ) ] = sol::nil;
	lua[ XorStr( "collectgarbage" ) ] = sol::nil;
	lua[ XorStr( "newproxy" ) ] = sol::nil;
	lua[ XorStr( "coroutine" ) ] = sol::nil;
	lua[ XorStr( "setfenv" ) ] = sol::nil;
	lua[ XorStr( "rawget" ) ] = sol::nil;
	lua[ XorStr( "rawset" ) ] = sol::nil;
	lua[ XorStr( "rawequal" ) ] = sol::nil;
	lua[ XorStr( "_G" ) ] = sol::nil;
	lua[ XorStr( "string" ) ][ XorStr( "dump" ) ] = sol::nil;
	lua[ XorStr( "ffi" ) ][ XorStr( "C" ) ] = sol::nil; // native C lib for LuaJit
	lua[ XorStr( "ffi" ) ][ XorStr( "load" ) ] = sol::nil;
	lua[ XorStr( "ffi" ) ][ XorStr( "gc" ) ] = sol::nil;
	lua[ XorStr( "ffi" ) ][ XorStr( "fill" ) ] = sol::nil;
#endif
}

void LuaImplementation::CreateLuaState( )
{
	lua = sol::state( sol::c_call< decltype( &LuaPanic ), &LuaPanic > );

	// open default libraries
	lua.open_libraries( );

	// open extra libraries
	lua.open_libraries( sol::lib::os, sol::lib::ffi, sol::lib::bit32 );

	//DisableTables( lua );

	// override print
	lua[ XorStr( "print" ) ] = MyLuaPrint;

	// require ffi by default
	lua.script( XorStr( "local ffi = require(\"ffi\")" ) );

	// https://github.com/ThePhD/sol2/issues/90
	// override folder used when searching for libraries for require
	// require("something.lua")
	// helpers::szLibrariesPath
	{
		//const std::string default_package_path = lua [ XorStr( "package" ) ][ XorStr( "path" ) ];
		//lua [ XorStr( "package" ) ][ XorStr( "path" ) ] = helpers::szLibrariesPath + XorStr( "?.lua" ) + ( default_package_path.empty( ) ? "" : XorStr( ";" ) + default_package_path );
		lua[ XorStr( "package" ) ][ XorStr( "path" ) ] =
			helpers::szLibrariesPath + XorStr( "?.lua" ) +
			XorStr( ";" ) + helpers::szLibrariesPath + XorStr( "?\\?.lua" ) +
			XorStr( ";" ) + helpers::szLibrariesPath + XorStr( "?\\init.lua" );
	}

	/* BaseMenuItem_t */ {
		auto ut_BaseMenuItem_t = lua.new_usertype< BaseMenuItem_t >( XorStr( "BaseMenuItem_t" ) );
		ut_BaseMenuItem_t[ XorStr( "IsVisible" ) ] = &BaseMenuItem_t::IsVisible;
		ut_BaseMenuItem_t[ XorStr( "SetVisible" ) ] = &BaseMenuItem_t::SetVisible;
	}

	/* CheckboxMenuItem_t */ {
		auto ut_CheckboxMenuItem_t = lua.new_usertype< CheckboxMenuItem_t >( XorStr( "CheckboxMenuItem_t" ), sol::base_classes, sol::bases< BaseMenuItem_t >( ) );
		ut_CheckboxMenuItem_t[ XorStr( "Get" ) ] = &CheckboxMenuItem_t::Get;
		ut_CheckboxMenuItem_t[ XorStr( "Set" ) ] = &CheckboxMenuItem_t::Set;

		// from BaseMenuItem_t
		ut_CheckboxMenuItem_t[ XorStr( "IsVisible" ) ] = &CheckboxMenuItem_t::IsVisible;
		ut_CheckboxMenuItem_t[ XorStr( "SetVisible" ) ] = &CheckboxMenuItem_t::SetVisible;
	}

	/* SliderIntMenuItem_t */ {
		auto ut_SliderIntMenuItem_t = lua.new_usertype< SliderIntMenuItem_t >( XorStr( "SliderIntMenuItem_t" ), sol::base_classes, sol::bases< BaseMenuItem_t >( ) );
		ut_SliderIntMenuItem_t[ XorStr( "Get" ) ] = &SliderIntMenuItem_t::Get;
		ut_SliderIntMenuItem_t[ XorStr( "Set" ) ] = &SliderIntMenuItem_t::Set;
		ut_SliderIntMenuItem_t[ XorStr( "GetMin" ) ] = &SliderIntMenuItem_t::GetMin;
		ut_SliderIntMenuItem_t[ XorStr( "SetMin" ) ] = &SliderIntMenuItem_t::SetMin;
		ut_SliderIntMenuItem_t[ XorStr( "GetMax" ) ] = &SliderIntMenuItem_t::GetMax;
		ut_SliderIntMenuItem_t[ XorStr( "SetMax" ) ] = &SliderIntMenuItem_t::SetMax;
		ut_SliderIntMenuItem_t[ XorStr( "GetFormat" ) ] = &SliderIntMenuItem_t::GetFormat;
		ut_SliderIntMenuItem_t[ XorStr( "SetFormat" ) ] = &SliderIntMenuItem_t::SetFormat;
		ut_SliderIntMenuItem_t[ XorStr( "ResetFormat" ) ] = &SliderIntMenuItem_t::ResetFormat;

		// from BaseMenuItem_t
		ut_SliderIntMenuItem_t[ XorStr( "IsVisible" ) ] = &SliderIntMenuItem_t::IsVisible;
		ut_SliderIntMenuItem_t[ XorStr( "SetVisible" ) ] = &SliderIntMenuItem_t::SetVisible;
	}

	/* SliderFloatMenuItem_t */ {
		auto ut_SliderFloatMenuItem_t = lua.new_usertype< SliderFloatMenuItem_t >( XorStr( "SliderFloatMenuItem_t" ), sol::base_classes, sol::bases< BaseMenuItem_t >( ) );
		ut_SliderFloatMenuItem_t[ XorStr( "Get" ) ] = &SliderFloatMenuItem_t::Get;
		ut_SliderFloatMenuItem_t[ XorStr( "Set" ) ] = &SliderFloatMenuItem_t::Set;
		ut_SliderFloatMenuItem_t[ XorStr( "GetMin" ) ] = &SliderFloatMenuItem_t::GetMin;
		ut_SliderFloatMenuItem_t[ XorStr( "SetMin" ) ] = &SliderFloatMenuItem_t::SetMin;
		ut_SliderFloatMenuItem_t[ XorStr( "GetMax" ) ] = &SliderFloatMenuItem_t::GetMax;
		ut_SliderFloatMenuItem_t[ XorStr( "SetMax" ) ] = &SliderFloatMenuItem_t::SetMax;
		ut_SliderFloatMenuItem_t[ XorStr( "GetFormat" ) ] = &SliderFloatMenuItem_t::GetFormat;
		ut_SliderFloatMenuItem_t[ XorStr( "SetFormat" ) ] = &SliderFloatMenuItem_t::SetFormat;
		ut_SliderFloatMenuItem_t[ XorStr( "ResetFormat" ) ] = &SliderFloatMenuItem_t::ResetFormat;

		// from BaseMenuItem_t
		ut_SliderFloatMenuItem_t[ XorStr( "IsVisible" ) ] = &SliderFloatMenuItem_t::IsVisible;
		ut_SliderFloatMenuItem_t[ XorStr( "SetVisible" ) ] = &SliderFloatMenuItem_t::SetVisible;
	}

	/* KeybindMenuItem_t */ {
		auto ut_KeybindMenuItem_t = lua.new_usertype< KeybindMenuItem_t >( XorStr( "KeybindMenuItem_t" ), sol::base_classes, sol::bases< BaseMenuItem_t >( ) );
		ut_KeybindMenuItem_t[ XorStr( "IsActive" ) ] = &KeybindMenuItem_t::IsActive;
		ut_KeybindMenuItem_t[ XorStr( "Get" ) ] = &KeybindMenuItem_t::Get;
		ut_KeybindMenuItem_t[ XorStr( "Set" ) ] = &KeybindMenuItem_t::Set;
		ut_KeybindMenuItem_t[ XorStr( "GetKeyMode" ) ] = &KeybindMenuItem_t::GetKeyMode;
		ut_KeybindMenuItem_t[ XorStr( "SetKeyMode" ) ] = &KeybindMenuItem_t::SetKeyMode;

		// from BaseMenuItem_t
		ut_KeybindMenuItem_t[ XorStr( "IsVisible" ) ] = &KeybindMenuItem_t::IsVisible;
		ut_KeybindMenuItem_t[ XorStr( "SetVisible" ) ] = &KeybindMenuItem_t::SetVisible;
	}

	/* ComboMenuItem_t */ 
	{
		auto ut_ComboMenuItem_t = lua.new_usertype< ComboMenuItem_t >( XorStr( "ComboMenuItem_t" ), sol::base_classes, sol::bases< BaseMenuItem_t >( ) );
		ut_ComboMenuItem_t[ XorStr( "Get" ) ] = &ComboMenuItem_t::Get;
		ut_ComboMenuItem_t[ XorStr( "Set" ) ] = &ComboMenuItem_t::Set;

		// from BaseMenuItem_t
		ut_ComboMenuItem_t[ XorStr( "IsVisible" ) ] = &ComboMenuItem_t::IsVisible;
		ut_ComboMenuItem_t[ XorStr( "SetVisible" ) ] = &ComboMenuItem_t::SetVisible;
	}

	/* MultiComboMenuItem_t */ 
	{
		auto ut_MultiComboMenuItem_t = lua.new_usertype< MultiComboMenuItem_t >( XorStr( "MultiComboMenuItem_t" ), sol::base_classes, sol::bases< BaseMenuItem_t >( ) );
		ut_MultiComboMenuItem_t[ XorStr( "Get" ) ] = &MultiComboMenuItem_t::Get;
		ut_MultiComboMenuItem_t[ XorStr( "Set" ) ] = &MultiComboMenuItem_t::Set;
		ut_MultiComboMenuItem_t[ XorStr( "GetByLabel" ) ] = &MultiComboMenuItem_t::GetByLabel;
		ut_MultiComboMenuItem_t[ XorStr( "SetByLabel" ) ] = &MultiComboMenuItem_t::SetByLabel;

		// from BaseMenuItem_t
		ut_MultiComboMenuItem_t[ XorStr( "IsVisible" ) ] = &MultiComboMenuItem_t::IsVisible;
		ut_MultiComboMenuItem_t[ XorStr( "SetVisible" ) ] = &MultiComboMenuItem_t::SetVisible;
	}

	/* ColorPickerMenuItem_t */ 
	{
		auto ut_ColorPickerMenuItem_t = lua.new_usertype< ColorPickerMenuItem_t >( XorStr( "ColorPickerMenuItem_t" ), sol::base_classes, sol::bases< BaseMenuItem_t >( ) );
		ut_ColorPickerMenuItem_t[ XorStr( "Get" ) ] = &ColorPickerMenuItem_t::Get;
		ut_ColorPickerMenuItem_t[ XorStr( "Set" ) ] = &ColorPickerMenuItem_t::Set;

		// from BaseMenuItem_t
		ut_ColorPickerMenuItem_t[ XorStr( "IsVisible" ) ] = &ColorPickerMenuItem_t::IsVisible;
		ut_ColorPickerMenuItem_t[ XorStr( "SetVisible" ) ] = &ColorPickerMenuItem_t::SetVisible;
	}

	/* ButtonMenuItem_t */ 
	{
		auto ut_ButtonMenuItem_t = lua.new_usertype< ButtonMenuItem_t >( XorStr( "ButtonMenuItem_t" ), sol::base_classes, sol::bases< BaseMenuItem_t >( ) );
		ut_ButtonMenuItem_t[ XorStr( "SetFunction" ) ] = &ButtonMenuItem_t::SetFunction;

		// from BaseMenuItem_t
		ut_ButtonMenuItem_t[ XorStr( "IsVisible" ) ] = &ButtonMenuItem_t::IsVisible;
		ut_ButtonMenuItem_t[ XorStr( "SetVisible" ) ] = &ButtonMenuItem_t::SetVisible;
	}

	/* Keybind_t */ 
	{
		//auto ut_Keybind_t = lua.new_usertype< Keybind_t >( XorStr( "Keybind_t" ) );
		//ut_Keybind_t[ XorStr( "GetName" ) ] = &Keybind_t::GetName;
		//ut_Keybind_t[ XorStr( "GetKey" ) ] = &Keybind_t::GetKey;
		//ut_Keybind_t[ XorStr( "GetKeyMode" ) ] = &Keybind_t::GetKeyMode;
		//ut_Keybind_t[ XorStr( "IsActive" ) ] = &Keybind_t::IsActive;
	}

	/* IGameEvent */ 
	{
		auto ut_IGameEvent = lua.new_usertype< IGameEvent >( XorStr( "IGameEvent" ) );
		ut_IGameEvent[ XorStr( "GetBool" ) ] = &IGameEvent::GetBool;
		ut_IGameEvent[ XorStr( "GetInt" ) ] = &IGameEvent::GetInt;
		ut_IGameEvent[ XorStr( "GetFloat" ) ] = &IGameEvent::GetFloat;
		ut_IGameEvent[ XorStr( "GetString" ) ] = &IGameEvent::GetString;
		ut_IGameEvent[ XorStr( "SetBool" ) ] = &IGameEvent::SetBool;
		ut_IGameEvent[ XorStr( "SetInt" ) ] = &IGameEvent::SetInt;
		ut_IGameEvent[ XorStr( "SetFloat" ) ] = &IGameEvent::SetFloat;
		ut_IGameEvent[ XorStr( "SetString" ) ] = &IGameEvent::SetString;
	}

	/* ConVar */ 
	{
		auto ut_ConVar = lua.new_usertype< CConVar >( XorStr( "ConVar" ) );
		ut_ConVar[ XorStr( "GetString" ) ] = &CConVar::GetString;
		ut_ConVar[ XorStr( "GetFloat" ) ] = &CConVar::GetFloat;
		ut_ConVar[ XorStr( "GetInt" ) ] = &CConVar::GetInt;
		ut_ConVar[ XorStr( "SetValueString" ) ] = &CConVar::SetValueString;
		ut_ConVar[ XorStr( "SetValueFloat" ) ] = &CConVar::SetValueFloat;
		ut_ConVar[ XorStr( "SetValueInt" ) ] = &CConVar::SetValueInt;
	}

	/* CGameTrace */ 
	{
		auto ut_CGameTrace = lua.new_usertype< CGameTrace >(
			XorStr( "CGameTrace" ),

			sol::meta_function::index, &CGameTrace::_lua_get
			// no sol::meta_function::new_index

		);
	}

	/* Vector2D */ 
	{
		auto ut_vec2_t = lua.new_usertype< Vector2D >(
			XorStr( "Vector2D" ),

			sol::call_constructor, sol::constructors< Vector2D( ), Vector2D( float, float ) >( ),

			sol::meta_function::index, & Vector2D::_lua_get,
			sol::meta_function::new_index, & Vector2D::_lua_set,

			XorStr( "Length" ), & Vector2D::Length
		);
	}

	/* Vector3D */ 
	{
		auto ut_vec3_t = lua.new_usertype< Vector >(
			XorStr( "Vector3D" ),

			sol::call_constructor, sol::constructors< Vector( ), Vector( float, float, float ) >( ),

			sol::meta_function::index, & Vector::_lua_get,
			sol::meta_function::new_index, & Vector::_lua_set,

			XorStr( "Length2D" ), & Vector::Length2D,
			XorStr( "Length2DSqr" ), & Vector::Length2DSqr,
			XorStr( "Length" ), & Vector::Length,
			XorStr( "LengthSqr" ), & Vector::LengthSqr
		);
	}

	/* Color */ 
	{
		auto ut_Color = lua.new_usertype< Color >(
			XorStr( "Color" ),

			sol::call_constructor, sol::constructors< Color( ), Color( int, int, int ), Color( int, int, int, int ) >( ),

			sol::meta_function::index, &Color::_lua_get,
			sol::meta_function::new_index, & Color::_lua_set
		);
	}

	///* ScriptVarReference_t */ {
	//	auto ut_ScriptVarReference_t = lua.new_usertype< ScriptVarReference_t >( XorStr( "ScriptVarReference_t" ) );
	//	ut_ScriptVarReference_t[ XorStr( "GetBool" ) ] = &ScriptVarReference_t::GetBool;
	//	ut_ScriptVarReference_t[ XorStr( "GetInt" ) ] = &ScriptVarReference_t::GetInt;
	//	ut_ScriptVarReference_t[ XorStr( "GetFloat" ) ] = &ScriptVarReference_t::GetFloat;
	//	ut_ScriptVarReference_t[ XorStr( "GetColor" ) ] = &ScriptVarReference_t::GetColor;
	//	ut_ScriptVarReference_t[ XorStr( "GetString" ) ] = &ScriptVarReference_t::GetString;
	//	ut_ScriptVarReference_t[ XorStr( "SetBool" ) ] = &ScriptVarReference_t::SetBool;
	//	ut_ScriptVarReference_t[ XorStr( "SetInt" ) ] = &ScriptVarReference_t::SetInt;
	//	ut_ScriptVarReference_t[ XorStr( "SetFloat" ) ] = &ScriptVarReference_t::SetFloat;
	//	ut_ScriptVarReference_t[ XorStr( "SetColor" ) ] = &ScriptVarReference_t::SetColor;
	//	ut_ScriptVarReference_t[ XorStr( "SetString" ) ] = &ScriptVarReference_t::SetString;
	//}

	/* INetChannelInfo */ {
		auto ut_INetChannelInfo = lua.new_usertype< INetChannelInfo >( XorStr( "INetChannelInfo" ) );
		ut_INetChannelInfo[ XorStr( "GetAddress" ) ] = &INetChannelInfo::GetAddress;
		ut_INetChannelInfo[ XorStr( "IsLoopback" ) ] = &INetChannelInfo::IsLoopback;
		ut_INetChannelInfo[ XorStr( "IsTimingOut" ) ] = &INetChannelInfo::IsTimingOut;
		ut_INetChannelInfo[ XorStr( "IsPlayback" ) ] = &INetChannelInfo::IsPlayback;
		ut_INetChannelInfo[ XorStr( "GetLatency" ) ] = &INetChannelInfo::GetLatency;
		ut_INetChannelInfo[ XorStr( "GetAvgLatency" ) ] = &INetChannelInfo::GetAvgLatency;
		ut_INetChannelInfo[ XorStr( "GetAvgLoss" ) ] = &INetChannelInfo::GetAvgLoss;
		ut_INetChannelInfo[ XorStr( "GetAvgChoke" ) ] = &INetChannelInfo::GetAvgChoke;
	}

	/* player_info_t */ {
		//auto ut_player_info_t = lua.new_usertype< lua_classes::LuaPlayerInfo >( XorStr( "player_info_t" ) );
		//ut_player_info_t [ XorStr( "name" ) ] = &lua_classes::LuaPlayerInfo::name;
		//ut_player_info_t [ XorStr( "userid" ) ] = &lua_classes::LuaPlayerInfo::userid;
		//ut_player_info_t [ XorStr( "guid" ) ] = &lua_classes::LuaPlayerInfo::guid;
		//ut_player_info_t [ XorStr( "bot" ) ] = &lua_classes::LuaPlayerInfo::bot;
		auto ut_Color = lua.new_usertype< LUAClasses::LuaPlayerInfo >(
			XorStr( "player_info_t" ),

			sol::meta_function::index, & LUAClasses::LuaPlayerInfo::_lua_get
		);
	}

	/* PlayerList_PlayerSettings */ 
	{
		auto ut_PlayerList_PlayerSettings = lua.new_usertype< LUAClasses::LuaPlayerList_PlayerSettings >( XorStr( "PlayerList_PlayerSettings" ) );
		ut_PlayerList_PlayerSettings[ XorStr( "IsRagebotWhitelisted" ) ] = &LUAClasses::LuaPlayerList_PlayerSettings::IsRagebotWhitelisted;
		ut_PlayerList_PlayerSettings[ XorStr( "SetRagebotWhitelist" ) ] = &LUAClasses::LuaPlayerList_PlayerSettings::SetRagebotWhitelist;
		ut_PlayerList_PlayerSettings[ XorStr( "IsOverridingResolver" ) ] = &LUAClasses::LuaPlayerList_PlayerSettings::IsOverridingResolver;
		ut_PlayerList_PlayerSettings[ XorStr( "ToggleResolverOverride" ) ] = &LUAClasses::LuaPlayerList_PlayerSettings::ToggleResolverOverride;
		ut_PlayerList_PlayerSettings[ XorStr( "SetCustomResolveYaw" ) ] = &LUAClasses::LuaPlayerList_PlayerSettings::SetCustomResolveYaw;

	}

	/* LuaRageBot_CachedData */
	{
		auto ut_RageBot_CachedData = lua.new_usertype< LUAClasses::RageBot_CachedData >( XorStr( "RageBot_CachedData" ) );
		// get
		ut_RageBot_CachedData[ XorStr( "GetBacktrack" ) ] = &LUAClasses::RageBot_CachedData::GetBacktrack;
		ut_RageBot_CachedData[ XorStr( "GetDamage" ) ] = &LUAClasses::RageBot_CachedData::GetDamage;
		ut_RageBot_CachedData[ XorStr( "GetHitbox" ) ] = &LUAClasses::RageBot_CachedData::GetHitbox;
		ut_RageBot_CachedData[ XorStr( "GetHitChance" ) ] = &LUAClasses::RageBot_CachedData::GetHitChance;
		ut_RageBot_CachedData[ XorStr( "GetLocalShootPosition" ) ] = &LUAClasses::RageBot_CachedData::GetLocalShootPosition;
		ut_RageBot_CachedData[ XorStr( "GetTarget" ) ] = &LUAClasses::RageBot_CachedData::GetTarget;
		ut_RageBot_CachedData[ XorStr( "GetTargetSimulationTime" ) ] = &LUAClasses::RageBot_CachedData::GetTargetSimulationTime;

		// can
		ut_RageBot_CachedData[ XorStr( "CanShoot" ) ] = &LUAClasses::RageBot_CachedData::CanShoot;
	}

	/* CUserCmd */ {
		
		auto ut_CUserCmd = lua.new_usertype< CUserCmd >(
			XorStr( "CUserCmd" ),

			sol::meta_function::index, &CUserCmd::_lua_get,
			sol::meta_function::new_index, &CUserCmd::_lua_set
		);
	}

	///* font_t */ {
	//	// we also need CSurfaceFont, cuz ScriptFont_t it's derived from this
	//	auto ut_CSurfaceFont = lua.new_usertype< CSurfaceFont >( XorStr( "CSurfaceFont" ) );

	//	auto ut_font_t = lua.new_usertype< ScriptFont_t >( XorStr( "font_t" ), sol::base_classes, sol::bases< CSurfaceFont >( ) );
	//	ut_font_t[ XorStr( "GetTextSize" ) ] = &ScriptFont_t::GetTextSize_;
	//	ut_font_t[ XorStr( "DrawText" ) ] = &ScriptFont_t::DrawText_;
	//}

	/* CEntity */ {
		auto ut_CEntity = lua.new_usertype< LUAClasses::LuaEntity >( XorStr( "CEntity" ) );
		ut_CEntity[ XorStr( "GetEyePosition" ) ] = &LUAClasses::LuaEntity::GetEyePosition;
		ut_CEntity[ XorStr( "ToPlayer" ) ] = &LUAClasses::LuaEntity::ToPlayer;
		ut_CEntity[ XorStr( "GetAddress" ) ] = &LUAClasses::LuaEntity::GetAddress;
		ut_CEntity[ XorStr( "GetIndex" ) ] = &LUAClasses::LuaEntity::GetIndex;
		ut_CEntity[ XorStr( "IsDormant" ) ] = &LUAClasses::LuaEntity::IsDormant;
		ut_CEntity[ XorStr( "GetPropInt" ) ] = &LUAClasses::LuaEntity::GetPropInt;
		ut_CEntity[ XorStr( "GetPropFloat" ) ] = &LUAClasses::LuaEntity::GetPropFloat;
		ut_CEntity[ XorStr( "GetPropShort" ) ] = &LUAClasses::LuaEntity::GetPropShort;
		ut_CEntity[ XorStr( "GetPropDouble" ) ] = &LUAClasses::LuaEntity::GetPropDouble;
		ut_CEntity[ XorStr( "GetPropBool" ) ] = &LUAClasses::LuaEntity::GetPropBool;
		ut_CEntity[ XorStr( "GetPropVector" ) ] = &LUAClasses::LuaEntity::GetPropVector;
		ut_CEntity[ XorStr( "SetPropInt" ) ] = &LUAClasses::LuaEntity::SetPropInt;
		ut_CEntity[ XorStr( "SetPropFloat" ) ] = &LUAClasses::LuaEntity::SetPropFloat;
		ut_CEntity[ XorStr( "SetPropShort" ) ] = &LUAClasses::LuaEntity::SetPropShort;
		ut_CEntity[ XorStr( "SetPropDouble" ) ] = &LUAClasses::LuaEntity::SetPropDouble;
		ut_CEntity[ XorStr( "SetPropBool" ) ] = &LUAClasses::LuaEntity::SetPropBool;
		ut_CEntity[ XorStr( "SetPropVector" ) ] = &LUAClasses::LuaEntity::SetPropVector;

	}

	/* CPlayer */ {
		auto ut_CPlayer = lua.new_usertype< LUAClasses::LuaPlayer >( XorStr( "CPlayer" ), sol::base_classes, sol::bases< LUAClasses::LuaEntity >( ) );
		ut_CPlayer[ XorStr( "IsEnemy" ) ] = &LUAClasses::LuaPlayer::IsEnemyOf;
		ut_CPlayer[ XorStr( "GetAbsOrigin" ) ] = &LUAClasses::LuaPlayer::GetAbsOrigin;
		ut_CPlayer[ XorStr( "GetAbsAngles" ) ] = &LUAClasses::LuaPlayer::GetAbsAngles;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Now Functions in general
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* Callbacks */ {
		auto Callbacks = lua.create_table( );
		Callbacks[ XorStr( "Add" ) ] = LUAModules::CallBacks::Add;
		lua[ XorStr( "Callbacks" ) ] = Callbacks;
	}

	/* GlobalVars */ 
	{
		auto GlobalVars = lua.create_table( );
		GlobalVars[ XorStr( "GetRealtime" ) ] = LUAModules::GlobalVars::GetRealtime;
		GlobalVars[ XorStr( "GetFrameCount" ) ] = LUAModules::GlobalVars::GetFrameCount;
		GlobalVars[ XorStr( "GetAbsFrametime" ) ] = LUAModules::GlobalVars::GetAbsFrametime;
		GlobalVars[ XorStr( "GetCurtime" ) ] = LUAModules::GlobalVars::GetCurtime;
		GlobalVars[ XorStr( "GetFrametime" ) ] = LUAModules::GlobalVars::GetFrametime;
		GlobalVars[ XorStr( "GetTickcount" ) ] = LUAModules::GlobalVars::GetTickcount;
		GlobalVars[ XorStr( "GetIntervalPerTick" ) ] = LUAModules::GlobalVars::GetIntervalPerTick;
		GlobalVars[ XorStr( "GetInterpolationAmount" ) ] = LUAModules::GlobalVars::GetInterpolationAmount;
		lua[ XorStr( "GlobalVars" ) ] = GlobalVars;
	}

	/* ClientState */ 
	{
		auto ClientState = lua.create_table( );
		ClientState[ XorStr( "GetLastOutgoingCommand" ) ] = LUAModules::ClientState::GetLastOutgoingCommand;
		ClientState[ XorStr( "GetChokedCommands" ) ] = LUAModules::ClientState::GetChokedCommands;
		ClientState[ XorStr( "GetLastCommandAck" ) ] = LUAModules::ClientState::GetLastCommandAck;
		ClientState[ XorStr( "GetCommandAck" ) ] = LUAModules::ClientState::GetCommandAck;
		lua[ XorStr( "ClientState" ) ] = ClientState;
	}

	/* Engine */ 
	{
		auto Engine = lua.create_table( );
		Engine[ XorStr( "GetViewAngles" ) ] = LUAModules::Engine::GetViewAngles;
		Engine[ XorStr( "SetViewAngles" ) ] = LUAModules::Engine::SetViewAngles;
		Engine[ XorStr( "ExecuteClientCmd" ) ] = LUAModules::Engine::ExecuteClientCmd;
		Engine[ XorStr( "GetNetChannelInfo" ) ] = LUAModules::Engine::GetNetChannelInfo;
		Engine[ XorStr( "GetPlayerInfo" ) ] = LUAModules::Engine::GetPlayerInfo;
		Engine[ XorStr( "GetLocalPlayer" ) ] = LUAModules::Engine::GetLocalPlayer;
		Engine[ XorStr( "GetPlayerForUserID" ) ] = LUAModules::Engine::GetPlayerForUserID;
		Engine[ XorStr( "IsInGame" ) ] = LUAModules::Engine::IsInGame;
		Engine[ XorStr( "IsConnected" ) ] = LUAModules::Engine::IsConnected;
		Engine[ XorStr( "IsVoiceRecording" ) ] = LUAModules::Engine::IsVoiceRecording;
		Engine[ XorStr( "GetScreenSize" ) ] = LUAModules::Engine::GetScreenSize;
		lua[ XorStr( "Engine" ) ] = Engine;
	}

	/* Render */ 
	{
		auto Render = lua.create_table( );
		Render[ XorStr( "DrawFilledRect" ) ] = LUAModules::Render::DrawFilledRect;
		Render[ XorStr( "DrawOutlinedRect" ) ] = LUAModules::Render::DrawOutlinedRect;
		Render[ XorStr( "DrawLine" ) ] = LUAModules::Render::DrawLine;
		Render[ XorStr( "DrawString" ) ] = LUAModules::Render::DrawString;
		lua[ XorStr( "Render" ) ] = Render;
	}

	/* Print */
	{
		auto Print = lua.create_table( );
		Print[ XorStr( "PrintToConsole" ) ] = LUAModules::Print::PrintToConsole;
		lua[ XorStr( "Print" ) ] = Print;
	}

	/* Netvars */ 
	{
		auto Netvars = lua.create_table( );
		Netvars[ XorStr( "FindOffset" ) ] = LUAModules::NetVars::FindOffset;
		lua[ XorStr( "Netvars" ) ] = Netvars;
	}

	/* EntityList */ {
		auto EntityList = lua.create_table( );
		EntityList[ XorStr( "GetClientEntity" ) ] = LUAModules::EntityList::GetClientEntity;
		EntityList[ XorStr( "GetClientEntityFromHandle" ) ] = LUAModules::EntityList::GetClientEntityFromHandle;
		EntityList[ XorStr( "GetHighestEntityIndex" ) ] = LUAModules::EntityList::GetHighestEntityIndex;
		lua[ XorStr( "EntityList" ) ] = EntityList;
	}

	/* Menu */ {
		auto Menu = lua.create_table( );
		Menu[ XorStr( "IsOpened" ) ] = LUAModules::Menu::IsOpened;
		Menu[ XorStr( "AddCheckbox" ) ] = LUAModules::Menu::AddCheckbox;
		Menu[ XorStr( "AddSliderInt" ) ] = LUAModules::Menu::AddSliderInt;
		Menu[ XorStr( "AddSliderFloat" ) ] = LUAModules::Menu::AddSliderFloat;
		Menu[ XorStr( "AddKeybind" ) ] = LUAModules::Menu::AddKeybind;
		Menu[ XorStr( "AddCombo" ) ] = LUAModules::Menu::AddCombo;
		Menu[ XorStr( "AddMultiCombo" ) ] = LUAModules::Menu::AddMultiCombo;
		Menu[ XorStr( "AddColorPicker" ) ] = LUAModules::Menu::AddColorPicker;
		Menu[ XorStr( "AddButton" ) ] = LUAModules::Menu::AddButton;
		lua[ XorStr( "Menu" ) ] = Menu;
	}

	/* Vars */ {
		auto Vars = lua.create_table( );
		Vars[ XorStr( "GetBool" ) ] = LUAModules::Variables::GetBool;
		Vars[ XorStr( "GetFloat" ) ] = LUAModules::Variables::GetFloat;
		Vars[ XorStr( "GetInt" ) ] = LUAModules::Variables::GetInt;
		Vars[ XorStr( "GetString" ) ] = LUAModules::Variables::GetString;

		Vars[ XorStr( "SetBool" ) ] = LUAModules::Variables::SetBool;
		Vars[ XorStr( "SetFloat" ) ] = LUAModules::Variables::SetFloat;
		Vars[ XorStr( "SetInt" ) ] = LUAModules::Variables::SetInt;
		Vars[ XorStr( "SetString" ) ] = LUAModules::Variables::SetString;
		lua[ XorStr( "Vars" ) ] = Vars;
	}

	/* Convars */ {
		auto Convars = lua.create_table( );
		Convars[ XorStr( "FindVar" ) ] = LUAModules::CVars::FindVar;
		lua[ XorStr( "Convars" ) ] = Convars;
	}

	/* Input */ 
	{
		auto Input = lua.create_table( );
		/*Input[ XorStr( "PressedKey" ) ] = LUAModules::_input::PressedKey;
		Input[ XorStr( "HoldingKey" ) ] = LUAModules::_input::HoldingKey;
		Input[ XorStr( "ReleasedKey" ) ] = LUAModules::_input::ReleasedKey;
		Input[ XorStr( "GetCursorPos" ) ] = LUAModules::_input::GetCursorPos;*/
		lua[ XorStr( "Input" ) ] = Input;
	}

	/* Utils */ 
	{
		auto Utils = lua.create_table( );
		Utils[ XorStr( "CreateInterface" ) ] = LUAModules::Utilities::CreateInterface;
		Utils[ XorStr( "FindSignature" ) ] = LUAModules::Utilities::FindSignature;
		Utils[ XorStr( "WorldToScreen" ) ] = LUAModules::Utilities::WorldToScreen;
		lua[ XorStr( "Utils" ) ] = Utils;
	}

	/* Trace */ 
	{
		auto Trace = lua.create_table( );
		Trace[ XorStr( "TraceLine" ) ] = LUAModules::Trace::TraceLine;
		lua[ XorStr( "Trace" ) ] = Trace;
	}

	/* PlayerList */ {
		auto PlayerList = lua.create_table( );
		PlayerList[ XorStr( "GetPlayerSettings" ) ] = LUAModules::PlayerList::GetPlayerSettings;
		lua[ XorStr( "PlayerList" ) ] = PlayerList;
	}
}

void LuaImplementation::Initialize( )
{
	vecScriptInfos.clear( );

	vecCallbackList.resize( CALLBACK_LIST_END );
	vecCallbackList[ CALLBACK_ON_UNLOAD ] = XorStr( "Unload" );
	vecCallbackList[ CALLBACK_ON_DRAW ] = XorStr( "Draw" );
	vecCallbackList[ CALLBACK_PRE_PREDICTION ] = XorStr( "Pre-Prediction" );
	vecCallbackList[ CALLBACK_PREDICTION ] = XorStr( "Prediction" );
	vecCallbackList[ CALLBACK_ON_CREATE_MOVE ] = XorStr( "Createmove" );
	vecCallbackList[ CALLBACK_FRAME_STAGE_NOTIFY ] = XorStr( "FrameStageNotify" );
	vecCallbackList[ CALLBACK_FIRE_GAME_EVENT ] = XorStr( "Event" );

	LuaImplementation::Parse( );

	CreateLuaState( );
}

void LuaImplementation::RunChecks( )
{
	// Should we unload any script?
	for (auto& script : vecScriptInfos) {
		bool bShouldUnload = script.bShouldUnload;
		script.bShouldUnload = false;

		if (!script.bLoaded)
			continue;

		if (bShouldUnload) {
			script.Unload( );
		}
	}
}

void LuaImplementation::Parse( )
{
	CHAR path[ MAX_PATH ];
	HRESULT result = SHGetFolderPathA( NULL, CSIDL_APPDATA, NULL, 0, path );
	if (result != S_OK) {
		PrintError( XorStr( "Could not get path to User\\appdata" ) );
		return;
	}

	std::string szPath = path;

	// btw https://stackoverflow.com/questions/9235679/create-a-directory-if-it-doesnt-exist

	// appdata/ryzextr
	szPath += XorStr( "\\ryzextr" );
	CreateDirectory( szPath.c_str( ), NULL );

	// appdata/ryzextr/scripts
	szPath += XorStr( "\\scripts" );
	CreateDirectory( szPath.c_str( ), NULL );

	// jump inside folder
	szPath += XorStr( "\\" );

	// create folder appdata/ryzextr/scripts/lib
	helpers::szLibrariesPath = ( szPath + XorStr( "lib" ) );
	CreateDirectory( helpers::szLibrariesPath.c_str( ), NULL );
	helpers::szLibrariesPath += XorStr( "\\" );

	// https://thispointer.com/c-case-insensitive-string-comparison-using-stl-c11-boost-library/
	auto caseInSensStringCompareCpp11 = []( std::string str1, std::string str2 )
	{
		return ( ( str1.size( ) == str2.size( ) ) && std::equal( str1.begin( ), str1.end( ), str2.begin( ), []( char& c1, char& c2 ) {
			return ( c1 == c2 || std::toupper( c1 ) == std::toupper( c2 ) );
			} ) );
	};

	// List of files that can be in list ( path )
	std::vector< std::string > vecAllowedPaths = { };

	int iIndex = 0;

	// Iterate files and work
	for (const auto& entry : std::filesystem::directory_iterator( szPath )) {
		if (entry.is_regular_file( )
			&& entry.path( ).has_filename( )
			&& entry.path( ).has_extension( )
			&& ( caseInSensStringCompareCpp11( entry.path( ).extension( ).string( ), XorStr( ".lua" ) ) || caseInSensStringCompareCpp11( entry.path( ).extension( ).string( ), XorStr( ".luac" ) ) ))
		{
			// Have we already found this script?
			bool bFoundScriptAlready = false;
			for (auto& script : vecScriptInfos) {
				if (script.szPath == entry.path( ).string( )) {
					bFoundScriptAlready = true;
					vecAllowedPaths.push_back( entry.path( ).string( ) );

					script.iIndex = iIndex;
					iIndex++;

					break;
				}
			}

			// We already found script, don't add it to list, since it's already there
			if (bFoundScriptAlready)
				continue;

			vecAllowedPaths.push_back( entry.path( ).string( ) );

			auto& script = vecScriptInfos.emplace_back( );
			script.szName = entry.path( ).filename( ).string( );
			script.szPath = entry.path( ).string( );
			script.iIndex = iIndex;
			//script.vecCallbacks = { };
			//script.bLoaded = false;

			iIndex++;
		}
	}

	// Erase scripts that are not inside list
	/*m_vecScriptInfos.erase( std::remove_if( m_vecScriptInfos.begin( ) , m_vecScriptInfos.end( ) , [ & ] ( const LuaImplementation::ScriptInfo_t& script_info ) {
		return std::find( vecAllowedPaths.begin( ) , vecAllowedPaths.end( ) , script_info.szPath ) == vecAllowedPaths.end( );
		} ) , m_vecScriptInfos.end( ) );*/

		// https://stackoverflow.com/questions/19461980/lua-close-crash-when-there-is-too-many-lua-state-in-an-vector-of-objects-in-c
	for (int i = 0; i < vecScriptInfos.size( ); i++) {
		if (std::find( vecAllowedPaths.begin( ), vecAllowedPaths.end( ), vecScriptInfos[ i ].szPath ) == vecAllowedPaths.end( )) {
			vecScriptInfos[ i ].Destroy( );
			vecScriptInfos.erase( vecScriptInfos.begin( ) + i );
		}
	}

	// Sort scripts by index
	std::sort( vecScriptInfos.begin( ), vecScriptInfos.end( ), []( const LuaImplementation::ScriptInfo_t& a, const LuaImplementation::ScriptInfo_t& b ) {
		return a.iIndex < b.iIndex;
		} );

}

//void LuaImplementation::SaveToJson( nlohmann::json& j )
//{
//	j[ XorStr( "scripts" ) ] = { };
//
//	for (auto& script : m_vecScriptInfos) {
//		// only save loaded scripts
//		if (!script.bLoaded)
//			continue;
//
//		auto& current_json = j[ XorStr( "scripts" ) ][ script.szName ];
//		current_json = { };
//
//		// save menu items
//		for (auto& item : script.vecMenuItems) {
//			if (item.iType == MENUITEM_CHECKBOX) {
//				LuaImplementation::CheckboxMenuItem_t& checkbox = reinterpret_cast< LuaImplementation::CheckboxMenuItem_t& >( item );
//				current_json[ checkbox.szVarName ] = { };
//				current_json[ checkbox.szVarName ][ XorStr( "type" ) ] = XorStr( "checkbox" );
//				current_json[ checkbox.szVarName ][ XorStr( "value" ) ] = checkbox.bValue;
//				//current_json [ checkbox.szVarName ][ "visible" ] = checkbox.bVisible;
//			}
//			else if (item.iType == MENUITEM_SLIDERINT) {
//				LuaImplementation::SliderIntMenuItem_t& slider = reinterpret_cast< LuaImplementation::SliderIntMenuItem_t& >( item );
//				current_json[ slider.szVarName ] = { };
//				current_json[ slider.szVarName ][ XorStr( "type" ) ] = XorStr( "sliderint" );
//				current_json[ slider.szVarName ][ XorStr( "value" ) ] = slider.iValue;
//			}
//			else if (item.iType == MENUITEM_SLIDERFLOAT) {
//				LuaImplementation::SliderFloatMenuItem_t& slider = reinterpret_cast< LuaImplementation::SliderFloatMenuItem_t& >( item );
//				current_json[ slider.szVarName ] = { };
//				current_json[ slider.szVarName ][ XorStr( "type" ) ] = XorStr( "sliderfloat" );
//				current_json[ slider.szVarName ][ XorStr( "value" ) ] = slider.flValue;
//			}
//			else if (item.iType == MENUITEM_KEYBIND) {
//				LuaImplementation::KeybindMenuItem_t& keybind = reinterpret_cast< LuaImplementation::KeybindMenuItem_t& >( item );
//				current_json[ keybind.szVarName ] = { };
//				current_json[ keybind.szVarName ][ XorStr( "type" ) ] = XorStr( "keybind" );
//				current_json[ keybind.szVarName ][ XorStr( "value" ) ] = keybind.iValue;
//				current_json[ keybind.szVarName ][ XorStr( "value_key_mode" ) ] = keybind.iKeyMode;
//			}
//			else if (item.iType == MENUITEM_COMBOBOX) {
//				LuaImplementation::ComboMenuItem_t& combo = reinterpret_cast< LuaImplementation::ComboMenuItem_t& >( item );
//				current_json[ combo.szVarName ] = { };
//				current_json[ combo.szVarName ][ XorStr( "type" ) ] = XorStr( "combobox" );
//				current_json[ combo.szVarName ][ XorStr( "value" ) ] = combo.iValue;
//			}
//			else if (item.iType == MENUITEM_MULTICOMBOBOX) {
//				LuaImplementation::MultiComboMenuItem_t& multi_combo = reinterpret_cast< LuaImplementation::MultiComboMenuItem_t& >( item );
//				current_json[ multi_combo.szVarName ] = { };
//				current_json[ multi_combo.szVarName ][ XorStr( "type" ) ] = XorStr( "multi_combobox" );
//				current_json[ multi_combo.szVarName ][ XorStr( "values" ) ] = { };
//				for (int i = 0; i < multi_combo.vecComboboxLabels.size( ) && i < multi_combo.vecMultiComboValues.size( ); i++) {
//					current_json[ multi_combo.szVarName ][ XorStr( "values" ) ][ multi_combo.vecComboboxLabels[ i ] ] = multi_combo.vecMultiComboValues[ i ];
//				}
//			}
//			else if (item.iType == MENUITEM_COLORPICKER) {
//				LuaImplementation::ColorPickerMenuItem_t& colorpicker = reinterpret_cast< LuaImplementation::ColorPickerMenuItem_t& >( item );
//				current_json[ colorpicker.szVarName ] = { };
//				current_json[ colorpicker.szVarName ][ XorStr( "type" ) ] = XorStr( "colorpicker" );
//				current_json[ colorpicker.szVarName ][ XorStr( "value" ) ] = ( unsigned int )colorpicker.cValue.GetU32( ); // we transform to u32 and back
//			}
//		}
//	}
//}
//
//void LuaImplementation::LoadFromJson( nlohmann::json j )
//{
//	if (j[ XorStr( "scripts" ) ].empty( )) {
//		// unload every script
//		for (auto& script : m_vecScriptInfos) {
//			if (!script.bLoaded)
//				continue;
//
//			script.Unload( );
//		}
//
//		return;
//	}
//
//	std::vector< std::string > vecLoadedScripts = { };
//
//	// here we loop through scripts
//	// load wanted scripts
//	// and load config
//	for (auto& i : j[ XorStr( "scripts" ) ].items( ))
//	{
//		if (i.key( ).find( XorStr( ".lua" ) ) == std::string::npos)
//			continue;
//
//		auto pScript = FindScriptByName( i.key( ) );
//		if (!pScript)
//			continue;
//
//		// load script
//		vecLoadedScripts.push_back( i.key( ) );
//		pScript->Load( );
//
//		// here we loop through menu items
//		for (auto& j : i.value( ).items( ))
//		{
//			// make sure it has a type
//			if (j.value( )[ XorStr( "type" ) ].empty( ) || !j.value( )[ XorStr( "type" ) ].is_string( ))
//				continue;
//
//			// find matching menu item
//			for (auto& menu_item : pScript->vecMenuItems) {
//
//				// matches variable name
//				if (menu_item.szVarName == j.key( )) {
//
//					// matching type
//					if (menu_item.iType == MENUITEM_CHECKBOX) {
//						if (j.value( )[ XorStr( "type" ) ].get< std::string >( ) != XorStr( "checkbox" ))
//							continue;
//
//						// load checkbox
//						if (j.value( )[ XorStr( "value" ) ].empty( ) || !j.value( )[ XorStr( "value" ) ].is_boolean( ))
//							continue;
//
//						LuaImplementation::CheckboxMenuItem_t& checkbox = reinterpret_cast< LuaImplementation::CheckboxMenuItem_t& >( menu_item );
//						checkbox.bValue = j.value( )[ XorStr( "value" ) ].get< bool >( );
//					}
//					else if (menu_item.iType == MENUITEM_SLIDERINT) {
//						if (j.value( )[ XorStr( "type" ) ].get< std::string >( ) != XorStr( "sliderint" ))
//							continue;
//
//						// load sliderint
//						if (j.value( )[ XorStr( "value" ) ].empty( ) || !j.value( )[ XorStr( "value" ) ].is_number_integer( ))
//							continue;
//
//						LuaImplementation::SliderIntMenuItem_t& slider = reinterpret_cast< LuaImplementation::SliderIntMenuItem_t& >( menu_item );
//						slider.iValue = j.value( )[ XorStr( "value" ) ].get< int >( );
//					}
//					else if (menu_item.iType == MENUITEM_SLIDERFLOAT) {
//						if (j.value( )[ XorStr( "type" ) ].get< std::string >( ) != XorStr( "sliderfloat" ))
//							continue;
//
//						// load sliderfloat
//						if (j.value( )[ XorStr( "value" ) ].empty( ) /*|| !j.value( ) [ "value" ].is_number_float( )*/ || !j.value( )[ XorStr( "value" ) ].is_number( ))
//							continue;
//
//						LuaImplementation::SliderFloatMenuItem_t& slider = reinterpret_cast< LuaImplementation::SliderFloatMenuItem_t& >( menu_item );
//						slider.flValue = j.value( )[ XorStr( "value" ) ].get< float >( );
//					}
//					else if (menu_item.iType == MENUITEM_KEYBIND) {
//						if (j.value( )[ XorStr( "type" ) ].get< std::string >( ) != XorStr( "keybind" ))
//							continue;
//
//						// load keybind
//						if (j.value( )[ XorStr( "value" ) ].empty( ) || !j.value( )[ XorStr( "value" ) ].is_number_integer( ))
//							continue;
//
//						LuaImplementation::KeybindMenuItem_t& keybind = reinterpret_cast< LuaImplementation::KeybindMenuItem_t& >( menu_item );
//						keybind.iValue = j.value( )[ XorStr( "value" ) ].get< int >( );
//
//						if (j.value( )[ XorStr( "value_key_mode" ) ].empty( ) || !j.value( )[ XorStr( "value_key_mode" ) ].is_number_integer( ))
//							continue;
//
//						keybind.iKeyMode = j.value( )[ XorStr( "value_key_mode" ) ].get< int >( );
//					}
//					else if (menu_item.iType == MENUITEM_COMBOBOX) {
//						if (j.value( )[ XorStr( "type" ) ].get< std::string >( ) != XorStr( "combobox" ))
//							continue;
//
//						// load combobox
//						if (j.value( )[ XorStr( "value" ) ].empty( ) || !j.value( )[ XorStr( "value" ) ].is_number_integer( ))
//							continue;
//
//						LuaImplementation::ComboMenuItem_t& combo = reinterpret_cast< LuaImplementation::ComboMenuItem_t& >( menu_item );
//						combo.iValue = j.value( )[ XorStr( "value" ) ].get< int >( );
//					}
//					else if (menu_item.iType == MENUITEM_MULTICOMBOBOX) {
//						if (j.value( )[ XorStr( "type" ) ].get< std::string >( ) != XorStr( "multi_combobox" ))
//							continue;
//
//						// load combobox
//						if (j.value( )[ XorStr( "values" ) ].empty( ))
//							continue;
//
//						LuaImplementation::MultiComboMenuItem_t& multi_combo = reinterpret_cast< LuaImplementation::MultiComboMenuItem_t& >( menu_item );
//						for (auto& k : j.value( )[ XorStr( "values" ) ].items( )) {
//							if (k.key( ).empty( ) || !k.value( ).is_boolean( ))
//								continue;
//
//							multi_combo.SetByLabel( k.key( ), k.value( ).get< bool >( ) );
//						}
//					}
//					else if (menu_item.iType == MENUITEM_COLORPICKER) {
//						if (j.value( )[ XorStr( "type" ) ].get< std::string >( ) != XorStr( "colorpicker" ))
//							continue;
//
//						// load colorpicker
//						if (j.value( )[ XorStr( "value" ) ].empty( ) || !j.value( )[ XorStr( "value" ) ].is_number_unsigned( ))
//							continue;
//
//						LuaImplementation::ColorPickerMenuItem_t& color_picker = reinterpret_cast< LuaImplementation::ColorPickerMenuItem_t& >( menu_item );
//
//						/*ImU32*/ unsigned int u32col = j.value( )[ XorStr( "value" ) ].get< /*ImU32*/ unsigned int >( );
//						color_picker.cValue = color::FromU32( u32col );
//					}
//				}
//			}
//		}
//	}
//
//	// unload not loaded scripts
//	for (auto& script : m_vecScriptInfos) {
//		bool bLoaded = script.bLoaded;
//		std::string szName = script.szName;
//
//		if (!bLoaded) {
//			continue;
//		}
//
//		if (std::find( vecLoadedScripts.begin( ), vecLoadedScripts.end( ), szName ) == vecLoadedScripts.end( )) {
//			script.Unload( );
//		}
//	}
//}

bool LuaImplementation::ScriptInfo_t::Load( )
{
	if (bLoaded) {
		return true;
	}

	// delete everything
	this->Destroy( );

	// make lua state fresh
	auto& lua_state = LuaImplementation::lua;

	bool bSucceded = true;

	//sol::state_view state( luaState );
	//state.script_file( path, []( lua_State* me, sol::protected_function_result result ) {

	try {
		auto ret = lua_state.do_file( this->szPath );
		if (!ret.valid( ))
		{
			sol::error err = ret;
			throw err;
		}
	}
	catch (sol::error& ex)
	{
		PrintError( ex.what( ) );
		this->Destroy( );
		bSucceded = false;
	}
	catch (...) {
		PrintError( XorStr( "unknown" ) );
		this->Destroy( );
		bSucceded = false;
	}

	bLoaded = bSucceded;

	return true;
}

void LuaImplementation::ScriptInfo_t::Unload( )
{
	if (!bLoaded) {
		bShouldUnload = true;
		return;
	}

	// on unload callback
	this->RunCallback( LuaImplementation::vecCallbackList[ LuaImplementation::CALLBACK_ON_UNLOAD ] );

	// clear stuff
	this->vecCallbacks.clear( );
	this->vecFonts.clear( );
	this->vecMenuItems.clear( );

	bLoaded = false;
}
