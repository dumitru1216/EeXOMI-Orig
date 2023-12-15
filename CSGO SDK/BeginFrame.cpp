#include "hooked.hpp"
#include "BulletBeamTracer.hpp"

void __fastcall Hooked::BeginFrame( void* ECX, void* EDX, float ft ) {
   g_Vars.globals.szLastHookCalled = XorStr( "1" );
   if ( g_Vars.esp.beam_enabled && g_Vars.globals.HackIsReady && g_Vars.globals.RenderIsReady )
	  IBulletBeamTracer::Get( )->Main( );

   oBeginFrame( ECX, ft );
}
