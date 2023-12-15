#include "Hooked.hpp"
#include "Movement.hpp"
#include "player.hpp"
#include "InputSys.hpp"
#include "Exploits.hpp"

using WriteUsercmdDeltaToBufferFn = bool( __thiscall* )( void*, int, void*, int, int, bool );
#if 0
void WriteUsercmd( bf_write* buf, CUserCmd* incmd, CUserCmd* outcmd ) {
   using WriteUsercmd_t = void( __fastcall* )( void*, CUserCmd*, CUserCmd* );
   static WriteUsercmd_t WriteUsercmdF = ( WriteUsercmd_t ) Memory::Scan( "client.dll", ( "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D" ) );

   __asm
   {
	  mov     ecx, buf
	  mov     edx, incmd
	  push    outcmd
	  call    WriteUsercmdF
	  add     esp, 4
   }
}

#if 0
bool TickbaseManipulation( void* pThis, int tickbase_shift, CCLCMsg_Move * CL_Move, void* ECX, int nSlot, int from, void* buffer ) {
   static auto ret = Source::m_pClientSwap->VCall<WriteUsercmdDeltaToBufferFn>( 24 );

   auto v28 = CL_Move->num_new_commands( ) + tickbase_shift;
   auto v29 = CL_Move->num_new_commands( );
   CL_Move->set_num_backup_commands( 0u );
   if ( v28 > 62 )
	  v28 = 62;
   CUserCmd to_cmd, from_cmd;

   CL_Move->set_num_backup_commands( v28 );

   auto nextCmdNr = Source::m_pClientState->m_nChokedCommands( ) + Source::m_pClientState->m_nLastOutgoingCommand( ) + 1;
   auto to = nextCmdNr - v29 + 1;
   if ( to > nextCmdNr ) {
   LABEL_41:
	  auto cmd_from_slot = &Source::m_pInput->m_pCommands[ from % MULTIPLAYER_BACKUP ];
	  if ( cmd_from_slot ) {
		 from_cmd = *cmd_from_slot;
		 to_cmd = CUserCmd{};

		 ++to_cmd.command_number;
		 to_cmd.tick_count += 200;
		 if ( v29 <= v28 ) {
			auto totalNewCommands = v28 - v29 + 1;
			do {
			   WriteUsercmd( ( bf_write* ) buffer, &to_cmd, &from_cmd );
			   from_cmd = to_cmd;
			   ++to_cmd.command_number;
			   ++to_cmd.tick_count;
			   --totalNewCommands;
			} while ( totalNewCommands );
		 }
	  }
	  return true;
   } else {
	  while ( ret( pThis, nSlot, buffer, from, to, 1 ) ) {
		 from = to++;
		 if ( to > nextCmdNr )
			goto LABEL_41;
	  }
	  return false;
   }
   return false;
}
#endif

bool __stdcall TickbaseManipulation( CCLCMsg_Move_t* msg, int tickbase_shift, void* this0, int nSlot, void* buffer ) {
   static auto WriteUsercmdDeltaToBuffer = Source::m_pClientSwap->VCall<WriteUsercmdDeltaToBufferFn>( 24 );

   auto backupNumCmds = msg->m_nNewCommands;

   auto NewCommands = msg->m_nNewCommands + tickbase_shift;
   if ( msg->m_nNewCommands > 62 )
	  NewCommands = 62;

   msg->m_nNewCommands = NewCommands;
   msg->m_nBackupCommands = 0;

   auto from = -1;
   auto nextCmdNr = Source::m_pClientState->m_nLastOutgoingCommand( ) + Source::m_pClientState->m_nChokedCommands( ) + 1;
   auto to = nextCmdNr - backupNumCmds + 1;
   while ( to <= nextCmdNr ) {
	  auto result = WriteUsercmdDeltaToBuffer( this0, nSlot, buffer, from, ++to, true );
	  from = to;
	  if ( !result )
		 return false;
   }

   auto v13 = Memory::VCall<CUserCmd * ( __thiscall* )( void*, int, int ) >( Source::m_pInput, 32 / 4 )( Source::m_pInput, nSlot, from );
   if ( !v13 )
	  return true;

   if ( backupNumCmds <= NewCommands ) {
	  CUserCmd from_cmd = *v13;
	  CUserCmd to_cmd = *v13;
	  ++to_cmd.command_number;
	  to_cmd.tick_count += 200;

	  auto totalNewCommands = NewCommands - backupNumCmds + 1;
	  do {
		 WriteUsercmd( ( bf_write* ) buffer, &to_cmd, &from_cmd );
		 from_cmd = to_cmd;
		 ++to_cmd.command_number;
		 ++to_cmd.tick_count;
		 --totalNewCommands;
	  } while ( totalNewCommands );
   }

   return true;
}
#endif

bool __fastcall Hooked::WriteUsercmdDeltaToBuffer( void* ECX, void* EDX, int nSlot, void* buffer, int o_from, int o_to, bool isnewcommand ) {
   g_Vars.globals.szLastHookCalled = XorStr( "37" );
#if 0
   static auto ret = Source::m_pClientSwap->VCall<WriteUsercmdDeltaToBufferFn>( 24 );

   uintptr_t framePtr;
   __asm mov framePtr, ebp;
   auto msg = reinterpret_cast< CCLCMsg_Move_t* >( framePtr + 0xFCC );

   if ( !g_Vars.globals.TickbaseShift )
	  return ret( ECX, nSlot, buffer, o_from, o_to, isnewcommand );

   if ( o_from != -1 )
	  return 1;

   auto tickbaseShift = ( g_Vars.globals.TickbaseShift );
   g_Vars.globals.TickbaseShift = 0;

   //if ( tickbaseShift < 0 )
	 // return TickbaseManipulation( msg, std::abs( tickbaseShift ), ECX, nSlot, buffer );

   msg->m_nBackupCommands = 0;

   auto newCmds = msg->m_nNewCommands;
   auto shiftedTickbase = newCmds - abs( tickbaseShift );
   if ( shiftedTickbase < 1 )
	  shiftedTickbase = 1;

   msg->m_nNewCommands = newCmds;
   newCmds = shiftedTickbase;

   auto from = -1;
   auto nextCmdNr = Source::m_pClientState->m_nChokedCommands( ) + Source::m_pClientState->m_nLastOutgoingCommand( ) + 1;
   auto to = nextCmdNr - shiftedTickbase + 1;
   if ( to > nextCmdNr ) {
   LABEL_12:
	  auto v14 = newCmds + g_Vars.globals.LagLimit - shiftedTickbase;
	  if ( v14 > 16 ) {
		 g_Vars.globals.LagLimit = 16;
		 return 1;
	  }

	  if ( v14 < 0 )
		 v14 = 0;

	  g_Vars.globals.LagLimit = v14;
	  return 1;
   }

   while ( ret( ECX, nSlot, buffer, from, to, true ) ) {
	  from = to++;
	  if ( to > nextCmdNr ) {
		 goto LABEL_12;
	  }
   }
#endif
   return 0;
}
