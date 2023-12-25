#include "IEngineTrace.hpp"
#include "entity.hpp"
#include "displacement.hpp"
#include "source.hpp"
bool CTraceFilterPlayersOnlySkipOne::ShouldHitEntity( IHandleEntity* pEntityHandle, int ) {
	return pEntityHandle != pEnt && ( ( IClientEntity* )pEntityHandle )->GetClientClass( )->m_ClassID == CCSPlayer;
}

bool CTraceFilter::ShouldHitEntity( IHandleEntity* pEntityHandle, int ) {
	ClientClass* pEntCC = ( ( IClientEntity* )pEntityHandle )->GetClientClass( );
	if ( pEntCC && strcmp( ccIgnore, "" ) ) {
		if ( pEntCC->m_pNetworkName == ccIgnore )
			return false;
	}

	return !( pEntityHandle == pSkip );
}

bool CGameTrace::DidHitWorld( ) const {
	return hit_entity == Source::m_pEntList->GetClientEntity( 0 );
}

bool CGameTrace::DidHitNonWorldEntity( ) const {
	return hit_entity != nullptr && !DidHitWorld( );
}


__forceinline uint32_t IEngineTrace::GetFilterSimpleVtable( ) {
	static const auto filter_simple = *reinterpret_cast< uint32_t* >( Engine::Displacement.Function.m_TraceFilterSimple );
	return filter_simple;
}
