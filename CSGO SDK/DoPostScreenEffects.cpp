#include "Hooked.hpp"
#include "Glow.hpp"
#include "CChams.hpp"
#include "CVariables.hpp"
#include "player.hpp"

namespace Hooked
{
   int __stdcall DoPostScreenEffects( int a1 ) {
	  g_Vars.globals.szLastHookCalled = XorStr( "5" );
	  if ( g_Vars.globals.HackIsReady && g_Vars.globals.RenderIsReady )
		 GlowOutline::Get( )->Render( );

	  Source::IChams::Get( )->OnPostScreenEffects( );

	  bool prev_scoped;

	 // C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	 // if ( !local->IsDead( ) ) {
	//	  prev_scoped = local->m_bIsScoped( );
	 //
	//	  local->m_bIsScoped( ) = false;
	 //
	//	  return oDoPostScreenEffects( Source::m_pClientMode.Xor( ), a1 );
	 //
	//	  local->m_bIsScoped( ) = prev_scoped;
	 // }

	  return oDoPostScreenEffects( Source::m_pClientMode.Xor( ), a1 );
   }
}
