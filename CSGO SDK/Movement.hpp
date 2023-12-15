#pragma once
#include "sdk.hpp"

class CCSGOPlayerAnimState;

class C_SimulationData {
public:
   Vector m_vecVeloctity;
   Vector m_vecOrigin;

   int m_iFlags;

   bool m_bJumped;

   C_CSPlayer* m_player;
};

extern void RotateMovement( Encrypted_t<CUserCmd> cmd, QAngle wish_angle, QAngle old_angles );
extern void SimulateMovement( C_SimulationData& data );
extern void ExtrapolatePlayer( C_SimulationData& data, int ticks, const Vector& wishvel, float wishspeed, float maxSpeed );

namespace Source
{
   class __declspec( novtable ) Movement : public NonCopyable {
   public:
	  static Movement* Get( );
	  virtual void PrePrediction( Encrypted_t<CUserCmd> cmd, C_CSPlayer* pLocal, bool* pSendPacket, uintptr_t* cl_move ) = 0;
	  virtual void InPrediction( ) = 0;
	  virtual void PostPrediction( ) = 0;
	  virtual void ThirdPerson( ) = 0;
	  virtual float GetLBYUpdateTime( ) = 0;
	  virtual QAngle& GetMovementAngle( ) = 0;
	  virtual bool StopPlayer( ) = 0;
	  virtual bool GetStopState( ) = 0;
	  virtual void StopPlayerAtMinimalSpeed( ) = 0;
	  virtual bool CreateMoveRecursion( ) = 0;
	  virtual void AutoStop( int ticks = 1 ) = 0;
   protected:
	  Movement( ) { };
	  virtual ~Movement( ) { };
   };
}
