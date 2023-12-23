#include "source.hpp"

#include "Ragebot.hpp"
#include "LagCompensation.hpp"
#include "Movement.hpp"
#include "Prediction.hpp"
#include "Autowall.h"
#include "Fakelag.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "CBaseHandle.hpp"
#include "InputSys.hpp"
#include "Render.hpp"
#include "RoundFireBulletsStore.hpp"
#include "Utils\threading.h"
#include <algorithm>
#include <atomic>
#include <thread>
#include "RayTracer.h"
#include "Profiler.hpp"
#include "Displacement.hpp"
#include "Resolver.hpp"
#include "EventLogger.hpp"
#include "Resolver.hpp"
#include "TickbaseShift.hpp"

#include "CChams.hpp"

#include <sstream>


// #define DEBUG_REPREDICT
#ifdef DEBUG_REPREDICT
std::map<int, std::tuple<Vector, Vector>> predicted_origins;
#endif

extern int LastShotTime;
extern CUserCmd* previous_cmd;
#ifdef DevelopMode
// #define DEBUG_HITCHANCE
// #define OPTIMIZATION_CHECKS
// #define DEBUG_ONSHOT_AA
#endif

// TODO: 
// Refactoring
// Rework exploits

// placed here for debug
Engine::C_LagRecord extrapolated;
C_CSPlayer* extrapolatedPlayer = nullptr;

enum OverrideConditions {
	OnShot,
	Running,
	Walking,
	InAir,
	Standing,
	Backward,
	Sideways,
	Unresolved,
	InDuck,
	//Lethal,
};

typedef __declspec( align( 16 ) ) union {
	float f[ 4 ];
	__m128 v;
} m128;

__forceinline __m128 sqrt_ps( const __m128 squared ) {
	return _mm_sqrt_ps( squared );
}

struct BoundingBox {
	Vector min, max;
	int idx;

	BoundingBox( void ) { };
	BoundingBox( const Vector& min, const Vector& max, int idx ) :
		min( min ), max( max ), idx( idx ) { };
};

struct CapsuleHitbox {
	CapsuleHitbox( ) = default;
	CapsuleHitbox( const Vector& mins, const Vector& maxs, const float radius, int idx )
		: m_mins( mins ), m_maxs( maxs ), m_radius( radius ), m_idx( idx ) { }

	Vector m_mins;
	Vector m_maxs;
	float m_radius;
	int m_idx;
};
#define SMALL_NUM   0.00000001 // anything that avoids division overflow
#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
#define norm(v)    sqrt(dot(v,v))  // norm = length of  vector

float dist_segment_to_segment( Vector s1, Vector s2, Vector k1, Vector k2 ) {
	Vector   u = s2 - s1;
	Vector   v = k2 - k1;
	Vector   w = s1 - k1;
	float    a = dot( u, u );
	float    b = dot( u, v );
	float    c = dot( v, v );
	float    d = dot( u, w );
	float    e = dot( v, w );
	float    D = a * c - b * b;
	float    sc, sN, sD = D;
	float    tc, tN, tD = D;

	if ( D < SMALL_NUM ) {
		sN = 0.0;
		sD = 1.0;
		tN = e;
		tD = c;
	} else {
		sN = ( b * e - c * d );
		tN = ( a * e - b * d );
		if ( sN < 0.0 ) {
			sN = 0.0;
			tN = e;
			tD = c;
		} else if ( sN > sD ) {
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if ( tN < 0.0 ) {
		tN = 0.0;

		if ( -d < 0.0 )
			sN = 0.0;
		else if ( -d > a )
			sN = sD;
		else {
			sN = -d;
			sD = a;
		}
	} else if ( tN > tD ) {
		tN = tD;

		if ( ( -d + b ) < 0.0 )
			sN = 0;
		else if ( ( -d + b ) > a )
			sN = sD;
		else {
			sN = ( -d + b );
			sD = a;
		}
	}

	sc = ( abs( sN ) < SMALL_NUM ? 0.0 : sN / sD );
	tc = ( abs( tN ) < SMALL_NUM ? 0.0 : tN / tD );

	Vector  dP = w + ( u * sc ) - ( v * tc );

	return norm( dP );
}

auto segment_to_segment = [ ]( const Vector& start, const Vector& end, const Vector& min, const Vector& max, float radius ) {
	auto dist = dist_segment_to_segment( start, end, min, max );

	return dist < radius;
	};

void get_edge( Vector& end, const Vector& start, float range, C_CSPlayer* player ) {
	Ray_t ray;
	if ( range > 0.0f )
		ray.Init( start, end, Vector( -range, -range, -range ), Vector( range, range, range ) );
	else
		ray.Init( start, end );

	CTraceFilter filter;
	filter.pSkip = player;

	CGameTrace trace;
	Source::m_pEngineTrace->TraceRay( ray, 0x46004003u, &filter, &trace );
	if ( trace.fraction <= 0.99f ) {
		end = start + ( ( end - start ) * trace.fraction );
	}
};

int convert_hitbox_to_hitgroup( int hitbox ) {
	switch ( hitbox ) {
		case HITBOX_HEAD:
		case HITBOX_NECK:
			return Hitgroup_Head;
		case HITBOX_UPPER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_LOWER_CHEST:
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_RIGHT_UPPER_ARM:
			return Hitgroup_Chest;
		case HITBOX_PELVIS:
		case HITBOX_LEFT_THIGH:
		case HITBOX_RIGHT_THIGH:
		case HITBOX_STOMACH:
			return Hitgroup_Stomach;
		case HITBOX_LEFT_CALF:
		case HITBOX_LEFT_FOOT:
			return Hitgroup_LeftLeg;
		case HITBOX_RIGHT_CALF:
		case HITBOX_RIGHT_FOOT:
			return Hitgroup_RightLeg;
		case HITBOX_LEFT_FOREARM:
		case HITBOX_LEFT_HAND:
			return Hitgroup_LeftArm;
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_RIGHT_HAND:
			return Hitgroup_RightArm;
		default:
			return Hitgroup_Stomach;
	}
}

#ifdef AUTOWALL_CALLS
int AutowallCalls = 0;
#endif

namespace Source {
	bool EyeCompare( const Vector& a, const Vector& b ) {
		auto round_eye = [ ]( const Vector& vec ) {
			return Vector(
				std::roundf( vec.x * 1000.0f ) / 1000.0f,
				std::roundf( vec.y * 1000.0f ) / 1000.0f,
				std::roundf( vec.z * 1000.0f ) / 1000.0f
			);
			};

		return round_eye( a ) == round_eye( b );
	};

	struct C_AimTarget;
	struct C_AimPoint;

	struct C_AimPoint {
		Vector point;

		float damage = 0.0f;
		float hitchance = 0.0f;

		int health_ratio = 0;
		int hitbox_idx = 0;
		int hitgroup = 0;

		bool did_penetrate = false;
		bool is_head = false;
		bool is_body = false;
		bool is_safe = false;
		bool is_static_safe = false;
		bool is_should_baim = false;
		bool is_should_headaim = false;

		C_AimTarget* target = nullptr;
		Engine::C_LagRecord* record = nullptr;
	};

	struct C_AimTarget {
		std::vector<C_AimPoint> points;
		std::vector<BoundingBox> obb;
		std::vector<CapsuleHitbox> capsules;
		Engine::C_BaseLagRecord backup;
		Engine::C_LagRecord* record = nullptr;
		C_CSPlayer* player = nullptr;
		bool override_hitscan = false;
		bool prefer_head = false;
		bool prefer_body = false;
		bool prefer_safe = false;
		int side = 0;
		int type = 0;
	};

	struct C_PointsArray {
		C_AimPoint* points = nullptr;
		int pointsCount = 0;
	};

	struct C_HitchanceData {
		Vector direction;
		Vector end;
		bool hit = false;
		bool damageIsAccurate = false;
	};

	struct C_HitchanceArray {
		C_AimPoint* point;
		C_HitchanceData* data;
		int dataCount;
	};

	// hitchance

#ifdef DEBUG_HITCHANCE
	static int g_ClipTrace = 0;
	static int g_Intersection = 0;
	static bool g_PerfectAccuracy = false;
#endif

	struct RagebotData {
		// spread cone
		float m_flSpread;
		float m_flInaccuracy;

		Encrypted_t<CUserCmd> m_pCmd = nullptr;
		C_CSPlayer* m_pLocal = nullptr;
		C_WeaponCSBaseGun* m_pWeapon = nullptr;
		Encrypted_t<CCSWeaponInfo>  m_pWeaponInfo = nullptr;
		bool* m_pSendPacket = nullptr;

		Vector m_vecEyePos;

		// last entity iterated in list
		int m_nIterativeTarget = 0;

		// failed hitchance this tick
		bool m_bFailedHitchance = false;

		// damage > 0 = stop
		bool m_bPrepareAim = false;

		bool m_bPredictedScope = false;

		int m_iChokedCommands = -1;
		int m_iDelay = 0;

		int m_iDelayTicks = 0;
		int m_iDelayTicksStored = 0;
		CVariables::RAGE* rbot = nullptr;

		// delay shot
		Vector m_PeekingPosition;
		float m_flLastPeekTime;

		// hitchance
		static std::vector<std::tuple<float, float, float>> precomputed_seeds;
	};

	static RagebotData _rage_data;

	class C_Ragebot : public Ragebot {
	public:
		void StripAttack( Encrypted_t<CUserCmd> cmd );
		// only sanity checks, etc.
		virtual bool Run( Encrypted_t<CUserCmd> cmd, C_CSPlayer* local, bool* sendPacket );

		virtual bool GetBoxOption( mstudiobbox_t* hitbox, mstudiohitboxset_t* hitboxSet, float& ps, bool override_hitscan ) {
			if ( !hitbox )
				return false;

			if ( !hitboxSet )
				return false;

			if ( hitboxSet->pHitbox( HITBOX_PELVIS ) == hitbox ) {
				ps = rageData->rbot->body_ps;
				return override_hitscan ? rageData->rbot->bt_pelvis_hitbox : rageData->rbot->pelvis_hitbox;
			}

			if ( hitboxSet->pHitbox( HITBOX_RIGHT_FOOT ) == hitbox || hitboxSet->pHitbox( HITBOX_LEFT_FOOT ) == hitbox ) {
				ps = 50.0f;
				return override_hitscan ? rageData->rbot->bt_feets_hitbox : rageData->rbot->feets_hitbox;
			}

			switch ( hitbox->group ) {
				case Hitgroup_Head:
					ps = rageData->rbot->head_ps;
					return override_hitscan ? rageData->rbot->bt_head_hitbox : rageData->rbot->head_hitbox;
					break;
				case Hitgroup_Gear: // just neck
					ps = rageData->rbot->body_ps;
					return  override_hitscan ? rageData->rbot->bt_neck_hitbox : rageData->rbot->neck_hitbox;
					break;
				case Hitgroup_Chest:
					ps = rageData->rbot->body_ps;
					return  override_hitscan ? rageData->rbot->bt_chest_hitbox : rageData->rbot->chest_hitbox;
					break;
				case Hitgroup_Stomach:
					ps = rageData->rbot->body_ps;
					return  override_hitscan ? rageData->rbot->bt_stomach_hitbox : rageData->rbot->stomach_hitbox;
					break;
				case Hitgroup_RightLeg:
				case Hitgroup_LeftLeg:
					ps = 50.0f;
					return  override_hitscan ? rageData->rbot->bt_legs_hitbox : rageData->rbot->legs_hitbox;
					break;
				case Hitgroup_RightArm:
				case Hitgroup_LeftArm:
					ps = 50.0f;
					return override_hitscan ? rageData->rbot->bt_arms_hitbox : rageData->rbot->arms_hitbox;
					break;
				default:
					return false;
					break;
			}
		};

		// should override condition
		virtual bool OverrideHitscan( C_CSPlayer* player, Engine::C_LagRecord* record, int type );

		// return true if rage enabled
		virtual bool SetupRageOptions( );

		virtual void Multipoint( C_CSPlayer* player, Engine::C_LagRecord* record, int side, std::vector<Vector>& points, mstudiobbox_t* hitbox, mstudiohitboxset_t* hitboxSet, float ps );
	public:
		C_Ragebot( ) : rageData( &_rage_data ) { };
		virtual ~C_Ragebot( ) { };

	private:
		// run aimbot itself
		virtual bool RunInternal( );

		// aim at point
		virtual bool AimAtPoint( C_AimPoint* best_point );

		// get theoretical hit chance
		virtual bool Hitchance( C_AimPoint* point, const Vector& start, float chance );
		virtual bool AccuracyBoost( C_AimPoint* point, const Vector& start, float chance );

		__forceinline bool IsPointAccurate( C_AimPoint* point, const Vector& start );

		static void HitchanceRaysMT( C_HitchanceArray* _data );
		void HitchanceRaysInternal( C_HitchanceArray* _data );
		void AutowallHitchanceRay( C_HitchanceData* _data );

		virtual void AddPoint( C_CSPlayer* player, Engine::C_LagRecord* record, int side, std::vector<Vector>& points, const Vector& point, mstudiobbox_t* hitbox, mstudiohitboxset_t* hitboxSet, bool isMultipoint );

		virtual bool RunHitscan( );

		static void ScanPointArrayMT( void* _data );
		void ScanPointArrayInternal( C_PointsArray* _data );
		void ScanPoint( C_AimPoint* _data );

		// sort records, choose best and calculate damage
		// return true if target is valid
		__forceinline int GeneratePoints( C_CSPlayer* player, std::vector<C_AimTarget>& aim_targets, std::vector<C_AimPoint>& aim_points );
		virtual Engine::C_LagRecord* GetBestLagRecord( C_CSPlayer* player );

		virtual bool IsRecordValid( C_CSPlayer* player, Engine::C_LagRecord* record );

		virtual int GetResolverSide( C_CSPlayer* player, Engine::C_LagRecord* record, int& type );

		virtual bool IsPointSafe( Engine::C_LagRecord* record, const Vector& outPoint, mstudiobbox_t* hitbox, int minHits = 3 );
		virtual bool IsStaticPointSafe( Engine::C_LagRecord* record, const Vector& point, mstudiobbox_t* hitbox, int side );

		Encrypted_t<RagebotData> rageData;
	};

	void C_Ragebot::StripAttack( Encrypted_t<CUserCmd> cmd ) {

		auto local = C_CSPlayer::GetLocalPlayer( );

		auto weapon = ( C_WeaponCSBaseGun* )local->m_hActiveWeapon( ).Get( );
		if ( !weapon ) {
			return;
		}

		auto weaponInfo = weapon->GetCSWeaponData( );
		if ( !weaponInfo.IsValid( ) ) {
			return;
		}

		if ( weapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER )
			cmd->buttons &= ~IN_ATTACK2;

		else
			cmd->buttons &= ~IN_ATTACK;
	}

	bool C_Ragebot::Run( Encrypted_t<CUserCmd> cmd, C_CSPlayer* local, bool* sendPacket ) {
#if 0
		if ( !InputSys::Get( )->IsKeyDown( VirtualKeys::F ) )
			return false;
#endif

		if ( !g_Vars.rage.enabled )
			return false;

#ifdef AUTOWALL_CALLS
		AutowallCalls = 0;
#endif

		if ( !g_Vars.globals.RandomInit ) {
			return false;
		}

		auto weapon = ( C_WeaponCSBaseGun* )local->m_hActiveWeapon( ).Get( );
		if ( !weapon )
			return false;

		auto weaponInfo = weapon->GetCSWeaponData( );
		if ( !weaponInfo.IsValid( ) )
			return false;

		// run aim on zeus
		if ( weapon->m_iItemDefinitionIndex( ) != WEAPON_ZEUS
			 && ( weaponInfo->m_iWeaponType == WEAPONTYPE_KNIFE || weaponInfo->m_iWeaponType == WEAPONTYPE_GRENADE || weaponInfo->m_iWeaponType == WEAPONTYPE_C4 ) )
			return false;

		if ( !SetupRageOptions( ) )
			return false;

		if ( !local || local->IsDead( ) )
			return false;

		if ( !local->CanShoot( ) )
			StripAttack( cmd );

		// we have a normal weapon or a non cocking revolver
		// choke if its the processing tick.
		bool revolver = weapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER;
		if ( local->CanShoot( ) && !Source::m_pClientState->m_nChokedCommands( ) && !revolver && !( g_Vars.rage.double_tap_bind.enabled && g_Vars.rage.exploit ) ) {
			*sendPacket = false;
			StripAttack( cmd );
			return false;
		}

		rageData->m_pLocal = local;
		rageData->m_pWeapon = weapon;
		rageData->m_pWeaponInfo = weaponInfo;
		rageData->m_pSendPacket = sendPacket;
		rageData->m_pCmd = cmd;

		/* maybe this shit is still unavaible */
		if ( !rageData->m_pCmd.IsValid( ) || !rageData->m_pLocal || !rageData->m_pWeapon || !rageData->m_pWeaponInfo.IsValid( ) )
			return false;

		if ( weapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER ) {
			static bool unk_meme = false;
			static bool unk_rofl = false;
			static float cockTime = 0.f;
			float curtime = local->m_nTickBase( ) * Source::m_pGlobalVars->interval_per_tick;
			rageData->m_pCmd->buttons &= ~IN_ATTACK2;
			if ( rageData->m_pLocal->CanShoot( 0, true ) ) {
				if ( cockTime <= curtime ) {
					if ( weapon->m_flNextSecondaryAttack( ) <= curtime )
						cockTime = curtime + 0.234375f;
					else
						rageData->m_pCmd->buttons |= IN_ATTACK2;
				} else
					rageData->m_pCmd->buttons |= IN_ATTACK;
			} else {
				cockTime = curtime + 0.234375f;
				rageData->m_pCmd->buttons &= ~IN_ATTACK;
			}

#if 0
			unk_rofl = unk_meme;
			unk_meme = TickbaseShiftCtx.over_choke_nr > 0;
#endif
		} else if ( !this->rageData->m_pLocal->CanShoot( ) ) {
			if ( !rageData->m_pWeaponInfo->m_ucFullAuto )
				rageData->m_pCmd->buttons &= ~IN_ATTACK;

			if ( Source::m_pGlobalVars->curtime < rageData->m_pLocal->m_flNextAttack( )
				 || rageData->m_pWeapon->m_iClip1( ) < 1
				 || !rageData->rbot->between_shots )
				return false;
		}

		bool ret = RunInternal( );

		return ret;
	}

	bool C_Ragebot::SetupRageOptions( ) {
		auto local = C_CSPlayer::GetLocalPlayer( );
		if ( !local || local->IsDead( ) )
			return false;

		auto weapon = ( C_WeaponCSBaseGun* )local->m_hActiveWeapon( ).Get( );
		if ( !weapon )
			return false;

		auto weaponInfo = weapon->GetCSWeaponData( );
		if ( !weaponInfo.IsValid( ) )
			return false;

		rageData->m_pWeapon = weapon;
		rageData->m_pWeaponInfo = weaponInfo;

		auto id = weapon->m_iItemDefinitionIndex( );
		if ( id != WEAPON_ZEUS ) {
			switch ( weaponInfo->m_iWeaponType ) {
				case WEAPONTYPE_PISTOL:
					if ( id == WEAPON_DEAGLE || id == WEAPON_REVOLVER )
						rageData->rbot = &g_Vars.rage_heavypistols;
					else
						rageData->rbot = &g_Vars.rage_pistols;
					break;
				case WEAPONTYPE_SUBMACHINEGUN:
					rageData->rbot = &g_Vars.rage_smgs;
					break;
				case WEAPONTYPE_RIFLE:
					rageData->rbot = &g_Vars.rage_rifles;
					break;
				case WEAPONTYPE_SHOTGUN:
					rageData->rbot = &g_Vars.rage_shotguns;
					break;
				case WEAPONTYPE_SNIPER_RIFLE:
					if ( id == WEAPON_G3SG1 || id == WEAPON_SCAR20 )
						rageData->rbot = &g_Vars.rage_autosnipers;
					else
						rageData->rbot = ( id == WEAPON_AWP ) ? &g_Vars.rage_awp : &g_Vars.rage_scout;
					break;
				case WEAPONTYPE_MACHINEGUN:
					rageData->rbot = &g_Vars.rage_heavys;
					break;
				default:
					rageData->rbot = &g_Vars.rage_default;
					break;
			}
		} else {
			rageData->rbot = &g_Vars.rage_taser;
		}

		if ( !rageData->rbot )
			return false;

		if ( !rageData->rbot->active ) {
			rageData->rbot = &g_Vars.rage_default;
		}

		return rageData->rbot->active;
	}

	bool C_Ragebot::RunInternal( ) {
		auto cmd_backup = *rageData->m_pCmd.Xor( );

		bool reset_cmd = true;
		bool repredict = false;
		int restore_zoom_level = INT_MAX;

		rageData->m_bPredictedScope = false;

		rageData->m_flSpread = Engine::Prediction::Instance( )->GetSpread( );
		rageData->m_flInaccuracy = Engine::Prediction::Instance( )->GetInaccuracy( );

		bool aim_success = RunHitscan( );
		if ( aim_success ) {
			reset_cmd = false;
		}

		// if failed at hitchancing or aimbot good  
		if ( rageData->m_bFailedHitchance || aim_success || rageData->m_bPrepareAim ) {
			if ( !rageData->m_bPrepareAim && rageData->rbot->autostop ) {
				int ticks = 1;

				// fullstop
				if ( rageData->rbot->autostop == 1 ) {
					auto speed = rageData->m_pLocal->m_vecVelocity( ).Length2D( );
					if ( speed > 20.0f ) {
						float maxSpeed = ( rageData->m_pWeapon->m_weaponMode( ) == 0 ) ? rageData->m_pWeaponInfo->m_flMaxSpeed : rageData->m_pWeaponInfo->m_flMaxSpeed2;
						static int playerSurfaceFrictionOffset = Horizon::Memory::FindInDataMap( rageData->m_pLocal->GetPredDescMap( ), XorStr( "m_surfaceFriction" ) );
						float playerSurfaceFriction = *( float* )( uintptr_t( rageData->m_pLocal ) + playerSurfaceFrictionOffset );
						float max_accelspeed = g_Vars.sv_accelerate->GetFloat( ) * Source::m_pGlobalVars->interval_per_tick * maxSpeed * playerSurfaceFriction;
						ticks = int( speed / max_accelspeed ) + 1;
					}
				} else
					Source::Movement::Get( )->StopPlayerAtMinimalSpeed( );

				if ( rageData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) {
					Source::Movement::Get( )->AutoStop( ticks );

					if ( Source::Movement::Get( )->StopPlayer( ) )
						repredict = true;
				}
				reset_cmd = false;
			}

			if ( rageData->rbot->autoscope && rageData->m_pWeaponInfo->m_iWeaponType == WEAPONTYPE_SNIPER_RIFLE && rageData->m_pWeapon->m_zoomLevel( ) <= 0 && rageData->m_pLocal->m_fFlags( ) & FL_ONGROUND ) {
				rageData->m_pCmd->buttons |= IN_ATTACK2;
				rageData->m_pCmd->buttons &= ~IN_ATTACK;
				restore_zoom_level = rageData->m_pWeapon->m_zoomLevel( );
				rageData->m_pWeapon->m_zoomLevel( ) = 1;
				rageData->m_bPredictedScope = true;
				repredict = true;
				reset_cmd = false;
			}

			rageData->m_bFailedHitchance = false;
		}

		if ( repredict ) {
			Engine::Prediction::Instance( )->Repredict( );
		} else {
			reset_cmd = false;
		}

		// compensate recoil 
		rageData->m_pCmd->viewangles -= rageData->m_pLocal->m_aimPunchAngle( ) * g_Vars.weapon_recoil_scale->GetFloat( );
		rageData->m_pCmd->viewangles.Normalize( );

		if ( rageData->rbot->no_spread && !g_Vars.misc.anti_untrusted
			 && ( g_Vars.globals.m_iServerType > 7 ) && rageData->m_pLocal->CanShoot( ) && rageData->m_pCmd->buttons & IN_ATTACK ) {
			reset_cmd = false;

			auto weapon_inaccuracy = Engine::Prediction::Instance( )->GetInaccuracy( );
			auto weapon_spread = Engine::Prediction::Instance( )->GetSpread( );

			auto random_seed = rageData->m_pCmd->random_seed & 255;

			auto rand1 = g_Vars.globals.SpreadRandom[ random_seed ].flRand1;
			auto rand_pi1 = g_Vars.globals.SpreadRandom[ random_seed ].flRandPi1;
			auto rand2 = g_Vars.globals.SpreadRandom[ random_seed ].flRand2;
			auto rand_pi2 = g_Vars.globals.SpreadRandom[ random_seed ].flRandPi2;

			int id = rageData->m_pWeapon->m_iItemDefinitionIndex( );
			auto recoil_index = rageData->m_pWeapon->m_flRecoilIndex( );

			if ( id == 64 ) {
				if ( rageData->m_pCmd->buttons & IN_ATTACK2 ) {
					rand1 = 1.0f - rand1 * rand1;
					rand2 = 1.0f - rand2 * rand2;
				}
			} else if ( id == 28 && recoil_index < 3.0f ) {
				for ( int i = 3; i > recoil_index; i-- ) {
					rand1 *= rand1;
					rand2 *= rand2;
				}

				rand1 = 1.0f - rand1;
				rand2 = 1.0f - rand2;
			}

			auto rand_inaccuracy = rand1 * weapon_inaccuracy;
			auto rand_spread = rand2 * weapon_spread;

			Vector2D spread =
			{
			   std::cos( rand_pi1 ) * rand_inaccuracy + std::cos( rand_pi2 ) * rand_spread,
			   std::sin( rand_pi1 ) * rand_inaccuracy + std::sin( rand_pi2 ) * rand_spread,
			};

			// 
			// pitch/yaw/roll
			// 

			Vector side, up;
			Vector forward = QAngle::Zero.ToVectors( &side, &up );

			Vector direction = forward + ( side * spread.x ) + ( up * spread.y );
			direction.Normalize( );

			QAngle angles_spread = direction.ToEulerAngles( );

			angles_spread.x -= rageData->m_pCmd->viewangles.x;
			angles_spread.Normalize( );

			forward = angles_spread.ToVectorsTranspose( &side, &up );

			angles_spread = forward.ToEulerAngles( &up );
			angles_spread.Normalize( );

			angles_spread.y += rageData->m_pCmd->viewangles.y;
			angles_spread.Normalize( );

			rageData->m_pCmd->viewangles = angles_spread;
			rageData->m_pCmd->viewangles.Normalize( );

			// 
			// pitch/roll
			// 
			// cmd->viewangles.x += ToDegrees( std::atan( spread.Length() ) );
			// cmd->viewangles.z = -ToDegrees( std::atan2( spread.x, spread.y ) );

			// 
			// yaw/roll
			// 
			// cmd->viewangles.y += ToDegrees( std::atan( spread.Length() ) );
			// cmd->viewangles.z = -( ToDegrees( std::atan2( spread.x, spread.y ) ) - 90.0f );
		}

		if ( restore_zoom_level != INT_MAX )
			rageData->m_pWeapon->m_zoomLevel( ) = restore_zoom_level;

		if ( reset_cmd ) {
			*rageData->m_pCmd.Xor( ) = cmd_backup;
			Engine::Prediction::Instance( )->Repredict( );
		} else {
#ifdef DEBUG_REPREDICT
			// std::get<0>( predicted_origins[ rageData->m_pCmd->command_number ] ) = rageData->m_pLocal->m_vecOrigin( );
			// std::get<1>( predicted_origins[ rageData->m_pCmd->command_number ] ) = rageData->m_pLocal->m_vecVelocity( );
#endif
		}

		return aim_success;
	}

	void C_Ragebot::AddPoint( C_CSPlayer* player, Engine::C_LagRecord* record, int side, std::vector<Vector>& points, const Vector& point, mstudiobbox_t* hitbox, mstudiohitboxset_t* hitboxSet, bool isMultipoint ) {
		// auto matrix = record->GetBoneMatrix( side );
		 //auto pointTransformed = point.Transform( matrix[ hitbox->bone ] );
		auto pointTransformed = point;

		bool safety = false;

		if ( !hitbox )
			return;

		if ( !hitboxSet )
			return;

		switch ( hitbox->group ) {
			case Hitgroup_Head:
				safety = rageData->rbot->mp_safety_head_hitbox;
				break;
			case Hitgroup_Gear: // just neck
				safety = rageData->rbot->mp_safety_neck_hitbox;
				break;
			case Hitgroup_Chest:
				safety = rageData->rbot->mp_safety_chest_hitbox;
				break;
			case Hitgroup_Stomach:
				safety = rageData->rbot->mp_safety_stomach_hitbox;
				break;
			case Hitgroup_RightLeg:
			case Hitgroup_LeftLeg:
				if ( hitboxSet->pHitbox( HITBOX_RIGHT_FOOT ) == hitbox || hitboxSet->pHitbox( HITBOX_LEFT_FOOT ) == hitbox )
					safety = rageData->rbot->mp_safety_feets_hitbox;
				else
					safety = rageData->rbot->mp_safety_legs_hitbox;
				break;
			case Hitgroup_RightArm:
			case Hitgroup_LeftArm:
				safety = rageData->rbot->mp_safety_arms_hitbox;
				break;
			default:
				safety = false;
				break;
		}


		auto missed = false;
		auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
		if ( lag_data.IsValid( ) )
			missed = lag_data->m_iMissedShots >= rageData->rbot->max_misses;

		if ( missed || safety ) {
			if ( !IsPointSafe( record, pointTransformed, hitbox, 3 )/* && !IsStaticPointSafe( record, pointTransformed, hitbox )*/ )
				return;
		}

		points.push_back( pointTransformed );
	}

	void C_Ragebot::HitchanceRaysMT( C_HitchanceArray* data ) {
		auto ragebot = ( C_Ragebot* )( Ragebot::Get( ).Xor( ) );
		ragebot->HitchanceRaysInternal( data );
	}

	void C_Ragebot::HitchanceRaysInternal( C_HitchanceArray* data ) {
		auto target = data->point->target;
		for ( int i = 0; i < data->dataCount; ++i ) {
			auto ray = &data->data[ i ];

			auto intersection = false;
			for ( auto capsule : target->capsules ) {
				intersection = segment_to_segment( rageData->m_vecEyePos, ray->end, capsule.m_mins, capsule.m_maxs, capsule.m_radius );
				if ( intersection ) {
					intersection = true;
					break;
				}
			}

			if ( !intersection ) {
				for ( auto box : target->obb ) {
					intersection = Math::IntersectionBoundingBox( rageData->m_vecEyePos, ray->end, box.min, box.max );
					if ( intersection ) {
						break;
					}
				}
			}

			if ( !intersection )
				continue;

			Autowall::C_FireBulletData fireData;
			fireData.m_bPenetration = true;

			fireData.m_vecStart = this->rageData->m_vecEyePos;
			fireData.m_vecDirection = ray->direction;
			fireData.m_iHitgroup = convert_hitbox_to_hitgroup( data->point->hitbox_idx );
			fireData.m_Player = this->rageData->m_pLocal;
			fireData.m_TargetPlayer = target->player;
			fireData.m_WeaponData = this->rageData->m_pWeaponInfo.Xor( );
			fireData.m_Weapon = this->rageData->m_pWeapon;

			auto damage = Autowall::FireBullets( &fireData );
			if ( damage < 1.0f ) {
				continue;
			}

			ray->hit = true;

			auto ratio = int( float( target->player->m_iHealth( ) ) / damage ) + 1;
			if ( ratio <= data->point->health_ratio )
				ray->damageIsAccurate = true;
		}
	}

	bool C_Ragebot::Hitchance( C_AimPoint* point, const Vector& start, float chance ) {
		if ( chance <= 0.0f )
			return true;

		// performance optimization.
		if ( ( rageData->m_pLocal->GetEyePosition( ) - point->point ).Length( ) > rageData->m_pWeaponInfo->m_flWeaponRange )
			return false;

		Vector forward = point->point - start;
		forward.Normalize( );

		Vector right, up;
		forward.GetVectors( right, up );

		auto renderable = rageData->m_pLocal->GetClientRenderable( );
		if ( !renderable )
			return false;

		auto model = renderable->GetModel( );
		if ( !model )
			return false;

		auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
		if ( !hdr )
			return false;

		auto hitbox_set = hdr->pHitboxSet( point->target->player->m_nHitboxSet( ) );

		if ( !hitbox_set )
			return false;

		auto hitbox = hitbox_set->pHitbox( point->hitbox_idx );

		if ( !hitbox )
			return false;

		const auto is_capsule = hitbox->m_flRadius != -1.f;

		const auto max_traces = 256;
		auto minimal_hits = int( float( max_traces ) * ( chance * 0.01f ) );
		auto hits = 0;
		for ( int i = 0; i < max_traces; ++i ) {
			int seed = i;
			float flRand1 = g_Vars.globals.SpreadRandom[ seed ].flRand1;
			float flRandPi1 = g_Vars.globals.SpreadRandom[ seed ].flRandPi1;
			float flRand2 = g_Vars.globals.SpreadRandom[ seed ].flRand2;
			float flRandPi2 = g_Vars.globals.SpreadRandom[ seed ].flRandPi2;

			float m_flRecoilIndex = rageData->m_pWeapon->m_flRecoilIndex( );
			if ( rageData->m_pWeapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER ) {
#if 0
				flRand1 = 1.f - flRand1 * flRand1;
				flRand2 = 1.f - flRand2 * flRand2;
#endif
			} else if ( rageData->m_pWeapon->m_iItemDefinitionIndex( ) == WEAPON_NEGEV && m_flRecoilIndex < 3.f ) {
				for ( int x = 3; x > m_flRecoilIndex; --x ) {
					flRand1 *= flRand1;
					flRand2 *= flRand2;
				}

				flRand1 = 1.f - flRand1;
				flRand2 = 1.f - flRand2;
			}

			float flRandInaccuracy = flRand1 * rageData->m_flInaccuracy;
			float flRandSpread = flRand2 * rageData->m_flSpread;

			float flRandPi1Cos, flRandPi1Sin;
			DirectX::XMScalarSinCos( &flRandPi1Sin, &flRandPi1Cos, flRandPi1 );

			float flRandPi2Cos, flRandPi2Sin;
			DirectX::XMScalarSinCos( &flRandPi2Sin, &flRandPi2Cos, flRandPi2 );

			float spread_x = flRandPi1Cos * flRandInaccuracy + flRandPi2Cos * flRandSpread;
			float spread_y = flRandPi1Sin * flRandInaccuracy + flRandPi2Sin * flRandSpread;

			Vector direction;
			direction.x = forward.x + ( spread_x * right.x ) + ( spread_y * up.x );
			direction.y = forward.y + ( spread_x * right.y ) + ( spread_y * up.y );
			direction.z = forward.z + ( spread_x * right.z ) + ( spread_y * up.z );

			Vector end = start + direction * rageData->m_pWeaponInfo->m_flWeaponRange;

			auto hit = false;
			auto target = point->target;

			if ( hitbox->m_flRadius > 0.f ) {
				for ( auto capsule : target->capsules ) {
					hit = segment_to_segment( rageData->m_vecEyePos, end, capsule.m_mins, capsule.m_maxs, capsule.m_radius );
					if ( hit ) {
						break;
					}
				}
			} else {
				for ( auto box : target->obb ) {
					hit = Math::IntersectionBoundingBox( rageData->m_vecEyePos, end, box.min, box.max );
					if ( hit ) {
						break;
					}
				}
			}
			if ( hit ) {
				hits++;
			}

			// abort if hitchance is already sufficent.
			if ( static_cast< float >( hits ) / static_cast< float >( max_traces ) >= chance / 100.f )
				return true;

			// abort if we can no longer reach hitchance.
			if ( static_cast< float >( hits + max_traces - i ) / static_cast< float >( max_traces ) < chance / 100.f )
				return false;
		}

		return static_cast< float >( hits ) / static_cast< float >( max_traces ) >= chance / 100.f;
	}

	bool C_Ragebot::AccuracyBoost( C_AimPoint* point, const Vector& start, float chance ) {
		int hits = 0;
		int autowall_hits = 0;

		Vector forward = point->point - start;
		forward.Normalize( );

		Vector right, up;
		forward.GetVectors( right, up );

		auto spread = rageData->m_flSpread + rageData->m_flInaccuracy;

		// NOTICE: accuracy boost calculation from ot
		for ( int i = 1; i <= 6; ++i ) {
			for ( int j = 0; j < 8; ++j ) {
				float flSpread = spread * float( float( i ) / 6.f );

				// TODO: cache it
				float flDirCos, flDirSin;
				DirectX::XMScalarSinCos( &flDirCos, &flDirSin, DirectX::XM_2PI * float( float( j ) / 8.0f ) );

				auto spread_x = flDirCos * flSpread;
				auto spread_y = flDirSin * flSpread;

				Vector direction;
				direction.x = forward.x + ( spread_x * right.x ) + ( spread_y * up.x );
				direction.y = forward.y + ( spread_x * right.y ) + ( spread_y * up.y );
				direction.z = forward.z + ( spread_x * right.z ) + ( spread_y * up.z );

				auto end = start + direction * rageData->m_pWeaponInfo->m_flWeaponRange;

				auto hit = false;
				auto target = point->target;
				for ( auto capsule : target->capsules ) {
					hit = segment_to_segment( rageData->m_vecEyePos, end, capsule.m_mins, capsule.m_maxs, capsule.m_radius );
					if ( hit ) {
						break;
					}
				}

				if ( !hit ) {
					for ( auto box : target->obb ) {
						hit = Math::IntersectionBoundingBox( rageData->m_vecEyePos, end, box.min, box.max );
						if ( hit ) {
							break;
						}
					}
				}

				if ( hit ) {
					hits++;

					Autowall::C_FireBulletData fireData;
					fireData.m_bPenetration = true;

					fireData.m_vecStart = this->rageData->m_vecEyePos;
					fireData.m_vecDirection = direction;
					fireData.m_iHitgroup = convert_hitbox_to_hitgroup( point->hitbox_idx );
					fireData.m_Player = this->rageData->m_pLocal;
					fireData.m_TargetPlayer = target->player;
					fireData.m_WeaponData = this->rageData->m_pWeaponInfo.Xor( );
					fireData.m_Weapon = this->rageData->m_pWeapon;

					auto damage = Autowall::FireBullets( &fireData );
					if ( damage >= 1.0f ) {
						if ( this->rageData->m_pWeapon->m_iItemDefinitionIndex( ) != WEAPON_SSG08 && this->rageData->m_pWeapon->m_iItemDefinitionIndex( ) != WEAPON_AWP ) {
							autowall_hits++;
							continue;
						}

						auto health_ratio = int( float( target->player->m_iHealth( ) ) / damage ) + 1;
						if ( health_ratio <= point->health_ratio ) {
							autowall_hits++;
						}
					}
				}
			}
		}

		if ( hits > 0 && autowall_hits > 0 ) {
			auto hitchance = ( float( hits ) / 48.0f ) * 100.f;

			if ( hitchance < chance )
				return false;

			auto final_chance = ( float( autowall_hits ) / 48.f ) * 100.f;

			return final_chance >= chance;
		}
		return false;
	}

	bool C_Ragebot::IsPointAccurate( C_AimPoint* point, const Vector& start ) {
		if ( rageData->m_bPredictedScope ) {
			rageData->m_pCmd->buttons |= IN_ATTACK2;
			rageData->m_pCmd->buttons &= ~IN_ATTACK;
			return false;
		}

		//if ( TickbaseShiftCtx.over_choke_nr ) {
		   //return false;
		//}

		auto exploits_enabled = [ ]( ) {
			return ( g_Vars.rage.exploit && g_Vars.rage.double_tap_bind.enabled );
		};

		if ( !rageData->m_pLocal->CanShoot( ) )
			return false;

		if ( rageData->rbot->shotdelay && !exploits_enabled( ) ) {
			float delay = rageData->rbot->shotdelay_amount * 0.01f;
			float nextShotTime = this->rageData->m_pWeaponInfo->m_flUnknownFloat0 + TICKS_TO_TIME( LastShotTime );
			if ( ( ( ( rageData->m_pWeaponInfo->m_flUnknownFloat0 * delay )
					 + ( rageData->m_pWeaponInfo->m_flUnknownFloat0 * delay ) ) + nextShotTime ) > TICKS_TO_TIME( Source::m_pGlobalVars->tickcount ) ) {
				rageData->m_pCmd->buttons &= ~IN_ATTACK;
				return false;
			}
		}

		if ( rageData->rbot->delay_shot_on_unducking && rageData->m_pLocal->m_flDuckAmount( ) >= ( rageData->rbot->on_shot_aa ? 0.25f : 0.125f ) ) {
			if ( g_Vars.globals.m_flPreviousDuckAmount > rageData->m_pLocal->m_flDuckAmount( ) ) {
				return false;
			}
		}

		if ( point->damage < point->target->player->m_iHealth( ) && rageData->m_iDelayTicks < 3 ) {
			auto eye_pos = rageData->m_pLocal->GetEyePosition( );
			auto peek_add = rageData->m_pLocal->m_vecVelocity( ) * TICKS_TO_TIME( 1 );
			auto second_scan = eye_pos;
			if ( rageData->m_pLocal->m_Collision( )->m_vecMaxs.x > peek_add.Length2D( ) ) {
				auto peek_direction = rageData->m_pLocal->m_vecVelocity( ).Normalized( );
				second_scan += peek_direction * rageData->m_pLocal->m_Collision( )->m_vecMaxs.x;
			} else {
				second_scan += peek_add;
			}

			float predicted_damage = 0.f;

			Autowall::C_FireBulletData data;
			data.m_bPenetration = true;

			data.m_vecStart = second_scan;
			data.m_vecDirection = ( point->point - second_scan ).Normalized( );
			data.m_Player = rageData->m_pLocal;
			data.m_TargetPlayer = point->target->player;
			data.m_WeaponData = rageData->m_pWeaponInfo.Xor( );
			data.m_Weapon = rageData->m_pWeapon;

			predicted_damage = Autowall::FireBullets( &data );

			if ( predicted_damage > point->damage ) {
				rageData->m_iDelayTicks++;
				return false;
			}
		}

		//store delay for fired shot at
		rageData->m_iDelayTicksStored = rageData->m_iDelayTicks;

		// reset delay state
		rageData->m_iDelayTicks = 0;

		// delay shot for best peek
#if 0
		if ( rageData->m_pLocal->m_vecVelocity( ).Length2D( ) >= 2.f && !exploits_enabled( ) ) {
			Vector predicted_pos = rageData->m_vecEyePos + ( rageData->m_pLocal->m_vecVelocity( ) * 0.2f );
			get_edge( predicted_pos, rageData->m_vecEyePos, 16.0f, rageData->m_pLocal );

			if ( std::fabsf( rageData->m_flLastPeekTime - TICKS_TO_TIME( Source::m_pGlobalVars->tickcount ) ) <= 0.2f ) {
				if ( predicted_pos.Distance( rageData->m_vecEyePos ) > rageData->m_PeekingPosition.Distance( rageData->m_vecEyePos ) ) {
					rageData->m_PeekingPosition = predicted_pos;
					rageData->m_flLastPeekTime = TICKS_TO_TIME( Source::m_pGlobalVars->tickcount );
				} else {
					predicted_pos = rageData->m_PeekingPosition;
				}
			} else {
				rageData->m_PeekingPosition = predicted_pos;
				rageData->m_flLastPeekTime = TICKS_TO_TIME( Source::m_pGlobalVars->tickcount );
			}

#if 0
			if ( InputSys::Get( )->IsKeyDown( VirtualKeys::F ) ) {
				Source::m_pDebugOverlay->AddLineOverlay( rageData->m_vecEyePos, predicted_pos,
														 255, 255, 255, false, 1.0f );
				Source::m_pDebugOverlay->AddBoxOverlay( predicted_pos, Vector( -1.f, -1.f, -1.f ), Vector( 1.f, 1.f, 1.f ),
														QAngle( ), 255, 0, 0, 150, 1.0f );
			}
#endif

			{
				Autowall::C_FireBulletData fireData;
				fireData.m_bPenetration = true;

				fireData.m_vecStart = predicted_pos;
				fireData.m_vecDirection = point->point - predicted_pos;
				fireData.m_iHitgroup = convert_hitbox_to_hitgroup( point->hitbox_idx );
				fireData.m_Player = this->rageData->m_pLocal;
				fireData.m_TargetPlayer = point->target->player;
				fireData.m_WeaponData = this->rageData->m_pWeaponInfo.Xor( );
				fireData.m_Weapon = this->rageData->m_pWeapon;

				auto damage = Autowall::FireBullets( &fireData );
				auto health_ratio = 100;
				auto lethal_damage = Autowall::ScaleDamage( fireData.m_TargetPlayer, fireData.m_WeaponData->m_iWeaponDamage, fireData.m_WeaponData->m_flArmorRatio, fireData.m_iHitgroup );
				if ( damage > 0.0f ) {
					health_ratio = int( float( point->target->player->m_iHealth( ) ) / damage ) + 1;
				}

				if ( point->health_ratio > health_ratio ) {
					return false;
				}
			}

			if ( !point->is_safe ) {
				auto hdr = *( studiohdr_t** )( point->target->player->m_pStudioHdr( ) );
				auto hitboxSet = hdr->pHitboxSet( point->target->player->m_nHitboxSet( ) );
				auto pelvis = hitboxSet->pHitbox( HITBOX_PELVIS );
				auto stomach = hitboxSet->pHitbox( HITBOX_STOMACH );

				auto ps = 0.0f;
				if ( GetBoxOption( pelvis, hitboxSet, ps, point->target->override_hitscan ) ) {
					auto p = ( pelvis->bbmax + pelvis->bbmin ) * 0.5f;
					p = p.Transform( point->target->record->GetBoneMatrix( point->target->side )[ pelvis->bone ] );

					Autowall::C_FireBulletData fireData;
					fireData.m_bPenetration = true;

					fireData.m_vecStart = predicted_pos;
					fireData.m_vecDirection = p - predicted_pos;
					fireData.m_iHitgroup = convert_hitbox_to_hitgroup( point->hitbox_idx );
					fireData.m_Player = this->rageData->m_pLocal;
					fireData.m_TargetPlayer = point->target->player;
					fireData.m_WeaponData = this->rageData->m_pWeaponInfo.Xor( );
					fireData.m_Weapon = this->rageData->m_pWeapon;
					auto damage = Autowall::FireBullets( &fireData );

					if ( damage > 0.0f ) {
						auto health_ratio = int( float( point->target->player->m_iHealth( ) ) / damage ) + 1;
						if ( point->health_ratio > health_ratio ) {
							return false;
						}
					}
				}

				if ( GetBoxOption( stomach, hitboxSet, ps, point->target->override_hitscan ) ) {
					auto p = ( stomach->bbmax + stomach->bbmin ) * 0.5f;
					p = p.Transform( point->target->record->GetBoneMatrix( point->target->side )[ stomach->bone ] );

					Autowall::C_FireBulletData fireData;
					fireData.m_bPenetration = true;

					fireData.m_vecStart = predicted_pos;
					fireData.m_vecDirection = p - predicted_pos;
					fireData.m_iHitgroup = convert_hitbox_to_hitgroup( point->hitbox_idx );
					fireData.m_Player = this->rageData->m_pLocal;
					fireData.m_TargetPlayer = point->target->player;
					fireData.m_WeaponData = this->rageData->m_pWeaponInfo.Xor( );
					fireData.m_Weapon = this->rageData->m_pWeapon;
					auto damage = Autowall::FireBullets( &fireData );

					if ( damage > 0.0f ) {
						auto  health_ratio = int( float( point->target->player->m_iHealth( ) ) / damage ) + 1;
						if ( point->health_ratio > health_ratio ) {
							return false;
						}
					}
				}
			}
		}
#endif
		auto can_hitchance = [ this ]( ) {
			// nospread enabled
			if ( rageData->m_flSpread == 0.0f || rageData->m_flInaccuracy == 0.0f )
				return false;

			const auto weapon_id = rageData->m_pWeapon->m_iItemDefinitionIndex( );
			const auto crouched = rageData->m_pLocal->m_fFlags( ) & FL_DUCKING;
			const auto sniper = rageData->m_pWeaponInfo->m_iWeaponType == WEAPONTYPE_SNIPER_RIFLE;
			const auto round_acc = [ ]( const float accuracy ) { return roundf( accuracy * 1000.f ) / 1000.f; };

			float rounded_acc = round_acc( rageData->m_flInaccuracy );

			// no need for hitchance, if we can't increase it anyway.
			if ( crouched ) {
				if ( rounded_acc == round_acc( sniper ? rageData->m_pWeaponInfo->m_flInaccuracyCrouchAlt : rageData->m_pWeaponInfo->m_flInaccuracyCrouch ) ) {
					return false;
				}
			} else {
				if ( rounded_acc == round_acc( sniper ? rageData->m_pWeaponInfo->m_flInaccuracyStandAlt : rageData->m_pWeaponInfo->m_flInaccuracyStand ) ) {
					return false;
				}
			}

			return true;
			};

		float hitchance = rageData->rbot->hitchance;
		if ( g_TickbaseController.Using( ) )
			hitchance = rageData->rbot->doubletap_hitchance;

		if ( rageData->rbot->hitchance > 0.0f && can_hitchance( ) ) {
			if ( !Hitchance( point, start, hitchance ) )
				return false;

			if ( rageData->m_pLocal->m_fFlags( ) & FL_ONGROUND && !AccuracyBoost( point, start, rageData->rbot->hitchance_accuracy ) )
				return false;
		}

		return true;
	}

	void C_Ragebot::Multipoint( C_CSPlayer* player, Engine::C_LagRecord* record, int side, std::vector<Vector>& points, mstudiobbox_t* hitbox, mstudiohitboxset_t* hitboxSet, float pointScale ) {
		auto boneMatrix = record->GetBoneMatrix( );

		if ( !hitbox )
			return;

		if ( !hitboxSet )
			return;

		Vector center = ( hitbox->bbmax + hitbox->bbmin ) * 0.5f;
		Vector centerTrans = center.Transform( boneMatrix[ hitbox->bone ] );

		// FIXME: move eye pos to global variables
		if ( g_Vars.esp.aim_points ) {
			rageData->m_vecEyePos = C_CSPlayer::GetLocalPlayer( )->GetEyePosition( );
		}

		AddPoint( player, record, side, points,
				  centerTrans,
				  hitbox, hitboxSet, false
		);

		auto local = C_CSPlayer::GetLocalPlayer( );
		if ( !local || local->IsDead( ) )
			return;

		if ( pointScale <= 0.0f )
			return;

		if ( rageData->rbot->dynamic_ps && rageData->m_pWeapon && hitbox->m_flRadius > 0.0f ) {
			float spreadCone = Engine::Prediction::Instance( )->GetSpread( ) + Engine::Prediction::Instance( )->GetInaccuracy( );
			float dist = centerTrans.Distance( rageData->m_vecEyePos );

			dist /= sinf( DEG2RAD( 90.0f - RAD2DEG( spreadCone ) ) );

			spreadCone = sinf( spreadCone );

			float radiusScaled = ( hitbox->m_flRadius - ( dist * spreadCone ) );
			if ( radiusScaled <= 0.0f )
				return;

			float ps = pointScale;
			pointScale = ( radiusScaled / hitbox->m_flRadius );
			pointScale = Math::Clamp( pointScale, 0.0f, ps );
		}

		bool optimization = hitbox->group != Hitgroup_LeftLeg && hitbox->group != Hitgroup_RightLeg ||
			hitbox != hitboxSet->pHitbox( HITBOX_RIGHT_THIGH ) && hitbox != hitboxSet->pHitbox( HITBOX_LEFT_THIGH );

		auto IsLimb = [ ]( int group ) {
			return group == Hitgroup_LeftLeg || group == Hitgroup_RightLeg || group == Hitgroup_RightArm || group == Hitgroup_LeftArm || group == Hitgroup_Gear;
			};

		if ( hitbox->m_flRadius <= 0.0f ) {
			if ( optimization ) {

				AddPoint( player, record, side, points,
						  Vector( ( ( hitbox->bbmax.x - center.x ) * pointScale ) + center.x, center.y, center.z ).Transform( boneMatrix[ hitbox->bone ] ),
						  hitbox, hitboxSet, true
				);

				AddPoint( player, record, side, points,
						  Vector( ( ( hitbox->bbmin.x - center.x ) * pointScale ) + center.x, center.y, center.z ).Transform( boneMatrix[ hitbox->bone ] ),
						  hitbox, hitboxSet, true
				);
			}
		} else {
			auto density = 3;//rageData->rbot->mp_density;
			if ( density == 3 ) {
				static float last_add_time = 0.f;
				static auto adaptive_density = 0;
				if ( **( int** )Engine::Displacement.Data.m_uHostFrameTicks > 1 ) {
					adaptive_density = 0;
				} else if ( std::fabsf( Source::m_pGlobalVars->realtime - last_add_time ) >= 0.2f ) {
					adaptive_density++;
					last_add_time = Source::m_pGlobalVars->realtime;
				}

				density = adaptive_density;
			}

			bool vertical = hitbox == hitboxSet->pHitbox( HITBOX_HEAD );

			auto min = hitbox->bbmin.Transform( boneMatrix[ hitbox->bone ] );
			auto max = hitbox->bbmax.Transform( boneMatrix[ hitbox->bone ] );

			auto delta = centerTrans - rageData->m_vecEyePos;
			delta.Normalized( );


			auto max_min = max - min;
			max_min.Normalized( );

			auto cr = max_min.Cross( delta );

			Vector right, up;
			if ( vertical ) {
				auto cr_angle = cr.ToEulerAngles( );
				cr_angle.ToVectors( &right, &up );
				cr_angle.roll = delta.ToEulerAngles( ).pitch;

				Vector _up = up, _right = right, _cr = cr;
				cr = _right;
				right = _cr;
			} else {
				delta.GetVectors( up, right );
			}

			RayTracer::Hitbox box( min, max, hitbox->m_flRadius );
			RayTracer::Trace trace;

			std::vector<Vector> points_unscaled;

			if ( hitbox == hitboxSet->pHitbox( HITBOX_HEAD ) ) {
				Vector middle = ( right.Normalized( ) + up.Normalized( ) ) * 0.5f;
				Vector middle2 = ( right.Normalized( ) - up.Normalized( ) ) * 0.5f;

#if 0
				if ( InputSys::Get( )->IsKeyDown( VirtualKeys::H ) ) {
					Source::m_pDebugOverlay->AddLineOverlay( centerTrans - cr * 10.0f, centerTrans, 0, 255, 0, false, 1.0f );
					Source::m_pDebugOverlay->AddLineOverlay( centerTrans + cr * 10.0f, centerTrans, 0, 255, 0, false, 1.0f );

					Source::m_pDebugOverlay->AddLineOverlay( centerTrans - right * 10.0f, centerTrans, 0, 0, 255, false, 1.0f );
					Source::m_pDebugOverlay->AddLineOverlay( centerTrans + right * 10.0f, centerTrans, 0, 0, 255, false, 1.0f );

					Source::m_pDebugOverlay->AddLineOverlay( centerTrans - up * 10.0f, centerTrans, 255, 0, 0, false, 1.0f );
					Source::m_pDebugOverlay->AddLineOverlay( centerTrans + up * 10.0f, centerTrans, 255, 0, 0, false, 1.0f );

					Source::m_pDebugOverlay->AddLineOverlay( centerTrans - middle * 10.0f, centerTrans, 255, 255, 255, false, 1.0f );
					Source::m_pDebugOverlay->AddLineOverlay( centerTrans + middle * 10.0f, centerTrans, 255, 255, 255, false, 1.0f );

					Source::m_pDebugOverlay->AddLineOverlay( centerTrans - middle2 * 10.0f, centerTrans, 255, 0, 255, false, 1.0f );
					Source::m_pDebugOverlay->AddLineOverlay( centerTrans + middle2 * 10.0f, centerTrans, 255, 0, 255, false, 1.0f );
				}
#endif
				RayTracer::Ray ray = RayTracer::Ray( rageData->m_vecEyePos, centerTrans + ( middle * 1000.0f ) );
				RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
				points_unscaled.push_back( trace.m_traceEnd );

				ray = RayTracer::Ray( rageData->m_vecEyePos, centerTrans - ( middle2 * 1000.0f ) );
				RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
				points_unscaled.push_back( trace.m_traceEnd );

				ray = RayTracer::Ray( rageData->m_vecEyePos, centerTrans + ( up * 1000.0f ) );
				RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
				points_unscaled.push_back( trace.m_traceEnd );

				ray = RayTracer::Ray( rageData->m_vecEyePos, centerTrans - ( up * 1000.0f ) );
				RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
				points_unscaled.push_back( trace.m_traceEnd );
			} else {
				RayTracer::Ray ray = RayTracer::Ray( rageData->m_vecEyePos, centerTrans - ( ( vertical ? cr : up ) * 1000.0f ) );
				RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
				points_unscaled.push_back( trace.m_traceEnd );

				ray = RayTracer::Ray( rageData->m_vecEyePos, centerTrans + ( ( vertical ? up : up ) * 1000.0f ) );
				RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
				points_unscaled.push_back( trace.m_traceEnd );
			}

			for ( size_t i = 0u; i < points_unscaled.size( ); ++i ) {
				auto delta_center = points_unscaled[ i ] - centerTrans;
				points_unscaled[ i ] = centerTrans + delta_center * pointScale;

				AddPoint( player, record, side, points,
						  points_unscaled[ i ],
						  hitbox, hitboxSet, true
				);
			}

#if 0
			if ( density >= 2 ) {
				Vector p1, p2;
				if ( vertical ) {
					p1 = ( centerTrans + p[ 0 ] ) * 0.5f;
					p2 = ( centerTrans + p[ 1 ] ) * 0.5f;
				} else {
					p1 = ( centerTrans + p[ 6 ] ) * 0.5f;
					p2 = ( centerTrans + p[ 7 ] ) * 0.5f;
				}

				AddPoint( player, record, side, points,
						  p1,
						  hitbox, hitboxSet, true );
				AddPoint( player, record, side, points,
						  p2,
						  hitbox, hitboxSet, true );
			}
#endif
		}
	}

	bool C_Ragebot::RunHitscan( ) {
		rageData->m_vecEyePos = rageData->m_pLocal->GetEyePosition( );

		rageData->m_bFailedHitchance = false;
		rageData->m_bPrepareAim = false;

		//g_TickbaseController.m_bSupressRecharge = false; // suppress recharge

		// IProfiler::ProfileData_t ProfileData = IProfiler::Get( )->GetData( 0 ); 
		std::vector<C_AimPoint> aim_points;
		aim_points.reserve( 256 );

		std::vector<C_AimTarget> aim_targets;
		aim_targets.reserve( Source::m_pGlobalVars->maxClients );

		// TODO: implement team check in lag comp
		for ( int idx = 1; idx <= Source::m_pGlobalVars->maxClients; ++idx ) {
			auto player = C_CSPlayer::GetPlayerByIndex( idx );
			if ( !player || player == rageData->m_pLocal || player->IsDead( ) || player->m_bGunGameImmunity( ) || player->IsTeammate( rageData->m_pLocal ) )
				continue;

			auto data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
			if ( !data.IsValid( ) || data->m_History.empty( ) )
				continue;

			if ( GeneratePoints( player, aim_targets, aim_points ) ) {
				auto hitboxSet = ( *( studiohdr_t** )player->m_pStudioHdr( ) )->pHitboxSet( player->m_nHitboxSet( ) );

				auto& target = aim_targets.back( );
				target.capsules.reserve( 15 );
				target.obb.reserve( 4 );

				for ( int i = 0; i < hitboxSet->numhitboxes; ++i ) {
					auto box = hitboxSet->pHitbox( i );

					if ( box->m_flRadius == -1.f ) {
						auto& obb = target.obb.emplace_back( );
						obb.min = box->bbmin.Transform( player->m_CachedBoneData( ).Element( box->bone ) );
						obb.max = box->bbmax.Transform( player->m_CachedBoneData( ).Element( box->bone ) );
						obb.idx = i;
					} else {
						auto& capsule = target.capsules.emplace_back( );
						capsule.m_mins = box->bbmin.Transform( player->m_CachedBoneData( ).Element( box->bone ) );
						capsule.m_maxs = box->bbmax.Transform( player->m_CachedBoneData( ).Element( box->bone ) );
						capsule.m_radius = box->m_flRadius;
						capsule.m_idx = i;
					}
				}
			}
		}

		if ( aim_targets.empty( ) )
			return false;

		if ( aim_points.empty( ) ) {
			for ( auto& target : aim_targets )
				target.backup.Apply( target.player );
			return false;
		}

		std::vector< std::pair<C_AimTarget, std::vector<C_AimPoint>> > aim_data;

		for ( auto& target : aim_targets ) {
			std::vector<C_AimPoint> temp_points;
			for ( auto& point : aim_points ) {
				if ( point.target->player->entindex( ) == target.player->entindex( ) )
					temp_points.emplace_back( point );
			}

			if ( !temp_points.empty( ) ) {
				if ( g_Vars.rage.rage_multithread ) {
					auto numThreads = std::thread::hardware_concurrency( );
					numThreads = Math::Clamp<unsigned int>( numThreads, 2, 16 );

					C_PointsArray arrayScan[ 32 ] = {};
					auto threadScan = numThreads;
					auto pointsPerThread = temp_points.size( ) / numThreads;
					auto additionalPoints = 0;
					auto additionalStep = 0;
					if ( temp_points.size( ) <= numThreads ) {
						threadScan = temp_points.size( );
						pointsPerThread = 1;
					} else if ( ( temp_points.size( ) % numThreads ) != 0 ) {
						additionalPoints = temp_points.size( ) % numThreads;
						additionalStep = additionalPoints;
						additionalStep = std::min( additionalPoints, int( numThreads ) / additionalPoints );
					}

					auto currentIt = 0;
					for ( uint32_t i = 0; i < threadScan; ++i ) {
						arrayScan[ i ].pointsCount = pointsPerThread;
						arrayScan[ i ].points = &temp_points[ currentIt ];

						currentIt += pointsPerThread;
						if ( additionalPoints > 0 ) {
							arrayScan[ i ].pointsCount += additionalStep;
							currentIt += additionalStep;

							additionalPoints -= additionalStep;
						}

						Threading::QueueJobRef( C_Ragebot::ScanPointArrayMT, &arrayScan[ i ] );
					}

					Threading::FinishQueue( );
				} else {
					for ( size_t i = 0u; i < temp_points.size( ); ++i )
						ScanPoint( &temp_points.at( i ) );
				}

				for ( auto& p : temp_points ) {
					if ( p.damage > 1.0f ) {
						std::pair< C_AimTarget, std::vector<C_AimPoint> > temp;
						temp.first = target;
						temp.second = temp_points;
						aim_data.emplace_back( temp );
						continue;
					} else {
						continue;
					}
				}
			} else {
				continue;
			}
		}

		C_AimTarget* best_target = nullptr;
		std::vector<C_AimPoint> best_aimpoints;

		for ( auto& data : aim_data ) {
			if ( !best_target ) {
				best_target = &data.first;
				continue;
			}

			auto check_targets = [ & ]( C_AimTarget a, C_AimTarget b ) -> bool {
				switch ( rageData->rbot->target_selection ) {
					case SELECT_LOWEST_HP:
						return a.player->m_iHealth( ) <= b.player->m_iHealth( ); break;
					case SELECT_LOWEST_DISTANCE:
						return a.player->m_vecOrigin( ).Distance( rageData->m_pLocal->m_vecOrigin( ) ) <= b.player->m_vecOrigin( ).Distance( rageData->m_pLocal->m_vecOrigin( ) ); break;
					case SELECT_LOWEST_PING:
						return ( *Source::m_pPlayerResource.Xor( ) )->GetPlayerPing( a.player->entindex( ) ) <= ( *Source::m_pPlayerResource.Xor( ) )->GetPlayerPing( b.player->entindex( ) ); break;
					case SELECT_HIGHEST_ACCURACY:
						float average_hitchance_a = 0.f, average_hitchance_b = 0.f;

						for ( auto& point : a.points )
							average_hitchance_a += point.hitchance;

						for ( auto& point : b.points )
							average_hitchance_b += point.hitchance;

						return average_hitchance_a >= average_hitchance_b; break;
				}
				};

			if ( check_targets( data.first, *best_target ) ) {
				best_target = &data.first;
				best_aimpoints = data.second;
				continue;
			}
		}

		if ( !best_target ) {
			return false;
		}

		if ( !best_aimpoints.size( ) ) {
			return false;
		}

		C_AimPoint* best_point = nullptr;

		float lethal_damage = !Autowall::IsArmored( best_target->player, convert_hitbox_to_hitgroup( HITBOX_STOMACH ) ) ? rageData->m_pWeaponInfo->m_iWeaponDamage : Autowall::ScaleDamage( best_target->player, rageData->m_pWeaponInfo->m_iWeaponDamage, rageData->m_pWeaponInfo->m_flArmorRatio, convert_hitbox_to_hitgroup( HITBOX_STOMACH ) );
		bool head_have_safepoints = false;
		bool body_have_safepoints = false;
		bool head_have_static_safepoints = false;
		bool body_have_static_safepoints = false;
		bool head_have_points = false;
		bool body_have_points = false;

		bool only_head = false;

		if ( g_Vars.rage.override_key.enabled ) {
			if ( rageData->rbot->bt_head_hitbox &&
				 !rageData->rbot->bt_arms_hitbox &&
				 !rageData->rbot->bt_chest_hitbox &&
				 !rageData->rbot->bt_feets_hitbox &&
				 !rageData->rbot->bt_legs_hitbox &&
				 !rageData->rbot->bt_neck_hitbox &&
				 !rageData->rbot->bt_pelvis_hitbox &&
				 !rageData->rbot->bt_stomach_hitbox )
				only_head = true;
		} else {
			if ( rageData->rbot->head_hitbox &&
				 !rageData->rbot->arms_hitbox &&
				 !rageData->rbot->chest_hitbox &&
				 !rageData->rbot->feets_hitbox &&
				 !rageData->rbot->legs_hitbox &&
				 !rageData->rbot->neck_hitbox &&
				 !rageData->rbot->pelvis_hitbox &&
				 !rageData->rbot->stomach_hitbox )
				only_head = true;
		}

		for ( auto& p : best_aimpoints ) {
			if ( p.damage < 1.0f ) {
				continue;
			}

			if ( p.is_body ) {
				body_have_points = true;
				if ( p.is_safe )
					body_have_safepoints = true;

				if ( p.is_static_safe )
					body_have_static_safepoints;
			}

			if ( p.is_head ) {
				head_have_points = true;
				if ( p.is_safe )
					head_have_safepoints = true;

				if ( p.is_static_safe )
					head_have_static_safepoints;
			}
		}

		for ( auto& p : best_aimpoints ) {
			if ( p.damage < 1.0f )
				continue;

			if ( lethal_damage >= best_target->player->m_iHealth( ) ) {
				if ( p.is_head && !p.is_safe )
					continue;
			}

			int resolver_type = g_Vars.globals.m_iResolverType[ best_target->player->entindex( ) ];

			if ( rageData->rbot->hitbox_selection == 2 && !p.is_safe )
				continue;

			int hp = best_target->player->m_iHealth( );

			float mindmg = rageData->rbot->min_damage;
			if ( rageData->rbot->min_damage_override && g_Vars.rage.key_dmg_override.enabled ) {
				mindmg = rageData->rbot->min_damage_override_amount;
			} else if ( !p.did_penetrate ) {
				mindmg = rageData->rbot->min_damage_visible;
			}

			if ( rageData->m_pWeapon->m_iItemDefinitionIndex( ) == WEAPON_SCAR20 ||
				 rageData->m_pWeapon->m_iItemDefinitionIndex( ) == WEAPON_G3SG1 &&
				 g_Vars.rage.exploit &&
				 g_Vars.rage.double_tap_bind.enabled
				 ) {

				if ( hp < lethal_damage && rageData->m_pLocal->m_iShotsFired( ) > 0 )
					mindmg = hp;
				else
					mindmg = hp * 0.5f;

				if ( p.damage < mindmg ) {
					continue;
				}
			} else {
				if ( p.damage < mindmg ) {
					int add = 3;
					if ( rageData->rbot->health_override ) {
						add = rageData->rbot->health_override_amount;
						if ( p.damage < hp + add ) {
							continue;
						}
					} else {
						if ( p.damage < hp + add ) {
							continue;
						}
					}
				}
			}

			if ( !best_point ) {
				best_point = &p;
				continue;
			}

			if ( p.damage >= best_point->damage && p.is_head && !p.is_safe && head_have_safepoints )
				continue;

			if ( p.damage >= best_point->damage && p.is_body && !p.is_safe && body_have_safepoints )
				continue;

			/* if ( p.damage >= best_point->damage && p.is_head && !p.is_static_safe && head_have_static_safepoints )
				continue;

			 if ( p.damage >= best_point->damage && p.is_body && !p.is_static_safe && body_have_static_safepoints )
				continue;*/

			auto best_target = best_point->target;

			if ( best_target->prefer_safe ) {
				if ( best_point->is_safe != p.is_safe ) {
					best_point = best_point->is_safe ? best_point : &p;
					continue;
				}
			} else if ( best_target->prefer_body ) {
				if ( best_point->is_body != p.is_body ) {
					best_point = best_point->is_body ? best_point : &p;
					continue;
				}
			} else if ( best_target->prefer_head ) {
				if ( best_point->is_head != p.is_head ) {
					best_point = best_point->is_head ? best_point : &p;
					continue;
				}
			}
			switch ( rageData->rbot->hitbox_selection ) {
				case 0: // damage
					if ( p.damage >= best_point->damage ) {
						best_point = &p;
						continue;
					}
					break;
				case 1: // accuracy
					if ( p.hitchance >= best_point->hitchance ) {
						best_point = &p;
						continue;
					} else if ( p.is_safe != best_point->is_safe ) {
						if ( p.is_safe ) {
							best_point = &p;
							continue;
						}
					}
					break;
				case 2: // safety 
					if ( p.is_safe != best_point->is_safe ) {
						if ( p.is_safe ) {
							best_point = &p;
							continue;
						}
					}
					break;
				default:
					break;
			}
		}

		bool result = false;
		if ( best_point ) {
			if ( IsPointAccurate( best_point, rageData->m_vecEyePos ) ) {
				if ( AimAtPoint( best_point ) && rageData->m_pCmd->buttons & IN_ATTACK ) {
					Encrypted_t<Engine::C_EntityLagData> lagData = Engine::LagCompensation::Get( )->GetLagData( best_point->target->player->m_entIndex );
					/* if ( TIME_TO_TICKS( lagData->m_History.front( ).m_flSimulationTime - best_point->target->record->m_flSimulationTime ) > 0 )
						TickbaseShiftCtx.AdjustPlayerTimeBaseFix( Source::m_pClientState->m_nChokedCommands( ) + TickbaseShiftCtx.will_shift_tickbase + 1 );*/

					//g_TickbaseController.m_bSupressRecharge = false; // suppress recharge


					rageData->m_pCmd->tick_count = TIME_TO_TICKS( best_point->target->record->m_flSimulationTime + Engine::LagCompensation::Get( )->GetLerp( ) );

					// ch ck for animation fix accuracy 

					IRoundFireBulletsStore::Get( )->AimCallback(
						TIME_TO_TICKS( lagData->m_History.front( ).m_flSimulationTime - best_point->target->record->m_flSimulationTime ),
						best_point->hitchance,
						best_point->hitbox_idx,
						best_point->damage );

					std::stringstream msg;

					auto FixedStrLenght = [ ]( std::string str ) -> std::string {
						if ( ( int )str[ 0 ] > 255 )
							return rand( ) % 2 ? XorStr( "dolbaeb" ) : XorStr( "debil" );

						if ( str.size( ) < 15 )
							return str;

						std::string result;
						for ( size_t i = 0; i < 15u; i++ )
							result.push_back( str.at( i ) );
						return result;
						};

					auto TranslateHitbox = [ ]( int hitbox ) -> std::string {
						std::string result = { };
						switch ( hitbox ) {
							case HITBOX_HEAD:
								result = XorStr( "head" ); break;
							case HITBOX_NECK:
								result = XorStr( "neck" ); break;
							case HITBOX_CHEST:
							case HITBOX_LOWER_CHEST:
							case HITBOX_UPPER_CHEST:
								result = XorStr( "chest" ); break;
							case HITBOX_RIGHT_FOOT:
							case HITBOX_RIGHT_CALF:
							case HITBOX_RIGHT_THIGH:
							case HITBOX_LEFT_FOOT:
							case HITBOX_LEFT_CALF:
							case HITBOX_LEFT_THIGH:
								result = XorStr( "leg" ); break;
							case HITBOX_LEFT_FOREARM:
							case HITBOX_LEFT_HAND:
							case HITBOX_LEFT_UPPER_ARM:
							case HITBOX_RIGHT_FOREARM:
							case HITBOX_RIGHT_HAND:
							case HITBOX_RIGHT_UPPER_ARM:
								result = XorStr( "arm" ); break;
							case HITBOX_STOMACH:
								result = XorStr( "stomach" ); break;
							case HITBOX_PELVIS:
								result = XorStr( "pelvis" ); break;
							default:
								result = XorStr( "-" );
						}

						return result;
						};

					player_info_t info;

					//#ifdef _DEBUG
					if ( Source::m_pEngine->GetPlayerInfo( best_point->target->player->entindex( ), &info ) && g_Vars.esp.event_aimbot ) {
						int ping = 0;

						auto netchannel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
						if ( !netchannel.IsValid( ) )
							ping = 0;
						else
							ping = static_cast< int >( netchannel->GetLatency( FLOW_OUTGOING ) * 1000.0f );

						msg << XorStr( "fired shot at " ) << FixedStrLenght( info.szName ).c_str( );
						msg << XorStr( " | " );
						msg << TranslateHitbox( best_point->hitbox_idx ).c_str( );
						msg << XorStr( " | " );
						msg << XorStr( " bt: " ) << TIME_TO_TICKS( lagData->m_History.front( ).m_flSimulationTime - best_point->target->record->m_flSimulationTime );
						msg << XorStr( " (" ) << lagData->m_History.front( ).m_flSimulationTime - best_point->target->record->m_flSimulationTime << XorStr( "s) " );
						msg << XorStr( " | " );
						msg << XorStr( " hp: " ) << best_point->target->player->m_iHealth( );
						msg << XorStr( " | " );
						msg << XorStr( " p hp: " ) << Math::Clamp<int>( best_point->target->player->m_iHealth( ) - int( best_point->damage ), 0, best_point->target->player->m_iHealth( ) );
						msg << XorStr( " | " );
						msg << XorStr( " dmg: " ) << int( best_point->damage );
						msg << XorStr( " | " );
						msg << XorStr( " hc: " ) << best_point->hitchance;
						msg << XorStr( " | " );
						msg << XorStr( " r: " ) << g_ResolverData[ best_point->target->player->entindex( ) ].m_ResolverText;
						msg << XorStr( " | " );
						msg << XorStr( " safe: " ) << int( best_point->is_safe );
						msg << XorStr( " | " );
						msg << XorStr( " s safe: " ) << int( best_point->is_static_safe );
						msg << XorStr( " | " );
						msg << XorStr( " sp: " ) << int( *rageData->m_pSendPacket );
						msg << XorStr( " | " );
						msg << XorStr( " shot: " ) << int( best_point->target->record->m_bIsShoting );
						msg << XorStr( " | " );
						msg << XorStr( " delay: " ) << rageData->m_iDelayTicksStored;
						msg << XorStr( " | " );
						msg << XorStr( " ping: " ) << int( ping );
						msg << XorStr( " | " );
						msg << XorStr( " rt: " ) << lagData->m_iResolverMode;
						msg << XorStr( " | " );
						msg << XorStr( " miss: " ) << lagData->m_iMissedShots << ":" << lagData->m_iMissedStand1 << ":" << lagData->m_iMissedStand2 << ":" << lagData->m_iMissedStand3 <<
							":" << lagData->m_iMissedStand4 << ":" << lagData->m_iMissedStand5;

						ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 0.5f, 0.5f, 0.5f ) );
					}
					//#endif


					Engine::C_Resolver::Get( )->CreateSnapshot( best_point->target->player, rageData->m_vecEyePos, best_point->point, best_point->target->record, best_point->target->side, best_point->hitgroup );

					if ( g_Vars.esp.esp_enable && g_Vars.esp.chams_enabled && g_Vars.esp.hitmatrix )
						IChams::Get( )->AddHitmatrix( best_point->target->player, best_point->target->record->GetBoneMatrix( /*best_point->target->side*/ ) );

					if ( g_Vars.esp.draw_hitboxes ) {
						auto matrix = best_point->target->record->GetBoneMatrix( /*best_point->target->side*/ );

						auto hdr = Source::m_pModelInfo->GetStudiomodel( best_point->target->player->GetModel( ) );
						auto hitboxSet = hdr->pHitboxSet( best_point->target->player->m_nHitboxSet( ) );
						for ( int i = 0; i < hitboxSet->numhitboxes; ++i ) {
							auto hitbox = hitboxSet->pHitbox( i );
							if ( hitbox->m_flRadius <= 0.f )
								continue;

							auto min = hitbox->bbmin.Transform( matrix[ hitbox->bone ] );
							auto max = hitbox->bbmax.Transform( matrix[ hitbox->bone ] );

							Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius, g_Vars.esp.hitboxes_color.r * 255, g_Vars.esp.hitboxes_color.g * 255, g_Vars.esp.hitboxes_color.b * 255, g_Vars.esp.hitboxes_color.a * 255,
																		Source::m_pCvar->FindVar( XorStr( "sv_showlagcompensation_duration" ) )->GetFloat( ) );
						}
					}

					result = true;
				} //else {
					//g_TickbaseController.m_bSupressRecharge = false;
				//}
			} else {
				this->rageData->m_bFailedHitchance = true;
				this->rageData->m_bPrepareAim = false;
				//g_TickbaseController.m_bSupressRecharge = false;
			}
		}

		for ( auto& target : aim_targets ) {
			target.backup.Apply( target.player );
		}

		return result;
	}

	bool C_Ragebot::OverrideHitscan( C_CSPlayer* player, Engine::C_LagRecord* record, int type ) {
		auto local = C_CSPlayer::GetLocalPlayer( );
		if ( !local )
			return false;

		bool override_cond[ ] =
		{
		   rageData->rbot->override_shot,
		   rageData->rbot->override_running,
		   rageData->rbot->override_walking,
		   rageData->rbot->override_inair,
		   rageData->rbot->override_standing,
		   rageData->rbot->override_backward,
		   rageData->rbot->override_sideways,
		   rageData->rbot->override_unresolved,
		   rageData->rbot->override_induck,
		};

		bool safe_cond[ ] =
		{
		   rageData->rbot->safe_shot,
		   rageData->rbot->safe_running,
		   rageData->rbot->safe_walking,
		   rageData->rbot->safe_inair,
		   rageData->rbot->safe_standing,
		   rageData->rbot->safe_backward,
		   rageData->rbot->safe_sideways,
		   rageData->rbot->safe_unresolved,
		   rageData->rbot->safe_induck,
		};

		bool head_cond[ ] =
		{
		   rageData->rbot->prefer_head_shot,
		   rageData->rbot->prefer_head_running,
		   rageData->rbot->prefer_head_walking,
		   rageData->rbot->prefer_head_inair,
		   rageData->rbot->prefer_head_standing,
		   rageData->rbot->prefer_head_backward,
		   rageData->rbot->prefer_head_sideways,
		   rageData->rbot->prefer_head_unresolved,
		   rageData->rbot->prefer_head_induck,
		};

		bool body_cond[ ] =
		{
		   rageData->rbot->prefer_body_shot,
		   rageData->rbot->prefer_body_running,
		   rageData->rbot->prefer_body_walking,
		   rageData->rbot->prefer_body_inair,
		   rageData->rbot->prefer_body_standing,
		   rageData->rbot->prefer_body_backward,
		   rageData->rbot->prefer_body_sideways,
		   rageData->rbot->prefer_body_unresolved,
		   rageData->rbot->prefer_body_induck,
		};

		bool* current = nullptr;
		switch ( type ) {
			case 0:
				current = override_cond;
				break;
			case 1:
				current = safe_cond;
				break;
			case 2:
				current = body_cond;
				break;
			case 3:
				current = head_cond;
				break;
			default:
				return false;
				break;
		}

		// if ( current[ Unresolved ] && !record->m_iCorrectionType )
		//	 return true;

		if ( current[ InAir ] && !( record->m_iFlags & FL_ONGROUND ) )
			return true;

		if ( current[ InDuck ] && record->m_flDuckAmount > 0.f )
			return true;

		if ( current[ OnShot ] && record->m_bIsShoting )
			return true;

		if ( record->m_iFlags & FL_ONGROUND ) {
			if ( current[ Standing ] && record->m_flAnimationVelocity <= 0.1f )
				return true;

			float maxSpeed = rageData->m_pWeapon->m_weaponMode( ) == 0 ? rageData->m_pWeaponInfo->m_flMaxSpeed : rageData->m_pWeaponInfo->m_flMaxSpeed2;
			float ratio = record->m_flAnimationVelocity / maxSpeed;

			if ( current[ Running ] && ratio >= 0.52f )
				return true;

			if ( current[ Walking ] && record->m_flAnimationVelocity > 0.1f && ratio < 0.52f )
				return true;
		}

		if ( !current[ Backward ] || !current[ Sideways ] )
			return false;

		auto direction = local->m_vecOrigin( ) - record->m_vecOrigin;
		direction.Normalize( );

		auto yaw = ToDegrees( std::atan2( direction.y, direction.x ) );
		yaw = std::remainderf( yaw, 360.0f );

		auto prefer_threshold = 15.0f;

		float delta = std::fabsf( std::remainderf( yaw - record->m_flEyeYaw, 360.0f ) );
		if ( current[ Backward ] && std::fabsf( std::remainderf( delta - 180.0f, 360.0f ) ) <= prefer_threshold )
			return true;

		if ( current[ Sideways ] && delta >= 90.0f - prefer_threshold && delta <= 90.0f + prefer_threshold )
			return true;

		return false;
	}

	void C_Ragebot::ScanPointArrayMT( void* _data ) {
		auto data = ( C_PointsArray* )_data;
		auto ragebot = ( C_Ragebot* )( Ragebot::Get( ).Xor( ) );
		ragebot->ScanPointArrayInternal( data );
	}

	void C_Ragebot::ScanPointArrayInternal( C_PointsArray* data ) {
		for ( int i = 0; i < data->pointsCount; ++i )
			ScanPoint( &data->points[ i ] );
	}

	void C_Ragebot::ScanPoint( C_AimPoint* point_scan ) {
		Autowall::C_FireBulletData fireData;
		fireData.m_bPenetration = this->rageData->rbot->autowall;

		auto dir = point_scan->point - this->rageData->m_vecEyePos;
		dir.Normalize( );

		fireData.m_vecStart = this->rageData->m_vecEyePos;
		fireData.m_vecDirection = dir;
		fireData.m_iHitgroup = convert_hitbox_to_hitgroup( point_scan->hitbox_idx );
		fireData.m_Player = this->rageData->m_pLocal;
		fireData.m_TargetPlayer = point_scan->target->player;
		fireData.m_WeaponData = this->rageData->m_pWeaponInfo.Xor( );
		fireData.m_Weapon = this->rageData->m_pWeapon;

		point_scan->damage = Autowall::FireBullets( &fireData );
		point_scan->did_penetrate = fireData.m_iPenetrationCount < 4;

		if ( point_scan->damage > 0.f ) {
			point_scan->hitgroup = fireData.m_EnterTrace.hitgroup;
			point_scan->health_ratio = int( float( point_scan->target->player->m_iHealth( ) ) / point_scan->damage ) + 1;

			auto hitboxSet = ( *( studiohdr_t** )point_scan->target->player->m_pStudioHdr( ) )->pHitboxSet( point_scan->target->player->m_nHitboxSet( ) );
			auto hitbox = hitboxSet->pHitbox( point_scan->hitbox_idx );

			point_scan->is_head = point_scan->hitbox_idx == HITBOX_HEAD || point_scan->hitbox_idx == HITBOX_NECK;
			point_scan->is_body = point_scan->hitbox_idx == HITBOX_PELVIS || point_scan->hitbox_idx == HITBOX_STOMACH;
			point_scan->is_safe = IsPointSafe( point_scan->record, point_scan->point, hitbox, 3 );
			point_scan->is_static_safe = IsStaticPointSafe( point_scan->record, point_scan->point, hitbox, point_scan->target->side );

			// nospread enabled
			if ( rageData->m_flSpread == 0.0f && rageData->m_flInaccuracy == 0.0f ) {
				float spread = rageData->m_flSpread + rageData->m_flInaccuracy;

				Vector right, up;
				dir.GetVectors( right, up );

				int hits = 0;

				auto min = hitbox->bbmin.Transform( point_scan->target->player->m_CachedBoneData( ).Element( hitbox->bone ) );
				auto max = hitbox->bbmax.Transform( point_scan->target->player->m_CachedBoneData( ).Element( hitbox->bone ) );

				for ( int x = 1; x <= 4; ++x ) {
					for ( int j = 0; j < x * 5; ++j ) {
						float flSpread = spread * float( float( x ) / 4.f );

						float flDirCos, flDirSin;
						DirectX::XMScalarSinCos( &flDirCos, &flDirSin, DirectX::XM_2PI * float( float( j ) / float( x * 5 ) ) );

						float spread_x = flDirCos * flSpread;
						float spread_y = flDirSin * flSpread;

						Vector direction;
						direction.x = dir.x + ( spread_x * right.x ) + ( spread_y * up.x );
						direction.y = dir.y + ( spread_x * right.y ) + ( spread_y * up.y );
						direction.z = dir.z + ( spread_x * right.z ) + ( spread_y * up.z );

						auto end = rageData->m_vecEyePos + direction * rageData->m_pWeaponInfo->m_flWeaponRange;

						auto did_hit = false;
						if ( hitbox->m_flRadius > 0.f ) {
							did_hit = segment_to_segment( rageData->m_vecEyePos, end, min, max, hitbox->m_flRadius );
							if ( did_hit )
								hits++;
						} else {
							did_hit = Math::IntersectionBoundingBox( rageData->m_vecEyePos, end, min, max );
							if ( did_hit )
								hits++;
						}

						if ( !did_hit ) {
							for ( auto& capsule : point_scan->target->capsules ) {
								if ( capsule.m_idx != point_scan->hitbox_idx ) {
									did_hit = segment_to_segment( rageData->m_vecEyePos, end, capsule.m_mins, capsule.m_maxs, capsule.m_radius );
									if ( did_hit ) {
										hits++;
										break;
									}
								}
							}

							if ( !did_hit ) {
								for ( auto& obb : point_scan->target->obb ) {
									if ( obb.idx != point_scan->hitbox_idx ) {
										did_hit = Math::IntersectionBoundingBox( rageData->m_vecEyePos, end, obb.min, obb.max );
										if ( did_hit ) {
											hits++;
											break;
										}
									}
								}
							}
						}
					}
				}

				point_scan->hitchance = float( hits ) / 0.5f;
			} else {
				point_scan->hitchance = 100.0f;
			}
		} else {
			point_scan->health_ratio = 100;
			point_scan->hitchance = 0.0f;
		}
	}

	int C_Ragebot::GeneratePoints( C_CSPlayer* player, std::vector<C_AimTarget>& aim_targets, std::vector<C_AimPoint>& aim_points ) {
		auto lagData = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
		if ( !lagData.IsValid( ) || lagData->m_History.empty( ) || !lagData->m_History.front( ).m_bIsValid )
			return 0;

		player_info_t info;
		if ( !Source::m_pEngine->GetPlayerInfo( player->m_entIndex, &info ) )
			return 0;

		if ( player->IsDormant( ) && lagData->m_History.front( ).m_bTeleportDistance )
			return 0;

		auto animState = player->m_PlayerAnimState( );

		if ( !animState )
			return 0;

		auto renderable = player->GetClientRenderable( );
		if ( !renderable )
			return 0;

		auto model = renderable->GetModel( );
		if ( !model )
			return 0;

		auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
		if ( !hdr )
			return 0;

		Engine::C_BaseLagRecord backup;
		backup.Setup( player );

		auto record = GetBestLagRecord( player );
		if ( !record || !IsRecordValid( player, record ) ) {
			backup.Apply( player );
			return 0;
		}

		auto check_hitbox = [ this ]( C_CSPlayer* player, Engine::C_LagRecord* record, int hitboxIdx ) {
			auto renderable = player->GetClientRenderable( );
			if ( !renderable )
				return -1.0f;

			auto model = player->GetModel( );
			if ( !model )
				return -1.0f;

			auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
			if ( !hdr )
				return -1.0f;

			auto boneMatrix = player->m_CachedBoneData( ).Base( );
			auto hitboxSet = hdr->pHitboxSet( player->m_nHitboxSet( ) );
			auto point = boneMatrix[ hitboxSet->pHitbox( hitboxIdx )->bone ].at( 3 );

			Autowall::C_FireBulletData fireData;
			fireData.m_bPenetration = rageData->rbot->autowall;

			fireData.m_vecStart = rageData->m_vecEyePos;
			fireData.m_vecDirection = point - rageData->m_vecEyePos;
			fireData.m_flMaxLength = fireData.m_vecDirection.Normalize( ); // optimization 
			fireData.m_iHitgroup = convert_hitbox_to_hitgroup( hitboxIdx );

			fireData.m_Player = rageData->m_pLocal;
			fireData.m_TargetPlayer = player;
			fireData.m_WeaponData = rageData->m_pWeaponInfo.Xor( );
			fireData.m_Weapon = rageData->m_pWeapon;

			auto damage = Autowall::FireBullets( &fireData );
			return damage;

#ifdef AUTOWALL_CALLS
			AutowallCalls++;
#endif
			};

		int v36 = 0;

		static bool prev_right_side = false;
		static bool prev_left_side = false;

		float left_damage = 0.f, right_damage = 0.f;
		record->Apply( player );
		// record->Apply( player);
		// record->Apply( player );

		auto hitboxSet = hdr->pHitboxSet( player->m_nHitboxSet( ) );

		if ( !hitboxSet )
			return 0;

		auto& aim_target = aim_targets.emplace_back( );
		aim_target.player = player;
		aim_target.record = record;
		aim_target.side = GetResolverSide( player, record, aim_target.type );

		aim_target.backup = backup;
		aim_target.override_hitscan = false;
		if ( rageData->rbot->override_hitscan ) {
			if ( g_Vars.rage.override_key.enabled ) {
				aim_target.override_hitscan = true;
			} else if ( this->OverrideHitscan( player, record, 0 ) ) {
				aim_target.override_hitscan = true;
			}
		}

		record->Apply( player/*, aim_target.side*/ );

		aim_target.prefer_safe = rageData->rbot->prefer_safety && ( g_Vars.rage.prefer_safe.enabled || this->OverrideHitscan( player, record, 1 ) );
		aim_target.prefer_body = rageData->rbot->prefer_body && ( g_Vars.rage.prefer_body.enabled || this->OverrideHitscan( player, record, 2 ) );
		aim_target.prefer_head = rageData->rbot->prefer_head && ( g_Vars.rage.prefer_head.enabled || this->OverrideHitscan( player, record, 3 ) );

		auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );

		auto added_points = 0;
		for ( int i = 0; i < HITBOX_MAX; i++ ) {
			auto hitbox = hitboxSet->pHitbox( i );

			auto is_limb = hitbox->group > Hitgroup_Stomach && hitbox->group < Hitgroup_Gear;
			if ( rageData->rbot->ignorelimbs_ifwalking && record->m_vecVelocity.Length2DSquared( ) > 1.0f
				 && is_limb ) {
				continue;
			}

			if ( lag_data.IsValid( ) && lag_data->m_iMissedShots >= rageData->rbot->max_misses && is_limb ) {
				continue;
			}

			float ps = 0.0f;
			if ( !GetBoxOption( hitbox, hitboxSet, ps, aim_target.override_hitscan ) ) {
				continue;
			}

			/*if ( rageData->m_pCmd->command_number % 6 >= 3 ) {
			   if ( hitbox->group == Hitgroup_RightArm || hitbox->group == Hitgroup_RightLeg )
				  continue;
			} else if ( hitbox->group == Hitgroup_LeftArm || hitbox->group == Hitgroup_LeftLeg ) {
			   continue;
			}*/

			ps *= 0.01f;

			std::vector<Vector> vec_points;
			Multipoint( player, record, aim_target.side, vec_points, hitbox, hitboxSet, ps );
			if ( !vec_points.size( ) )
				continue;

			for ( const auto& point : vec_points ) {
				C_AimPoint& p = aim_points.emplace_back( );

				p.point = point;
				p.target = &aim_target;
				p.record = record;
				p.hitbox_idx = i;

				added_points++;
			}
		}

		return added_points;
	}

	Engine::C_LagRecord* C_Ragebot::GetBestLagRecord( C_CSPlayer* player ) {
		auto lagData = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
		if ( !lagData.IsValid( ) || lagData->m_History.empty( ) )
			return nullptr;

		auto& record = lagData->m_History.front( );
		if ( !record.m_bIsValid )
			return nullptr;


		if ( record.m_bTeleportDistance ) {
			int latency_ticks = 0;
			auto netchannel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
			if ( netchannel.IsValid( ) ) {
				latency_ticks = TIME_TO_TICKS( netchannel->GetLatency( 0 ) + netchannel->GetLatency( 1 ) ) + 1;
			}

			int tick_count = TIME_TO_TICKS( record.m_flRealTime );
			int vel_per_tick = int( float( -4096.0f / record.m_vecVelocity.Length2DSquared( ) ) );
			if ( Source::m_pGlobalVars->tickcount - TIME_TO_TICKS( record.m_flRealTime ) > 3
				 && tick_count - Source::m_pGlobalVars->tickcount - vel_per_tick > latency_ticks ) {
				return nullptr;
			}

			return &record;
		}

		bool doubleTapEnabled = g_Vars.rage.double_tap_bind.enabled && g_Vars.rage.exploit;

		int recordsCount = 0;
		Engine::C_LagRecord* arrRecords[ 64 ] = { nullptr };
		for ( auto it = lagData->m_History.begin( ); it != lagData->m_History.end( ); ++it ) {

			//if ( doubleTapEnabled ) {
			//	return &record;
			//}

			// breaking teleport distance
			if ( it->m_bSkipDueToResolver ) {
				continue;
			}

			if ( !it->m_bIsValid || !IsRecordValid( player, &*it ) ) {
				continue;
			}

			arrRecords[ recordsCount ] = &*it;
			recordsCount++;

			if ( it->m_bTeleportDistance )
				break;

			if ( recordsCount + 1 >= 64 )
				break;
		}

		if ( recordsCount <= 1 ) {
			return &record;
		}

		Engine::C_LagRecord* bestRecord = nullptr;
		Engine::C_LagRecord* bestRecord2 = nullptr;

		int best_ratio = 100;
		int currentPriority = 4;
		while ( true ) {
			Engine::C_LagRecord* prevRecord = nullptr;
			for ( int i = 0; i < recordsCount; ++i ) {
				auto rec = arrRecords[ i ];
				if ( rec->m_iRecordPriority != currentPriority )
					continue;

				if ( !prevRecord
					 || prevRecord->m_iRecordPriority != rec->m_iRecordPriority
					 || prevRecord->m_vecOrigin != rec->m_vecOrigin
					 || prevRecord->m_angAngles.yaw != rec->m_angAngles.yaw
					 || prevRecord->m_angAngles.pitch != rec->m_angAngles.pitch
					 || prevRecord->m_angAngles.roll != rec->m_angAngles.roll
					 || prevRecord->m_iFlags != rec->m_iFlags
					 || prevRecord->m_vecMaxs.z != rec->m_vecMaxs.z ) {
					auto check_hitbox = [ this ]( C_CSPlayer* player, Engine::C_LagRecord* record, int hitboxIdx ) {
						auto renderable = player->GetClientRenderable( );
						if ( !renderable )
							return 0.0f;

						auto model = player->GetModel( );
						if ( !model )
							return 0.0f;

						auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
						if ( !hdr )
							return 0.0f;

						auto boneMatrix = player->m_CachedBoneData( ).Base( );
						auto hitboxSet = hdr->pHitboxSet( player->m_nHitboxSet( ) );
						auto point = boneMatrix[ hitboxSet->pHitbox( hitboxIdx )->bone ].at( 3 );

						Autowall::C_FireBulletData fireData;
						fireData.m_bPenetration = rageData->rbot->autowall;

						fireData.m_vecStart = rageData->m_vecEyePos;
						fireData.m_vecDirection = point - rageData->m_vecEyePos;
						fireData.m_flMaxLength = fireData.m_vecDirection.Normalize( ); // optimization 	  
						fireData.m_iHitgroup = convert_hitbox_to_hitgroup( hitboxIdx );

						fireData.m_Player = rageData->m_pLocal;
						fireData.m_TargetPlayer = player;
						fireData.m_WeaponData = rageData->m_pWeaponInfo.Xor( );
						fireData.m_Weapon = rageData->m_pWeapon;

						auto damage = Autowall::FireBullets( &fireData );
						return damage;

#ifdef AUTOWALL_CALLS
						AutowallCalls++;
#endif
						};

					auto stub = 0;
					prevRecord = rec;

					g_Vars.globals.m_iRecordPriority[ player->entindex( ) ] = rec->m_iRecordPriority;

					rec->Apply( player );

					auto pelvisDamage = check_hitbox( player, rec, HITBOX_PELVIS );
					auto headDamage = check_hitbox( player, rec, HITBOX_HEAD );
					auto optDamage = std::fmaxf( headDamage, pelvisDamage );

					if ( optDamage < 1.0f ) {
						goto sup;
					}

					auto hp_ration = int( float( player->m_iHealth( ) ) / optDamage ) + 1;
					if ( hp_ration >= best_ratio ) {
sup:
						bestRecord = bestRecord2;
					} else {
						bestRecord = rec;
						best_ratio = hp_ration;
						bestRecord2 = rec;
					}
				}
			}

			if ( bestRecord ) {
				break;
			}

			currentPriority--;
			if ( currentPriority < 0 ) {
				record.Apply( player );
				return &record;
			}
		}

		if ( !bestRecord ) {
			record.Apply( player );
			return &record;
		}

		record.Apply( player );
		return bestRecord;
	}

	bool C_Ragebot::IsRecordValid( C_CSPlayer* player, Engine::C_LagRecord* record ) {
		return !Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *record, 0.2f );
	}

	int C_Ragebot::GetResolverSide( C_CSPlayer* player, Engine::C_LagRecord* record, int& type ) {
		auto lagData = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );

		auto get_side = [ ]( int s ) {
			switch ( s ) {
				case RESOLVER_SIDE_LEFT:
					return ANIMATION_RESOLVER_LEFT;
					break;
				case RESOLVER_SIDE_RIGHT:
					return ANIMATION_RESOLVER_RIGHT;
					break;
				default:
				case RESOLVER_SIDE_FAKE:
					return ANIMATION_RESOLVER_FAKE;
					break;
			}
			};

		type = 0;

		player_info_t player_info;
		if ( !Source::m_pEngine->GetPlayerInfo( player->m_entIndex, &player_info ) ) {
			return 0;
		}

		if ( player_info.fakeplayer )
			return 0;

		return g_BruteforceData[ player->m_entIndex ].GetYawSide( player->m_entIndex, *record );
	}

	bool C_Ragebot::IsStaticPointSafe( Engine::C_LagRecord* record, const Vector& point, mstudiobbox_t* hitbox, int side ) {
		auto end = point - rageData->m_vecEyePos;
		auto dir = end.Normalized( );
		end = rageData->m_vecEyePos + dir * 8092.0f;

		int hits = 0;

		if ( hitbox->m_flRadius <= 0.0f ) {
			if ( side > 0 || side < 0 ) {
				auto boneMatrix = record->GetBoneMatrix( );
				auto min = hitbox->bbmin.Transform( boneMatrix[ hitbox->bone ] );
				auto max = hitbox->bbmax.Transform( boneMatrix[ hitbox->bone ] );
				hits += Math::IntersectionBoundingBox( rageData->m_vecEyePos, end, min, max );

				boneMatrix = record->GetBoneMatrix( );
				min = hitbox->bbmin.Transform( boneMatrix[ hitbox->bone ] );
				max = hitbox->bbmax.Transform( boneMatrix[ hitbox->bone ] );
				hits += Math::IntersectionBoundingBox( rageData->m_vecEyePos, end, min, max );
			}
		} else {
			if ( side > 0 || side < 0 ) {
				auto boneMatrix = record->GetBoneMatrix( );
				auto min = hitbox->bbmin.Transform( boneMatrix[ hitbox->bone ] );
				auto max = hitbox->bbmax.Transform( boneMatrix[ hitbox->bone ] );
				hits += segment_to_segment( rageData->m_vecEyePos, end, min, max, hitbox->m_flRadius );

				boneMatrix = record->GetBoneMatrix( );
				min = hitbox->bbmin.Transform( boneMatrix[ hitbox->bone ] );
				max = hitbox->bbmax.Transform( boneMatrix[ hitbox->bone ] );
				hits += segment_to_segment( rageData->m_vecEyePos, end, min, max, hitbox->m_flRadius );
			}
		}

		return hits >= 2;
	}

	bool C_Ragebot::IsPointSafe( Engine::C_LagRecord* record, const Vector& point, mstudiobbox_t* hitbox, int minHits ) {
		auto end = point - rageData->m_vecEyePos;
		auto dir = end.Normalized( );
		end = rageData->m_vecEyePos + dir * 8092.0f;

		int hits = 0;
		if ( hitbox->m_flRadius <= 0.0f ) {
			Vector dir = point - rageData->m_vecEyePos;
			for ( int i = -1; i <= 1; ++i ) {
				auto boneMatrix = record->GetBoneMatrix( );
				auto min = hitbox->bbmin.Transform( boneMatrix[ hitbox->bone ] );
				auto max = hitbox->bbmax.Transform( boneMatrix[ hitbox->bone ] );
				hits += Math::IntersectionBoundingBox( rageData->m_vecEyePos, end, min, max );
			}
		} else {
			for ( int i = -1; i <= 1; ++i ) {
				auto boneMatrix = record->GetBoneMatrix( );
				auto min = hitbox->bbmin.Transform( boneMatrix[ hitbox->bone ] );
				auto max = hitbox->bbmax.Transform( boneMatrix[ hitbox->bone ] );
				hits += segment_to_segment( rageData->m_vecEyePos, end, min, max, hitbox->m_flRadius );
			}
		}

		return hits >= minHits;
	}

	bool C_Ragebot::AimAtPoint( C_AimPoint* best_point ) {
		C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );
		if ( !pLocal && !pLocal->IsDead( ) )
			return false;

		C_WeaponCSBaseGun* pWeapon = ( C_WeaponCSBaseGun* )pLocal->m_hActiveWeapon( ).Get( );
		if ( !pWeapon )
			return false;

		rageData->m_pCmd->buttons &= ~IN_USE;

		// todo: aimstep
		Vector delta = best_point->point - rageData->m_vecEyePos;
		delta.Normalize( );

		QAngle aimAngles = delta.ToEulerAngles( );
		aimAngles.Normalize( );

#if 0
		if ( rageData->rbot->on_shot_aa ) {
			bool tickbase_hide = g_Vars.rage.exploit && g_Vars.rage.exploit_type == 0 && ( g_Vars.rage.double_tap_bind.enabled || g_Vars.rage.hide_shots_bind.enabled );
			bool can_hide_shot = false;
			if ( g_Vars.globals.Fakeducking )
				can_hide_shot = *rageData->m_pSendPacket == false;
			else
				can_hide_shot = Source::m_pClientState->m_nChokedCommands( ) < g_Vars.fakelag.lag_limit;

			if ( !tickbase_hide && can_hide_shot ) {
				auto animState = rageData->m_pLocal->m_PlayerAnimState( );
				auto fraction = animState->GetMaxFraction( );
				auto maxYawModifier = fraction * animState->m_flMaxBodyYaw;
				auto minYawModifier = fraction * animState->m_flMinBodyYaw;

				auto get_clamp = [ animState, maxYawModifier, minYawModifier ]( float yaw, float aim_angle ) {
					auto eye_feet_delta = std::remainderf( aim_angle - yaw, 360.0f );
					if ( eye_feet_delta <= maxYawModifier ) {
						if ( minYawModifier > eye_feet_delta )
							return 1;
					} else {
						return -1;
					}

					if ( eye_feet_delta == 0.0f ) {
						return 0;
					}

					return 2 * ( eye_feet_delta <= 0.f ) - 1;
					};

				auto real_angle = std::remainderf( animState->m_flAbsRotation, 360.0f );
				float aim_angle = std::remainderf( aimAngles.yaw, 360.0f );
				int fake_side = get_clamp( std::remainderf( g_Vars.globals.m_FakeAngles.yaw, 360.0f ), aim_angle );
				int real_side = get_clamp( real_angle, aim_angle );

				if ( real_side == fake_side && real_side != 0 ) {
#if 1
					auto sequence_num = rageData->m_pCmd->command_number - 1;
					auto prev_cmd = &Source::m_pInput->m_pCommands[ sequence_num % 150 ];

					if ( Source::m_pClientState->m_nChokedCommands( ) > 0 && !( prev_cmd->buttons & IN_ATTACK ) ) {
						auto backup_angle = prev_cmd->viewangles;
						prev_cmd->viewangles.yaw = std::remainderf( aimAngles.yaw + 120.0f * real_side, 360.0f );

						RotateMovement( prev_cmd, prev_cmd->viewangles, backup_angle );

						auto verifed_prev_cmd = &Source::m_pInput->m_pVerifiedCommands[ sequence_num % 150 ];
						verifed_prev_cmd->m_crc = prev_cmd->GetChecksum( );
						verifed_prev_cmd->m_cmd = *prev_cmd;
					}
#endif

					{
#ifdef DEBUG_ONSHOT_AA
						printf( XorStr( "real_side: %d | fake_side %d | real_yaw: %.1f | fake_yaw: %.1f | aim_yaw: %.1f" ), real_side, fake_side,
								std::remainderf( animState->m_flAbsRotation, 360.0f ),
								std::remainderf( g_Vars.globals.m_FakeAngles.yaw, 360.0f ), std::remainderf( aimAngles.yaw, 360.0f ) );
#endif

						real_side = -fake_side;

#ifdef DEBUG_ONSHOT_AA
						printf( " | inverted to %d", real_side );
#endif
						auto max_dsc = maxYawModifier + std::fabsf( minYawModifier );
						auto target_angle = std::remainderf( aim_angle + max_dsc * real_side, 360.0f );

#ifdef DEBUG_ONSHOT_AA
						printf( " | %.1f", target_angle );
#endif

						rageData->m_pCmd->viewangles.yaw = target_angle;

						*rageData->m_pSendPacket = false;
						g_Vars.globals.HideShots = true;

#ifdef DEBUG_ONSHOT_AA
						printf( "\n" );
#endif
					}
				}
			}
		}
#endif


		if ( !rageData->rbot->silent_aim )
			Source::m_pEngine->SetViewAngles( aimAngles );

		auto result = true;

		if ( ( g_Vars.globals.m_iServerType == 1 && g_Vars.globals.m_iGameMode == 1 ) && previous_cmd ) {
			auto delta = aimAngles - previous_cmd->viewangles;
			delta.Normalize( );

			auto len = std::sqrtf( delta.x * delta.x + delta.y * delta.y + delta.z * delta.z );
			if ( len > 29.0f ) {
				aimAngles = ( ( delta / len ) * 29.0f ) + previous_cmd->viewangles;
				aimAngles.Normalize( );
				result = false;
			} else {
				rageData->m_pCmd->viewangles = aimAngles;
				rageData->m_pCmd->viewangles.Normalize( );
			}
		} else {
			rageData->m_pCmd->viewangles = aimAngles;
			rageData->m_pCmd->viewangles.Normalize( );
		}

		// if ( !TickbaseShiftCtx.in_rapid ) {
		// 	if ( best_point->target->record->m_bTeleportDistance || !g_Vars.globals.Fakeducking ) {
		// 		*rageData->m_pSendPacket = true;
		// 	}
		// }

		g_Vars.globals.CorrectShootPosition = true;
		g_Vars.globals.AimPoint = best_point->point;
		g_Vars.globals.ShootPosition = rageData->m_vecEyePos;

		rageData->m_iChokedCommands = -1;
		rageData->m_bFailedHitchance = false;


		if ( result ) {
			//if ( g_Vars.rage.double_tap_bind.enabled && g_Vars.rage.exploit ) {
				//g_TickbaseController.m_bSupressRecharge = true;
			//}

			rageData->m_pCmd->buttons |= IN_ATTACK;
		}

#ifdef DEBUG_REPREDICT
		std::get<0>( predicted_origins[ rageData->m_pCmd->command_number ] ) = rageData->m_pLocal->m_vecOrigin( );
		std::get<1>( predicted_origins[ rageData->m_pCmd->command_number ] ) = rageData->m_pLocal->m_vecVelocity( );
#endif

		return result;
	}

	static C_Ragebot instance;
	Encrypted_t<Ragebot> Source::Ragebot::Get( ) {
		return &instance;
	}
}
