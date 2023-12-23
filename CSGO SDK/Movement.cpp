#include "source.hpp"
#include "Player.hpp"
#include "weapon.hpp"
#include "Movement.hpp"
#include "Math.h"
#include "AntiAim.hpp"
#include "CBaseHandle.hpp"
#include "InputSys.hpp"
#include "FakeLag.hpp"
#include "Ragebot.hpp"
#include "AntiAim.hpp"
#include "KnifeBot.hpp"
#include "Prediction.hpp"
#include "LegitBot.hpp"
#include "Displacement.hpp"
#include "SetupBones.hpp"
#include "utils\threading.h"
#include "TickbaseShift.hpp"

// todo: move this
C_AnimationLayer FakeAnimLayers[ 13 ];

float PreviousYaw = 0.0f;
extern bool WasShootinPeek;
extern Vector AutoPeekPos;

extern matrix3x4_t HeadBone;

void SimulateMovement( C_SimulationData& data ) {
   if ( !( data.m_iFlags & FL_ONGROUND ) ) {
	  data.m_vecVeloctity.z -= ( Source::m_pGlobalVars->interval_per_tick * g_Vars.sv_gravity->GetFloat( ) * 0.5f );
   } else if ( data.m_bJumped ) {
	  data.m_bJumped = false;
	  data.m_vecVeloctity.z = g_Vars.sv_jump_impulse->GetFloat( );
	  data.m_iFlags &= ~FL_ONGROUND;
   }

   // can't step up onto very steep slopes
   static const float MIN_STEP_NORMAL = 0.7f;

   if ( !data.m_vecVeloctity.IsZero( ) ) {
	  auto collidable = data.m_player->GetCollideable( );
	  const Vector mins = collidable->OBBMins( );
	  const Vector max = collidable->OBBMaxs( );

	  const Vector src = data.m_vecOrigin;
	  Vector end = src + ( data.m_vecVeloctity * Source::m_pGlobalVars->interval_per_tick );

	  Ray_t ray;
	  ray.Init( src, end, mins, max );

	  CGameTrace trace;
	  CTraceFilter filter;
	  filter.pSkip = data.m_player;

	  Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

	  // CGameMovement::TryPlayerMove
	  if ( trace.fraction != 1.f ) {
		 // BUGFIXME: is it should be 4? ( not 2 )
		 for ( int i = 0; i < 2; i++ ) {
			// decompose velocity into plane
			data.m_vecVeloctity -= trace.plane.normal * data.m_vecVeloctity.Dot( trace.plane.normal );

			const float dot = data.m_vecVeloctity.Dot( trace.plane.normal );
			if ( dot < 0.f ) { // moving against plane
			   data.m_vecVeloctity.x -= dot * trace.plane.normal.x;
			   data.m_vecVeloctity.y -= dot * trace.plane.normal.y;
			   data.m_vecVeloctity.z -= dot * trace.plane.normal.z;
			}

			end = trace.endpos + ( data.m_vecVeloctity * ( Source::m_pGlobalVars->interval_per_tick * ( 1.f - trace.fraction ) ) );

			ray.Init( trace.endpos, end, mins, max );
			Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );
			if ( trace.fraction == 1.f )
			   break;
		 }
	  }

	  data.m_vecOrigin = trace.endpos;
	  end = trace.endpos;
	  end.z -= 2.f;

	  ray.Init( data.m_vecOrigin, end, mins, max );
	  Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

	  data.m_iFlags &= ~FL_ONGROUND;

	  if ( trace.DidHit( ) && trace.plane.normal.z >= MIN_STEP_NORMAL ) {
		 data.m_iFlags |= FL_ONGROUND;
	  }

	  if ( data.m_iFlags & FL_ONGROUND )
		 data.m_vecVeloctity.z = 0.0f;
	  else
		 data.m_vecVeloctity.z -= Source::m_pGlobalVars->interval_per_tick * g_Vars.sv_gravity->GetFloat( ) * 0.5f;
   }
}

void ExtrapolatePlayer( C_SimulationData& data, int simulationTicks, const Vector& wishvel, float wishspeed, float maxSpeed ) {
   for ( int i = 0; i < simulationTicks; ++i ) {
	  SimulateMovement( data );

   #if 0
	  if ( InputSys::Get( )->IsKeyDown( VirtualKeys::J ) ) {
		 Source::m_pDebugOverlay->AddBoxOverlay( data.m_vecOrigin,
												 Vector( -2.0f, -2.0f, -2.0f ), Vector( 2.0f, 2.0f, 2.0f ), QAngle( 0.0f, 0.0f, 0.0f ), 255, 180, 0, 200, 2.0f );
	  }
   #endif

	  if ( data.m_vecVeloctity.IsZero( ) )
		 break;

	  auto Accelerate = [&data] ( const Vector& wishdir, float wishspeed, float maxSpeed ) {
		 float addspeed, accelspeed, currentspeed;

		 // See if we are changing direction a bit
		 currentspeed = data.m_vecVeloctity.Dot( wishdir );

		 // Reduce wishspeed by the amount of veer.
		 addspeed = wishspeed - currentspeed;

		 // If not going to add any speed, done.
		 if ( addspeed <= 0 )
			return;

		 // Determine amount of accleration.
		 static int playerSurfaceFrictionOffset = Horizon::Memory::FindInDataMap( data.m_player->GetPredDescMap( ), XorStr( "m_surfaceFriction" ) );
		 float playerSurfaceFriction = *( float* ) ( uintptr_t( data.m_player ) + playerSurfaceFrictionOffset );
		 accelspeed = g_Vars.sv_accelerate->GetFloat( ) * Source::m_pGlobalVars->interval_per_tick * maxSpeed * playerSurfaceFriction;

		 // Cap at addspeed
		 if ( accelspeed > addspeed )
			accelspeed = addspeed;

		 // Adjust velocity.
		 for ( int i = 0; i < 3; i++ ) {
			data.m_vecVeloctity[ i ] += accelspeed * wishdir[ i ];
		 }

		 float len = data.m_vecVeloctity.Length2D( );
		 if ( len > ( ( data.m_iFlags & FL_ONGROUND ) ? ( maxSpeed ) : 320.0f ) ) {
			float z = data.m_vecVeloctity.z;
			data.m_vecVeloctity.z = 0.0f;
			data.m_vecVeloctity = data.m_vecVeloctity.Normalized( ) * ( ( data.m_iFlags & FL_ONGROUND ) ? ( maxSpeed ) : 320.0f );
			data.m_vecVeloctity.z = z;
		 }
	  };

	  Accelerate( wishvel, wishspeed, maxSpeed );
   }
}

void RotateMovement( Encrypted_t<CUserCmd> cmd, QAngle wish_angle, QAngle old_angles ) {
   // aimware movement fix, reversed by ph4ge/senator for gucci
   if ( old_angles.x != wish_angle.x || old_angles.y != wish_angle.y || old_angles.z != wish_angle.z ) {
	  Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

	  auto viewangles = old_angles;
	  auto movedata = Vector( cmd->forwardmove, cmd->sidemove, cmd->upmove );
	  viewangles.Normalize( );

	  if ( viewangles.z != 0.f ) {
		 auto pLocal = C_CSPlayer::GetLocalPlayer( );

		 if ( pLocal && !( pLocal->m_fFlags( ) & FL_ONGROUND ) )
			movedata.y = 0.f;
	  }

	  wish_forward = wish_angle.ToVectors( &wish_right, &wish_up );
	  cmd_forward = viewangles.ToVectors( &cmd_right, &cmd_up );

	  auto v8 = sqrt( wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y ), v10 = sqrt( wish_right.x * wish_right.x + wish_right.y * wish_right.y ), v12 = sqrt( wish_up.z * wish_up.z );

	  Vector wish_forward_norm( 1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f ),
		 wish_right_norm( 1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f ),
		 wish_up_norm( 0.f, 0.f, 1.0f / v12 * wish_up.z );

	  auto v14 = sqrt( cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y ), v16 = sqrt( cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y ), v18 = sqrt( cmd_up.z * cmd_up.z );

	  Vector cmd_forward_norm( 1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f ),
		 cmd_right_norm( 1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f ),
		 cmd_up_norm( 0.f, 0.f, 1.0f / v18 * cmd_up.z );

	  auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

	  cmd->forwardmove = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 + ( cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28 ) + ( cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27 );
	  cmd->sidemove = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 + ( cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28 ) + ( cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27 );
	  cmd->upmove = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 + ( cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28 ) + ( cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27 );

	  cmd->forwardmove = Math::Clamp( cmd->forwardmove, -g_Vars.cl_forwardspeed->GetFloat( ), g_Vars.cl_forwardspeed->GetFloat( ) );
	  cmd->sidemove = Math::Clamp( cmd->sidemove, -g_Vars.cl_sidespeed->GetFloat( ), g_Vars.cl_sidespeed->GetFloat( ) );
	  cmd->upmove = Math::Clamp( cmd->upmove, -g_Vars.cl_upspeed->GetFloat( ), g_Vars.cl_upspeed->GetFloat( ) );

	  auto local_player = C_CSPlayer::GetLocalPlayer( );
	  if ( local_player && local_player->m_MoveType( ) != MOVETYPE_LADDER ) {
		 cmd->buttons &= ~( IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD );

		 if ( g_Vars.misc.active && g_Vars.misc.slide_walk ) {
			if ( cmd->sidemove != 0.0f )
			   cmd->buttons |= ( cmd->sidemove < 0.0f ) ? IN_MOVERIGHT : IN_MOVELEFT;

			if ( cmd->forwardmove != 0.0f )
			   cmd->buttons |= ( cmd->forwardmove < 0.0f ) ? IN_FORWARD : IN_BACK;
		 } else {
			if ( cmd->sidemove != 0.0f )
			   cmd->buttons |= ( cmd->sidemove < 0.0f ) ? IN_MOVELEFT : IN_MOVERIGHT;

			if ( cmd->forwardmove != 0.0f )
			   cmd->buttons |= ( cmd->forwardmove < 0.0f ) ? IN_BACK : IN_FORWARD;
		 }
	  }
   }
}

namespace Source
{
   struct MovementData {
	  Encrypted_t<CUserCmd> m_pCmd = nullptr;
	  C_CSPlayer* m_pLocal = nullptr;
	  bool* m_pSendPacket = nullptr;

	  uintptr_t* m_pCLMoveReturn = nullptr;

	  bool m_bInRecursion = false;
	  int m_iRecursionTicks = 0;

	  QAngle m_angRenderViewangles{};
	  QAngle m_angPreviousAngles{};
	  QAngle m_angMovementAngle{};
	  float m_flOldYaw = 0.0f;

	  int m_fPrePredFlags = 0;

	  int   m_bStopPlayer = 0;
	  bool  m_bMinimalSpeed = false;
	  bool  m_bFastStop = false;
	  bool  m_bStoppedTick = false;
	  bool  m_bFakeducking = false;
	  bool  m_bStopOk = false;
	  bool  jumped_last_tick = false;
	  bool  should_fake_jump = false;

	  float m_flLowerBodyUpdateTime = 0.0f;

	  int m_iLatency = 0;
   };

   static MovementData _move_data;

   class C_Movement : public Movement {
   public:
	  virtual void PrePrediction( Encrypted_t<CUserCmd> cmd, C_CSPlayer* pLocal, bool* pSendPacket, uintptr_t* cl_move );
	  virtual void InPrediction( );
	  virtual void PostPrediction( );
	  virtual void ThirdPerson( );

	  virtual float GetLBYUpdateTime( ) {
		 return  movementData->m_flLowerBodyUpdateTime;
	  }

	  virtual QAngle& GetMovementAngle( ) {
		 return movementData->m_angMovementAngle;
	  }

	  virtual bool StopPlayer( ) {
		 if ( movementData->m_bStoppedTick ) {
			return false;
		 }

		 return AutoStopInternal( );
	  }

	  virtual void StopPlayerAtMinimalSpeed( ) {
		 movementData->m_bMinimalSpeed = true;
	  }

	  virtual void AutoStop( int ticks = 1 );
	  virtual bool GetStopState( );
	  virtual void SpoofImpacts( );
	  virtual bool CreateMoveRecursion( );

	  virtual void FixLocalPlayerAnimations( bool is_outgoing );
	  virtual void HandleVisualAnimation( );

	  C_Movement( ) : movementData( &_move_data ) { ; };
	  virtual ~C_Movement( ) { };

   private:
	  Encrypted_t<MovementData> movementData;

	  bool AutoStopInternal( );
	  void AutoJump( );
	  void AutoStrafe( );
	  void FakeDuck( );
	  void SlowWalk( float speed );

	  inline float GetMaxSpeed( ) {
		 if ( !movementData->m_pLocal )
			return 250.0f;

		 auto hWeapon = movementData->m_pLocal->m_hActiveWeapon( );
		 if ( hWeapon == -1 )
			return 250.0f;

		 auto pWeapon = reinterpret_cast< C_WeaponCSBaseGun* >( movementData->m_pLocal->m_hActiveWeapon( ).Get( ) );
		 if ( !pWeapon )
			return 250.0f;

		 auto pWeaponData = pWeapon->GetCSWeaponData( );
		 if ( !pWeaponData.IsValid( ) )
			return 250.0f;

		 if ( pWeapon->m_weaponMode( ) == 0 )
			return pWeaponData->m_flMaxSpeed;

		 return pWeaponData->m_flMaxSpeed2;
	  }
   };

   Movement* Movement::Get( ) {
	  static C_Movement movement;
	  return &movement;
   }

   void C_Movement::PrePrediction( Encrypted_t<CUserCmd> cmd, C_CSPlayer* pLocal, bool* pSendPacket, uintptr_t* cl_move ) {
	  movementData->m_pCmd = cmd;
	  movementData->m_pLocal = pLocal;
	  movementData->m_pSendPacket = pSendPacket;
	  movementData->m_angRenderViewangles = movementData->m_pCmd->viewangles;

	  movementData->m_pCLMoveReturn = cl_move;
	  movementData->m_bStopOk = false;
	  movementData->m_bStoppedTick = false;

	  // static int autostop_ticks = 0;

	  if ( !pLocal || pLocal->IsDead( ) )
		 return;

	  {
		 float delta = std::fabsf( Source::m_pEngine->GetNetChannelInfo( )->GetLatency( FLOW_OUTGOING ) - Source::m_pEngine->GetNetChannelInfo( )->GetAvgLatency( FLOW_OUTGOING ) );
		 movementData->m_iLatency = TIME_TO_TICKS( delta ) + 1 - **( int** ) Engine::Displacement.Data.m_uHostFrameTicks;
	  }

	  movementData->m_angMovementAngle = movementData->m_pCmd->viewangles;

	  if ( g_Vars.misc.active ) {
		 movementData->m_fPrePredFlags = movementData->m_pLocal->m_fFlags( );

		 if ( g_Vars.misc.fastduck )
			movementData->m_pCmd->buttons |= IN_BULLRUSH;

		 if ( g_Vars.misc.duckjump ) {
			if ( movementData->m_pCmd->buttons & IN_JUMP ) {
			   if ( !( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) )
				  movementData->m_pCmd->buttons |= IN_DUCK;
			}
		 }

		 if ( g_Vars.misc.minijump && !( ( movementData->m_pCmd->buttons & IN_DUCK ) ) ) {
			if ( !( movementData->m_pCmd->buttons & IN_JUMP ) || ( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) || movementData->m_pLocal->m_vecVelocity( ).z >= -140.0f )
			   movementData->m_pCmd->buttons &= ~IN_DUCK;
			else
			   movementData->m_pCmd->buttons |= IN_DUCK;
		 }

		 if ( g_Vars.misc.autojump )
			AutoJump( );

		 if ( g_Vars.misc.autostrafer )
			AutoStrafe( );

		 FakeDuck( );

		 if ( g_Vars.misc.quickstop && !WasShootinPeek && movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND && !( movementData->m_pCmd->buttons & IN_JUMP ) &&
			  movementData->m_pLocal->m_vecVelocity( ).Length( ) > 1.2f ) {
			if ( !( movementData->m_pCmd->buttons & IN_JUMP ) && movementData->m_pCmd->forwardmove == movementData->m_pCmd->sidemove && movementData->m_pCmd->sidemove == 0.0f ) {
			   movementData->m_bStopPlayer = 1;
			   movementData->m_bMinimalSpeed = false;
			}
		 }
	  }

   #if 0
	  if ( !movementData->m_bStopPlayer && autostop_ticks > 0 ) {
		 printf( XorStr( "stoped in %d ticks\n" ), autostop_ticks );
		 autostop_ticks = 0;
	  }
   #endif

	  bool peek = true;
	  if ( g_Vars.misc.autopeek && WasShootinPeek && !AutoPeekPos.IsZero( ) ) {
		 cmd->buttons &= ~IN_JUMP;

		 auto delta = AutoPeekPos - movementData->m_pLocal->m_vecOrigin( );
		 movementData->m_angMovementAngle = delta.ToEulerAngles( );

		 peek = false;

		 movementData->m_pCmd->forwardmove = g_Vars.cl_forwardspeed->GetFloat( );
		 movementData->m_pCmd->sidemove = 0.0f;
		 if ( delta.Length2D( ) <= std::fmaxf( 24.0f, movementData->m_pLocal->m_vecVelocity( ).Length2D( ) * Source::m_pGlobalVars->interval_per_tick ) ) {
			float maxSpeed = GetMaxSpeed( );
			static int playerSurfaceFrictionOffset = Horizon::Memory::FindInDataMap( movementData->m_pLocal->GetPredDescMap( ), XorStr( "m_surfaceFriction" ) );
			float playerSurfaceFriction = *( float* ) ( uintptr_t( movementData->m_pLocal ) + playerSurfaceFrictionOffset );
			float max_accelspeed = g_Vars.sv_accelerate->GetFloat( ) * Source::m_pGlobalVars->interval_per_tick * maxSpeed * playerSurfaceFriction;

			movementData->m_bStopPlayer = int( movementData->m_pLocal->m_vecVelocity( ).Length2D( ) / max_accelspeed ) - 1;
			movementData->m_bMinimalSpeed = false;

			peek = true;

			WasShootinPeek = false;
		 }
	  } else {
		 WasShootinPeek = false;
	  }

	  if ( peek ) {
		 if ( movementData->m_bStopPlayer && AutoStopInternal( ) ) {
			movementData->m_bStoppedTick = true;
		 }
	  }

	  if ( !movementData->m_bStoppedTick ) {
		 if ( g_Vars.misc.active && g_Vars.misc.slow_walk && g_Vars.misc.slow_walk_bind.enabled ) {
			SlowWalk( g_Vars.misc.slow_walk_speed * 0.01f );
		 } else if ( g_Vars.misc.active && g_Vars.misc.accurate_walk && movementData->m_pCmd->buttons & IN_SPEED ) {
			SlowWalk( 0.325f );
		 }
		 if ( g_Vars.antiaim.twist_speed && ( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) ) {
			 static float twisted_speed = 0.f;
			 static bool twist_switch = false;

			 if ( twisted_speed <= 0.0f )
				twist_switch = true;
			 else if ( twisted_speed >= 0.5f )
				twist_switch = false;

			 twist_switch ? twisted_speed += 0.1f : twisted_speed -= 0.1f;

			 SlowWalk( 0.95f + twist_switch );
		  }
	  }

	  movementData->m_bStopPlayer--;
	  movementData->m_bMinimalSpeed = false;
	  if ( movementData->m_bStopPlayer < 0 )
		 movementData->m_bStopPlayer = 0;

	  Source::AntiAimbot::Get( )->PrePrediction( movementData->m_pSendPacket, cmd );

	  movementData->m_pCmd->viewangles.Normalize( );
	  RotateMovement( movementData->m_pCmd, movementData->m_angMovementAngle, movementData->m_pCmd->viewangles );
   }

   void C_Movement::InPrediction( ) {
	  if ( !movementData->m_pLocal  || movementData->m_pLocal->IsDead( ) || !movementData->m_pSendPacket )
		 return;

	  if ( g_Vars.misc.edgejump && g_Vars.misc.edgejump_bind.enabled &&
		   Engine::Prediction::Instance( ).GetFlags( ) & FL_ONGROUND && !( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) ) {
		 movementData->m_pCmd->buttons |= IN_JUMP;
	  }

	  g_Vars.globals.HideShots = false;

	  auto animState = movementData->m_pLocal->m_PlayerAnimState( );
	  if ( !animState )
		 return;

	  movementData->m_pLocal->SetAbsAngles( QAngle( 0.f, animState->m_flAbsRotation, 0.f ) );

	  movementData->m_pLocal->InvalidateBoneCache( );

	  // predict animation data
	  {
		 auto animStateBackup = *movementData->m_pLocal->m_PlayerAnimState( );

		 float poses[ 20 ];
		 C_AnimationLayer layers[ 13 ];
		 std::memcpy( layers, movementData->m_pLocal->m_AnimOverlay( ).Base( ), sizeof( layers ) );
		 std::memcpy( poses, movementData->m_pLocal->m_flPoseParameter( ), sizeof( poses ) );

		 movementData->m_pLocal->pl( ).v_angle = movementData->m_pCmd->viewangles;

		 auto land = !( Engine::Prediction::Instance( )->GetFlags( ) & FL_ONGROUND ) && movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND;
		 if ( g_Vars.globals.Fakeducking || animState->m_bHitground || land ) {
			movementData->m_pLocal->pl( ).v_angle.pitch = 0.0f;
			movementData->m_pLocal->pl( ).v_angle.yaw = movementData->m_angPreviousAngles.yaw;
		 }

		 if ( animState->m_nLastFrame == Source::m_pGlobalVars->framecount )
			animState->m_nLastFrame = Source::m_pGlobalVars->framecount - 1;

		 movementData->m_pLocal->m_iEFlags( ) &= ~( EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY );

		 auto ishltv = Source::m_pClientState->m_bIsHLTV( );
		 auto updateanim = movementData->m_pLocal->m_bClientSideAnimation( );
		 Source::m_pClientState->m_bIsHLTV( ) = true;
		 movementData->m_pLocal->m_bClientSideAnimation( ) = true;
		 movementData->m_pLocal->UpdateClientSideAnimation( );
		 movementData->m_pLocal->m_bClientSideAnimation( ) = updateanim;
		 Source::m_pClientState->m_bIsHLTV( ) = ishltv;

		 Engine::C_SetupBones boneSetup;
		 boneSetup.Init( movementData->m_pLocal, BONE_USED_BY_ANYTHING, movementData->m_pLocal->m_CachedBoneData( ).Base( ) );
		 boneSetup.Setup( );

		 if ( movementData->m_pLocal->m_CachedBoneData( ).Base( ) != movementData->m_pLocal->m_BoneAccessor( ).m_pBones ) {
			std::memcpy( movementData->m_pLocal->m_BoneAccessor( ).m_pBones, movementData->m_pLocal->m_CachedBoneData( ).Base( ), movementData->m_pLocal->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );
		 }

		 auto GetBoneIndex = [this] ( int hitbox ) {
			auto model = movementData->m_pLocal->GetModel( );
			if ( !model )
			   return -1;
			auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
			if ( !hdr )
			   return -1;
			auto set = hdr->pHitboxSet( movementData->m_pLocal->m_nHitboxSet( ) );
			return set->pHitbox( hitbox )->bone;
		 };

		 auto bone_idx = GetBoneIndex( HITBOX_HEAD );

		 HeadBone = movementData->m_pLocal->m_CachedBoneData( ).Element( bone_idx );

		 std::memcpy( movementData->m_pLocal->m_AnimOverlay( ).Base( ), layers, sizeof( layers ) );
		 std::memcpy( movementData->m_pLocal->m_flPoseParameter( ), poses, sizeof( poses ) );
		 *movementData->m_pLocal->m_PlayerAnimState( ) = animStateBackup;
	  }

	  // Source::FakeLag::Get( )->Main( movementData->m_pSendPacket, movementData->m_pCmd );


	  if ( g_TickbaseController.Using( ) ) {
		  bool bShouldLag = g_TickbaseController.s_nExtraProcessingTicks > 1;
		  *movementData->m_pSendPacket = !bShouldLag;
	  } else if ( !g_TickbaseController.Building( ) && g_Vars.fakelag.enabled ) {
		  Source::FakeLag::Get( )->Main( movementData->m_pSendPacket, movementData->m_pCmd );
	  }


	 // if ( Source::m_pClientState->m_nChokedCommands( ) >= TickbaseShiftCtx.lag_limit ) {
	//	 *movementData->m_pSendPacket = true;
	 // }
#if 0
	  if ( TickbaseShiftCtx.in_rapid )
		 *movementData->m_pSendPacket = 0;
	  if ( TickbaseShiftCtx.was_in_rapid )
		 *movementData->m_pSendPacket = 1;
#endif
	  bool success = Source::Ragebot::Get( )->Run( movementData->m_pCmd, movementData->m_pLocal, movementData->m_pSendPacket );

	  if ( !g_Vars.rage.enabled )
		 Source::LegitBot::Get( )->Main( movementData->m_pCmd.Xor( ) );

	  Source::AntiAimbot::Get( )->Main( movementData->m_pSendPacket, movementData->m_pCmd, success );
	  Source::KnifeBot::Get( )->Main( movementData->m_pCmd, movementData->m_pSendPacket );
	  SpoofImpacts( );
	  g_Vars.globals.m_bInCreateMove = false;
   }

   void C_Movement::PostPrediction( ) {
	  if ( !movementData->m_pLocal )
		 return;

	  movementData->m_pCmd->forwardmove = Math::Clamp( movementData->m_pCmd->forwardmove, -g_Vars.cl_forwardspeed->GetFloat( ), g_Vars.cl_forwardspeed->GetFloat( ) );
	  movementData->m_pCmd->sidemove = Math::Clamp( movementData->m_pCmd->sidemove, -g_Vars.cl_sidespeed->GetFloat( ), g_Vars.cl_sidespeed->GetFloat( ) );
	  movementData->m_pCmd->upmove = Math::Clamp( movementData->m_pCmd->upmove, -g_Vars.cl_upspeed->GetFloat( ), g_Vars.cl_upspeed->GetFloat( ) );
	  movementData->m_pCmd->viewangles.Normalize( );
	  movementData->m_pCmd->viewangles.Clamp( );

	  // later
   #if 0
	  if ( g_GameRules->m_bIsValveDS( ) ) {
	  }
   #endif

	  float delta_x = std::remainderf( movementData->m_pCmd->viewangles.x - movementData->m_angPreviousAngles.x, 360.0f );
	  float delta_y = std::remainderf( movementData->m_pCmd->viewangles.y - movementData->m_angPreviousAngles.y, 360.0f );

	  if ( delta_x != 0.0f ) {
		 float mouse_y = -( ( delta_x / g_Vars.m_pitch->GetFloat( ) ) / g_Vars.sensitivity->GetFloat( ) );
		 short mousedy;
		 if ( mouse_y <= 32767.0f ) {
			if ( mouse_y >= -32768.0f ) {
			   if ( mouse_y >= 1.0f || mouse_y < 0.0f ) {
				  if ( mouse_y <= -1.0f || mouse_y > 0.0f )
					 mousedy = static_cast< short >( mouse_y );
				  else
					 mousedy = -1;
			   } else {
				  mousedy = 1;
			   }
			} else {
			   mousedy = 0x8000u;
			}
		 } else {
			mousedy = 0x7FFF;
		 }

		 movementData->m_pCmd->mousedy = mousedy;
	  }

	  if ( delta_y != 0.0f ) {
		 float mouse_x = -( ( delta_y / g_Vars.m_yaw->GetFloat( ) ) / g_Vars.sensitivity->GetFloat( ) );
		 short mousedx;
		 if ( mouse_x <= 32767.0f ) {
			if ( mouse_x >= -32768.0f ) {
			   if ( mouse_x >= 1.0f || mouse_x < 0.0f ) {
				  if ( mouse_x <= -1.0f || mouse_x > 0.0f )
					 mousedx = static_cast< short >( mouse_x );
				  else
					 mousedx = -1;
			   } else {
				  mousedx = 1;
			   }
			} else {
			   mousedx = 0x8000u;
			}
		 } else {
			mousedx = 0x7FFF;
		 }

		 movementData->m_pCmd->mousedx = mousedx;
	  }

	  RotateMovement( movementData->m_pCmd, movementData->m_angRenderViewangles, movementData->m_pCmd->viewangles );

	  movementData->m_pLocal->SetAbsVelocity( movementData->m_pLocal->m_vecVelocity( ) );

	  FixLocalPlayerAnimations( *movementData->m_pSendPacket );
	  if ( *movementData->m_pSendPacket )
		 this->HandleVisualAnimation( );

	  movementData->m_angPreviousAngles = movementData->m_pCmd->viewangles;
   }

   bool C_Movement::AutoStopInternal( ) {
	  float maxSpeed = GetMaxSpeed( );
	  Vector velocity = Engine::Prediction::Instance( )->GetVelocity( );
	  velocity.z = 0.0f;

	  float speed = velocity.Length2D( );
	  if ( speed <= 2.0f && ( movementData->m_pCmd->forwardmove == 0.f && movementData->m_pCmd->sidemove == 0.f ) )
		 return true;

	  bool fullstop = true;
	  if ( movementData->m_bMinimalSpeed && ( movementData->m_pCmd->forwardmove != 0.f || movementData->m_pCmd->sidemove != 0.f ) && speed <= maxSpeed * 0.34f ) {
		 fullstop = false;
	  }

	  if ( fullstop && speed <= 2.0f && ( movementData->m_pCmd->forwardmove != 0.f || movementData->m_pCmd->sidemove != 0.f ) ) {
		 movementData->m_pCmd->forwardmove = 0.f;
		 movementData->m_pCmd->sidemove = 0.f;
		 return true;
	  }

	  if ( fullstop ) {
		 static int playerSurfaceFrictionOffset = Horizon::Memory::FindInDataMap( movementData->m_pLocal->GetPredDescMap( ), XorStr( "m_surfaceFriction" ) );
		 float playerSurfaceFriction = *( float* ) ( uintptr_t( movementData->m_pLocal ) + playerSurfaceFrictionOffset );
		 float max_accelspeed = g_Vars.sv_accelerate->GetFloat( ) * Source::m_pGlobalVars->interval_per_tick * maxSpeed * playerSurfaceFriction;
		 if ( speed - max_accelspeed <= -1.f ) {
			movementData->m_bStopPlayer = 0;
			movementData->m_pCmd->forwardmove = speed / max_accelspeed;
		 } else {
			movementData->m_pCmd->forwardmove = g_Vars.cl_forwardspeed->GetFloat( );
		 }

		 movementData->m_pCmd->sidemove = 0.0f;

		 QAngle move_dir = movementData->m_angMovementAngle;

		 float direction = atan2( velocity.y, velocity.x );
		 move_dir.yaw = std::remainderf( ToDegrees( direction ) + 180.0f, 360.0f );
		 RotateMovement( movementData->m_pCmd, move_dir, movementData->m_pCmd->viewangles );

		 return true;
	  } 

	  // hardcoded value, much stable than 0.33
	  float slow_walk_speed = 0.30f;
	  if ( g_Vars.misc.active && g_Vars.misc.slow_walk && g_Vars.misc.slow_walk_bind.enabled ) {
		 slow_walk_speed = std::fminf( g_Vars.misc.slow_walk_speed * 0.01f, slow_walk_speed );

		 SlowWalk( slow_walk_speed );
	  }  

	  if ( !fullstop && movementData->m_bMinimalSpeed )
		 SlowWalk( slow_walk_speed );

	/* auto weapon = ( C_WeaponCSBaseGun* ) movementData->m_pLocal->m_hActiveWeapon( ).Get( );

	  if ( !weapon )
		 return false;

	  auto weapon_data = weapon->GetCSWeaponData( ).Xor( );

	  if ( !weapon_data )
		 return false;

	  float max_speed = 0.f;
	  if ( movementData->m_pLocal->m_bIsScoped( ) ) {
		 max_speed = weapon_data->m_flMaxSpeed;
	  } else {
		 max_speed = weapon_data->m_flMaxSpeed2;
	  }
	  float sidemove = 0.f;
	  float forwardmove = 0.f;
	  float v168 = max_speed * 0.33000001;
	  float unpredictedVel = Engine::Prediction::Instance( )->GetVelocity( ).Length2DSquared( );
	  if ( unpredictedVel < v168 + 1.0f ) {
		 sidemove = movementData->m_pCmd->sidemove;
		 forwardmove = movementData->m_pCmd->forwardmove;
		 float v90 = ( sidemove * sidemove ) + ( forwardmove * forwardmove );
		 if ( ( ( movementData->m_pCmd->upmove * movementData->m_pCmd->upmove ) + v90 ) > 0.0f ) {
			movementData->m_pCmd->buttons |= IN_SPEED;
			if ( unpredictedVel <= 0.1f ) {
			   sidemove = movementData->m_pCmd->sidemove * v168;
			   forwardmove = movementData->m_pCmd->forwardmove * v168;
			} else {
			   float v91 = sqrtf( v90 );
			   sidemove = ( movementData->m_pCmd->sidemove / v91 ) * v168;
			   forwardmove = ( movementData->m_pCmd->forwardmove / v91 ) * v168;
			}
		 }
	  } else {
		 Vector v247 = Vector::Zero;
		 auto v266 = Engine::Prediction::Instance( )->GetVelocity( ) * -1.0f;
		 v247.x = RAD2DEG( std::atan2( v266.z * -1.f, sqrtf( ( v266.x * v266.x ) + ( v266.y * v266.y ) ) ) );
		 v247.y = RAD2DEG( std::atan2( v266.y, v266.x ) );
		 auto v100 = movementData->m_pCmd->viewangles.y - v247.y;
		 auto v101 = v100 * 0.017453292f;
		 auto v103 = cos( v101 );
		 auto v105 = sin( v101 );
		 auto v107 = ( v247.x * 0.017453292f );
		 v107 = sin( v107 );
		 sidemove = ( v107 * v103 ) * 450.f;
		 forwardmove = ( v107 * v105 ) * 450.f;
	  }

	  float v92 = fminf( 450.0, forwardmove );
	  if ( v92 <= -450.0 )
		 movementData->m_pCmd->forwardmove = -450.0;
	  else
		 movementData->m_pCmd->forwardmove = v92;
	  float v93 = fminf( 450.0, sidemove );
	  if ( v93 <= -450.0 )
		 movementData->m_pCmd->sidemove = -450.0;
	  else
		 movementData->m_pCmd->sidemove = v93;*/

	  return true;
   }

   void C_Movement::AutoJump( ) {
	  if ( movementData->m_pLocal->m_MoveType( ) == MOVETYPE_LADDER )
		 return;

	  if ( g_Vars.rage.enabled ) {
		 if ( !( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) ) {
			movementData->m_pCmd->buttons &= ~IN_JUMP;
		 }
		 return;
	  }

	  if ( !movementData->jumped_last_tick && movementData->should_fake_jump ) {
		 movementData->should_fake_jump = false;
		 movementData->m_pCmd->buttons |= IN_JUMP;
	  } else if ( movementData->m_pCmd->buttons & IN_JUMP ) {
		 if ( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) {
			movementData->jumped_last_tick = true;
			movementData->should_fake_jump = true;
		 } else {
			movementData->m_pCmd->buttons &= ~IN_JUMP;
			movementData->jumped_last_tick = false;
		 }
	  } else {
		 movementData->jumped_last_tick = false;
		 movementData->should_fake_jump = false;
	  }
   }

   void C_Movement::AutoStrafe( ) {
	  if ( movementData->m_pCmd->buttons & IN_SPEED )
		 return;

	  if ( ( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) && !( movementData->m_pCmd->buttons & IN_JUMP ) ) {
		 return;
	  }

	  if ( movementData->m_pLocal->m_MoveType( ) != MOVETYPE_WALK )
		 return;

	  static auto side = 1.0f;
	  side = -side;

	  auto velocity = movementData->m_pLocal->m_vecVelocity( );
	  velocity.z = 0.0f;

	  auto speed = velocity.Length2D( );
	  auto ideal_strafe = Math::Clamp( ToDegrees( atan( 15.f / speed ) ), 0.0f, 90.0f );

	  if ( g_Vars.misc.autostrafer_wasd && ( movementData->m_pCmd->forwardmove != 0.0f || movementData->m_pCmd->sidemove != 0.0f ) ) {
		 float move_dir = ToDegrees( atan2( -movementData->m_pCmd->sidemove, movementData->m_pCmd->forwardmove ) );
		 movementData->m_angMovementAngle.yaw = std::remainderf( movementData->m_angMovementAngle.yaw + move_dir, 360.0f );
		 movementData->m_pCmd->forwardmove = movementData->m_pCmd->sidemove = 0.0f;
	  } else if ( movementData->m_pCmd->forwardmove > 0.0f ) {
		 movementData->m_pCmd->forwardmove = 0.0f;
	  }

	  auto yaw_delta = std::remainderf( movementData->m_angMovementAngle.yaw - movementData->m_flOldYaw, 360.0f );
	  auto abs_angle_delta = abs( yaw_delta );
	  movementData->m_flOldYaw = movementData->m_angMovementAngle.yaw;

	  if ( abs_angle_delta <= ideal_strafe || abs_angle_delta >= 30.0f ) {
		 auto velocity_direction = velocity.ToEulerAngles( );
		 auto velocity_delta = std::remainderf( movementData->m_angMovementAngle.yaw - velocity_direction.yaw, 360.0f );
		 auto retrack = Math::Clamp( ToDegrees( atan( 30.0f / speed ) ), 0.0f, 90.0f ) * g_Vars.misc.autostrafer_retrack;
		 if ( velocity_delta <= retrack || speed <= 15.0f ) {
			if ( -( retrack ) <= velocity_delta || speed <= 15.0f ) {
			   movementData->m_angMovementAngle.yaw += side * ideal_strafe;
			   movementData->m_pCmd->sidemove = g_Vars.cl_sidespeed->GetFloat( ) * side;
			} else {
			   movementData->m_angMovementAngle.yaw = velocity_direction.yaw - retrack;
			   movementData->m_pCmd->sidemove = g_Vars.cl_sidespeed->GetFloat( );
			}
		 } else {
			movementData->m_angMovementAngle.yaw = velocity_direction.yaw + retrack;
			movementData->m_pCmd->sidemove = -g_Vars.cl_sidespeed->GetFloat( );
		 }
	  } else if ( yaw_delta > 0.0f ) {
		 movementData->m_pCmd->sidemove = -g_Vars.cl_sidespeed->GetFloat( );
	  } else if ( yaw_delta < 0.0f ) {
		 movementData->m_pCmd->sidemove = g_Vars.cl_sidespeed->GetFloat( );
	  }

	  movementData->m_angMovementAngle.Normalize( );
   }

   void C_Movement::FakeDuck( ) {
	  // fatality fakeduck
   #if 1
	  if ( g_Vars.misc.fakeduck && g_Vars.misc.fakeduck_bind.enabled ) {
		 if ( movementData->m_bFakeducking ) {
			if ( Source::m_pClientState->m_nChokedCommands( ) <= 6 )
			   movementData->m_pCmd->buttons &= ~IN_DUCK;
			else
			   movementData->m_pCmd->buttons |= IN_DUCK;

			*movementData->m_pSendPacket = Source::m_pClientState->m_nChokedCommands( ) > 13;
			g_Vars.globals.FakeDuckWillChoke = 14 - Source::m_pClientState->m_nChokedCommands( );
			if ( g_Vars.globals.FakeDuckWillChoke < 0 )
			   g_Vars.globals.FakeDuckWillChoke = 0;
		 } else {
			g_Vars.globals.FakeDuckWillChoke = 6 - Source::m_pClientState->m_nChokedCommands( );
			if ( g_Vars.globals.FakeDuckWillChoke < 0 )
			   g_Vars.globals.FakeDuckWillChoke = 0;

			*movementData->m_pSendPacket = Source::m_pClientState->m_nChokedCommands( ) > 5;
			if ( *movementData->m_pSendPacket )
			   movementData->m_bFakeducking = true;
			movementData->m_pCmd->buttons |= IN_DUCK;
		 }
	  } else {
		 if ( movementData->m_bFakeducking ) {
			*movementData->m_pSendPacket = false;
		 }
		 movementData->m_bFakeducking = false;
	  }
	  if ( g_Vars.misc.fakeduck && g_Vars.misc.fakeduck_bind.enabled ) {
		 movementData->m_pCmd->buttons |= IN_BULLRUSH;

		 int maxChoke = std::min( 14, std::min( 16, g_Vars.fakelag.lag_limit ) );

		 int chokeGoal = maxChoke / 2;
		 bool shouldCrouch = Source::m_pClientState->m_nChokedCommands( ) >= chokeGoal;

		 if ( shouldCrouch )
			movementData->m_pCmd->buttons |= IN_DUCK;
		 else
			movementData->m_pCmd->buttons &= ~IN_DUCK;

		 *movementData->m_pSendPacket = Source::m_pClientState->m_nChokedCommands( ) >= maxChoke;

		 g_Vars.globals.FakeDuckWillChoke = maxChoke - Source::m_pClientState->m_nChokedCommands( );
}
   #endif

/*	  g_Vars.globals.FakeDuckWillChoke = 0;

	  // onetap.su fakeduck
	  auto ticks = g_Vars.globals.m_iServerType < 8 ? 6 : 14;
	  auto previous_state = g_Vars.globals.Fakeducking;
	  g_Vars.globals.Fakeducking = movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND
		 && g_Vars.misc.fakeduck
		 && g_Vars.misc.fakeduck_bind.enabled
		 && !TickbaseShiftCtx.over_choke_nr
		 && !TickbaseShiftCtx.exploits_enabled
		 && int( 1.0f / Source::m_pGlobalVars->interval_per_tick ) == 64;

	  if ( !previous_state ) {
		 if ( !g_Vars.globals.Fakeducking )
			return;

		 if ( Source::m_pClientState->m_nChokedCommands( ) == 0 ) {
			movementData->m_pCmd->buttons |= IN_BULLRUSH;
			movementData->m_pCmd->buttons &= ~IN_DUCK;
		 } else {
			g_Vars.globals.Fakeducking = false;
		 }

		 return;
	  }
	  if ( !( movementData->m_pCmd->buttons & IN_BULLRUSH ) )
		 movementData->m_pCmd->buttons | IN_BULLRUSH;

	  if ( Source::m_pClientState->m_nChokedCommands( ) <= 6u ) {
		 movementData->m_pCmd->buttons &= ~IN_DUCK;
	  } else {
		 movementData->m_pCmd->buttons |= IN_DUCK;
	  }

	  g_Vars.globals.FakeDuckWillChoke = Source::m_pGlobalVars->tickcount - Source::m_pClientState->m_nChokedCommands( ) + ticks;

	  if ( Source::m_pClientState->m_nChokedCommands( ) < 14u ) {
		 *movementData->m_pSendPacket = false;              // choke
	  } else {
		 *movementData->m_pSendPacket = true;             // send packet
	  }

	  /* auto goal_duck_amount = ( movementData->m_pCmd->buttons & IN_DUCK ) ? 1.0f : 0.0f;
	   if ( g_Vars.globals.Fakeducking || goal_duck_amount == movementData->m_pLocal->m_flDuckAmount( ) ) {
		  movementData->m_pCmd->buttons |= IN_BULLRUSH;
		  if ( Source::m_pClientState->m_nChokedCommands( ) < ticks >> 1 ) {
			 movementData->m_pCmd->buttons &= ~IN_DUCK;
		  } else {
			 movementData->m_pCmd->buttons |= IN_DUCK;
		  }

		  g_Vars.globals.FakeDuckWillChoke = ticks + Source::m_pGlobalVars->tickcount - Source::m_pClientState->m_nChokedCommands( );
		  if ( !TickbaseShiftCtx.in_rapid )
			 * movementData->m_pSendPacket = Source::m_pClientState->m_nChokedCommands( ) >= ticks;
	   } else {
		  movementData->m_pCmd->buttons |= IN_BULLRUSH;
		  if ( goal_duck_amount <= movementData->m_pLocal->m_flDuckAmount( ) )
			 movementData->m_pCmd->buttons &= ~IN_DUCK;
		  else
			 movementData->m_pCmd->buttons |= IN_DUCK;
	   }*/
   }

   void C_Movement::SlowWalk( float speed ) {
	  C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

	  if ( !LocalPlayer || LocalPlayer->IsDead( ) )
		 return;

	  // aimware slow walk
   #if 0
	  float maxSpeed = GetMaxSpeed( );
	  maxSpeed *= speed;

	  if ( movementData->m_pCmd->forwardmove != 0.f || movementData->m_pCmd->sidemove != 0.f ) {
		 float dir = atan2( movementData->m_pCmd->forwardmove, movementData->m_pCmd->sidemove );
		 float sinDir, sinCos;
		 DirectX::XMScalarSinCos( &sinDir, &sinCos, dir );

		 movementData->m_pCmd->forwardmove = sinDir * maxSpeed;
		 movementData->m_pCmd->sidemove = sinCos * maxSpeed;
   }
   #else
	  float maxSpeed = GetMaxSpeed( );
	  maxSpeed *= speed;

	  if ( speed <= 0.52f )
		 movementData->m_pCmd->buttons |= IN_SPEED;
	  else
		 movementData->m_pCmd->buttons &= ~IN_SPEED;

	  float movement_speed = sqrtf( movementData->m_pCmd->forwardmove * movementData->m_pCmd->forwardmove + movementData->m_pCmd->sidemove * movementData->m_pCmd->sidemove );
	  if ( movement_speed > maxSpeed ) {
		 float forward_ratio = movementData->m_pCmd->forwardmove / movement_speed;
		 float side_ratio = movementData->m_pCmd->sidemove / movement_speed;
		 float mov_speed = movementData->m_pLocal->m_vecVelocity( ).Length2D( );
		 if ( ( maxSpeed + 1.0f ) <= mov_speed ) {
			movementData->m_pCmd->forwardmove = 0.0f;
			movementData->m_pCmd->sidemove = 0.0f;
		 } else {
			movementData->m_pCmd->forwardmove = forward_ratio * maxSpeed;
			movementData->m_pCmd->sidemove = side_ratio * maxSpeed;
		 }
	  }
   #endif
   }

   void C_Movement::ThirdPerson( ) {
	  if ( !movementData->m_pLocal )
		 return;

	  auto local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || local != movementData->m_pLocal )
		 return;

	  C_WeaponCSBaseGun* Weapon = ( C_WeaponCSBaseGun* ) movementData->m_pLocal->m_hActiveWeapon( ).Get( );

	  if ( !Weapon )
		 return;

	  auto weaponInfo = Weapon->GetCSWeaponData( );
	  if ( !weaponInfo.IsValid( ) )
		 return;

	  if ( weaponInfo->m_iWeaponType == WEAPONTYPE_GRENADE && g_Vars.misc.off_third_person_on_grenade ) {
		 Source::m_pInput->m_fCameraInThirdPerson = false;
		 return;
	  }

	  static auto require_reset = false;
	  static auto percent = 0.f;

	  static Vector vecCameraOffsetBackup = Vector( 0, 0, 0 );
	  if ( !Source::m_pInput->m_fCameraInThirdPerson )
		 vecCameraOffsetBackup = Source::m_pInput->m_vecCameraOffset;
	  else
		 Source::m_pInput->m_vecCameraOffset = vecCameraOffsetBackup;

	  if ( g_Vars.misc.third_person && g_Vars.misc.third_person_bind.enabled ) {
		 Source::m_pInput->m_fCameraInThirdPerson = true;

		 percent = Math::Clamp( percent + Source::m_pGlobalVars->frametime * 8.f, 0.f, 1.f );
		 float ideal_distance = g_Vars.misc.third_person_dist * percent;

		 QAngle view_angles;
		 Source::m_pEngine->GetViewAngles( view_angles );

		 QAngle inverse_angles;
		 Source::m_pEngine->GetViewAngles( inverse_angles );
		 inverse_angles.z = ideal_distance;

		 Vector camForward, camRight, camUp;
		 camForward = inverse_angles.ToVectors( &camRight, &camUp );

		 CTraceFilterWorldAndPropsOnly filter;
		 CGameTrace trace;
		 Ray_t ray;

		 Vector eyePosition = movementData->m_pLocal->GetAbsOrigin( ) + movementData->m_pLocal->m_vecViewOffset( );

		 Vector vecCamOffset = eyePosition + ( camForward * -ideal_distance ) + ( camRight * 1.f ) + ( camUp * 1.f );

		 ray.Init( eyePosition, vecCamOffset, Vector( -4.0f, -4.0f, -4.0f ), Vector( 4.0f, 4.0f, 4.0f ) );
		 Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

		 view_angles.roll = ideal_distance * trace.fraction;

		 Source::m_pInput->m_vecCameraOffset = Vector( view_angles.pitch, view_angles.yaw, view_angles.roll );

		 movementData->m_pLocal->UpdateVisibilityAllEntities( );
	  } else {
		 percent = 0.f;
		 Source::m_pInput->m_fCameraInThirdPerson = false;
	  }

	  if ( require_reset )
		 movementData->m_pLocal->m_iObserverMode( ) = 5;

	  if ( movementData->m_pLocal->m_iObserverMode( ) == 4 ||
		   movementData->m_pLocal->m_iObserverMode( ) == 3 )
		 require_reset = true;

   }

   void C_Movement::AutoStop( int ticks ) {
	  movementData->m_bStopPlayer = ticks;
   }

   bool C_Movement::GetStopState( ) {
	  return movementData->m_bStopOk;
   }

   void C_Movement::SpoofImpacts( ) {
	  // axyenna spasiba ikfakof za govnocode
	  // ya vse pofixil
	  /*static auto enabled = false;
	  static bool oldState = false;
	  if ( !g_Vars.misc.impacts_spoof ) {
		 if ( enabled ) {
			enabled = false;
			g_Vars.sv_show_impacts->SetValue( oldState );
		 }
		 return;
	  }

	  if ( !enabled )
		 oldState = g_Vars.sv_show_impacts->GetBool( );

	  enabled = true;
	  g_Vars.sv_show_impacts->SetValue( true );*/
   }

   bool C_Movement::CreateMoveRecursion( ) {
	  if ( !movementData->m_bInRecursion || movementData->m_iRecursionTicks <= 0 ) {
		 movementData->m_bInRecursion = false;
		 movementData->m_iRecursionTicks = 0;
		 return false;
	  }

	  movementData->m_iRecursionTicks--;
	  return true;
   }

   void C_Movement::FixLocalPlayerAnimations( bool is_outgoing ) {
	  movementData->m_pLocal->pl( ).v_angle = movementData->m_pCmd->viewangles;

	  auto animState = movementData->m_pLocal->m_PlayerAnimState( );
	  if ( animState->m_nLastFrame == Source::m_pGlobalVars->framecount )
		 animState->m_nLastFrame = Source::m_pGlobalVars->framecount - 1;

	  movementData->m_pLocal->m_iEFlags( ) &= ~( EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY );

	  animState->m_flFeetYawRate = 0.f;

	  movementData->m_pLocal->FixedAnimationsUpdate( );

	  /* desync */
	  if ( animState->m_velocity > 0.1f || std::fabsf( animState->flUpVelocity ) > 100.0f ) {
		 movementData->m_flLowerBodyUpdateTime = Source::m_pGlobalVars->curtime + 0.22f;
		 movementData->m_pLocal->m_flLowerBodyYawTarget( ) = animState->m_flEyeYaw;
	  } else if ( Source::m_pGlobalVars->curtime > movementData->m_flLowerBodyUpdateTime ) {
		 if ( std::fabsf( Math::AngleDiff( animState->m_flAbsRotation, animState->m_flEyeYaw ) ) > 35.0f ) {
			movementData->m_flLowerBodyUpdateTime = Source::m_pGlobalVars->curtime + 1.1f;
			movementData->m_pLocal->m_flLowerBodyYawTarget( ) = animState->m_flEyeYaw;
		 }
	  }

	  Engine::C_SetupBones boneSetup;

	  g_Vars.globals.m_flLbyUpdateTime = movementData->m_flLowerBodyUpdateTime;
	  auto flWeight12Backup = movementData->m_pLocal->m_AnimOverlay( ).Element( 12 ).m_flWeight;
	  auto flWeight3Backup = movementData->m_pLocal->m_AnimOverlay( ).Element( 3 ).m_flWeight;
	  auto flCycle3Backup = movementData->m_pLocal->m_AnimOverlay( ).Element( 3 ).m_flCycle;

	  movementData->m_pLocal->m_AnimOverlay( ).Element( 12 ).m_flWeight = 0.f;
	  movementData->m_pLocal->m_AnimOverlay( ).Element( 3 ).m_flWeight = 0.f;
	  movementData->m_pLocal->m_AnimOverlay( ).Element( 3 ).m_flCycle = 0.f;

	  boneSetup.Init( movementData->m_pLocal, is_outgoing ? ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_BONE_MERGE ) : BONE_USED_BY_ANYTHING, movementData->m_pLocal->m_CachedBoneData( ).Base( ) );
	  boneSetup.m_bShouldAttachment = false;

	  if ( is_outgoing ) {
		 boneSetup.Setup( );

		 if ( animState )
			g_Vars.globals.flRealYaw = animState->m_flAbsRotation;
		 PreviousYaw = g_Vars.globals.angViewangles.yaw;
		 g_Vars.globals.angViewangles = movementData->m_pCmd->viewangles;

		 // currently always, TODO: call ShouldDrawLocalPlayer
		 {
			// copy real bone positions
			auto boneCount = movementData->m_pLocal->m_CachedBoneData( ).Count( );
			std::memcpy( g_Vars.globals.m_RealBonesPositions, movementData->m_pLocal->m_vecBonePos( ), boneCount * sizeof( Vector ) );
			std::memcpy( g_Vars.globals.m_RealBonesRotations, movementData->m_pLocal->m_quatBoneRot( ), boneCount * sizeof( Quaternion ) );
		 }
	  }

	  movementData->m_pLocal->m_AnimOverlay( ).Element( 12 ).m_flWeight = flWeight12Backup;
	  movementData->m_pLocal->m_AnimOverlay( ).Element( 3 ).m_flWeight = flWeight3Backup;
	  movementData->m_pLocal->m_AnimOverlay( ).Element( 3 ).m_flCycle = flCycle3Backup;

	  if ( movementData->m_pLocal->m_CachedBoneData( ).Base( ) != movementData->m_pLocal->m_BoneAccessor( ).m_pBones ) {
		 std::memcpy( movementData->m_pLocal->m_BoneAccessor( ).m_pBones, movementData->m_pLocal->m_CachedBoneData( ).Base( ), movementData->m_pLocal->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );
	  }
   }

   void C_Movement::HandleVisualAnimation( ) {
	  static CCSGOPlayerAnimState m_FakeAnimState;
	  static float m_flFakeSpawnTime = 0.0f;

	  if ( !movementData.IsValid( ) ||
		   !movementData->m_pLocal ||
		   movementData->m_pLocal->IsDead( ) || 
		   !movementData->m_pCmd.IsValid( ) ||
		   movementData->m_pCmd->viewangles.IsZero( ) ||
		   !Source::m_pEngine->IsInGame( ) ||
		   !Source::m_pEngine->IsConnected( ) )
		 return;

	  Encrypted_t<CVariables::GLOBAL> globals( &g_Vars.globals );

	  if ( !globals->m_bFakeInit ) {
		 using CreateAnimState_t = void( __thiscall* )( CCSGOPlayerAnimState*, C_CSPlayer* );
		 ( ( CreateAnimState_t ) Engine::Displacement.Function.m_uCreateAnimState )( &m_FakeAnimState, movementData->m_pLocal );

		 m_flFakeSpawnTime = movementData->m_pLocal->m_flSpawnTime( );
		 globals->m_bFakeInit = true;
	  } else if ( m_flFakeSpawnTime != movementData->m_pLocal->m_flSpawnTime( ) ) {
		 m_FakeAnimState.m_Player = movementData->m_pLocal;
		 m_FakeAnimState.Reset( );

		 m_flFakeSpawnTime = movementData->m_pLocal->m_flSpawnTime( );
	  }

	  globals->LagOrigin = movementData->m_pLocal->m_vecOrigin( );

	  if ( g_Vars.esp.chams_lag ) {
		 std::memcpy( globals->LagPosition, movementData->m_pLocal->m_CachedBoneData( ).Base( ), movementData->m_pLocal->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );
	  }

	  if ( g_Vars.esp.chams_ghost || g_Vars.rage.enabled && movementData->m_pLocal && !movementData->m_pLocal->IsDead( ) && (&m_FakeAnimState) != nullptr ) {
		 movementData->m_pLocal->m_iEFlags( ) &= ~( EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY );

		 float flCurrentTime = Source::m_pGlobalVars->curtime;
		 float flFrameTime = Source::m_pGlobalVars->frametime;

		 if ( m_FakeAnimState.m_nLastFrame == Source::m_pGlobalVars->framecount )
			m_FakeAnimState.m_nLastFrame = Source::m_pGlobalVars->framecount - 1;

		 // set correct time
		 Source::m_pGlobalVars->curtime = static_cast< float >( movementData->m_pLocal->m_nTickBase( ) )* Source::m_pGlobalVars->interval_per_tick;
		 Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;

		 C_AnimationLayer layers_backup[ 15 ];
		 float pose_params_backup[ 24 ];
		 float weapon_pose_params_backup[ 24 ];

		 C_CSPlayer* weaponWorldModel = nullptr;

		 auto weapon = ( C_BaseAttributableItem* ) movementData->m_pLocal->m_hActiveWeapon( ).Get( );
		 if ( weapon )
			weaponWorldModel = ( C_CSPlayer* ) ( weapon )->m_hWeaponWorldModel( ).Get( );

		 // backup correct animation data
		 auto& anim_overlays = movementData->m_pLocal->m_AnimOverlay( );
		 std::memcpy( layers_backup, movementData->m_pLocal->m_AnimOverlay( ).Base( ), sizeof( layers_backup ) );
		 std::memcpy( pose_params_backup, movementData->m_pLocal->m_flPoseParameter( ), sizeof( pose_params_backup ) );
		 if ( weaponWorldModel )
			std::memcpy( weapon_pose_params_backup, weaponWorldModel->m_flPoseParameter( ), sizeof( weapon_pose_params_backup ) );

		 m_FakeAnimState.m_flFeetYawRate = 0.f;

		 // update fake animations
		 m_FakeAnimState.Update( movementData->m_pCmd->viewangles );

		 // fix approachangles too big when fakelagging
		 float eyeFeetDelta = std::remainderf( movementData->m_pCmd->viewangles.yaw - m_FakeAnimState.m_flAbsRotation, 360.0f );

		 float maxFraction = m_FakeAnimState.GetMaxFraction( );
		 float flMaxYawModifier = maxFraction * m_FakeAnimState.m_flMaxBodyYaw;
		 float flMinYawModifier = maxFraction * m_FakeAnimState.m_flMinBodyYaw;

		 if ( eyeFeetDelta <= flMaxYawModifier ) {
			if ( flMinYawModifier > eyeFeetDelta )
			   m_FakeAnimState.m_flAbsRotation = fabs( flMinYawModifier ) + movementData->m_pCmd->viewangles.yaw;
		 } else {
			m_FakeAnimState.m_flAbsRotation = movementData->m_pCmd->viewangles.yaw - ( fabs( flMaxYawModifier ) );
		 }

		 m_FakeAnimState.m_flAbsRotation = std::remainderf( m_FakeAnimState.m_flAbsRotation, 360.0f );

		 Math::ApproachAngle( movementData->m_pCmd->viewangles.yaw, m_FakeAnimState.m_flAbsRotation,
			( m_FakeAnimState.m_velocity > 0.1f || std::fabsf( m_FakeAnimState.flUpVelocity ) > 100.0f )
							  ? ( movementData->m_pLocal->m_PlayerAnimState( )->m_flGroundFraction * 20.0f + 30.0f )
							  : Source::m_pGlobalVars->interval_per_tick * 100.0f );

		 m_FakeAnimState.m_flAbsRotation = Math::AngleNormalize( std::remainderf( m_FakeAnimState.m_flAbsRotation, 360.0f ) );

		 globals->m_FakeAngles.yaw = m_FakeAnimState.m_flAbsRotation;

		 std::memcpy( FakeAnimLayers, movementData->m_pLocal->m_AnimOverlay( ).Base( ), sizeof( FakeAnimLayers ) );

		 // restore real rotation
		 movementData->m_pLocal->SetAbsAngles( QAngle( 0.0f, globals->flRealYaw, 0.0f ) );

		 // store fake poses
		 std::memcpy( globals->m_flFakePoseParams, movementData->m_pLocal->m_flPoseParameter( ), sizeof( globals->m_flFakePoseParams ) );
		 if ( weaponWorldModel )
			std::memcpy( globals->FakeWeaponPoses, weaponWorldModel->m_flPoseParameter( ), sizeof( globals->FakeWeaponPoses ) );

		 // restore correct animation data
		 std::memcpy( movementData->m_pLocal->m_AnimOverlay( ).Base( ), layers_backup, sizeof( layers_backup ) );
		 std::memcpy( movementData->m_pLocal->m_flPoseParameter( ), pose_params_backup, sizeof( pose_params_backup ) );
		 if ( weaponWorldModel )
			std::memcpy( weaponWorldModel->m_flPoseParameter( ), weapon_pose_params_backup, sizeof( weapon_pose_params_backup ) );

		 // restore time
		 Source::m_pGlobalVars->curtime = flCurrentTime;
		 Source::m_pGlobalVars->frametime = flFrameTime;
	  }

   #if 0
	  struct origin_record {
		 Vector origin;
		 Vector velocity;
		 Vector linear_velocity;
		 float time;
	  };

	  static std::deque<origin_record > records;
	  records.push_front( origin_record{ movementData->m_pLocal->m_vecOrigin( ), Vector( 0, 0, 0 ), Vector( 0, 0, 0 ),  Source::m_pGlobalVars->curtime } );

	  while ( records.size( ) > 64 ) {
		 records.pop_back( );
	  }

	  static bool switched = false;
	  if ( InputSys::Get( )->WasKeyPressed( VirtualKeys::H ) ) {
		 printf( XorStr( "switched!\n" ) );
		 switched = !switched;
	  }

	  if ( records.size( ) > 3 ) {
	  #if 0
		 float time_delta = records.at( 0 ).time - records.at( 1 ).time;
		 Vector origin_delta = records.at( 0 ).origin - records.at( 1 ).origin;
		 records.at( 0 ).velocity = origin_delta / time_delta;

		 auto cur_spd = records.at( 0 ).velocity.Length2D( );
		 auto prev_spd = records.at( 1 ).velocity.Length2D( );

		 auto speed_recalc = cur_spd + ( cur_spd - prev_spd );
		 if ( cur_spd <= 0.1f )
			speed_recalc = 0.0f;

		 auto max_speed = !( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) ? 320.0f : GetMaxSpeed( );
		 if ( speed_recalc > max_speed )
			speed_recalc = max_speed;

		 float ang_delta = ToDegrees( atan2( records.at( 1 ).origin.y - records.at( 2 ).origin.y, records.at( 1 ).origin.x - records.at( 2 ).origin.x ) );
		 float direction = ToDegrees( atan2( origin_delta.y, origin_delta.x ) );
		 float average_direction = std::remainderf( direction - ang_delta, 360.0f );
		 average_direction = ToRadians( average_direction * 0.5f + direction );

		 float dir_cos, dir_sin;
		 DirectX::XMScalarSinCos( &dir_sin, &dir_cos, average_direction );

		 records.at( 0 ).velocity.x = dir_cos * speed_recalc;
		 records.at( 0 ).velocity.y = dir_sin * speed_recalc;
		 records.at( 0 ).velocity.z = ( origin_delta.z ) / time_delta;

		 if ( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND )
			records.at( 0 ).velocity.z -= g_Vars.sv_gravity->GetFloat( ) * 0.5f;
	  #else
		 auto prev_record = &records.at( 1 );
		 auto penultimate_record = &records.at( 2 );

		 auto origin_delta = records.at( 0 ).origin - records.at( 1 ).origin;
		 auto time_delta = records.at( 0 ).time - records.at( 1 ).time;
		 if ( !origin_delta.IsZero( ) ) {
			auto velocity = origin_delta / time_delta;
			records.at( 0 ).linear_velocity = velocity;

			auto prev_origin_delta = records.at( 1 ).origin - records.at( 2 ).origin;
			if ( origin_delta != prev_origin_delta ) {
			   auto curr_direction = ToDegrees( atan2( origin_delta.y, origin_delta.x ) );

			   auto  cur_spd = velocity.Length2D( );
			   auto prev_spd = prev_record->velocity.Length2D( );

			#if 0
			   auto max_weapon_speed = GetMaxSpeed( );

			   auto max_accelspeed = g_Vars.sv_accelerate->GetFloat( ) * Source::m_pGlobalVars->interval_per_tick * max_weapon_speed;

			   // calculate average speed
			   auto speed_recalc = cur_spd + ( cur_spd - prev_spd );

			   // dont recalculate speed if not moving
			   if ( cur_spd <= 0.1f )
				  speed_recalc = 0.0f;

			   // clamp speed
			   auto max_speed = !( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) ? 320.0f : max_weapon_speed;
			   if ( speed_recalc > max_speed )
				  speed_recalc = max_speed;
			#else
			   auto speed_recalc = velocity.Length2D( );
			#endif

			   // calculate average direction
			   auto average_direction = 0.0f;
			   if ( !prev_origin_delta.IsZero( ) ) {
				  auto prev_direction = ToDegrees( atan2( prev_origin_delta.y, prev_origin_delta.x ) );
				  average_direction = std::remainderf( curr_direction - prev_direction, 360.0f );
				  average_direction = ToRadians( std::remainderf( curr_direction + average_direction * 0.5f, 360.0f ) );
			   } else {
				  // started moving
				  average_direction = ToRadians( curr_direction );
			   }

			   // calculate new velocity using average values
			   float dir_sin, dir_cos;
			   DirectX::XMScalarSinCos( &dir_sin, &dir_cos, average_direction );

			   velocity.x = dir_cos * speed_recalc;
			   velocity.y = dir_sin * speed_recalc;
			   velocity.z = ( origin_delta.z ) / time_delta;
			}

			// fix CGameMovement::FinishGravity
			if ( !( movementData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) )
			   velocity.z -= g_Vars.sv_gravity->GetFloat( ) * time_delta * 0.5f;

			records.at( 0 ).velocity = velocity;
		 }

	  #endif
		 static std::vector<float> delta_calc;
		 static std::vector<float> delta_linear;

		 static bool enabled = false;
		 if ( switched ) {
			float yaw_real = ToDegrees( std::atan2f( movementData->m_pLocal->m_vecVelocity( ).y, movementData->m_pLocal->m_vecVelocity( ).x ) );
			float yaw_calc = ToDegrees( std::atan2f( records.at( 0 ).velocity.y, records.at( 0 ).velocity.x ) );
			float yaw_linear = ToDegrees( std::atan2f( records.at( 0 ).linear_velocity.y, records.at( 0 ).linear_velocity.x ) );

			delta_calc.push_back( std::remainderf( yaw_real - yaw_calc, 360.0f ) );
			delta_linear.push_back( std::remainderf( yaw_real - yaw_linear, 360.0f ) );

			printf( XorStr( "real %.1f | calc %.1f | linear %.1f | %.1f | %.1f | ticks: %d\n" ),
					yaw_real, yaw_calc, yaw_linear,
					std::remainderf( yaw_real - yaw_calc, 360.0f ),
					std::remainderf( yaw_real - yaw_linear, 360.0f ),
					TIME_TO_TICKS( time_delta ) );

		 #if 0
			printf( XorStr( "diff: %.1f | linear diff: %.1f | real: %.1f | calc: %.1f | pre calc: %.1f | origin | X %.1f | Y %.1f | ticks: %d\n" ),
					movementData->m_pLocal->m_vecVelocity( ).Length2D( ) - records.at( 0 ).velocity.Length2D( ),
					movementData->m_pLocal->m_vecVelocity( ).Length2D( ) - records.at( 0 ).linear_velocity.Length2D( ),
					movementData->m_pLocal->m_vecVelocity( ).Length2D( ),
					records.at( 0 ).velocity.Length2D( ),
					cur_spd,
					origin_delta.x,
					origin_delta.y,
					TIME_TO_TICKS( time_delta ) );

			float yaw_real = ToDegrees( std::atan2f( movementData->m_pLocal->m_vecVelocity( ).y, movementData->m_pLocal->m_vecVelocity( ).x ) );
			float yaw_calc = ToDegrees( average_direction );

			delta_calc.push_back( std::remainderf( yaw_real - yaw_calc, 360.0f ) );
			delta_before.push_back( std::remainderf( yaw_real - curr_direction, 360.0f ) );

			printf( XorStr( "diff speed: %.1f | spd: %.1f | calc spd: %.1f | dyaw: %.1f | ryaw: %.1f | cyaw: %.1f | nyaw: %.1f | ticks: %d\n" ),
					movementData->m_pLocal->m_vecVelocity( ).Length2D( ) - records.at( 0 ).velocity.Length2D( ),
					movementData->m_pLocal->m_vecVelocity( ).Length2D( ),
					records.at( 0 ).velocity.Length2D( ),
					std::remainderf( yaw_real - yaw_calc, 360.0f ),
					yaw_real,
					yaw_calc,
					curr_direction,
					TIME_TO_TICKS( time_delta ) );
		 #endif

			enabled = true;
		 } else if ( enabled ) {
		 #if 1
			if ( delta_calc.size( ) > 0 ) {
			   auto accum_cacl = std::accumulate( delta_calc.begin( ), delta_calc.end( ), 0.0f ) / delta_calc.size( );
			   auto accum_linear = std::accumulate( delta_linear.begin( ), delta_linear.end( ), 0.0f ) / delta_linear.size( );
			   printf( XorStr( "linear: %.1f | calculated: %.1f\n" ), accum_linear, accum_cacl );
			}

			delta_calc.clear( );
			delta_linear.clear( );
		 #endif

			enabled = false;
			printf( XorStr( "=================================\n" ) );
		 }
	  }
   #endif
   }
}
