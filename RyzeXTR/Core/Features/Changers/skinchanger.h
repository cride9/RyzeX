#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <codecvt>
#include <unordered_set>

#include "../../SDK/DataTyes/Color.h"
#include "../../Interface/Interfaces/IItemSystem.h"

struct EConItem_t
{
	struct EconomyItem_t
	{
		EconomyItem_t( int entity_quality_index, int paint_kit_index, int stat_trak, float wear )
			: entity_quality_index( entity_quality_index ), paint_kit_index( paint_kit_index ), stat_trak( stat_trak ), wear( wear ) { }
		int entity_quality_index = 0;
		int paint_kit_index = 0;
		int stat_trak = 0;
		float wear = FLT_MIN;
	};
};

struct SkinObject_t
{
	SkinObject_t( const char* szName, const char* szModel, const char* szKillIcon = nullptr )
		: szName( szName ), szModel( szModel ), szKillIcon( szKillIcon ) { }

	const char* szName = nullptr;
	const char* szModel = nullptr;
	const char* szKillIcon = nullptr;
};

struct AgentObject_t
{
	AgentObject_t( std::string szModelName, std::string szDisplayName )
		: szModelName( szModelName ), szDisplayName( szDisplayName ) {}

	std::string szModelName = "";
	std::string szDisplayName = "";
};

struct WeaponInfo_t
{
	constexpr WeaponInfo_t( const char* model, const char* icon = nullptr ) :
		model( model ),
		icon( icon )
	{}

	const char* model;
	const char* icon;
};

struct SkinKit_t
{
	int m_nID;
	std::string m_szName;
	std::string m_szImagePath;
	int m_nRarity;
	int m_iWeaponID;

	bool operator < ( const SkinKit_t& other ) const { return ( m_szName < other.m_szName ); }
};

struct SkinColors_t
{
	Color m_colColor[ 5 ];
};

namespace SkinChanger 
{
	void Dump( );

	IItemSchema* m_pItemSchematic;

	/*containers*/
	std::vector<SkinKit_t> SkinKits;
	std::vector<SkinKit_t> GloveKits;
	std::vector<SkinKit_t> StickerKits;
	std::vector<SkinColors_t> SkinColors;
}


//struct weapon_name_t
//{
//	constexpr weapon_name_t(int32_t definition_index, const char* name) :
//		definition_index(definition_index),
//		name(name) {
//	}
//
//	int32_t definition_index = 0;
//	const char* name = nullptr;
//};
//
//struct weaponinfo_t
//{
//	constexpr weaponinfo_t(const char* model, const char* icon = nullptr) :
//		model(model),
//		icon(icon)
//	{}
//
//	const char* model;
//	const char* icon;
//};
//class c_skins
//{
//public:
//
//	struct skin_info_t
//	{
//		int seed = -1;
//		int paintkit;
//		int rarity;
//		std::string tag_name;
//		std::string shortname;
//		std::string name;
//	};
//
//	std::unordered_map<std::string, std::set<std::string>> weapon_skins;
//	std::unordered_map<std::string, skin_info_t> skin_map;
//	std::unordered_map<std::string, std::string> skin_names;
//
//	int weapon_index_skins = 7;
//	int weapon_vector_index_skins = 0;
//
//	struct item_setting
//	{
//		char name[32] = "";
//		int definition_vector_index = 0;
//		int definition_index = 1;
//		int paint_kit_vector_index = 0;
//		int paint_kit_index = 0;
//		int definition_override_vector_index = 0;
//		int definition_override_index = 0;
//		int seed = 0;
//		bool enable_stat_track = false;
//		int stat_trak = 0;
//		float wear = 0.0f;
//		float custom_paint_kit_x[3];
//		float custom_paint_kit_y[3];
//		float custom_paint_kit_z[3];
//		float custom_paint_kit_w[3];
//	};
//
//
//	struct options_t
//	{
//		std::map<int, item_setting> items;
//		std::unordered_map<std::string, std::string> icon_overrides;
//	};
//
//	options_t options;
//
//	void run();
//	void agent_changer();
//	void update_skins();
//	void menu();
//	void check_update();
//	void override_hud_icon(i_game_event*);
//	int remap_knife_animations(short index, const int sequence);
//
//	bool first_time_render;
//	bool force_update;
//	const char* index;
//
//	const char* player_model_t[35] =
//	{
//		"none",
//		"getaway sally",
//		"number k",
//		"little kev",
//		"safecracker voltzmann",
//		"bloody darryl the strapped",
//		"sir bloody loudmouth darryl",
//		"sir bloody darryl royale",
//		"sir bloody skullhead darryl",
//		"sir bloody silent darryl",
//		"sir bloody miami darryl",
//		"street soldier",
//		"soldier",
//		"slingshot",
//		"enforcer",
//		"muhlik",
//		"shahmatw",
//		"osiris",
//		"ground rebel",
//		"the elite mr. muhlik",
//		"trapper",
//		"trapper aggressor",
//		"vypa sista of the revolution",
//		"col mangos dabisi",
//		"arno the overgrown",
//		"medium rare' crasswater",
//		"crasswater the forgotten",
//		"elite trapper solman",
//		"the doctor' romanov",
//		"blackwolf",
//		"maximus",
//		"dragomir",
//		"rezan the ready",
//		"rezan the redshirt",
//		"dragomir",
//	};
//
//	const char* player_model_ct[30] =
//	{
//		"none",
//		"davida 'goggles' fernandez",
//		"frank 'wet sox' baroud",
//		"lieutenant rex krikey",
//		"michael syfers",
//		"operator",
//		"special agent ava",
//		"markus delrow",
//		"sous-lieutenant medic",
//		"chem-haz capitaine",
//		"chef d'escadron rouchard",
//		"aspirant",
//		"officer jacques beltram",
//		"d squadron officer",
//		"b squadron officer",
//		"seal team 6 soldier",
//		"buckshot",
//		"commander ricksaw",
//		"blueberries' buckshot",
//		"3rd commando company",
//		"two times' mccoy",
//		"two times' mccoy",
//		"primeiro tenente",
//		"mae 'dead cold' jamison",
//		"1st lieutenant farlow",
//		"john 'van healen' kask",
//		"bio-haz specialist",
//		"sergeant bombson",
//		"chem-haz specialist",
//		"lieutenant 'tree hugger' farlow"
//	};
//
//	const char* player_model_index_ct[29] =
//	{
//		("models/player/custom_player/legacy/ctm_diver_varianta.mdl"), // Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman
//		("models/player/custom_player/legacy/ctm_diver_variantb.mdl"), // Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman
//		("models/player/custom_player/legacy/ctm_diver_variantc.mdl"), // Lieutenant Rex Krikey | SEAL Frogman
//		("models/player/custom_player/legacy/ctm_fbi_varianth.mdl"), // Michael Syfers | FBI Sniper
//		("models/player/custom_player/legacy/ctm_fbi_variantf.mdl"), // Operator | FBI SWAT
//		("models/player/custom_player/legacy/ctm_fbi_variantb.mdl"), // Special Agent Ava | FBI
//		("models/player/custom_player/legacy/ctm_fbi_variantg.mdl"), // Markus Delrow | FBI HRT
//		("models/player/custom_player/legacy/ctm_gendarmerie_varianta.mdl"), // Sous-Lieutenant Medic | Gendarmerie Nationale
//		("models/player/custom_player/legacy/ctm_gendarmerie_variantb.mdl"), // Chem-Haz Capitaine | Gendarmerie Nationale
//		("models/player/custom_player/legacy/ctm_gendarmerie_variantc.mdl"), // Chef d'Escadron Rouchard | Gendarmerie Nationale
//		("models/player/custom_player/legacy/ctm_gendarmerie_variantd.mdl"), // Aspirant | Gendarmerie Nationale
//		("models/player/custom_player/legacy/ctm_gendarmerie_variante.mdl"), // Officer Jacques Beltram | Gendarmerie Nationale
//		("models/player/custom_player/legacy/ctm_sas_variantg.mdl"), // D Squadron Officer | NZSAS
//		("models/player/custom_player/legacy/ctm_sas_variantf.mdl"), // B Squadron Officer | SAS
//		("models/player/custom_player/legacy/ctm_st6_variante.mdl"), // Seal Team 6 Soldier | NSWC SEAL
//		("models/player/custom_player/legacy/ctm_st6_variantg.mdl"), // Buckshot | NSWC SEAL
//		("models/player/custom_player/legacy/ctm_st6_varianti.mdl"), // Lt. Commander Ricksaw | NSWC SEAL
//		("models/player/custom_player/legacy/ctm_st6_variantj.mdl"), // 'Blueberries' Buckshot | NSWC SEAL
//		("models/player/custom_player/legacy/ctm_st6_variantk.mdl"), // 3rd Commando Company | KSK
//		("models/player/custom_player/legacy/ctm_st6_variantl.mdl"), // 'Two Times' McCoy | TACP Cavalry
//		("models/player/custom_player/legacy/ctm_st6_variantm.mdl"), // 'Two Times' McCoy | USAF TACP
//		("models/player/custom_player/legacy/ctm_st6_variantn.mdl"), // Primeiro Tenente | Brazilian 1st Battalion
//		("models/player/custom_player/legacy/ctm_swat_variante.mdl"), // Cmdr. Mae 'Dead Cold' Jamison | SWAT
//		("models/player/custom_player/legacy/ctm_swat_variantf.mdl"), // 1st Lieutenant Farlow | SWAT
//		("models/player/custom_player/legacy/ctm_swat_variantg.mdl"), // John 'Van Healen' Kask | SWAT
//		("models/player/custom_player/legacy/ctm_swat_varianth.mdl"), // Bio-Haz Specialist | SWAT
//		("models/player/custom_player/legacy/ctm_swat_varianti.mdl"), // Sergeant Bombson | SWAT
//		("models/player/custom_player/legacy/ctm_swat_variantj.mdl"), // Chem-Haz Specialist | SWAT
//		("models/player/custom_player/legacy/ctm_swat_variantk.mdl") // Lieutenant 'Tree Hugger' Farlow | SWAT
//	};
//
//	const char* player_model_index_t[34] =
//	{
//		("models/player/custom_player/legacy/tm_professional_varj.mdl"), // Getaway Sally | The Professionals
//		("models/player/custom_player/legacy/tm_professional_vari.mdl"), // Number K | The Professionals
//		("models/player/custom_player/legacy/tm_professional_varh.mdl"), // Little Kev | The Professionals
//		("models/player/custom_player/legacy/tm_professional_varg.mdl"), // Safecracker Voltzmann | The Professionals
//		("models/player/custom_player/legacy/tm_professional_varf5.mdl"), // Bloody Darryl The Strapped | The Professionals
//		("models/player/custom_player/legacy/tm_professional_varf4.mdl"), // Sir Bloody Loudmouth Darryl | The Professionals
//		("models/player/custom_player/legacy/tm_professional_varf3.mdl"), // Sir Bloody Darryl Royale | The Professionals
//		("models/player/custom_player/legacy/tm_professional_varf2.mdl"), // Sir Bloody Skullhead Darryl | The Professionals
//		("models/player/custom_player/legacy/tm_professional_varf1.mdl"), // Sir Bloody Silent Darryl | The Professionals
//		("models/player/custom_player/legacy/tm_professional_varf.mdl"), // Sir Bloody Miami Darryl | The Professionals
//		("models/player/custom_player/legacy/tm_phoenix_varianti.mdl"), // Street Soldier | Phoenix
//		("models/player/custom_player/legacy/tm_phoenix_varianth.mdl"), // Soldier | Phoenix
//		("models/player/custom_player/legacy/tm_phoenix_variantg.mdl"), // Slingshot | Phoenix
//		("models/player/custom_player/legacy/tm_phoenix_variantf.mdl"), // Enforcer | Phoenix
//		("models/player/custom_player/legacy/tm_leet_variantj.mdl"), // Mr. Muhlik | Elite Crew
//		("models/player/custom_player/legacy/tm_leet_varianti.mdl"), // Prof. Shahmat | Elite Crew
//		("models/player/custom_player/legacy/tm_leet_varianth.mdl"), // Osiris | Elite Crew
//		("models/player/custom_player/legacy/tm_leet_variantg.mdl"), // Ground Rebel | Elite Crew
//		("models/player/custom_player/legacy/tm_leet_variantf.mdl"), // The Elite Mr. Muhlik | Elite Crew
//		("models/player/custom_player/legacy/tm_jungle_raider_variantf2.mdl"), // Trapper | Guerrilla Warfare
//		("models/player/custom_player/legacy/tm_jungle_raider_variantf.mdl"), // Trapper Aggressor | Guerrilla Warfare
//		("models/player/custom_player/legacy/tm_jungle_raider_variante.mdl"), // Vypa Sista of the Revolution | Guerrilla Warfare
//		("models/player/custom_player/legacy/tm_jungle_raider_variantd.mdl"), // Col. Mangos Dabisi | Guerrilla Warfare
//		("models/player/custom_player/legacy/tm_jungle_raider_variantñ.mdl"), // Arno The Overgrown | Guerrilla Warfare
//		("models/player/custom_player/legacy/tm_jungle_raider_variantb2.mdl"), // 'Medium Rare' Crasswater | Guerrilla Warfare
//		("models/player/custom_player/legacy/tm_jungle_raider_variantb.mdl"), // Crasswater The Forgotten | Guerrilla Warfare
//		("models/player/custom_player/legacy/tm_jungle_raider_varianta.mdl"), // Elite Trapper Solman | Guerrilla Warfare
//		("models/player/custom_player/legacy/tm_balkan_varianth.mdl"), // 'The Doctor' Romanov | Sabre
//		("models/player/custom_player/legacy/tm_balkan_variantj.mdl"), // Blackwolf | Sabre
//		("models/player/custom_player/legacy/tm_balkan_varianti.mdl"), // Maximus | Sabre
//		("models/player/custom_player/legacy/tm_balkan_variantf.mdl"), // Dragomir | Sabre
//		("models/player/custom_player/legacy/tm_balkan_variantg.mdl"), // Rezan The Ready | Sabre
//		("models/player/custom_player/legacy/tm_balkan_variantk.mdl"), // Rezan the Redshirt | Sabre
//		("models/player/custom_player/legacy/tm_balkan_variantl.mdl"), // Dragomir | Sabre Footsoldier
//	};
//
//	bool backup_model = false;
//	std::unordered_map <std::string, int> model_indexes;
//	std::unordered_map <std::string, int> player_model_indexes;
//
//	const std::vector<weapon_name_t> knife_names =
//	{
//		{0, XOR("default")},
//		{weapon_bayonet, XOR("bayonet")},
//		{weapon_knife_css, XOR("classic knife")},
//		{weapon_knife_cord, XOR("cord knife")},
//		{weapon_knife_canis, XOR("canis knife")},
//		{weapon_knife_outdoor, XOR("outdoor knife")},
//		{weapon_knife_skeleton, XOR("skeleton knife")},
//		{weapon_knife_flip, XOR("flip knife")},
//		{weapon_knife_gut, XOR("gut knife")},
//		{weapon_knife_karambit, XOR("karambit")},
//		{weapon_knife_m9_bayonet, XOR("m9 bayonet")},
//		{weapon_knife_tactical, XOR("huntsman knife")},
//		{weapon_knife_falchion, XOR("falchion knife")},
//		{weapon_knife_survival_bowie, XOR("bowie knife")},
//		{weapon_knife_butterfly, XOR("butterfly knife")},
//		{weapon_knife_push, XOR("shadow daggers")},
//		{weapon_knife_ursus, XOR("ursus knife")},
//		{weapon_knife_gypsy_jackknife, XOR("navaja knife")},
//		{weapon_knife_stiletto, XOR("stiletto knife")},
//		{weapon_knife_widowmaker, XOR("talon knife")}
//	};
//
//	std::vector< weapon_name_t> weapon_names_full =
//	{
//		{ weapon_knife, ("Knife")},
//		{ glove_t_side, ("Glove")},
//		{ weapon_ak47, ("AK-47") },
//		{ weapon_aug, ("AUG") },
//		{ weapon_awp, ("AWP") },
//		{ weapon_cz75a, ("CZ75 Auto") },
//		{ weapon_deagle, ("Desert Eagle") },
//		{ weapon_elite, ("Dual Berettas") },
//		{ weapon_famas, ("FAMAS") },
//		{ weapon_fiveseven, ("Five-SeveN") },
//		{ weapon_g3sg1, ("G3SG1") },
//		{ weapon_galilar, ("Galil AR") },
//		{ weapon_glock, ("Glock-18") },
//		{ weapon_m249, ("M249") },
//		{ weapon_m4a1_silencer, ("M4A1-S") },
//		{ weapon_m4a1, ("M4A4") },
//		{ weapon_mac10, ("MAC-10") },
//		{ weapon_mag7, ("MAG-7") },
//		{ weapon_mp7, ("MP7") },
//		{ weapon_mp5sd, ("MP5") },
//		{ weapon_mp9, ("MP9") },
//		{ weapon_negev, ("Negev") },
//		{ weapon_nova, ("Nova") },
//		{ weapon_hkp2000, ("P2000") },
//		{ weapon_p250, ("P250") },
//		{ weapon_p90, ("P90") },
//		{ weapon_bizon, ("PP-Bizon") },
//		{ weapon_revolver, ("R8 Revolver") },
//		{ weapon_sawedoff, ("Sawed-Off") },
//		{ weapon_scar20, ("SCAR-20") },
//		{ weapon_ssg08, ("SSG 08") },
//		{ weapon_sg556, ("SG 553") },
//		{ weapon_tec9, ("Tec-9") },
//		{ weapon_ump45, ("UMP-45") },
//		{ weapon_usp_silencer, ("USP-S") },
//		{ weapon_xm1014, ("XM1014") },
//	};
//
//	const char* glovemodels[9] = { "default","bloodhound","sport","driver","hand wraps","motorcycle","specialist","hydra", "broken fang" };
//
//	const char* glove_skinz_blood[4] =
//	{
//		"charred",
//		"snakebite",
//		"bronzed",
//		"guerilla"
//	};
//
//	const char* glove_skinz_sport[8] =
//	{
//		"hedge maze",
//		"pandoras box",
//		"superconductor",
//		"arid",
//		"vice",
//		"omega",
//		"amphibious",
//		"bronze morph"
//	};
//
//	const char* glove_skinz_driver[8] =
//	{
//		"lunar weave",
//		"convoy",
//		"crimson weave",
//		"diamondback",
//		"overtake",
//		"racing green",
//		"king snake",
//		"imperial plaid"
//	};
//
//	const char* glove_skinz_handwar[8] =
//	{
//		"leather",
//		"spruce ddpat",
//		"slaughter",
//		"badlands",
//		"cobalt skulls",
//		"overprint",
//		"duct tape",
//		"arboreal"
//	};
//
//	const char* glove_skinz_motor[8] =
//	{
//		"eclipse",
//		"spearmint",
//		"boom",
//		"cool mint",
//		"turtle",
//		"transport",
//		"polygon",
//		"pow"
//	};
//
//	const char* glove_skinz_specialist[8] =
//	{
//		"forest ddpat",
//		"crimson kimono",
//		"emerald web",
//		"foundation",
//		"crimson web",
//		"buckshot",
//		"fade",
//		"mogul"
//	};
//
//	const char* glove_skinz_hydra[4] =
//	{
//		"emerald",
//		"mangrove",
//		"rattler",
//		"case hardened"
//	};
//
//	const char* glove_skinz_fang[4] =
//	{
//		"jade",
//		"needle poin",
//		"unhinged",
//		"yellow-banded"
//	};
//
//	__forceinline const char* get_weapon_name_by_id(int id)
//	{
//		switch (id)
//		{
//		case 1:
//			return XOR("deagle");
//		case 2:
//			return XOR("elite");
//		case 3:
//			return XOR("fiveseven");
//		case 4:
//			return XOR("glock");
//		case 7:
//			return XOR("ak47");
//		case 8:
//			return XOR("aug");
//		case 9:
//			return XOR("awp");
//		case 10:
//			return XOR("famas");
//		case 11:
//			return XOR("g3sg1");
//		case 13:
//			return XOR("galilar");
//		case 14:
//			return XOR("m249");
//		case 60:
//			return XOR("m4a1_silencer");
//		case 16:
//			return XOR("m4a1");
//		case 17:
//			return XOR("mac10");
//		case 19:
//			return XOR("p90");
//		case 23:
//			return XOR("mp5sd");
//		case 24:
//			return XOR("ump45");
//		case 25:
//			return XOR("xm1014");
//		case 26:
//			return XOR("bizon");
//		case 27:
//			return XOR("mag7");
//		case 28:
//			return XOR("negev");
//		case 29:
//			return XOR("sawedoff");
//		case 30:
//			return XOR("tec9");
//		case 32:
//			return XOR("hkp2000");
//		case 33:
//			return XOR("mp7");
//		case 34:
//			return XOR("mp9");
//		case 35:
//			return XOR("nova");
//		case 36:
//			return XOR("p250");
//		case 38:
//			return XOR("scar20");
//		case 39:
//			return XOR("sg556");
//		case 40:
//			return XOR("ssg08");
//		case 61:
//			return XOR("usp_silencer");
//		case 63:
//			return XOR("cz75a");
//		case 64:
//			return XOR("revolver");
//		case 508:
//			return XOR("knife_m9_bayonet");
//		case 500:
//			return XOR("bayonet");
//		case 505:
//			return XOR("knife_flip");
//		case 506:
//			return XOR("knife_gut");
//		case 507:
//			return XOR("knife_karambit");
//		case 509:
//			return XOR("knife_tactical");
//		case 512:
//			return XOR("knife_falchion");
//		case 514:
//			return XOR("knife_survival_bowie");
//		case 515:
//			return XOR("knife_butterfly");
//		case 516:
//			return XOR("knife_push");
//
//		case 519:
//			return XOR("knife_ursus");
//		case 520:
//			return XOR("knife_gypsy_jackknife");
//		case 522:
//			return XOR("knife_stiletto");
//		case 523:
//			return XOR("knife_widowmaker");
//
//		case weapon_knife_css:
//			return XOR("knife_css");
//
//		case weapon_knife_cord:
//			return XOR("knife_cord");
//
//		case weapon_knife_canis:
//			return XOR("knife_canis");
//
//		case weapon_knife_outdoor:
//			return XOR("knife_outdoor");
//
//		case weapon_knife_skeleton:
//			return XOR("knife_skeleton");
//
//		case 5027:
//			return XOR("studded_bloodhound_gloves");
//		case 5028:
//			return XOR("t_gloves");
//		case 5029:
//			return XOR("ct_gloves");
//		case 5030:
//			return XOR("sporty_gloves");
//		case 5031:
//			return XOR("slick_gloves");
//		case 5032:
//			return XOR("leather_handwraps");
//		case 5033:
//			return XOR("motorcycle_gloves");
//		case 5034:
//			return XOR("specialist_gloves");
//		case 5035:
//			return XOR("studded_hydra_gloves");
//
//		default:
//			return XOR("");
//		}
//	}
//
//	__forceinline static auto is_knife(const int i) -> bool
//	{
//		return (i >= weapon_bayonet && i < glove_studded_bloodhound) || i == weapon_knife_t || i == weapon_knife;
//	}
//
//	template <typename T>
//	__forceinline void get_current_weapon(int* idx, int* vec_idx, std::vector<T> arr)
//	{
//		const auto local = g_client.local;
//
//		if (!local)
//			return;
//
//		if (!g_interfaces.engine->is_in_game())
//			return;
//
//		auto weapon = local->get_active_weapon();
//
//		if (!weapon)
//			return;
//		short wpn_idx = weapon->get_item_definition_index();
//
//		if (is_knife(wpn_idx))
//		{
//			*idx = weapon_names_full.at(0).definition_index;
//			*vec_idx = 0;
//			return;
//		}
//
//		auto wpn_it = std::find_if(arr.begin(), arr.end(), [wpn_idx](const T& a)
//			{
//				return a.definition_index == wpn_idx;
//			});
//
//		if (wpn_it != arr.end())
//		{
//			*idx = wpn_idx;
//			*vec_idx = std::abs(std::distance(arr.begin(), wpn_it));
//		}
//	}
//
//	__forceinline static auto create_wearable() -> createclientclass_t
//	{
//		auto client_class = g_interfaces.client->get_all_classes();
//
//		for (client_class = g_interfaces.client->get_all_classes(); client_class; client_class = client_class->next)
//		{
//			if (client_class->classid == econwearable)
//				return client_class->create;
//		}
//	}
//
//	__forceinline static auto make_glove(int entry, int serial) -> base_attributable_item_t*
//	{
//		static auto create_wearable_fn = create_wearable();
//		create_wearable_fn(entry, serial);
//
//		const auto glove = reinterpret_cast<base_attributable_item_t*>(g_interfaces.entitylist->get_client_entity(entry));
//		assert(glove); {
//			static auto set_abs_origin_addr = (size_t)g_pattern.find(g_modules.client_dll, XOR("55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8"));
//			const auto set_abs_origin_fn = reinterpret_cast<void(__thiscall*)(void*, const std::array<float, 3>&)>(set_abs_origin_addr);
//			static constexpr std::array<float, 3> new_pos = { 10000.f, 10000.f, 10000.f };
//			set_abs_origin_fn(glove, new_pos);
//		}
//		return glove;
//	}
//
//	__forceinline int get_glove_skin(int gloveIndex, int skinIndex)
//	{
//		if (gloveIndex == 0) // bloudhound
//		{
//			switch (skinIndex)
//			{
//			case 0:
//				return 10006;
//			case 1:
//				return 10007;
//			case 2:
//				return 10008;
//			case 3:
//				return 10039;
//			default:
//				return 0;
//			}
//		}
//		else if (gloveIndex == 1) // Sport
//		{
//			switch (skinIndex)
//			{
//			case 0:
//				return 10038;
//			case 1:
//				return 10037;
//			case 2:
//				return 10018;
//			case 3:
//				return 10019;
//			case 4:
//				return 10048;
//			case 5:
//				return 10047;
//			case 6:
//				return 10045;
//			case 7:
//				return 10046;
//			default:
//				return 0;
//			}
//		}
//		else if (gloveIndex == 2) // Driver
//		{
//			switch (skinIndex)
//			{
//			case 0:
//				return 10013;
//			case 1:
//				return 10015;
//			case 2:
//				return 10016;
//			case 3:
//				return 10040;
//			case 4:
//				return 10043;
//			case 5:
//				return 10044;
//			case 6:
//				return 10041;
//			case 7:
//				return 10042;
//			default:
//				return 0;
//			}
//		}
//		else if (gloveIndex == 3) // Wraps
//		{
//			switch (skinIndex)
//			{
//			case 0:
//				return 10009;
//			case 1:
//				return 10010;
//			case 2:
//				return 10021;
//			case 3:
//				return 10036;
//			case 4:
//				return 10053;
//			case 5:
//				return 10054;
//			case 6:
//				return 10055;
//			case 7:
//				return 10056;
//			default:
//				return 0;
//			}
//		}
//		else if (gloveIndex == 4) // Moto
//		{
//			switch (skinIndex)
//			{
//			case 0:
//				return 10024;
//			case 1:
//				return 10026;
//			case 2:
//				return 10027;
//			case 3:
//				return 10028;
//			case 4:
//				return 10050;
//			case 5:
//				return 10051;
//			case 6:
//				return 10052;
//			case 7:
//				return 10049;
//
//			default:
//				return 0;
//			}
//		}
//		else if (gloveIndex == 5) // Specialist
//		{
//			switch (skinIndex)
//			{
//			case 0:
//				return 10030;
//			case 1:
//				return 10033;
//			case 2:
//				return 10034;
//			case 3:
//				return 10035;
//			case 4:
//				return 10061;
//			case 5:
//				return 10062;
//			case 6:
//				return 10063;
//			case 7:
//				return 10064;
//			default:
//				return 0;
//			}
//		}
//		else if (gloveIndex == 6)
//		{
//			switch (skinIndex)
//			{
//			case 0:
//				return 10057;
//			case 1:
//				return 10058;
//			case 2:
//				return 10059;
//			case 3:
//				return 10060;
//			}
//		}
//		else if (gloveIndex == 7)
//		{
//			switch (skinIndex)
//			{
//			case 0:
//				return 10085;
//			case 1:
//				return 10087;
//			case 2:
//				return 10088;
//			case 3:
//				return 10086;
//			}
//		}
//		else
//			return 0;
//		return 0;
//	}
//
//	bool apply_glove_model(base_attributable_item_t*, const char*);
//	bool apply_glove_skin(base_attributable_item_t*, int, int, int, int, int, float);
//	void glove_changer();
//
//};
//
//inline c_skins g_skins;