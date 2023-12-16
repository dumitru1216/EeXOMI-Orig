#include "LegitBot.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "CVariables.hpp"
#include "LagCompensation.hpp"
#include "InputSys.hpp"
#include "Autowall.h"
#include "Movement.hpp"
#include "Displacement.hpp"
#include "prediction.hpp"


//vse
namespace Source
{
   class CLegitBot : public LegitBot {
   public:
	  void Main( CUserCmd* pCmd ) override;
	  void GameEventCallBack( ) override;
	  CLegitBot( ) {

	  }
   private:
	  void TriggerBot( );
	  void TriggerFire( );
	  bool Hitchance( C_CSPlayer* player, const Vector& start, const Vector& point, float chance );

	  bool FindTarget( );
	  bool ValidateTarget( );
	  Vector GetHitscanPos( );
	  void AimAtTarget( );
	  void CalculateRecoil( );
	  bool BacktrackPlayers( );
	  void RestorePlayers( );
	  void AutoPistol( );
	  bool SmokeCheck( const Vector& start, const Vector& end );

	  float AngleDistance( QAngle& angles, const Vector& start, const Vector& end ) {
		 auto direction = end - start;
		 auto aimAngles = direction.ToEulerAngles( );
		 auto delta = aimAngles - angles;
		 delta.Normalize( );

		 return sqrtf( delta.x * delta.x + delta.y * delta.y );
	  };

	  Engine::C_LagRecord* GetLagRecord( C_CSPlayer* player );

	  C_CSPlayer* m_player = nullptr;
	  C_CSPlayer* m_local = nullptr;
	  C_CSPlayer* m_triggerTarget = nullptr;

	  C_WeaponCSBaseGun* m_weapon = nullptr;
	  Encrypted_t<CCSWeaponInfo> m_weaponInfo = nullptr;

	  CUserCmd* m_cmd = nullptr;

	  struct C_EntityRestore {
		 C_CSPlayer* player;
		 Engine::C_BaseLagRecord backup;
		 float fov;
		 int tickCount;
	  };
	  std::vector<C_EntityRestore> m_restore;
	  float m_flTargetTime = 0.0f;

	  QAngle m_viewAngles;
	  QAngle m_angRecoil;

	  float m_flLastKillTime = 0.f;
	  float m_flLastAutopistolTime = 0.f;
	  float m_flFirstShotDelay = 0.f;
	  float m_flLastTriggerShot = 0.f;
	  float m_flBurstTime = 0.f;

	  bool m_deadZoneReached = false;
	  bool m_triggerBot = false;

	  Vector m_target_pos = Vector( 0, 0, 0 );

	  CVariables::LEGIT* lbot_option = nullptr;
   };

   LegitBot* LegitBot::Get( ) {
	  static CLegitBot instance;
	  return &instance;
   }

   void CLegitBot::CalculateRecoil( ) {
	  QAngle PunchAngle = m_local->m_aimPunchAngle( ) * g_Vars.weapon_recoil_scale->GetFloat( );
	  PunchAngle *= QAngle( lbot_option->rcs_y * 0.01f, lbot_option->rcs_x * 0.01f, 0.0f );

	  if ( m_weapon->m_flNextPrimaryAttack( ) >= Source::m_pGlobalVars->curtime ) {
		 auto delta_angles = PunchAngle - m_angRecoil;
		 auto delta = m_weapon->m_flNextPrimaryAttack( ) - Source::m_pGlobalVars->curtime + Source::m_pGlobalVars->interval_per_tick;
		 if ( delta >= Source::m_pGlobalVars->interval_per_tick ) {
			auto deltaTicks = static_cast< float >( TIME_TO_TICKS( delta ) );
			m_angRecoil += delta_angles / deltaTicks;
		 } else {
			m_angRecoil = PunchAngle;
		 }
	  } else {
		 m_angRecoil = PunchAngle;
	  }

	  m_angRecoil.Normalize( );
   }

   bool CLegitBot::BacktrackPlayers( ) {
	  auto eyePos = m_local->GetEyePosition( );

	  for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; i++ ) {
		 auto player = ( C_CSPlayer* ) Source::m_pEntList->GetClientEntity( i );
		 if ( !player || player->IsDead( ) || player->IsDormant( ) || player->m_bGunGameImmunity( ) || player == C_CSPlayer::GetLocalPlayer( ) || player->IsTeammate( C_CSPlayer::GetLocalPlayer( ) ) )
			continue;

		 auto lagRecord = GetLagRecord( player );
		 if ( !lagRecord )
			continue; // rip

		 auto& pair = m_restore.emplace_back( );
		 pair.player = player;
		 pair.backup.Setup( player );
		 pair.fov = AngleDistance( m_viewAngles, eyePos, lagRecord->m_vecOrigin + ( lagRecord->m_vecMins + lagRecord->m_vecMaxs ) * 0.5f );
		 pair.tickCount = TIME_TO_TICKS( lagRecord->m_flSimulationTime + Engine::LagCompensation::Get( )->GetLerp( ) );

		 lagRecord->Apply( player );
	  }

	  if ( m_restore.size( ) > 0 ) {
		 std::sort( m_restore.begin( ), m_restore.end( ), [] ( const C_EntityRestore& a, const C_EntityRestore& b ) {
			return a.fov < b.fov;
		 } );

		 return true;
	  }

	  return false;
   }

   void CLegitBot::RestorePlayers( ) {
	  for ( auto restore : m_restore ) {
		 restore.backup.Apply( restore.player );
	  }
   }

   void CLegitBot::AutoPistol( ) {
	  if ( this->m_weaponInfo->m_iWeaponType == WEAPONTYPE_PISTOL && g_Vars.legit.autopistol ) {
		 if ( m_cmd->buttons & IN_ATTACK && m_weapon->m_flNextPrimaryAttack( ) + g_Vars.legit.autopistol_delay >= Source::m_pGlobalVars->curtime ) {
			m_cmd->buttons &= ~IN_ATTACK;
		 }
	  }
   }

   bool CLegitBot::SmokeCheck( const Vector& start, const Vector& end ) {
	  static auto check = ( bool( __cdecl* )( Vector, Vector, int16_t ) )Engine::Displacement.Function.m_LineGoesThroughSmoke;
	  if ( g_Vars.legit.throughsmoke )
		 return false;

	  return check( start, end, true );
   }

   Engine::C_LagRecord* CLegitBot::GetLagRecord( C_CSPlayer* player ) {
	  // check if we have at least one entry
	  auto lagData = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
	  if ( !lagData.IsValid( ) || lagData->m_History.empty( ) )
		 return nullptr;

	  auto eyePos = m_local->GetEyePosition( );

	  if ( g_Vars.legit.position_adjustment ) {
		 float bestFov = FLT_MAX;
		 Engine::C_LagRecord* bestRecord = nullptr;
		 Engine::C_LagRecord* prevRecord = nullptr;

		 // Walk context looking for any invalidating event
		 auto& record = lagData->m_History.begin( );
		 while ( record != lagData->m_History.end( ) ) {
			if ( !Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *record, 0.2f, 0, false ) ) {
			   if ( !prevRecord
				  || prevRecord->m_vecOrigin != record->m_vecOrigin
				  || prevRecord->m_angAngles.yaw != record->m_angAngles.yaw
				  || prevRecord->m_flEyeYaw != record->m_flEyeYaw
				  || prevRecord->m_flEyePitch != record->m_flEyePitch
				  || prevRecord->m_flDuckAmount != record->m_flDuckAmount
				  || prevRecord->m_vecMaxs.z != record->m_vecMaxs.z
				  || prevRecord->m_iFlags != record->m_iFlags ) {
				  float fov = AngleDistance( m_viewAngles, eyePos, record->m_vecOrigin + ( record->m_vecMins + record->m_vecMaxs ) * 0.5f );
				  if ( bestFov > fov ) {
					 bestFov = fov;
					 bestRecord = &*record;
				  }

				  prevRecord = &*record;
			   }
			}

			// go one step back
			record++;
		 }

		 return bestRecord;
	  }

	  // Walk context looking for any invalidating event
	  auto& record = lagData->m_History.begin( );
	  while ( record != lagData->m_History.end( ) ) {
		 // lost track, too much difference
		 if ( Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *record, 0.2f, 0, false ) ) {
			return nullptr;
		 }

		 // did we find a context smaller than target time ?
		 if ( record->m_flSimulationTime <= m_flTargetTime ) {
			// hurra, stop
			return &*record;
		 }

		 // go one step back
		 record++;
	  }

	  // TODO: interpolate position here
	  // https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/server/player_lagcompensation.cpp#L388-L418

	  return nullptr;
   }

   void CLegitBot::Main( CUserCmd* pCmd ) {
	  // if ( !g_Vars.legit.active )
	  //	 return;

	  m_local = C_CSPlayer::GetLocalPlayer( );

	  m_cmd = pCmd;

	  if ( !m_local || !m_cmd || m_local->IsDead( ) )
		 return;

	  m_weapon = ( C_WeaponCSBaseGun* ) m_local->m_hActiveWeapon( ).Get( );

	  if ( !m_weapon )
		 return;

	  m_weaponInfo = m_weapon->GetCSWeaponData( );

	  if ( !m_weaponInfo.IsValid( ) )
		 return;

	  m_triggerBot = false;
	  lbot_option = nullptr;

	  auto id = m_weapon->m_iItemDefinitionIndex( );
	  for ( int i = 0; i < g_Vars.legit_weapons.Size( ); i++ ) {
		 auto weap = g_Vars.legit_weapons[ i ];
		 if ( weap->item_idx == id ) {
			lbot_option = weap;
			break;
		 }
	  }

	  // no weapon found, so its not aiming weapon
	  if ( !lbot_option ) {
		 m_angRecoil.Set( );
		 return;
	  }

	  if ( !lbot_option->active ) {
		 switch ( m_weaponInfo->m_iWeaponType ) {
			case WEAPONTYPE_PISTOL:
			   if ( id == WEAPON_REVOLVER || id == WEAPON_DEAGLE )
				  lbot_option = &g_Vars.legit_heavypistols;
			   else
				  lbot_option = &g_Vars.legit_pistols;
			   AutoPistol( );
			   break;
			case WEAPONTYPE_SUBMACHINEGUN:
			   lbot_option = &g_Vars.legit_smgs;
			   break;
			case WEAPONTYPE_RIFLE:
			   lbot_option = &g_Vars.legit_rifles;
			   break;
			case WEAPONTYPE_SHOTGUN:
			   lbot_option = &g_Vars.legit_shotguns;
			   break;
			case WEAPONTYPE_SNIPER_RIFLE:
			   if ( id == WEAPON_SCAR20 || id == WEAPON_G3SG1 )
				  lbot_option = &g_Vars.legit_autosnipers;
			   else
				  lbot_option = &g_Vars.legit_snipers;
			   break;
			case WEAPONTYPE_MACHINEGUN:
			   lbot_option = &g_Vars.legit_heavys;
			   break;
			default:
			   lbot_option = nullptr;
			   break;
		 }

		 if ( !lbot_option || !lbot_option->active ) {
			m_angRecoil.Set( );
			return;
		 }
	  }

	  if ( m_flBurstTime > Source::m_pGlobalVars->realtime && Source::m_pGlobalVars->realtime > m_flLastTriggerShot + lbot_option->trg_delay ) {
		 m_cmd->buttons |= IN_ATTACK;
	  }

	  if ( g_Vars.legit.whileblind && m_local->m_flSecondFlashAlpha( ) >= 200.f )
		 return;

	  m_deadZoneReached = false;
	  m_viewAngles = m_cmd->viewangles + m_angRecoil;
	  m_viewAngles.Normalize( );

	  if ( m_cmd->buttons & IN_ATTACK && lbot_option->rcs_standalone ) {
		 m_cmd->viewangles = m_viewAngles;

		 CalculateRecoil( );

		 if ( m_local->m_iShotsFired( ) >= lbot_option->rcs_shots ) {
			m_viewAngles = m_cmd->viewangles - m_angRecoil;
			m_viewAngles.Normalize( );

			m_cmd->viewangles = m_viewAngles;

			Source::m_pEngine->SetViewAngles( m_viewAngles );
		 }
	  } else {
		 CalculateRecoil( );
	  }

	  // correct tick send by player 
	  m_flTargetTime = TICKS_TO_TIME( m_cmd->tick_count ) - Engine::LagCompensation::Get( )->GetLerp( );

	  m_restore.clear( );

	  if ( !BacktrackPlayers( ) ) {
		 return;
	  }

	  TriggerBot( );

	  if ( lbot_option->kill_delay ) {
		 if ( m_flLastKillTime + lbot_option->kill_shot_delay > Source::m_pGlobalVars->realtime ) {
			RestorePlayers( );
			return;
		 }
	  }

	  if ( !( m_cmd->buttons & IN_ATTACK ) && m_local->m_iShotsFired( ) < 1 )
		 m_flFirstShotDelay = Source::m_pGlobalVars->realtime + lbot_option->first_shot_delay;

	  if ( ( g_Vars.legit.snipers_only_scope && this->m_weaponInfo->m_iWeaponType == WEAPONTYPE_SNIPER_RIFLE && m_weapon->m_weaponMode( ) == 0 )
		 || ( !lbot_option->key.enabled && !m_triggerBot ) || !FindTarget( ) ) {
		 if ( m_deadZoneReached ) {
			if ( lbot_option->fsd_enabled && ( m_cmd->buttons & IN_ATTACK ) ) {
			   if ( m_flFirstShotDelay < Source::m_pGlobalVars->realtime ) {
				  m_flFirstShotDelay = 0.f;
				  m_cmd->buttons |= IN_ATTACK;
			   } else {
				  m_cmd->buttons &= ~IN_ATTACK;
			   }
			}
		 }

		 RestorePlayers( );

		 return;
	  }

	  if ( lbot_option->fsd_enabled && ( m_cmd->buttons & IN_ATTACK ) ) {
		 if ( lbot_option->auto_delay ) {
			Vector min, max;
			auto restore = std::find_if( m_restore.begin( ), m_restore.end( ), [this] ( const C_EntityRestore& a ) {  return a.player == m_player; } );
			if ( restore != m_restore.end( ) ) {
			   if ( m_player->ComputeHitboxSurroundingBox( min, max, restore->backup.m_BoneMatrix ) ) {
				  bool hit = Math::IntersectionBoundingBox( m_local->GetEyePosition( ), m_cmd->viewangles.ToVectors( ), min, max );

				  if ( hit ) {
					 m_cmd->buttons |= IN_ATTACK;
				  } else {
					 m_cmd->buttons &= ~IN_ATTACK;
				  }
			   }
			}
		 } else {
			if ( m_flFirstShotDelay < Source::m_pGlobalVars->realtime ) {
			   m_flFirstShotDelay = 0.f;
			   m_cmd->buttons |= IN_ATTACK;
			} else {
			   m_cmd->buttons &= ~IN_ATTACK;
			}
		 }
	  }

	  AimAtTarget( );

	  if ( lbot_option->autofire && m_local->CanShoot( ) ) {
		 auto eyePos = m_local->GetEyePosition( );

		 Ray_t ray;
		 ray.Init( eyePos, eyePos + m_cmd->viewangles.ToVectors( ) * this->m_weaponInfo->m_flWeaponRange );

		 CGameTrace tr;
		 Source::m_pEngineTrace->ClipRayToEntity( ray, MASK_SHOT_HULL | CONTENTS_HITBOX, m_player, &tr );
		 if ( tr.DidHit( ) ) {
			m_cmd->buttons |= IN_ATTACK;
			AutoPistol( );
		 }
	  }

	  if ( g_Vars.esp.draw_hitboxes && m_cmd->buttons & IN_ATTACK && m_local->CanShoot( ) ) {
		 auto matrix = m_player->m_CachedBoneData( ).Base( );

		 auto hdr = Source::m_pModelInfo->GetStudiomodel( m_player->GetModel( ) );
		 auto hitboxSet = hdr->pHitboxSet( m_player->m_nHitboxSet( ) );
		 for ( int i = 0; i < hitboxSet->numhitboxes; ++i ) {
			auto hitbox = hitboxSet->pHitbox( i );
			if ( hitbox->m_flRadius <= 0.f )
			   continue;

			auto min = hitbox->bbmin.Transform( matrix[ hitbox->bone ] );
			auto max = hitbox->bbmax.Transform( matrix[ hitbox->bone ] );

			Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius, g_Vars.esp.hitboxes_color.r * 255, g_Vars.esp.hitboxes_color.g * 255, g_Vars.esp.hitboxes_color.b * 255, g_Vars.esp.hitboxes_color.a * 255,
			   Source::m_pCvar->FindVar( XorStr( "sv_showlagcompensation_duration" ) )->GetFloat( ) );
		 }
	  }

	  if ( this->lbot_option->quickstop ) {
		 Source::Movement::Get( )->StopPlayer( );
	  }

	  RestorePlayers( );
   }

   void CLegitBot::GameEventCallBack( ) {
	  m_flLastKillTime = Source::m_pGlobalVars->realtime;
   }

   void CLegitBot::TriggerBot( ) {
	  m_triggerTarget = nullptr;
	  if ( !lbot_option->trg_enabled || !lbot_option->trg_key.enabled )
		 return;

	  if ( Source::m_pGlobalVars->realtime <= m_flLastTriggerShot + lbot_option->trg_delay ) {
		 if ( lbot_option->trg_aimbot )
			m_triggerBot = true;
		 return;
	  }

	  Autowall::C_FireBulletData data;

	  data.m_Player = m_local;
	  data.m_Weapon = m_weapon;
	  data.m_WeaponData = m_weaponInfo.Xor( );
	  data.m_bPenetration = lbot_option->trg_autowall;

	  data.m_vecStart = m_local->GetEyePosition( );
	  data.m_vecDirection = m_viewAngles.ToVectors( );

	  float dmg = Autowall::FireBullets( &data );
	  if ( dmg < 3.0f ) {
		 m_flLastTriggerShot = 0.0f;
		 return;
	  }

	  C_CSPlayer* pEntity = ( C_CSPlayer* ) data.m_EnterTrace.hit_entity;
	  if ( !pEntity || pEntity->IsDead( ) || !pEntity->IsPlayer( ) ) {
		 m_flLastTriggerShot = 0.0f;
		 return;
	  }

	  if ( pEntity->m_iTeamNum( ) == m_local->m_iTeamNum( ) ) {
		 m_flLastTriggerShot = 0.0f;
		 return;
	  }

	  if ( !lbot_option->trg_head_hitbox && data.m_EnterTrace.hitgroup == Hitgroup_Head
		 || !lbot_option->trg_head_hitbox && data.m_EnterTrace.hitgroup == Hitgroup_Gear // neck
		 || !lbot_option->trg_chest_hitbox && data.m_EnterTrace.hitgroup == Hitgroup_Chest
		 || !lbot_option->trg_stomach_hitbox && data.m_EnterTrace.hitgroup == Hitgroup_Stomach
		 || !lbot_option->trg_arms_hitbox && ( data.m_EnterTrace.hitgroup == Hitgroup_LeftArm || data.m_EnterTrace.hitgroup == Hitgroup_RightArm )
		 || !lbot_option->trg_legs_hitbox && ( data.m_EnterTrace.hitgroup == Hitgroup_LeftLeg || data.m_EnterTrace.hitgroup == Hitgroup_RightLeg ) )
		 return;

	  if ( lbot_option->trg_aimbot )
		 m_triggerBot = true;

	  m_triggerTarget = pEntity;
	  if ( lbot_option->trg_hitchance > 0.0f && g_Vars.globals.RandomInit )
		 if ( !Hitchance( pEntity, m_local->GetEyePosition( ), data.m_EnterTrace.endpos, lbot_option->trg_hitchance ) )
			return;

	  TriggerFire( );
   }

   void CLegitBot::TriggerFire( ) {
	  if ( Source::m_pGlobalVars->realtime > m_flLastTriggerShot + lbot_option->trg_delay ) {
		 m_cmd->buttons |= IN_ATTACK;
		 m_flLastTriggerShot = Source::m_pGlobalVars->realtime;
		 m_flBurstTime = Source::m_pGlobalVars->realtime + RandomFloat( 0.05f, 0.15f ) + lbot_option->trg_burst;
	  }
   }

   bool CLegitBot::Hitchance( C_CSPlayer* player, const Vector& start, const Vector& point, float chance ) {
	  float spread = Engine::Prediction::Instance( )->GetSpread( );
	  float inaccuracy = Engine::Prediction::Instance( )->GetInaccuracy( );
	  if ( spread == 0.0f || inaccuracy == 0.0f ) // nospread enabled
		 return true;

	  const auto weapon_id = m_weapon->m_iItemDefinitionIndex( );
	  const auto crouched = m_local->m_fFlags( ) & FL_DUCKING;
	  const auto sniper = m_weaponInfo->m_iWeaponType == WEAPONTYPE_SNIPER_RIFLE;
	  const auto round_acc = [] ( const float accuracy ) { return roundf( accuracy * 1000.f ) / 1000.f; };

	  float rounded_acc = round_acc( inaccuracy );

	  // no need for hitchance, if we can't increase it anyway.
	  if ( crouched ) {
		 if ( rounded_acc == round_acc( sniper ? m_weaponInfo->m_flInaccuracyCrouchAlt : m_weaponInfo->m_flInaccuracyCrouch ) ) {
			return true;
		 }
	  } else {
		 if ( rounded_acc == round_acc( sniper ? m_weaponInfo->m_flInaccuracyStandAlt : m_weaponInfo->m_flInaccuracyStand ) ) {
			return true;
		 }
	  }

	  constexpr auto HITCHANCE_TRACES = 48;

	  Vector dir = point - start;
	  dir.Normalize( );

	  auto angles = dir.ToEulerAngles( );

	  Vector right, up;
	  Vector forward = angles.ToVectors( &right, &up );

	  chance *= 0.01f;

	  int hits = 0;
	  for ( int i = 0; i < HITCHANCE_TRACES; ++i ) {
		 int seed = ( m_cmd->command_number + i ) % HITCHANCE_TRACES;

		 float flRand1 = g_Vars.globals.SpreadRandom[ seed ].flRand1;
		 float flRandPi1 = g_Vars.globals.SpreadRandom[ seed ].flRandPi1;
		 float flRand2 = g_Vars.globals.SpreadRandom[ seed ].flRand2;
		 float flRandPi2 = g_Vars.globals.SpreadRandom[ seed ].flRandPi2;

		 float m_flRecoilIndex = m_weapon->m_flRecoilIndex( );
		 if ( m_weapon->m_iItemDefinitionIndex( ) == WEAPON_NEGEV && m_flRecoilIndex < 3.f ) {
			for ( int i = 3; i > m_flRecoilIndex; --i ) {
			   flRand1 *= flRand1;
			   flRand2 *= flRand2;
			}

			flRand1 = 1.f - flRand1;
			flRand2 = 1.f - flRand2;
		 }

		 float flRandInaccuracy = flRand1 * inaccuracy;
		 float flRandSpread = flRand2 * spread;

		 float flRandPi1Cos, flRandPi1Sin;
		 DirectX::XMScalarSinCos( &flRandPi1Sin, &flRandPi1Cos, flRandPi1 );

		 float flRandPi2Cos, flRandPi2Sin;
		 DirectX::XMScalarSinCos( &flRandPi2Sin, &flRandPi2Cos, flRandPi2 );

		 float spread_x = flRandPi1Cos * flRandInaccuracy + flRandPi2Cos * flRandSpread;
		 float spread_y = flRandPi1Sin * flRandInaccuracy + flRandPi2Sin * flRandSpread;

		 Autowall::C_FireBulletData data;

		 data.m_Player = m_local;
		 data.m_TargetPlayer = m_player;
		 data.m_Weapon = m_weapon;

		 data.m_vecStart = start;
		 data.m_WeaponData = this->m_weaponInfo.Xor( );
		 data.m_bPenetration = true;

		 data.m_vecDirection.x = forward.x + ( spread_x * right.x ) + ( spread_y * up.x );
		 data.m_vecDirection.y = forward.y + ( spread_x * right.y ) + ( spread_y * up.y );
		 data.m_vecDirection.z = forward.z + ( spread_x * right.z ) + ( spread_y * up.z );

		 float dmg = Autowall::FireBullets( &data );
		 if ( dmg >= 1.0f ) {
			hits++;
		 }

		 // abort if hitchance is already sufficent.
		 if ( static_cast< float >( hits ) / static_cast< float >( HITCHANCE_TRACES ) >= chance )
			return true;

		 // abort if we can no longer reach hitchance.
		 if ( static_cast< float >( hits + HITCHANCE_TRACES - i ) / static_cast< float >( HITCHANCE_TRACES ) < chance )
			return false;
	  }

	  return static_cast< float >( hits ) / static_cast< float >( HITCHANCE_TRACES ) >= chance;
   }

   bool CLegitBot::FindTarget( ) {
	  m_player = m_triggerTarget;
	  if ( ValidateTarget( ) )
		 return true;

	  for ( auto restore : m_restore ) {
		 m_player = restore.player;

		 if ( restore.player == m_triggerTarget )
			continue;

		 if ( !ValidateTarget( ) ) {
			if ( m_deadZoneReached )
			   return false;
			continue;
		 }

		 return true;
	  }

	  return false;
   }

   bool CLegitBot::ValidateTarget( ) {
	  if ( !m_player || m_player->IsDead( ) || m_player->IsDormant( ) || m_player->m_bGunGameImmunity( ) || m_player->entindex( ) == m_local->entindex( ) )
		 return false;

	  if ( g_Vars.legit.ignorejump && !( m_player->m_fFlags( ) & FL_ONGROUND ) )
		 return false;

	  m_target_pos = GetHitscanPos( );

	  if ( m_deadZoneReached || m_target_pos.IsZero( ) )
		 return false;

	  return true;
   }

   Vector CLegitBot::GetHitscanPos( ) {
	  Vector finalPos = Vector( 0.f, 0.f, 0.f );

	  auto studio_model = Source::m_pModelInfo->GetStudiomodel( m_player->GetModel( ) );
	  auto hitboxSet = studio_model->pHitboxSet( m_player->m_nHitboxSet( ) );

	  auto GetHitboxPos = [&] ( int id ) -> Vector {
		 if ( studio_model ) {
			auto hitbox = hitboxSet->pHitbox( id );
			if ( hitbox ) {
			   auto
				  min = Vector{},
				  max = Vector{};

			   auto pos = ( ( hitbox->bbmin + hitbox->bbmax ) * 0.5f ).Transform( m_player->m_CachedBoneData( ).m_Memory.m_pMemory[ hitbox->bone ] );
			   if ( lbot_option->randomize > 0.f ) {
				  int tickrate = int( 1.0f / Source::m_pGlobalVars->interval_per_tick );
				  RandomSeed( m_cmd->command_number / ( tickrate / 2 ) );

				  float random = RandomFloat( -lbot_option->randomize, lbot_option->randomize );
			   #if 0
				  if ( m_cmd->command_number % tickrate > tickrate / 2 )
					 random *= -1.0f;
			   #endif

				  pos += Vector( random, random, random );
			   }

			   return pos;
			}
		 }
	  };

	  if ( lbot_option->hitbox_selection == 1 ) {
		 auto eyePos = m_local->GetEyePosition( );

		 float bestDistance = 360.f;
		 for ( int i = HITBOX_HEAD; i < HITBOX_MAX; i++ ) {
			auto hitbox = hitboxSet->pHitbox( i );
			if ( hitbox->group == Hitgroup_Head && !lbot_option->head_hitbox
			   || hitbox->group == Hitgroup_Gear && !lbot_option->neck_hitbox
			   || hitbox->group == Hitgroup_Chest && !lbot_option->chest_hitbox
			   || ( hitbox->group == Hitgroup_RightLeg || hitbox->group == Hitgroup_LeftLeg ) && !lbot_option->legs_hitbox
			   || ( hitbox->group == Hitgroup_RightArm || hitbox->group == Hitgroup_LeftArm ) && !lbot_option->arms_hitbox )
			   continue;

			if ( i == HITBOX_PELVIS ) {
			   if ( !lbot_option->pelvis_hitbox )
				  continue;
			} else if ( hitbox->group == Hitgroup_Stomach ) {
			   if ( !lbot_option->stomach_hitbox )
				  continue;
			}

			Vector hitboxPos = GetHitboxPos( i );

			float fov = AngleDistance( m_viewAngles, m_local->GetEyePosition( ), hitboxPos );
			if ( lbot_option->deadzone > 0.f && fov < lbot_option->deadzone ) {
			   m_deadZoneReached = true;
			   finalPos = Vector( 0.f, 0.f, 0.f );
			   break;
			}

			if ( fov > lbot_option->fov ) {
			   continue;
			}

			if ( SmokeCheck( eyePos, hitboxPos ) )
			   continue;

			Autowall::C_FireBulletData data;

			data.m_Player = m_local;
			data.m_TargetPlayer = m_player;
			data.m_Weapon = m_weapon;

			data.m_vecStart = eyePos;
			data.m_vecDirection = ( hitboxPos - eyePos ).Normalized( );
			data.m_WeaponData = this->m_weaponInfo.Xor( );
			data.m_bPenetration = lbot_option->autowall;

			float dmg = Autowall::FireBullets( &data );
			if ( dmg < 3.0f )
			   continue;

			if ( bestDistance > fov ) {
			   bestDistance = fov;
			   finalPos = hitboxPos;
			   continue;
			}
		 }

	  } else {
		 int hitbox_idx = 0;
		 switch ( lbot_option->hitbox ) {
			case 0:
			   hitbox_idx = HITBOX_HEAD;
			   break;
			case 1:
			   hitbox_idx = HITBOX_NECK;
			   break;
			case 2:
			   hitbox_idx = HITBOX_CHEST;
			   break;
			case 3:
			   hitbox_idx = HITBOX_STOMACH;
			   break;
			default:
			   break;
		 }

		 auto hitboxPos = GetHitboxPos( lbot_option->hitbox );
		 auto eyePos = m_local->GetEyePosition( );

		 auto fov = AngleDistance( m_cmd->viewangles, m_local->GetEyePosition( ), hitboxPos );
		 auto deadzone_failed = ( lbot_option->deadzone > 0.f && fov < lbot_option->deadzone );
		 if ( deadzone_failed )
			m_deadZoneReached = true;

		 if ( fov > lbot_option->fov || deadzone_failed ) {
			return Vector( 0.f, 0.f, 0.f );
		 }

		 if ( SmokeCheck( eyePos, hitboxPos ) ) {
			return Vector( 0.f, 0.f, 0.f );
		 }

		 Autowall::C_FireBulletData data;

		 data.m_Player = m_local;
		 data.m_TargetPlayer = m_player;
		 data.m_Weapon = m_weapon;

		 data.m_vecStart = eyePos;
		 data.m_vecDirection = ( hitboxPos - eyePos ).Normalized( );
		 data.m_WeaponData = this->m_weaponInfo.Xor( );
		 data.m_bPenetration = lbot_option->autowall;

		 float dmg = Autowall::FireBullets( &data );
		 if ( dmg < 3.0f ) {
			return Vector( 0.f, 0.f, 0.f );
		 }

		 finalPos = hitboxPos;
	  }

	  return finalPos;
   }

   void CLegitBot::AimAtTarget( ) {
	  Vector vecDelta = m_target_pos - m_local->GetEyePosition( );
	  vecDelta.Normalize( );

	  QAngle aimAngles = vecDelta.ToEulerAngles( );
	  // FIXME: when autofire/first shot delay enabled m_iShotsFired == 1
	  if ( m_local->m_iShotsFired( ) >= lbot_option->rcs_shots && m_flFirstShotDelay < Source::m_pGlobalVars->realtime )
		 aimAngles -= m_angRecoil;

	  aimAngles.Normalize( );

	  if ( lbot_option->smooth > 1.0f
		 && aimAngles.x != m_cmd->viewangles.x
		 && aimAngles.y != m_cmd->viewangles.y ) {
		 QAngle delta = aimAngles - m_cmd->viewangles;
		 delta.Normalize( );

		 if ( lbot_option->smooth_type ) {
			float len = std::sqrtf( delta.x * delta.x + delta.y * delta.y );
			float norm_len = 1.0f / len;

			delta = QAngle( delta.x * norm_len, delta.y * norm_len, 0.0f );

			RandomSeed( Source::m_pGlobalVars->tickcount );
			float smooth = RandomFloat( -0.1f, 0.1f ) + lbot_option->smooth / 4.0f;

			smooth = ( 1.0f / smooth ) * ( Source::m_pGlobalVars->interval_per_tick * 64.0f );
			if ( smooth > len )
			   smooth = len;
			delta *= smooth;
		 } else {
			delta *= ( 1.0f / lbot_option->smooth ) * ( Source::m_pGlobalVars->interval_per_tick * 64.0f );
		 }

		 delta.Normalize( );
		 aimAngles = m_cmd->viewangles + delta;
		 aimAngles.Normalize( );
	  }

	  if ( g_Vars.legit.position_adjustment ) {
		 auto restore = std::find_if( m_restore.begin( ), m_restore.end( ), [this] ( const C_EntityRestore& a ) {  return a.player == m_player; } );
		 if ( restore != m_restore.end( ) ) {
			m_cmd->tick_count = restore->tickCount;
		 }
	  }

	  m_cmd->viewangles = aimAngles;
	  Source::m_pEngine->SetViewAngles( aimAngles );
   }
}