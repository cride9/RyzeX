#pragma once
class CEconItemDefinition
{
public:
	
	__forceinline const char* get_player_display_model()
	{
		return util::GetVFunc< const char* (__thiscall*)(void*) >(this, 6)(this);
	}

	__forceinline const char* get_world_display_model()
	{
		return util::GetVFunc< const char* (__thiscall*)(void*) >(this, 7)(this);
	}

	__forceinline std::uint8_t get_quality() noexcept
	{
		return *reinterpret_cast<std::uint8_t*>(std::uintptr_t(this) + 0x2B);
	}

	__forceinline int get_item_type() noexcept
	{
		return *reinterpret_cast<int*>(std::uintptr_t(this) + 0x130);
	}

	__forceinline const char* get_definition_name() noexcept
	{
		return *reinterpret_cast<const char**>(this + 0x1DC);
	}
};

class IItemSchema
{
public:
	__forceinline CEconItemDefinition* get_item_definition_interface(int id)
	{
		return util::GetVFunc< CEconItemDefinition * (__thiscall*)(void*, int) >(this, 4)(this, id);
	}

	__forceinline auto get_item_definition_interface(short id) noexcept
	{
		return get_item_definition_interface(static_cast<int>(id));
	}
};

class IItemSystem
{
public:
	__forceinline IItemSchema* get_item_schema()
	{
		return util::GetVFunc< IItemSchema * (__thiscall*)(void*) >(this, 0)(this);
	}
};