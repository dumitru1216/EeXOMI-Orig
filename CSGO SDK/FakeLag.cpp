#include "FakeLag.hpp"
#include "CVariables.hpp"

#include "Player.hpp"
#include "Weapon.hpp"
#include "Autowall.h"

#include "InputSys.hpp"
#include "LagCompensation.hpp"
#include "Prediction.hpp"

#include "Source.hpp"

#include "Displacement.hpp"
#include "Movement.hpp"

#include "SetupBones.hpp"
#include "SimulationContext.hpp"

#include "AnimationSystem.hpp"
#include "TickbaseShift.hpp"

extern bool HideRealAfterShot;
extern int OutgoingTickcount;
extern CUserCmd* previous_cmd;

namespace Source
{
   struct FakelagData {
	  int m_iChoke, m_iMaxChoke;
	  int m_iWillChoke = 0;
	  int m_iLatency = 0;
	  int m_iLimit = 0;

	  bool m_bAlternative = false;
	  bool m_bLanding = false;
   };

   static FakelagData _fakelag_data;

   class C_FakeLag : public FakeLag {
   public:
	  C_FakeLag( ) : fakelagData( &_fakelag_data ) {

	  }

	  virtual void Main( bool* bSendPacket, Encrypted_t<CUserCmd> cmd );
	  virtual int GetFakelagChoke( ) const {
		 return fakelagData.Xor( )->m_iWillChoke;
	  }

	  virtual int GetMaxFakelagChoke( ) const {
		 return fakelagData.Xor( )->m_iMaxChoke;
	  }

	  virtual bool IsPeeking( Encrypted_t<CUserCmd> cmd );

   private:
	  __forceinline bool AlternativeChoke( bool* bSendPacket );
	  bool VelocityChange( Encrypted_t<CUserCmd> cmd );
	  bool BreakLagComp( int trigger_limit );
	  bool AlternativeCondition( Encrypted_t<CUserCmd> cmd, bool* bSendPacket );
	  bool MainCondition( Encrypted_t<CUserCmd> cmd, bool* bSendPacket );

	  Encrypted_t<FakelagData> fakelagData;
   };

   Encrypted_t<FakeLag> FakeLag::Get( ) {
	  static C_FakeLag instance;
	  return &instance;
   }

   void C_FakeLag::Main( bool* bSendPacket, Encrypted_t<CUserCmd> cmd ) {
	  fakelagData->m_iWillChoke = 0;
	  fakelagData->m_iMaxChoke = 0;

	  C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );
	  if ( !LocalPlayer || LocalPlayer->IsDead( ) ) {
		 return;
	  }

	  if ( g_Vars.globals.WasShootingInChokeCycle && !g_Vars.globals.Fakeducking ) {
		 fakelagData->m_bAlternative = false;
		 *bSendPacket = true;
		 return;
	  }

	 if ( g_Vars.rage.exploit && g_Vars.rage.double_tap_bind.enabled || !( *bSendPacket ) ) //  we use dt
		 return;

	  if ( !g_Vars.fakelag.enabled || HideRealAfterShot )
		 return;

	  auto g_GameRules = *( uintptr_t** ) ( Engine::Displacement.Data.m_GameRules );
	  if ( *( bool* ) ( *( uintptr_t* ) g_GameRules + 0x20 ) )
		 return;

	  if ( g_Vars.globals.Fakeducking )
		 return;

	  if ( LocalPlayer->m_fFlags( ) & 0x40 )
		 return;

	  auto weapon = reinterpret_cast< C_WeaponCSBaseGun* >( LocalPlayer->m_hActiveWeapon( ).Get( ) );
	  if ( !weapon )
		 return;

	  auto pWeaponData = weapon->GetCSWeaponData( );
	  if ( !pWeaponData.Xor( ) || !pWeaponData.IsValid( ) )
		 return;

	  if ( pWeaponData->m_iWeaponType == WEAPONTYPE_GRENADE ) {
		 if ( !weapon->m_bPinPulled( ) || ( cmd->buttons & ( IN_ATTACK | IN_ATTACK2 ) ) ) {
			float throwTime = weapon->m_fThrowTime( );
			if ( throwTime > 0.f ) {
			   *bSendPacket = true;
			   return;
			}
		 }
	  } 
	 // else if ( cmd->buttons & IN_ATTACK ) {
	//	 if ( !TickbaseShiftCtx.in_rapid ) {
	//		if ( !g_Vars.globals.Fakeducking ) {
	//		   *bSendPacket = true;
	//		   return;
	//		}
	//	 }
	 // }

	  fakelagData->m_iLimit = g_Vars.fakelag.lag_limit;
	  if ( fakelagData->m_iLimit <= 0 )
		 return;

	  if ( g_Vars.fakelag.when_land && !( Engine::Prediction::Instance( ).GetFlags( ) & FL_ONGROUND ) && LocalPlayer->m_fFlags( ) & FL_ONGROUND ) {
		 fakelagData->m_bLanding = true;
	  }

	  if ( fakelagData->m_bLanding ) {
		 if ( AlternativeChoke( bSendPacket ) )
			fakelagData->m_bLanding = false;
		 return;
	  }

	  if ( !g_Vars.fakelag.when_dormant ) {
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
			return;
	  }

	  auto net_channel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
	  if ( !net_channel.IsValid( ) )
		 return;

	  fakelagData->m_iMaxChoke = 0;
	  if ( !AlternativeCondition( cmd, bSendPacket ) && !MainCondition( cmd, bSendPacket ) ) {
		 return;
	  }

	  if ( fakelagData->m_iMaxChoke < 1 )
		 return;

	  RandomSeed( cmd->command_number );

	  // skeet.cc
	  int variance = 0;
	  if ( g_Vars.fakelag.variance > 0.f && !g_Vars.globals.Fakeducking ) {
		 variance = int( float( g_Vars.fakelag.variance * 0.01f ) * float( fakelagData->m_iMaxChoke ) );
	  }

	  auto apply_variance = [this] ( int variance, int fakelag_amount ) {
		 if ( variance > 0 && fakelag_amount > 0 ) {
			auto max = Math::Clamp( variance, 0, ( fakelagData->m_iMaxChoke ) - fakelag_amount );
			auto min = Math::Clamp( -variance, -fakelag_amount, 0 );
			fakelag_amount += RandomInt( min, max );
			if ( fakelag_amount == g_Vars.globals.LastChokedCommands ) {
			   if ( fakelag_amount >= ( fakelagData->m_iMaxChoke ) || fakelag_amount > 2 && !RandomInt( 0, 1 ) ) {
				  --fakelag_amount;
			   } else {
				  ++fakelag_amount;
			   }
			}
		 }

		 return fakelag_amount;
	  };

	  if ( **( int** ) Engine::Displacement.Data.m_uHostFrameTicks > 1 )
		 fakelagData->m_iMaxChoke += **( int** ) Engine::Displacement.Data.m_uHostFrameTicks - 1;

	  fakelagData->m_iMaxChoke = Math::Clamp( fakelagData->m_iMaxChoke, 0, fakelagData->m_iLimit );

	  auto fakelag_amount = fakelagData->m_iMaxChoke;
	  fakelag_amount = apply_variance( variance, fakelag_amount );
	  *bSendPacket = Source::m_pClientState->m_nChokedCommands( ) >= fakelag_amount;
	  fakelagData->m_iWillChoke = fakelag_amount - Source::m_pClientState->m_nChokedCommands( );

   #if 0
	  //	  ( g_Vars.fakelag.when_exploits && TickbaseShiftCtx.exploits_enabled
			   //&& TickbaseShiftCtx.ticks_allowed >= fakelag_amount )
   #endif

	  auto diff_too_large = abs( Source::m_pGlobalVars->tickcount - OutgoingTickcount ) > fakelagData->m_iMaxChoke;
	  if ( Source::m_pClientState->m_nChokedCommands( ) > 0 && diff_too_large ) {
		 *bSendPacket = true;
		 fakelagData->m_bAlternative = false;
		 fakelagData->m_iWillChoke = 0;
		 return;
	  }
   }

   int FindPeekTarget( bool dormant_peek ) {
	  static auto LastPeekTime = 0;
	  static auto LastPeekTarget = 0;
	  static auto InPeek = false;

	  if ( LastPeekTime == Source::m_pGlobalVars->tickcount ) {
		 if ( dormant_peek || InPeek )
			return LastPeekTarget;
		 return 0;
	  }

	  InPeek = false;

	  auto local = C_CSPlayer::GetLocalPlayer( );

	  if ( !local || local->IsDead( ) )
		 return 0;

	  bool any_alive_players = false;
	  Engine::C_AnimationData* players[ 64 ];
	  int player_count = 0;
	  for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
		 if ( i > 63 )
			break;

		 auto target = Engine::AnimationSystem::Get( )->GetAnimationData( i );
		 if ( !target )
			continue;

		 // todo: check for round count
		 auto player = C_CSPlayer::GetPlayerByIndex( i );
		 if ( !player || player->IsTeammate( local ) )
			continue;

		 if ( target->m_bIsAlive )
			any_alive_players = true;

		 if ( !player->IsDormant( ) )
			InPeek = true;

		 players[ player_count ] = target;
		 player_count++;
	  }

	  if ( !player_count ) {
		 LastPeekTime = Source::m_pGlobalVars->tickcount;
		 LastPeekTarget = 0;
		 return 0;
	  }

	  auto eye_pos = local->m_vecOrigin( ) + Vector( 0.0f, 0.0f, 64.0f );

	  auto best_target = 0;
	  auto best_fov = 9999.0f;
	  for ( int i = 0; i < player_count; ++i ) {
		 auto animation = players[ i ];
		 auto player = C_CSPlayer::GetPlayerByIndex( animation->ent_index );
		 if ( !player )
			continue;

		 if ( InPeek && player->IsDormant( ) || any_alive_players && !animation->m_bIsAlive )
			continue;

		 auto origin = player->m_vecOrigin( );
		 if ( player->IsDormant( ) ) {
			origin = animation->m_vecOrigin;
		 }

		 origin.z += 64.0f;

		 Ray_t ray;
		 ray.Init( eye_pos, origin );

		 CTraceFilter filter;
		 filter.pSkip = local;

		 CGameTrace tr;
		 Source::m_pEngineTrace->TraceRay( ray, 0x4600400B, &filter, &tr );
		 if ( tr.fraction >= 0.99f || tr.hit_entity == player ) {
			LastPeekTime = Source::m_pGlobalVars->tickcount;
			LastPeekTarget = player->m_entIndex;
			return player->m_entIndex;
		 }

		 QAngle viewangles;
		 Source::m_pEngine->GetViewAngles( viewangles );

		 auto delta = origin - eye_pos;
		 auto angles = delta.ToEulerAngles( );
		 auto view_delta = angles - viewangles;
		 view_delta.Normalize( );

		 auto fov = std::sqrtf( view_delta.x * view_delta.x + view_delta.y * view_delta.y );

		 if ( fov >= best_fov )
			continue;

		 best_fov = fov;
		 best_target = player->m_entIndex;
	  }

	  LastPeekTime = Source::m_pGlobalVars->tickcount;
	  LastPeekTarget = best_target;
	  if ( dormant_peek || InPeek )
		 return best_target;

	  return 0;
   }

   bool C_FakeLag::IsPeeking( Encrypted_t<CUserCmd> cmd ) {
	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || local->IsDead( ) || local->m_vecVelocity( ).Length( ) <= 20.0f )
		 return false;

	  auto pWeapon = reinterpret_cast< C_WeaponCSBaseGun* >( local->m_hActiveWeapon( ).Get( ) );
	  if ( !pWeapon )
		 return false;

	  auto target = FindPeekTarget( true );
	  if ( !target )
		 return false;

	  auto player = C_CSPlayer::GetPlayerByIndex( target );
	  if ( !player )
		 return false;

	  auto enemy_weapon = reinterpret_cast< C_WeaponCSBaseGun* >( player->m_hActiveWeapon( ).Get( ) );
	  if ( !enemy_weapon )
		 return false;

	  auto weapon_data = enemy_weapon->GetCSWeaponData( );
	  auto enemy_eye_pos = player->GetEyePosition( );

	  auto eye_pos = local->GetEyePosition( );
	  auto peek_add = local->m_vecVelocity( ) * TICKS_TO_TIME( 14 );
	  auto second_scan = eye_pos;
	  if ( local->m_Collision( )->m_vecMaxs.x > peek_add.Length2D( ) ) {
		 auto peek_direction = local->m_vecVelocity( ).Normalized( );
		 second_scan += peek_direction * local->m_Collision( )->m_vecMaxs.x;
	  } else {
		 second_scan += peek_add;
	  }

   #if 0
	  if ( InputSys::Get( )->IsKeyDown( VirtualKeys::F ) ) {
		 Source::m_pDebugOverlay->AddBoxOverlay( eye_pos, Vector( -1.0f, -1.0f, -1.0f ), Vector( 1.0f, 1.0f, 1.0f ), QAngle( ), 200, 100, 200, 100, 2.0f );
		 Source::m_pDebugOverlay->AddBoxOverlay( second_scan, Vector( -1.0f, -1.0f, -1.0f ), Vector( 1.0f, 1.0f, 1.0f ), QAngle( ), 255, 255, 255, 100, 2.0f );
	  }
   #endif

	   Autowall::C_FireBulletData data;

	   data.m_bPenetration = true;
	   data.m_vecStart = eye_pos;

	   data.m_Player = local;
	   data.m_TargetPlayer = player;
	   data.m_WeaponData = pWeapon->GetCSWeaponData( ).Xor( );
	   data.m_Weapon = pWeapon;

	   data.m_vecDirection = ( enemy_eye_pos - eye_pos ).Normalized( );
	   data.m_flPenetrationDistance = data.m_vecDirection.Normalize( );
	   float damage = Autowall::FireBullets( &data );

	   data = Autowall::C_FireBulletData{};
	   data.m_bPenetration = true;
	   data.m_vecStart = second_scan;

	   data.m_Player = local;
	   data.m_TargetPlayer = player;
	   data.m_WeaponData = pWeapon->GetCSWeaponData( ).Xor( );
	   data.m_Weapon = pWeapon;

	   data.m_vecDirection = ( enemy_eye_pos - second_scan ).Normalized( );
	   data.m_flPenetrationDistance = data.m_vecDirection.Normalize( );
	   
	   float damage2 = Autowall::FireBullets( &data );

	   if ( damage >= 1.0f )
		  return false;

	   if ( damage2 >= 1.0f )
		  return true;

	  return false;
   }

   bool C_FakeLag::AlternativeChoke( bool* bSendPacket ) {
	  fakelagData->m_iMaxChoke = g_Vars.fakelag.alternative_choke;

	  if ( **( int** ) Engine::Displacement.Data.m_uHostFrameTicks > 1 )
		 fakelagData->m_iMaxChoke += **( int** ) Engine::Displacement.Data.m_uHostFrameTicks - 1;

	  fakelagData->m_iMaxChoke = Math::Clamp( fakelagData->m_iMaxChoke, 0, fakelagData->m_iLimit );

	  *bSendPacket = Source::m_pClientState->m_nChokedCommands( ) >= fakelagData->m_iMaxChoke;
	  fakelagData->m_iWillChoke = fakelagData->m_iMaxChoke - Source::m_pClientState->m_nChokedCommands( );

	  if ( *bSendPacket ) {
		 fakelagData->m_bAlternative = false;
		 return true;
	  }

	  return false;
   }

   bool C_FakeLag::VelocityChange( Encrypted_t<CUserCmd> cmd ) {
	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || local->IsDead( ) )
		 return false;

	  static auto init_velocity = false;
	  static auto previous_velocity = Vector{};
	  if ( local->m_vecVelocity( ).Length( ) < 5.0f ) {
		 init_velocity = false;
		 return false;
	  }

	  if ( !init_velocity ) {
		 init_velocity = true;
	  }

	  auto move_dir = RAD2DEG( std::atan2f( previous_velocity.y, previous_velocity.x ) );
	  auto current_move_dir = RAD2DEG( std::atan2f( local->m_vecVelocity( ).y, local->m_vecVelocity( ).x ) );
	  auto delta = std::remainderf( current_move_dir - move_dir, 360.0f );
	  if ( std::fabsf( delta ) >= 30.0f ) {
		 previous_velocity = local->m_vecVelocity( );
		 return true;
	  }

	  return false;
   }

   bool C_FakeLag::BreakLagComp( int trigger_limit ) {
	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || local->IsDead( ) )
		 return false;

	  if ( !g_Vars.fakelag.break_lag_compensation )
		 return false;

	  auto speed = local->m_vecVelocity( ).LengthSquared( );
	  if ( speed < 5.0f )
		 return false;

	  auto choke = trigger_limit - Source::m_pClientState->m_nChokedCommands( );
	  if ( choke < 1 )
		 return false;

	  auto simulated_origin = local->m_vecOrigin( );
	  auto move_per_tick = local->m_vecVelocity( ) * Source::m_pGlobalVars->interval_per_tick;
	  for ( int i = 0; i < choke; ++i ) {
		 simulated_origin += move_per_tick;

		 auto distance = g_Vars.globals.m_vecNetworkedOrigin.DistanceSquared( simulated_origin );
		 if ( distance > 4096.0f )
			return true;
	  }

	  return false;
   }

   bool C_FakeLag::AlternativeCondition( Encrypted_t<CUserCmd> cmd, bool* bSendPacket ) {
	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || local->IsDead( ) || g_Vars.fakelag.alternative_choke <= 0 ) {
		 fakelagData->m_bAlternative = false;
		 return false;
	  }

	  fakelagData->m_iMaxChoke = 0;

	  if ( g_Vars.fakelag.break_lag_compensation ) {
		 auto distance = local->m_vecOrigin( ).DistanceSquared( g_Vars.globals.m_vecNetworkedOrigin );
		 if ( distance > 4096.0f )
			return true;
	  }

	  if ( !fakelagData->m_bAlternative ) {
		 bool landing = false;
		 if ( g_Vars.fakelag.when_unducking && local->m_flDuckAmount( ) > 0.0f && g_Vars.globals.m_flPreviousDuckAmount > local->m_flDuckAmount( ) ) {
			fakelagData->m_bAlternative = true;
		 } else if ( g_Vars.fakelag.when_switching_weapon && ( cmd->weaponselect != 0 || local->m_flNextAttack( ) > Source::m_pGlobalVars->curtime ) ) {
			fakelagData->m_bAlternative = true;
		 } else if ( g_Vars.fakelag.when_reloading && local->IsReloading( ) ) {
			fakelagData->m_bAlternative = true;
		 } else if ( g_Vars.fakelag.when_velocity_change && VelocityChange( cmd ) ) {
			fakelagData->m_bAlternative = true;
		 } else if ( g_Vars.fakelag.break_lag_compensation && BreakLagComp( g_Vars.fakelag.alternative_choke ) ) {
			fakelagData->m_bAlternative = true;
		 } else if ( g_Vars.fakelag.when_peek && Source::FakeLag::Get( )->IsPeeking( cmd ) ) {
			fakelagData->m_bAlternative = true;
		 }

		 if ( fakelagData->m_bAlternative && Source::m_pClientState->m_nChokedCommands( ) > 0 ) {
			*bSendPacket = true;
			return true;
		 }
	  }

	  if ( fakelagData->m_bAlternative ) {
		 fakelagData->m_iMaxChoke = g_Vars.fakelag.alternative_choke;
	  }

	  return fakelagData->m_bAlternative;
   }

   bool C_FakeLag::MainCondition( Encrypted_t<CUserCmd> cmd, bool* bSendPacket ) {
	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || local->IsDead( ) || g_Vars.fakelag.choke < 1 )
		 return false;

	  auto animState = local->m_PlayerAnimState( );
	  auto velocity = Math::GetSmoothedVelocity( Source::m_pGlobalVars->interval_per_tick * 2000.0f, local->m_vecVelocity( ), animState->m_vecVelocity );

	  bool moving = velocity.Length( ) >= 1.2f;
	  bool air = !( local->m_fFlags( ) & FL_ONGROUND );

	  if ( !moving && !g_Vars.fakelag.when_standing )
		 return false;
	  else if ( air && !g_Vars.fakelag.when_air )
		 return false;
	  else if ( moving && !g_Vars.fakelag.when_moving )
		 return false;

	  fakelagData->m_iMaxChoke = g_Vars.fakelag.choke;
	  return true;
   }
}
