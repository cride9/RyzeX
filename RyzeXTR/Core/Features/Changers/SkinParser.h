#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <codecvt>
#include <unordered_set>

#include "../../SDK/DataTyes/Color.h"
#include "../../Interface/Interfaces/IItemSystem.h"
#include "../../utilities.h"
#include "../../SDK/Entity.h"

struct EConItem_t
{
	struct EconomyItem_t
	{
		EconomyItem_t(int entity_quality_index, int paint_kit_index, int stat_trak, float wear)
			: entity_quality_index(entity_quality_index), paint_kit_index(paint_kit_index), stat_trak(stat_trak), wear(wear) { }
		int entity_quality_index = 0;
		int paint_kit_index = 0;
		int stat_trak = 0;
		float wear = FLT_MIN;
	};
};

struct SkinObject_t
{
	SkinObject_t(const char* szName, const char* szModel, const char* szKillIcon = nullptr)
		: szName(szName), szModel(szModel), szKillIcon(szKillIcon) { }

	const char* szName = nullptr;
	const char* szModel = nullptr;
	const char* szKillIcon = nullptr;
};

struct AgentObject_t
{
	AgentObject_t(std::string szModelName, std::string szDisplayName)
		: szModelName(szModelName), szDisplayName(szDisplayName) {}

	std::string szModelName = "";
	std::string szDisplayName = "";
};

struct WeaponInfo_t
{
	constexpr WeaponInfo_t(const char* model, const char* icon = nullptr) :
		model(model),
		icon(icon)
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

	bool operator < (const SkinKit_t& other) const { return (m_szName < other.m_szName); }
};

struct SkinColors_t
{
	Color m_colColor[5];
};

namespace SkinChanger
{
	void Dump();

	//ItemSchema_t* m_pItemSchematic;

	/*containers*/
	std::vector<SkinKit_t> SkinKits;
	std::vector<SkinKit_t> GloveKits;
	std::vector<SkinKit_t> StickerKits;
	std::vector<SkinColors_t> SkinColors;
}