#include "hooked.hpp"
#include "player.hpp"
#include "CVariables.hpp"

class N00000B3B {
public:
   char pad_0x0000[ 0xC4 ]; //0x0000
   Vector N00000B6D; //0x00C4
   char pad_0x00D0[ 0x770 ]; //0x00D0

}; //Size=0x0840

void __fastcall Hooked::hkRenderView( void* ECX, void* EDX, const CViewSetup& view, CViewSetup& hudViewSetup, int nClearFlags, int whatToDraw ) {
   g_Vars.globals.szLastHookCalled = XorStr( "30" );
   uintptr_t arg_0;

   if ( g_Vars.misc.override_player_view_key.enabled ) {
	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local ) {
		 oRenderView( ECX, view, hudViewSetup, nClearFlags, whatToDraw );
		 return;
	  }

	  __asm {
		 mov esi, [ ebp + 0x8 ]
		 mov arg_0, esi
	  }
	  if ( !local->IsDead( ) ) {
		 N00000B3B* viewsetup = ( N00000B3B* ) arg_0;
		 viewsetup->N00000B6D.y += 180;
	  }
   }

   oRenderView( ECX, view, hudViewSetup, nClearFlags, whatToDraw );
}