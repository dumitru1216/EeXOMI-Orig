#pragma once

#include "entity.hpp"

#include "PlayerAnimState.hpp"
#include "CBaseHandle.hpp"

class C_BaseViewModel;

class CPlayerState {
public:
   virtual ~CPlayerState( ) { }
   bool deadflag;
   QAngle		v_angle; // Viewing angle (player only)

   // The client .dll only cares about deadflag
   // the game and engine .dlls need to worry about the rest of this data
   // Player's network name
   string_t	netname;

   // 0:nothing, 1:force view angles, 2:add avelocity
   int			fixangle;

   // delta angle for fixangle == FIXANGLE_RELATIVE
   QAngle		anglechange;

   // flag to single the HLTV/Replay fake client, not transmitted
   bool		hltv;
   bool		replay;
   int			frags;
   int			deaths;
};

class C_BasePlayer : public C_BaseCombatCharacter {
public:
   QAngle& m_aimPunchAngle( );
   QAngle& m_aimPunchAngleVel( );
   QAngle& m_viewPunchAngle( );
   Vector& m_vecViewOffset( );
   Vector& m_vecVelocity( );
   Vector& m_vecBaseVelocity( );
   float& m_flFallVelocity( );
   float& m_flDuckAmount( );
   float& m_flDuckSpeed( );
   char& m_lifeState( );
   int& m_nTickBase( );
   int& m_nPredictedTickbase( );
   int& m_iHealth( );
   int& m_fFlags( );
   int& m_iDefaultFOV( );
   int& m_iObserverMode( );

   CPlayerState& pl( );
   CBaseHandle& m_hObserverTarget( );
   CHandle<C_BaseViewModel> m_hViewModel( );
   int& m_vphysicsCollisionState( );
   float SequenceDuration( CStudioHdr* pStudioHdr, int iSequence );
   float GetSequenceMoveDist( CStudioHdr* pStudioHdr, int iSequence );
public:

   bool IsDead( );
   void SetCurrentCommand( CUserCmd* cmd );

   Vector GetEyePosition( );
   Vector GetViewHeight( );
};

class C_CSPlayer : public C_BasePlayer {
public:
   static C_CSPlayer* GetLocalPlayer( );
   static C_CSPlayer* GetPlayerByIndex( int index );

   bool IsTeammate( C_CSPlayer* player );
   bool CanShoot( int tickbase_shift = 0, bool skip_revolver = false );
   bool IsReloading( );

   Vector GetEyePosition( );

public:
   CCSGOPlayerAnimState*& m_PlayerAnimState( );
   void SetEyeAngles( QAngle Angle );
   QAngle& m_angEyeAngles( );
   int& m_nSurvivalTeam( );
   int& m_ArmorValue( );
   int& m_iAccount( );
   int& m_iShotsFired( );
   float& m_flFlashDuration( );
   float& m_flSecondFlashAlpha( );
   float& m_flVelocityModifier( );
   float& m_flLowerBodyYawTarget( );
   float& m_flSpawnTime( );
   float& m_flHealthShotBoostExpirationTime( );
   bool& m_bHasHelmet( );
   bool& m_bHasHeavyArmor( );
   bool& m_bIsScoped( );
   bool& m_bWaitForNoAttack( );
   int& m_iMatchStats_Kills( );
   int& m_iMatchStats_Deaths( );
   int& m_iMatchStats_HeadShotKills( );
   bool& m_bGunGameImmunity( );
   bool PhysicsRunThink( int unk01 );
   void Think( );
   void PreThink( );
   void PostThink( );
};

FORCEINLINE C_CSPlayer* ToCSPlayer( C_BaseEntity* pEnt ) {
   if ( !pEnt || !pEnt->IsPlayer( ) )
	  return nullptr;

   return ( C_CSPlayer* ) ( pEnt );
}
