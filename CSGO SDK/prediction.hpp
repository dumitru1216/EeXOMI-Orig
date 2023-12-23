#pragma once

#include "sdk.hpp"
#include <deque>

class C_CSPlayer;

namespace Engine
{
   struct RestoreData {
	  void Reset( ) {
		 m_aimPunchAngle.Set( );
		 m_aimPunchAngleVel.Set( );
		 m_viewPunchAngle.Set( );

		 m_vecViewOffset.Set( );
		 m_vecBaseVelocity.Set( );
		 m_vecVelocity.Set( );
		 m_vecOrigin.Set( );

		 m_flFallVelocity = 0.0f;
		 m_flVelocityModifier = 0.0f;
		 m_flDuckAmount = 0.0f;
		 m_flDuckSpeed = 0.0f;
		 m_surfaceFriction = 0.0f;

		 m_fAccuracyPenalty = 0.0f;
		 m_flRecoilIndex = 0.f;

		 m_hGroundEntity = 0;
		 m_nMoveType = 0;
		 m_nFlags = 0;
		 m_nTickBase = 0;
	  }

	  void Setup( C_CSPlayer* player );

	  void Apply( C_CSPlayer* player );

	  CMoveData m_MoveData;

	  QAngle m_aimPunchAngle = {};
	  QAngle m_aimPunchAngleVel = {};
	  QAngle m_viewPunchAngle = {};

	  Vector m_vecViewOffset = {};
	  Vector m_vecBaseVelocity = {};
	  Vector m_vecVelocity = {};
	  Vector m_vecOrigin = {};

	  float m_flFallVelocity = 0.0f;
	  float m_flVelocityModifier = 0.0f;
	  float m_flDuckAmount = 0.0f;
	  float m_flDuckSpeed = 0.0f;
	  float m_surfaceFriction = 0.0f;

	  float m_fAccuracyPenalty = 0.0f;
	  float m_flRecoilIndex = 0;

	  int m_hGroundEntity = 0;
	  int m_nMoveType = 0;
	  int m_nFlags = 0;
	  int m_nTickBase = 0;

	  bool is_filled = false;
   };

   struct PlayerData {
	  int m_nTickbase = 0;
	  QAngle m_aimPunchAngle = { };
	  QAngle m_aimPunchAngleVel = { };
	  QAngle m_viewPunchAngle = { };
	  Vector m_vecBaseVelocity = { };
	  Vector m_vecViewOffset = { };
	  Vector m_vecVelocity = { };
	  Vector m_vecOrigin = { };
	  float m_flFallVelocity = 0.0f;
   };

   struct LastPredData {
	  QAngle m_aimPunchAngle = { };
	  QAngle m_aimPunchAngleVel = { };
	  Vector m_vecBaseVelocity = { };
	  Vector m_vecViewOffset = { };
	  Vector m_vecOrigin = { };
	  float m_flFallVelocity = 0.0f;
	  float m_flDuckAmount = 0.0f;
	  float m_flDuckSpeed = 0.0f;
	  float m_flVelocityModifier = 0.0f;
	  int m_nTickBase = 0;
   };

   struct CorrectionData {
	  int command_nr;
	  int tickbase;
	  int tickbase_shift;
	  int tickcount;
	  int chokedcommands;
	  bool detecting_limit;
   };

   struct OutgoingData {
	  int command_nr;
	  int prev_command_nr;

	  bool is_outgoing;
	  bool is_used;
   };

   struct PredictionData {
	  Encrypted_t<CUserCmd> m_pCmd = nullptr;
	  bool* m_pSendPacket = nullptr;

	  C_CSPlayer* m_pPlayer = nullptr;
	  C_WeaponCSBaseGun* m_pWeapon = nullptr;

	  int m_nTickBase = 0;
	  int m_fFlags = 0;
	  Vector m_vecVelocity{};

	  bool m_bInPrediction = false;
	  bool m_bFirstTimePrediction = false;
	  bool m_bSendingPacket = false;

	  float m_flCurrentTime = 0.0f;
	  float m_flFrameTime = 0.0f;

	  CMoveData m_MoveData = {};
	  RestoreData m_RestoreData;

	  PlayerData m_Data[ 150 ] = { };

	  std::deque<CorrectionData> m_CorrectionData;
	  std::vector<OutgoingData> m_OutgoingCommands;
	  std::vector<int> m_ChokedNr;

	  bool m_bInitDatamap = false;
	  int m_Offset_nImpulse;
	  int m_Offset_nButtons;
	  int m_Offset_afButtonLast;
	  int m_Offset_afButtonPressed;
	  int m_Offset_afButtonReleased;
	  int m_Offset_afButtonForced;
   };

   class Prediction : public Core::Singleton<Prediction> {
   public:
	   void StartCommand( C_CSPlayer* player, CUserCmd* ucmd );
	   void Begin( Encrypted_t<CUserCmd> cmd, bool* send_packet );
	  void Repredict( );
	  void End( );
	  void Invalidate( );
	  void RunGamePrediction( );

	  int GetFlags( );
	  Vector GetVelocity( );

	  Encrypted_t<CUserCmd> GetCmd( );

	  float GetFrametime( );
	  float GetCurtime( );
	  float GetSpread( );
	  float GetInaccuracy( );

	  void OnFrameStageNotify( ClientFrameStage_t stage );
	  void OnRunCommand( C_CSPlayer* player, CUserCmd* cmd );

	  void PostEntityThink( C_CSPlayer* player );

	  bool ShouldSimulate( int command_number );
	  void PacketStart( int incoming_sequence, int outgoing_acknowledged );
	  void PacketCorrection( uintptr_t cl_state );
	  void KeepCommunication( bool* bSendPacket );
	  void ProccesingNetvarCompresion( );

	  bool InPrediction( ) {
		 return m_bInPrediction;
	  }

   private:
	  Prediction( );
	  ~Prediction( );

	  LastPredData m_LastPredictedData;

	  friend class Core::Singleton<Prediction>;
	  Encrypted_t<PredictionData> predictionData;
	  bool m_bInPrediction = false;

	  float m_flSpread = 0.f;
	  float m_flInaccuracy = 0.f;
	  
	  bool m_bFixSendDataGram = false;
	  bool m_bNeedStoreNetvarsForFixingNetvarCompresion = false;
   #if 0
	  CUserCmd * m_pCmd = nullptr;

	  C_CSPlayer* m_pPlayer = nullptr;
	  C_WeaponCSBaseGun* m_pWeapon = nullptr;

	  int m_Activity = 0;
	  int m_nTickBase = 0;
	  int m_fFlags = 0;
	  Vector m_vecVelocity{};

	  float m_flCurrentTime = 0.0f;
	  float m_flFrameTime = 0.0f;

	  PlayerData m_Data[ 128 ] = { };
   #endif
   };
}
