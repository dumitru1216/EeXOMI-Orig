#include "hooked.hpp"
#include "player.hpp"
#include "CVariables.hpp"

bool __fastcall Hooked::hkDispatchUserMessage( void* ECX, void* EDX, int msg_type, int unk1, int nBytes, bf_read& msg_data ) {
   g_Vars.globals.szLastHookCalled = XorStr( "4" );
  return oDispatchUserMessage( ECX, msg_type, unk1, nBytes, msg_data );
}
