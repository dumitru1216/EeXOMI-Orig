#include "TickbaseShift.hpp"
#include "source.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "hooked.hpp"
#include "Exploits.hpp"
#include "displacement.hpp"


#pragma optimize("", off)

c_exploits g_tickbase_control;

using namespace Source;
bool c_exploits::is_charged( ) {
	return m_charged_ticks >= m_max_process_ticks;
}

void c_exploits::cl_move( bool bFinalTick, float accumulated_extra_samples ) {

	m_double_tap = g_Vars.rage.exploit && g_Vars.rage.double_tap_bind.enabled;
	m_should_charge = fabsf( m_pGlobalVars->realtime - m_last_exploit_time ) > 0.5f;

	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );

	if ( !pLocal ) {
		m_double_tap = false;
		m_should_charge = true;
		m_charged = false;
		m_charged_ticks = 0;
		m_second_shot = false;
		return Hooked::oCL_Move( bFinalTick, accumulated_extra_samples );
	}

	if ( m_double_tap && m_should_charge && !is_charged( ) ) {
		++m_charged_ticks;
		m_charged = is_charged( );
		return;
	}


	Hooked::oCL_Move( bFinalTick, accumulated_extra_samples );
	return;
}

void c_exploits::handle_exploits( bool* send_packet, CUserCmd* pCmd ) {

	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );

	// reset this!!
	m_can_shift = false;

	if ( !pLocal )
		return;

	if ( !m_double_tap ) {

		if ( m_charged_ticks ) {
			m_shift_amount = 14;
			m_last_exploit_time = m_pGlobalVars->realtime;
			m_last_shift_time = m_pGlobalVars->realtime;
			m_should_charge = false;
			m_break_lc = false;
		}


		if ( TIME_TO_TICKS( fabsf( m_pGlobalVars->realtime - m_last_shift_time ) ) <= 32 )
			*send_packet = true;

		m_should_charge = true;
		m_charged = false;
		m_charged_ticks = 0;
		m_second_shot = false;
		return;
	}

	m_charged = is_charged( );
	*send_packet = true;

	// if (TIME_TO_TICKS(fabsf(m_pGlobalVars->realtime - m_last_shift_time)) < 14) 
	//	*bSendPacket = true;


	const auto pWeapon = reinterpret_cast< C_WeaponCSBaseGun* >( pLocal->m_hActiveWeapon( ).Get( ) );
	if ( !pWeapon )
		return;

	auto pWeaponData = pWeapon->GetCSWeaponData( );
	if ( !pWeaponData.IsValid( ) )
		return;

	if ( !m_charged ) {

		if ( pCmd->buttons & IN_ATTACK )
			m_last_shift_time = m_pGlobalVars->realtime;


		m_second_shot = false;
		return;
	}

	if ( pWeaponData->m_iWeaponType == WEAPONTYPE_C4 ||
		 /*pWeaponData->m_iWeaponType == WEAPONTYPE_KNIFE ||*/
		 pWeaponData->m_iWeaponType == WEAPONTYPE_GRENADE ) {
		return;
	}

	// can we shift?
	m_can_shift = true;// pLocal->CanShoot( ) && pLocal->CanShoot( false, 14 );

	// yes we can, apply then!!
	if ( pCmd->buttons & IN_ATTACK && m_can_shift ) {
		m_shift_amount = 14;
		m_last_exploit_time = m_pGlobalVars->realtime;
		m_last_shift_time = m_pGlobalVars->realtime;
		m_shift_command = pCmd->command_number;
		m_should_charge = false;
		m_break_lc = false;
		*send_packet = true;
	} 
	// else if ( g_Vars.rage.dt1 ) { // thats broken
	// 
	// 	if ( ++m_break_lc_cycle >= 14 )
	// 		m_break_lc_cycle = 0;
	// 
	// 	m_shift_amount = m_break_lc_cycle > 0 ? 16 : 0;
	// }

}

int c_exploits::calc_correction_ticks( ) const {

	static auto lol = m_pCvar->FindVar( "sv_clockcorrection_msecs" );

	if ( !lol )
		return -1;

	if ( m_pGlobalVars->maxClients <= 1 )
		return -1;

	return TIME_TO_TICKS( std::clamp( lol->GetFloat( ) / 1000.f, 0.f, 1.f ) );
}


int c_exploits::adjust_tick_base( const int old_new_cmds, const int total_new_cmds, const int delta ) const {
	auto ret = -1;

	auto player = C_CSPlayer::GetLocalPlayer( );
	const auto correction_ticks = calc_correction_ticks( );
	if ( correction_ticks != -1 ) {
		const auto& prev_local_data = g_tickbase_control.m_local_data[ m_pClientState->m_nLastCommandAck( ) % 150 ];
		if ( prev_local_data.m_spawn_time == player->m_flSpawnTime( ) ) {
			ret = prev_local_data.m_tick_base + 1;

			const auto tick_count = ret + old_new_cmds;

			const auto ideal_final_tick = tick_count + correction_ticks;

			const auto too_fast_limit = ideal_final_tick + correction_ticks;
			const auto too_slow_limit = ideal_final_tick - correction_ticks;

			const auto adjusted_final_tick = ret + total_new_cmds;

			if ( adjusted_final_tick > too_fast_limit
				 || adjusted_final_tick < too_slow_limit ) {
				ret = ideal_final_tick - total_new_cmds;
			}
		}
	}

	if ( ret != -1 )
		return ret;

	const auto& local_data = g_tickbase_control.m_local_data[ ( m_pClientState->m_nLastOutgoingCommand( ) + 1 ) % 150 ];

	return ( local_data.m_spawn_time == player->m_flSpawnTime( )
			 ? local_data.m_tick_base : player->m_nTickBase( ) ) - delta;
}


#pragma optimize("", on)