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

//extern std::map<int, Vector> predicted_origins;

#if 0
if ( predicted_origins.count( ucmd->command_number ) > 0 ) {
   const auto origin = player->m_vecOrigin( );
   const auto vel = player->m_vecVelocity( );
   const auto pre = predicted_origins[ ucmd->command_number ];

   const auto org_pred = std::get<0>( pre );
   const auto vel_pred = std::get<1>( pre );

   if ( org_pred.Distance( origin ) >= 0.01f ) {
	  printf( XorStr( "| ORG | len: %.4f | delta ( %.4f %.4f %.4f ) \n" ), org_pred.Distance( origin ),
		 org_pred.x - origin.x, org_pred.y - origin.y, org_pred.z - origin.z );

	  if ( InputSys::Get( )->IsKeyDown( VirtualKeys::H ) ) {
		 Source::m_pDebugOverlay->AddBoxOverlay( origin, Vector( -1.f, -1.f, -1.f ), Vector( 1.f, 1.f, 1.f ), QAngle( ), 255, 0, 0, 150, 1.0f );
		 Source::m_pDebugOverlay->AddBoxOverlay( org_pred, Vector( -1.f, -1.f, -1.f ), Vector( 1.f, 1.f, 1.f ), QAngle( ), 0, 0, 255, 150, 1.0f );
	  }
   }

   if ( vel_pred.Distance( vel ) >= 0.01f ) {
	  printf( XorStr( "| VEL | len: %.4f | delta ( %.4f %.4f %.4f ) \n" ), vel_pred.Distance( vel ),
		 vel_pred.x - vel.x, vel_pred.y - vel.y, vel_pred.z - vel.z );
   }

   predicted_origins.erase( ucmd->command_number );
}
#endif

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
