#include "Hooked.hpp"
#include "Movement.hpp"
#include "player.hpp"
#include "InputSys.hpp"
#include "Exploits.hpp"
#include "TickbaseShift.hpp"
#include "displacement.hpp"
#pragma optimize("", off)


void WriteUsercmdd( bf_write* buf, CUserCmd* incmd, CUserCmd* outcmd ) {
	__asm
	{
		mov     ecx, buf
		mov     edx, incmd
		push    outcmd
		call    Engine::Displacement.Function.m_WriteUsercmd
		add     esp, 4
	}
}

bool __fastcall Hooked::write_user_cmd_delta_to_buffer(
	void* ecx, void* edx,
	int slot, bf_write* buffer,
	int from, int to, bool is_new_cmd
) {


	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );
	if ( !pLocal || pLocal->IsDead( ) )
		return Hooked::oWriteUsercmdDeltaToBuffer( ( void* )ecx, slot, ( void* )buffer, from, to, true );

	if ( g_tickbase_control.m_shift_amount ) {

		if ( from == -1 ) {

			uintptr_t frame_ptr = 0;
			__asm mov frame_ptr, ebp;
			auto backup_commands = reinterpret_cast < int* > ( frame_ptr + 0xFD8 );
			auto new_commands = reinterpret_cast < int* > ( frame_ptr + 0xFDC );


			std::cout << "CLCMove called with " << std::to_string( g_tickbase_control.m_shift_amount ) << " shift\n";


			if ( g_tickbase_control.m_break_lc )
				g_tickbase_control.handle_break_lc( ecx, edx, slot, buffer, from, to, new_commands, backup_commands );
			else
				g_tickbase_control.handle_other_shift( ecx, edx, slot, buffer, from, to, new_commands, backup_commands );
		}

		return true;
	}

	if ( from == -1 ) {
		uintptr_t frame_ptr = 0;
		__asm mov frame_ptr, ebp;
		auto new_commands = reinterpret_cast < int* > ( frame_ptr + 0xFDC );


		const auto v86 = std::min( *new_commands + g_tickbase_control.m_charged_ticks, 16 );

		int v69{};

		const auto v70 = v86 - *new_commands;
		if ( v70 >= 0 )
			v69 = v70;

		g_tickbase_control.m_charged_ticks = v69;
	}

	return oWriteUsercmdDeltaToBuffer( ecx, slot, buffer, from, to, is_new_cmd );
}

using namespace Source;
void c_exploits::handle_break_lc( void* ecx, void* edx, const int slot, bf_write* buffer, int& from, int& to, int* m_new_cmds, int* m_backup_cmds ) {
	auto shift_amount = m_shift_amount;
	m_shift_amount = 0;

	const auto actual_cmds = std::min( *m_new_cmds + m_ticks_allowed, 16 );
	int ticks_allowed{};

	const auto who_tf = actual_cmds - *m_new_cmds;
	if ( who_tf >= 0 )
		ticks_allowed = who_tf;

	m_ticks_allowed = ticks_allowed;

	const auto old_new_cmds = *m_new_cmds;

	*m_new_cmds = std::clamp( *m_new_cmds + shift_amount, 1, 62 );
	*m_backup_cmds = 0;

	const auto next_cmd_number = m_pClientState->m_nLastOutgoingCommand( ) + m_pClientState->m_nChokedCommands( ) + 1;

	for ( to = next_cmd_number - old_new_cmds + 1; to <= next_cmd_number; ++to ) {
		if ( !Hooked::oWriteUsercmdDeltaToBuffer( ( void* )ecx, slot, ( void* )buffer, from, to, true ) )
			return;

		from = to;
	}

	for ( auto i = m_pClientState->m_nLastOutgoingCommand( ) + 1; i <= next_cmd_number; ++i )
		g_tickbase_control.m_local_data[ i % 150 ].m_shift_amount = shift_amount;

	const auto user_cmd = m_pInput->GetUserCmd( slot, from );
	if ( !user_cmd )
		return;

	CUserCmd from_user_cmd = *user_cmd, to_user_cmd = *user_cmd;

	if ( shift_amount ) {
		++to_user_cmd.command_number;
		to_user_cmd.tick_count = std::numeric_limits< int >::max( );

		std::cout << "processing fake cmds (break lc)\n";

		do {
			
			WriteUsercmdd( buffer, &to_user_cmd, &from_user_cmd );
			from_user_cmd = to_user_cmd;
			++to_user_cmd.command_number;

			--shift_amount;
		} while ( shift_amount );
	}
}

void c_exploits::handle_defensive_shift( void* ecx, void* edx, const int slot, bf_write* buffer, int& from, int& to, int* m_new_cmds, int* m_backup_cmds ) {
	auto shift_amount = m_shift_amount;
	m_shift_amount = 0;

	const auto actual_cmds = std::min( *m_new_cmds + m_ticks_allowed, 16 );

	int new_ticks_allowed{};

	auto wtf = actual_cmds - *m_new_cmds;
	wtf -= shift_amount;

	if ( wtf >= 0 )
		new_ticks_allowed = wtf;

	m_ticks_allowed = new_ticks_allowed;

	const auto old_new_cmds = *m_new_cmds;

	*m_new_cmds = std::clamp( *m_new_cmds + shift_amount, 1, 62 );
	*m_backup_cmds = 0;

	auto first_tick_base = adjust_tick_base( old_new_cmds, *m_new_cmds, shift_amount );
	const auto next_cmd_number = m_pClientState->m_nLastOutgoingCommand( ) + m_pClientState->m_nChokedCommands( ) + 1;

	for ( to = next_cmd_number - old_new_cmds + 1; to <= next_cmd_number; ++to ) {
		if ( !Hooked::oWriteUsercmdDeltaToBuffer( ( void* )ecx, slot, ( void* )buffer, from, to, true ) )
			return;

		from = to;
	}

	for ( auto i = m_pClientState->m_nLastOutgoingCommand( ) + 1; i <= next_cmd_number; ++i ) {
		auto& local_data = g_tickbase_control.m_local_data[ i % 150 ];

		local_data.m_shift_amount = 0;
		local_data.m_override_tick_base = true;
		local_data.m_restore_tick_base = local_data.m_restore_tick_base;//!write_real_cmds; thats mess
		local_data.m_adjusted_tick_base = first_tick_base++;
	}

	const auto user_cmd = m_pInput->GetUserCmd( slot, from );
	if ( !user_cmd ) {
		std::cout << "failed to get a usercmd, returning.\n";
		return;
	}

	CUserCmd from_user_cmd = *user_cmd, to_user_cmd = *user_cmd;

	++to_user_cmd.command_number;

	*( DWORD* )( m_pPrediction.Xor( ) + 0xC ) = -1;
	*( DWORD* )( m_pPrediction.Xor( ) + 0x1C ) = 0;

	Vector2D target_move{};
	
	// shift
	this->m_shift_cycle = true;

	++m_pClientState->m_nChokedCommands( );
	++m_pClientState->net_channel->m_nChokedPackets;
	++m_pClientState->net_channel->m_nOutSequenceNr;

	const auto& local_data = g_tickbase_control.m_local_data[ next_cmd_number % 150 ];
	target_move = { local_data.m_move.x, local_data.m_move.y };

	int shifted_cmds{};

	do {
		m_pPrediction->Update(
			m_pClientState->m_nDeltaTick( ),
			m_pClientState->m_nDeltaTick( ) > 0,
			m_pClientState->m_nLastCommandAck( ),
			m_pClientState->m_nLastOutgoingCommand( ) + m_pClientState->m_nChokedCommands( )
		);

		to_user_cmd.buttons &= ~0xFFBEFFF9;
		to_user_cmd.forwardmove = to_user_cmd.sidemove = {};

		auto g_local_player = C_CSPlayer::GetLocalPlayer( );
		if ( g_local_player && !g_local_player->IsDead( ) ) {
			/*
			 if ( m_type != 4
                    && !( to_user_cmd.m_buttons & valve::e_buttons::in_jump )
                    && g_local_player->self( )->flags( ) & valve::e_ent_flags::on_ground ) {
			*/

			if ( this->m_type != 4 && !( to_user_cmd.buttons & IN_JUMP ) && g_local_player->m_fFlags() & FL_ONGROUND ) {

			}
		}
	} while ( shifted_cmds < shift_amount );
}

void c_exploits::handle_other_shift( void* ecx, void* edx, const int slot, bf_write* buffer, int& from, int& to, int* m_new_cmds, int* m_backup_cmds ) {

	auto shift_amount = m_shift_amount;
	m_shift_amount = 0;

	const auto write_real_cmds = !m_break_lc;

	const auto v86 = std::min( *m_new_cmds + m_ticks_allowed, 16 );

	int v69{};

	auto v70 = v86 - *m_new_cmds;
	if ( write_real_cmds )
		v70 -= shift_amount;

	if ( v70 >= 0 )
		v69 = v70;

	m_ticks_allowed = v69;

	const auto old_new_cmds = *m_new_cmds;

	*m_new_cmds = std::clamp( *m_new_cmds + shift_amount, 1, 62 );
	*m_backup_cmds = 0;

	auto first_tick_base = adjust_tick_base( old_new_cmds, *m_new_cmds, shift_amount );

	const auto next_cmd_number = m_pClientState->m_nLastOutgoingCommand( ) + m_pClientState->m_nChokedCommands( ) + 1;

	for ( to = next_cmd_number - old_new_cmds + 1; to <= next_cmd_number; ++to ) {
		if ( !Hooked::oWriteUsercmdDeltaToBuffer( ( void* )ecx, slot, ( void* )buffer, from, to, true ) )
			return;

		from = to;
	}

	for ( auto i = m_pClientState->m_nLastOutgoingCommand( ) + 1; i <= next_cmd_number; ++i ) {
		auto& local_data = g_tickbase_control.m_local_data[ i % 150 ];

		local_data.m_shift_amount = write_real_cmds ? 0 : shift_amount;
		local_data.m_override_tick_base = true;
		local_data.m_restore_tick_base = !write_real_cmds;
		local_data.m_adjusted_tick_base = first_tick_base++;
	}

	const auto user_cmd = m_pInput->GetUserCmd( slot, from );
	if ( !user_cmd ) {
		std::cout << "failed to get a usercmd, returning.\n";
		return;
	}

	CUserCmd from_user_cmd = *user_cmd, to_user_cmd = *user_cmd;

	++to_user_cmd.command_number;

	*( DWORD* )( m_pPrediction.Xor( ) + 0xC ) = -1;
	*( DWORD* )( m_pPrediction.Xor( ) + 0x1C ) = 0;

	Vector2D target_move{};

	if ( write_real_cmds ) {

		std::cout << "processing real cmds.\n";
		++m_pClientState->m_nChokedCommands( );
		++m_pClientState->net_channel->m_nChokedPackets;
		++m_pClientState->net_channel->m_nOutSequenceNr;

		bool shifting = false;//m_type == 4;

		if ( !shifting ) {
			const auto& local_data = g_tickbase_control.m_local_data[ next_cmd_number % 150 ];
			target_move = { local_data.m_move.x, local_data.m_move.y };


			//if ( g_Vars.rage.dt2 ) {
			//	if ( std::abs( local_data.m_move.x ) > 10.f )
			//		target_move.x = std::copysign( 450.f, local_data.m_move.x );
			//
			//	if ( std::abs( local_data.m_move.y ) > 10.f )
			//		target_move.y = std::copysign( 450.f, local_data.m_move.y );
			//}
		}

		int v80{};

		do {
			m_pPrediction->Update(
				m_pClientState->m_nDeltaTick( ),
				m_pClientState->m_nDeltaTick( ) > 0,
				m_pClientState->m_nLastCommandAck( ),
				m_pClientState->m_nLastOutgoingCommand( ) + m_pClientState->m_nChokedCommands( )
			);

			to_user_cmd.buttons &= ~0xFFBEFFF9;
			to_user_cmd.forwardmove = to_user_cmd.sidemove = {};

			auto player = C_CSPlayer::GetLocalPlayer( );

			if ( player && !player->IsDead( ) ) {

				// extended teleport!!
				//if ( g_Vars.rage.dt2 ) {
				//	to_user_cmd.forwardmove = target_move.x;
				//	to_user_cmd.sidemove = target_move.y;
				//}



				// store revelant data here!! (woo)
				auto& local_data = g_tickbase_control.m_local_data[ to_user_cmd.command_number % 150 ];
				local_data.m_tick_base = local_data.m_adjusted_tick_base = player->m_nTickBase( );
				local_data.m_spawn_time = player->m_flSpawnTime( );
			}

			m_pInput->m_pCommands[ to_user_cmd.command_number % 150 ] = to_user_cmd;
			m_pInput->m_pVerifiedCommands[ to_user_cmd.command_number % 150 ] = { to_user_cmd, to_user_cmd.GetChecksum( ) };

			WriteUsercmdd( buffer, &to_user_cmd, &from_user_cmd );

			auto& local_data = g_tickbase_control.m_local_data[ to_user_cmd.command_number % 150 ];

			local_data.m_override_tick_base = true;
			local_data.m_adjusted_tick_base = first_tick_base++;

			++v80;


			if ( v80 >= shift_amount ) {

				// unload shit here ig
				g_tickbase_control.m_charged_ticks = 0;
				g_tickbase_control.m_second_shot = 1;
				g_tickbase_control.m_shift_to_fix = shift_amount;
				g_tickbase_control.m_should_fix = true;
				g_tickbase_control.m_is_charged = false;

				// set second shot as true
				g_tickbase_control.m_second_shot = true;
			} else {
				++m_pClientState->m_nChokedCommands( );
				++m_pClientState->net_channel->m_nChokedPackets;
				++m_pClientState->net_channel->m_nOutSequenceNr;
			}

			from_user_cmd = to_user_cmd;

			++to_user_cmd.command_number;
		} while ( v80 < shift_amount );
	} else {
		to_user_cmd.tick_count = std::numeric_limits< int >::max( );
		std::cout << "processing fake cmds.\n";
		do {
			WriteUsercmdd( buffer, &to_user_cmd, &from_user_cmd );

			from_user_cmd = to_user_cmd;
			++to_user_cmd.command_number;

			--shift_amount;
		} while ( shift_amount );
	}

}
#pragma optimize("", on)