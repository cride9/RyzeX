//#pragma once
//#include "../../SDK/Entity.h"
//
//namespace VirtualMethod
//{
//	template <typename T, std::size_t Idx, typename ...Args>
//	constexpr T call( void* classBase, Args... args ) noexcept
//	{
//		return ( *reinterpret_cast< T( __thiscall*** )( void*, Args... ) >( classBase ) )[ Idx ]( classBase, args... );
//	}
//}
//
//#define VIRTUAL_METHOD(returnType, name, idx, args, argsRaw) \
//returnType name args noexcept \
//{ \
//    return VirtualMethod::call<returnType, idx>argsRaw; \
//}
//
//#ifdef _WIN32
//
//#define VIRTUAL_METHOD_V(returnType, name, idx, args, argsRaw) \
//returnType name args noexcept \
//{ \
//    return VirtualMethod::call<returnType, idx>argsRaw; \
//}
//
//#else
//
//#define VIRTUAL_METHOD_V(returnType, name, idx, args, argsRaw) \
//returnType name args noexcept \
//{ \
//    return VirtualMethod::call<returnType, idx + 1>argsRaw; \
//}
//
//#endif
//
//#define CONCAT(a, b) a##b
//#define PAD_NAME(n) CONCAT(pad, n)
//
//#define PAD(size) \
//private: \
//    std::byte PAD_NAME(__COUNTER__) [size]; \
//public:
//
//template <typename T>
//class UtlVector {
//public:
//	constexpr T& operator[]( int i ) noexcept { return memory[ i ]; };
//	constexpr const T& operator[]( int i ) const noexcept { return memory[ i ]; };
//
//	T* memory;
//	int allocationCount;
//	int growSize;
//	int size;
//	T* elements;
//};
//
//template <typename T>
//struct UtlMemory {
//	T& operator[]( int i ) noexcept { return memory[ i ]; };
//
//	T* memory;
//	int allocationCount;
//	int growSize;
//};
//
//template <typename Key, typename Value>
//struct Node {
//	int left;
//	int right;
//	int parent;
//	int type;
//	Key key;
//	Value value;
//};
//
//template <typename Key, typename Value>
//struct UtlMap
//{
//	void* lessFunc;
//	UtlMemory<Node<Key, Value>> memory;
//	int root;
//	int numElements;
//	int firstFree;
//	int lastAlloc;
//	Node<Key, Value>* elements;
//};
//
//struct String
//{
//	UtlMemory<char> buffer;
//	int length;
//
//	char* data( ) noexcept { return buffer.memory; }
//};
//
//
//struct PaintKit_t
//{
//	int m_nID;
//	String m_szName;
//	String m_szDescription;
//	String m_szDescriptionTag;
//	String m_szUnk1;
//	String m_szPattern;
//	String m_szUnk2;
//	String m_szLogoMaterial;
//	bool m_bBaseDiffuseOverride;
//	int m_nRarity;
//	int m_nStyle;
//	Color m_rgbaColor[ 4 ];
//	Color m_rgbaLogoColor[ 4 ];
//	float m_flWearDefault;
//	float m_flWearRemapMin;
//	float m_flWearRemapMax;
//	unsigned char m_nFixedSeed;
//	unsigned char m_uchPhongExponent;
//	unsigned char m_uchPhongAlbedoBoost;
//	unsigned char m_uchPhongIntensity;
//	float m_flPatternScale;
//	float m_flPatternOffsetXStart;
//	float m_flPatternOffsetXEnd;
//	float m_flPatternOffsetYStart;
//	float m_flPatternOffsetYEnd;
//	float m_flPatternRotateStart;
//	float m_flPatternRotateEnd;
//	float m_flLogoScale;
//	float m_flLogoOffsetX;
//	float m_flLogoOffsetY;
//	float m_flLogoRotation;
//	bool m_bIgnoreWeaponSizeScale;
//	char m_chUnk3[ 3 ];
//	int m_nViewModelExponentOverrideSize;
//	bool m_bOnlyFirstMaterial;
//	bool m_bUnk4;
//	char m_chUnk5[ 2 ];
//	float m_flPearlescent;
//};
//
//struct StickerKit
//{
//	int id;
//	int rarity;
//	String name;
//	String description;
//	String itemName;
//	PAD( 2 * sizeof( String ) )
//		String inventoryImage;
//};
//
//class EconItemDefinition {
//public:
//	VIRTUAL_METHOD( EItemDefinitionIndex, getWeaponId, 0, ( ), ( this ) )
//		VIRTUAL_METHOD( const char*, getItemBaseName, 2, ( ), ( this ) )
//		VIRTUAL_METHOD( const char*, getItemTypeName, 3, ( ), ( this ) )
//		VIRTUAL_METHOD( const char*, getPlayerDisplayModel, 6, ( ), ( this ) )
//		VIRTUAL_METHOD( const char*, getWorldDisplayModel, 7, ( ), ( this ) )
//		VIRTUAL_METHOD( std::uint8_t, getRarity, 12, ( ), ( this ) )
//
//		int getCapabilities( ) noexcept
//	{
//		return *reinterpret_cast< int* >( this + 0x148 );
//	}
//
//	bool isPaintable( ) noexcept
//	{
//		return getCapabilities( ) & 1; // ITEM_CAP_PAINTABLE
//	}
//
//	const char* getDefinitionName( ) noexcept
//	{
//		return *reinterpret_cast< const char** >( this + 0x1DC ); // TODO: update linux
//	}
//};
//
//
//struct ItemListEntry {
//	int itemDef;
//	int paintKit;
//	PAD( 20 )
//
//		auto weaponId( ) const noexcept
//	{
//		return static_cast< EItemDefinitionIndex >( itemDef );
//	}
//};
//
//class EconLootListDefinition {
//public:
//	VIRTUAL_METHOD( const char*, getName, 0, ( ), ( this ) )
//		VIRTUAL_METHOD( const UtlVector<ItemListEntry>&, getLootListContents, 1, ( ), ( this ) )
//};
//
//class EconItemSetDefinition {
//public:
//		VIRTUAL_METHOD( const char*, getLocKey, 1, ( ), ( this ) )
//		VIRTUAL_METHOD( int, getItemCount, 4, ( ), ( this ) )
//		VIRTUAL_METHOD( EItemDefinitionIndex, getItemDef, 5, ( int index ), ( this, index ) )
//		VIRTUAL_METHOD( int, getItemPaintKit, 6, ( int index ), ( this, index ) )
//};
//
//struct EconItemQualityDefinition {
//	int value;
//	const char* name;
//	unsigned weight;
//	bool explicitMatchesOnly;
//	bool canSupportSet;
//	const char* hexColor;
//};
//
//struct UtlString {
//	UtlMemory<char> buffer;
//	int length;
//
//	const char* data( ) const noexcept { return buffer.memory; }
//};
//
//struct AlternateIconData {
//	UtlString simpleName;
//	UtlString largeSimpleName;
//	UtlString iconURLSmall;
//	UtlString iconURLLarge;
//	PAD( 28 )
//};
//
//struct EconMusicDefinition {
//	int id;
//	const char* name;
//	const char* nameLocalized;
//	PAD( 2 * sizeof( const char* ) )
//		const char* inventoryImage;
//};
//
//class IItemSchema
//{
//public:
//	PAD( ( 0x88 ) )
//		UtlMap<int, EconItemQualityDefinition> qualities;
//	PAD( ( 0x48 ) )
//		UtlMap<int, EconItemDefinition*> itemsSorted;
//	PAD( ( 0x60 ) )
//		UtlMap<int, const char*> revolvingLootLists;
//	PAD( ( 0x80 ) )
//		UtlMap<std::uint64_t, AlternateIconData> alternateIcons;
//	PAD( ( 0x48 ) )
//		UtlMap<int, PaintKit_t*> m_pPaintKits;
//	UtlMap<int, StickerKit*> stickerKits;
//	PAD( ( 0x11C ) )
//		UtlMap<int, EconMusicDefinition*> musicKits;
//
//		VIRTUAL_METHOD( EconItemDefinition*, getItemDefinitionInterface, 4, ( EItemDefinitionIndex id ), ( this, id ) )
//		VIRTUAL_METHOD( const char*, getRarityName, 19, ( int rarity ), ( this, rarity ) )
//		VIRTUAL_METHOD( int, getItemSetCount, 28, ( ), ( this ) )
//		VIRTUAL_METHOD( EconItemSetDefinition*, getItemSet, 29, ( int index ), ( this, index ) )
//		VIRTUAL_METHOD( EconLootListDefinition*, getLootList, 32, ( int index ), ( this, index ) )
//		VIRTUAL_METHOD( int, getLootListCount, 34, ( ), ( this ) )
//		VIRTUAL_METHOD( EconItemDefinition*, getItemDefinitionByName, 42, ( const char* name ), ( this, name ) )
//};
//
//class IItemSystem
//{
//public:
//	__forceinline IItemSchema* get_item_schema()
//	{
//		return util::GetVFunc< IItemSchema * (__thiscall*)(void*) >(this, 0)(this);
//	}
//};