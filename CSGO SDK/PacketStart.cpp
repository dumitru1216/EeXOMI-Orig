#pragma once
#include "Hooked.hpp"
#include "source.hpp"
#include "prediction.hpp"

#include "player.hpp"

void __fastcall Hooked::PacketStart( void* ECX, void* EDX, int incoming_sequence, int outgoing_acknowledged ) {
   g_Vars.globals.szLastHookCalled = XorStr( "19" );
   Engine::Prediction::Instance( )->PacketStart( incoming_sequence, outgoing_acknowledged );

#if 0
   for ( auto it = cmds_nr.begin( ); it != cmds_nr.end( ); it++ ) {
	  if ( *it == outgoing_acknowledged ) {
		 Source::m_pClientStateSwap->VCall<PacketStartFn>( 5 )( ECX, incoming_sequence, outgoing_acknowledged );
		 break; 
	  }
   }

   for ( auto it = cmds_nr.begin( ); it != cmds_nr.end( ); )
	  if ( *it < outgoing_acknowledged )
		 it = cmds_nr.erase( it );
	  else
		 ++it;
#endif
}

void __fastcall Hooked::PacketEnd( void* ECX, void* EDX ) {
   g_Vars.globals.szLastHookCalled = XorStr( "20" );
   Engine::Prediction::Instance( )->PacketCorrection( uintptr_t( ECX ) );
   oPacketEnd( ECX );
}