#include "Hooked.hpp"

void __stdcall Hooked::LockCursor( ) {
   g_Vars.globals.szLastHookCalled = XorStr( "16" );
   oLockCursor( (void*)Source::m_pSurface.Xor( ) );

   if ( g_Vars.globals.menuOpen )
	  Source::m_pSurface->UnlockCursor( );
}
