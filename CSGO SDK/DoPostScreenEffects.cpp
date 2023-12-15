#include "Hooked.hpp"
#include "Glow.hpp"
#include "CChams.hpp"
#include "CVariables.hpp"

namespace Hooked
{
   int __stdcall DoPostScreenEffects( int a1 ) {
	  g_Vars.globals.szLastHookCalled = XorStr( "5" );
	  if ( g_Vars.globals.HackIsReady && g_Vars.globals.RenderIsReady )
		 GlowOutline::Get( )->Render( );

	  Source::IChams::Get( )->OnPostScreenEffects( );

	  return oDoPostScreenEffects( Source::m_pClientMode.Xor( ), a1 );
   }
}
