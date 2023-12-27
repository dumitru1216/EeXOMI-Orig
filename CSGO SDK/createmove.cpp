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
	auto local = C_CSPlayer::GetLocalPlayer( );

	if ( !local || !Source::m_pEngine->IsInGame( ) || !Source::m_pEngine->IsConnected( ) ) {
		return;
	}

	static auto status = false;
	static float m_spawn_time = local->m_flSpawnTime( );

	auto set = false;
	if ( m_spawn_time != local->m_flSpawnTime( ) || status != g_Vars.esp.preserve_killfeed ) {
		set = true;
		status = g_Vars.esp.preserve_killfeed;
		m_spawn_time = local->m_flSpawnTime( );
	}

	for ( int i = 0; i < Source::g_pDeathNotices->m_vecDeathNotices.Count( ); i++ ) {
		auto cur = &Source::g_pDeathNotices->m_vecDeathNotices[ i ];
		if ( !cur ) {
			continue;
		}

		if ( local->IsDead( ) || set ) {
			if ( cur->set != 1.f && !set ) {
				continue;
			}

			cur->m_flStartTime = Source::m_pGlobalVars->curtime;
			cur->m_flStartTime -= local->m_iHealth( ) <= 0 ? 2.f : 7.5f;
			cur->set = 2.f;

			continue;
		}

		if ( cur->set == 2.f ) {
			continue;
		}

		if ( !status ) {
			cur->set = 1.f;
			return;
		}

		if ( cur->set == 1.f ) {
			continue;
		}

		if ( cur->m_flLifeTimeModifier == 1.5f ) {
			cur->m_flStartTime = FLT_MAX;
		}

		cur->set = 1.f;
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
			g_Vars.globals.aStartPos.Set( );

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
							AutoPeekPos = pLocal->m_vecOrigin( ); // startoos
							g_Vars.globals.aStartPos = pLocal->m_vecOrigin( );
							if ( !( pLocal->m_fFlags( ) & FL_ONGROUND ) ) {
								Ray_t ray;
								ray.Init( pLocal->m_vecOrigin( ), pLocal->m_vecOrigin( ) - Vector( 0.0f, 0.0f, 1000.0f ) );

								CTraceFilterWorldAndPropsOnly filter;
								CGameTrace tr;
								Source::m_pEngineTrace->TraceRay( ray, 0x46004003u, &filter, &tr );
								if ( tr.fraction < 1.0f ) {
									AutoPeekPos = tr.endpos + Vector( 0.0f, 0.0f, 2.0f );
									g_Vars.globals.aStartPos = tr.endpos + Vector( 0.0f, 0.0f, 2.0f );
									
								}
							}
						} else {
							AutoPeekPos = Vector( 0.0f, 0.0f, 0.0f );
							g_Vars.globals.aStartPos = Vector( 0.0f, 0.0f, 0.0f );
						}
					}
					was_enabled = false;
				} else {
					was_enabled = true;
				}
			}

			movement->InPrediction( );

			if ( Source::m_pClientState->m_nChokedCommands( ) >= 14 ) {
				*bSendPacket = true;
			}

			movement->PostPrediction( );

			if ( cmd->buttons & IN_ATTACK
				 && weapon->m_iItemDefinitionIndex( ) != WEAPON_C4
				 && weaponInfo->m_iWeaponType >= WEAPONTYPE_KNIFE
				 && weaponInfo->m_iWeaponType <= WEAPONTYPE_MACHINEGUN
				 && pLocal->CanShoot( )
				 && ( weaponInfo->m_ucFullAuto /*|| ( prev_buttons & IN_ATTACK ) == 0 )*/ ) ) {
				lockedAngles = cmd->viewangles;
				g_tickbase_control.m_last_exploit_time = Source::m_pGlobalVars->realtime;

				LastShotTime = Source::m_pGlobalVars->tickcount;

				g_Vars.globals.m_flLastShotTime = Source::m_pGlobalVars->realtime;


				g_Vars.globals.WasShootingInChokeCycle = !( *bSendPacket );
				g_Vars.globals.WasShooting = true;

				if ( weaponInfo->m_iWeaponType != WEAPONTYPE_KNIFE )
					WasShootinPeek = true;

			} else {
				g_Vars.globals.WasShooting = false;
			}



			IGrenadePrediction::Get( )->Tick( cmd->buttons );

			g_Vars.globals.m_flPreviousDuckAmount = pLocal->m_flDuckAmount( );

			Engine::C_Resolver::Get( )->CorrectSnapshots( *bSendPacket );
		}
		prediction.End( );

		g_tickbase_control.handle_exploits( bSendPacket, cmd.Xor( ) );

		float move1 = std::clamp( cmd.Xor( )->forwardmove, -450.f, 450.f );
		float move2 = std::clamp( cmd.Xor( )->sidemove, -450.f, 450.f );
		float move3 = std::clamp( cmd.Xor( )->upmove, -320.f, 320.f );

		g_tickbase_control.m_local_data[ cmd.Xor( )->command_number % 150 ].m_move = Vector( move1, move2, move3 );


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

		//if ( g_TickbaseController.Using( ) ) {
		//	*bSendPacket = g_TickbaseController.m_tick_to_shift == 1; // Only send on the last shifted tick
		//	_cmd->buttons &= ~( IN_ATTACK | IN_ATTACK2 );
		//}

		auto pLocal = C_CSPlayer::GetLocalPlayer( );
		if ( !g_Vars.globals.HackIsReady || !pLocal || !Source::m_pEngine->IsInGame( ) ) {
			Engine::Prediction::Instance( ).Invalidate( );
			return oCreateMove( ft, _cmd );
		}

		return result;
	}

	void __vectorcall CL_Move( bool bFinalTick, float accumulated_extra_samples ) {
		g_tickbase_control.cl_move( bFinalTick, accumulated_extra_samples );
	}
}
