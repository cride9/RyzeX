#pragma once
#pragma comment(lib, "lua51.lib")
#pragma comment(lib, "luajit.lib")

#include <Windows.h>
#include <vector>
#include <deque>
#include <string>
#include <filesystem>
#include <shlobj.h>
#include <shlwapi.h>

// https://www.unknowncheats.me/forum/counterstrike-global-offensive/164842-implementing-lua-into-counter-strike.html

#include "../SDK/DataTyes/Vector.h"
#include "../SDK/Menu/config.h"
#include <../lua/embedding/sol/sol.hpp>
#include "../xorstr.h"

inline std::mutex LUACallbackMutex;

namespace LuaImplementation {
	inline sol::state lua;

	void PrintError( std::string szError );

	void DisableTables( sol::state& lua );

	// Reference to variable helper
	class ScriptVarReference_t 
	{
		//ConfigValue<std::any>* pValue;
		void* pValue;
	public:
		ScriptVarReference_t( ) : pValue{ nullptr } { }
		ScriptVarReference_t( void* pValue ) : pValue{ pValue } { }

		// Get
		//bool GetBool( ) 
		//{
		//	if (!pValue)
		//		return false;

		//	return *pValue->value;
		//}

		//int GetInt( ) 
		//{
		//	if (!pValue)
		//		return 0;

		//	return pValue->get< int >( );
		//}

		//float GetFloat( ) 
		//{
		//	if (!pValue)
		//		return 0.f;

		//	return pValue->get< float >( );
		//}

		//Color GetColor( ) 
		//{
		//	if (!pValue->value)
		//		return color( 0, 0, 0, 0 );

		//	return &pValue->value<Color>();
		//}

		//std::string GetString( ) 
		//{
		//	if (!pValue)
		//		return "";

		//	// they think they're funny
		//	if (pValue->get_type( ) != vars::VarType_String)
		//		return std::to_string( pValue->get< double >( ) );

		//	return pValue->get_string( );
		//}

		//// Set
		//void SetBool( bool value ) 
		//{
		//	if (!pValue)
		//		return;

		//	pValue->set< bool >( value );
		//}

		//void SetInt( int value ) 
		//{
		//	if (!pValue)
		//		return;

		//	pValue->set< int >( value );
		//}

		//void SetFloat( float value ) 
		//{
		//	if (!pValue)
		//		return;

		//	pValue->set< float >( value );
		//}

		//void SetColor( color value ) 
		//{
		//	if (!pValue)
		//		return;

		//	pValue->set_color( value );
		//}

		//void SetString( std::string value ) 
		//{
		//	if (!pValue)
		//		return;

		//	pValue->set_value_string( value );
		//}

	};

	enum MENUITEMTYPE : int {
		MENUITEM_INVALID = -1,
		MENUITEM_CHECKBOX = 0,
		MENUITEM_SLIDERINT,
		MENUITEM_SLIDERFLOAT,
		MENUITEM_KEYBIND,
		MENUITEM_COMBOBOX,
		MENUITEM_MULTICOMBOBOX,
		MENUITEM_COLORPICKER,
		MENUITEM_BUTTON,
		// maybe add LABEL?
	};

	// Class for menu variables
	class BaseMenuItem_t {
	public:
		BaseMenuItem_t( ) { }
		virtual ~BaseMenuItem_t( ) { }

		// Path to parent script
		std::string szPathToScript = "";

		// Type of menu item
		int iType = MENUITEM_INVALID;

		// Is menu item visible?
		bool bVisible = true;

		// Label to be shown in menu
		std::string szLabel = "";

		// Variable name
		std::string szVarName = "";

		bool IsVisible( ) { return this->bVisible; }
		void SetVisible( bool value ) { this->bVisible = value; }

		bool bValue = false;

		int iValue = 0;

		// for keybinds
		// iValue is the key
		bool bIsKeybindActive = false;

		int iMinValue = 0;
		int iMaxValue = 0;

		float flValue = 0;

		float flMinValue = 0;
		float flMaxValue = 0;

		Color cValue = Color(255, 255, 255, 255);

		std::vector< std::string > vecComboboxLabels; // using this for both combo and multi combo box
		std::deque< bool > vecMultiComboValues;

		// for buttons
		std::deque< sol::function > vecFunctions;

		std::string szFormat = "%d";
	};

	class CheckboxMenuItem_t : public BaseMenuItem_t {
	public:
		bool Get( ) { return this->bValue; }
		void Set( bool value ) { this->bValue = value; }
	};

	class SliderIntMenuItem_t : public BaseMenuItem_t {
	public:
		int Get( ) { return this->iValue; }
		void Set( int value ) { this->iValue = value; }

		int GetMin( ) { return this->iMinValue; }
		void SetMin( int value ) { this->iMinValue = value; }

		int GetMax( ) { return this->iMaxValue; }
		void SetMax( int value ) { this->iMaxValue = value; }

		std::string GetFormat( ) { return this->szFormat; }
		void SetFormat( std::string value ) { this->szFormat = value; }
		void ResetFormat( ) { this->szFormat = "%d"; }
	};

	class SliderFloatMenuItem_t : public BaseMenuItem_t {
	public:
		float Get( ) { return this->flValue; }
		void Set( float value ) { this->flValue = value; }

		float GetMin( ) { return this->flMinValue; }
		void SetMin( float value ) { this->flMinValue = value; }

		float GetMax( ) { return this->flMaxValue; }
		void SetMax( float value ) { this->flMaxValue = value; }

		std::string GetFormat( ) { return this->szFormat; }
		void SetFormat( std::string value ) { this->szFormat = value; }
		void ResetFormat( ) { this->szFormat = "%.2f"; }
	};

	class KeybindMenuItem_t : public BaseMenuItem_t {
	public:
		bool IsActive( ) { return this->bIsKeybindActive; }

		int Get( ) { return this->iValue; }
		void Set( int value ) { this->iValue = value; }
	};

	class ComboMenuItem_t : public BaseMenuItem_t {
	public:
		int Get( ) { return this->iValue; }
		void Set( int value ) { this->iValue = value; }
	};

	class MultiComboMenuItem_t : public BaseMenuItem_t {
	public:
		bool Get( int idx ) { if (idx < 0 || idx > this->vecMultiComboValues.size( ) || this->vecMultiComboValues.empty( )) return false; return this->vecMultiComboValues[ idx ]; }
		void Set( int idx, bool value ) { if (idx < 0 || idx > this->vecMultiComboValues.size( ) || this->vecMultiComboValues.empty( )) return; this->vecMultiComboValues[ idx ] = value; }

		bool GetByLabel( std::string szLabel ) {

			if (this->vecMultiComboValues.empty( ) || this->vecComboboxLabels.empty( ) || this->vecMultiComboValues.size( ) != this->vecComboboxLabels.size( ))
				return false;

			int i = -1;
			for (int j = 0; j < this->vecComboboxLabels.size( ); j++) {
				if (this->vecComboboxLabels[ j ] == szLabel) {
					i = j;
					break;
				}
			}

			if (i >= 0 && i < this->vecMultiComboValues.size( )) {
				return this->vecMultiComboValues[ i ];
			}

			return false;
		}

		void SetByLabel( std::string szLabel, bool value ) {

			if (this->vecMultiComboValues.empty( ) || this->vecComboboxLabels.empty( ) || this->vecMultiComboValues.size( ) != this->vecComboboxLabels.size( ))
				return;

			int i = -1;
			for (int j = 0; j < this->vecComboboxLabels.size( ); j++) {
				if (this->vecComboboxLabels[ j ] == szLabel) {
					i = j;
					break;
				}
			}

			if (i >= 0 && i < this->vecMultiComboValues.size( )) {
				this->vecMultiComboValues[ i ] = value;
			}
		}
	};

	class ColorPickerMenuItem_t : public BaseMenuItem_t {
	public:
		Color Get( ) { return Color( this->cValue[ 0 ] * 255, this->cValue[ 1 ] * 255, this->cValue[ 2 ] * 255, this->cValue[ 3 ] * 255 ); }
		void Set( Color value ) { Color( this->cValue[ 0 ] * 255, this->cValue[ 1 ] * 255, this->cValue[ 2 ] * 255, this->cValue[ 3 ] * 255 ) = value; }
	};

	class ButtonMenuItem_t : public BaseMenuItem_t {
	public:
		void SetFunction( sol::function pFunction ) {
			LUACallbackMutex.lock( );
			this->vecFunctions.clear( );
			this->vecFunctions.push_back( pFunction );
			LUACallbackMutex.unlock( );
		}
	};

	 //special font class
	class ScriptFont_t /*: public CSurfaceFont*/ {
	public:
		/*ScriptFont_t( ) : CSurfaceFont( ) { }

		vec2_t GetTextSize_( std::string szText, sol::this_state L );
		void DrawText_( vec2_t vecPos, std::string szText, color cColor, sol::this_state L );*/
	};

	struct ScriptCallback_t {
		ScriptCallback_t( ) : pFunction{ nullptr }, szName{ "" } { };
		ScriptCallback_t( sol::function pFunction, std::string szName ) : pFunction{ pFunction }, szName{ szName } { };

		sol::function pFunction;
		std::string szName;
	};

	class ScriptInfo_t {
	private:
	public:
		ScriptInfo_t( ) : szName{ "" }, szPath{ "" }, iIndex{ 0 }, vecCallbacks{ }, bLoaded{ false }, bShouldUnload{ false } { }

#if 0
		~ScriptInfo_t( )
		{
			this->Unload( );

			// clear stuff
			this->vecCallbacks.clear( );

			// destroy lua state
			if (this->pLuaState) {
				lua_close( this->pLuaState );
				this->pLuaState = nullptr;
			}
		}
#endif

		/// <summary>
		/// Destroy ScriptInfo_t object (actually, just clear what's inside)
		/// </summary>
		void Destroy( ) {
			this->Unload( );

			// clear stuff
			this->vecCallbacks.clear( );
			this->vecFonts.clear( );
			this->vecMenuItems.clear( );

			this->bShouldUnload = false;
		}

		/// <summary>
		/// this->Destroy() but special for callback errors
		/// </summary>
		void OnCallbackFail( )
		{
			// we do this so we don't do Unload callback
			this->bShouldUnload = false;
			this->bLoaded = false;
			this->Destroy( );
		}

		std::string szName;
		std::string szPath;
		int iIndex;
		std::deque< ScriptCallback_t > vecCallbacks;
		std::deque< ScriptFont_t > vecFonts;
		std::deque< BaseMenuItem_t > vecMenuItems;
		bool bLoaded;
		bool bShouldUnload;

		bool Load( );
		void Unload( );

		/// <summary>
		/// Adds callback to script
		/// </summary>
		/// <param name="szName">Callback name</param>
		/// <param name="pFunction">Callback function</param>
		void AddCallback( std::string szName, sol::function pFunction )
		{
			LUACallbackMutex.lock( );
			this->vecCallbacks.emplace_back( pFunction, szName );
			LUACallbackMutex.unlock( );
		}

		/// <summary>
		/// Run callback by name
		/// </summary>
		/// <param name="szName">Callback's name</param>
		void RunCallback( std::string szName )
		{
			for (auto& callback : vecCallbacks) {
				if (callback.szName == szName) {
					try {
						auto ret = callback.pFunction( );
						if (!ret.valid( )) {
							sol::error err = ret;
							throw err;
						}
					}
					catch (sol::error& ex)
					{
						PrintError( ex.what( ) );
						this->OnCallbackFail( );
					}
					catch (std::string& ex)
					{
						PrintError( ex );
						this->OnCallbackFail( );
					}
					catch (std::exception& ex)
					{
						PrintError( ex.what( ) );
						this->OnCallbackFail( );
					}
					catch (...) {
						PrintError( XorStr( "unknown" ) );
						this->OnCallbackFail( );
					}
				}
			}
		}

		/// <summary>
		/// Run callback by name with argument given
		/// </summary>
		/// <param name="szName">Callback's name</param>
		template< typename... Args >
		void RunCallback( std::string szName, Args... args )
		{
			for (auto& callback : vecCallbacks) {
				if (callback.szName == szName) {
					try {
						auto ret = callback.pFunction( args... );
						if (!ret.valid( )) {
							sol::error err = ret;
							throw err;
						}
					}
					catch (sol::error& ex)
					{
						PrintError( ex.what( ) );
						this->OnCallbackFail( );
					}
					catch (std::string& ex)
					{
						PrintError( ex );
						this->OnCallbackFail( );
					}
					catch (std::exception& ex)
					{
						PrintError( ex.what( ) );
						this->OnCallbackFail( );
					}
					catch (...) {
						PrintError( XorStr( "unknown" ) );
						this->OnCallbackFail( );
					}
				}
			}
		}

		/// <summary>
		/// Find menu item by variable name, and optionally also by type
		/// </summary>
		/// <param name="szVarName">Item's variable name</param>
		/// <param name="iType">Optional, specify the type that menu item should be</param>
		/// <returns></returns>
		BaseMenuItem_t* FindMenuItem( std::string szVarName, int iType = MENUITEM_INVALID )
		{
			for (auto& item : vecMenuItems) {
				if (item.szVarName == szVarName) {
					if (iType != MENUITEM_INVALID) {
						if (item.iType != iType)
							continue;
					}

					return &item;
				}
			}

			return nullptr;
		}
	};

	// Stored script infos
	inline std::deque< ScriptInfo_t > vecScriptInfos = { };

	// Callback list ( excluding game events )
	enum ECallbackList_t : int {
		CALLBACK_ON_UNLOAD,
		CALLBACK_ON_DRAW,
		CALLBACK_PRE_PREDICTION,
		CALLBACK_PREDICTION,
		CALLBACK_ON_CREATE_MOVE,
		CALLBACK_FRAME_STAGE_NOTIFY,
		CALLBACK_FIRE_GAME_EVENT,
		CALLBACK_ON_RAGEBOT_SHOT,
		CALLBACK_LIST_END
	};

	inline std::vector< std::string > vecCallbackList = { };

	/// <summary>
	/// Creates lua state and includes libraries and stuff needed
	/// </summary>
	/// <returns>sol::state</returns>
	void CreateLuaState( );

	/// <summary>
	/// Initialize Lua Implementation
	/// </summary>
	void Initialize( );

	/// <summary>
	/// Run additional checks
	/// </summary>
	void RunChecks( );

	/// <summary>
	/// Parse files and add script info
	/// </summary>
	void Parse( );

	/// <summary>
	/// Save to config
	/// </summary>
	/// <param name="j">JSON to be saved to</param>
	//void SaveToJson( nlohmann::json& j );

	/// <summary>
	/// Load from config
	/// </summary>
	/// <param name="j">JSON to be loaded from</param>
	//void LoadFromJson( nlohmann::json j );

	/// <summary>
	/// Get pointer to script info, by path
	/// </summary>
	/// <param name="szPath">Script path</param>
	/// <returns></returns>
	inline ScriptInfo_t* FindScriptByPath( std::string szPath ) {
		if (vecScriptInfos.empty( )) {
			return nullptr;
		}

		for (auto& script : vecScriptInfos) {
			if (script.szPath == szPath) {
				return &script;
			}
		}

		return nullptr;
	}

	/// <summary>
	/// Get pointer to script info, by name (including extension)
	/// </summary>
	/// <param name="szName">Script name + extension</param>
	/// <returns></returns>
	inline ScriptInfo_t* FindScriptByName( std::string szName ) {
		if (vecScriptInfos.empty( )) {
			return nullptr;
		}

		for (auto& script : vecScriptInfos) {
			if (script.szName == szName) {
				return &script;
			}
		}

		return nullptr;
	}

	/// <summary>
	/// Run callbacks by name
	/// </summary>
	/// <param name="szName">Callback's name</param>
	inline void RunCallbacks( std::string szName )
	{
		LUACallbackMutex.lock( );
		for (auto& script : vecScriptInfos) {
			script.RunCallback( szName );
		}
		LUACallbackMutex.unlock( );
	}

	/// <summary>
	/// Run callbacks by name with argument given
	/// </summary>
	/// <param name="szName">Callback's name</param>
	template< typename... Args >
	inline void RunCallbacks( std::string szName, Args... args )
	{
		LUACallbackMutex.lock( );
		for (auto& script : vecScriptInfos) {
			script.RunCallback( szName, args... );
		}
		LUACallbackMutex.unlock( );
	}
}

