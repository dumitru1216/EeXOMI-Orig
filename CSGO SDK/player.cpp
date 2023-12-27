#include "player.hpp"
#include "displacement.hpp"
#include "source.hpp"
#include "weapon.hpp"
#include "TickbaseShift.hpp"

QAngle& C_BasePlayer::m_aimPunchAngle( ) {
   return *( QAngle* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_aimPunchAngle );
}

QAngle& C_BasePlayer::m_aimPunchAngleVel( ) {
   return *( QAngle* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_aimPunchAngleVel );
}

QAngle& C_BasePlayer::m_viewPunchAngle( ) {
   return *( QAngle* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_viewPunchAngle );
}

Vector& C_BasePlayer::m_vecViewOffset( ) {
   return *( Vector* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_vecViewOffset );
}
Vector& C_BasePlayer::m_vecVelocity( ) {
   return *( Vector* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_vecVelocity );
}

Vector& C_BasePlayer::m_vecBaseVelocity( ) {
   return *( Vector* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_vecBaseVelocity );
}

float& C_BasePlayer::m_flFallVelocity( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_flFallVelocity );
}

float& C_BasePlayer::m_flDuckAmount( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_flDuckAmount );
}

float& C_BasePlayer::m_flDuckSpeed( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_flDuckSpeed );
}

char& C_BasePlayer::m_lifeState( ) {
   return *( char* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_lifeState );
}

int& C_BasePlayer::m_nTickBase( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_nTickBase );
}

int& C_BasePlayer::m_nPredictedTickbase( ) {
	return *( int* )( ( uintptr_t )this + Engine::Displacement.DT_BasePlayer.m_nTickBase + 0x4u );
}

int& C_BasePlayer::m_iHealth( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_iHealth );
}

int& C_BasePlayer::m_fFlags( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_fFlags );
}

int& C_BasePlayer::m_iDefaultFOV( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_iDefaultFOV );
}

int& C_BasePlayer::m_iObserverMode( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_iObserverMode );
}

CPlayerState& C_BasePlayer::pl( ) {
   return *( CPlayerState* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.pl );
}

CBaseHandle& C_BasePlayer::m_hObserverTarget( ) {
   return *( CBaseHandle* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_hObserverTarget );
}

CHandle<C_BaseViewModel> C_BasePlayer::m_hViewModel( ) {
   return *( CHandle<C_BaseViewModel>* )( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_hViewModel );
}

int& C_BasePlayer::m_vphysicsCollisionState( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BasePlayer.m_vphysicsCollisionState );
}

float C_BasePlayer::SequenceDuration( CStudioHdr* pStudioHdr, int iSequence ) {
   //55 8B EC 53 57 8B 7D 08 8B D9 85 FF 75 double __userpurge SequenceDuration@<st0>(int a1@<ecx>, float a2@<xmm0>, int *a3, int a4)
   using SequenceDurationFn = float( __thiscall* )( void*, CStudioHdr*, int );
   return Memory::VCall< SequenceDurationFn >( this, 220 )( this, pStudioHdr, iSequence );
}

float C_BasePlayer::GetSequenceMoveDist( CStudioHdr* pStudioHdr, int iSequence ) {
   Vector vecReturn;

   auto rel_32_fix = [] ( uintptr_t ptr ) -> uintptr_t { // TODO: Move this to displacement
      auto offset = *( uintptr_t* ) ( ptr + 0x1 );
      return ( uintptr_t ) ( ptr + 5 + offset );
   };

   // int __usercall GetSequenceLinearMotion@<eax>(int a1@<edx>, _DWORD *a2@<ecx>, int a3, _DWORD *a4)
   // it fastcall, but edx and ecx swaped
   // xref: Bad pstudiohdr in GetSequenceLinearMotion()!\n | Bad sequence (%i out of %i max) in GetSequenceLinearMotion() for model '%s'!\n
   static uintptr_t ptr = rel_32_fix( Memory::Scan( XorStr( "client.dll" ), XorStr( "E8 ? ? ? ? F3 0F 10 4D ? 83 C4 08 F3 0F 10 45 ? F3 0F 59 C0" ) ) );


   using GetSequenceLinearMotionFn = int( __fastcall* )( CStudioHdr*, int, float*, Vector* );
   ( ( GetSequenceLinearMotionFn ) ptr )( pStudioHdr, iSequence, m_flPoseParameter( ), &vecReturn );
   __asm {
      add esp, 8
   }
   return vecReturn.Length( );
}

bool C_BasePlayer::IsDead( ) {
   return ( this->m_lifeState( ) );
}

void C_BasePlayer::SetCurrentCommand( CUserCmd* cmd ) {
   *( CUserCmd** ) ( ( uintptr_t ) this + Engine::Displacement.C_BasePlayer.m_pCurrentCommand ) = cmd;
}

Vector C_BasePlayer::GetEyePosition( ) {
   Vector vecOrigin = m_vecOrigin( );

   // ebany rot etogo game movementa
#if 0
   // rebuilded GameMovement SetDuckedEyePosition
   Vector vecMinDuckHull = Source::m_pGameMovement->GetPlayerMins( true );
   Vector vecMaxStandHull = Source::m_pGameMovement->GetPlayerMaxs( false );

   Vector vecDuckViewOffset = Source::m_pGameMovement->GetPlayerViewOffset( true );
   Vector vecStandViewOffset = Source::m_pGameMovement->GetPlayerViewOffset( false );

   float_t flDuckAmount = m_flDuckAmount( );
   float_t duckAmounta = ( ( flDuckAmount * flDuckAmount ) * 3.0f ) - ( ( ( flDuckAmount * flDuckAmount ) * 2.0f ) * flDuckAmount );

   Vector offset = GetViewHeight( );
   offset.z = ( ( vecDuckViewOffset.z - ( vecMinDuckHull.z - vecMaxStandHull.z ) ) * duckAmounta ) + ( ( 1.0f - duckAmounta ) * vecStandViewOffset.z );
   vecOrigin += offset;
#endif

   Vector offset = m_vecViewOffset( );
   if ( offset.z >= 46.1f ) {
	  if ( offset.z > 64.0f ) {
		 offset.z = 64.0f;
	  }
   } else {
	  offset.z = 46.0f;
   }
   vecOrigin += offset;

   return vecOrigin;
}

Vector C_BasePlayer::GetViewHeight( ) {
   Vector offset;
   if ( this->m_flDuckAmount( ) == 0.0f ) {
	  offset = Source::m_pGameMovement->GetPlayerViewOffset( false );
   } else {
	  offset = m_vecViewOffset( );
   }
   return offset;
}

C_CSPlayer* C_CSPlayer::GetLocalPlayer( ) {
   auto index = Source::m_pEngine->GetLocalPlayer( );

   if ( !index )
	  return nullptr;

   auto client = Source::m_pEntList->GetClientEntity( index );

   if ( !client )
	  return nullptr;

   return ToCSPlayer( client->GetBaseEntity( ) );
}

C_CSPlayer* C_CSPlayer::GetPlayerByIndex( int index ) {
   if ( !index )
	  return nullptr;

   auto client = Source::m_pEntList->GetClientEntity( index );

   if ( !client )
	  return nullptr;

   return ToCSPlayer( client->GetBaseEntity( ) );
}

CCSGOPlayerAnimState*& C_CSPlayer::m_PlayerAnimState( ) {
   return *( CCSGOPlayerAnimState** ) ( ( uintptr_t ) this + Engine::Displacement.C_CSPlayer.m_PlayerAnimState );
}

void C_CSPlayer::SetEyeAngles( QAngle Angle ) {
	static int m_angEyeAngles = Engine::Displacement.DT_CSPlayer.m_angEyeAngles;
	*reinterpret_cast< QAngle* >( uintptr_t( this ) + m_angEyeAngles ) = Angle;
}

QAngle& C_CSPlayer::m_angEyeAngles( ) {
   return *( QAngle* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_angEyeAngles );
}

int& C_CSPlayer::m_nSurvivalTeam( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_nSurvivalTeam );
}

int& C_CSPlayer::m_ArmorValue( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_ArmorValue );
}

int& C_CSPlayer::m_iAccount( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_iAccount );
}

int& C_CSPlayer::m_iShotsFired( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_iShotsFired );
}

float& C_CSPlayer::m_flFlashDuration( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_flFlashDuration );
}

float& C_CSPlayer::m_flSecondFlashAlpha( ) {
   return *( float* ) ( uintptr_t( this ) + Engine::Displacement.DT_CSPlayer.m_flFlashDuration - 0xC );
}

float& C_CSPlayer::m_flVelocityModifier( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_flVelocityModifier );
}

float& C_CSPlayer::m_flLowerBodyYawTarget( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_flLowerBodyYawTarget );
}

float& C_CSPlayer::m_flSpawnTime( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.C_CSPlayer.m_flSpawnTime );
}

float& C_CSPlayer::m_flHealthShotBoostExpirationTime( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_flHealthShotBoostExpirationTime );
}

bool& C_CSPlayer::m_bHasHelmet( ) {
   return *( bool* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_bHasHelmet );
}

bool& C_CSPlayer::m_bHasHeavyArmor( ) {
   return *( bool* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_bHasHeavyArmor );
}

bool& C_CSPlayer::m_bIsScoped( ) {
   return *( bool* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_bScoped );
}

bool& C_CSPlayer::m_bWaitForNoAttack( ) {
   return *( bool* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_bWaitForNoAttack );
}

int& C_CSPlayer::m_iMatchStats_Kills( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_iMatchStats_Kills );
}

int& C_CSPlayer::m_iMatchStats_Deaths( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_iMatchStats_Deaths );
}

int& C_CSPlayer::m_iMatchStats_HeadShotKills( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_iMatchStats_HeadShotKills );
}

bool& C_CSPlayer::m_bGunGameImmunity( ) {
   return *( bool* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_bGunGameImmunity );
}

bool C_CSPlayer::PhysicsRunThink( int unk01 ) {
   static auto impl_PhysicsRunThink = ( bool( __thiscall* )( void*, int ) )Engine::Displacement.Function.m_uImplPhysicsRunThink;
   return impl_PhysicsRunThink( this, unk01 );
}

void C_CSPlayer::Think( ) {
	static auto index = *( int* )( Memory::Scan( XorStr( "client.dll" ), XorStr( "FF 90 ? ? ? ? FF 35 ? ? ? ? 8B 4C 24 3C" ) ) + 2 ) / 4; // ref: CPrediction::ProcessMovement  (*(void (__thiscall **)(_DWORD *))(*player + 552))(player);
	//xref CPrediction::ProcessMovement (*(void (__thiscall **)(_DWORD *))(*a2 + 552))(a2);
	using Fn = void( __thiscall* )( void* );
	Memory::VCall<Fn>( this, 137 )( this ); // 139
}

void C_CSPlayer::PreThink( ) {
	static auto index = *( int* )( Memory::Scan( XorStr( "client.dll" ), XorStr( "FF 90 ? ? ? ? 8B 86 ? ? ? ? 83 F8 FF" ) ) + 2 ) / 4;
	//xref CPrediction::ProcessMovement 
	// if ( (unsigned __int8)sub_102FED00(0) )
	// (*(void (__thiscall **)(_DWORD *))(*a2 + 1268))(a2);
	using Fn = void( __thiscall* )( void* );
	Memory::VCall<Fn>( this, 307 )( this ); // 169
}

void C_CSPlayer::PostThink( ) {
	using Fn = void( __thiscall* )( void* );
	Memory::VCall<Fn>( this, /*316*/ 308 )( this );
}

bool C_CSPlayer::IsTeammate( C_CSPlayer* player ) {
   if ( !player || !this )
	  return false;

   if ( g_Vars.game_type->GetInt( ) == 6 ) {
	  if ( m_nSurvivalTeam( ) >= 0 && m_nSurvivalTeam( ) == player->m_nSurvivalTeam( ) )
		 return true;
	  return false;
   }

   return this->m_iTeamNum( ) == player->m_iTeamNum( );
}

bool C_CSPlayer::CanShoot( int tickbase_shift, bool skip_revolver ) {
   if ( g_Vars.globals.WasShootingInChokeCycle || g_Vars.globals.WasShooting )
	  return false;

   if ( this->m_fFlags( ) & 0x40 )
	  return false;

   auto g_GameRules = *( uintptr_t** ) ( Engine::Displacement.Data.m_GameRules );
   if ( *( bool* ) ( *( uintptr_t* ) g_GameRules + 0x20 ) ) {
	  return false;
   }

   if ( this->m_bWaitForNoAttack( ) )
	  return false;

   if ( *( int* ) ( uintptr_t( this ) + Engine::Displacement.DT_CSPlayer.m_iPlayerState ) )
	  return false;
   
   if ( *( bool* ) ( ( uintptr_t ) this + Engine::Displacement.DT_CSPlayer.m_bIsDefusing ) )
	  return false;

   if ( this->IsReloading( ) )
	  return false;

   auto weapon = ( C_WeaponCSBaseGun* ) ( this->m_hActiveWeapon( ).Get( ) );
   if ( !weapon )
	  return false;

   auto weapon_data = weapon->GetCSWeaponData( );
   if ( !weapon_data.IsValid( ) )
	  return false;

   if ( weapon_data->m_iWeaponType >= WEAPONTYPE_PISTOL && weapon_data->m_iWeaponType <= WEAPONTYPE_MACHINEGUN && weapon->m_iClip1( ) < 1 )
	  return false;

   auto tickbase = this->m_nTickBase( );
   if ( tickbase_shift == 0 ) {
	  if ( g_tickbase_control.m_charged_ticks > 0 ) /* should be cor */
		 tickbase += -1 - g_tickbase_control.m_charged_ticks;
   } else {
	  tickbase -= tickbase_shift;
   }

   float curtime = TICKS_TO_TIME( tickbase );
   if ( curtime < m_flNextAttack( ) )
	  return false;
   
   if ( curtime < weapon->m_flNextPrimaryAttack( ) )
	  return false;

   if ( weapon->m_iItemDefinitionIndex( ) != WEAPON_REVOLVER )
	  return true;

   if ( skip_revolver )
	  return true;

   if ( *( int* ) ( uintptr_t( weapon ) + Engine::Displacement.DT_BaseAnimating.m_nSequence ) != 5 )
	  return false;

   return curtime >= weapon->m_flPostponeFireReadyTime( );
}

bool C_CSPlayer::IsReloading( ) {
   auto animLayer = this->m_AnimOverlay( ).Element( 1 );
   if ( !animLayer.m_pOwner )
	  return false;

   return GetSequenceActivity( animLayer.m_nSequence ) == 967 && animLayer.m_flWeight != 0.f;

#if 0
   static auto inReload = *( uint32_t* ) ( Memory::Scan( "client.dll", "C6 87 ? ? ? ? ? 8B 06 8B CE FF 90" ) + 2 );
   return *( bool* ) ( ( uintptr_t ) this + inReload );
#endif
}

Vector C_CSPlayer::GetEyePosition( ) {
   auto eyePosition = C_BasePlayer::GetEyePosition( );
   auto animState = this->m_PlayerAnimState( );

   // rebuild from server.dll august 2018
   if ( animState ) {
	  if ( animState->m_bHitground || animState->m_fDuckAmount != 0.0f || !( this->m_fFlags( ) & FL_ONGROUND ) ) {
		 // was lazy to sig LookupBone
		 auto GetBoneIndex = [this] ( int hitbox ) {
			auto model = this->GetModel( );
			if ( !model )
			   return -1;
			auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
			if ( !hdr )
			   return -1;
			auto set = hdr->pHitboxSet( this->m_nHitboxSet( ) );
			return set->pHitbox( hitbox )->bone;
		 };

		 int bone_idx = GetBoneIndex( HITBOX_HEAD );
		 if ( bone_idx != -1 ) {
			matrix3x4_t bonetoworld = this->m_CachedBoneData( ).Element( bone_idx );

			Vector head_position;
			head_position.x = bonetoworld.m[ 0 ][ 3 ];
			head_position.y = bonetoworld.m[ 1 ][ 3 ];
			head_position.z = bonetoworld.m[ 2 ][ 3 ];

			float headMax = head_position.z + 1.7f;
			// TODO: fix when eyePosition.z > headMax if antiaim enabled
		 #if 1
			if ( eyePosition.z > headMax ) {
			   // Hermite_Spline
			   float t = Math::Clamp( ( std::fabsf( eyePosition.z - headMax ) - 4.0f ) / 6.0f, 0.0f, 1.0f );
			   float tSqr = t * t;
			   float tCube = t * tSqr;
			   eyePosition.z += ( headMax - eyePosition.z ) * ( tSqr * 3.0f - tCube * 2.0f );
			}
		 #else
			if ( eyePosition.z > headMax ) {
			   float v9 = 0.0f;
			   float  v10 = ( std::fabsf( eyePosition.z - headMax ) - 4.0f ) / 6.0f;
			   if ( v10 >= 0.0 )
				  v9 = fminf( v10, 1.0 );
			   eyePosition.z += ( ( headMax - eyePosition.z ) * ( ( ( v9 * v9 ) * 3.0f ) - ( ( ( v9 * v9 ) * 2.0f ) * v9 ) ) );
			}
		 #endif
		 }
	  }
   }

   return eyePosition;
}

