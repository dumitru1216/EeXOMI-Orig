#include "hooked.hpp"
#include "InputSys.hpp"
#include "Exploits.hpp"
#include "Displacement.hpp"
#include "ExtendedBactrack.hpp"
#include "player.hpp"
#include "TickbaseShift.hpp"

struct CIncomingSequence {
   int InSequence;
   int ReliableState;
};

std::vector<CIncomingSequence> IncomingSequences;

void WriteUsercmd( bf_write* buf, CUserCmd* incmd, CUserCmd* outcmd ) {
   __asm
   {
	  mov     ecx, buf
	  mov     edx, incmd
	  push    outcmd
	  call    Engine::Displacement.Function.m_WriteUsercmd
	  add     esp, 4
   }
}

#if 0
void BypassChokeLimit( CCLCMsg_Move_t* CL_Move, INetChannel* pNetChan ) {
   // not shifting or dont need do extra fakelag
   if ( TickbaseShiftCtx.commands_to_shift == 0 && ( CL_Move->m_nNewCommands != 15 || Source::m_pClientState->m_nChokedCommands( ) <= 14 ) )
	  return;

   using assign_lol = std::string& ( __thiscall* )( void*, uint8_t*, size_t );
   auto assign_std_autistic_string = ( assign_lol ) Engine::Displacement.Function.m_StdStringAssign;

   // rebuild CL_SendMove
   uint8_t data[ 4000 ];
   bf_write buf;
   buf.m_nDataBytes = 4000;
   buf.m_nDataBits = 32000;
   buf.m_pData = data;
   buf.m_iCurBit = false;
   buf.m_bOverflow = false;
   buf.m_bAssertOnOverflow = false;
   buf.m_pDebugName = false;
   int numCmd = Source::m_pClientState->m_nChokedCommands( ) + 1;
   int nextCmdNr = Source::m_pClientState->m_nLastOutgoingCommand( ) + numCmd;
   if ( numCmd > 62 )
	  numCmd = 62;

   bool bOk = true;

   auto to = nextCmdNr - numCmd + 1;
   auto from = -1;
   if ( to <= nextCmdNr ) {
	  int newcmdnr = to >= ( nextCmdNr - numCmd + 1 );
	  do {
		 bOk = bOk && Source::m_pInput->WriteUsercmdDeltaToBuffer( 0, &buf, from, to, to >= newcmdnr );
		 from = to++;
	  } while ( to <= nextCmdNr );
   }

   if ( bOk ) {
	  if ( TickbaseShiftCtx.commands_to_shift > 0 ) {
		 CUserCmd from_cmd, to_cmd;
		 from_cmd = Source::m_pInput->m_pCommands[ nextCmdNr % MULTIPLAYER_BACKUP ];
		 to_cmd = from_cmd;
		 to_cmd.tick_count = INT_MAX;

		 do {
			if ( numCmd >= 62 ) {
			   TickbaseShiftCtx.commands_to_shift = 0;
			   break;
			}

			to_cmd.command_number++;
			WriteUsercmd( &buf, &to_cmd, &from_cmd );

			TickbaseShiftCtx.commands_to_shift--;
			numCmd++;
		 } while ( TickbaseShiftCtx.commands_to_shift > 0 );
	  } else {
		 TickbaseShiftCtx.commands_to_shift = 0;
	  }

	  // bypass choke limit
	  CL_Move->m_nNewCommands = numCmd;
	  CL_Move->m_nBackupCommands = 0;

	  int curbit = ( buf.m_iCurBit + 7 ) >> 3;
	  assign_std_autistic_string( CL_Move->m_data, buf.m_pData, curbit );
   }
}
#endif

bool __fastcall Hooked::SendNetMsg( INetChannel* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice ) {
   g_Vars.globals.szLastHookCalled = XorStr( "33" );
   if ( pNetChan != Source::m_pEngine->GetNetChannelInfo( ) || !g_Vars.globals.HackIsReady )
	  return oSendNetMsg( pNetChan, msg, bForceReliable, bVoice );

   if ( g_Vars.misc.sv_pure_bypass && msg.GetType( ) == 14 ) // Return and don't send messsage if its FileCRCCheck
	  return false;

#if 0
   if ( msg.GetGroup( ) == 11 ) {
	  BypassChokeLimit( ( CCLCMsg_Move_t* ) & msg, pNetChan );
   } else 
#endif

   if ( msg.GetGroup( ) == 9 ) { // group 9 is VoiceData
	  // Fixing fakelag with voice
	  bVoice = true;
	  g_Vars.globals.VoiceEnable = true;
   } else
	  g_Vars.globals.VoiceEnable = false;

   return oSendNetMsg( pNetChan, msg, bForceReliable, bVoice );
}

int __fastcall Hooked::SendDatagram( INetChannel* pNetChan, void* edx, void* buf ) {
   g_Vars.globals.szLastHookCalled = XorStr( "33" );
   if ( pNetChan != Source::m_pEngine->GetNetChannelInfo( ) || !g_Vars.misc.extended_backtrack || !g_Vars.globals.HackIsReady )
	  return oSendDatagram( pNetChan, buf );

#if 1
   auto v10 = pNetChan->m_nInSequenceNr;
   auto v16 = pNetChan->m_nInReliableState;
   auto v17 = pNetChan->GetLatency( FLOW_OUTGOING );
   if ( v17 < g_Vars.misc.extended_backtrack_time ) {
	  auto v13 = pNetChan->m_nInSequenceNr - TIME_TO_TICKS( g_Vars.misc.extended_backtrack_time / 1000.0f - v17 );
	  pNetChan->m_nInSequenceNr = v13;
	  for ( auto& seq : IncomingSequences ) {
		 if ( seq.InSequence != v13 )
			continue;

		 pNetChan->m_nInReliableState = seq.ReliableState;
	  }
   }

   auto result = oSendDatagram( pNetChan, buf );
   pNetChan->m_nInSequenceNr = v10;
   pNetChan->m_nInReliableState = v16;
   return result;
#else

   const auto backup_in_seq = pNetChan->m_nInSequenceNr;

   IExtendenBacktrack::Get( )->SetSuitableInSequence( pNetChan );

   const auto original = oSendDatagram( pNetChan, buf );
   pNetChan->m_nInSequenceNr = backup_in_seq;

   return original;
#endif
}

void __fastcall Hooked::ProcessPacket( INetChannel* pNetChan, void* edx, void* packet, bool header ) {
   g_Vars.globals.szLastHookCalled = XorStr( "34" );
   oProcessPacket( pNetChan, packet, header );
   if ( pNetChan != Source::m_pEngine->GetNetChannelInfo( ) || !g_Vars.globals.HackIsReady ) {
	  return;
   }

   IncomingSequences.push_back( CIncomingSequence{ pNetChan->m_nInSequenceNr, pNetChan->m_nInReliableState } );
   for ( auto it = IncomingSequences.begin( ); it != IncomingSequences.end( ); ++it ) {
	  auto delta = abs( pNetChan->m_nInSequenceNr - it->InSequence );
	  if ( delta > 128 ) {
		 it = IncomingSequences.erase( it );
	  }
   }

#if 0
   const auto local = C_CSPlayer::GetLocalPlayer( );
   if ( Source::m_pEngine->IsInGame( ) && local && !local->IsDead( ) )
	  IExtendenBacktrack::Get( )->FlipState( pNetChan );
#endif
}

void __fastcall Hooked::Shutdown( INetChannel* pNetChan, void* EDX, const char* reason ) {
   g_Vars.globals.szLastHookCalled = XorStr( "35" );
#if 0
   Source::m_pNetChannelSwap.reset( );
   Source::m_pNetChannelSwap = nullptr;
#endif 
   return oShutdown( pNetChan, reason );
}
