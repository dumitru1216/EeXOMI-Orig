#include "Hooked.hpp"

void __stdcall Hooked::LockCursor( ) {
   g_Vars.globals.szLastHookCalled = XorStr( "16" );

   //if ( !ECX || !EDX )
   //   return;

   oLockCursor( (void*)Source::m_pSurface.Xor( ) );

   //Memory::VCall< LockCursorFn >( Source::m_pSurface.Xor( ), Index::VguiSurface::LockCursor );

   if ( g_Vars.globals.menuOpen )
	  Source::m_pSurface->UnlockCursor( );
}
