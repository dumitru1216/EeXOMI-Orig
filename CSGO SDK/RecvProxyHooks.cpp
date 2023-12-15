#include "Hooked.hpp"
#include "displacement.hpp"
#include "sdk.hpp"
#include "player.hpp"
#include "CBaseHandle.hpp"
#include "LagCompensation.hpp"

namespace Hooked
{
   void m_nSmokeEffectTickBegin( const CRecvProxyData* pData, void* pStruct, void* pOut ) {
	  g_Vars.globals.szLastHookCalled = XorStr( "28" );

	  if ( !pData || !pStruct || !pOut )
		 return;

	  Source::m_pDidSmokeEffectSwap->GetOriginalFunction( )( pData, pStruct, pOut );
	  if ( g_Vars.esp.remove_smoke ) {
		 // ref: "explosion_smokegrenade"
		 // ifdefd 2020
		 // *( uint8_t* ) ( ( uintptr_t ) pStruct + Engine::Displacement.DT_SmokeGrenadeProjectile.m_SmokeParticlesSpawned ) = 1;
		 *reinterpret_cast< bool* >( reinterpret_cast< uintptr_t >( pOut ) + 0x1 ) = true;
	  }
   }

   void RecvProxy_m_flAbsYaw( const CRecvProxyData* pData, void* pStruct, void* pOut ) {
	  g_Vars.globals.szLastHookCalled = XorStr( "29" );

	  if ( !pData || !pStruct || !pOut )
		 return;

	  Source::m_pFlAbsYawSwap->GetOriginalFunction( )( pData, pStruct, pOut );

	  if ( Source::m_pEngine->IsConnected( ) && Source::m_pEngine->IsInGame( ) ) {
		 CBaseHandle handle = *( CBaseHandle* ) ( ( uintptr_t ) pStruct + Engine::Displacement.DT_CSRagdoll.m_hPlayer );
		 if ( handle.IsValid( ) ) {
			auto player = ( C_CSPlayer* ) handle.Get( );

			if ( player ) {
			   auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->entindex( ) ).Xor( );
			   if ( lag_data && lag_data->m_History.size( ) ) {
				  lag_data->m_bGotAbsYaw = true;
				  lag_data->m_flAbsYawHandled = pData->m_Value.m_Float;
			   }
			}
		 }

		 Source::m_pFlAbsYawSwap->GetOriginalFunction( )( pData, pStruct, pOut );
	  }
   }

   void RecvProxy_PlaybackRate( const CRecvProxyData* pData, void* pStruct, void* pOut ) {
	  g_Vars.globals.szLastHookCalled = XorStr( "47" );
	  // PlaybackRate
	  Source::m_pPlaybackRateSwap->GetOriginalFunction( )( pData, pStruct, pOut );

	  C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

	  if ( !LocalPlayer )
		 return;

	  auto pAnimOverlay = ( C_AnimationLayer* ) pStruct;
	  if ( pAnimOverlay ) {
		 auto player = ( C_BasePlayer* ) pAnimOverlay->m_pOwner;
		 if ( !player || player == LocalPlayer )
			return;

		 auto& lag_data = Engine::LagCompensation::Get( )->GetLagData( player->entindex( ) );
		 if ( lag_data.Xor( ) ) {
			lag_data->m_flRate = pAnimOverlay->m_flPlaybackRate;
			lag_data->m_bRateCheck = true;
		 }
	  }
   }
}
