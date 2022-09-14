#pragma once
#include <deque>
#include "../../SDK/entity.h"
#include "../../Interface/interfaces.h"
#include "../../globals.h"
#include "../Misc/enginepred.h"

class CNetworking
{
public:
	void OnPacketEnd( CClientState* pClientState );
	void SaveNetvarData( int iCommand );
	void RestoreNetvarData( int iCommand );
private:
	std::array < CNetvarData, 150 > pCompressData = { };
};
inline CNetworking networking;

class CPacketManager
{
public:
	bool ShouldProcessPacketStart( int iCommand );
	std::deque<int> pCommandList;
};
inline CPacketManager packetManager;