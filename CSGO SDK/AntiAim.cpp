#include "AntiAim.hpp"
#include "CVariables.hpp"
#include "Movement.hpp"
#include "Source.hpp"
#include "InputSys.hpp"
#include "player.hpp"
#include "CBaseHandle.hpp"
#include "weapon.hpp"
#include "LagCompensation.hpp"
#include "Autowall.h"
#include "SimulationContext.hpp"
#include "displacement.hpp"
#include "Render.hpp"
#include "TickbaseShift.hpp"

bool MoveJitter = false;
Vector JitterOrigin;
extern int ShotsSwitch;
extern bool HideRealAfterShot;

namespace Source
{
   class C_AntiAimbot : public AntiAimbot {
   public:
	  void Main( bool* bSendPacket, Encrypted_t<CUserCmd> cmd, bool ragebot ) override;
	  void PrePrediction( bool* bSendPacket, Encrypted_t<CUserCmd> cmd ) override;
   private:
	  virtual float GetAntiAimX( Encrypted_t<CVariables::ANTIAIM_STATE> settings );
	  virtual float GetAntiAimY( Encrypted_t<CVariables::ANTIAIM_STATE> settings );

	  // virtual void Shift( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings );
	  virtual void DesyncAnimation( Encrypted_t<CUserCmd> cmd, bool* bSendPacket, Encrypted_t<CVariables::ANTIAIM_STATE> settings );
	  virtual bool AutoDirection( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings );
	  virtual bool RunAutoDirection( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings, bool only_desync );
	  virtual void AtTarget( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings );
	  virtual bool Edge( Encrypted_t<CUserCmd> cmd );

	  virtual bool IsEnabled( Encrypted_t<CUserCmd> cmd );

	  bool m_bAutomaticDir = true;
	  int m_iAutoDirection = false;
	  float m_flDirection = 0.0f;
	  float m_flLastDirection = 0.0f;
	  float m_flLastDirectionTime = 0.0f;

   #if 0
	  bool m_bDoShift = true;
   #endif

	  bool m_bInvertJitter = false;
	  bool m_bBreakLBY = false;
	  bool m_bInverted = false;
	  bool m_bFlipMove = false;

	  int m_bRechoke = 0;
	  int m_BalanceState = -1;
	  bool m_bBalanceMove = false;
	  bool m_bForceBreakLBY = false;
	  bool m_bSwitch = false;

	  float m_flSpin = 0.0f;
   };

   bool C_AntiAimbot::Edge( Encrypted_t<CUserCmd> cmd ) {
	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || local->IsDead( ) )
		 return false;

	  if ( local->m_MoveType( ) == 9 )
		 return false;

	  auto collision = local->m_Collision( );
	  auto maxs = collision->m_vecMaxs + Vector( 32.0f, 32.0f, 0.0f );
	  auto mins = collision->m_vecMins - Vector( 32.0f, 32.0f, 0.0f );

	  auto start = local->m_vecOrigin( );
	  start.z += ( ( ( 1.0f - local->m_flDuckAmount( ) ) * 18.0f ) + 38.0f );

	  auto obb_center = ( mins + maxs ) * 0.5f;

	  obb_center.z = 0.0f;

	  Ray_t ray;
	  ray.Init( start, start, mins, maxs );

	  CTraceFilterWorldAndPropsOnly filter;

	  CGameTrace trace;
	  Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );
	  if ( !trace.startsolid )
		 return false;

	  Vector best_plane;

	  float best_fraction = 1.0f;
	  float angle = 0.0f;
	  do {
		 Vector end = Vector( start.x + cosf( angle ) * 32.0f, start.y + sinf( angle ) * 32.0f,
							  start.z );

		 ray.Init( start, end );
		 Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );
		 if ( best_fraction > trace.fraction ) {
			best_plane = trace.plane.normal;
			best_fraction = trace.fraction;
		 }

		 angle += DirectX::XM_2PI / 20.0f;
	  } while ( angle <= DirectX::XM_2PI );
	  if ( best_fraction == 1.0f || best_plane.z >= 0.1f )
		 return false;

	  Vector plane = Vector( -best_plane.x, -best_plane.y, best_plane.z );
	  plane.Normalize( );

	  Vector plane_dir = start + plane * 24.0f;
	  plane_dir.z = start.z;

	  if ( Source::m_pEngineTrace->GetPointContents( plane_dir, MASK_SOLID ) & MASK_SOLID ) {
		 ray.Init( plane_dir, plane_dir + Vector( 0.0f, 0.0f, 16.0f ) );
		 Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );
		 if ( trace.fraction < 1.0f && !trace.startsolid && trace.plane.normal.z >= 0.7f ) {
			cmd->viewangles.yaw = std::remainderf( ToDegrees( atan2f( plane.y, plane.x ) ), 360.0f );
			return true;
		 }
	  }

   #if 0
	  if ( InputSys::Get( )->IsKeyDown( VirtualKeys::H ) ) {
		 Source::m_pDebugOverlay->AddLineOverlay( start, plane_dir, 255, 255, 255, false, 4.0f );
		 Source::m_pDebugOverlay->AddBoxOverlay( plane_dir, Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ), QAngle( 0.0f, 0.0f, 0.0f ), 255, 0, 0, 200, 4.0f );
	  }
   #endif
	  const float MAX_EDGE = 48.0f;

	  Vector v66 = Vector( plane_dir.x - ( best_plane.y * MAX_EDGE ), plane_dir.y - ( best_plane.x * MAX_EDGE ), plane_dir.z );

	  ray.Init( v66, plane_dir,
				Vector( plane * -3.0f - Vector( 1.0f, 1.0f, 1.0f ) ),
				Vector( plane * 3.0f + Vector( 1.0f, 1.0f, 1.0f ) )
	  );

	  Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

	  float PlaneNormalZ_1 = 0.0f;
	  if ( trace.startsolid )
		 PlaneNormalZ_1 = 0.0f;
	  else
		 PlaneNormalZ_1 = trace.fraction;

   #if 0
	  if ( InputSys::Get( )->IsKeyDown( VirtualKeys::H ) ) {
		 Source::m_pDebugOverlay->AddLineOverlay( v66, plane_dir, 255, 255, 255, false, 4.0f );
		 Source::m_pDebugOverlay->AddBoxOverlay( v66, Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ), QAngle( 0.0f, 0.0f, 0.0f ), 0, 255, 0, 200, 4.0f );
	  }
   #endif

	  Vector v67 = Vector( plane_dir.x + ( best_plane.y * MAX_EDGE ), plane_dir.y + ( best_plane.x * MAX_EDGE ), plane_dir.z );

	  ray.Init( v67, plane_dir,
				Vector( plane * -3.0f - Vector( 1.0f, 1.0f, 1.0f ) ),
				Vector( plane * 3.0f + Vector( 1.0f, 1.0f, 1.0f ) )
	  );

	  Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

	  float v37 = trace.startsolid ? 0.0f : trace.fraction;
	  if ( v37 == 0.0f && PlaneNormalZ_1 == 0.0f )
		 return false;

   #if 0
	  if ( InputSys::Get( )->IsKeyDown( VirtualKeys::H ) ) {
		 Source::m_pDebugOverlay->AddLineOverlay( v67, plane_dir, 255, 255, 255, false, 4.0f );
		 Source::m_pDebugOverlay->AddBoxOverlay( v67, Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ), QAngle( 0.0f, 0.0f, 0.0f ), 0, 0, 255, 200, 4.0f );
	  }
   #endif

	  float a1 = std::remainderf( cmd->viewangles.yaw - ( v66 - start ).ToEulerAngles( ).yaw, 360.0f );
	  float a2 = std::remainderf( cmd->viewangles.yaw - ( v67 - start ).ToEulerAngles( ).yaw, 360.0f );

	  if ( v37 < PlaneNormalZ_1 ) {
		 cmd->viewangles.yaw += ( a1 <= 0.f ) ? -90.0f : 90.0f;
	  } else {
		 cmd->viewangles.yaw += ( a2 > 0.f ) ? 90.0f : -90.0f;
	  }

   #if 0

	  constexpr float MAX_EDGE = 64.0f;
	  float fraction = 0.0f;
	  for ( float frac = -64.0f; frac <= 64.0f; frac += 4.0f ) {
		 Vector point = Vector( plane_dir.x + ( best_plane.y * frac ), plane_dir.y + ( best_plane.x * frac ), plane_dir.z );
		 if ( Source::m_pEngineTrace->GetPointContents_WorldOnly( point, MASK_SOLID ) ) {
			fraction += ( frac >= 0.f ) ? 4.0f : -4.0f;
		 }
	  }

	  if ( fraction == 0.0f )
		 return false;

	  cmd->viewangles.yaw = std::remainderf( ToDegrees( atan2f( best_plane.y, best_plane.x ) ), 360.0f );

	  if ( fraction > 0.0f ) {
		 cmd->viewangles.yaw += 90.0f;
	  } else {
		 cmd->viewangles.yaw -= 90.0f;
	  }
   #endif

	  return true;
   }

   bool C_AntiAimbot::IsEnabled( Encrypted_t<CUserCmd> cmd ) {
	  C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );
	  if ( !LocalPlayer || LocalPlayer->IsDead( ) || g_Vars.globals.HideShots )
		 return false;

	  if ( cmd->buttons & IN_USE )
		 return false;

	  if ( LocalPlayer->m_MoveType( ) == MOVETYPE_NOCLIP )
		 return false;

	  //if ( TickbaseShiftCtx.over_choke_nr )
		 //return false;

	  if ( g_Vars.antiaim.on_freeze_period ) {
		 static auto g_GameRules = *( uintptr_t** ) ( Engine::Displacement.Data.m_GameRules );
		 if ( *( bool* ) ( *( uintptr_t* ) g_GameRules + 0x20 ) )
			return false;
	  }

	  C_WeaponCSBaseGun* Weapon = ( C_WeaponCSBaseGun* ) LocalPlayer->m_hActiveWeapon( ).Get( );

	  if ( !Weapon )
		 return false;

	  auto WeaponInfo = Weapon->GetCSWeaponData( );
	  if ( !WeaponInfo.IsValid( ) )
		 return false;

	  if ( WeaponInfo->m_iWeaponType == WEAPONTYPE_GRENADE ) {
		 if ( g_Vars.antiaim.on_grenade )
			return false;

		 if ( !Weapon->m_bPinPulled( ) || ( cmd->buttons & ( IN_ATTACK | IN_ATTACK2 ) ) ) {
			float throwTime = Weapon->m_fThrowTime( );
			if ( throwTime > 0.f )
			   return false;
		 }
	  } else if ( cmd->buttons & IN_ATTACK ) {
		 if ( LocalPlayer->CanShoot( 0 ) )
			return false;
		 //g_Vars.globals.DidTickbaseExploit ? g_Vars.globals.TickbaseAmount : 0
	  }

	  if ( WeaponInfo->m_iWeaponType == WEAPONTYPE_KNIFE && Weapon->m_iItemDefinitionIndex( ) != WEAPON_ZEUS && g_Vars.antiaim.on_knife ) {
		 return false;
	  }

	  if ( g_Vars.antiaim.on_dormant ) {
		 bool found_player = false;
		 for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
			auto player = C_CSPlayer::GetPlayerByIndex( i );
			if ( !player || player->IsDead( ) || player->IsDormant( ) || player == LocalPlayer )
			   continue;

			if ( !LocalPlayer->IsTeammate( player ) ) {
			   found_player = true;
			   break;
			}
		 }

		 if ( !found_player )
			return false;
	  }

	  if ( LocalPlayer->m_MoveType( ) == MOVETYPE_LADDER )
		 if ( g_Vars.antiaim.on_ladder )
			return false;

	  return true;
   }

   Encrypted_t<AntiAimbot> AntiAimbot::Get( ) {
	  static C_AntiAimbot instance;
	  return &instance;
   }

   void C_AntiAimbot::Main( bool* bSendPacket, Encrypted_t<CUserCmd> cmd, bool ragebot ) {
	  C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

	  if ( !LocalPlayer || LocalPlayer->IsDead( ) )
		 return;

	  auto animState = LocalPlayer->m_PlayerAnimState( );
	  if ( !animState )
		 return;

	  if ( !g_Vars.antiaim.enabled || g_Vars.globals.WasShootingInChokeCycle || ( g_Vars.globals.m_iServerType == 1 && g_Vars.globals.m_iGameMode == 1 ) )
		 return;

	  //if ( TickbaseShiftCtx.double_tapped )
		 //return;

	  Encrypted_t<CVariables::ANTIAIM_STATE> settings( &g_Vars.antiaim_stand );

	  if ( !( LocalPlayer->m_fFlags( ) & FL_ONGROUND ) )
		 settings = &g_Vars.antiaim_air;
	  else if ( animState->m_velocity > 2.0f )
		 settings = &g_Vars.antiaim_move;

	  C_WeaponCSBaseGun* Weapon = ( C_WeaponCSBaseGun* ) LocalPlayer->m_hActiveWeapon( ).Get( );

	  if ( !Weapon )
		 return;

	  auto WeaponInfo = Weapon->GetCSWeaponData( );
	  if ( !WeaponInfo.IsValid( ) )
		 return;

	  if ( !IsEnabled( cmd ) )
		 return;

	  if ( LocalPlayer->m_MoveType( ) == MOVETYPE_LADDER ) {
		 auto eye_pos = LocalPlayer->GetEyePosition( );

		 CTraceFilterWorldAndPropsOnly filter;
		 CGameTrace tr;
		 Ray_t ray;
		 float angle = 0.0f;
		 while ( true ) {
			float cosa, sina;
			DirectX::XMScalarSinCos( &cosa, &sina, angle );

			Vector pos;
			pos.x = ( cosa * 32.0f ) + eye_pos.x;
			pos.y = ( sina * 32.0f ) + eye_pos.y;
			pos.z = eye_pos.z;

			ray.Init( eye_pos, pos,
					  Vector( -1.0f, -1.0f, -4.0f ),
					  Vector( 1.0f, 1.0f, 4.0f ) );
			Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &tr );
			if ( tr.fraction < 1.0f )
			   break;

			angle += DirectX::XM_PIDIV2;
			if ( angle >= DirectX::XM_2PI ) {
			   return;
			}
		 }

		 float v23 = atan2( tr.plane.normal.x, std::fabsf( tr.plane.normal.y ) );
		 float v24 = RAD2DEG( v23 ) + 90.0f;
		 cmd->viewangles.pitch = 89.0f;
		 if ( v24 <= 180.0f ) {
			if ( v24 < -180.0f ) {
			   v24 = v24 + 360.0f;
			}
			cmd->viewangles.yaw = v24;
		 } else {
			cmd->viewangles.yaw = v24 - 360.0f;
		 }

		 if ( cmd->buttons & IN_BACK ) {
			cmd->buttons |= IN_FORWARD;
			cmd->buttons &= ~IN_BACK;
		 } else  if ( cmd->buttons & IN_FORWARD ) {
			cmd->buttons |= IN_BACK;
			cmd->buttons &= ~IN_FORWARD;
		 }

		 return;
	  }

	  this->m_bAutomaticDir = false;

	  if ( settings->base_yaw == 1 ) {
		 AtTarget( cmd, settings );
	  } else if ( animState->m_velocity > 2.0f && settings->base_yaw == 2 ) {
		 cmd->viewangles.yaw = ToDegrees( std::atan2f( animState->m_vecVelocity.y, animState->m_vecVelocity.x ) );
	  }

	  float pitch = GetAntiAimX( settings );
	  if ( pitch != FLT_MAX )
		 cmd->viewangles.x = pitch;

	  /*
	  g_Vars.globals.manual_aa = ( m_iAutoDirection > 0 ) ? 2 : 0;
	  */

	  float yaw_backup = cmd->viewangles.yaw;
	  if ( g_Vars.antiaim.manual && g_Vars.globals.MouseOverrideEnabled ) {
		 RunAutoDirection( cmd, settings, true );
		 cmd->viewangles.yaw = std::remainderf( g_Vars.globals.MouseOverrideYaw + 180.0f, 360.0f );
	  } else {
		 float yaw = GetAntiAimY( settings );
		 if ( yaw != FLT_MAX ) {
			cmd->viewangles.y += yaw;
		 }

		 RunAutoDirection( cmd, settings, false );
	  }

	  cmd->viewangles.Normalize( );

	  if ( settings->spin ) {
		 float spin_speed = settings->spin_speed * 0.01f;
		 if ( settings->spin_switch ) {
			float range = settings->spin_range;
			if ( spin_speed < 0.f )
			   spin_speed = -spin_speed;

			if ( m_bSwitch ) {
			   m_flSpin += ( spin_speed * range );
			   if ( m_flSpin > range ) {
				  m_bSwitch = false;
				  m_flSpin = range;
			   }
			} else {
			   m_flSpin -= ( spin_speed * range );
			   if ( m_flSpin < -range ) {
				  m_bSwitch = true;
				  m_flSpin = -range;
			   }
			}
		 } else {
			if ( m_flSpin > settings->spin_range )
			   m_flSpin = -settings->spin_range;
			else if ( m_flSpin < -settings->spin_range )
			   m_flSpin = settings->spin_range;

			m_flSpin += spin_speed * settings->spin_range;
		 }

		 cmd->viewangles.yaw = std::remainderf( cmd->viewangles.yaw, 360.0f );
		 cmd->viewangles.yaw += m_flSpin;
	  }

	  if ( settings->jitter ) {
		 RandomSeed( ( cmd->command_number % 255 ) + 1 );
		 cmd->viewangles.yaw += RandomFloat( -settings->jitter_range, settings->jitter_range );
	  }

	  cmd->viewangles.Normalize( );

	  ///if ( settings->desync ) {
	//	 DesyncAnimation( cmd, bSendPacket, settings );
	 // }

   #if 0
	  if ( g_Vars.globals.TickbaseShift > 0 || ( *bSendPacket && g_Vars.antiaim.shift && ( settings->shift_pitch || settings->shift_yaw ) ) ) {
		 Shift( cmd, settings );
	  }
   #endif
   }

   void C_AntiAimbot::PrePrediction( bool* bSendPacket, Encrypted_t<CUserCmd> cmd ) {
	  bool prev_state = MoveJitter;
	  MoveJitter = false;

	  if ( !g_Vars.antiaim.enabled )
		 return;

	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || local->IsDead( ) )
		 return;

	  if ( cmd->sidemove != 0.0f || cmd->forwardmove != 0.0f || !( local->m_fFlags( ) & FL_ONGROUND ) || cmd->buttons & IN_JUMP || local->m_vecVelocity( ).Length2D( ) >= 4.5f )
		 return;

	  auto animState = local->m_PlayerAnimState( );
	  if ( !animState )
		 return;

	  Encrypted_t<CVariables::ANTIAIM_STATE> settings( &g_Vars.antiaim_stand );

	  if ( !settings->desync )
		 return;

	  if ( !IsEnabled( cmd ) )
		 return;

	  auto exploits_enabled = [] ( ) {
		 if ( !g_Vars.rage.exploit ) {
			return false;
		 }

		 if ( g_Vars.globals.Fakeducking )
			return false;

		 if ( g_Vars.rage.exploit_type == 0 ) {
			return g_Vars.rage.hide_shots_bind.enabled || g_Vars.rage.double_tap_bind.enabled;
		 }

		 return TickbaseShiftCtx.exploits_enabled;
	  };

	  float lbyUpdate = Source::Movement::Get( )->GetLBYUpdateTime( );

	  if ( g_Vars.antiaim.mirco_move_type == 0 || exploits_enabled( ) ) { // eye yaw
		 float move = ( local->m_vecViewOffset( ).z <= 63.0f ) ? 3.3f : 1.1f;
		 if ( !( cmd->command_number & 1 ) )
			move = move * -1.f;

		 if ( g_Vars.antiaim.break_lby && Source::m_pClientState->m_nChokedCommands( ) <= TickbaseShiftCtx.ticks_allowed &&
			  !exploits_enabled( ) ) {
			if ( TICKS_TO_TIME( local->m_nTickBase( ) ) >= lbyUpdate ) {
			   // force move, break lby, and then balance movement
			   cmd->forwardmove = move;
			   this->m_bForceBreakLBY = true;
			   m_bBalanceMove = !m_bBalanceMove;
			}
		 } else {
			cmd->forwardmove = move;
		 }
	  }

	  if ( cmd->forwardmove != 0.0f ) {
		 if ( !prev_state )
			JitterOrigin = local->m_vecOrigin( );
		 MoveJitter = true;
	  }
   }

   float C_AntiAimbot::GetAntiAimX( Encrypted_t<CVariables::ANTIAIM_STATE> settings ) {
	  switch ( settings->pitch ) {
		 case 1: // down
		 return 89.f;
		 case 2: // up 
		 return -89.f;
		 case 3: // zero
		 return 0.f;
		 default:
		 return FLT_MAX;
		 break;
	  }
   }

   float C_AntiAimbot::GetAntiAimY( Encrypted_t<CVariables::ANTIAIM_STATE> settings ) {
	  auto local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || local->IsDead( ) )
		 return 0.f;

	  if ( !g_Vars.antiaim.manual ) {
		 switch ( settings->yaw ) {
			case 0: // forward
			return 0.f; break;
			case 1: // backward
			return 180.f; break;
			default:
			break;
		 }
	  }

	  switch ( g_Vars.globals.manual_aa ) {
		 case 0:
		 return 90.f;
		 break;
		 case 1:
		 return 180.f;
		 break;
		 case 2:
		 return -90.f;
		 break;
	  }

	  return 180.0f;
   }

#if 0
   void C_AntiAimbot::Shift( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings ) {
	  if ( g_Vars.globals.TickbaseShift == 0 ) {
		 if ( m_bDoShift && cmd->command_number % 10 < 5 ) {
			return;
		 } else if ( !m_bDoShift ) {
			return;
		 }

		 g_Vars.globals.TickbaseShift = std::max( 6, Source::m_pClientState->m_nChokedCommands( ) + 4 );
	  }

	  g_Vars.globals.DoTickbaseShift = true;

	  switch ( settings->shift_pitch ) {
		 case 1: // inverse
		 cmd->viewangles.pitch -= 180.0f;
		 cmd->viewangles.pitch = std::remainderf( cmd->viewangles.pitch, 360.0f );
		 break;
		 case 2: // forward
		 cmd->viewangles.pitch = 0.0f;
		 break;
	  }

	  static float spin = 0.0f;

	  switch ( settings->shift_yaw ) {
		 case 1: // inverse
		 cmd->viewangles.yaw -= 180.0f;
		 cmd->viewangles.yaw = std::remainderf( cmd->viewangles.yaw, 360.0f );
		 break;
		 case 2: // static
		 break;
		 case 3: // spin
		 spin = std::remainderf( spin + settings->spin_speed, 360.0f );
		 cmd->viewangles.yaw = spin;
		 break;
	  }
   }
#endif

   void C_AntiAimbot::DesyncAnimation( Encrypted_t<CUserCmd> cmd, bool* bSendPacket, Encrypted_t<CVariables::ANTIAIM_STATE> settings ) {
	  enum DesyncAA {
		 Static = 1,
		 Jitter,
	  };

	  static bool force_choke = false;
	  float lbyUpdate = Source::Movement::Get( )->GetLBYUpdateTime( );

	  auto local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local )
		 return;

	  auto animState = local->m_PlayerAnimState( );
	  if ( !animState )
		 return;

	  if ( !m_bAutomaticDir ) {
		 if ( g_Vars.antiaim.move_sync && local->m_vecVelocity( ).Length2DSquared( ) > 135.f ) {
			float time = Source::m_pGlobalVars->curtime;
			static float last_switch_time = 0.f;

			if ( time > last_switch_time + 0.2f ) {
			   last_switch_time = time;
			   m_bInverted = !m_bInverted;
			}
		 } else
			m_bInverted = g_Vars.antiaim.desync_flip_bind.enabled;

		 if ( g_Vars.antiaim.hide_real_on_shot ) {
			if ( HideRealAfterShot ) {
			   m_bInverted = !m_bInverted;
			}
		 }
	  }

	  float m_flDesyncSide = -1.0f;
	  if ( m_bInverted )
		 m_flDesyncSide = 1.0f;

	  auto desync = animState->GetDesyncDelta( );
	  auto half_desync = desync * 0.5f;

	  if ( settings->desync == Jitter ) {
		 if ( m_bInvertJitter )
			cmd->viewangles.yaw = std::remainderf( cmd->viewangles.y - 180.0f, 360.0f );
	  } else {
		 float desync_amount = desync * ( m_bInverted ? settings->desync_amount_flipped * 0.01f : settings->desync_amount * 0.01f );
		 cmd->viewangles.y = m_bInverted ? std::remainderf( cmd->viewangles.y - desync_amount, 360.0f )
			: std::remainderf( cmd->viewangles.y + desync_amount, 360.0f );
	  }

	  if ( g_Vars.antiaim.mirco_move_type == 1 && Source::m_pClientState->m_nChokedCommands( ) == 0 && animState->m_velocity < 7.f &&
		   !TickbaseShiftCtx.ExploitsEnabled( ) ) {
		 cmd->viewangles.y = Math::AngleNormalize( local->m_angEyeAngles( ).yaw - 180.f );
		 bSendPacket = false;

		 float move = ( local->m_vecViewOffset( ).z <= 63.0f ) ? 4.941177f : 1.01f;
		 if ( !( cmd->command_number & 1 ) )
			move = move * -1.f;
		 cmd->forwardmove = move;

		 force_choke = true;
	  } else {
		 if ( HideRealAfterShot ) {
			if ( settings->desync == Jitter ) {
			   if ( !m_bInvertJitter )
				  cmd->viewangles.yaw += half_desync * m_flDesyncSide;
			   else
				  cmd->viewangles.yaw -= half_desync * m_flDesyncSide;
			   cmd->viewangles.Normalize( );
			}

			return;
		 }

	  #if 0
		 if ( settings->desync == RealAround ) {
			if ( !m_bInverted )
			   cmd->viewangles.yaw -= desync * ( settings->desync_amount * 0.01f );
			else
			   cmd->viewangles.yaw += desync * ( settings->desync_amount_flipped * 0.01f );
		 } else if ( settings->desync == FakeAround ) {
			if ( !m_bInverted )
			   cmd->viewangles.yaw += desync * ( settings->desync_amount * 0.01f );
			else
			   cmd->viewangles.yaw -= desync * ( settings->desync_amount_flipped * 0.01f );
		 }
	  #endif

		 int break_lby_type = g_Vars.antiaim.break_lby;

		 if ( g_Vars.antiaim.mirco_move_type == 1 /*&& TickbaseShiftCtx.ExploitsEnabled( )*/ ) {
			break_lby_type = 0;
		 }

		 if ( Source::m_pClientState->m_nChokedCommands( ) >= g_Vars.fakelag.lag_limit || !m_bForceBreakLBY ) {
			if ( Source::m_pClientState->m_nChokedCommands( ) >= g_Vars.fakelag.lag_limit
				 || ( !m_bBreakLBY && *bSendPacket && Source::m_pClientState->m_nChokedCommands( ) > 0 ) ) {
			   m_bInvertJitter = !m_bInvertJitter;
			   if ( settings->desync == Jitter ) {
				  if ( !m_bInvertJitter )
					 cmd->viewangles.yaw += half_desync * m_flDesyncSide;
				  else
					 cmd->viewangles.yaw -= half_desync * m_flDesyncSide;
				  cmd->viewangles.Normalize( );
			   }

			   return;
			}

			if ( settings->desync == Jitter ) {
			   RandomSeed( cmd->command_number );
			   if ( m_bInvertJitter ) {
				  cmd->viewangles.yaw -= ( desync + 3.0f ) * m_flDesyncSide;
			   } else {
				  cmd->viewangles.yaw += ( desync + 3.0f ) * m_flDesyncSide;
			   }
			} else {
			   if ( break_lby_type == 3 && local->m_vecVelocity( ).Length2D( ) < 2.0f ) {
				  cmd->viewangles.yaw += ( desync + 3.0f ) * m_flDesyncSide;
			   } else {
				  float desync_offset = ( Source::m_pClientState->m_nChokedCommands( ) >= 1 && local->m_vecVelocity( ).Length2D( ) >= 2.0f ) ? desync : std::min( 178.0f - desync, desync * 2.0f );
				  cmd->viewangles.yaw += desync_offset * m_flDesyncSide;
			   }
			}

			m_bBreakLBY = false;
			*bSendPacket = false;
		 } else {
			// sway
			auto dsc_side = m_flDesyncSide;
			if ( break_lby_type == 2 ) {
			   static auto last_tick = 0;
			   static auto invert = false;
			   // 0.6 = 60 / (100 / 64) / 64
			   // auto approach_speed = Source::m_pGlobalVars->interval_per_tick * 100.0f;
			   // 60.0f / ( 100.0f / Source::m_pGlobalVars->interval_per_tick ) / ( 1.0f / Source::m_pGlobalVars->interval_per_tick )
			   if ( TICKS_TO_TIME( std::abs( Source::m_pGlobalVars->tickcount - last_tick ) ) >= ( desync + 10.0f ) * 0.01f ) {
				  last_tick = Source::m_pGlobalVars->tickcount;
				  invert = !invert;
			   }

			   if ( invert )
				  dsc_side = -m_flDesyncSide;
			}

			if ( settings->desync == Jitter ) {
			   if ( m_bInvertJitter ) {
				  cmd->viewangles.yaw += 90.0f * dsc_side;
			   } else {
				  cmd->viewangles.yaw -= 90.0f * dsc_side;
			   }
			} else {
			   cmd->viewangles.yaw -= ( desync + 30.0f ) * dsc_side;
			}

			m_bBreakLBY = true; // break abs rotation again after breaking lby
			m_bForceBreakLBY = false;
			*bSendPacket = false;
		 }
	  }

	  if ( force_choke ) {
		 bSendPacket = false;
		 force_choke = false;
	  }

	  cmd->viewangles.Normalize( );
   }

   bool C_AntiAimbot::AutoDirection( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings ) {
	  struct edgy_sort {
		 C_CSPlayer* pl;
		 float dst;
		 Vector eye;

		 bool operator<( const edgy_sort& a ) const {
			return dst < a.dst;
		 }
	  };

	  C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

	  if ( !LocalPlayer || LocalPlayer->IsDead( ) )
		 return false;

	  auto pWeapon = reinterpret_cast< C_WeaponCSBaseGun* >( LocalPlayer->m_hActiveWeapon( ).Get( ) );
	  if ( !pWeapon )
		 return false;

	  auto pWeaponData = pWeapon->GetCSWeaponData( );
	  if ( !pWeaponData.IsValid( ) )
		 return false;

	  float maxSpeed = 250.0f;
	  if ( pWeapon ) {
		 if ( pWeaponData.IsValid( ) ) {
			if ( pWeapon->m_weaponMode( ) == 0 )
			   maxSpeed = pWeaponData->m_flMaxSpeed;
			else
			   maxSpeed = pWeaponData->m_flMaxSpeed2;
		 }
	  }

	  auto viewHeight = LocalPlayer->GetEyePosition( ) - LocalPlayer->m_vecOrigin( );
	  Vector eyePos = ( settings->autodirection_ignore_duck ) ? LocalPlayer->m_vecOrigin( ) + Vector( 0.0f, 0.0f, 58.0f ) : LocalPlayer->GetEyePosition( );

	  std::vector< edgy_sort > players_sorted;
	  for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
		 auto player = C_CSPlayer::GetPlayerByIndex( i );
		 if ( !player || player->IsDormant( ) || player->IsDead( ) )
			continue;

		 bool is_team = player->IsTeammate( LocalPlayer );
		 if ( is_team /*&& !Engine::LagCompensation::Get( )->BacktrackTeam( ) */ )
			continue;

		 auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
		 if ( !lag_data.IsValid( ) )
			continue;

		 if ( player->IsReloading( ) )
			continue;

		 auto wpn = ( C_WeaponCSBaseGun* ) player->m_hActiveWeapon( ).Get( );
		 if ( !wpn || wpn->m_iClip1( ) <= 0 )
			continue;

		 auto& pl = players_sorted.emplace_back( );
		 pl.pl = player;
		 pl.eye = lag_data->GetExtrapolatedPosition( player ) + player->m_vecViewOffset( );

		 Vector dir = pl.eye - eyePos;
		 pl.dst = dir.Normalize( );
	  }

	  if ( players_sorted.empty( ) ) {
		 if ( settings->autodirection && Edge( cmd ) ) {
			cmd->viewangles.y = std::remainderf( cmd->viewangles.y, 360.0f );
			return true;
		 }

		 return false;
	  }

   #if 0
	  C_SimulationData data;
	  data.m_bJumped = cmd->buttons & IN_JUMP;
	  data.m_iFlags = LocalPlayer->m_fFlags( );
	  data.m_player = LocalPlayer;
	  data.m_vecOrigin = LocalPlayer->m_vecOrigin( );
	  data.m_vecVeloctity = LocalPlayer->m_vecVelocity( );

	  if ( settings->extrapolation_ticks > 0 ) {
		 Vector wishvel;
		 Vector wishdir;
		 Vector right, up;
		 Vector forward = cmd->viewangles.ToVectors( &right, &up );

		 for ( int i = 0; i < 2; i++ )       // Determine x and y parts of velocity
			wishvel[ i ] = forward[ i ] * cmd->forwardmove + right[ i ] * cmd->sidemove;

		 float wishspeed = wishvel.Normalize( );
		 ExtrapolatePlayer( data, settings->extrapolation_ticks, wishvel, wishspeed, maxSpeed );
	  }
   #else
	  SimulationContext data;
	  data.InitSimulationContext( LocalPlayer );
	  for ( int i = 0; i < settings->extrapolation_ticks; ++i )
		 data.RebuildGameMovement( cmd.Xor( ) );
   #endif

	  eyePos = ( settings->autodirection_ignore_duck ) ? data.m_vecOrigin + Vector( 0.0f, 0.0f, 58.0f ) : data.m_vecOrigin + viewHeight;
	  std::sort( players_sorted.begin( ), players_sorted.end( ), std::less< edgy_sort >( ) );

	  bool found_valid = false;

	  float finalDistance = 0.0f;

	  Vector direction;
	  for ( auto& sorted : players_sorted ) {
		 auto player = sorted.pl;
		 if ( !player )
			continue;

		 auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
		 if ( !lag_data.IsValid( ) )
			continue;

		 auto weapon = ( C_WeaponCSBaseGun* ) player->m_hActiveWeapon( ).Get( );
		 if ( !weapon )
			continue;

		 auto weapon_data = weapon->GetCSWeaponData( );
		 if ( !weapon_data.IsValid( ) )
			continue;

		 auto FREESTAND_MAX = settings->autodirection_range;

		 direction = sorted.eye - eyePos;
		 direction.Normalize( );

		 CTraceFilterWorldAndPropsOnly filter;

		 CGameTrace tr;
		 Ray_t ray;

		 Vector endPos = Vector(
			( direction.y * FREESTAND_MAX ) + eyePos.x,
			( direction.x * -FREESTAND_MAX ) + eyePos.y,
			eyePos.z );

		 ray.Init( eyePos, endPos );
		 Source::m_pEngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &tr );

		 float fraction_01 = tr.fraction * FREESTAND_MAX;

		 endPos = Vector(
			eyePos.x + ( direction.y * -FREESTAND_MAX ),
			eyePos.y + ( direction.x * FREESTAND_MAX ),
			eyePos.z );
		 ray.Init( eyePos, endPos );
		 Source::m_pEngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &tr );

		 float fraction_02 = tr.fraction * -FREESTAND_MAX;

		 endPos = Vector(
			sorted.eye.x + ( direction.y * FREESTAND_MAX ),
			sorted.eye.y + ( direction.x * -FREESTAND_MAX ),
			sorted.eye.z );
		 ray.Init( sorted.eye, endPos );
		 Source::m_pEngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &tr );

		 float fraction_03 = tr.fraction * FREESTAND_MAX;

		 endPos = Vector(
			sorted.eye.x + ( direction.y * -FREESTAND_MAX ),
			sorted.eye.y + ( -direction.x * -FREESTAND_MAX ),
			sorted.eye.z );
		 ray.Init( sorted.eye, endPos );
		 Source::m_pEngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &tr );

		 float fraction_04 = tr.fraction * -FREESTAND_MAX;

		 float enemyOffset = -FREESTAND_MAX;
		 float localOffset = -FREESTAND_MAX;

		 do {
			if ( localOffset <= fraction_01 && fraction_02 <= localOffset ) {
			   enemyOffset = -FREESTAND_MAX;
			   fraction_03 = fraction_03;
			   do {
				  if ( enemyOffset <= fraction_03 && fraction_04 <= enemyOffset ) {
					 CTraceFilterWorldAndPropsOnly filter;

					 Autowall::C_FireBulletData data;
					 data.m_bPenetration = true;
					 data.m_TargetPlayer = player;
					 data.m_Player = LocalPlayer;
					 data.m_WeaponData = weapon_data.Xor( );
					 data.m_Weapon = weapon;
					 data.m_Filter = &filter;
					 data.m_bShouldIgnoreDistance = true;

					 data.m_vecStart.x = ( enemyOffset * direction.y ) + sorted.eye.x;
					 data.m_vecStart.y = ( enemyOffset * -direction.x ) + sorted.eye.y;
					 data.m_vecStart.z = sorted.eye.z;

					 data.m_vecDirection.x = ( ( localOffset * direction.y ) + eyePos.x ) - data.m_vecStart.x;
					 data.m_vecDirection.y = ( ( localOffset * -direction.x ) + eyePos.y ) - data.m_vecStart.y;
					 data.m_vecDirection.z = eyePos.z - sorted.eye.z;

				  #if 0
					 if ( InputSys::Get( )->IsKeyDown( VirtualKeys::H ) ) {
						Source::m_pDebugOverlay->AddBoxOverlay( data.m_vecStart,
																Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ), QAngle( 0.0f, 0.0f, 0.0f ), 255, 180, 0, 200, 2.0f );
					 }

					 if ( InputSys::Get( )->IsKeyDown( VirtualKeys::J ) ) {
						Source::m_pDebugOverlay->AddBoxOverlay( data.m_vecDirection + data.m_vecStart,
																Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ), QAngle( 0.0f, 0.0f, 0.0f ), 0, 180, 255, 200, 2.0f );
					 }
				  #endif

					 data.m_flPenetrationDistance = data.m_vecDirection.Normalize( );
					 auto damage = Autowall::FireBullets( &data );
					 if ( damage >= 1.0f ) {
						found_valid = true;
						finalDistance += localOffset;
					 }
				  }
				  enemyOffset = enemyOffset + FREESTAND_MAX / 2.0f;
			   } while ( enemyOffset <= FREESTAND_MAX );
			   fraction_01 = fraction_01;
			}
			localOffset = localOffset + FREESTAND_MAX / 2.0f;
		 } while ( localOffset <= FREESTAND_MAX );

		 if ( found_valid )
			break;
	  }

	  if ( !found_valid || finalDistance == 0.0f ) {
		 if ( settings->autodirection && !found_valid && Edge( cmd ) ) {
			cmd->viewangles.y = std::remainderf( cmd->viewangles.y, 360.0f );
			return true;
		 }

		 return false;
	  }

	  float angle = RAD2DEG( atan2( direction.y, direction.x ) );
	  bool desync_enabled = settings->desync && settings->desync_autodir;

	  m_flDirection = finalDistance;

	  if ( desync_enabled ) {
		 m_bAutomaticDir = true;

		 m_bInverted = finalDistance <= 0.0f;
		 if ( settings->desync_autodir == 2 )
			m_bInverted = !m_bInverted;
	  }

	  if ( settings->autodirection ) {
		 if ( finalDistance <= 0.0f )
			cmd->viewangles.y = angle - 90.0f;
		 else
			cmd->viewangles.y = angle + 90.0f;
		 cmd->viewangles.y = std::remainderf( cmd->viewangles.y, 360.0f );
	  }

	  return true;
   }

   bool C_AntiAimbot::RunAutoDirection( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings, bool only_desync ) {
	  if ( g_Vars.antiaim.autodirection_override.enabled )
		 return false;

	  static bool autodir = false;

	  float yaw_backup = cmd->viewangles.yaw;
	  if ( ( !only_desync && settings->autodirection ) || settings->desync_autodir ) {
		 if ( AutoDirection( cmd, settings ) ) {
			autodir = true;
		 } else {
			autodir = false;
		 }
	  } else {
		 autodir = false;
	  }

	  return false;
   }

   void C_AntiAimbot::AtTarget( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings ) {
	  auto local = C_CSPlayer::GetLocalPlayer( );
	  auto found = false;
	  float bestFov = std::numeric_limits< float >::max( );

	  Vector bestOrigin;
	  for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; i++ ) {
		 auto player = C_CSPlayer::GetPlayerByIndex( i );
		 if ( !player || player->IsDormant( ) || player->IsDead( ) )
			continue;

		 bool is_team = player->IsTeammate( local );
		 if ( is_team )
			continue;

		 auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
		 if ( !lag_data.IsValid( ) )
			continue;

		 Vector origin = lag_data->GetExtrapolatedPosition( player );

		 auto AngleDistance = [&] ( QAngle& angles, const Vector& start, const Vector& end ) -> float {
			auto direction = end - start;
			auto aimAngles = direction.ToEulerAngles( );
			auto delta = aimAngles - angles;
			delta.Normalize( );

			return sqrtf( delta.x * delta.x + delta.y * delta.y );
		 };

		 auto studio_model = Source::m_pModelInfo->GetStudiomodel( player->GetModel( ) );
		 auto hitboxSet = studio_model->pHitboxSet( player->m_nHitboxSet( ) );

		 auto GetHitboxPos = [&] ( int id ) -> Vector {
			if ( studio_model ) {
			   auto hitbox = hitboxSet->pHitbox( id );
			   if ( hitbox ) {
				  auto
					 min = Vector{},
					 max = Vector{};

				  return ( ( hitbox->bbmin + hitbox->bbmax ) * 0.5f ).Transform( player->m_CachedBoneData( ).m_Memory.m_pMemory[ hitbox->bone ] );
			   }
			}
		 };

		 Vector hitboxPos = GetHitboxPos( HITBOX_PELVIS );
		 float Fov = AngleDistance( cmd->viewangles, local->GetEyePosition( ), hitboxPos );

		 if ( Fov < bestFov ) {
			bestOrigin = origin;
			bestFov = Fov;
			found = true;
		 }
	  }

	  if ( found ) {
		 const float yaw = Math::CalcAngle( local->m_vecOrigin( ), bestOrigin ).y + 180.f;
		 cmd->viewangles.yaw = std::remainderf( yaw, 360.0f );
	  }
   }
}
