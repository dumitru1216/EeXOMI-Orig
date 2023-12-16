#pragma once
#include "sdk.hpp"

#include "Weapon.hpp"
#include "Player.hpp"

#include <map>
#include <deque>

namespace Engine
{
   class C_SideAnimation {
   public:
	  alignas( 16 ) matrix3x4_t m_Bones[ 128 ]{};
	  float m_flAbsRotation;

	  void Update( C_CSPlayer* player ) {
		 auto animState = player->m_PlayerAnimState( );
		 m_flAbsRotation = animState->m_flAbsRotation;
	  }
   };

   class C_SimulationInfo {
   public:
	  bool bOnGround = false;
	  float m_flTime = 0.f;
	  float m_flDuckAmount = 0.f;
	  float m_flEyeYaw = 0.f;
	  Vector m_vecOrigin{};
	  Vector m_vecVelocity{};
   };

   class C_AnimationRecord {
   public:
	  bool m_bIsInvalid;
	  bool m_bIsShoting;
	  bool m_bTeleportDistance;
	  bool m_bShiftingTickbase;
	  bool m_bFakeWalking;

	  bool m_bNoFakeAngles;
	  bool m_bAnimationResolverUsed;
	  bool m_bHistoryResolver;

	  int m_iResolverSide;

	  Vector m_vecOrigin;
	  Vector m_vecVelocity;
	  QAngle m_angEyeAngles;

	  int m_fFlags;
	  int m_iChokeTicks;

	  float m_flChokeTime;
	  float m_flSimulationTime;
	  float m_flShotTime;
	  float m_flDuckAmount;
	  float m_flLowerBodyYawTarget;

	  float m_flFeetYawRate;
	  float m_flFeetCycle;

	  float m_flAbsRotation;
	  float m_flAbsRotationRight;
	  float m_flAbsRotationLeft;
	  float m_flAbsRotationRightLow;
	  float m_flAbsRotationLeftLow;

	  C_AnimationLayer m_serverAnimOverlays[ 13 ];

	  C_AnimationLayer fakeLayersFake[ 13 ];
	  C_AnimationLayer fakeLayersRight[ 13 ];
	  C_AnimationLayer fakeLayersLeft[ 13 ];
	  C_AnimationLayer fakeLayersRightLow[ 13 ];
	  C_AnimationLayer fakeLayersLeftLow[ 13 ];
   };

   class C_AnimationData {
   public:
	  void Update( );
	  void Collect( C_CSPlayer* player );

	  void SimulateAnimations( Encrypted_t<Engine::C_AnimationRecord> current, Encrypted_t<Engine::C_AnimationRecord> previous );
	  void AnimationResolver( Encrypted_t<Engine::C_AnimationRecord> current );

	  C_CSPlayer* player;
	  int ent_index;

	  float m_flSpawnTime;
	  float m_flSimulationTime;
	  float m_flOldSimulationTime;

	  Vector m_vecOrigin;

	  bool m_bSuppressAnimationResolver = false;
	  bool m_bIsDormant = false;
	  bool m_bBonesCalculated = false;
	  bool m_bUpdated = false;

	  bool m_bResolved = false;
	  bool m_bIsAlive = false;
	  bool m_bInvertedSide = false;
	  bool m_bForceFake = false;
	  bool m_bAimbotTarget = false;
	  float m_flLastScannedYaw = 0.0f;

	  int m_iCurrentTickCount = 0;
	  int m_iOldTickCount = 0;
	  int m_iTicksAfterDormancy = 0;
	  int m_iTicksUnknown = 0;
	  int m_iResolverSide = 0;
	  int m_iLastLowDeltaTick = 0;

	  std::deque<C_AnimationRecord> m_AnimationRecord;
	  std::vector<C_SimulationInfo> m_vecSimulationData;

	  C_SideAnimation m_Animations[ 5 ] = { };
   };

   class __declspec( novtable ) AnimationSystem : public NonCopyable {
   public:
	  static Encrypted_t<AnimationSystem> Get( );

	  virtual void CollectData( ) = 0;
	  virtual void Update( ) = 0;

	  virtual C_AnimationData* GetAnimationData( int index ) = 0;

   protected:
	  AnimationSystem( ) { };
	  virtual ~AnimationSystem( ) { };
   };
}
