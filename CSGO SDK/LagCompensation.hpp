#pragma once
#include "sdk.hpp"

#include "Weapon.hpp"
#include "Player.hpp"

#include <map>
#include <deque>
#include "AnimationSystem.hpp"

#define RESOLVER_SIDE_FAKE 0
#define RESOLVER_SIDE_LEFT 1
#define RESOLVER_SIDE_RIGHT -1

#define ANIMATION_RESOLVER_FAKE 1
#define ANIMATION_RESOLVER_LEFT 2
#define ANIMATION_RESOLVER_RIGHT 4

namespace Engine
{
   // base lag record, generally used for backup and restore
   class C_BaseLagRecord {
   public:
	  C_CSPlayer* player = nullptr;
	  Vector m_vecMins, m_vecMaxs;
	  Vector m_vecOrigin;
	  QAngle m_angAngles;

	  float m_flSimulationTime;

	  alignas( 16 ) matrix3x4_t m_BoneMatrix[ 128 ];

	  void Setup( C_CSPlayer* player );
	  void Apply( C_CSPlayer* player );
   };

   // full lag record, will be stored in lag compensation history
   class C_LagRecord : public C_BaseLagRecord {
   public:
	  float m_flServerLatency;
	  float m_flRealTime;
	  float m_flLastShotTime;
	  float m_flEyeYaw;
	  float m_flEyePitch;
	  float m_flAnimationVelocity;

	  bool m_bIsShoting;
	  bool m_bIsValid;
	  bool m_bBonesCalculated;
	  bool m_bExtrapolated;
	  bool m_bIsNoDesyncAnimation;

	  alignas( 16 ) matrix3x4_t m_BoneMatrixLeft[ 128 ];
	  alignas( 16 ) matrix3x4_t m_BoneMatrixRight[ 128 ];
	  alignas( 16 ) matrix3x4_t m_BoneMatrixLowLeft[ 128 ];
	  alignas( 16 ) matrix3x4_t m_BoneMatrixLowRight[ 128 ];
	  float m_flAbsRotationLeft;
	  float m_flAbsRotationRight;
	  float m_flAbsRotationLeftLow;
	  float m_flAbsRotationRightLow;

	  int m_iResolverSide = 0;
	  int m_iRecordPriority = 0;
	  int m_iFlags;
	  int m_iLaggedTicks = 0;

	  float m_flInterpolateTime = 0.f;

	  // for sorting
	  Vector m_vecVelocity;
	  float m_flDuckAmount;

	  bool m_bSkipDueToResolver = false; // skip record in hitscan
	  bool m_bTeleportDistance = false; // teleport distance was broken

	  float GetAbsYaw( /*int matrixIdx*/ );
	  matrix3x4_t* GetBoneMatrix( /*int matrixIdx*/ );
	  void Setup( C_CSPlayer* player );
	  void Apply( C_CSPlayer* player /*, int matrixIdx = 0*/ );
   };

   class C_EntityLagData {
   public:
	  C_EntityLagData( );

	  static void UpdateRecordData( Encrypted_t< C_EntityLagData > pThis, C_CSPlayer* player, const player_info_t& info, int updateType );

	  static bool Extrapolate( Encrypted_t<C_EntityLagData> pThis, C_CSPlayer* player, Engine::C_LagRecord* outRecord );

	  void Clear( );

	  std::deque<Engine::C_LagRecord> m_History = {};
	  int m_iUserID = -1;

	  float m_flLastUpdateTime = 0.0f;
	  float m_flRate = 0.0f;

	  int m_iMissedShots = 0;
	  int m_iResolverType = 0;
	  int m_iResolverSide = 0;

	  // resolver new
	  int m_iMissedStand1 = 0; // lm
	  int m_iMissedStand2 = 0; // vm-brute
	  int m_iResolverMode = 0;

	  bool m_bGotAbsYaw = false;
	  bool m_bGotAbsYawShot = false;
	  bool m_bNotResolveIfShooting = false;
	  bool m_bRateCheck = false;

	  float m_flAbsYawHandled = 0.f;

	  // ragebot scan data
	  float m_flLastSpread, m_flLastInaccuracy;
	  float m_flLastScanTime;
	  Vector m_vecLastScanPos;

	  // estimate next origin, shouldnt be used for aimbot
	  Vector GetExtrapolatedPosition( C_CSPlayer* player );

	  // player prediction, need many improvments
	  static bool DetectAutoDirerction( Encrypted_t< C_EntityLagData > pThis, C_CSPlayer* player );
   };

   class __declspec( novtable ) LagCompensation : public NonCopyable {
   public:
	  static LagCompensation* Get( );

	  virtual void Update( ) = 0;
	  virtual bool IsRecordOutOfBounds( const Engine::C_LagRecord& record, float target_time = 0.2f, int tickbase_shift = 0, bool tick_count_check = true ) const = 0;;
	  virtual float GetLerp( ) const = 0;

	  virtual Encrypted_t<C_EntityLagData> GetLagData( int entindex ) = 0;

	  virtual void ClearLagData( ) = 0;

	  float m_flOutLatency;
	  float m_flServerLatency;
   protected:
	  LagCompensation( ) { };
	  virtual ~LagCompensation( ) { };
   };
}
