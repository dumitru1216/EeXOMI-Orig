#pragma once
#include "sdk.hpp"

#if 0
struct C_TickbaseShift {
   int max_choke = 14;
   int future_ticks = 8;
   int max_tickbase_shift = 13;
   int ticks_allowed = 0;
   int min_tickbase_shift = 5;
   bool exploits_enabled = false;
   int lag_limit = 14;
   bool in_rapid = false;
   bool was_in_rapid = false;
   int over_choke_nr = 0;
   int will_shift_tickbase = 0;
   int commands_to_shift = 0;
   bool hold_tickbase_shift = false;
   bool didnt_shift_tifckbase_previously = false;
   bool double_tapped = false;
   int tickbase_shift_nr = 0;
   int fix_tickbase_tick = 0;
   int previous_tickbase_shift = 0;

   bool ExploitsEnabled( );

   bool IsTickcountValid( int tickcount );
   bool CanShiftTickbase( void );
   void SelectShift( void );
   void AdjustPlayerTimeBaseFix( int simulation_ticks );
   void ApplyShift( Encrypted_t<CUserCmd> cmd, bool* bSendPacket );
   void OnNewPacket( int command_nr, int tickbase_from_server, int tickbase_from_record, int tickbase_shift );
};

extern C_TickbaseShift TickbaseShiftCtx;

struct TickbaseShift_t {
	TickbaseShift_t( ) = delete;
	TickbaseShift_t( int _cmdnum, int _tickbase );
	~TickbaseShift_t( ) = default;

	int cmdnum, tickbase;
};

class TickbaseSystem {
public:
	size_t s_nSpeed = 14;

	bool m_didFakeFlick;
	bool ignoreallcmds;
	int lastShiftedCmdNr;
	int m_nServerTick;
	int m_nCompensatedServerTick;
	size_t s_nTickRate = 64;
	float s_flTickInterval = 1.f / ( float )s_nTickRate;

	// an unreplicated convar: sv_clockcorrection_msecs
	float s_flClockCorrectionSeconds = 30.f / 1000.f;

	int s_iClockCorrectionTicks = ( int )( s_flClockCorrectionSeconds * s_flTickInterval + 0.5f );
	int s_iNetBackup = 64;
	int m_nSimulationTicks = 0;
	bool s_bFreshFrame = false;
	bool s_bAckedBuild = true;
	bool s_bShiftedBullet = false;
	bool m_bSupressRecharge = false;
	bool m_bForceUnChargeState = true;
	bool allw = false;
	float s_flTimeRequired = 0.5f;
	float s_flTime = 0.5f;
	size_t s_nTicksRequired = ( int )( s_flTimeRequired / s_flTickInterval + s_flTime );
	size_t s_nTicksDelay = 32u;

	bool s_bInMove = false;
	int s_iMoveTickBase = 0;
	int s_FinalTickbase = 0;
	int s_PostTickBase = 0;
	int s_PreTickBase = 0;
	int s_PredCurtime = 0;
	int estimated_tb = 0;
	size_t s_nTicksSinceUse = 0u;
	size_t s_nTicksSinceStarted = 0u;
	bool s_InMovePrediction = false;
	int s_iServerIdealTick = 0;
	bool s_bBuilding = false;
	bool bShifting = false;
	bool charging = false;
	bool bAllow = false;
	size_t s_nExtraProcessingTicks = 0;
	std::vector<TickbaseShift_t> g_iTickbaseShifts;

	bool IsTickcountValid( int nTick );
	int AdjustPlayerTimeBase( int nSimulationTicks );
	//void OnFrameStageNotify(ClientFrameStage_t Stage);
	void OnRunSimulation( void* this_, int iCommandNumber, CUserCmd* pCmd, size_t local );
	void OnPredictionUpdate( void* prediction, void*, int startframe, bool validframe, int incoming_acknowledged, int outgoing_command );
	void OnCLMove( bool bFinalTick, float unk );
	void copy_command( CUserCmd* pCmd, bool* v1 );
	bool Building( ) const;
	bool Using( ) const;
};

extern TickbaseSystem g_TickbaseController;
#endif 

typedef void( *CLMove_t )( float accumulated_extra_samples, bool bFinalTick );
inline CLMove_t o_CLMove;
void CL_Move( float accumulated_extra_samples, bool bFinalTick );

class TickbaseSystem {
public:

	void tickbase_manipulation( Encrypted_t<CUserCmd> pCmd, bool* sendPacket );
	bool Building( ) const;
	bool Using( ) const;

	bool p_doubletap;
	bool p_shifting;
	bool p_charged_dt;
	int p_shift_commands;
	int p_shift_tickbase;
	int p_charged_ticks;
	int p_charge_timer;
	int p_ticks_to_shift;
	int p_alternate_ticks;
	int recharge_to_ticks;
	bool p_shifted;

	//bool m_double_tap;
	bool m_shifting;
	bool m_charged;
	int m_shift_cmd;
	int m_shift_tickbase;
	int m_charged_ticks;
	int m_charge_timer;
	int m_tick_to_shift;
	int m_tick_to_shift_alternate;
	int m_tick_to_recharge;
	bool m_shifted;
};

extern TickbaseSystem g_TickbaseController;