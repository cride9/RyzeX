#pragma once
#include "../../SDK/Entity.h"
#include "SkinChanger.h"
#include "../../SDK/Menu/config.h"
#include "../Rage/ragebot.h"

namespace beforeIfuckUpEverything {

	enum ESequence
	{
		SEQUENCE_DEFAULT_DRAW = 0,
		SEQUENCE_DEFAULT_IDLE1 = 1,
		SEQUENCE_DEFAULT_IDLE2 = 2,
		SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
		SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
		SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
		SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
		SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
		SEQUENCE_DEFAULT_LOOKAT01 = 12,

		SEQUENCE_BUTTERFLY_DRAW = 0,
		SEQUENCE_BUTTERFLY_DRAW2 = 1,
		SEQUENCE_BUTTERFLY_LOOKAT01 = 13,
		SEQUENCE_BUTTERFLY_LOOKAT03 = 15,

		SEQUENCE_FALCHION_IDLE1 = 1,
		SEQUENCE_FALCHION_HEAVY_MISS1 = 8,
		SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP = 9,
		SEQUENCE_FALCHION_LOOKAT01 = 12,
		SEQUENCE_FALCHION_LOOKAT02 = 13,

		SEQUENCE_CSS_LOOKAT01 = 14,
		SEQUENCE_CSS_LOOKAT02 = 15,

		SEQUENCE_DAGGERS_IDLE1 = 1,
		SEQUENCE_DAGGERS_LIGHT_MISS1 = 2,
		SEQUENCE_DAGGERS_LIGHT_MISS5 = 6,
		SEQUENCE_DAGGERS_HEAVY_MISS2 = 11,
		SEQUENCE_DAGGERS_HEAVY_MISS1 = 12,

		SEQUENCE_BOWIE_IDLE1 = 1,
	};

	inline int RandomSequence(int low, int high)
	{
		return rand() % (high - low + 1) + low;
	}

	inline auto GetNewAnimation( const FNV1A::Hash model, const int sequence ) -> int
	{
		switch ( model )
		{
		case FNV( "models/weapons/v_knife_butterfly.mdl" ):
		{
			switch ( sequence )
			{
			case SEQUENCE_DEFAULT_DRAW:
				return RandomSequence( SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2 );
			case SEQUENCE_DEFAULT_LOOKAT01:
				return RandomSequence( SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03 );
			default:
				return sequence + 1;
			}
		}
		case FNV( "models/weapons/v_knife_falchion_advanced.mdl" ):
		{
			switch ( sequence )
			{
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_FALCHION_IDLE1;
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
				return RandomSequence( SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP );
			case SEQUENCE_DEFAULT_LOOKAT01:
				return RandomSequence( SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02 );
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			default:
				return sequence - 1;
			}

		}
		case FNV( "models/weapons/v_knife_css.mdl" ):
		{
			switch ( sequence )
			{
			case SEQUENCE_DEFAULT_LOOKAT01:
				return RandomSequence( SEQUENCE_CSS_LOOKAT01, SEQUENCE_CSS_LOOKAT02 );
			default:
				return sequence;
			}
		}
		case FNV( "models/weapons/v_knife_push.mdl" ):
		{
			switch ( sequence )
			{
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_DAGGERS_IDLE1;
			case SEQUENCE_DEFAULT_LIGHT_MISS1:
			case SEQUENCE_DEFAULT_LIGHT_MISS2:
				return RandomSequence( SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5 );
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
				return RandomSequence( SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1 );
			case SEQUENCE_DEFAULT_HEAVY_HIT1:
			case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
			case SEQUENCE_DEFAULT_LOOKAT01:
				return sequence + 3;
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			default:
				return sequence + 2;
			}
		}
		case FNV( "models/weapons/v_knife_survival_bowie.mdl" ):
		{
			switch ( sequence )
			{
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_BOWIE_IDLE1;
			default:
				return sequence - 1;
			}
		}
		case FNV( "models/weapons/v_knife_ursus.mdl" ):
		case FNV( "models/weapons/v_knife_cord.mdl" ):
		case FNV( "models/weapons/v_knife_canis.mdl" ):
		case FNV( "models/weapons/v_knife_outdoor.mdl" ):
		case FNV( "models/weapons/v_knife_skeleton.mdl" ):
		{
			switch ( sequence )
			{
			case SEQUENCE_DEFAULT_DRAW:
				return RandomSequence( SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2 );
			case SEQUENCE_DEFAULT_LOOKAT01:
				return RandomSequence( SEQUENCE_BUTTERFLY_LOOKAT01, 14 );
			default:
				return sequence + 1;
			}
		}
		case FNV( "models/weapons/v_knife_stiletto.mdl" ):
		{
			switch ( sequence )
			{
			case SEQUENCE_DEFAULT_LOOKAT01:
				return RandomSequence( 12, 13 );
			default:
				return sequence;
			}
		}
		case FNV( "models/weapons/v_knife_widowmaker.mdl" ):
		{
			switch ( sequence )
			{
			case SEQUENCE_DEFAULT_LOOKAT01:
				return RandomSequence( 14, 15 );
			default:
				return sequence;
			}
		}
		default:
			return sequence;
		}
	}

	inline EItemDefinitionIndex whatTheFuckBackward(int value) {
		switch (value) {
		case 1:
			return WEAPON_DEAGLE;
		case 2:
			return WEAPON_ELITE;
		case 3:
			return WEAPON_FIVESEVEN;
		case 4:
			return WEAPON_GLOCK;
		case 5:
			return WEAPON_AK47;
		case 6:
			return WEAPON_AUG;
		case 7:
			return WEAPON_AWP;
		case 8:
			return WEAPON_FAMAS;
		case 9:
			return WEAPON_G3SG1;
		case 10:
			return WEAPON_GALILAR;
		case 11:
			return WEAPON_M249;
		case 12:
			return WEAPON_M4A1;
		case 13:
			return WEAPON_MAC10;
		case 14:
			return WEAPON_P90;
		case 15:
			return WEAPON_ZONE_REPULSOR;
		case 16:
			return WEAPON_MP5SD;
		case 17:
			return WEAPON_UMP45;
		case 18:
			return WEAPON_XM1014;
		case 19:
			return WEAPON_BIZON;
		case 20:
			return WEAPON_MAG7;
		case 21:
			return WEAPON_NEGEV;
		case 22:
			return WEAPON_SAWEDOFF;
		case 23:
			return WEAPON_TEC9;
		case 24:
			return WEAPON_TASER;
		case 25:
			return WEAPON_HKP2000;
		case 26:
			return WEAPON_MP7;
		case 27:
			return WEAPON_MP9;
		case 28:
			return WEAPON_NOVA;
		case 29:
			return WEAPON_P250;
		case 30:
			return WEAPON_SHIELD;
		case 31:
			return WEAPON_SCAR20;
		case 32:
			return WEAPON_SG556;
		case 33:
			return WEAPON_SSG08;
		case 34:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 35:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 36:
			return WEAPON_FLASHBANG;
		case 37:
			return WEAPON_HEGRENADE;
		case 38:
			return WEAPON_SMOKEGRENADE;
		case 39:
			return WEAPON_MOLOTOV;
		case 40:
			return WEAPON_DECOY;
		case 41:
			return WEAPON_INCGRENADE;
		case 42:
			return WEAPON_C4;
		case 43:
			return WEAPON_HEALTHSHOT;
		case 44:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 45:
			return WEAPON_M4A1_SILENCER;
		case 46:
			return WEAPON_USP_SILENCER;
		case 47:
			return WEAPON_CZ75A;
		case 48:
			return WEAPON_REVOLVER;
		case 49:
			return WEAPON_TAGRENADE;
		case 50:
			return WEAPON_FISTS;
		case 51:
			return WEAPON_BREACHCHARGE;
		case 52:
			return WEAPON_TABLET;
		case 53:
			return WEAPON_MELEE;
		case 54:
			return WEAPON_AXE;
		case 55:
			return WEAPON_HAMMER;
		case 56:
			return WEAPON_SPANNER;
		case 57:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 58:
			return WEAPON_FIREBOMB;
		case 59:
			return WEAPON_DIVERSION;
		case 60:
			return WEAPON_FRAG_GRENADE;
		case 61:
			return WEAPON_SNOWBALL;
		case 62:
			return WEAPON_BUMPMINE;
		case 63:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 64:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 65:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 66:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 67:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 68:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 69:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 70:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 71:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 72:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 73:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 74:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 75:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 76:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel); //
		case 77:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel);
		case 78:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel);
		case 79:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel);
		case 80:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel);
		case 81:
			return (EItemDefinitionIndex)skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel);
		case 82:
			return GLOVE_STUDDED_BROKENFANG;
		case 83:
			return GLOVE_STUDDED_BLOODHOUND;
		case 84:
			return GLOVE_T;
		case 85:
			return GLOVE_CT;
		case 86:
			return GLOVE_SPORTY;
		case 87:
			return GLOVE_SLICK;
		case 88:
			return GLOVE_LEATHER_HANDWRAPS;
		case 89:
			return GLOVE_MOTORCYCLE;
		case 90:
			return GLOVE_SPECIALIST;
		case 91:
			return GLOVE_STUDDED_HYDRA;
		default:
			return EItemDefinitionIndex(0);
		}
	}

	inline int whatTheFuck(EItemDefinitionIndex itemdefindex) {

		switch (itemdefindex) {
		case WEAPON_DEAGLE:
			return 1;
		case WEAPON_ELITE:
			return 2;
		case WEAPON_FIVESEVEN:
			return 3;
		case WEAPON_GLOCK:
			return 4;
		case WEAPON_AK47:
			return 5;
		case WEAPON_AUG:
			return 6;
		case WEAPON_AWP:
			return 7;
		case WEAPON_FAMAS:
			return 8;
		case WEAPON_G3SG1:
			return 9;
		case WEAPON_GALILAR:
			return 10;
		case WEAPON_M249:
			return 11;
		case WEAPON_M4A1:
			return 12;
		case WEAPON_MAC10:
			return 13;
		case WEAPON_P90:
			return 14;
		case WEAPON_ZONE_REPULSOR:
			return 15;
		case WEAPON_MP5SD:
			return 16;
		case WEAPON_UMP45:
			return 17;
		case WEAPON_XM1014:
			return 18;
		case WEAPON_BIZON:
			return 19;
		case WEAPON_MAG7:
			return 20;
		case WEAPON_NEGEV:
			return 21;
		case WEAPON_SAWEDOFF:
			return 22;
		case WEAPON_TEC9:
			return 23;
		case WEAPON_TASER:
			return 24;
		case WEAPON_HKP2000:
			return 25;
		case WEAPON_MP7:
			return 26;
		case WEAPON_MP9:
			return 27;
		case WEAPON_NOVA:
			return 28;
		case WEAPON_P250:
			return 29;
		case WEAPON_SHIELD:
			return 30;
		case WEAPON_SCAR20:
			return 31;
		case WEAPON_SG556:
			return 32;
		case WEAPON_SSG08:
			return 33;
		case WEAPON_KNIFE_GG:
			return 34;
		case WEAPON_KNIFE:
			return 35;
		case WEAPON_FLASHBANG:
			return 36;
		case WEAPON_HEGRENADE:
			return 37;
		case WEAPON_SMOKEGRENADE:
			return 38;
		case WEAPON_MOLOTOV:
			return 39;
		case WEAPON_DECOY:
			return 40;
		case WEAPON_INCGRENADE:
			return 41;
		case WEAPON_C4:
			return 42;
		case WEAPON_HEALTHSHOT:
			return 43;
		case WEAPON_KNIFE_T:
			return 44;
		case WEAPON_M4A1_SILENCER:
			return 45;
		case WEAPON_USP_SILENCER:
			return 46;
		case WEAPON_CZ75A:
			return 47;
		case WEAPON_REVOLVER:
			return 48;
		case WEAPON_TAGRENADE:
			return 49;
		case WEAPON_FISTS:
			return 50;
		case WEAPON_BREACHCHARGE:
			return 51;
		case WEAPON_TABLET:
			return 52;
		case WEAPON_MELEE:
			return 53;
		case WEAPON_AXE:
			return 54;
		case WEAPON_HAMMER:
			return 55;
		case WEAPON_SPANNER:
			return 56;
		case WEAPON_KNIFE_GHOST:
			return 57;
		case WEAPON_FIREBOMB:
			return 58;
		case WEAPON_DIVERSION:
			return 59;
		case WEAPON_FRAG_GRENADE:
			return 60;
		case WEAPON_SNOWBALL:
			return 61;
		case WEAPON_BUMPMINE:
			return 62;
		case WEAPON_KNIFE_BAYONET:
			return 63;
		case WEAPON_KNIFE_CSS:
			return 64;
		case WEAPON_KNIFE_FLIP:
			return 65;
		case WEAPON_KNIFE_GUT:
			return 66;
		case WEAPON_KNIFE_KARAMBIT:
			return 67;
		case WEAPON_KNIFE_M9_BAYONET:
			return 68;
		case WEAPON_KNIFE_TACTICAL:
			return 69;
		case WEAPON_KNIFE_FALCHION:
			return 70;
		case WEAPON_KNIFE_SURVIVAL_BOWIE:
			return 71;
		case WEAPON_KNIFE_BUTTERFLY:
			return 72;
		case WEAPON_KNIFE_PUSH:
			return 73;
		case WEAPON_KNIFE_CORD:
			return 74;
		case WEAPON_KNIFE_CANIS:
			return 75;
		case WEAPON_KNIFE_URSUS:
			return 76;
		case WEAPON_KNIFE_GYPSY_JACKKNIFE:
			return 77;
		case WEAPON_KNIFE_OUTDOOR:
			return 78;
		case WEAPON_KNIFE_STILETTO:
			return 79;
		case WEAPON_KNIFE_WIDOWMAKER:
			return 80;
		case WEAPON_KNIFE_SKELETON:
			return 81;
		case GLOVE_STUDDED_BROKENFANG:
			return 82;
		case GLOVE_STUDDED_BLOODHOUND:
			return 83;
		case GLOVE_T:
			return 84;
		case GLOVE_CT:
			return 85;
		case GLOVE_SPORTY:
			return 86;
		case GLOVE_SLICK:
			return 87;
		case GLOVE_LEATHER_HANDWRAPS:
			return 88;
		case GLOVE_MOTORCYCLE:
			return 89;
		case GLOVE_SPECIALIST:
			return 90;
		case GLOVE_STUDDED_HYDRA:
			return 91;
		default:
			return 0;
		}
	}

	inline int weaponInHand = 0;
	inline std::array<int, 92> iBackup{ 0 };

	inline void UpdateSkins() {

		g::bUpdatingSkins = true;
		//util::ForceFullUpdate();
		typedef void(*fn) (void);
		static fn update = (fn)MEM::FindPattern(ENGINE_DLL, XorStr("A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85"));
		//update();

		// lets update hud again to make sure user will have the correct skin name
		static bool applied_update_time = false;
		float update_time{};

		// apply the timer
		if (!update_time && !applied_update_time)
		{

			update_time = i::GlobalVars->flCurrentTime + 10.f;
			applied_update_time = true;
		}

		// check for the right time
		if (i::GlobalVars->flCurrentTime >= update_time)
		{
			// update the hud and exit from function
			update();
			update_time = 0;
			applied_update_time = false;
		}
		g::bUpdatingSkins = false;
	}

	inline bool ApplyKnifeModel(CBaseCombatWeapon* pWeapon)
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

		pViewmodel->GetModelIndex() = i::ModelInfo->GetModelIndex(mapItemList.at(skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel)).szModel);
		pWeapon->GetItemDefinitionIndex() = skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel);

		return true;
	}

	inline bool ApplyKnifeSkin(CBaseCombatWeapon* pWeapon)
	{
		if (g::pLocal == nullptr)
			return false;

		pWeapon->GetItemDefinitionIndex() = skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel);
		pWeapon->GetModelIndex() = i::ModelInfo->GetModelIndex(mapItemList.at(skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel)).szModel);

		CBaseHandle pWorldModelHandle = pWeapon->GetWorldModelHandle();
		if (!pWorldModelHandle)
			return false;

		CBaseCombatWeapon* pWorldModel = (CBaseCombatWeapon*)(i::EntityList->GetClientEntityFromHandle(pWorldModelHandle));
		if (!pWorldModel)
			return false;

		pWorldModel->GetModelIndex() = i::ModelInfo->GetModelIndex(mapItemList.at(skinChanger.GetKnifeDefinitionIndex(cfg::skin::iKnifeModel)).szModel) + 1;

		return true;
	}

	inline void SetSkin(CBaseEntity* pLocal) {

		if (!pLocal || !pLocal->GetWeapon() || !cfg::skin::bEnableSkinChagner || i::ClientState->iSignonState != SIGNONSTATE_FULL)
			return;

		auto pWeapon = pLocal->GetWeapon();

		if (pWeapon)
			weaponInHand = beforeIfuckUpEverything::whatTheFuck((EItemDefinitionIndex)pWeapon->GetItemDefinitionIndex());

		bool bUpdate = false;
		for (auto nIndex : pLocal->GetWeaponsHandle()) {

			CBaseCombatWeapon* pWeapon = static_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(nIndex));

			if (pWeapon == nullptr)
				continue;

			CCSWeaponData* pWeaponData = reinterpret_cast<CCSWeaponData*>(pWeapon->GetCSWpnData());

			if (pWeaponData == nullptr || (pWeaponData->nWeaponType == WEAPONTYPE_GRENADE) || (pWeaponData->nWeaponType == WEAPONTYPE_C4) || (pWeaponData->nWeaponType == WEAPONTYPE_FISTS) || (pWeaponData->nWeaponType == WEAPONTYPE_BREACHCHARGE) || (pWeaponData->nWeaponType == WEAPONTYPE_BUMPMINE) || (pWeaponData->nWeaponType == WEAPONTYPE_HEALTHSHOT) || (pWeaponData->nWeaponType == WEAPONTYPE_TABLET))
				continue;

			if (pWeapon->GetItemDefinitionIndex() == WEAPON_TASER)
				continue;

			if (pWeapon->GetClientClass()->nClassID == EClassIndex::CKnife && cfg::skin::iKnifeModel > 0)
				ApplyKnifeModel(pWeapon);

			if (pWeapon->GetClientClass()->nClassID == EClassIndex::CKnife && cfg::skin::iKnifeModel > 0)
				ApplyKnifeSkin(pWeapon);

			auto weaponIndexMenu = beforeIfuckUpEverything::whatTheFuck((EItemDefinitionIndex)pWeapon->GetItemDefinitionIndex());;

			int selectedIndex = skinChanger.SkinKits.at(cfg::skin::iSkinId[weaponIndexMenu]).m_nID;

			pWeapon->GetItemIDHigh() = -1;
			pWeapon->GetFallbackWear() = cfg::skin::flSkinWear[weaponIndexMenu];
			pWeapon->GetFallbackPaintKit() = selectedIndex;
			//pWeapon->GetFallbackStatTrak() = cfg::skin::iSkinStattrak[weaponIndexMenu];
			pWeapon->GetFallbackSeed() = cfg::skin::iFallbackSeed[weaponIndexMenu];
			//if (!cfg::skin::szSkinNametag[weaponIndexMenu].empty())
			//	strcpy(pWeapon->GetCustomName(), cfg::skin::szSkinNametag[weaponIndexMenu].c_str());

			if (iBackup[weaponIndexMenu] != selectedIndex) {
				bUpdate = true;
				iBackup[weaponIndexMenu] = selectedIndex;
			}
		}

		if (bUpdate) {
			UpdateSkins();
			bUpdate = false;
		}
	}
}