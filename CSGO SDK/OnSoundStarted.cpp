#include "hooked.hpp"
#include "ServerSounds.hpp"

namespace Hooked
{
   void __fastcall OnSoundStarted( void* ECX, void* EDX, int guid, StartSoundParams_t& params, char const* soundname ) {
	  g_Vars.globals.szLastHookCalled = XorStr( "17" );
	  oOnSoundStarted( ECX, guid, params, soundname );
	  //if ( params.ServerFlags & SV_SND_FROMSERVER )
		 //g_ServerSounds.ProcessSound( guid, params, soundname );
   }

#if 0
   void __stdcall CL_AddSound( C_SoundInfo * info, int* a1, bool* a2 ) {
	  oCL_AddSound( info, a1, a2 );

	  if ( info ) {
		 Vector kek = info->vDirection;
		 const char* name = info->pszName;
		 printf( "%s | x: %.f | y: %.f | z: %.z", name, kek.x, kek.y, kek.z );
	  }
}
#endif
}