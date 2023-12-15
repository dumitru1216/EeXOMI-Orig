#include "LagCompensation.hpp"
#include "source.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "sdk.hpp"
#include "CBaseHandle.hpp"

#include "CVariables.hpp"
#include "Displacement.hpp"
#include "core.hpp"
#include "FnvHash.hpp"

#include "RoundFireBulletsStore.hpp"

#include "EventLogger.hpp"

#include "Autowall.h"
#include "Movement.hpp"

#include "SetupBones.hpp"
#include "Utils/Threading.h"
#include "Utils/shared_mutex.h"

#include "InputSys.hpp"

#include "ServerSounds.hpp"
#include "Fakelag.hpp"

#include "Hooked.hpp"

#include "AnimationSystem.hpp"
#include "TickbaseShift.hpp"

#include <sstream>

#define MT_SETUP_BONES
//#define DEBUG_RESOLVER

namespace Engine
{
   struct LagCompData {
	  std::map< int, Engine::C_EntityLagData > m_PlayerHistory;

	  float m_flLerpTime, m_flOutLatency, m_flServerLatency;
	  bool m_GetEvents = false;
   };

   static LagCompData _lagcomp_data;

   class C_LagCompensation : public Engine::LagCompensation {
   public:
	  virtual void Update( );
	  virtual bool IsRecordOutOfBounds( const Engine::C_LagRecord& record, float target_time = 0.2f, int tickbase_shift = 0, bool tick_count_check = true ) const;

	  virtual Encrypted_t<C_EntityLagData> GetLagData( int entindex ) {
		 C_EntityLagData* data = nullptr;
		 if ( lagData->m_PlayerHistory.count( entindex ) > 0 )
			data = &lagData->m_PlayerHistory.at( entindex );
		 return Encrypted_t<C_EntityLagData>( data );
	  }

	  virtual float GetLerp( ) const {
		 return lagData.Xor( )->m_flLerpTime;
	  }

	  virtual void ClearLagData( ) {
		 lagData->m_PlayerHistory.clear( );
	  }

	  C_LagCompensation( ) : lagData( &_lagcomp_data ) { };
	  virtual ~C_LagCompensation( ) { };
   private:
	  virtual void SetupLerpTime( );
	  Encrypted_t<LagCompData> lagData;
   };

   C_LagCompensation g_LagComp;
   Engine::LagCompensation* Engine::LagCompensation::Get( ) {
	  return &g_LagComp;
   }

   Engine::C_EntityLagData::C_EntityLagData( ) {
	  ;
   }

   void C_LagCompensation::Update( ) {
	  if ( !Source::m_pEngine->IsInGame( ) || !Source::m_pEngine->GetNetChannelInfo( ) ) {
		 lagData->m_PlayerHistory.clear( );
		 return;
	  }

	  auto updateReason = 0;

	  auto local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || !g_Vars.globals.HackIsReady )
		 return;

	  SetupLerpTime( );

	  for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
		 auto player = C_CSPlayer::GetPlayerByIndex( i );
		 if ( !player || player == local || !player->IsPlayer( ) )
			continue;

		 player_info_t player_info;
		 if ( !Source::m_pEngine->GetPlayerInfo( player->m_entIndex, &player_info ) ) {
			continue;
		 }

		 if ( !player->GetClientRenderable( ) )
			continue;

		 if ( Hooked::player_hooks.count( player->m_entIndex ) < 1 )
			continue;

		 auto lag_data = Encrypted_t<C_EntityLagData>( &lagData->m_PlayerHistory[ player->m_entIndex ] );
		 lag_data->UpdateRecordData( lag_data, player, player_info, updateReason );
	  }
   }

   bool C_LagCompensation::IsRecordOutOfBounds( const Engine::C_LagRecord& record, float target_time, int tickbase_shift, bool tick_count_check ) const {
	  auto netchannel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
	  if ( !netchannel.IsValid( ) )
		 return true;

	  auto local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local )
		 return true;

	  // temprorary override
	  tickbase_shift = TickbaseShiftCtx.will_shift_tickbase;

	  float outgoing = netchannel->GetLatency( FLOW_OUTGOING );

	  float correct = 0.f;
	  correct += outgoing;
	  correct += netchannel->GetLatency( FLOW_INCOMING );
	  correct += lagData.Xor( )->m_flLerpTime;

	  correct = Math::Clamp( correct, 0.f, g_Vars.sv_maxunlag->GetFloat( ) );

	  float curtime = TICKS_TO_TIME( local->m_nTickBase( ) - tickbase_shift );
	  float deltaTime = correct - ( curtime - record.m_flSimulationTime );
	  if ( fabsf( deltaTime ) > target_time ) {
		 return true;
	  }

	  if ( tick_count_check ) {
		 // onetap.su part
		 // check if lagrecord will be removed due to outdate ( simtime < server time - sv_maxunlag )
		 // auto simulationTicks = TIME_TO_TICKS( record.m_flSimulationTime );

		 // server tickcount when cmd will arrive
		 auto serverTickcount = Source::m_pGlobalVars->tickcount + 2;
		 if ( g_Vars.globals.Fakeducking )
			serverTickcount = g_Vars.globals.FakeDuckWillChoke;
		 //else if ( g_Vars.fakelag.enabled && g_Vars.fakelag.when_shooting )
		 //	serverTickcount = Source::m_pGlobalVars->tickcount + Source::FakeLag::Get( )->GetFakelagChoke( );

		 // account for outgoing latency
		 // FIXME: find out, should we add or sub 2 ticks? 
		 serverTickcount += int( outgoing / Source::m_pGlobalVars->interval_per_tick ) + 2;

	  #if 0
		 /*auto sim_ticks = TIME_TO_TICKS( record.m_flSimulationTime );
		 if ( sim_ticks - 1 > serverTickcount )
			return false;

		 while ( float( int( TICKS_TO_TIME( sim_ticks ) - g_Vars.sv_maxunlag->GetFloat( ) ) ) <= record.m_flSimulationTime ) {
			if ( ++sim_ticks > serverTickcount )
			   return false;
		 }*/

		 //bool is_record_dead = record.m_flSimulationTime < floorf( curtime - g_Vars.sv_maxunlag->GetFloat( ) );

		 //if ( is_record_dead )
			//return true;

		 return true;
	  #else
		 // remove all records before that time:
		 int dead_time = int( TICKS_TO_TIME( serverTickcount ) - g_Vars.sv_maxunlag->GetFloat( ) );
		 return record.m_flSimulationTime < dead_time;//float( dead_time );
	  #endif
	  }

	  return false;
   }

   void C_LagCompensation::SetupLerpTime( ) {
	  float updaterate = g_Vars.cl_updaterate->GetFloat( );

	  float minupdaterate = g_Vars.sv_minupdaterate->GetFloat( );
	  float maxupdaterate = g_Vars.sv_maxupdaterate->GetFloat( );

	  float min_interp = g_Vars.sv_client_min_interp_ratio->GetFloat( );
	  float max_interp = g_Vars.sv_client_max_interp_ratio->GetFloat( );

	  float flLerpAmount = g_Vars.cl_interp->GetFloat( );
	  float flLerpRatio = g_Vars.cl_interp_ratio->GetFloat( );
	  flLerpRatio = Math::Clamp( flLerpRatio, min_interp, max_interp );
	  if ( flLerpRatio == 0.0f )
		 flLerpRatio = 1.0f;

	  float updateRate = Math::Clamp( updaterate, minupdaterate, maxupdaterate );
	  lagData->m_flLerpTime = std::fmaxf( flLerpAmount, flLerpRatio / updateRate );

	  auto netchannel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
	  lagData->m_flOutLatency = netchannel->GetLatency( FLOW_OUTGOING );
	  lagData->m_flServerLatency = netchannel->GetLatency( FLOW_INCOMING );
   }

   void Engine::C_EntityLagData::UpdateRecordData( Encrypted_t< C_EntityLagData > pThis, C_CSPlayer* player, const player_info_t& info, int updateType ) {
	  auto local = C_CSPlayer::GetLocalPlayer( );
	  auto team_check = g_Vars.rage.enabled && !g_Vars.rage.team_check && player->IsTeammate( C_CSPlayer::GetLocalPlayer( ) );
	  if ( player->IsDead( ) || team_check ) {
		 pThis->m_History.clear( );
		 pThis->m_flLastUpdateTime = 0.0f;
		 pThis->m_flLastScanTime = 0.0f;
		 return;
	  }

	  bool isDormant = player->IsDormant( );

	  // no need to store insane amount of data
	  while ( pThis->m_History.size( ) > int( 1.0f / Source::m_pGlobalVars->interval_per_tick ) ) {
		 pThis->m_History.pop_back( );
	  }

	  if ( isDormant ) {
		 pThis->m_flLastUpdateTime = 0.0f;
		 if ( pThis->m_History.size( ) > 0 && pThis->m_History.front( ).m_bTeleportDistance ) {
			pThis->m_History.clear( );
		 }

		 return;
	  }

	  if ( info.userId != pThis->m_iUserID ) {
		 pThis->Clear( );
		 pThis->m_iUserID = info.userId;
	  }

	  // did player update?
	  float simTime = player->m_flSimulationTime( );
	  if ( pThis->m_flLastUpdateTime >= simTime ) {
		 return;
	  }

	  auto anim_data = AnimationSystem::Get( )->GetAnimationData( player->m_entIndex );
	  if ( !anim_data )
		 return;

	  if ( anim_data->m_AnimationRecord.empty( ) )
		 return;

	  auto anim_record = &anim_data->m_AnimationRecord.front( );
	  if ( anim_record->m_bShiftingTickbase )
		 return;

	  pThis->m_flLastUpdateTime = simTime;

	  bool isTeam = local->IsTeammate( player );

	  // invalidate all records, if player abusing teleport distance
	  if ( !anim_record->m_bIsInvalid && anim_record->m_bTeleportDistance && pThis->m_History.size( ) > 0 ) {
		 for ( auto& record : pThis->m_History )
			record.m_bSkipDueToResolver = true;
	  }

	  // add new record and get reference to newly added record.
	  auto record = Encrypted_t<C_LagRecord>( &pThis->m_History.emplace_front( ) );

	  record->Setup( player );
	  record->m_flRealTime = Source::m_pEngine->GetLastTimeStamp( );
	  record->m_flServerLatency = Engine::LagCompensation::Get( )->m_flServerLatency;
	  record->m_flDuckAmount = anim_record->m_flDuckAmount;
	  record->m_flEyeYaw = anim_record->m_angEyeAngles.yaw;
	  record->m_flEyePitch = anim_record->m_angEyeAngles.pitch;
	  record->m_bIsShoting = anim_record->m_bIsShoting;
	  record->m_bIsValid = !anim_record->m_bIsInvalid;
	  record->m_bBonesCalculated = anim_data->m_bBonesCalculated;
	  record->m_flAnimationVelocity = player->m_PlayerAnimState( )->m_velocity;
	  record->m_bTeleportDistance = anim_record->m_bTeleportDistance;
	  record->m_flAbsRotationLeft = anim_record->m_flAbsRotationLeft;
	  record->m_flAbsRotationRight = anim_record->m_flAbsRotationRight;
	  record->m_flAbsRotationLeftLow = anim_record->m_flAbsRotationLeftLow;
	  record->m_flAbsRotationRightLow = anim_record->m_flAbsRotationRightLow;
	  record->m_iResolverSide = 0;
	  record->m_iLaggedTicks = TIME_TO_TICKS( player->m_flSimulationTime( ) - player->m_flOldSimulationTime( ) );
	  record->m_bIsNoDesyncAnimation = anim_record->m_bNoFakeAngles;

	  if ( anim_record->m_bAnimationResolverUsed ) {
		 record->m_iResolverSide = anim_record->m_iResolverSide;
		 record->m_iRecordPriority = 3;
	  } else if ( anim_data->m_bAimbotTarget ) {
		 if ( std::fabsf( Math::AngleNormalize( Math::AngleDiff( anim_record->m_angEyeAngles.yaw, anim_data->m_flLastScannedYaw ) ) ) < 35.0f )
			record->m_iRecordPriority = 1;
	  }

	  std::memcpy( record->m_BoneMatrix, anim_data->m_Animations[ 0 ].m_Bones, player->m_CachedBoneData( ).m_Size * sizeof( matrix3x4_t ) );
	  if ( record->m_bBonesCalculated ) {
		 std::memcpy( record->m_BoneMatrixRight, anim_data->m_Animations[ 1 ].m_Bones, player->m_CachedBoneData( ).m_Size * sizeof( matrix3x4_t ) );
		 std::memcpy( record->m_BoneMatrixLeft, anim_data->m_Animations[ 2 ].m_Bones, player->m_CachedBoneData( ).m_Size * sizeof( matrix3x4_t ) );
		 std::memcpy( record->m_BoneMatrixLowRight, anim_data->m_Animations[ 3 ].m_Bones, player->m_CachedBoneData( ).m_Size * sizeof( matrix3x4_t ) );
		 std::memcpy( record->m_BoneMatrixLowLeft, anim_data->m_Animations[ 4 ].m_Bones, player->m_CachedBoneData( ).m_Size * sizeof( matrix3x4_t ) );
	  }

	  auto player_is_bot = false;
	  player_info_t player_info;
	  if ( Source::m_pEngine->GetPlayerInfo( player->m_entIndex, &player_info ) ) {
		 player_is_bot = player_info.fakeplayer;
	  }

	  if ( player_is_bot ) {
		 record->m_iRecordPriority = 4;
	  }
   }

   bool Engine::C_EntityLagData::Extrapolate( Encrypted_t< C_EntityLagData > pThis, C_CSPlayer* player, Engine::C_LagRecord* outRecord ) {
	  // need at least 3 records 
	  if ( pThis->m_History.size( ) < 3 )
		 return false;

	  auto& lastRecord = Encrypted_t<C_LagRecord>( &*pThis->m_History.begin( ) );
	  auto& prevRecord = Encrypted_t<C_LagRecord>( &*( pThis->m_History.begin( ) + 1 ) );
	  auto& penultimateRecord = Encrypted_t<C_LagRecord>( &*( pThis->m_History.begin( ) + 2 ) );

	  auto local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || &lastRecord == nullptr || &prevRecord == nullptr || &penultimateRecord == nullptr )
		 return false;

	  auto extrapolatedRecord = Encrypted_t<C_LagRecord>( outRecord );
	  *extrapolatedRecord.Xor( ) = *lastRecord.Xor( );

	  // setup simulation data
	  C_SimulationData data;
	  data.m_vecVeloctity = extrapolatedRecord->m_vecVelocity;
	  data.m_vecOrigin = extrapolatedRecord->m_vecOrigin;
	  data.m_iFlags = extrapolatedRecord->m_iFlags;
	  data.m_player = player;

	  // estimate time when new record will arrive
	  float realtime = TICKS_TO_TIME( Source::m_pGlobalVars->tickcount );
	  int timeTicks = TIME_TO_TICKS( ( realtime + lastRecord.Xor( )->m_flServerLatency ) - lastRecord.Xor( )->m_flRealTime );

	  // clamp between zero and tickrate
	  timeTicks = Math::Clamp< int >( timeTicks, 0, ( int ) ( 1.0f / Source::m_pGlobalVars->interval_per_tick ) );

	  float velocityDirection = atan2( lastRecord.Xor( )->m_vecVelocity.y, lastRecord.Xor( )->m_vecVelocity.x );
	  velocityDirection = RAD2DEG( velocityDirection );

	  float prevVelocityDirection = atan2( prevRecord.Xor( )->m_vecVelocity.y, prevRecord.Xor( )->m_vecVelocity.x );
	  prevVelocityDirection = RAD2DEG( prevVelocityDirection );

	  float deltaTime = lastRecord.Xor( )->m_flSimulationTime - prevRecord.Xor( )->m_flSimulationTime;
	  deltaTime = Math::Clamp < float >( deltaTime, Source::m_pGlobalVars->interval_per_tick, 1.0f );

	  // estimate new velocity direction
	  float deltaDirection = Math::AngleNormalize( velocityDirection - prevVelocityDirection );
	  float directionPerTick = deltaDirection / deltaTime;
	  float currentSpeed = data.m_vecVeloctity.Length2D( );

	  // i dont think using penultimate choke for first time extrapolation is good idea
	  // i think this is polak mistake
	  float penultimateChoke = prevRecord.Xor( )->m_flSimulationTime - penultimateRecord.Xor( )->m_flSimulationTime;
	  penultimateChoke = Math::Clamp( penultimateChoke, Source::m_pGlobalVars->interval_per_tick, 1.0f );

	  int simulationTicks = TIME_TO_TICKS( penultimateChoke );
	  int deltaTicks = TIME_TO_TICKS( deltaTime );

	  deltaTicks = Math::Clamp( deltaTicks, 1, 17 );
	  simulationTicks = Math::Clamp( simulationTicks, 1, 17 );

	  // FIXME: this calculations will be correct only for spinbots and jitters with big delta
	  float yawDeltaPrevious = Math::AngleNormalize( lastRecord.Xor( )->m_flEyeYaw - prevRecord.Xor( )->m_flEyeYaw ) / float( deltaTicks );
	  float yawDeltaPenultimate = Math::AngleNormalize( prevRecord.Xor( )->m_flEyeYaw - penultimateRecord.Xor( )->m_flEyeYaw ) / float( simulationTicks );
	  float yawPerTick = ( yawDeltaPrevious + yawDeltaPenultimate ) * 0.5f;

	  // useless extrapolation
	  if ( yawPerTick == 0.0f && currentSpeed <= 0.0f )
		 return false;

	  data.m_bJumped = true;
	  if ( lastRecord.Xor( )->m_iFlags & FL_ONGROUND ) {
		 if ( prevRecord.Xor( )->m_iFlags & FL_ONGROUND )
			data.m_bJumped = false;
	  }

	  int flags = player->m_fFlags( );
	  bool addTicks = false;

	  auto realtime_backup = Source::m_pGlobalVars->realtime;
	  auto curtime = Source::m_pGlobalVars->curtime;
	  auto frametime = Source::m_pGlobalVars->frametime;
	  auto absoluteframetime = Source::m_pGlobalVars->absoluteframetime;
	  auto framecount = Source::m_pGlobalVars->framecount;
	  auto tickcount = Source::m_pGlobalVars->tickcount;
	  auto interpolation_amount = Source::m_pGlobalVars->interpolation_amount;

	  float time = player->m_flSimulationTime( );
	  int ticks = TIME_TO_TICKS( time );

	  // calculate animations based on ticks aka server frames instead of render frames
	  Source::m_pGlobalVars->realtime = time;
	  Source::m_pGlobalVars->curtime = time;
	  Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;
	  Source::m_pGlobalVars->absoluteframetime = Source::m_pGlobalVars->interval_per_tick;
	  Source::m_pGlobalVars->framecount = ticks;
	  Source::m_pGlobalVars->tickcount = ticks;
	  Source::m_pGlobalVars->interpolation_amount = 0.f;

	  C_AnimationLayer layers[ 13 ];
	  std::memcpy( layers, player->m_AnimOverlay( ).m_Memory.m_pMemory, std::min( 13, player->m_AnimOverlay( ).m_Size ) * sizeof( C_AnimationLayer ) );

	  CCSGOPlayerAnimState animState = *player->m_PlayerAnimState( );

	  // backup current data 
	  C_LagRecord record;
	  record.Setup( player );

	  QAngle backupAngles = player->m_angEyeAngles( );

	  // Simulate player movement
	  for ( int latencyDelta = timeTicks - simulationTicks; latencyDelta >= 0; latencyDelta -= simulationTicks ) {
		 while ( simulationTicks > 0 ) {
			// rotate movement
			float velocitySin, velocityCos;
			DirectX::XMScalarSinCos( &velocitySin, &velocityCos, DEG2RAD( velocityDirection ) );

			// TODO: extrapolate speed too
			data.m_vecVeloctity.y = velocitySin * currentSpeed;
			data.m_vecVeloctity.x = velocityCos * currentSpeed;

			velocityDirection += Source::m_pGlobalVars->interval_per_tick * directionPerTick;

			SimulateMovement( data );

			if ( data.m_iFlags & FL_ONGROUND )
			   player->m_fFlags( ) |= FL_ONGROUND;
			else
			   player->m_fFlags( ) &= ~FL_ONGROUND;

			player->m_vecVelocity( ) = data.m_vecVeloctity;
			player->SetAbsVelocity( data.m_vecVeloctity );
			player->SetAbsOrigin( data.m_vecOrigin );

			player->m_angEyeAngles( ).yaw = Math::AngleNormalize( player->m_angEyeAngles( ).yaw + yawPerTick );

			player->m_flOldSimulationTime( ) = player->m_flSimulationTime( );
			player->m_flSimulationTime( ) += Source::m_pGlobalVars->interval_per_tick;
			Source::m_pGlobalVars->curtime = player->m_flSimulationTime( );

			{
			   auto animState = player->m_PlayerAnimState( );
			   if ( player->m_PlayerAnimState( )->m_nLastFrame == Source::m_pGlobalVars->framecount )
				  animState->m_nLastFrame = Source::m_pGlobalVars->framecount - 1;
			   animState->m_Player = player;
			}

			float chokedTime = Source::m_pGlobalVars->interval_per_tick;

			// force to use correct abs origin and velocity ( no CalcAbsolutePosition and CalcAbsoluteVelocity calls )
			player->m_iEFlags( ) &= ~( EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY );

			// Update pose parameters and abs rotation
			player->FixedAnimationsUpdate( );

			// restore server anim overlays
			std::memcpy( player->m_AnimOverlay( ).m_Memory.m_pMemory, layers, std::min( 13, player->m_AnimOverlay( ).m_Size ) * sizeof( C_AnimationLayer ) );

			--simulationTicks;
		 }

		 simulationTicks = deltaTicks;
		 if ( addTicks )
			simulationTicks++;

		 addTicks = deltaTicks > 1;
	  }
	  player->m_angEyeAngles( ) = backupAngles;

	  player->m_fFlags( ) = flags;

	  // build valid bones matrix
	  player->InvalidateBoneCache( );

	  Engine::C_SetupBones boneSetup;
	  boneSetup.Init( player, BONE_USED_BY_ANYTHING | BONE_USED_BY_ATTACHMENT );
	  boneSetup.Setup( );

	  // setup extrapolated recorrd
	  extrapolatedRecord->m_flSimulationTime = player->m_flSimulationTime( );
	  extrapolatedRecord->m_vecOrigin = data.m_vecOrigin;
	  extrapolatedRecord->m_vecVelocity = data.m_vecVeloctity;
	  extrapolatedRecord->m_bExtrapolated = true;

	  std::memcpy( extrapolatedRecord->m_BoneMatrix, player->m_CachedBoneData( ).m_Memory.m_pMemory,
				   player->m_CachedBoneData( ).m_Size * sizeof( matrix3x4_t ) );

	  // restore animstate
	  *player->m_PlayerAnimState( ) = animState;

	  Source::m_pGlobalVars->realtime = realtime_backup;
	  Source::m_pGlobalVars->curtime = curtime;
	  Source::m_pGlobalVars->frametime = frametime;
	  Source::m_pGlobalVars->absoluteframetime = absoluteframetime;
	  Source::m_pGlobalVars->framecount = framecount;
	  Source::m_pGlobalVars->tickcount = tickcount;
	  Source::m_pGlobalVars->interpolation_amount = interpolation_amount;

	  record.Apply( player );

	  return true;
   }

   void Engine::C_EntityLagData::Clear( ) {
	  this->m_History.clear( );
	  m_iUserID = -1;
	  m_flLastScanTime = 0.0f;
	  m_flLastUpdateTime = 0.0f;
   }

   Vector Engine::C_EntityLagData::GetExtrapolatedPosition( C_CSPlayer* player ) {
	  Vector origin = player->m_vecOrigin( );
	  if ( m_History.size( ) <= 1 )
		 return origin;

	  auto lastRecord = Encrypted_t<C_LagRecord>( &m_History.front( ) );
	  float curtime = TICKS_TO_TIME( Source::m_pGlobalVars->tickcount );
	  return origin + player->m_vecVelocity( ) * ( curtime - lastRecord->m_flRealTime + lastRecord->m_flServerLatency );
   }

   float C_LagRecord::GetAbsYaw( int matrixIdx ) {
	  switch ( matrixIdx ) {
		 case -1:
		 return this->m_flAbsRotationRight;
		 break;
		 case 1:
		 return this->m_flAbsRotationLeft;
		 break;
		 default:
		 return this->m_angAngles.yaw;
		 break;
	  }
   }

   matrix3x4_t* C_LagRecord::GetBoneMatrix( int matrixIdx ) {
	  if ( !this->m_bBonesCalculated )
		 return this->m_BoneMatrix;

	  switch ( matrixIdx ) {
		 case 0:
		 return this->m_BoneMatrix;
		 case -1:
		 return this->m_BoneMatrixRight;
		 break;
		 case 1:
		 return this->m_BoneMatrixLeft;
		 break;
		 case 2:
		 return this->m_BoneMatrixLowRight;
		 break;
		 case 3:
		 return this->m_BoneMatrixLowLeft;
		 break;
		 default:
		 return this->m_BoneMatrix;
		 break;
	  }
   }

   void Engine::C_LagRecord::Setup( C_CSPlayer* player ) {
	  auto collidable = player->m_Collision( );
	  this->m_vecMins = collidable->m_vecMins;
	  this->m_vecMaxs = collidable->m_vecMaxs;

	  this->m_vecOrigin = player->m_vecOrigin( );
	  this->m_angAngles = player->GetAbsAngles( );

	  this->m_vecVelocity = player->m_vecVelocity( );
	  this->m_flSimulationTime = player->m_flSimulationTime( );

	  this->m_iFlags = player->m_fFlags( );

	  std::memcpy( this->m_BoneMatrix, player->m_CachedBoneData( ).m_Memory.m_pMemory,
				   player->m_CachedBoneData( ).m_Size * sizeof( matrix3x4_t ) );

	  this->player = player;
   }

   void Engine::C_LagRecord::Apply( C_CSPlayer* player, int matrixIdx ) {
	  auto collidable = player->m_Collision( );
	  collidable->SetCollisionBounds( this->m_vecMins, this->m_vecMaxs );

	  player->m_flSimulationTime( ) = this->m_flSimulationTime;

	  QAngle absAngles = this->m_angAngles;
	  absAngles.yaw = this->GetAbsYaw( matrixIdx );

	  player->SetAbsAngles( absAngles );
	  player->SetAbsOrigin( this->m_vecOrigin );

	  matrix3x4_t* matrix = GetBoneMatrix( matrixIdx );

	  std::memcpy( player->m_CachedBoneData( ).m_Memory.m_pMemory, matrix,
				   player->m_CachedBoneData( ).m_Size * sizeof( matrix3x4_t ) );

	  // force bone cache
	  player->m_iMostRecentModelBoneCounter( ) = *( int* ) Engine::Displacement.Data.m_uModelBoneCounter;
	  player->m_BoneAccessor( ).m_ReadableBones = player->m_BoneAccessor( ).m_WritableBones = 0xFFFFFFFF;
	  player->m_flLastBoneSetupTime( ) = FLT_MAX;
   }

   void C_BaseLagRecord::Setup( C_CSPlayer* player ) {
	  auto collidable = player->m_Collision( );
	  this->m_vecMins = collidable->m_vecMins;
	  this->m_vecMaxs = collidable->m_vecMaxs;

	  this->m_flSimulationTime = player->m_flSimulationTime( );

	  this->m_angAngles = player->GetAbsAngles( );
	  this->m_vecOrigin = player->m_vecOrigin( );

	  std::memcpy( this->m_BoneMatrix, player->m_CachedBoneData( ).m_Memory.m_pMemory,
				   player->m_CachedBoneData( ).m_Size * sizeof( matrix3x4_t ) );

	  this->player = player;
   }

   void C_BaseLagRecord::Apply( C_CSPlayer* player ) {
	  auto collidable = player->m_Collision( );
	  collidable->SetCollisionBounds( this->m_vecMins, this->m_vecMaxs );

	  player->m_flSimulationTime( ) = this->m_flSimulationTime;

	  player->SetAbsAngles( this->m_angAngles );
	  player->SetAbsOrigin( this->m_vecOrigin );

	  std::memcpy( player->m_CachedBoneData( ).m_Memory.m_pMemory, this->m_BoneMatrix,
				   player->m_CachedBoneData( ).m_Size * sizeof( matrix3x4_t ) );

	  // force bone cache
	  player->m_iMostRecentModelBoneCounter( ) = *( int* ) Engine::Displacement.Data.m_uModelBoneCounter;
	  player->m_BoneAccessor( ).m_ReadableBones = player->m_BoneAccessor( ).m_WritableBones = 0xFFFFFFFF;
	  player->m_flLastBoneSetupTime( ) = FLT_MAX;
   }
}
