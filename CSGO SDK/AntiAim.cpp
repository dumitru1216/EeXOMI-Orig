#include "AntiAim.hpp"
#include "CVariables.hpp"
#include "Movement.hpp"
#include "Source.hpp"
#include "InputSys.hpp"
#include "player.hpp"
#include "CBaseHandle.hpp"
#include "weapon.hpp"
#include "LagCompensation.hpp"
#include "Autowall.h"
#include "SimulationContext.hpp"
#include "displacement.hpp"
#include "Render.hpp"
#include "TickbaseShift.hpp"

bool MoveJitter = false;
Vector JitterOrigin;
extern int ShotsSwitch;
extern bool HideRealAfterShot;

namespace Source {
	class C_AntiAimbot : public AntiAimbot {
	public:
		void Main( bool* bSendPacket, Encrypted_t<CUserCmd> cmd, bool ragebot ) override;
		void PrePrediction( bool* bSendPacket, Encrypted_t<CUserCmd> cmd ) override;
	private:
		virtual float GetAntiAimX( Encrypted_t<CVariables::ANTIAIM_STATE> settings );
		virtual float GetAntiAimY( Encrypted_t<CVariables::ANTIAIM_STATE> settings );

		virtual bool AutoDirection( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings );
		virtual bool RunAutoDirection( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings, bool only_desync );
		virtual void AtTarget( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings );
		virtual bool Edge( Encrypted_t<CUserCmd> cmd );

		virtual void Distort( Encrypted_t<CUserCmd> cmd );

		virtual bool IsEnabled( Encrypted_t<CUserCmd> cmd );

		bool m_bAutomaticDir = true;
		int m_iAutoDirection = false;
		float m_flDirection = 0.0f;
		float m_flLastDirection = 0.0f;
		float m_flLastDirectionTime = 0.0f;

#if 0
		bool m_bDoShift = true;
#endif

		bool m_bInvertJitter = false;
		bool m_bBreakLBY = false;
		bool m_bInverted = false;
		bool m_bFlipMove = false;

		int m_bRechoke = 0;
		int m_BalanceState = -1;
		bool m_bBalanceMove = false;
		bool m_bForceBreakLBY = false;
		bool m_bSwitch = false;

		float m_flSpin = 0.0f;

		/* just a basic sys */
		float m_flLowerBodyUpdateTime = 0.f;
		float m_flLowerBodyUpdateYaw = FLT_MAX;
	};

	void C_AntiAimbot::Distort( Encrypted_t<CUserCmd> cmd ) {
		auto local = C_CSPlayer::GetLocalPlayer( );
		if ( !local || local->IsDead( ) )
			return;

		if ( !g_Vars.antiaim.distort )
			return;

		bool bDoDistort = true;
		// if ( g_Vars.antiaim.distort_disable_fakewalk  ) // && g_Vars.globals.Fakewalking
		// 	bDoDistort = false;

		if ( g_Vars.antiaim.distort_disable_air && !( local->m_fFlags( ) & FL_ONGROUND ) )
			bDoDistort = false;

		static float flLastMoveTime = FLT_MAX;
		static float flLastMoveYaw = FLT_MAX;
		static bool bGenerate = true;
		static float flGenerated = 0.f;

		if ( local->m_PlayerAnimState( )->m_velocity > 0.1f && ( local->m_fFlags( ) & FL_ONGROUND ) /*&& !g_Vars.globals.Fakewalking*/ ) {
			flLastMoveTime = Source::m_pGlobalVars->realtime;
			flLastMoveYaw = local->m_flLowerBodyYawTarget( );

			if ( g_Vars.antiaim.distort_disable_run )
				bDoDistort = false;
		}

		if ( g_Vars.globals.manual_aa != -1 && !g_Vars.antiaim.distort_manual_aa )
			bDoDistort = false;

		if ( flLastMoveTime == FLT_MAX )
			return;

		if ( flLastMoveYaw == FLT_MAX )
			return;

		if ( !bDoDistort ) {
			bGenerate = true;
		}

		if ( bDoDistort ) {
			// don't distort for longer than this
			if ( fabs( Source::m_pGlobalVars->realtime - flLastMoveTime ) > g_Vars.antiaim.distort_max_time && g_Vars.antiaim.distort_max_time > 0.f ) {
				return;
			}

			if ( g_Vars.antiaim.distort_twist ) {
				float flDistortion = std::sin( ( Source::m_pGlobalVars->realtime * g_Vars.antiaim.distort_speed ) * 0.5f + 0.5f );

				cmd->viewangles.y += g_Vars.antiaim.distort_range * flDistortion;
				return;
			}

			if ( bGenerate ) {
				float flNormalised = std::remainderf( g_Vars.antiaim.distort_range, 360.f );

				flGenerated = RandomFloat( -flNormalised, flNormalised );
				bGenerate = false;
			}

			float flDelta = fabs( flLastMoveYaw - local->m_flLowerBodyYawTarget( ) );
			cmd->viewangles.y += flDelta + flGenerated;
		}
	}

	bool C_AntiAimbot::Edge( Encrypted_t<CUserCmd> cmd ) {
		C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
		if ( !local || local->IsDead( ) )
			return false;

		if ( local->m_MoveType( ) == 9 )
			return false;

		auto collision = local->m_Collision( );
		auto maxs = collision->m_vecMaxs + Vector( 32.0f, 32.0f, 0.0f );
		auto mins = collision->m_vecMins - Vector( 32.0f, 32.0f, 0.0f );

		auto start = local->m_vecOrigin( );
		start.z += ( ( ( 1.0f - local->m_flDuckAmount( ) ) * 18.0f ) + 38.0f );

		auto obb_center = ( mins + maxs ) * 0.5f;

		obb_center.z = 0.0f;

		Ray_t ray;
		ray.Init( start, start, mins, maxs );

		CTraceFilterWorldAndPropsOnly filter;

		CGameTrace trace;
		Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );
		if ( !trace.startsolid )
			return false;

		Vector best_plane;

		float best_fraction = 1.0f;
		float angle = 0.0f;
		do {
			Vector end = Vector( start.x + cosf( angle ) * 32.0f, start.y + sinf( angle ) * 32.0f,
								 start.z );

			ray.Init( start, end );
			Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );
			if ( best_fraction > trace.fraction ) {
				best_plane = trace.plane.normal;
				best_fraction = trace.fraction;
			}

			angle += DirectX::XM_2PI / 20.0f;
		} while ( angle <= DirectX::XM_2PI );
		if ( best_fraction == 1.0f || best_plane.z >= 0.1f )
			return false;

		Vector plane = Vector( -best_plane.x, -best_plane.y, best_plane.z );
		plane.Normalize( );

		Vector plane_dir = start + plane * 24.0f;
		plane_dir.z = start.z;

		if ( Source::m_pEngineTrace->GetPointContents( plane_dir, MASK_SOLID ) & MASK_SOLID ) {
			ray.Init( plane_dir, plane_dir + Vector( 0.0f, 0.0f, 16.0f ) );
			Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );
			if ( trace.fraction < 1.0f && !trace.startsolid && trace.plane.normal.z >= 0.7f ) {
				cmd->viewangles.yaw = std::remainderf( ToDegrees( atan2f( plane.y, plane.x ) ), 360.0f );
				return true;
			}
		}

		const float MAX_EDGE = 48.0f;

		Vector v66 = Vector( plane_dir.x - ( best_plane.y * MAX_EDGE ), plane_dir.y - ( best_plane.x * MAX_EDGE ), plane_dir.z );

		ray.Init( v66, plane_dir,
				  Vector( plane * -3.0f - Vector( 1.0f, 1.0f, 1.0f ) ),
				  Vector( plane * 3.0f + Vector( 1.0f, 1.0f, 1.0f ) )
		);

		Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

		float PlaneNormalZ_1 = 0.0f;
		if ( trace.startsolid )
			PlaneNormalZ_1 = 0.0f;
		else
			PlaneNormalZ_1 = trace.fraction;

		Vector v67 = Vector( plane_dir.x + ( best_plane.y * MAX_EDGE ), plane_dir.y + ( best_plane.x * MAX_EDGE ), plane_dir.z );

		ray.Init( v67, plane_dir,
				  Vector( plane * -3.0f - Vector( 1.0f, 1.0f, 1.0f ) ),
				  Vector( plane * 3.0f + Vector( 1.0f, 1.0f, 1.0f ) )
		);

		Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

		float v37 = trace.startsolid ? 0.0f : trace.fraction;
		if ( v37 == 0.0f && PlaneNormalZ_1 == 0.0f )
			return false;


		float a1 = std::remainderf( cmd->viewangles.yaw - ( v66 - start ).ToEulerAngles( ).yaw, 360.0f );
		float a2 = std::remainderf( cmd->viewangles.yaw - ( v67 - start ).ToEulerAngles( ).yaw, 360.0f );

		if ( v37 < PlaneNormalZ_1 ) {
			cmd->viewangles.yaw += ( a1 <= 0.f ) ? -90.0f : 90.0f;
		} else {
			cmd->viewangles.yaw += ( a2 > 0.f ) ? 90.0f : -90.0f;
		}

#if 0

		constexpr float MAX_EDGE = 64.0f;
		float fraction = 0.0f;
		for ( float frac = -64.0f; frac <= 64.0f; frac += 4.0f ) {
			Vector point = Vector( plane_dir.x + ( best_plane.y * frac ), plane_dir.y + ( best_plane.x * frac ), plane_dir.z );
			if ( Source::m_pEngineTrace->GetPointContents_WorldOnly( point, MASK_SOLID ) ) {
				fraction += ( frac >= 0.f ) ? 4.0f : -4.0f;
			}
		}

		if ( fraction == 0.0f )
			return false;

		cmd->viewangles.yaw = std::remainderf( ToDegrees( atan2f( best_plane.y, best_plane.x ) ), 360.0f );

		if ( fraction > 0.0f ) {
			cmd->viewangles.yaw += 90.0f;
		} else {
			cmd->viewangles.yaw -= 90.0f;
		}
#endif

		return true;
	}

	bool C_AntiAimbot::IsEnabled( Encrypted_t<CUserCmd> cmd ) {
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );
		if ( !LocalPlayer || LocalPlayer->IsDead( ) || g_Vars.globals.HideShots )
			return false;

		if ( cmd->buttons & IN_USE )
			return false;

		if ( LocalPlayer->m_MoveType( ) == MOVETYPE_NOCLIP )
			return false;

		if ( g_Vars.antiaim.on_freeze_period ) {
			static auto g_GameRules = *( uintptr_t** )( Engine::Displacement.Data.m_GameRules );
			if ( *( bool* )( *( uintptr_t* )g_GameRules + 0x20 ) )
				return false;
		}

		C_WeaponCSBaseGun* Weapon = ( C_WeaponCSBaseGun* )LocalPlayer->m_hActiveWeapon( ).Get( );

		if ( !Weapon )
			return false;

		auto WeaponInfo = Weapon->GetCSWeaponData( );
		if ( !WeaponInfo.IsValid( ) )
			return false;

		if ( WeaponInfo->m_iWeaponType == WEAPONTYPE_GRENADE ) {
			if ( g_Vars.antiaim.on_grenade )
				return false;

			if ( !Weapon->m_bPinPulled( ) || ( cmd->buttons & ( IN_ATTACK | IN_ATTACK2 ) ) ) {
				float throwTime = Weapon->m_fThrowTime( );
				if ( throwTime > 0.f )
					return false;
			}
		} else {
			// xref: kaaba
			if ( ( WeaponInfo->m_iWeaponType == WEAPONTYPE_KNIFE && cmd->buttons & ( IN_ATTACK | IN_ATTACK2 ) ) || cmd->buttons & IN_ATTACK ) {
				if ( LocalPlayer->CanShoot( ) )
					return false;
			}
		}

		if ( WeaponInfo->m_iWeaponType == WEAPONTYPE_KNIFE && Weapon->m_iItemDefinitionIndex( ) != WEAPON_ZEUS && g_Vars.antiaim.on_knife ) {
			return false;
		}

		if ( g_Vars.antiaim.on_dormant ) {
			bool found_player = false;
			for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
				auto player = C_CSPlayer::GetPlayerByIndex( i );
				if ( !player || player->IsDead( ) || player->IsDormant( ) || player == LocalPlayer )
					continue;

				if ( !LocalPlayer->IsTeammate( player ) ) {
					found_player = true;
					break;
				}
			}

			if ( !found_player )
				return false;
		}

		if ( g_Vars.antiaim.on_manual_shot ) {
			if ( ( cmd->buttons & IN_ATTACK ) ) {
				return false;
			}
		}

		if ( LocalPlayer->m_MoveType( ) == MOVETYPE_LADDER )
			if ( g_Vars.antiaim.on_ladder )
				return false;

		return true;
	}

	Encrypted_t<AntiAimbot> AntiAimbot::Get( ) {
		static C_AntiAimbot instance;
		return &instance;
	}

	void C_AntiAimbot::Main( bool* bSendPacket, Encrypted_t<CUserCmd> cmd, bool ragebot ) {
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

		if ( !LocalPlayer || LocalPlayer->IsDead( ) )
			return;

		auto animState = LocalPlayer->m_PlayerAnimState( );
		if ( !animState )
			return;

		if ( !g_Vars.antiaim.enabled || g_Vars.globals.WasShootingInChokeCycle || ( g_Vars.globals.m_iServerType == 1 && g_Vars.globals.m_iGameMode == 1 ) )
			return;

		Encrypted_t<CVariables::ANTIAIM_STATE> settings( &g_Vars.antiaim_stand );

		if ( !( LocalPlayer->m_fFlags( ) & FL_ONGROUND ) )
			settings = &g_Vars.antiaim_air;
		else if ( animState->m_velocity > 2.0f )
			settings = &g_Vars.antiaim_move;

		C_WeaponCSBaseGun* Weapon = ( C_WeaponCSBaseGun* )LocalPlayer->m_hActiveWeapon( ).Get( );

		if ( !Weapon )
			return;

		auto WeaponInfo = Weapon->GetCSWeaponData( );
		if ( !WeaponInfo.IsValid( ) )
			return;

		if ( !IsEnabled( cmd ) )
			return;

		if ( LocalPlayer->m_MoveType( ) == MOVETYPE_LADDER ) {
			auto eye_pos = LocalPlayer->GetEyePosition( );

			CTraceFilterWorldAndPropsOnly filter;
			CGameTrace tr;
			Ray_t ray;
			float angle = 0.0f;
			while ( true ) {
				float cosa, sina;
				DirectX::XMScalarSinCos( &cosa, &sina, angle );

				Vector pos;
				pos.x = ( cosa * 32.0f ) + eye_pos.x;
				pos.y = ( sina * 32.0f ) + eye_pos.y;
				pos.z = eye_pos.z;

				ray.Init( eye_pos, pos,
						  Vector( -1.0f, -1.0f, -4.0f ),
						  Vector( 1.0f, 1.0f, 4.0f ) );
				Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &tr );
				if ( tr.fraction < 1.0f )
					break;

				angle += DirectX::XM_PIDIV2;
				if ( angle >= DirectX::XM_2PI ) {
					return;
				}
			}

			float v23 = atan2( tr.plane.normal.x, std::fabsf( tr.plane.normal.y ) );
			float v24 = RAD2DEG( v23 ) + 90.0f;
			cmd->viewangles.pitch = 89.0f;
			if ( v24 <= 180.0f ) {
				if ( v24 < -180.0f ) {
					v24 = v24 + 360.0f;
				}
				cmd->viewangles.yaw = v24;
			} else {
				cmd->viewangles.yaw = v24 - 360.0f;
			}

			if ( cmd->buttons & IN_BACK ) {
				cmd->buttons |= IN_FORWARD;
				cmd->buttons &= ~IN_BACK;
			} else  if ( cmd->buttons & IN_FORWARD ) {
				cmd->buttons |= IN_BACK;
				cmd->buttons &= ~IN_FORWARD;
			}

			return;
		}

		this->m_bAutomaticDir = false;

		if ( settings->base_yaw == 1 ) {
			AtTarget( cmd, settings );
		} else if ( animState->m_velocity > 2.0f && settings->base_yaw == 2 ) {
			cmd->viewangles.yaw = ToDegrees( std::atan2f( animState->m_vecVelocity.y, animState->m_vecVelocity.x ) );
		}

		float yaw = GetAntiAimY( settings );
		float pitch = GetAntiAimX( settings );
		if ( pitch != FLT_MAX )
			cmd->viewangles.x = pitch;

		/*
		g_Vars.globals.manual_aa = ( m_iAutoDirection > 0 ) ? 2 : 0;
		*/


		//cmd->viewangles.Normalize( );

#if 0
		if ( settings->spin ) {
			float spin_speed = settings->spin_speed * 0.01f;
			if ( settings->spin_switch ) {
				float range = settings->spin_range;
				if ( spin_speed < 0.f )
					spin_speed = -spin_speed;

				if ( m_bSwitch ) {
					m_flSpin += ( spin_speed * range );
					if ( m_flSpin > range ) {
						m_bSwitch = false;
						m_flSpin = range;
					}
				} else {
					m_flSpin -= ( spin_speed * range );
					if ( m_flSpin < -range ) {
						m_bSwitch = true;
						m_flSpin = -range;
					}
				}
			} else {
				if ( m_flSpin > settings->spin_range )
					m_flSpin = -settings->spin_range;
				else if ( m_flSpin < -settings->spin_range )
					m_flSpin = settings->spin_range;

				m_flSpin += spin_speed * settings->spin_range;
			}

			cmd->viewangles.yaw = std::remainderf( cmd->viewangles.yaw, 360.0f );
			cmd->viewangles.yaw += m_flSpin;
		}
		if ( settings->jitter ) {
			RandomSeed( ( cmd->command_number % 255 ) + 1 );
			cmd->viewangles.yaw += RandomFloat( -settings->jitter_range, settings->jitter_range );
		}
#endif

		//cmd->viewangles.Normalize( );

		float yaw_backup = cmd->viewangles.yaw;
		if ( g_Vars.antiaim.manual && g_Vars.globals.MouseOverrideEnabled ) {
			RunAutoDirection( cmd, settings, true );
			cmd->viewangles.yaw = std::remainderf( g_Vars.globals.MouseOverrideYaw + 180.0f, 360.0f );
		} else {
			if ( yaw != FLT_MAX ) {
				cmd->viewangles.y += yaw;
			}

			RunAutoDirection( cmd, settings, false );
		}

		/* run this shit */
		Distort( cmd );

		// gheto
		const float CSGO_ANIM_LOWER_REALIGN_DELAY = 1.1f;

		// bodypred: g_Vars.globals.m_flAnimTime + ( CSGO_ANIM_LOWER_REALIGN_DELAY * 0.2f );
		float m_flBodyPred = LocalPlayer->m_flAnimationTime( ) + ( CSGO_ANIM_LOWER_REALIGN_DELAY * 0.2f );

		float lbyUpdate = Source::Movement::Get( )->GetLBYUpdateTime( );

		// lby upd
		if ( g_Vars.antiaim.lbypred ) {
			if ( !Source::m_pClientState->m_nChokedCommands( ) &&
				 TICKS_TO_TIME( LocalPlayer->m_nTickBase( ) ) >= lbyUpdate && ( LocalPlayer->m_fFlags( ) & FL_ONGROUND ) ) {
				cmd->viewangles.y = yaw + 119.f;

				m_flLowerBodyUpdateYaw = LocalPlayer->m_flLowerBodyYawTarget( );
			}
		}
#if 0
		if ( g_Vars.globals.TickbaseShift > 0 || ( *bSendPacket && g_Vars.antiaim.shift && ( settings->shift_pitch || settings->shift_yaw ) ) ) {
			Shift( cmd, settings );
		}
#endif
	}

	void C_AntiAimbot::PrePrediction( bool* bSendPacket, Encrypted_t<CUserCmd> cmd ) {
		bool prev_state = MoveJitter;
		MoveJitter = false;

		if ( !g_Vars.antiaim.enabled )
			return;

		C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
		if ( !local || local->IsDead( ) )
			return;

		if ( cmd->sidemove != 0.0f || cmd->forwardmove != 0.0f || !( local->m_fFlags( ) & FL_ONGROUND ) || cmd->buttons & IN_JUMP || local->m_vecVelocity( ).Length2D( ) >= 4.5f )
			return;

		auto animState = local->m_PlayerAnimState( );
		if ( !animState )
			return;

		Encrypted_t<CVariables::ANTIAIM_STATE> settings( &g_Vars.antiaim_stand );

#if 0
		if ( !settings->desync )
			return;
#endif 

		if ( !IsEnabled( cmd ) )
			return;
	}

	float C_AntiAimbot::GetAntiAimX( Encrypted_t<CVariables::ANTIAIM_STATE> settings ) {
		switch ( settings->pitch ) {
			case 1: // down
				return 89.f;
			case 2: // up 
				return -89.f;
			case 3: // zero
				return 0.f;
			default:
				return FLT_MAX;
				break;
		}
	}

	float C_AntiAimbot::GetAntiAimY( Encrypted_t<CVariables::ANTIAIM_STATE> settings ) {
		auto local = C_CSPlayer::GetLocalPlayer( );
		if ( !local || local->IsDead( ) )
			return 0.f;

		if ( !g_Vars.antiaim.manual ) {
			switch ( settings->yaw ) {
				case 0: // forward
					return 0.f; break;
				case 1: // backward
					return 180.f; break;
				default:
					break;
			}
		}

		switch ( g_Vars.globals.manual_aa ) {
			case 0:
				return 90.f;
				break;
			case 1:
				return 180.f;
				break;
			case 2:
				return -90.f;
				break;
		}

		return 180.0f;
	}

	bool C_AntiAimbot::AutoDirection( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings ) {
		struct edgy_sort {
			C_CSPlayer* pl;
			float dst;
			Vector eye;

			bool operator<( const edgy_sort& a ) const {
				return dst < a.dst;
			}
		};

		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

		if ( !LocalPlayer || LocalPlayer->IsDead( ) )
			return false;

		auto pWeapon = reinterpret_cast< C_WeaponCSBaseGun* >( LocalPlayer->m_hActiveWeapon( ).Get( ) );
		if ( !pWeapon )
			return false;

		auto pWeaponData = pWeapon->GetCSWeaponData( );
		if ( !pWeaponData.IsValid( ) )
			return false;

		float maxSpeed = 250.0f;
		if ( pWeapon ) {
			if ( pWeaponData.IsValid( ) ) {
				if ( pWeapon->m_weaponMode( ) == 0 )
					maxSpeed = pWeaponData->m_flMaxSpeed;
				else
					maxSpeed = pWeaponData->m_flMaxSpeed2;
			}
		}

		auto viewHeight = LocalPlayer->GetEyePosition( ) - LocalPlayer->m_vecOrigin( );
		Vector eyePos = ( settings->autodirection_ignore_duck ) ? LocalPlayer->m_vecOrigin( ) + Vector( 0.0f, 0.0f, 58.0f ) : LocalPlayer->GetEyePosition( );

		std::vector< edgy_sort > players_sorted;
		for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
			auto player = C_CSPlayer::GetPlayerByIndex( i );
			if ( !player || player->IsDormant( ) || player->IsDead( ) )
				continue;

			bool is_team = player->IsTeammate( LocalPlayer );
			if ( is_team /*&& !Engine::LagCompensation::Get( )->BacktrackTeam( ) */ )
				continue;

			auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
			if ( !lag_data.IsValid( ) )
				continue;

			if ( player->IsReloading( ) )
				continue;

			auto wpn = ( C_WeaponCSBaseGun* )player->m_hActiveWeapon( ).Get( );
			if ( !wpn || wpn->m_iClip1( ) <= 0 )
				continue;

			auto& pl = players_sorted.emplace_back( );
			pl.pl = player;
			pl.eye = lag_data->GetExtrapolatedPosition( player ) + player->m_vecViewOffset( );

			Vector dir = pl.eye - eyePos;
			pl.dst = dir.Normalize( );
		}

		if ( players_sorted.empty( ) ) {
			if ( settings->autodirection && Edge( cmd ) ) {
				cmd->viewangles.y = std::remainderf( cmd->viewangles.y, 360.0f );
				return true;
			}

			return false;
		}

		SimulationContext data;
		data.InitSimulationContext( LocalPlayer );
		for ( int i = 0; i < settings->extrapolation_ticks; ++i )
			data.RebuildGameMovement( cmd.Xor( ) );

		eyePos = ( settings->autodirection_ignore_duck ) ? data.m_vecOrigin + Vector( 0.0f, 0.0f, 58.0f ) : data.m_vecOrigin + viewHeight;
		std::sort( players_sorted.begin( ), players_sorted.end( ), std::less< edgy_sort >( ) );

		bool found_valid = false;

		float finalDistance = 0.0f;

		Vector direction;
		for ( auto& sorted : players_sorted ) {
			auto player = sorted.pl;
			if ( !player )
				continue;

			auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
			if ( !lag_data.IsValid( ) )
				continue;

			auto weapon = ( C_WeaponCSBaseGun* )player->m_hActiveWeapon( ).Get( );
			if ( !weapon )
				continue;

			auto weapon_data = weapon->GetCSWeaponData( );
			if ( !weapon_data.IsValid( ) )
				continue;

			auto FREESTAND_MAX = settings->autodirection_range;

			direction = sorted.eye - eyePos;
			direction.Normalize( );

			CTraceFilterWorldAndPropsOnly filter;

			CGameTrace tr;
			Ray_t ray;

			Vector endPos = Vector(
				( direction.y * FREESTAND_MAX ) + eyePos.x,
				( direction.x * -FREESTAND_MAX ) + eyePos.y,
				eyePos.z );

			ray.Init( eyePos, endPos );
			Source::m_pEngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &tr );

			float fraction_01 = tr.fraction * FREESTAND_MAX;

			endPos = Vector(
				eyePos.x + ( direction.y * -FREESTAND_MAX ),
				eyePos.y + ( direction.x * FREESTAND_MAX ),
				eyePos.z );
			ray.Init( eyePos, endPos );
			Source::m_pEngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &tr );

			float fraction_02 = tr.fraction * -FREESTAND_MAX;

			endPos = Vector(
				sorted.eye.x + ( direction.y * FREESTAND_MAX ),
				sorted.eye.y + ( direction.x * -FREESTAND_MAX ),
				sorted.eye.z );
			ray.Init( sorted.eye, endPos );
			Source::m_pEngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &tr );

			float fraction_03 = tr.fraction * FREESTAND_MAX;

			endPos = Vector(
				sorted.eye.x + ( direction.y * -FREESTAND_MAX ),
				sorted.eye.y + ( -direction.x * -FREESTAND_MAX ),
				sorted.eye.z );
			ray.Init( sorted.eye, endPos );
			Source::m_pEngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &tr );

			float fraction_04 = tr.fraction * -FREESTAND_MAX;

			float enemyOffset = -FREESTAND_MAX;
			float localOffset = -FREESTAND_MAX;

			do {
				if ( localOffset <= fraction_01 && fraction_02 <= localOffset ) {
					enemyOffset = -FREESTAND_MAX;
					fraction_03 = fraction_03;
					do {
						if ( enemyOffset <= fraction_03 && fraction_04 <= enemyOffset ) {
							CTraceFilterWorldAndPropsOnly filter;

							Autowall::C_FireBulletData data;
							data.m_bPenetration = true;
							data.m_TargetPlayer = player;
							data.m_Player = LocalPlayer;
							data.m_WeaponData = weapon_data.Xor( );
							data.m_Weapon = weapon;
							data.m_Filter = &filter;
							data.m_bShouldIgnoreDistance = true;

							data.m_vecStart.x = ( enemyOffset * direction.y ) + sorted.eye.x;
							data.m_vecStart.y = ( enemyOffset * -direction.x ) + sorted.eye.y;
							data.m_vecStart.z = sorted.eye.z;

							data.m_vecDirection.x = ( ( localOffset * direction.y ) + eyePos.x ) - data.m_vecStart.x;
							data.m_vecDirection.y = ( ( localOffset * -direction.x ) + eyePos.y ) - data.m_vecStart.y;
							data.m_vecDirection.z = eyePos.z - sorted.eye.z;

							data.m_flPenetrationDistance = data.m_vecDirection.Normalize( );
							auto damage = Autowall::FireBullets( &data );
							if ( damage >= 1.0f ) {
								found_valid = true;
								finalDistance += localOffset;
							}
						}
						enemyOffset = enemyOffset + FREESTAND_MAX / 2.0f;
					} while ( enemyOffset <= FREESTAND_MAX );
					fraction_01 = fraction_01;
				}
				localOffset = localOffset + FREESTAND_MAX / 2.0f;
			} while ( localOffset <= FREESTAND_MAX );

			if ( found_valid )
				break;
		}

		if ( !found_valid || finalDistance == 0.0f ) {
			if ( settings->autodirection && !found_valid && Edge( cmd ) ) {
				cmd->viewangles.y = std::remainderf( cmd->viewangles.y, 360.0f );
				return true;
			}

			return false;
		}

		float angle = RAD2DEG( atan2( direction.y, direction.x ) );

		m_flDirection = finalDistance;

		if ( settings->autodirection ) {
			if ( finalDistance <= 0.0f )
				cmd->viewangles.y = angle - 90.0f;
			else
				cmd->viewangles.y = angle + 90.0f;
			cmd->viewangles.y = std::remainderf( cmd->viewangles.y, 360.0f );
		}

		return true;
	}

	bool C_AntiAimbot::RunAutoDirection( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings, bool only_desync ) {
		if ( g_Vars.antiaim.autodirection_override.enabled )
			return false;

		if ( settings->autodirection ) {
			AutoDirection( cmd, settings );
		}

		return false;
	}

	void C_AntiAimbot::AtTarget( Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings ) {
		auto local = C_CSPlayer::GetLocalPlayer( );
		auto found = false;
		float bestFov = std::numeric_limits< float >::max( );

		Vector bestOrigin;
		for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; i++ ) {
			auto player = C_CSPlayer::GetPlayerByIndex( i );
			if ( !player || player->IsDormant( ) || player->IsDead( ) )
				continue;

			bool is_team = player->IsTeammate( local );
			if ( is_team )
				continue;

			auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
			if ( !lag_data.IsValid( ) )
				continue;

			Vector origin = lag_data->GetExtrapolatedPosition( player );

			auto AngleDistance = [ & ]( QAngle& angles, const Vector& start, const Vector& end ) -> float {
				auto direction = end - start;
				auto aimAngles = direction.ToEulerAngles( );
				auto delta = aimAngles - angles;
				delta.Normalize( );

				return sqrtf( delta.x * delta.x + delta.y * delta.y );
				};

			auto studio_model = Source::m_pModelInfo->GetStudiomodel( player->GetModel( ) );
			auto hitboxSet = studio_model->pHitboxSet( player->m_nHitboxSet( ) );

			auto GetHitboxPos = [ & ]( int id ) -> Vector {
				if ( studio_model ) {
					auto hitbox = hitboxSet->pHitbox( id );
					if ( hitbox ) {
						auto
							min = Vector{},
							max = Vector{};

						return ( ( hitbox->bbmin + hitbox->bbmax ) * 0.5f ).Transform( player->m_CachedBoneData( ).m_Memory.m_pMemory[ hitbox->bone ] );
					}
				}
				};

			Vector hitboxPos = GetHitboxPos( HITBOX_PELVIS );
			float Fov = AngleDistance( cmd->viewangles, local->GetEyePosition( ), hitboxPos );

			if ( Fov < bestFov ) {
				bestOrigin = origin;
				bestFov = Fov;
				found = true;
			}
		}

		if ( found ) {
			const float yaw = Math::CalcAngle( local->m_vecOrigin( ), bestOrigin ).y + 180.f;
			cmd->viewangles.yaw = std::remainderf( yaw, 360.0f );
		}
	}
}
