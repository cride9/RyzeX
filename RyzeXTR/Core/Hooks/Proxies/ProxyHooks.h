#pragma once
#include "../../SDK/NetVar/datatable.h"

class CRecvProxyData;
using RecvVarProxyFn_t = void(__cdecl*)(const CRecvProxyData*, void*, void*);
using ArrayLengthProxyFn_t = void(__cdecl*)(void*, int, int);
using DataTableProxyFn_t = void(__cdecl*)(const RecvProp_t*, void**, void*, int);

class CRecvPropHook
{
public:
	bool Create(RecvProp_t* pRecvProp, const RecvVarProxyFn_t pNewProxyFn)
	{
		if (pRecvProp == nullptr)
			return false;

		pProperty = pRecvProp;
		fnOriginal = pRecvProp->oProxyFn;

		Replace(pNewProxyFn);
		return true;
	}

	// replace property proxy with our function
	void Replace(const RecvVarProxyFn_t pNewProxyFn) const
	{
		pProperty->oProxyFn = pNewProxyFn;
	}

	// restore original property proxy function
	void Restore() const
	{
		if (fnOriginal != nullptr)
			pProperty->oProxyFn = fnOriginal;
	}

	/// @returns: original property proxy function
	[[nodiscard]] RecvVarProxyFn_t GetOriginal() const
	{
		return this->fnOriginal;
	}

private:
	// current property
	RecvProp_t* pProperty = nullptr;
	// original proxy function to have ability to restore it later
	RecvVarProxyFn_t fnOriginal = nullptr;
};

namespace p {

	bool Setup();
	void Destroy();

	/* @section: handlers */
	void BaseViewModelSequence(const CRecvProxyData* pData, void* pStruct, void* pOut);

	/* @section: managers */
	inline CRecvPropHook hkBaseViewModelSequence;
}