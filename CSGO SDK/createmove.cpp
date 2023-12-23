#include "Hooked.hpp"
#include "Displacement.hpp"
#include "Player.hpp"
#include "weapon.hpp"
#include "Prediction.hpp"
#include "Movement.hpp"
#include <intrin.h>
#include "Ragebot.hpp"
#include "Miscellaneous.hpp"
#include "InputSys.hpp"
#include "Exploits.hpp"
#include "Fakelag.hpp"
#include "LagExploit.hpp"
#include "LagCompensation.hpp"
#include "Utils/threading.h"
#include "CCSGO_HudDeathNotice.hpp"
#include "LegitBot.hpp"
#include "displacement.hpp"
#include "Profiler.hpp"
#include "Resolver.hpp"
#include <thread>
#include "GrenadePrediction.hpp"
#include "TickbaseShift.hpp"

extern float fl_Override;
extern bool g_Override;
bool HideRealAfterShot = false;

CUserCmd* previous_cmd = nullptr;

Vector AutoPeekPos;
bool WasShootinPeek = false;

int LastShotTime = 0;
int OutgoingTickcount = 0;

void PreserveKillfeed( ) {
	auto pLocal = C_CSPlayer::GetLocalPlayer( );

	static auto ClearNoticies = ( void( __thiscall* )( uintptr_t ) ) Engine::Displacement.Function.m_uClearDeathNotices;
	static auto DeathNoticeHud = FindHudElement<uintptr_t>( XorStr( "CCSGO_HudDeathNotice" ) );

	if ( !pLocal || pLocal->IsDead( ) || !g_Vars.globals.RenderIsReady || !g_Vars.globals.HackIsReady || !Source::m_pEngine->IsInGame( ) ) {
		DeathNoticeHud = 0;
		return;
	}

	if ( DeathNoticeHud == 0 )
		DeathNoticeHud = FindHudElement<uintptr_t>( XorStr( "CCSGO_HudDeathNotice" ) );

	if ( DeathNoticeHud ) {
		if ( g_Vars.esp.preserve_killfeed ) {
			if ( !g_Vars.globals.IsRoundFreeze && !pLocal->IsDead( ) ) {
				*( float* )( DeathNoticeHud + 80 ) = g_Vars.esp.preserve_killfeed_time;
			}
		}

		static float LastSpawnTime = 0.0f;
		float spawn_time = pLocal->m_flSpawnTime( );
		if ( *( float* )( DeathNoticeHud + 80 ) > 1.5f
			 && ( !g_Vars.esp.preserve_killfeed || LastSpawnTime != spawn_time ) ) {
			*( float* )( DeathNoticeHud + 80 ) = 1.5f;
			ClearNoticies( DeathNoticeHud - 20 );
			LastSpawnTime = spawn_time;
		}
	}
}

namespace Hooked {
	bool CreateMoveHandler( float ft, CUserCmd* _cmd, bool* bSendPacket ) {
		auto bRet = oCreateMove( ft, _cmd );

		g_Vars.globals.m_bInCreateMove = true;

		auto pLocal = C_CSPlayer::GetLocalPlayer( );
		if ( pLocal->IsDead( ) ) {
			WasShootinPeek = false;
			AutoPeekPos.Set( );

			Engine::Prediction::Instance( ).Invalidate( );
			g_Vars.globals.m_bInCreateMove = false;
			return bRet;
		}

		auto weapon = ( C_WeaponCSBaseGun* )( pLocal->m_hActiveWeapon( ).Get( ) );
		if ( !weapon ) {
			Engine::Prediction::Instance( ).Invalidate( );
			g_Vars.globals.m_bInCreateMove = false;

			return bRet;
		}

#if 0
		TickbaseShiftCtx.SelectShift( );
#endif
		Encrypted_t<CUserCmd> cmd( _cmd );

		if ( g_Vars.esp.force_sniper_crosshair ) {
			auto m_iCrosshairData = Source::m_pCvar->FindVar( XorStr( "weapon_debug_spread_show" ) );
			m_iCrosshairData->SetValue( !pLocal->m_bIsScoped( ) ? 3 : 0 );
		}

		Encrypted_t<CVariables::GLOBAL> globals( &g_Vars.globals );

		ILagExploit::Get( )->Main( bSendPacket );

		static QAngle lockedAngles = QAngle( );

		if ( g_Vars.globals.WasShootingInChokeCycle )
			cmd->viewangles = lockedAngles;

		if ( g_Vars.rage.enabled )
			cmd->tick_count += TIME_TO_TICKS( Engine::LagCompensation::Get( )->GetLerp( ) );

		if ( cmd->weaponselect > 0 ) {
			cmd->buttons &= ~IN_ATTACK;
		}

		auto weaponInfo = weapon->GetCSWeaponData( );

		Engine::Prediction::Instance( )->RunGamePrediction( );

		auto& prediction = Engine::Prediction::Instance( );
		auto movement = Source::Movement::Get( );

		movement->PrePrediction( cmd, pLocal, bSendPacket, nullptr );
		prediction.Begin( cmd, bSendPacket );
		{
			if ( g_Vars.misc.autopeek ) {
				static bool was_enabled = true;
				if ( g_Vars.misc.autopeek_bind.enabled ) {
					if ( was_enabled ) {
						static bool switch_option = false;
						switch_option = !switch_option;
						if ( switch_option ) {
							AutoPeekPos = pLocal->m_vecOrigin( );
							if ( !( pLocal->m_fFlags( ) & FL_ONGROUND ) ) {
								Ray_t ray;
								ray.Init( pLocal->m_vecOrigin( ), pLocal->m_vecOrigin( ) - Vector( 0.0f, 0.0f, 1000.0f ) );

								CTraceFilterWorldAndPropsOnly filter;
								CGameTrace tr;
								Source::m_pEngineTrace->TraceRay( ray, 0x46004003u, &filter, &tr );
								if ( tr.fraction < 1.0f ) {
									AutoPeekPos = tr.endpos + Vector( 0.0f, 0.0f, 2.0f );
								}
							}
						} else {
							AutoPeekPos = Vector( 0.0f, 0.0f, 0.0f );
						}
					}
					was_enabled = false;
				} else {
					was_enabled = true;
				}
			}

			movement->InPrediction( );

#if 0
			static float old_spawntime = 0.f;
			static auto memes = false;
			if ( memes ) {
				GoalChoke = 3;
				FakelagMemes = false;
				memes = false;
				LagLimit = 1;
				PreviousLagLimit = 0;
			}

			static bool init = true;
			if ( FakelagMemes || init ) {
				if ( old_spawntime != pLocal->m_flSpawnTime( ) ) {
					TickbaseStart = cmd->command_number;
					*bSendPacket = true;
					memes = true;
					old_spawntime = pLocal->m_flSpawnTime( );
				} else if ( InputSys::Get( )->IsKeyDown( VirtualKeys::F ) ) {
					TickbaseStart = cmd->command_number;
					*bSendPacket = true;
					memes = true;
				}
				init = false;
			}

			if ( !FakelagMemes ) {
				*bSendPacket = Source::m_pClientState->m_nChokedCommands( ) >= GoalChoke;
				if ( *bSendPacket )
					GoalChoke++;

				if ( Source::m_pClientState->m_nChokedCommands( ) >= 32 ) {
					*bSendPacket = true;
					FakelagMemes = true;
					g_Vars.fakelag.lag_limit = 24;
					LagLimit = 24;
				}
			}
			//else
#endif

  //if ( Source::m_pClientState->m_nChokedCommands( ) >= TickbaseShiftCtx.lag_limit ) {
  //	*bSendPacket = true;
  //}

			movement->PostPrediction( );

			if ( cmd->buttons & IN_ATTACK
				 && weapon->m_iItemDefinitionIndex( ) != WEAPON_C4
				 && weaponInfo->m_iWeaponType >= WEAPONTYPE_KNIFE
				 && weaponInfo->m_iWeaponType <= WEAPONTYPE_MACHINEGUN
				 && pLocal->CanShoot( )
				 && ( weaponInfo->m_ucFullAuto /*|| ( prev_buttons & IN_ATTACK ) == 0 )*/ ) ) {
				lockedAngles = cmd->viewangles;
				LastShotTime = Source::m_pGlobalVars->tickcount;

				g_Vars.globals.m_flLastShotTime = Source::m_pGlobalVars->realtime;

				//if ( TickbaseShiftCtx.exploits_enabled || TickbaseShiftCtx.hold_tickbase_shift ) {
				//   if ( !TickbaseShiftCtx.in_rapid )
				//	  *bSendPacket = true;
				//}

				g_Vars.globals.WasShootingInChokeCycle = !( *bSendPacket );
				g_Vars.globals.WasShooting = true;

				if ( weaponInfo->m_iWeaponType != WEAPONTYPE_KNIFE )
					WasShootinPeek = true;

				//if ( TickbaseShiftCtx.will_shift_tickbase > 0 ) {
				//   TickbaseShiftCtx.hold_tickbase_shift = 1;
				//   TickbaseShiftCtx.tickbase_shift_nr = cmd->command_number;
				//   TickbaseShiftCtx.fix_tickbase_tick = pLocal->m_nTickBase( );
				//   TickbaseShiftCtx.previous_tickbase_shift = TickbaseShiftCtx.will_shift_tickbase;
				//}
			} else {
				g_Vars.globals.WasShooting = false;
			}

			// TickbaseShiftCtx.ApplyShift( cmd, bSendPacket );

			IGrenadePrediction::Get( )->Tick( cmd->buttons );

			g_Vars.globals.m_flPreviousDuckAmount = pLocal->m_flDuckAmount( );

			Engine::C_Resolver::Get( )->CorrectSnapshots( *bSendPacket );
		}
		prediction.End( );

		if ( g_Vars.antiaim.enabled && g_Vars.antiaim.manual && g_Vars.antiaim.mouse_override.enabled ) {
			pLocal->pl( ).v_angle = globals->PreviousViewangles;
		}

		if ( *bSendPacket ) {
			if ( g_Vars.globals.WasShooting && !g_Vars.globals.Fakeducking )
				HideRealAfterShot = true;
			else if ( HideRealAfterShot )
				HideRealAfterShot = false;

			g_Vars.globals.WasShootingInChokeCycle = false;

			g_Vars.globals.LastChokedCommands = Source::m_pClientState->m_nChokedCommands( );

			if ( g_Vars.globals.FixCycle ) {
				g_Vars.globals.FixCycle = false;
				g_Vars.globals.UnknownCycleFix = true;
			}

			previous_cmd = cmd.Xor( );

			OutgoingTickcount = Source::m_pGlobalVars->tickcount;

			// TODO: make this lag compensated
			g_Vars.globals.m_iNetworkedTick = pLocal->m_nTickBase( );
			g_Vars.globals.m_vecNetworkedOrigin = pLocal->m_vecOrigin( );
		}

		g_Vars.r_jiggle_bones->SetValue( 0 );

		if ( g_Vars.misc.anti_untrusted ) {
			cmd->viewangles.Normalize( );
			cmd->viewangles.Clamp( );
		}

		g_Vars.globals.m_bInCreateMove = false;

		return false;
	}

	bool __stdcall CreateMove( float ft, CUserCmd* _cmd ) {
		g_Vars.globals.szLastHookCalled = XorStr( "2" );
		if ( !_cmd ) {
			return oCreateMove( ft, _cmd );
		}

		static float VirtualYaw = 0.0f;
		if ( g_Vars.antiaim.enabled && g_Vars.antiaim.manual && g_Vars.antiaim.mouse_override.enabled ) {
			VirtualYaw -= std::remainderf( _cmd->viewangles.yaw - g_Vars.globals.PreviousViewangles.yaw, 360.0f );
			VirtualYaw = std::remainderf( VirtualYaw, 360.0f );
			g_Vars.globals.MouseOverrideYaw = VirtualYaw;
			g_Vars.globals.MouseOverrideEnabled = true;

			Source::m_pEngine->SetViewAngles( g_Vars.globals.PreviousViewangles );

			if ( !_cmd->command_number ) {
				oCreateMove( ft, _cmd );
				return false;
			}
		} else {
			g_Vars.globals.PreviousViewangles = _cmd->viewangles;
			VirtualYaw = _cmd->viewangles.yaw;
		}

		if ( !_cmd->command_number )
			return oCreateMove( ft, _cmd );

		PreserveKillfeed( );

		Encrypted_t<uintptr_t> memes( ( uintptr_t* )_AddressOfReturnAddress( ) );
		bool* bSendPacket = reinterpret_cast< bool* >( uintptr_t( memes.Xor( ) ) + 0x14 );
		if ( !( *bSendPacket ) )
			*bSendPacket = true;

		auto result = CreateMoveHandler( ft, _cmd, bSendPacket );

		Engine::Prediction::Instance( )->KeepCommunication( bSendPacket );

		auto pLocal = C_CSPlayer::GetLocalPlayer( );
		if ( !g_Vars.globals.HackIsReady || !pLocal || !Source::m_pEngine->IsInGame( ) ) {
			Engine::Prediction::Instance( ).Invalidate( );
			return oCreateMove( ft, _cmd );
		}

		return result;
	}

	void __vectorcall CL_Move( float accumulated_extra_samples, bool bFinalTick ) {
		g_TickbaseController.OnCLMove( bFinalTick, accumulated_extra_samples );
	}

	void __fastcall RunSimulation( void* this_, void*, int iCommandNumber, CUserCmd* pCmd, size_t local ) {
		g_TickbaseController.OnRunSimulation( this_, iCommandNumber, pCmd, local );
	}

	void __fastcall PredictionUpdate( void* prediction, void*, int startframe, bool validframe, int incoming_acknowledged, int outgoing_command ) {
		g_TickbaseController.OnPredictionUpdate( prediction, nullptr, startframe, validframe, incoming_acknowledged, outgoing_command );
	}

#if 0
	void __vectorcall CL_Move( float accumulated_extra_samples, bool bFinalTick ) {
		g_Vars.globals.szLastHookCalled = XorStr( "3" );
		auto local = C_CSPlayer::GetLocalPlayer( );
		if ( !local || local->IsDead( ) || !Source::m_pEngine->IsConnected( ) || !Source::m_pEngine->IsInGame( ) || !g_Vars.globals.HackIsReady || !local->m_hActiveWeapon( ).Get( ) || g_Vars.globals.hackUnload ) {
			oCL_Move( accumulated_extra_samples, bFinalTick );

			printf( "1" ); // pass

			return;
		}

		if ( !TickbaseShiftCtx.CanShiftTickbase( ) ) {
			TickbaseShiftCtx.over_choke_nr = Source::m_pClientState->m_nLastOutgoingCommand( ) + Source::m_pClientState->m_nChokedCommands( ) + 1;

			printf( "2" ); // pass

			return;
		}

		if ( !TickbaseShiftCtx.exploits_enabled ) {
			printf( "3" );

			if ( TickbaseShiftCtx.ticks_allowed - Source::m_pClientState->m_nChokedCommands( ) > 1 ) {
				auto choke = Source::m_pClientState->m_nChokedCommands( );
				auto teleport_release = false;
				if ( teleport_release ) {
					printf( "4" );

					do {
						TickbaseShiftCtx.over_choke_nr = choke + Source::m_pClientState->m_nLastOutgoingCommand( ) + 2;
						TickbaseShiftCtx.lag_limit = 1;
						TickbaseShiftCtx.in_rapid = true;
						oCL_Move( accumulated_extra_samples, bFinalTick );
						choke = Source::m_pClientState->m_nChokedCommands( );

						printf( "6" );

					} while ( TickbaseShiftCtx.ticks_allowed - choke > 1 );
				} else {
					printf( "5" );

					TickbaseShiftCtx.over_choke_nr = choke + Source::m_pClientState->m_nLastOutgoingCommand( ) + 2;
					TickbaseShiftCtx.lag_limit = 1;
					TickbaseShiftCtx.in_rapid = true;
					oCL_Move( accumulated_extra_samples, bFinalTick );
				}

				TickbaseShiftCtx.in_rapid = false;
				TickbaseShiftCtx.was_in_rapid = true;
			}
		}

		oCL_Move( accumulated_extra_samples, bFinalTick );

		TickbaseShiftCtx.in_rapid = false;
		TickbaseShiftCtx.was_in_rapid = false;
	}
#endif
}
