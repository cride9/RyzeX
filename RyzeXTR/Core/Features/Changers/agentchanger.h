#include "../../globals.h"
#include "../../SDK/Entity.h"

struct AgentObject_t
{
	AgentObject_t( std::string szModelName, std::string szDisplayName )
		: szModelName( szModelName ), szDisplayName( szDisplayName ) {}

	std::string szModelName = "";
	std::string szDisplayName = "";
};

// @todo: add valve vdf/vpk parser and get skins, rarity things, etc with it
class CAgentChanger
{
public:
	void AgentChanger( EStage stage );

private:
	const std::vector<AgentObject_t> agentList =
	{
		{"models/player/custom_player/legacy/ctm_sas.mdl", "Default"},
		{"models/player/custom_player/legacy/tm_leet_variantf.mdl", "The Elite Mr. Muhlik | Elite Crew"},
		{"models/player/custom_player/legacy/tm_leet_varianti.mdl", "Prof. Shahmat | Elite Crew"},
		{"models/player/custom_player/legacy/tm_leet_varianth.mdl", "Osiris | Elite Crew"},
		{"models/player/custom_player/legacy/tm_leet_variantg.mdl", "Ground Rebel  | Elite Crew"},
		{"models/player/custom_player/legacy/ctm_fbi_variantb.mdl",	"Special Agent Ava | FBI"},
		{"models/player/custom_player/legacy/ctm_fbi_varianth.mdl", "Michael Syfers  | FBI Sniper"},
		{"models/player/custom_player/legacy/ctm_fbi_variantg.mdl", "Markus Delrow | FBI HRT"},
		{"models/player/custom_player/legacy/ctm_fbi_variantf.mdl", "Operator | FBI SWAT"},
		{"models/player/custom_player/legacy/ctm_st6_variante.mdl", "Seal Team 6 Soldier | NSWC SEAL"},
		{"models/player/custom_player/legacy/ctm_st6_variantm.mdl", "'Two Times' McCoy | USAF TACP"},
		{"models/player/custom_player/legacy/ctm_st6_variantg.mdl", "Buckshot | NSWC SEAL"},
		{"models/player/custom_player/legacy/ctm_st6_variantk.mdl", "3rd Commando Company | KSK"},
		{"models/player/custom_player/legacy/ctm_st6_varianti.mdl", "Lt. Commander Ricksaw | NSWC SEAL"},
		{"models/player/custom_player/legacy/ctm_st6_variantj.mdl", "'Blueberries' Buckshot | NSWC SEAL"},
		{"models/player/custom_player/legacy/ctm_st6_variantl.mdl", "'Two Times' McCoy | TACP Cavalry"},
		{"models/player/custom_player/legacy/ctm_swat_variante.mdl","Cmdr. Mae 'Dead Cold' Jamison | SWAT"},
		{"models/player/custom_player/legacy/ctm_swat_variantf.mdl","1st Lieutenant Farlow | SWAT"},
		{"models/player/custom_player/legacy/ctm_swat_variantg.mdl","John 'Van Healen' Kask | SWAT"} ,
		{"models/player/custom_player/legacy/ctm_swat_varianth.mdl","Bio-Haz Specialist | SWAT"},
		{"models/player/custom_player/legacy/ctm_swat_varianti.mdl","Sergeant Bombson | SWAT"},
		{"models/player/custom_player/legacy/ctm_swat_variantj.mdl","Chem-Haz Specialist | SWAT"},
		{"models/player/custom_player/legacy/tm_balkan_varianti.mdl", "Maximus | Sabre"},
		{"models/player/custom_player/legacy/tm_balkan_variantf.mdl", "Dragomir | Sabre"},
		{"models/player/custom_player/legacy/tm_balkan_varianth.mdl", "'The Doctor' Romanov | Sabre"},
		{"models/player/custom_player/legacy/tm_balkan_variantg.mdl",  "Rezan The Ready | Sabre"},
		{"models/player/custom_player/legacy/tm_balkan_variantj.mdl", "Blackwolf | Sabre"},
		{"models/player/custom_player/legacy/tm_balkan_variantk.mdl",  "Rezan the Redshirt | Sabre"},
		{"models/player/custom_player/legacy/tm_balkan_variantl.mdl", "Dragomir | Sabre Footsoldier"},
		{"models/player/custom_player/legacy/ctm_sas_variantf.mdl","B Squadron Officer | SAS"},
		{"models/player/custom_player/legacy/tm_phoenix_varianth.mdl", "Soldier | Phoenix"},
		{"models/player/custom_player/legacy/tm_phoenix_variantf.mdl", "Enforcer | Phoenix"},
		{"models/player/custom_player/legacy/tm_phoenix_variantg.mdl", "Slingshot | Phoenix"},
		{"models/player/custom_player/legacy/tm_phoenix_varianti.mdl", "Street Soldier | Phoenix"},
		{"models/player/custom_player/legacy/tm_professional_varf.mdl", "Sir Bloody Miami Darryl | The Professionals"},
		{"models/player/custom_player/legacy/tm_professional_varf1.mdl", "Sir Bloody Silent Darryl | The Professionals"},
		{"models/player/custom_player/legacy/tm_professional_varf2.mdl", "Sir Bloody Skullhead Darryl | The Professionals"},
		{"models/player/custom_player/legacy/tm_professional_varf3.mdl", "Sir Bloody Darryl Royale | The Professionals"},
		{"models/player/custom_player/legacy/tm_professional_varf4.mdl", "Sir Bloody Loudmouth Darryl | The Professionals"},
		{"models/player/custom_player/legacy/tm_professional_varg.mdl", "Safecracker Voltzmann | The Professionals"},
		{"models/player/custom_player/legacy/tm_professional_varh.mdl", "Little Kev | The Professionals"},
		{"models/player/custom_player/legacy/tm_professional_vari.mdl", "Number K | The Professionals"},
		{"models/player/custom_player/legacy/tm_professional_varj.mdl", "Getaway Sally | The Professionals"}

	};

};
inline CAgentChanger agentChanger;