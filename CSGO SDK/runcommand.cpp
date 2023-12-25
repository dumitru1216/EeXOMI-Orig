#include "Hooked.hpp"
#include "Prediction.hpp"
#include "weapon.hpp"
#include "CBaseHandle.hpp"
#include "player.hpp"
#include "Displacement.hpp"
#include "Movement.hpp"
#include <deque>
#include "TickbaseShift.hpp"

#include "InputSys.hpp"


// extern std::map<int, std::tuple<Vector, Vector>> predicted_origins;

extern int VelModTick;


void FixViewmodel( CUserCmd* cmd, bool restore ) {
   static float cycleBackup = 0.0f;
   static bool weaponAnimation = false;

   C_CSPlayer* player = C_CSPlayer::GetLocalPlayer( );
   auto viewModel = player->m_hViewModel( ).Get( );
   if ( viewModel ) {
	  if ( restore ) {
		 weaponAnimation = cmd->weaponselect > 0 || cmd->buttons & ( IN_ATTACK2 | IN_ATTACK );
		 cycleBackup = *( float* ) ( uintptr_t( viewModel ) + 0xA14 );
	  } else if ( weaponAnimation && !g_Vars.globals.FixCycle ) {
		 g_Vars.globals.FixCycle = *( float* ) ( uintptr_t( viewModel ) + 0xA14 ) == 0.0f && cycleBackup > 0.0f;
	  }
   }
}

namespace Hooked
{
   void __fastcall RunCommand( void* ecx, void* edx, C_CSPlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper ) {
	  g_Vars.globals.szLastHookCalled = XorStr( "32" );
	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || !player || player != local ) {
		 oRunCommand( ecx, player, ucmd, moveHelper );
		 return;
	  }

	  if ( ucmd->tick_count >= ( Source::m_pGlobalVars->tickcount + int( 1 / Source::m_pGlobalVars->interval_per_tick ) + 8 ) ) {
		  ucmd->hasbeenpredicted = true;
		  return;
	  }


	  auto weapon = ( C_WeaponCSBaseGun* ) local->m_hActiveWeapon( ).Get( );

	  FixViewmodel( ucmd, true );

	  auto backup = g_Vars.sv_show_impacts->GetInt( );
	  if ( g_Vars.misc.impacts_spoof ) {
		 g_Vars.sv_show_impacts->SetValue( 2 );
	  }

	  static float tickbase_records[ 150 ] = {};
	  static bool in_attack[ 150 ] = {};
	  static bool can_shoot_check[ 150 ] = {};
	  tickbase_records[ ucmd->command_number % 150 ] = player->m_nTickBase( );
	  in_attack[ ucmd->command_number % 150 ] = ( ucmd->buttons & ( IN_ATTACK2 | IN_ATTACK ) ) != 0;
	  can_shoot_check[ ucmd->command_number % 150 ] = player->CanShoot( 0, true );

	  auto fix_postpone_time = [player] ( int command_number ) {
		 auto weapon = ( C_WeaponCSBaseGun* ) player->m_hActiveWeapon( ).Get( );
		 if ( weapon ) {
			auto postpone = FLT_MAX;
			if ( weapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER ) {
			   auto tick_rate = int( 1.0f / Source::m_pGlobalVars->interval_per_tick );
			   if ( tick_rate >> 1 > 1 ) {
				  auto cmd_nr = command_number - 1;
				  auto shoot_nr = 0;
				  for ( int i = 1; i < tick_rate >> 1; ++i ) {
					 shoot_nr = cmd_nr;
					 if ( !in_attack[ cmd_nr % 150 ] || !can_shoot_check[ cmd_nr % 150 ] )
						break;

					 --cmd_nr;
				  }

				  if ( shoot_nr ) {
					 auto tick = 1 - ( signed int ) ( float ) ( -0.03348f / Source::m_pGlobalVars->interval_per_tick );
					 if ( command_number - shoot_nr >= tick )
						postpone = TICKS_TO_TIME( tickbase_records[ ( tick + shoot_nr ) % 150 ] ) + 0.2f;
				  }
			   }
			}

			weapon->m_flPostponeFireReadyTime( ) = postpone;
		 }
	  };
																													  
	  fix_postpone_time( ucmd->command_number );
	  oRunCommand( ecx, player, ucmd, moveHelper );

	  /* note from dutu:
		we could've fixed velocity modifier but i guess we wont
	  */

	  if ( g_Vars.misc.impacts_spoof ) {
		 g_Vars.sv_show_impacts->SetValue( backup );
	  }

	  FixViewmodel( ucmd, false );
	  
	  // cl_showerror fix
	  local->m_vphysicsCollisionState( ) = 0;

	  if ( !local->IsDead( ) ) {
		 auto& prediction = Engine::Prediction::Instance( );
		 prediction.OnRunCommand( local, ucmd );
	  }
   }
}
