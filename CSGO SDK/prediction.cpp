#include "Prediction.hpp"
#include "Player.hpp"
#include "weapon.hpp"
#include "source.hpp"
#include "CBaseHandle.hpp"
#include "displacement.hpp"
#include "TickbaseShift.hpp"
#include "Hooked.hpp"
#include <deque>

int VelModTick = 0;
int VelModBroken = 0;

namespace Engine
{
   // C_BasePlayer::PhysicsSimulate & CPrediction::RunCommand rebuild
   void Prediction::Begin( Encrypted_t<CUserCmd> _cmd, bool* send_packet ) {
	  predictionData->m_RestoreData.is_filled = false;
	  predictionData->m_pCmd = _cmd;
	  predictionData->m_pSendPacket = send_packet;

	  if ( !predictionData->m_pCmd.IsValid( ) )
		 return;

	  predictionData->m_pPlayer = C_CSPlayer::GetLocalPlayer( );

	  if ( !predictionData->m_pPlayer || predictionData->m_pPlayer->IsDead( ) )
		 return;

	  predictionData->m_pWeapon = ( C_WeaponCSBaseGun* ) predictionData->m_pPlayer->m_hActiveWeapon( ).Get( );

	  if ( !predictionData->m_pWeapon )
		 return;

	  m_bInPrediction = true;

	  Engine::Prediction::Instance( )->RunGamePrediction( );

	  predictionData->m_nTickBase = predictionData->m_pPlayer->m_nTickBase( );

	  predictionData->m_fFlags = predictionData->m_pPlayer->m_fFlags( );
	  predictionData->m_vecVelocity = predictionData->m_pPlayer->m_vecVelocity( );

	  predictionData->m_flCurrentTime = Source::m_pGlobalVars->curtime;
	  predictionData->m_flFrameTime = Source::m_pGlobalVars->frametime;

	  if ( Engine::Displacement.Function.m_MD5PseudoRandom ) {
		 predictionData->m_pCmd->random_seed = ( ( uint32_t( __thiscall* )( uint32_t ) )Displacement.Function.m_MD5PseudoRandom )( predictionData->m_pCmd->command_number ) & 0x7fffffff;
	  }

	  // StartCommand( player, ucmd ); 
	  predictionData->m_pPlayer->SetCurrentCommand( predictionData->m_pCmd.Xor( ) );
	  C_BaseEntity::SetPredictionRandomSeed( predictionData->m_pCmd.Xor( ) );
	  C_BaseEntity::SetPredictionPlayer( predictionData->m_pPlayer );

	  Source::m_pGameMovement->StartTrackPredictionErrors( predictionData->m_pPlayer );

	  static auto m_nImpulse = Memory::FindInDataMap( predictionData->m_pPlayer->GetPredDescMap( ), XorStr( "m_nImpulse" ) );
	  static auto m_nButtons = Memory::FindInDataMap( predictionData->m_pPlayer->GetPredDescMap( ), XorStr( "m_nButtons" ) );
	  static auto m_afButtonLast = Memory::FindInDataMap( predictionData->m_pPlayer->GetPredDescMap( ), XorStr( "m_afButtonLast" ) );
	  static auto m_afButtonPressed = Memory::FindInDataMap( predictionData->m_pPlayer->GetPredDescMap( ), XorStr( "m_afButtonPressed" ) );
	  static auto m_afButtonReleased = Memory::FindInDataMap( predictionData->m_pPlayer->GetPredDescMap( ), XorStr( "m_afButtonReleased" ) );

	  // not in datamap :(
	  static auto m_afButtonForced = m_nButtons + 0x138; // 0x3330

	 //  predictionData->m_pCmd->buttons |= *reinterpret_cast< uint32_t* >( uint32_t( predictionData->m_pPlayer ) + m_afButtonForced );

	//  if ( predictionData->m_pCmd->impulse )
	//	 *reinterpret_cast< uint32_t* >( uint32_t( predictionData->m_pPlayer ) + m_nImpulse ) = predictionData->m_pCmd->impulse;

	  // CBasePlayer::UpdateButtonState
	 // int* buttons = reinterpret_cast< int* >( uint32_t( predictionData->m_pPlayer ) + m_nButtons );
	 // const int buttonsChanged = predictionData->m_pCmd->buttons ^ *buttons;
	 //
	 // // Track button info so we can detect 'pressed' and 'released' buttons next frame
	 // *reinterpret_cast< int* >( uint32_t( predictionData->m_pPlayer ) + m_afButtonLast ) = *buttons;
	 // *reinterpret_cast< int* >( uint32_t( predictionData->m_pPlayer ) + m_nButtons ) = predictionData->m_pCmd->buttons;
	 // *reinterpret_cast< int* >( uint32_t( predictionData->m_pPlayer ) + m_afButtonPressed ) = buttonsChanged & predictionData->m_pCmd->buttons;  // The changed ones still down are "pressed"
	 // *reinterpret_cast< int* >( uint32_t( predictionData->m_pPlayer ) + m_afButtonReleased ) = buttonsChanged & ~predictionData->m_pCmd->buttons; // The ones not down are "released"

	  //	CBasePlayer::UpdateButtonState
	  {
		  predictionData->m_pCmd->buttons |= *reinterpret_cast< uint32_t* >( uint32_t( predictionData->m_pPlayer ) + 0x3310 );

		  const int v16 = predictionData->m_pCmd->buttons;
		  int* unk02 = reinterpret_cast< int* >( uint32_t( predictionData->m_pPlayer ) + 0x31E8 );
		  const int v17 = v16 ^ *unk02;

		  *reinterpret_cast< int* >( uint32_t( predictionData->m_pPlayer ) + 0x31DC ) = *unk02;
		  *reinterpret_cast< int* >( uint32_t( predictionData->m_pPlayer ) + 0x31E8 ) = v16;
		  *reinterpret_cast< int* >( uint32_t( predictionData->m_pPlayer ) + 0x31E0 ) = v16 & v17;
		  *reinterpret_cast< int* >( uint32_t( predictionData->m_pPlayer ) + 0x31E4 ) = v17 & ~v16;
	  }

	  // Call standard client pre-think
	  predictionData->m_pPlayer->PreThink( );

	  // Call Think if one is set
	  predictionData->m_pPlayer->Think( );

	  // Setup input.
	  Source::m_pPrediction->SetupMove( predictionData->m_pPlayer, predictionData->m_pCmd.Xor( ), Source::m_pMoveHelper.Xor( ), &predictionData->m_MoveData );

	  predictionData->m_RestoreData.m_MoveData = predictionData->m_MoveData;
	  predictionData->m_RestoreData.is_filled = true;
	  predictionData->m_RestoreData.Setup( predictionData->m_pPlayer );

	  Source::m_pGlobalVars->curtime = static_cast< float >( predictionData->m_nTickBase ) * Source::m_pGlobalVars->interval_per_tick;
	  Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;

	  predictionData->m_bFirstTimePrediction = *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 24 );
	  predictionData->m_bInPrediction = *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 8 );

	  *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 8 ) = true;
	  *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 24 ) = false;

	  // m_bEnginePaused || player frozen
	  if ( *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 10 ) || predictionData->m_pPlayer->m_fFlags( ) & 0x40 ) {
		 Source::m_pGlobalVars->frametime = 0.0f;
	  }

	  Source::m_pMoveHelper->SetHost( predictionData->m_pPlayer );

	  Source::m_pGameMovement->ProcessMovement( predictionData->m_pPlayer, &predictionData->m_MoveData );

	  Source::m_pPrediction->FinishMove( predictionData->m_pPlayer, predictionData->m_pCmd.Xor( ), &predictionData->m_MoveData );

	  predictionData->m_pPlayer->PostThink( );

	  Source::m_pMoveHelper->SetHost( nullptr );

	  *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 24 ) = predictionData->m_bFirstTimePrediction;
	  *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 8 ) = predictionData->m_bInPrediction;

	  predictionData->m_pWeapon->UpdateAccuracyPenalty( );

	  m_flSpread = predictionData->m_pWeapon->GetSpread( );
	  m_flInaccuracy = predictionData->m_pWeapon->GetInaccuracy( );
   }

   void Prediction::Repredict( ) {
	  Engine::Prediction::Instance( )->RunGamePrediction( );

	  predictionData->m_RestoreData.Apply( predictionData->m_pPlayer );

	  // set move data members that getting copied from CUserCmd
	  // rebuilded from CPrediction::SetupMove
	  predictionData->m_MoveData = predictionData->m_RestoreData.m_MoveData;
	  predictionData->m_MoveData.m_flForwardMove = predictionData->m_pCmd->forwardmove;
	  predictionData->m_MoveData.m_flSideMove = predictionData->m_pCmd->sidemove;
	  predictionData->m_MoveData.m_flUpMove = predictionData->m_pCmd->upmove;
	  predictionData->m_MoveData.m_nButtons = predictionData->m_pCmd->buttons;
	  predictionData->m_MoveData.m_vecOldAngles.y = predictionData->m_pCmd->viewangles.pitch;
	  predictionData->m_MoveData.m_vecOldAngles.z = predictionData->m_pCmd->viewangles.yaw;
	  predictionData->m_MoveData.m_outStepHeight = predictionData->m_pCmd->viewangles.roll;
	  predictionData->m_MoveData.m_vecViewAngles.pitch = predictionData->m_pCmd->viewangles.pitch;
	  predictionData->m_MoveData.m_vecViewAngles.yaw = predictionData->m_pCmd->viewangles.yaw;
	  predictionData->m_MoveData.m_vecViewAngles.roll = predictionData->m_pCmd->viewangles.roll;
	  predictionData->m_MoveData.m_nImpulseCommand = predictionData->m_pCmd->impulse;

	  Source::m_pGameMovement->StartTrackPredictionErrors( predictionData->m_pPlayer );

	  predictionData->m_pPlayer->PreThink( );

	  predictionData->m_pPlayer->Think( );

	  Source::m_pPrediction->SetupMove( predictionData->m_pPlayer, predictionData->m_pCmd.Xor( ), Source::m_pMoveHelper.Xor( ), &predictionData->m_MoveData );

	  *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 8 ) = true;
	  *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 24 ) = false;

	  Source::m_pMoveHelper->SetHost( predictionData->m_pPlayer );

	  Source::m_pGameMovement->ProcessMovement( predictionData->m_pPlayer, &predictionData->m_MoveData );

	  Source::m_pPrediction->FinishMove( predictionData->m_pPlayer, predictionData->m_pCmd.Xor( ), &predictionData->m_MoveData );

	  predictionData->m_pPlayer->PostThink( );

	  Source::m_pMoveHelper->SetHost( nullptr );

	  *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 24 ) = predictionData->m_bFirstTimePrediction;
	  *( bool* ) ( uintptr_t( Source::m_pPrediction.Xor( ) ) + 8 ) = predictionData->m_bInPrediction;

	  predictionData->m_pWeapon->UpdateAccuracyPenalty( );

	  m_flSpread = predictionData->m_pWeapon->GetSpread( );
	  m_flInaccuracy = predictionData->m_pWeapon->GetInaccuracy( );
   }

   void Prediction::End( ) {
	  if ( !predictionData->m_pCmd.IsValid( ) || !predictionData->m_pPlayer || !predictionData->m_pWeapon )
		 return;

	  Source::m_pGameMovement->FinishTrackPredictionErrors( predictionData->m_pPlayer );

	  Source::m_pGlobalVars->curtime = predictionData->m_flCurrentTime;
	  Source::m_pGlobalVars->frametime = predictionData->m_flFrameTime;

	  auto& correct = predictionData->m_CorrectionData.emplace_front( );
	  correct.command_nr = predictionData->m_pCmd->command_number;
	  correct.tickbase = predictionData->m_pPlayer->m_nTickBase( ) + 1;
	  correct.tickbase_shift = TickbaseShiftCtx.hold_tickbase_shift ? TickbaseShiftCtx.previous_tickbase_shift : 0;
	  correct.chokedcommands = Source::m_pClientState->m_nChokedCommands( ) + 1;
	  correct.tickcount = Source::m_pGlobalVars->tickcount;

	  auto& out = predictionData->m_OutgoingCommands.emplace_back( );
	  out.is_outgoing = *predictionData->m_pSendPacket != false;
	  out.command_nr = predictionData->m_pCmd->command_number;
	  out.is_used = false;
	  out.prev_command_nr = 0;

	  if ( *predictionData->m_pSendPacket ) {
		 predictionData->m_ChokedNr.clear( );
	  } else {
		 predictionData->m_ChokedNr.push_back( predictionData->m_pCmd->command_number );
	  }

	  while ( predictionData->m_CorrectionData.size( ) > int( 2.0f / Source::m_pGlobalVars->interval_per_tick ) ) {
		 predictionData->m_CorrectionData.pop_back( );
	  }

	  // Finish command
	  predictionData->m_pPlayer->SetCurrentCommand( nullptr );
	  C_BaseEntity::SetPredictionRandomSeed( nullptr );
	  C_BaseEntity::SetPredictionPlayer( nullptr );

	  m_bInPrediction = false;
   }

   void Prediction::Invalidate( ) {
	  predictionData->m_pCmd = nullptr;
	  predictionData->m_pPlayer = nullptr;
	  predictionData->m_pWeapon = nullptr;
	  predictionData->m_pSendPacket = nullptr;
	  predictionData->m_OutgoingCommands.clear( );

	  predictionData->m_RestoreData.Reset( );
	  predictionData->m_RestoreData.is_filled = false;
   }

   void Prediction::RunGamePrediction( ) {
	   // force game to repredict data
	   if ( ( g_Vars.globals.LastVelocityModifier < 1.0f ) && g_Vars.rage.fix_velocity_modifier ) {
		   // https://github.com/pmrowla/hl2sdk-csgo/blob/49e950f3eb820d88825f75e40f56b3e64790920a/game/client/prediction.cpp#L1533
		  *( uint8_t* )( uintptr_t( Source::m_pPrediction.Xor( ) ) + 0x24 ) = 1; // m_bPreviousAckHadErrors 
		  *( uint32_t* )( uintptr_t( Source::m_pPrediction.Xor( ) ) + 0x1C ) = 0; // m_nCommandsPredicted 
		}

	  if ( Source::m_pClientState->m_nDeltaTick( ) > 0 ) {
		 Source::m_pPrediction->Update( Source::m_pClientState->m_nDeltaTick( ), Source::m_pClientState->m_nDeltaTick( ) > 0,
										Source::m_pClientState->m_nLastCommandAck( ),
										Source::m_pClientState->m_nLastOutgoingCommand( ) + Source::m_pClientState->m_nChokedCommands( ) );
	  }
   }

   int Prediction::GetFlags( ) {
	  return predictionData->m_fFlags;
   }

   Vector Prediction::GetVelocity( ) {
	  return predictionData->m_vecVelocity;
   }

   Encrypted_t<CUserCmd> Prediction::GetCmd( ) {
	  return predictionData->m_pCmd;
   }

   float Prediction::GetFrametime( ) {
	  return predictionData->m_flFrameTime;
   }

   float Prediction::GetCurtime( ) {
	  return predictionData->m_flCurrentTime;
   }

   float Prediction::GetSpread( ) {
	  return m_flSpread;
   }

   float Prediction::GetInaccuracy( ) {
	  return m_flInaccuracy;
   }

   // Netvar compression fix
   void Prediction::OnFrameStageNotify( ClientFrameStage_t stage ) {
	  if ( stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START )
		 return;

	  auto local = C_CSPlayer::GetLocalPlayer( );

	  if ( !local )
		 return;

	  auto slot = local->m_nTickBase( );
	  auto data = &predictionData->m_Data[ slot % 150 ];

	  if ( !data )
		 return;

	  if ( data->m_nTickbase != slot )
		 return;

	  auto distance_lol = [] ( const QAngle& a, const QAngle& b ) {
		 auto delta = a - b;
		 return std::sqrt( delta.x * delta.x + delta.y * delta.y + delta.z * delta.z );
	  };

	  if ( distance_lol( local->m_aimPunchAngle( ), data->m_aimPunchAngle ) < 0.5f ) {
		 local->m_aimPunchAngle( ) = data->m_aimPunchAngle;
	  }

	  if ( distance_lol( local->m_aimPunchAngleVel( ), data->m_aimPunchAngleVel ) < 0.5f ) {
		 local->m_aimPunchAngleVel( ) = data->m_aimPunchAngleVel;
	  }

	  if ( distance_lol( local->m_viewPunchAngle( ), data->m_viewPunchAngle ) < 0.5f ) {
		 local->m_viewPunchAngle( ) = data->m_viewPunchAngle;
	  }

	  if ( local->m_vecBaseVelocity( ).Distance( data->m_vecBaseVelocity ) < 0.5f ) {
		 local->m_vecBaseVelocity( ) = data->m_vecBaseVelocity;
	  }

	  if ( local->m_vecViewOffset( ).Distance( data->m_vecViewOffset ) < 0.5f ) {
		 local->m_vecViewOffset( ) = data->m_vecViewOffset;
	  }

	  if ( local->m_vecOrigin( ).Distance( data->m_vecOrigin ) < 0.5f ) {
		 local->m_vecOrigin( ) = data->m_vecOrigin;
	  }

	  if ( local->m_vecVelocity( ).Distance( data->m_vecVelocity ) < 0.5f ) {
		 local->m_vecVelocity( ) = data->m_vecVelocity;
	  }

	  if ( std::fabsf( local->m_flFallVelocity( ) - data->m_flFallVelocity ) < 0.25f ) {
		 local->m_flFallVelocity( ) = data->m_flFallVelocity;
	  }
   }

   // NOTE: fixing netvar compression is incorrect if doing in RunCommand 
   // do it in PhysicsSimulate
   void Prediction::OnRunCommand( C_CSPlayer* player, CUserCmd* cmd ) {
	  auto local = C_CSPlayer::GetLocalPlayer( );

	  if ( !local || local != player )
		 return;

	  if ( cmd->hasbeenpredicted ) {
		 for ( auto& data : predictionData->m_Data ) {
			data.m_nTickbase--;
		 }

		 return;
	  }

   }

   void Prediction::PostEntityThink( C_CSPlayer* player ) {
	  static auto PostThinkVPhysics = ( bool( __thiscall* )( C_CSPlayer* ) )Engine::Displacement.Function.m_uPostThinkVPhysics;
	  static auto SimulatePlayerSimulatedEntities = ( void( __thiscall* )( C_CSPlayer* ) )Engine::Displacement.Function.m_SimulatePlayerSimulatedEntities;

	  if ( player && !player->IsDead( ) ) {
		  // xref: UpdateCollisionBoundsFn
		  //  using Fn = void( __thiscall* )( void* );
		  //  Memory::VCall<Fn>( player, MemeIndex )( player );

		  using UpdateCollisionBoundsFn = void( __thiscall* )( void* );
		  Memory::VCall<UpdateCollisionBoundsFn>( player, 329 )( player );


		 if ( player->m_fFlags( ) & FL_ONGROUND )
			*reinterpret_cast< uintptr_t* >( uintptr_t( player ) + 0x3004 ) = 0;

		 if ( *reinterpret_cast< int* >( uintptr_t( player ) + 0x28AC ) == -1 ) {
			// xref: SetSequenceFn
			// using Fn1 = void( __thiscall* )( void*, int );
			// Memory::VCall<Fn1>( player, MemeIndex2 )( player, NULL );

			using SetSequenceFn = void( __thiscall* )( void*, int );
			Memory::VCall<SetSequenceFn>( player, 213 )( player, 0 );
		 }

		 // xref: StudioFrameAdvanceFn
		 // Memory::VCall<Fn>( player, MemeIndex2 + 1 )( player );
		 using StudioFrameAdvanceFn = void( __thiscall* )( void* );
		 Memory::VCall<StudioFrameAdvanceFn>( player, 214 )( player );

		 PostThinkVPhysics( player );
	  }
	  SimulatePlayerSimulatedEntities( player );
   }

   bool Prediction::ShouldSimulate( int command_number ) {
	  return true;
   }

   void Prediction::PacketStart( int incoming_sequence, int outgoing_acknowledged ) {
	  if ( !predictionData->m_bSendingPacket ) {
		 Hooked::oPacketStart( ( void* ) ( uintptr_t( Source::m_pClientState.Xor( ) ) + 8 ),
							   incoming_sequence, outgoing_acknowledged );
		 return;
	  }

	  if ( m_bFixSendDataGram )
		 outgoing_acknowledged = Source::m_pClientState.Xor( )->m_nLastCommandAck( );

	  if ( !predictionData->m_OutgoingCommands.empty( ) ) {
		 for ( auto it = predictionData->m_OutgoingCommands.rbegin( ); it != predictionData->m_OutgoingCommands.rend( ); ++it ) {
			if ( !it->is_outgoing ) {
			   continue;
			}

			if ( it->command_nr == outgoing_acknowledged
				 || outgoing_acknowledged > it->command_nr && ( !it->is_used || it->prev_command_nr == outgoing_acknowledged ) ) {
			   it->prev_command_nr = outgoing_acknowledged;
			   it->is_used = true;
			   Hooked::oPacketStart( ( void* ) ( uintptr_t( Source::m_pClientState.Xor( ) ) + 8 ),
									 incoming_sequence, outgoing_acknowledged );
			   break;
			}
		 }

		 auto result = false;
		 for ( auto it = predictionData->m_OutgoingCommands.begin( ); it != predictionData->m_OutgoingCommands.end( ); ) {
			if ( outgoing_acknowledged == it->command_nr || outgoing_acknowledged == it->prev_command_nr )
			   result = true;

			if ( outgoing_acknowledged > it->command_nr && outgoing_acknowledged > it->prev_command_nr ) {
			   it = predictionData->m_OutgoingCommands.erase( it );
			} else {
			   it++;
			}
		 }

		 if ( !result )
			Hooked::oPacketStart( ( void* ) ( uintptr_t( Source::m_pClientState.Xor( ) ) + 8 ),
								  incoming_sequence, outgoing_acknowledged );
	  } else {
		 Hooked::oPacketStart( ( void* ) ( uintptr_t( Source::m_pClientState.Xor( ) ) + 8 ),
							   incoming_sequence, outgoing_acknowledged );
	  }
   }

   void Prediction::PacketCorrection( uintptr_t cl_state ) {
   #if 0
	  static auto last_tick_count = 0;
	  static auto sim_time = 0.f;
	  static auto sim_time_update = 0;
   #endif

	  auto local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || local->IsDead( ) ) {
		 predictionData->m_CorrectionData.clear( );
		 return;
	  }


	  /*
	  		if( *( int* )( uintptr_t( cl_state ) + 0x164 ) == *( int* )( uintptr_t( cl_state ) + 0x16C ) ) {
			auto ack_cmd = *( int* )( uintptr_t( cl_state ) + 0x4D2C );
	  */

	  if ( ( *( int* )( uintptr_t( cl_state ) + 0x164 ) == *( int* )( uintptr_t( cl_state ) + 0x16C ) ) && g_Vars.rage.fix_velocity_modifier ) {
		 auto ack_cmd = *( int* ) ( uintptr_t( cl_state ) + 0x4D2C );
		 auto correct = std::find_if( predictionData->m_CorrectionData.begin( ), predictionData->m_CorrectionData.end( ), [ack_cmd] ( const CorrectionData& a ) {
			return a.command_nr == ack_cmd;
		 } );

		 auto netchannel = Source::m_pEngine->GetNetChannelInfo( );
		 if ( netchannel && correct != predictionData->m_CorrectionData.end( ) ) {
			if ( g_Vars.globals.LastVelocityModifier > local->m_flVelocityModifier( ) + 0.1f ) {
			   auto weapon = ( C_WeaponCSBaseGun* ) local->m_hActiveWeapon( ).Get( );
			   if ( !weapon || ( weapon->m_iItemDefinitionIndex( ) != WEAPON_REVOLVER && weapon->GetCSWeaponData( )->m_iWeaponType != WEAPONTYPE_GRENADE ) ) {
				  for ( auto nr : predictionData->m_ChokedNr ) {
					 auto cmd = &Source::m_pInput->m_pCommands[ nr % 150 ];
					 auto verfived = &Source::m_pInput->m_pVerifiedCommands[ nr % 150 ];

					 if ( cmd->buttons & ( IN_ATTACK2 | IN_ATTACK ) ) {
						cmd->buttons &= ~IN_ATTACK;
						verfived->m_cmd = *cmd;
						verfived->m_crc = cmd->GetChecksum( );
					 }
				  }
			   }
			}

			if ( g_Vars.globals.LastVelocityModifier != local->m_flVelocityModifier( ) )
			   VelModBroken = ack_cmd;

			g_Vars.globals.LastVelocityModifier = local->m_flVelocityModifier( );
			VelModTick = ack_cmd;

			//TickbaseShiftCtx.OnNewPacket( correct->command_nr, local->m_nTickBase( ), correct->tickbase, correct->tickbase_shift );
		 }
	  }
   }

   void Prediction::KeepCommunication( bool* bSendPacket ) {
   #if 0
	  auto local = C_CSPlayer::GetLocalPlayer( ); // || Engine::Prediction::Instance( )->IsPacketAcknowledged( outgoing_acknowledged, true )
	  if ( !local || local->IsDead( ) )
		 return oPacketStart( ECX, incoming_sequence, outgoing_acknowledged );
   #endif

	  auto local = C_CSPlayer::GetLocalPlayer( );
	  auto g_GameRules = *( uintptr_t** ) ( Engine::Displacement.Data.m_GameRules );
	  auto freeze = *( bool* ) ( *( uintptr_t* ) g_GameRules + 0x20 );

	  static bool is_alive = false;

	  if ( *bSendPacket ) {
		 is_alive = local && !local->IsDead( ) && !( local->m_fFlags( ) & 0x40 ) && !freeze;
		 if ( !is_alive ) {
			predictionData->m_bSendingPacket = false;
		 }

		 return;
	  }

	  if ( !is_alive )
		 return;

	  predictionData->m_bSendingPacket = true;

	  auto _netchannel = Source::m_pEngine->GetNetChannelInfo( );

	  // baboom, stack is broken 
	  Encrypted_t<INetChannel> kekl( _netchannel );
	  Encrypted_t<Encrypted_t<INetChannel>> netchannel( &kekl );

	  if ( netchannel.Xor( )->IsValid( ) && !( ( *netchannel.Xor( ) )->m_nChokedPackets % 4 ) && predictionData->m_OutgoingCommands.size( ) > 4 && ( *netchannel.Xor( ) )->m_nChokedPackets > 0 ) {
		 m_bFixSendDataGram = true;
		 const auto choked = ( *netchannel.Xor( ) )->m_nChokedPackets;
		 ( *netchannel.Xor( ) )->m_nChokedPackets = 0;
		 ( *netchannel.Xor( ) )->SendDatagram( );
		 --( *netchannel.Xor( ) )->m_nOutSequenceNr;
		 ( *netchannel.Xor( ) )->m_nChokedPackets = choked;
	  }

	  m_bFixSendDataGram = false;
   }

   void Prediction::ProccesingNetvarCompresion( ) {
	  auto local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local )
		 return;

	  // collect data for netvar compression fix
	  auto slot = local->m_nTickBase( );
	  auto data = &predictionData->m_Data[ slot % 150 ];

	  data->m_nTickbase = local->m_nTickBase( );
	  data->m_aimPunchAngle = local->m_aimPunchAngle( );
	  data->m_aimPunchAngleVel = local->m_aimPunchAngleVel( );
	  data->m_viewPunchAngle = local->m_viewPunchAngle( );
	  data->m_vecBaseVelocity = local->m_vecBaseVelocity( );
	  data->m_vecViewOffset = local->m_vecViewOffset( );
	  data->m_vecVelocity = local->m_vecVelocity( );
	  data->m_vecOrigin = local->m_vecOrigin( );
	  data->m_flFallVelocity = local->m_flFallVelocity( );
   }

   static PredictionData _xoreddata;
   Prediction::Prediction( ) : predictionData( &_xoreddata ) {
   }

   Prediction::~Prediction( ) {
   }

   void RestoreData::Setup( C_CSPlayer* player ) {
	  this->m_aimPunchAngle = player->m_aimPunchAngle( );
	  this->m_aimPunchAngleVel = player->m_aimPunchAngleVel( );
	  this->m_viewPunchAngle = player->m_viewPunchAngle( );

	  this->m_vecViewOffset = player->m_vecViewOffset( );
	  this->m_vecBaseVelocity = player->m_vecBaseVelocity( );
	  this->m_vecVelocity = player->m_vecVelocity( );
	  this->m_vecOrigin = player->m_vecOrigin( );

	  this->m_flFallVelocity = player->m_flFallVelocity( );
	  this->m_flVelocityModifier = player->m_flVelocityModifier( );
	  this->m_flDuckAmount = player->m_flDuckAmount( );
	  this->m_flDuckSpeed = player->m_flDuckSpeed( );

	  static int playerSurfaceFrictionOffset = Horizon::Memory::FindInDataMap( player->GetPredDescMap( ), XorStr( "m_surfaceFriction" ) );
	  float playerSurfaceFriction = *( float* ) ( uintptr_t( player ) + playerSurfaceFrictionOffset );

	  this->m_surfaceFriction = playerSurfaceFriction;

	  static int hGroundEntity = Horizon::Memory::FindInDataMap( player->GetPredDescMap( ), XorStr( "m_hGroundEntity" ) );
	  int playerhGroundEntity = *( int* ) ( uintptr_t( player ) + hGroundEntity );

	  this->m_hGroundEntity = playerhGroundEntity;
	  this->m_nMoveType = player->m_MoveType( );
	  this->m_nFlags = player->m_fFlags( );
	  this->m_nTickBase = player->m_nTickBase( );

	  auto weapon = ( C_WeaponCSBaseGun* ) player->m_hActiveWeapon( ).Get( );
	  if ( weapon ) {
		 static int fAccuracyPenalty = Horizon::Memory::FindInDataMap( player->GetPredDescMap( ), XorStr( "m_fAccuracyPenalty" ) );
		 float playerfAccuracyPenalty = *( float* ) ( uintptr_t( player ) + fAccuracyPenalty );

		 this->m_fAccuracyPenalty = playerfAccuracyPenalty;
		 this->m_flRecoilIndex = weapon->m_flRecoilIndex( );
	  }

	  this->is_filled = true;
   }

   void RestoreData::Apply( C_CSPlayer* player ) {
	  if ( !this->is_filled )
		 return;

	  player->m_aimPunchAngle( ) = this->m_aimPunchAngle;
	  player->m_aimPunchAngleVel( ) = this->m_aimPunchAngleVel;
	  player->m_viewPunchAngle( ) = this->m_viewPunchAngle;

	  player->m_vecViewOffset( ) = this->m_vecViewOffset;
	  player->m_vecBaseVelocity( ) = this->m_vecBaseVelocity;
	  player->m_vecVelocity( ) = this->m_vecVelocity;
	  player->m_vecOrigin( ) = this->m_vecOrigin;

	  player->m_flFallVelocity( ) = this->m_flFallVelocity;
	  player->m_flVelocityModifier( ) = this->m_flVelocityModifier;
	  player->m_flDuckAmount( ) = this->m_flDuckAmount;
	  player->m_flDuckSpeed( ) = this->m_flDuckSpeed;

	  static int playerSurfaceFrictionOffset = Horizon::Memory::FindInDataMap( player->GetPredDescMap( ), XorStr( "m_surfaceFriction" ) );
	  float playerSurfaceFriction = *( float* ) ( uintptr_t( player ) + playerSurfaceFrictionOffset );
	  *( float* ) ( uintptr_t( player ) + playerSurfaceFrictionOffset ) = this->m_surfaceFriction;

	  static int hGroundEntity = Horizon::Memory::FindInDataMap( player->GetPredDescMap( ), XorStr( "m_hGroundEntity" ) );
	  *( int* ) ( uintptr_t( player ) + hGroundEntity ) = this->m_hGroundEntity;

	  player->m_MoveType( ) = this->m_nMoveType;
	  player->m_fFlags( ) = this->m_nFlags;
	  player->m_nTickBase( ) = this->m_nTickBase;

	  auto weapon = ( C_WeaponCSBaseGun* ) player->m_hActiveWeapon( ).Get( );
	  if ( weapon ) {
		 static int fAccuracyPenalty = Horizon::Memory::FindInDataMap( player->GetPredDescMap( ), XorStr( "m_fAccuracyPenalty" ) );

		 *( float* ) ( uintptr_t( player ) + fAccuracyPenalty ) = this->m_fAccuracyPenalty;
		 weapon->m_flRecoilIndex( ) = this->m_flRecoilIndex;
	  }
   }
}
