#pragma once
#include "sdk.hpp"

#define DOUBLE_TAP_CHARGE 14
#define BREAK_LC_CHARGE 14
#define BREAK_LC_CHARGE_HIDE_SHOTS 14
#define HIDE_SHOTS_CHARGE 8

namespace Engine
{
   class C_LagRecord;
}

namespace Source
{
   class __declspec( novtable ) Ragebot : public NonCopyable {
   public:
	  enum SelectTarget_e : int {
		 SELECT_LOWEST_HP = 0,
		 SELECT_LOWEST_DISTANCE,
		 SELECT_LOWEST_PING,
		 SELECT_HIGHEST_ACCURACY
	  };

	  static Encrypted_t<Ragebot> Get( );
	  virtual bool Run( Encrypted_t<CUserCmd> cmd, C_CSPlayer* local, bool* sendPacket ) = 0;
	  virtual bool GetBoxOption( mstudiobbox_t* hitbox, mstudiohitboxset_t* hitboxSet, float& ps, bool override_hitscan ) = 0;
	  virtual void Multipoint( C_CSPlayer* player, Engine::C_LagRecord* record, int side, std::vector<Vector>& points, mstudiobbox_t* hitbox, mstudiohitboxset_t* hitboxSet, float ps ) = 0;
	  virtual bool OverrideHitscan( C_CSPlayer* player, Engine::C_LagRecord* record, int type ) = 0;
	  virtual bool SetupRageOptions( ) = 0;
   protected:
	  Ragebot( ) { };
	  virtual ~Ragebot( ) { };
   };
}
