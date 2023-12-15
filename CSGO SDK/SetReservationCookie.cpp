#pragma once
#include "Hooked.hpp"
#include "source.hpp"

uint32_t cookie_0 = 0;
uint32_t cookie_1 = 0;

// just some test code
void __fastcall Hooked::SetReservationCookie( void* ECX, void* EDX, int a2, int a3 ) {
   g_Vars.globals.szLastHookCalled = XorStr( "36" );
#if 0
  // void* ECX, void* EDX, int a2, int a3 
  using SetReservationCookieFn = void( __thiscall* )( void*, int, int );
  Source::m_pClientStateSwap->VCall<SetReservationCookieFn>( 63 )( ECX, a2, a3 );

  if( a2 || a3 ) {
	 cookie_0 = a2;
	 cookie_1 = a3;
  }
#endif

  return;
}
