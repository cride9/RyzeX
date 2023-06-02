#include "ProxyHooks.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../Features/Changers/wtf.h"

bool p::Setup()
{
	// get viewmodel m_nSequence Netvar prop
	RecvProp_t* pViewModelSequence = n::netvars[ fnv::HashConst( "CBaseViewModel->m_nSequence", 0x811C9DC5 ) ].pRecvProp;
	if ( pViewModelSequence == nullptr )
		return false;

	hkBaseViewModelSequence = std::make_shared<CRecvPropHook>( pViewModelSequence, BaseViewModelSequence );

	return true;
}

void p::Destroy()
{
	// @note: as example
	hkBaseViewModelSequence->Restore();
}

void p::BaseViewModelSequence(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	const auto oSequence = hkBaseViewModelSequence->GetOriginal();

	// sanity checks
	if (g::pLocal == nullptr || !g::pLocal->IsAlive())
		return oSequence(pData, pStruct, pOut);

	// get modifiable proxy data
	auto ProxyData = const_cast<CRecvProxyData*>(pData);
	// get viewmodel pointer
	auto ViewModel = static_cast<CBaseViewModel*>(pStruct);
	// get owner from viewmodel pointer
	unsigned int OwnerHandle = ViewModel->GetOwnerHandle();

	if (ViewModel && OwnerHandle)
	{
		// get owner of current viewmodel
		CBaseEntity* Owner = static_cast<CBaseEntity*>(i::EntityList->GetClientEntityFromHandle(OwnerHandle));

		if (Owner == g::pLocal)
		{
			// Get the filename of the current view model.
			const Model_t* KnifeModel = i::ModelInfo->GetModel(ViewModel->GetModelIndex());
			const char* ModelName = i::ModelInfo->GetModelName(KnifeModel);
			// set proxy data
			ProxyData->Value.Int = beforeIfuckUpEverything::GetNewAnimation( FNV1A::hash_runtime( ModelName ), ProxyData->Value.Int);
		}
	}

	oSequence( ProxyData, pStruct, pOut );
}