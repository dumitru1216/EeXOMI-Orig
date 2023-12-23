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
#endif 

struct TickbaseShift_t {
	TickbaseShift_t( ) = delete;
	TickbaseShift_t( int _cmdnum, int _tickbase );
	~TickbaseShift_t( ) = default;

	int cmdnum, tickbase;
};

class TickbaseSystem {
public:
	size_t s_nSpeed = 14;

	size_t s_nTickRate = 64;
	float s_flTickInterval = 1.f / ( float )s_nTickRate;

	// an unreplicated convar: sv_clockcorrection_msecs
	float s_flClockCorrectionSeconds = 30.f / 1000.f;

	int s_iClockCorrectionTicks = ( int )( s_flClockCorrectionSeconds * s_flTickInterval + 0.5f );
	int s_iNetBackup = 64;

	bool s_bFreshFrame = false;
	bool s_bAckedBuild = true;

	bool m_bSupressRecharge = false;

	float s_flTimeRequired = 0.4f;
	size_t s_nTicksRequired = ( int )( s_flTimeRequired / s_flTickInterval + 0.5f );
	size_t s_nTicksDelay = 32u;

	bool s_bInMove = false;
	int s_iMoveTickBase = 0;
	size_t s_nTicksSinceUse = 0u;
	size_t s_nTicksSinceStarted = 0u;

	int s_iServerIdealTick = 0;
	bool s_bBuilding = false;

	size_t s_nExtraProcessingTicks = 0;
	std::vector<TickbaseShift_t> g_iTickbaseShifts;

	bool IsTickcountValid( int nTick );

	void OnRunSimulation( void* this_, int iCommandNumber, CUserCmd* pCmd, size_t local );
	void OnPredictionUpdate( void* prediction, void*, int startframe, bool validframe, int incoming_acknowledged, int outgoing_command );
	void OnCLMove( bool bFinalTick, float unk );

	bool Building( ) const;
	bool Using( ) const;
};

extern TickbaseSystem g_TickbaseController;