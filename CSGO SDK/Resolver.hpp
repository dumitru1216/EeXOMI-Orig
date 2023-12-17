#pragma once
#include "LagCompensation.hpp"
#include <vector>
#include <deque>

struct C_BruteforceData {
   int ResolverSides = 0;
   int SideIndex = 0;
   bool IsResolved = false;
   bool IsSideSwitched = false;
   bool IsNeedToSwitchSide = false;
   int LastSwitchSide = 0;
   int LastHitIndex = 0;
   int LastResolverAttempt = 0;
   int LastResolverSide = 0;
   int ResolverType = 0;
   int LastResovlerMissedSide = INT_MAX;
   int PenultimateResovlerMissedSide = INT_MAX;
   int LastAnimationResolverSide = INT_MAX;
   int LastResolverHitSide = INT_MAX;
   bool MissedDueToLogicResolver = false;
   bool ForceFake = false;

   std::vector<int> ResolvedSides = { };
   std::vector<int> MissedSides = { };
   std::vector<int> NotTryingSides = { 0, 1, 2 };

   int GetYawSide( int index, Engine::C_LagRecord record );
   int ConvertSideIndex( const int value, bool side_to_index = false );
};

extern C_BruteforceData g_BruteforceData[ 65 ];

enum eResolverModes {
	NONE,
	STAND,
	STAND_BRUTE_1,
	STAND_BRUTE_2,
	STAND_LM,
	AIR,
	PRED_22,
	PRED_11,
	PRED_LBY,
	PRED_SIDE_LM,
	PRED_BW
};

struct ResolverData_t {
	struct LastMoveData_t {
		float m_flLowerBodyYawTarget;
		float m_flSimulationTime;
		float m_flAnimTime;
		Vector m_vecOrigin;
	};
	LastMoveData_t m_sMoveData;

	std::string m_ResolverText;

	float m_flNextBodyUpdate;
	int m_iResolverMode;
	bool m_bPredictingUpdates, m_bCollectedValidMoveData, m_bBrutingLastmove, m_bBrokeLby;

	__forceinline void Reset( ) {
		m_sMoveData.m_flAnimTime = 0.f;
		m_sMoveData.m_flSimulationTime = 0.f;
		m_sMoveData.m_flLowerBodyYawTarget = 0.f;

		m_flNextBodyUpdate = 0.f;
		m_iResolverMode = 0;

		m_bPredictingUpdates = false;
		m_bCollectedValidMoveData = false;
		m_bBrutingLastmove = false;
		m_bBrokeLby = false;
	}
};
extern ResolverData_t g_ResolverData[ 65 ];

namespace Engine
{
   struct ShotSnapshot {
	  int playerIdx;
	  int resolverIdx;
	  int outSequence;
	  bool correctEyePos;
	  bool correctSequence;
	  bool haveReference;
	  C_CSPlayer* player;
	  Engine::C_LagRecord resolve_record;
	  Vector eye_pos;
	  float time;

	  // onepaste
	  Vector AimPoint;
	  int SideIndex;
	  int Hitgroup;
	  bool IsPrioritySide;
	  bool IsSwitchedSide;
   };

   struct PlayerHurt_t {
	  int damage;
	  int hitgroup;
	  C_CSPlayer* player;
	  int playerIdx;
   };

   struct WeaponFire_t {
	  std::vector< Vector > impacts;
	  std::vector< PlayerHurt_t > damage;
	  std::deque< ShotSnapshot >::iterator snapshot;
   };

   class C_Resolver {
   public:
	  static Encrypted_t<C_Resolver> Get( );

	  void Start( );
	  void ProcessEvents( );

	  void EventCallback( IGameEvent* gameEvent, uint32_t hash );
	  void CreateSnapshot( C_CSPlayer* player, const Vector& shootPosition, const Vector& aimPoint, Engine::C_LagRecord* record, int resolverSide, int hitgroup );
	  void CorrectSnapshots( bool is_sending_packet );

	  std::deque< ShotSnapshot > m_Shapshots;
	  std::vector< WeaponFire_t > m_Weaponfire;

	  bool m_GetEvents = false;
   };
}