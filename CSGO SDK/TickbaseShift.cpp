#include "TickbaseShift.hpp"
#include "source.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "hooked.hpp"
#include "Exploits.hpp"
#include "displacement.hpp"

/* optimize off */
#pragma optimize("", off)

c_exploits g_tickbase_control;

using namespace Source;

/* return charged */
bool c_exploits::is_charged( ) {
	return m_charged_ticks >= m_max_process_ticks;
}

void c_exploits::cl_move( bool bFinalTick, float accumulated_extra_samples ) {
	m_double_tap = g_Vars.rage.exploit && g_Vars.rage.double_tap_bind.enabled;
	m_should_charge = fabsf( m_pGlobalVars->realtime - m_last_exploit_time ) > 0.5f;

	C_CSPlayer* p_local = C_CSPlayer::GetLocalPlayer( );
	if ( !p_local ) {
		/* reset */
		m_double_tap = false;
		m_should_charge = true;
		m_charged = false;
		m_charged_ticks = 0;
		m_second_shot = false;

		/* return call */
		return Hooked::oCL_Move( bFinalTick, accumulated_extra_samples );
	}

	if ( m_double_tap && m_should_charge && !is_charged( ) ) {
		++m_charged_ticks;
		m_charged = is_charged( );
		return;
	}
	
	return Hooked::oCL_Move( bFinalTick, accumulated_extra_samples ); // lets call this shit once 
}

void c_exploits::handle_exploits( bool* send_packet, CUserCmd* pCmd ) {

	C_CSPlayer* p_local = C_CSPlayer::GetLocalPlayer( );
	if ( !p_local )
		return;

	// reset this!!
	m_can_shift = false;

	if ( !m_double_tap ) {
		if ( m_charged_ticks ) {
			m_shift_amount = 14;
			m_last_exploit_time = m_pGlobalVars->realtime;
			m_last_shift_time = m_pGlobalVars->realtime;
			m_should_charge = false;
			m_break_lc = false;
		}
		
		/* recharge time */
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

	const auto pWeapon = reinterpret_cast< C_WeaponCSBaseGun* >( p_local->m_hActiveWeapon( ).Get( ) );
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

	if ( pWeaponData->m_iWeaponType == WEAPONTYPE_C4 || pWeaponData->m_iWeaponType == WEAPONTYPE_GRENADE ) {
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
}

int c_exploits::calc_correction_ticks( ) const { // should be cor
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

/* optimize on */
#pragma optimize("", on)


/* onetap */
#if 0
class c_networking {
    struct correct_packet_t {
        int m_cmd_number{ }, m_prev_cmd_num{ };
        bool m_is_used{ }, m_send_packet{ };
    };
    std::vector< correct_packet_t > m_packets{ };

    struct backup_t {
        inline void store( const bool choking, const int tick_count, const int cmd_num, backup_t* const last ) {
            auto& exploits = g_exploits->data( );
            m_choking = choking;
            m_tick_count = tick_count;
            m_cmd_num = cmd_num;

            m_next_tickbase = g_globals->m_local->tickbase( ) + 1;
            m_fixed_tickbase = exploits.m_fixed_tickbase;
            m_got_tickbase = exploits.m_shifting_tickbase;
            if ( m_got_tickbase )
                m_fixed_tickbase = exploits.m_pred_tick_base;
            m_ran_last = false;
            m_shooting = g_globals->m_shot_cmd_num == m_cmd_num;

            auto send_packet = m_choking == false;
            if ( send_packet ) {
                send_packet = !m_got_tickbase;
                m_choked_cmds = interfaces::client_state->m_choked_commands + 1;

                int shift{ };
                if ( send_packet || exploits.m_instant_dt && exploits.m_type != e_exploit_type::hideshots )
                    shift = 0;
                else
                    shift = exploits.m_cl_move_shift;

                m_tickbase_shift = shift;
                m_arrival_tick = g_globals->m_tick_count + g_globals->m_latency;
                m_corrected_data = false;
            }

            if ( last ) {
                m_pred_tickbase = g_exploits->get_corrected_tick(
                    last->m_pred_tickbase, m_tickbase_shift + interfaces::client_state->m_choked_commands + 1, 0, 0 );
            } else {
                m_pred_tickbase = g_globals->m_local->tickbase( ) + 1;
            }
        }

        int m_cmd_num{ }, m_tick_count{ }, m_next_tickbase{ }, m_fixed_tickbase{ }, m_choked_cmds{ },
            m_tickbase_shift{ }, m_arrival_tick{ }, m_pred_tickbase{ };
        bool m_choking{ }, m_got_tickbase{ }, m_corrected_data{ }, m_ran_last{ }, m_shooting{ };
    }**m_backup{ };
};

void __vectorcall cl_move( const float accumulated_extra_samples, const bool final_tick ) {
    static auto original = detour->original( &cl_move );

    const int out_sequence =
        interfaces::client_state->m_net_channel ? interfaces::client_state->m_net_channel->m_out_sequence_nr : 0;

    auto& exploits = g_exploits->data( );
    if ( g_globals->m_local->is_alive( ) ) {
        if ( g_exploits->on_clmove( ) ) {
            if ( !exploits.m_dt_charged ) {
                auto v15 = exploits.m_ticks_allowed_for_processing - interfaces::client_state->m_choked_commands;
                if ( exploits.m_ticks_allowed_for_processing > interfaces::client_state->m_choked_commands
                     && v15 - 4 <= 27 ) {
                    g_globals->m_shifting_tickbase = true;
                    exploits.m_in_cl_move = true;

                    do {
                        exploits.m_next_cmd = interfaces::client_state->m_last_outgoing_command + 2
                            + interfaces::client_state->m_choked_commands;
                        exploits.m_fl_ticks = 1;
                        exploits.m_choke = true;

                        original( accumulated_extra_samples, final_tick );
                        --v15;
                    } while ( interfaces::client_state->m_choked_commands < 15 && v15 > 3 );

                    exploits.m_choke = false;
                    exploits.m_send_packet = true;
                }
            }

            original( accumulated_extra_samples, final_tick );

            if ( exploits.m_dt_charged ) {
                if ( exploits.m_instant_dt ) {
                    if ( exploits.m_cl_move_shift >= 6 ) {
                        // in createmove hk
                        // if can fire -> m_shooting_in_choke_cycle = true
                        // if send packet -> m_shooting_in_choke_cycle = false
                        if ( g_globals->m_shooting_in_choke_cycle ) {
                            if ( const auto wpn = g_globals->m_local->active_weapon( ); wpn && !wpn->is_knife( ) ) {
                                int iteator = 0;

                                g_globals->m_shifting_tickbase = true;
                                exploits.m_choke = true;
                                exploits.m_in_cl_move = true;
                                if ( exploits.m_cl_move_shift > 0 ) {
                                    while ( interfaces::client_state->m_choked_commands < 15 ) {
                                        if ( iteator == exploits.m_cl_move_shift - 1 ) {
                                            exploits.m_choke = false;
                                            exploits.m_send_packet = true;
                                        }
                                        original( accumulated_extra_samples, final_tick );
                                        if ( ++iteator >= exploits.m_cl_move_shift ) {
                                            exploits.m_choke = false;
                                            exploits.m_send_packet = false;
                                            exploits.m_in_cl_move = false;
                                            g_globals->m_shifting_tickbase = false;

                                            if ( g_globals->m_local->flags( ) & 0x40 ) {
                                                sdk::c_net_channel* net_channel =
                                                    interfaces::client_state->m_net_channel;
                                                if ( net_channel ) {
                                                    const auto choked_packets = net_channel->m_choked_packets;
                                                    const auto out_seq_nr = net_channel->m_out_sequence_nr;

                                                    net_channel->m_choked_packets = 0;
                                                    net_channel->m_out_sequence_nr = out_sequence;
                                                    net_channel->send_datagram( );
                                                    net_channel->m_choked_packets = choked_packets;
                                                    net_channel->m_out_sequence_nr = out_seq_nr;
                                                }
                                            }
                                        }
                                    }
                                    exploits.m_choke = false;
                                    exploits.m_send_packet = true;
                                    original( accumulated_extra_samples, final_tick );
                                }
                            }
                        }
                    }
                }
            }
        } else {
            exploits.m_next_cmd =
                interfaces::client_state->m_last_outgoing_command + 1 + interfaces::client_state->m_choked_commands;
        }
    } else {
        original( accumulated_extra_samples, final_tick );
    }
}

void __fastcall runcommand( void* ecx, void* edx, sdk::c_cs_player* const player, sdk::c_user_cmd* const cmd,
                            sdk::i_move_helper* const move_helper ) {
    static auto original = detour->original( &runcommand );

    if ( !player || !g_globals->m_local || player != g_globals->m_local || !g_globals->m_local->is_alive( ) )
        return original( ecx, edx, player, cmd, move_helper );

    if ( g_exploits->runcommand_check( cmd->m_tickcount ) ) {
        g_globals->m_in_runcommand = true;
        player->tickbase( ) = g_networking->get_corrected_tickbase( ).at( cmd->m_command_number % 150 );
        interfaces::global_vars->m_curtime = TICKS_TO_TIME( player->tickbase( ) );

        // missed some netvars/revolver/sequence/original calls & fixes

        g_networking->get_stored_tickbase( ).at( cmd->m_command_number % 150 ) =
            player->tickbase( ); // used in packet end
        g_globals->m_in_runcommand = false;
    } else {
        cmd->m_has_been_predicted = true;
    }
}

bool __fastcall write_user_cmd_delta_to_buffer( void* ecx,
                                                void* edx,
                                                const int slot,
                                                void* const buf,
                                                const int from,
                                                const int to,
                                                const bool is_new_cmd ) {
    static auto original = vmt::client->original< decltype( &writeusercmddeltatobuffer ) >( 24 );

    auto& exploits = g_exploits->data( );
    if ( exploits.m_write_user_shift >= 0 || exploits.m_instant_dt )
        return original( ecx, edx, slot, buf, from, to, is_new_cmd );

    if ( from != -1 )
        return true;

    exploits.m_write_user_shift = 0;

    std::uintptr_t frame_ptr;
    __asm mov frame_ptr, ebp;

    int* backup_cmds = reinterpret_cast< int* >( frame_ptr + 0xFD8 );
    int* new_cmds = reinterpret_cast< int* >( frame_ptr + 0xFDC );
    int choked_modifier = exploits.m_write_user_shift + *new_cmds;

    *backup_cmds = 0;

    if ( choked_modifier > 62 )
        choked_modifier = 62;

    int final_from = -1;
    *new_cmds = choked_modifier;

    const int next_cmd_num =
        interfaces::client_state->m_choked_commands + interfaces::client_state->m_last_outgoing_command + 1;
    int final_to = next_cmd_num - *new_cmds + 1;

    if ( final_to > next_cmd_num ) {
LABEL_11:
        const sdk::c_user_cmd* cmd = interfaces::input->get_user_cmd( final_from );
        if ( cmd ) {
            sdk::c_user_cmd to_cmd = *cmd, from_cmd = *cmd;
            ++to_cmd.m_command_number;
            to_cmd.m_tickcount += g_globals->m_tickrate + 2 * g_globals->m_tickrate;

            if ( *new_cmds <= choked_modifier ) {
                int v18 = choked_modifier - *new_cmds + 1;
                do {
                    write_user_cmd( buf, &to_cmd, &from_cmd );
                    from_cmd = to_cmd;
                    --v18;
                } while ( v18 );
            }
        }
        return true;
    } else {
        while ( original( ecx, edx, slot, buf, final_from, final_to, true ) ) {
            final_from = final_to++;
            if ( final_to > next_cmd_num )
                goto LABEL_11;
        }
        return false;
    }
}

c_networking::backup_t* c_networking::get_unchoked_backup( ) {
    const auto v2 = m_backup_start + m_backup_end;
    if ( m_backup_start == v2 )
        return nullptr;

    while ( m_backup[ m_backup_start & ( m_backup_size - 1 ) ]->m_choking ) {
        if ( ++m_backup_start == v2 )
            return nullptr;
    }
    return m_backup[ m_backup_start & ( m_backup_size - 1 ) ];
}

void c_networking::apply_tickbase( ) {
    int fixed_tickbase{ };
    int next_tickbase = g_globals->m_local->tickbase( ) + 1;
    for ( auto i = m_backup_start + m_backup_end; m_backup_start != i; ++m_backup_start ) {
        const auto stored_backup = m_backup[ m_backup_start & ( m_backup_size - 1 ) ];
        fixed_tickbase = stored_backup->m_choking ? next_tickbase - 1 : stored_backup->m_pred_tickbase - 1;

        m_corrected_tickbase.at( stored_backup->m_cmd_num % 150 ) = fixed_tickbase;
        if ( interfaces::client_state->m_last_command_ack > stored_backup->m_cmd_num )
            break;
        next_tickbase = fixed_tickbase;
    }

    backup_t* latest_unchoked_backup{ };
    const int v13 = m_backup_start + m_backup_end;

    /* if vtable -> networking */
    if ( m_backup_start == v13 ) {
LABEL_15:
        latest_unchoked_backup = nullptr;
    } else {
        auto v15 = m_backup_size - 1;
        while ( true ) {
            latest_unchoked_backup = m_backup[ m_backup_start & v15 ];
            if ( !latest_unchoked_backup->m_choking )
                break;

            v15 = m_backup_size - 1;
            ++m_backup_start;

            if ( m_backup_start == v13 )
                goto LABEL_15;
        }
    }
    //} else {
    // latest_unchoked_backup = get_unchoked_backup( );
    //}

    if ( latest_unchoked_backup ) {
        int next_pred_tickbase = latest_unchoked_backup->m_pred_tickbase + 1;
        if ( g_globals->m_cmd_num >= g_globals->m_shot_cmd_num ) {
            if ( g_globals->m_shot_cmd_num > latest_unchoked_backup->m_cmd_num )
                next_pred_tickbase =
                latest_unchoked_backup->m_cmd_num - g_globals->m_shot_cmd_num + g_globals->m_shot_pred_tickbase + 2;
        }
        int next_cmd_num = latest_unchoked_backup->m_cmd_num + 1;
        if ( next_cmd_num <= g_globals->m_cmd_num ) {
            int final_tickbase = next_pred_tickbase - 1;
            do {
                const auto magic_num = ( 0x1B4E81B5 * next_cmd_num++ ) >> 32;
                g_networking->m_corrected_tickbase.at( next_cmd_num
                                                       - 150 * ( ( magic_num >> 4 ) + ( magic_num >> 31 ) ) ) =
                    final_tickbase++; // next_cmd_num + 1
            } while ( next_cmd_num <= g_globals->m_cmd_num );
        }
    } else {
        int v33{ };
        if ( g_globals->m_cmd_num > 0 ) {
            do {
                const auto cmd_num = g_globals->m_cmd_num - 150 * ( g_globals->m_cmd_num / 150 ) - v33++;
                g_networking->m_corrected_tickbase.at( cmd_num ) = g_globals->m_local->tickbase( );
                g_globals->m_cmd_num = --g_globals->m_cmd_num;
            } while ( v33 < g_globals->m_cmd_num );
        }
    }
}

void c_networking::on_cmd_end( ) {
    backup_t* last_backup{ };
    const int v15 = m_backup_start + m_backup_end;
    bool didnt_get_smth{ };
    int cmd{ };
    if ( m_backup_start != v15 ) {
        while ( true ) {
            auto v18 = m_backup[ m_backup_start & ( m_backup_size - 1 ) ];
            if ( !v18->m_choking )
                break;

            cmd = g_globals->m_cmd->m_command_number;
            didnt_get_smth = !v18->m_shooting;
        }
        last_backup = m_backup[ m_backup_size & ( m_backup_size - 1 ) ];
    }

    auto v28 = m_backup_size - 1;
    didnt_get_smth = ( ( m_backup_size - 1 ) & m_backup_start ) == 0;
    m_backup_start &= m_backup_size - 1;
    auto v29 = didnt_get_smth ? m_backup_size : m_backup_start;

    m_backup[ v28 & ( v29 - 1 ) ]->store( *g_globals->m_send_packet, g_globals->m_tick_count, cmd, last_backup );
    ++m_backup_end;
    m_backup_start = v29 - 1;

    auto backup_end = m_backup_end;
    if ( backup_end > 150 ) {
        do {
            backup_end = m_backup_end - 1;
            m_backup_end = backup_end;
            if ( !backup_end )
                m_backup_start = 0;
        } while ( backup_end > 150 );
    }

    // for packet start
    // todo: enrase vector when start packets == end packets
    auto& packets = m_packets.emplace_back( );

    packets.m_cmd_number = g_globals->m_cmd->m_command_number;
    packets.m_send_packet = *g_globals->m_send_packet;
    packets.m_is_used = false;
    packets.m_prev_cmd_num = 0;
}

void c_networking::on_packet_end( const int a2 ) {
    if ( !g_globals->m_local->is_alive( ) ) {
        init( );
        return;
    }

    if ( interfaces::client_state->m_clock_drift_manager.m_client_tick == interfaces::client_state->m_clock_drift_manager.m_server_tick ) {
        // get backup
        // cmd num < last out acknowledged && unchoked_backup->pred_tick > tickbase && recent_backup->pred_tickbase < tickbase
        // aim punch calculatons
        // aim punch < 0.1f -> apply_tickbase
        // and other....
    }
}

int c_exploits::get_corrected_tick( const int pred_tickbase, const int shift_cmds, const int a3, const int a4 ) {
    int v4 = a4;
    const int clock_correction = static_cast< int >( 0.3f / interfaces::global_vars->m_interval_per_tick + 0.5f );
    if ( !a4 )
        v4 = a3 + g_globals->m_tick_count + g_globals->m_latency;
    if ( pred_tickbase + shift_cmds > v4 + clock_correction + clock_correction || pred_tickbase + shift_cmds < v4 )
        return v4 + clock_correction - shift_cmds + 1;
    else
        return pred_tickbase;
}

void c_exploits::tickbase_shift( ) {
    int allowed_ticks{ };
    if ( !*g_globals->m_send_packet )
        allowed_ticks = m_data.m_ticks_allowed_for_processing;

    if ( g_globals->m_cmd->m_buttons & 1 ) {
        m_data.m_write_user_shift = -m_data.m_goal_shift_amount;
        m_data.m_not_exploiting = m_data.m_goal_shift_amount == 0;
        m_data.m_dtapping = m_data.m_type == e_exploit_type::doubletap && m_data.m_goal_shift_amount >= 6;
    }

    if ( interfaces::client_state->m_choked_commands < 0 )
        allowed_ticks = m_data.m_ticks_allowed_for_processing;

    int choked_packets = interfaces::client_state->m_choked_commands;
    int cmd_number = g_globals->m_cmd->m_command_number - choked_packets;
    int iterator = cmd_number;
    do {
        sdk::c_user_cmd* cmd =
            &interfaces::input
            ->m_commands[ g_globals->m_cmd->m_command_number - 150 * ( cmd_number / 150 ) - choked_packets ];

        if ( !command || runcommand_check( cmd->m_tickcount ) ) {
            if ( --allowed_ticks <= 0 )
                allowed_ticks = 0;
            m_data.m_ticks_allowed_for_processing = allowed_ticks;
        }

        cmd_number = iterator + 1;
        --choked_packets;
        ++iterator;
    } while ( choked_packets >= 0 );

    static const auto sv_maxusrcmdprocessticks_client =
        interfaces::convar->find_var( fnva1( "sv_maxusrcmdprocessticks_client" ) );

    if ( allowed_ticks > sv_maxusrcmdprocessticks_client->get_int( ) ) {
        m_data.m_ticks_allowed_for_processing = allowed_ticks - 1;
        if ( allowed_ticks - 1 > 0 ) {
            if ( const auto wpn = g_globals->m_local->active_weapon( ); wpn ) {
                if ( wpn->index( ) <= 499 ) {
                    switch ( wpn->index( ) ) {
                        case 43: // grenades
                        case 44:
                        case 45:
                        case 46:
                        case 47:
                        case 48:
                        case 84:
                            break;
                        default:
                            goto LABEL_36;
                    }
                } else {
LABEL_36:
                    m_data.m_next_cmd = g_globals->m_cmd->m_command_number;
                }

                if ( m_data.m_ticks_allowed_for_processing > sv_maxusrcmdprocessticks_client->get_int( ) )
                    m_data.m_ticks_allowed_for_processing = sv_maxusrcmdprocessticks_client->get_int( );
            }
        } else {
            m_data.m_ticks_allowed_for_processing = 0;

            if ( sv_maxusrcmdprocessticks_client->get_int( ) < 0 )
                m_data.m_ticks_allowed_for_processing = sv_maxusrcmdprocessticks_client->get_int( );
        }
    }
}

bool c_exploits::runcommand_check( const int tick_count ) {
    static const auto sv_max_usercmd_future_ticks =
        interfaces::convar->find_var( fnva1( "sv_max_usercmd_future_ticks" ) );
    return g_globals->m_tickrate + g_globals->m_tick_count + sv_max_usercmd_future_ticks->get_int( ) > tick_count;
}

void c_exploits::tickbase_fix( const int a2 ) {
    const int v4 = g_networking->get_backup_start( ) + g_networking->get_backup_end( );
    auto& backup = g_networking->get_fixed_backup( );

    /* if vtable -> networking */
    if ( g_networking->get_backup_start( ) == v4 ) {
        backup = nullptr;
    } else {
        auto v5 = g_networking->get_backup_size( ) - 1;
        while ( true ) {
            backup = g_networking->get_backup( )[ g_networking->get_backup_start( ) & v5 ];
            if ( !backup->m_choking )
                break;
            v5 = g_networking->get_backup_size( ) - 1;
            if ( ++g_networking->get_backup_start( ) == v4 )
                backup = nullptr;
        }
    }
    /*else*/
    // backup = g_networking->get_unchoked_backup( );

    if ( backup ) {
        int new_cmds = interfaces::client_state->m_choked_commands + a2 + 1;
        int tickbase{ };
        if ( backup->m_got_tickbase ) {
            tickbase = interfaces::client_state->m_choked_commands
                + get_corrected_tick( backup->m_pred_tickbase, new_cmds, a2, 0 );
        } else {
            tickbase = g_globals->m_local->tickbase( );
        }
        m_data.m_fixed_tickbase = tickbase;

        int goal_shift_amount{ };
        if ( !m_data.m_instant_dt || m_data.m_type == e_exploit_type::hideshots )
            goal_shift_amount = m_data.m_goal_shift_amount;
        else
            new_cmds += m_data.m_goal_shift_amount;

        m_data.m_pred_tick_base = interfaces::client_state->m_choked_commands
            + get_corrected_tick( backup->m_pred_tickbase, new_cmds + goal_shift_amount, a2, 0 );
    } else {
        const auto v24 = g_globals->m_local->tickbase( ) - m_data.m_goal_shift_amount;
        m_data.m_fixed_tickbase = g_globals->m_local->tickbase( );
        m_data.m_pred_tick_base = g_globals->m_local->tickbase( ) - m_data.m_goal_shift_amount;
        if ( m_data.m_goal_shift_amount <= 0 ) {
            if ( m_data.m_dtapping || m_data.m_prev_instant_dt )
                m_data.m_pred_tick_base = g_globals->m_local->tickbase( ) - 1;
        } else {
            m_data.m_pred_tick_base = v24 - 1;
        }
    }
}

void c_exploits::shift( ) {
    if ( m_data.m_goal_shift_amount <= 0 )
        m_data.m_prev_instant_dt = false;

    m_data.m_shifting_tickbase = true;
    m_data.m_cmd_num_on_shift = g_globals->m_cmd->m_command_number;
    m_data.m_tickbase_on_shift = m_data.m_pred_tick_base;
    m_data.m_cl_move_shift = m_data.m_goal_shift_amount;
    m_data.m_prev_instant_dt = !m_data.m_instant_dt || m_data.m_goal_shift_amount < 6 ? false : true;
}

void c_exploits::on_createmove( ) {
    bool backup_dtaping{ };
    if ( m_data.m_shifting_tickbase ) {
        m_data.m_goal_shift_amount = m_data.m_cl_move_shift;
        backup_dtaping = m_data.m_dtapping;
        m_data.m_dtapping = false;

        if ( m_data.m_cl_move_shift ) {
            m_data.m_dt_type = 0;
        }
    } else {
        m_data.m_goal_shift_amount = 0;
        backup_dtaping = m_data.m_dtapping;
        m_data.m_dtapping = false;
    }

    m_data.m_type = 0;
    m_data.m_dt_type = 0;

    int shift_amount{ };
    const auto wpn = g_globals->m_local->active_weapon( );
    if ( !wpn )
        return;

    if ( wpn->index( ) > 500 ) {
        shift_amount = wpn->is_knife( ) ? 5 : 7;
    } else if ( wpn->index( ) == 500 ) {
        shift_amount = 5;
    } else {
        switch ( wpn->index( ) ) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 0x1E:
            case 0x20:
            case 0x24:
            case 0x3d:
            case 0x3f:
            case 0x40:
                shift_amount = 1;
                break;
            case 7:
            case 8:
            case 0xA:
            case 0xd:
            case 0x10:
            case 0x27:
            case 0x3c:
                shift_amount = 0;
                break;
            case 9:
            case 0xB:
            case 0x26:
            case 0x28:
                shift_amount = 2;
                break;
            case 0xE:
            case 0x19:
            case 0x1B:
            case 0x1C:
            case 0x1d:
            case 0x23:
                shift_amount = 4;
                break;
            case 0x11:
            case 0x13:
            case 0x17:
            case 0x18:
            case 0x1A:
            case 0x21:
            case 0x22:
                shift_amount = 3;
                break;
            case 0x29:
            case 0x2A:
            case 0x3B:
                shift_amount = 5;
                break;
            case 0x2B:
            case 0x2C:
            case 0x2d:
            case 0x2E:
            case 0x2f | 0x8:
            case 0x30:
            case 0x54:
                shift_amount = 6;
                break;
            default:
                shift_amount = 7;
                break;
        }
    }

    if ( m_data.m_dt_charged ) {
        const bool hs = g_hotkeys->hideshots.m_active, dt = g_hotkeys->doubletap.m_active,
            v39 = m_data.m_ticks_allowed_for_processing == m_data.m_dt_ticks,
            v40 = m_data.m_ticks_allowed_for_processing - m_data.m_dt_ticks < 0;

        const int8_t v41 = math::overflow_flag_of_sub( m_data.m_ticks_allowed_for_processing, m_data.m_dt_ticks );
        m_data.m_fl_ticks = 1;

        if ( !wpn->burst_mode( ) && shift_amount != 6 && shift_amount != 5 && wpn->index( ) != 64 && wpn->index( ) != 31
             && !( v40 ^ v41 | v39 ) && !m_data.m_shifting_tickbase ) {
            if ( dt ) {
                m_data.m_type = e_exploit_type::doubletap;
                constexpr int dt_shift = 12;
                if ( const auto data = wpn->get_cs_weapon_info( ); data ) {
                    const auto unk_wpn_var =
                        *reinterpret_cast< int* >( reinterpret_cast< std::uintptr_t >( data ) + 0xdcu );
                    if ( TIME_TO_TICKS( unk_wpn_var ) < dt_shift ) {
                        if ( g_globals->m_local->can_fire( m_data.m_pred_tick_base, dt_shift ) )
                            m_data.m_prev_instant_dt = false;
                        else if ( !m_data.m_not_exploiting )
                            m_data.m_dtapping = backup_dtaping;

                        m_data.m_goal_shift_amount = 0;
                        m_data.m_dt_type = 0;
                        if ( wpn->index( ) != 35 && wpn->index( ) != 27 && wpn->index( ) != 29 && wpn->index( ) != 9
                             && wpn->index( ) != 40 ) {
                            m_data.m_dt_type = 2;
                            if ( g_globals->m_local->can_fire( m_data.m_pred_tick_base, dt_shift )
                                 || m_data.m_not_exploiting )
                                m_data.m_dt_type = 1;
                        }
                        return;
                    }
                }
            }

            if ( hs ) {
                m_data.m_type = e_exploit_type::hideshots;
                m_data.m_goal_shift_amount = 8;
            }
        }
    }
}

bool c_exploits::on_clmove( ) {
    ++m_data.m_ticks_allowed_for_processing;
    m_data.m_fl_ticks = 14;

    bool hs = g_settings->ragebot.hs && g_hotkeys->hideshots.m_active,
        dt = g_settings->ragebot.dt && g_hotkeys->doubletap.m_active, instant_shift = g_settings->ragebot.instant_shift,
        prev_dt_disabled = m_data.m_ran_dt == 0;

    m_data.m_instant_dt = instant_shift;
    prev_dt_disabled = m_data.m_prev_instant_dt == 0;
    m_data.m_ran_hs = hs;
    m_data.m_ran_dt = dt;

    bool force_instant_shift = false;

    if ( dt ) {
        if ( g_settings->ragebot.instant_shift && m_data.m_is_recharge
             && std::abs( g_globals->m_shot_cmd_num - g_globals->m_cmd_num ) < g_globals->m_tickrate ) {
            force_instant_shift = true;
        } else if ( !prev_dt_disabled && !hs ) {
            force_instant_shift = true;
        }
    } else if ( hs ) {
        force_instant_shift = false;
    }

    if ( force_instant_shift ) {
        m_data.m_dt_ticks = 12;
    }

    bool smt_check = g_settings->ragebot.enable;
    const bool hd_check = smt_check;
    if ( smt_check ) {
        if ( g_globals->m_local->flags( ) & 1 && g_settings->ragebot.enable && g_settings->antiaim.enable
             && g_hotkeys->fake_duck.m_active && g_globals->m_tickrate == 64 )
            smt_check = false;
        else if ( g_hotkeys->slow_walking.m_active )
            smt_check = false;
        else
            smt_check =
            ( !( g_globals->m_local->flags( ) & 1 ) || !g_settings->antiaim.enable || !g_hotkeys->slow_walking.m_active )
            && hd_check;
    }

    if ( !interfaces::client_state->m_choked_commands ) {
        if ( m_data.m_ticks_allowed_for_processing <= 0 ) {
            m_data.m_recharge = 0.0f;
        } else {
            const float v29 = static_cast< float >( m_data.m_ticks_allowed_for_processing / ( m_data.m_dt_ticks + 1 ) );
            if ( v29 <= 1.0f ) {
                float recharge_time{ };
                if ( v29 >= 0.0f )
                    recharge_time =
                    static_cast< float >( m_data.m_ticks_allowed_for_processing / ( m_data.m_dt_ticks + 1 ) );
                m_data.m_recharge = recharge_time;
            } else {
                m_data.m_recharge = 1.0f;
            }
        }
    }

    bool should_recharge{ };
    if ( m_data.m_recharge >= 1.0f )
        should_recharge = false;

    if ( force_instant_shift && smt_check ) {
        m_data.m_dt_charged = true;
        if ( m_data.m_ticks_allowed_for_processing <= m_data.m_dt_ticks && ( should_recharge || m_data.m_is_recharge ) )
            return false;
    } else {
        m_data.m_dt_charged = false;
    }

    return true;
}
#endif