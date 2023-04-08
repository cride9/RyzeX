#include "agentchanger.h"

void CAgentChanger::AgentChanger( EStage stage )
{
	//if ( !C::Get<bool>( Vars.bEnableSkinChanger ) )
	//	return;

	//if ( !C::Get<std::vector<int>>( Vars.iSkinId ).at( 36 ) )
	//	return;

	//static int originalIdx = 0;

	//if ( !g::pLocal )
	//{
	//	originalIdx = 0;
	//	return;
	//}

	//if ( const auto model = C::Get<std::vector<std::string>>( Vars.vecSkinsImgPaths ).at( 36 ).c_str( ) )
	//{
	//	if ( stage == FRAME_RENDER_START )
	//		originalIdx = G::m_pLocal->GetModelIndex( );

	//	const auto idx = stage == FRAME_RENDER_END && originalIdx ? originalIdx : I::ModelInfo->GetModelIndex( model );
	//	G::m_pLocal->SetModelIndex( idx );
	//}
}