//#pragma once
//#include <map>
//#include "../../SDK/Entity.h"
//#include <set>
//#include "../../globals.h"
//
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
//	void override_hud_icon(IGameEvent*);
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
//		{0, ("default")},
//		{WEAPON_KNIFE_BAYONET, ("bayonet")},
//		{WEAPON_KNIFE_CSS, ("classic knife")},
//		{WEAPON_KNIFE_CORD, ("cord knife")},
//		{WEAPON_KNIFE_CANIS, ("canis knife")},
//		{WEAPON_KNIFE_OUTDOOR, ("outdoor knife")},
//		{WEAPON_KNIFE_SKELETON, ("skeleton knife")},
//		{WEAPON_KNIFE_FLIP, ("flip knife")},
//		{WEAPON_KNIFE_GUT, ("gut knife")},
//		{WEAPON_KNIFE_KARAMBIT, ("karambit")},
//		{WEAPON_KNIFE_M9_BAYONET, ("m9 bayonet")},
//		{WEAPON_KNIFE_TACTICAL, ("huntsman knife")},
//		{WEAPON_KNIFE_FALCHION, ("falchion knife")},
//		{WEAPON_KNIFE_SURVIVAL_BOWIE, ("bowie knife")},
//		{WEAPON_KNIFE_BUTTERFLY, ("butterfly knife")},
//		{WEAPON_KNIFE_PUSH, ("shadow daggers")},
//		{WEAPON_KNIFE_URSUS, ("ursus knife")},
//		{WEAPON_KNIFE_GYPSY_JACKKNIFE, ("navaja knife")},
//		{WEAPON_KNIFE_STILETTO, ("stiletto knife")},
//		{WEAPON_KNIFE_WIDOWMAKER, ("talon knife")}
//	};
//
//	std::vector< weapon_name_t> weapon_names_full =
//	{
//		{ WEAPON_KNIFE, ("Knife")},
//		{ GLOVE_T, ("Glove")},
//		{ WEAPON_AK47, ("AK-47") },
//		{ WEAPON_AUG, ("AUG") },
//		{ WEAPON_AWP, ("AWP") },
//		{ WEAPON_CZ75A, ("CZ75 Auto") },
//		{ WEAPON_DEAGLE, ("Desert Eagle") },
//		{ WEAPON_ELITE, ("Dual Berettas") },
//		{ WEAPON_FAMAS, ("FAMAS") },
//		{ WEAPON_FIVESEVEN, ("Five-SeveN") },
//		{ WEAPON_G3SG1, ("G3SG1") },
//		{ WEAPON_GALILAR, ("Galil AR") },
//		{ WEAPON_GLOCK, ("Glock-18") },
//		{ WEAPON_M249, ("M249") },
//		{ WEAPON_M4A1_SILENCER, ("M4A1-S") },
//		{ WEAPON_M4A1, ("M4A4") },
//		{ WEAPON_MAC10, ("MAC-10") },
//		{ WEAPON_MAG7, ("MAG-7") },
//		{ WEAPON_MP7, ("MP7") },
//		{ WEAPON_MP5SD, ("MP5") },
//		{ WEAPON_MP9, ("MP9") },
//		{ WEAPON_NEGEV, ("Negev") },
//		{ WEAPON_NOVA, ("Nova") },
//		{ WEAPON_HKP2000, ("P2000") },
//		{ WEAPON_P250, ("P250") },
//		{ WEAPON_P90, ("P90") },
//		{ WEAPON_BIZON, ("PP-Bizon") },
//		{ WEAPON_REVOLVER, ("R8 Revolver") },
//		{ WEAPON_SAWEDOFF, ("Sawed-Off") },
//		{ WEAPON_SCAR20, ("SCAR-20") },
//		{ WEAPON_SSG08, ("SSG 08") },
//		{ WEAPON_SG556, ("SG 553") },
//		{ WEAPON_TEC9, ("Tec-9") },
//		{ WEAPON_UMP45, ("UMP-45") },
//		{ WEAPON_USP_SILENCER, ("USP-S") },
//		{ WEAPON_XM1014, ("XM1014") },
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
//			return ("deagle");
//		case 2:
//			return ("elite");
//		case 3:
//			return ("fiveseven");
//		case 4:
//			return ("glock");
//		case 7:
//			return ("ak47");
//		case 8:
//			return ("aug");
//		case 9:
//			return ("awp");
//		case 10:
//			return ("famas");
//		case 11:
//			return ("g3sg1");
//		case 13:
//			return ("galilar");
//		case 14:
//			return ("m249");
//		case 60:
//			return ("m4a1_silencer");
//		case 16:
//			return ("m4a1");
//		case 17:
//			return ("mac10");
//		case 19:
//			return ("p90");
//		case 23:
//			return ("mp5sd");
//		case 24:
//			return ("ump45");
//		case 25:
//			return ("xm1014");
//		case 26:
//			return ("bizon");
//		case 27:
//			return ("mag7");
//		case 28:
//			return ("negev");
//		case 29:
//			return ("sawedoff");
//		case 30:
//			return ("tec9");
//		case 32:
//			return ("hkp2000");
//		case 33:
//			return ("mp7");
//		case 34:
//			return ("mp9");
//		case 35:
//			return ("nova");
//		case 36:
//			return ("p250");
//		case 38:
//			return ("scar20");
//		case 39:
//			return ("sg556");
//		case 40:
//			return ("ssg08");
//		case 61:
//			return ("usp_silencer");
//		case 63:
//			return ("cz75a");
//		case 64:
//			return ("revolver");
//		case 508:
//			return ("knife_m9_bayonet");
//		case 500:
//			return ("bayonet");
//		case 505:
//			return ("knife_flip");
//		case 506:
//			return ("knife_gut");
//		case 507:
//			return ("knife_karambit");
//		case 509:
//			return ("knife_tactical");
//		case 512:
//			return ("knife_falchion");
//		case 514:
//			return ("knife_survival_bowie");
//		case 515:
//			return ("knife_butterfly");
//		case 516:
//			return ("knife_push");
//
//		case 519:
//			return ("knife_ursus");
//		case 520:
//			return ("knife_gypsy_jackknife");
//		case 522:
//			return ("knife_stiletto");
//		case 523:
//			return ("knife_widowmaker");
//
//		case WEAPON_KNIFE_CSS:
//			return ("knife_css");
//
//		case WEAPON_KNIFE_CORD:
//			return ("knife_cord");
//
//		case WEAPON_KNIFE_CANIS:
//			return ("knife_canis");
//
//		case WEAPON_KNIFE_OUTDOOR:
//			return ("knife_outdoor");
//
//		case WEAPON_KNIFE_SKELETON:
//			return ("knife_skeleton");
//
//		case 5027:
//			return ("studded_bloodhound_gloves");
//		case 5028:
//			return ("t_gloves");
//		case 5029:
//			return ("ct_gloves");
//		case 5030:
//			return ("sporty_gloves");
//		case 5031:
//			return ("slick_gloves");
//		case 5032:
//			return ("leather_handwraps");
//		case 5033:
//			return ("motorcycle_gloves");
//		case 5034:
//			return ("specialist_gloves");
//		case 5035:
//			return ("studded_hydra_gloves");
//
//		default:
//			return ("");
//		}
//	}
//
//	__forceinline static auto is_knife(const int i) -> bool
//	{
//		return (i >= WEAPON_KNIFE_BAYONET && i < GLOVE_STUDDED_BLOODHOUND) || i == WEAPON_KNIFE_T || i == WEAPON_KNIFE;
//	}
//
//	template <typename T>
//	__forceinline void get_current_weapon(int* idx, int* vec_idx, std::vector<T> arr)
//	{
//		const auto local = g::pLocal;
//
//		if (!local)
//			return;
//
//		if (!i::EngineClient->IsInGame())
//			return;
//
//		auto weapon = local->GetWeapon();
//
//		if (!weapon)
//			return;
//		short wpn_idx = weapon->GetItemDefinitionIndex();
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
//	__forceinline static auto create_wearable() -> CreateClientClassFn
//	{
//		auto client_class = i::ClientDll->GetAllClasses();
//
//		for (client_class = i::ClientDll->GetAllClasses(); client_class; client_class = client_class->pNext)
//		{
//			if (client_class->nClassID == EClassIndex::CEconWearable)
//				return client_class->pCreateFn;
//		}
//	}
//
//	__forceinline static auto make_glove(int entry, int serial) -> CBaseCombatWeapon*
//	{
//		static auto create_wearable_fn = create_wearable();
//		create_wearable_fn(entry, serial);
//
//		const auto glove = reinterpret_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntity(entry));
//		assert(glove); {
//			static auto set_abs_origin_addr = (size_t)util::FindSignature("client.dll", ("55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8"));
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
//	bool apply_glove_model(CBaseCombatWeapon*, const char*);
//	bool apply_glove_skin(CBaseCombatWeapon*, int, int, int, int, int, float);
//	void glove_changer();
//
//};
//
//inline c_skins g_skins;