#pragma once

class IClientNetworkable;
using CreateClientClassFn = IClientNetworkable * (__cdecl*)(int, int);
using CreateEventFn = IClientNetworkable * (__cdecl*)();

class CStandartRecvProxies;
struct RecvTable_t;
enum class EClassIndex;

// @credits: https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/client_class.h
class CBaseClient
{
public:

	CreateClientClassFn		pCreateFn;
	CreateEventFn			pCreateEventFn;
	char* szNetworkName;
	RecvTable_t* pRecvTable;
	CBaseClient* pNext;
	EClassIndex				nClassID;
};

class IBaseClientDLL : ROP::VirtualCallable_t<ROP::ClientGadget_t> {

public:

	CBaseClient* GetAllClasses() {

		return CallVFunc<CBaseClient*, 8U>(this);
	}
};