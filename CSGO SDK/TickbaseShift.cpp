#include "TickbaseShift.hpp"
#include "source.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "hooked.hpp"
#include "displacement.hpp"

#if 0
C_TickbaseShift TickbaseShiftCtx;

bool C_TickbaseShift::ExploitsEnabled( ) {
   return g_Vars.rage.exploit && ( g_Vars.rage.double_tap_bind.enabled || g_Vars.rage.hide_shots_bind.enabled );
}

bool C_TickbaseShift::IsTickcountValid( int tick ) {
   return Source::m_pGlobalVars->tickcount + 64 + this->future_ticks > tick;
}

bool C_TickbaseShift::CanShiftTickbase( void ) {
   auto local = C_CSPlayer::GetLocalPlayer( );
   if ( !local )
	  return true;

  /* if ( Source::m_pGlobalVars->realtime - g_Vars.globals.m_flLastShotTime < 0.5f && ExploitsEnabled( ) && g_Vars.rage.double_tap_type == 1 )
	  return true;*/

   this->ticks_allowed++;

   if ( g_Vars.globals.m_iServerType ) {
	  if ( g_Vars.globals.m_iServerType < 8 )
		 this->max_choke = this->lag_limit = 6;
	  else
		 this->max_choke = this->lag_limit = 16;
   }

   g_Vars.fakelag.lag_limit = this->lag_limit;

   auto disabled_by_server = false;
   auto binds_activated = g_Vars.rage.hide_shots_bind.enabled || g_Vars.rage.double_tap_bind.enabled;
   if ( g_Vars.globals.m_iServerType < 8 ) {
	  disabled_by_server = true;
   } else if ( g_Vars.globals.Fakeducking || ( local->m_fFlags( ) & FL_ONGROUND && g_Vars.misc.fakeduck && g_Vars.misc.fakeduck_bind.enabled ) ) {
	  disabled_by_server = true;
   }

   this->exploits_enabled = g_Vars.rage.exploit && binds_activated && !disabled_by_server;
   return !this->exploits_enabled || this->ticks_allowed > this->max_tickbase_shift;
}

void C_TickbaseShift::SelectShift( void ) {
   auto local = C_CSPlayer::GetLocalPlayer( );
   if ( !local )
	  return;

   this->will_shift_tickbase = this->hold_tickbase_shift ? this->previous_tickbase_shift : 0;

   auto prev_dt_state = this->double_tapped;
   this->double_tapped = false;

   auto weapon = ( C_WeaponCSBaseGun* ) local->m_hActiveWeapon( ).Get( );

   if ( !weapon ) {
	  return;
   }

   auto data = weapon->GetCSWeaponData( );

   if ( this->exploits_enabled ) {
	  auto hide_shots_enabled = g_Vars.rage.hide_shots_bind.enabled;
	  auto double_tap_enabled = g_Vars.rage.double_tap_bind.enabled;

	  this->lag_limit = 1;

	  auto idx = weapon->m_iItemDefinitionIndex( );
	  if ( data->m_iWeaponType != WEAPONTYPE_KNIFE && data->m_iWeaponType != WEAPONTYPE_GRENADE
		   && idx != WEAPON_REVOLVER && idx != WEAPON_ZEUS /*&& !this->hold_tickbase_shift*/ ) {
		 if ( double_tap_enabled ) {
			auto rage_double_tap_fast_recovery = true;
			auto can_shifted_shoot = local->CanShoot( 13 );
			auto can_shoot = local->CanShoot( this->will_shift_tickbase );
			if ( can_shifted_shoot || !can_shoot && this->didnt_shift_tifckbase_previously ) {
			   this->will_shift_tickbase = /*g_Vars.rage.double_tap_type == 1 ? 0 :*/ 13;
			   this->double_tapped = false;
			} else {
			   this->will_shift_tickbase = 0;
			   this->double_tapped = prev_dt_state;
			}
		 } else if ( hide_shots_enabled ) {
			this->will_shift_tickbase = 9;
		 }
	  }
   }
}

void C_TickbaseShift::AdjustPlayerTimeBaseFix( int simulation_ticks ) {
   if ( simulation_ticks < 0 )
	  return;

   auto local = C_CSPlayer::GetLocalPlayer( );
   if ( !local )
	  return;

   // Start in the past so that we get to the sv.time that we'll hit at the end of the
   // frame, just as we process the final command

   if ( Source::m_pGlobalVars->maxClients == 1 ) {
	  // set TickBase so that player simulation tick matches gpGlobals->tickcount after
	  // all commands have been executed
	  local->m_nTickBase( ) = Source::m_pGlobalVars->tickcount - simulation_ticks + Source::m_pGlobalVars->simTicksThisFrame;
   } else { // multiplayer
	  float flCorrectionSeconds = Math::Clamp<float>( g_Vars.sv_clockcorrection_msecs->GetFloat( ) / 1000.0f, 0.0f, 1.0f );
	  int nCorrectionTicks = TIME_TO_TICKS( flCorrectionSeconds );

	  // Set the target tick flCorrectionSeconds (rounded to ticks) ahead in the future. this way the client can
	  // alternate around this target tick without getting smaller than gpGlobals->tickcount.
	  // After running the commands simulation time should be equal or after current gpGlobals->tickcount, 
	  // otherwise the simulation time drops out of the client side interpolated var history window.
	  int	nIdealFinalTick = Source::m_pGlobalVars->tickcount + nCorrectionTicks;

	  int nEstimatedFinalTick = local->m_nTickBase( ) + simulation_ticks;

	  // If client gets ahead of this, we'll need to correct
	  int	 too_fast_limit = nIdealFinalTick + nCorrectionTicks;
	  // If client falls behind this, we'll also need to correct
	  int	 too_slow_limit = nIdealFinalTick - nCorrectionTicks;

	  // See if we are too fast
	  if ( nEstimatedFinalTick > too_fast_limit ||
		   nEstimatedFinalTick < too_slow_limit ) {
		 int nCorrectedTick = nIdealFinalTick - simulation_ticks + Source::m_pGlobalVars->simTicksThisFrame;

		 local->m_nTickBase( ) = nCorrectedTick;
	  }
   }
}

void C_TickbaseShift::ApplyShift( Encrypted_t<CUserCmd> cmd, bool* bSendPacket ) {
   auto local = C_CSPlayer::GetLocalPlayer( );

   if ( !local )
	  return; 

   //if ( g_Vars.rage.double_tap_type == 1 && ExploitsEnabled( ) ) {
	  //if ( cmd->buttons & IN_ATTACK ) {
		 //auto v58 = cmd->command_number - 150 * ( ( cmd->command_number + 1 ) / 150 ) + 1;
		 //auto ccmd = &Source::m_pInput->m_pCommands[ v58 ];
		 //memcpy( ccmd, cmd.Xor( ), 0x64 );
		 //ccmd->command_number = cmd->command_number + 1;
		 //ccmd->buttons &= ~0x801u;
		 //auto commands_to_add = 0;
		 //do {
			//const auto v2 = commands_to_add + cmd->command_number;
			//auto command = &Source::m_pInput->m_pCommands[ v2 % 150 ];
			//auto v8 = &Source::m_pInput->m_pVerifiedCommands[ v2 % 150 ];

			//memcpy( command, cmd.Xor( ), 0x64 );
			//auto v7 = ( command->tick_count == 0x7F7FFFFF );
			//command->command_number = v2;
			//command->hasbeenpredicted = v7;
			//command->forwardmove = 0.f; //add an autostop to make second shot accurate? 
			//command->sidemove = 0.f; //add an autostop to make second shot accurate? 
		 //
			//memcpy( &v8->m_cmd, command, 0x64 );

			//v8->m_crc = command->GetChecksum( );

			//commands_to_add++;
		 //} while ( commands_to_add < 14 );

		 //Source::m_pClientState->m_nChokedCommands( ) += commands_to_add;

		 //*( DWORD* ) ( Source::m_pPrediction.Xor( ) + 0xC ) = -1;
		 //*( DWORD* ) ( Source::m_pPrediction.Xor( ) + 0x1C ) = 0;
	  //}
   //} else {
	  if ( *bSendPacket ) {
		 this->commands_to_shift = 0;
		 if ( g_Vars.globals.WasShooting || ( cmd->buttons & IN_ATTACK && local->CanShoot( ) ) ) {
			this->commands_to_shift = this->will_shift_tickbase;
			this->didnt_shift_tifckbase_previously = this->will_shift_tickbase == 0;
			this->double_tapped = this->will_shift_tickbase == 13;
		 }
	  }
   //}

   if ( *bSendPacket ) {
	  this->hold_tickbase_shift = 0;
	  auto v10 = Source::m_pClientState->m_nChokedCommands( );
	  if ( v10 >= 0 ) {
		 auto v11 = cmd->command_number - v10;
		 auto v33 = cmd->command_number - v10;
		 do {
			auto v12 = &Source::m_pInput->m_pCommands[ cmd->command_number - 150 * ( v11 / 150 ) - v10 ];
			if ( !v12 || IsTickcountValid( cmd->tick_count ) ) {
			   auto v13 = --this->ticks_allowed;
			   if ( v13 <= 0 )
				  v13 = 0;
			   this->ticks_allowed = v13;
			}
			v11 = v33 + 1;
			--v10;
			++v33;
		 } while ( v10 >= 0 );
	  }
   }

   auto v14 = this->ticks_allowed;
   if ( v14 > this->max_choke ) {
	  auto v15 = v14 - 1;
	  this->ticks_allowed = v15;
	  if ( v15 > 0 ) {
		 this->over_choke_nr = cmd->command_number;
	  } else {
		 this->ticks_allowed = 0;
	  }

	  auto v30 = this->max_choke;
	  if ( this->ticks_allowed > v30 )
		 this->ticks_allowed = v30;
   }

}

void C_TickbaseShift::OnNewPacket( int command_nr, int tickbase_from_server, int tickbase_from_record, int tickbase_shift ) {
   if ( this->over_choke_nr && command_nr >= this->over_choke_nr )
	  this->over_choke_nr = 0;

   if ( tickbase_shift <= 0 )
	  return;

   if ( tickbase_from_record - tickbase_shift < tickbase_from_server - 1 ) {
	  if ( tickbase_from_server > tickbase_from_record - tickbase_shift )
		 this->ticks_allowed = 0;
   } else {
	  C_CSPlayer::GetLocalPlayer( )->m_nTickBase( ) = tickbase_from_server;
	  Memory::VCall<void( __thiscall* )( void* )>( Source::m_pPrediction.Xor( ), 5 )( Source::m_pPrediction.Xor( ) );
   }
}
#endif

void TickbaseSystem::copy_command( CUserCmd* cmd, bool* v1 ) {
	if ( !s_InMovePrediction )
		return;

	static auto cl_forwardspeed = Source::m_pCvar->FindVar( XorStr( "cl_forwardspeed" ) );
	static auto cl_sidespeed = Source::m_pCvar->FindVar( XorStr( "cl_sidespeed" ) );

	/* boost movement function */
	auto BoostMovement = [ & ]( ) {
		/* get cmd move data */
		float& flForwardMove = cmd->forwardmove;
		float& flSideMove = cmd->sidemove;
		int& nButtons = cmd->buttons;

		/* Force .y movement */
		if ( flForwardMove > 5.0f ) {
			/* force buttons */
			nButtons |= IN_FORWARD;
			nButtons &= ~IN_BACK;

			/* force movement */
			flForwardMove = 450.0f;
		} else if ( flForwardMove < -5.0f ) {
			/* force buttons */
			nButtons |= IN_BACK;
			nButtons &= ~IN_FORWARD;

			/* force movement */
			flForwardMove = -450.0f;
		}

		/* Force .x movement */
		if ( flSideMove > 5.0f ) {
			/* force buttons */
			nButtons |= IN_MOVERIGHT;
			nButtons &= ~IN_MOVELEFT;

			/* force movement */
			flSideMove = 450.0f;
		} else if ( flSideMove < -5.0f ) {
			/* force buttons */
			nButtons |= IN_MOVELEFT;
			nButtons &= ~IN_MOVERIGHT;

			/* force movement */
			flSideMove = -450.0f;
		}

		/* do not slowdown */
		nButtons &= ~IN_SPEED;
		};
	BoostMovement( );
	*v1 = false;
	s_InMovePrediction = false;
}
void* g_pLocal = nullptr;
TickbaseSystem g_TickbaseController;

TickbaseShift_t::TickbaseShift_t( int _cmdnum, int _tickbase ) :
	cmdnum( _cmdnum ), tickbase( _tickbase ) {
	;
}

#define OFFSET_LASTOUTGOING 0x4CAC
#define OFFSET_CHOKED 0x4CB0
#define OFFSET_TICKBASE 0x3404

bool TickbaseSystem::IsTickcountValid( int nTick ) {
	return nTick >= ( Source::m_pGlobalVars->tickcount + int( 1 / Source::m_pGlobalVars->interval_per_tick ) + 8 );
}

void TickbaseSystem::OnCLMove( bool bFinalTick, float accumulated_extra_samples ) {
#ifndef STANDALONE_CSGO
	s_nTicksSinceUse++;
#endif

	//if we have low fps, we need to send our current batch 
	//before we can start building or we'll get a prediction error
	if ( !bFinalTick ) {
		s_bFreshFrame = false;
	}

	//can only start building on the final tick of this frame
	if ( ( !bFinalTick && !s_bBuilding ) || !g_pLocal ) {
		//level change; reset our shifts
		if ( !g_pLocal ) {
			g_iTickbaseShifts.clear( );
		}

		Hooked::oCL_Move( bFinalTick, accumulated_extra_samples );
		return;
	}


	const bool bStart = s_bBuilding;
	static bool doubleTapWasDisabled = false;
	bool doubleTapEnabled = g_Vars.rage.double_tap_bind.enabled && g_Vars.rage.exploit;
	s_bBuilding = g_Vars.rage.double_tap_bind.enabled && g_Vars.rage.exploit && s_nTicksSinceUse >= s_nTicksRequired && !m_bSupressRecharge;

	if ( doubleTapEnabled && doubleTapWasDisabled ) {
		s_flTime = 0.3f; /* apply fast recharge since this a desync hake for legacy lmao */
		printf( "[ tickbase CLMove ] Double tap just got enabled after being disabled.\n" );
		doubleTapWasDisabled = false;

	} else if ( !doubleTapEnabled && !doubleTapWasDisabled ) {
		printf( "[ tickbase CLMove ] Double tap just got disabled\n" );
		doubleTapWasDisabled = true;
	}

	if ( bStart && !s_bBuilding && ( ( s_nExtraProcessingTicks > 0 && !s_bAckedBuild ) || ( int )s_nExtraProcessingTicks < s_iClockCorrectionTicks ) ) {
		s_bBuilding = true;

		//wait for a fresh frame on low fps to start charging 
	} else if ( !bStart && s_bBuilding && !s_bFreshFrame ) {
		if ( bFinalTick ) {
			s_bFreshFrame = true;
		}

		s_iServerIdealTick++;
		Hooked::oCL_Move( bFinalTick, accumulated_extra_samples );
		return;
	}

	//do this afterwards so we catch when we're on the final tick
	//if we had multiple ticks this frame
	if ( bFinalTick ) {
		s_bFreshFrame = true;
	}

#ifndef STANDALONE_CSGO
	if ( !bStart && s_bBuilding ) {
		s_nTicksSinceStarted = 0;
	}

	if ( s_bBuilding ) {
		s_nTicksSinceStarted++;
		if ( s_nTicksSinceStarted <= s_nTicksDelay ) {
			/*lmao*/
			g_Vars.cl_interp->SetValue( 0 );
			s_iServerIdealTick++;
			charging = true;

			Hooked::oCL_Move( bFinalTick, accumulated_extra_samples );
			charging = false;

			g_Vars.cl_interp->SetValue( 1 );
			return;
		}
	}
#endif

	if ( s_bBuilding && s_nExtraProcessingTicks < s_nSpeed ) {
		s_bAckedBuild = false;


		//keep track of the server's ideal tickbase
		//note: should really do this once when host_currentframetick is 0 
		//but it doesn't matter since we don't do anything here anyway
		s_iServerIdealTick++;

		//allow time for this cmd to process; increase m_iTicksAllowedForProcessing
		s_nExtraProcessingTicks++;
		return;
	}
	int cmdnumber = 1;
	int choke = *( int* )( ( size_t )Source::m_pClientState.Xor( ) + OFFSET_CHOKED );
	cmdnumber += *( int* )( ( size_t )Source::m_pClientState.Xor( ) + OFFSET_LASTOUTGOING );
	cmdnumber += choke;

	//where the server wants us to be
	int arrive = s_iServerIdealTick;

	bool inya = false;
	//if we charged eg 15 ticks, our client's tickbase will be 15 ticks behind
	//and the next tick we send will cause our tickbase to be adjusted
	//so account for that
	if ( !s_bAckedBuild ) {
		s_bAckedBuild = true;

		//note that we should really be adding host_frameticks - 1 - host_currentframetick to estimated 
		//but that will only matter on low fps

		int estimated = *( int* )( ( size_t )g_pLocal + OFFSET_TICKBASE ) + **( int** )Engine::Displacement.Data.m_uHostFrameTicks + choke;
		if ( estimated > arrive + s_iClockCorrectionTicks || estimated < arrive - s_iClockCorrectionTicks ) {
			estimated = arrive - **( int** )Engine::Displacement.Data.m_uHostFrameTicks - choke + 1;
			g_iTickbaseShifts.emplace_back( cmdnumber, estimated );
		}

		Hooked::oCL_Move( bFinalTick, accumulated_extra_samples );

		//keep track of time
		s_iServerIdealTick++;
	}
	//if you want to have absolutely perfect prediction on low fps with doubletap, 
	//only double tap on the first tick of a frame with multiple ticks
	//this is because older ticks in the same frame will now be being run 
	//with a different tickbase if you shift later on
	//this does not really matter^^ which is why on shot anti aim with fakelag is a thing
	else if ( !s_bBuilding && s_nExtraProcessingTicks > 0 ) {
#ifndef STANDALONE_CSGO
		jmpRunExtraCommands :
#endif			

		allw = false;

		//the + 1 is because of the real command we are due
		int estimated = *( int* )( ( size_t )g_pLocal + OFFSET_TICKBASE ) + **( int** )Engine::Displacement.Data.m_uHostFrameTicks + s_nExtraProcessingTicks + choke;
		if ( estimated > arrive + s_iClockCorrectionTicks || estimated < arrive - s_iClockCorrectionTicks ) {
			estimated = arrive - s_nExtraProcessingTicks - choke - **( int** )Engine::Displacement.Data.m_uHostFrameTicks + 1;

			const size_t realcmd = **( int** )Engine::Displacement.Data.m_uHostFrameTicks + s_nExtraProcessingTicks + choke;
			for ( size_t i = 0; i < realcmd; i++ ) {
				//now account for the shift on all our new commands
				g_iTickbaseShifts.emplace_back( cmdnumber, estimated );

				cmdnumber++;
				estimated++;
			}

			//and then make sure the command after all of the above 
			//starts in the right spot
			//estimated = arrive + 1; // + 1 here??
			//g_iTickbaseShifts.emplace_back(cmdnumber, estimated);
		}

		if ( !inya ) {
			Hooked::oCL_Move( false, accumulated_extra_samples );

			__asm
			{
				xorps xmm0, xmm0
			}
		}

		while ( s_nExtraProcessingTicks > 0 ) {
			allw = false;

			bFinalTick = s_nExtraProcessingTicks == 1;
			bAllow = false;
			Hooked::oCL_Move( bFinalTick, 0.f );

			if ( inya ) {
				inya = false;
				__asm
				{
					xorps xmm0, xmm0
				}
			}

			s_nExtraProcessingTicks--;
		}


		//keep track of time
		s_iServerIdealTick++;

#ifndef STANDALONE_CSGO
		s_nTicksSinceUse = 0;
#endif
	} else {

		//otherwise copy the 'prestine' server ideal tick (m_nTickBase)
		//note that this will actually break on really low doubletap speeds 
		//(ie <= sv_clockcorrection_msecs because the server won't adjust your tickbase) 
		if ( g_iTickbaseShifts.size( ) ) {
			s_iServerIdealTick++;
		} else {
			s_iServerIdealTick = *( int* )( ( size_t )g_pLocal + OFFSET_TICKBASE ) + 1;
		}

#ifndef STANDALONE_CSGO
		int start = *( int* )( ( size_t )g_pLocal + OFFSET_TICKBASE );
		bool bPred = s_bBuilding && s_nExtraProcessingTicks > 0;

		if ( bPred ) {
			s_bInMove = true;
			s_iMoveTickBase = start;

			int estimated = start + s_nExtraProcessingTicks + **( int** )Engine::Displacement.Data.m_uHostFrameTicks + choke;
			if ( estimated > arrive + s_iClockCorrectionTicks || estimated < arrive - s_iClockCorrectionTicks ) {
				estimated = arrive - s_nExtraProcessingTicks - choke - **( int** )Engine::Displacement.Data.m_uHostFrameTicks + 1;

				s_iMoveTickBase = estimated;
				*( int* )( ( size_t )g_pLocal + OFFSET_TICKBASE ) = estimated;
			}
		} else {
			s_bInMove = false;
		}
#endif
		if ( !s_nExtraProcessingTicks )
			allw = true;

		// not building or it's not time to send
		Hooked::oCL_Move( bFinalTick, accumulated_extra_samples );

		if ( !s_nExtraProcessingTicks )
			allw = true;
#ifndef STANDALONE_CSGO
		if ( bPred ) {
			s_bInMove = false;
			*( int* )( ( size_t )g_pLocal + OFFSET_TICKBASE ) = start;

			if ( Source::m_pInput.Xor( ) ) {
				typedef CUserCmd* ( __thiscall* GetUserCmdFn_t )( void*, int, int );
				GetUserCmdFn_t fn = ( GetUserCmdFn_t )( ( *( void*** )Source::m_pInput.Xor( ) )[ 8 ] );
				if ( fn ) {
					CUserCmd* cmd = fn( Source::m_pInput.Xor( ), -1, cmdnumber );
					if ( cmd ) {

						auto local = C_CSPlayer::GetLocalPlayer( );

						C_WeaponCSBaseGun* Weapon = ( C_WeaponCSBaseGun* )local->m_hActiveWeapon( ).Get( );

						if ( !Weapon )
							return;

						auto weaponInfo = Weapon->GetCSWeaponData( );
						if ( !weaponInfo.IsValid( ) )
							return;

						if ( cmd->buttons & ( 1 << 0 ) && weaponInfo->m_iWeaponType != WEAPONTYPE_GRENADE && Weapon->m_iItemDefinitionIndex( ) != WEAPON_REVOLVER && Weapon->m_iItemDefinitionIndex( ) != WEAPON_C4 && m_bForceUnChargeState ) {
							s_InMovePrediction = true;//copy_command(cmd, s_nSpeed);
							s_bBuilding = false;
							inya = true;
							m_bSupressRecharge = true;
							allw = false;
							goto jmpRunExtraCommands;

						}
					}
				}
			}
		}
#endif
	}
}

// TODO: Move me
void InvokeRunSimulation( void* this_, float curtime, int cmdnum, CUserCmd* cmd, size_t local ) {
	__asm {
		push local
		push cmd
		push cmdnum

		movss xmm2, curtime
		mov ecx, this_

		call Hooked::RunSimulationDetor.m_pOldFunction
	}
}

void TickbaseSystem::OnRunSimulation( void* this_, int iCommandNumber, CUserCmd* pCmd, size_t local ) {
	g_pLocal = ( void* )local;

	float curtime;
	__asm
	{
		movss curtime, xmm2
	}

	for ( int i = 0; i < ( int )g_iTickbaseShifts.size( ); i++ ) {
		//ideally you compare the sequence we set this tickbase shift to
		//with the last acknowledged sequence
		if ( ( g_iTickbaseShifts[ i ].cmdnum < iCommandNumber - s_iNetBackup ) ||
			 ( g_iTickbaseShifts[ i ].cmdnum > iCommandNumber + s_iNetBackup ) ) {
			g_iTickbaseShifts.erase( g_iTickbaseShifts.begin( ) + i );
			i--;
		}
	}

	int tickbase = -1;
	for ( size_t i = 0; i < g_iTickbaseShifts.size( ); i++ ) {
		const auto& elem = g_iTickbaseShifts[ i ];

		if ( elem.cmdnum == iCommandNumber ) {
			tickbase = elem.tickbase;
			break;
		}
	}

	//apply our new shifted tickbase 
	if ( tickbase != -1 && local ) {

		*( int* )( local + OFFSET_TICKBASE ) = tickbase;
		curtime = tickbase * s_flTickInterval;
	}

	//run simulation is the perfect place to do this because
	//all other predictables (ie your weapon)
	//will be run in the right curtime 
	InvokeRunSimulation( this_, curtime, iCommandNumber, pCmd, local );
}

void TickbaseSystem::OnPredictionUpdate( void* prediction, void*, int startframe, bool validframe, int incoming_acknowledged, int outgoing_command ) {
	typedef void( __thiscall* PredictionUpdateFn_t )( void*, int, bool, int, int );
	PredictionUpdateFn_t fn = ( PredictionUpdateFn_t )Hooked::PredictionUpdateDetor.m_pOldFunction;
	fn( prediction, startframe, validframe, incoming_acknowledged, outgoing_command );

	if ( s_bInMove && g_pLocal ) {
		*( int* )( ( size_t )g_pLocal + OFFSET_TICKBASE ) = s_iMoveTickBase;
	}

	if ( g_pLocal ) {
		for ( size_t i = 0; i < g_iTickbaseShifts.size( ); i++ ) {
			const auto& elem = g_iTickbaseShifts[ i ];

			if ( elem.cmdnum == ( outgoing_command + 1 ) ) {
				//	*(int*)((size_t)g_pLocal + OFFSET_TICKBASE) = elem.tickbase;
				break;
			}

			if ( elem.cmdnum == ( outgoing_command ) ) {
				//*(int*)((size_t)g_pLocal + OFFSET_TICKBASE) = elem.tickbase;
				break;
			}

		}
	}
}

bool TickbaseSystem::Building( ) const {
	return s_bBuilding && !s_nExtraProcessingTicks && Source::m_pClientState->m_nChokedCommands( ) > 0;
}

bool TickbaseSystem::Using( ) const {
	return !s_bBuilding && s_nExtraProcessingTicks;
}
