#pragma once
#include "sdk.hpp"

struct C_TickbaseShift {
   int max_choke = 0x10;
   int future_ticks = 8;
   int max_tickbase_shift = 0xC;
   int ticks_allowed = 0;
   int min_tickbase_shift = 5;
   bool exploits_enabled = false;
   int lag_limit = 0xE;
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