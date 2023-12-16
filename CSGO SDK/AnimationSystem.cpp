#include "AnimationSystem.hpp"
#include "displacement.hpp"
#include "Math.h"
#include "SetupBones.hpp"
#include "Utils\threading.h"
#include "Resolver.hpp"
#define MT_SETUP_BONES

namespace Engine {
	struct SimulationRestore {
		int m_fFlags;
		float m_flDuckAmount;
		float m_flFeetCycle;
		float m_flFeetYawRate;
		QAngle m_angEyeAngles;
		Vector m_vecOrigin;

		void Setup( C_CSPlayer* player ) {
			m_fFlags = player->m_fFlags( );
			m_flDuckAmount = player->m_flDuckAmount( );
			// m_angEyeAngles = player->m_angEyeAngles( );
			m_vecOrigin = player->m_vecOrigin( );

			auto animState = player->m_PlayerAnimState( );
			m_flFeetCycle = animState->m_flFeetCycle;
			m_flFeetYawRate = animState->m_flFeetYawRate;
		}

		void Apply( C_CSPlayer* player ) const {
			player->m_fFlags( ) = m_fFlags;
			player->m_flDuckAmount( ) = m_flDuckAmount;
			// player->m_angEyeAngles( ) = m_angEyeAngles;
			player->m_vecOrigin( ) = m_vecOrigin;

			auto animState = player->m_PlayerAnimState( );
			animState->m_flFeetCycle = m_flFeetCycle;
			animState->m_flFeetYawRate = m_flFeetYawRate;
		}
	};

	struct AnimationBackup {
		CCSGOPlayerAnimState anim_state;
		C_AnimationLayer layers[ 13 ];
		float pose_params[ 19 ];

		AnimationBackup( ) {

		}

		void Apply( C_CSPlayer* player ) const;
		void Setup( C_CSPlayer* player );
	};

	void AnimationBackup::Apply( C_CSPlayer* player ) const {
		*player->m_PlayerAnimState( ) = this->anim_state;
		std::memcpy( player->m_AnimOverlay( ).m_Memory.m_pMemory, layers, sizeof( layers ) );
		std::memcpy( player->m_flPoseParameter( ), pose_params, sizeof( pose_params ) );
	}

	void AnimationBackup::Setup( C_CSPlayer* player ) {
		this->anim_state = *player->m_PlayerAnimState( );
		std::memcpy( layers, player->m_AnimOverlay( ).m_Memory.m_pMemory, sizeof( layers ) );
		std::memcpy( pose_params, player->m_flPoseParameter( ), sizeof( pose_params ) );
	}

	inline void FixBonesRotations( C_CSPlayer* player, matrix3x4_t* bones ) {
		// copypasted from supremacy/fatality, no difference imo
		// also seen that in aimware multipoints, but was lazy to paste, kek
		auto studio_hdr = player->m_pStudioHdr( );
		if ( studio_hdr ) {
			auto hdr = *( studiohdr_t** )studio_hdr;
			if ( hdr ) {
				auto hitboxSet = hdr->pHitboxSet( player->m_nHitboxSet( ) );
				for ( int i = 0; i < hitboxSet->numhitboxes; ++i ) {
					auto hitbox = hitboxSet->pHitbox( i );
					if ( hitbox->m_angAngles.IsZero( ) )
						continue;

					matrix3x4_t hitboxTransform;
					hitboxTransform.AngleMatrix( hitbox->m_angAngles );
					bones[ hitbox->bone ] = bones[ hitbox->bone ].ConcatTransforms( hitboxTransform );
				}
			}
		}
	}

	static void MultithreadedSetupBones( Engine::C_SetupBones* data ) {
		alignas( 16 ) Vector pos[ MAXSTUDIOBONES ];
		alignas( 16 ) Quaternion q[ MAXSTUDIOBONES ];

		data->m_vecBones = pos;
		data->m_quatBones = q;
		data->m_bShouldDoIK = false;
		data->m_flCurtime = data->m_animating->m_flSimulationTime( );
		data->m_bShouldAttachment = false;

		if ( g_Vars.rage.enabled &&
			 ( g_Vars.rage.visual_resolver
			   || g_Vars.esp.chams_enabled && g_Vars.esp.hitmatrix ) ) {
			data->m_boneMask = BONE_USED_BY_ANYTHING & ~BONE_USED_BY_BONE_MERGE;
		}

		data->Setup( );
		FixBonesRotations( data->m_animating, data->m_boneMatrix );
		delete data;

		// SetupBonesSem.Post( );
	}


#if 1
	auto GetServerEdict( int index ) -> std::uint8_t* {
		static uintptr_t pServerGlobals = **( uintptr_t** )( Memory::Scan( "server.dll", "8B 15 ? ? ? ? 33 C9 83 7A 18 01" ) + 0x2 );
		int iMaxClients = *( int* )( ( uintptr_t )pServerGlobals + 0x18 );
		if ( iMaxClients >= index ) {
			if ( index <= iMaxClients ) {
				int v10 = index * 16;
				uintptr_t v11 = *( uintptr_t* )( pServerGlobals + 96 );
				if ( v11 ) {
					if ( !( ( *( uintptr_t* )( v11 + v10 ) >> 1 ) & 1 ) ) {
						uintptr_t v12 = *( uintptr_t* )( v10 + v11 + 12 );
						if ( v12 ) {
							uint8_t* pReturn = nullptr;

							// abusing asm is not good
							__asm
							{
								pushad
								mov ecx, v12
								mov eax, dword ptr[ ecx ]
								call dword ptr[ eax + 0x14 ]
								mov pReturn, eax
								popad
							}

							return pReturn;
						}
					}
				}
			}
		}
		return nullptr;
	}
#endif

	static void MultithreadedRenderSetupBonesGeneral( Engine::C_SetupBones* data ) {
		alignas( 16 ) Vector pos[ MAXSTUDIOBONES ];
		alignas( 16 ) Quaternion q[ MAXSTUDIOBONES ];



		data->m_vecBones = pos;
		data->m_quatBones = q;
		data->m_bShouldDoIK = false;
		data->m_flCurtime = data->m_animating->m_flSimulationTime( );
		data->m_bShouldAttachment = false;
		data->m_pHdr = data->m_animating->m_pStudioHdr( );
		data->m_boneMask &= ~BONE_USED_BY_BONE_MERGE;
		data->Setup( );

		std::memcpy( data->m_animating->m_CachedBoneData( ).Base( ), data->m_boneMatrix, sizeof( matrix3x4_t ) * data->m_animating->m_CachedBoneData( ).Count( ) );
		if ( data->m_animating->m_CachedBoneData( ).Base( ) != data->m_animating->m_BoneAccessor( ).m_pBones )
			std::memcpy( data->m_animating->m_BoneAccessor( ).m_pBones, data->m_boneMatrix, sizeof( matrix3x4_t ) * data->m_animating->m_CachedBoneData( ).Count( ) );

		data->AttachmentHelper( data->m_animating, data->m_pHdr );

		FixBonesRotations( data->m_animating, data->m_boneMatrix );

		delete data;
	}

	class C_AnimationSystem : public AnimationSystem {
	public:
		virtual void CollectData( );
		virtual void Update( );

		virtual C_AnimationData* GetAnimationData( int index ) {
			if ( m_AnimatedEntities.count( index ) < 1 )
				return nullptr;

			return &m_AnimatedEntities[ index ];
		}

		std::map<int, C_AnimationData> m_AnimatedEntities = {};

		C_AnimationSystem( ) { };
		virtual ~C_AnimationSystem( ) { };
	};

	Encrypted_t<AnimationSystem> AnimationSystem::Get( ) {
		static C_AnimationSystem instance;
		return &instance;
	}

	void C_AnimationSystem::CollectData( ) {
		if ( !Source::m_pEngine->IsInGame( ) || !Source::m_pEngine->GetNetChannelInfo( ) ) {
			this->m_AnimatedEntities.clear( );
			return;
		}

		auto local = C_CSPlayer::GetLocalPlayer( );
		if ( !local || !g_Vars.globals.HackIsReady )
			return;

		for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
			auto player = C_CSPlayer::GetPlayerByIndex( i );
			if ( !player || player == local )
				continue;

			player_info_t player_info;
			if ( !Source::m_pEngine->GetPlayerInfo( player->m_entIndex, &player_info ) ) {
				continue;
			}

			this->m_AnimatedEntities[ i ].Collect( player );
		}
	}

	void C_AnimationSystem::Update( ) {
		if ( !Source::m_pEngine->IsInGame( ) || !Source::m_pEngine->GetNetChannelInfo( ) ) {
			this->m_AnimatedEntities.clear( );
			return;
		}

		auto local = C_CSPlayer::GetLocalPlayer( );
		if ( !local || !g_Vars.globals.HackIsReady )
			return;

		for ( auto& [key, value] : this->m_AnimatedEntities ) {
			auto entity = C_CSPlayer::GetPlayerByIndex( key );
			if ( !entity )
				continue;

			auto realtime = Source::m_pGlobalVars->realtime;
			auto curtime = Source::m_pGlobalVars->curtime;
			auto frametime = Source::m_pGlobalVars->frametime;
			auto absoluteframetime = Source::m_pGlobalVars->absoluteframetime;
			auto framecount = Source::m_pGlobalVars->framecount;
			auto tickcount = Source::m_pGlobalVars->tickcount;
			auto interpolation_amount = Source::m_pGlobalVars->interpolation_amount;

			float time = entity->m_flSimulationTime( );
			int ticks = TIME_TO_TICKS( time );

			Source::m_pGlobalVars->realtime = time;
			Source::m_pGlobalVars->curtime = time;
			Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;
			Source::m_pGlobalVars->absoluteframetime = Source::m_pGlobalVars->interval_per_tick;
			Source::m_pGlobalVars->framecount = ticks;
			Source::m_pGlobalVars->tickcount = ticks;
			Source::m_pGlobalVars->interpolation_amount = 0.f;

			if ( value.m_bUpdated )
				value.Update( );

			Source::m_pGlobalVars->realtime = realtime;
			Source::m_pGlobalVars->curtime = curtime;
			Source::m_pGlobalVars->frametime = frametime;
			Source::m_pGlobalVars->absoluteframetime = absoluteframetime;
			Source::m_pGlobalVars->framecount = framecount;
			Source::m_pGlobalVars->tickcount = tickcount;
			Source::m_pGlobalVars->interpolation_amount = interpolation_amount;

			value.m_bUpdated = false;
		}

	}

	void C_AnimationData::Update( ) {
		if ( !this->player || this->m_AnimationRecord.size( ) < 1 )
			return;

		C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
		if ( !local )
			return;

		auto record = Encrypted_t<Engine::C_AnimationRecord>( &this->m_AnimationRecord.front( ) );
		record->m_bAnimationResolverUsed = false;

		Encrypted_t<Engine::C_AnimationRecord> previous_record( nullptr );
		if ( this->m_AnimationRecord.size( ) > 1 ) {
			previous_record = &this->m_AnimationRecord.at( 1 );
		}

		C_SetupBones::Allocate( player, false, true );

		this->player->m_vecVelocity( ) = record->m_vecVelocity;

		auto animState = player->m_PlayerAnimState( );

		C_SetupBones* server = new C_SetupBones( );

		auto weapon = ( C_BaseAttributableItem* )player->m_hActiveWeapon( ).Get( );
		auto weaponWorldModel = weapon ? ( C_CSPlayer* )( weapon )->m_hWeaponWorldModel( ).Get( ) : nullptr;

		if ( !animState )
			return;

		server->m_animating = player;
		server->m_vecOrigin = player->m_vecOrigin( );
		server->m_animLayers = record->m_serverAnimOverlays;
		server->m_pHdr = player->m_pStudioHdr( );

		this->m_bBonesCalculated = false;
		record->m_bNoFakeAngles = true;

		SimulateAnimations( record, previous_record );

		this->m_Animations[ 0 ].Update( player );

		std::memcpy( record->fakeLayersFake, player->m_AnimOverlay( ).Base( ), 13 * sizeof( C_AnimationLayer ) );
		std::memcpy( player->m_AnimOverlay( ).Base( ), record->m_serverAnimOverlays, 13 * sizeof( C_AnimationLayer ) );

		record->m_flAbsRotation = this->m_Animations[ 0 ].m_flAbsRotation;

		server->m_nAnimOverlayCount = std::min( 13, player->m_AnimOverlay( ).Count( ) );

		server->m_angAngles = QAngle( 0.0f, this->m_Animations[ 0 ].m_flAbsRotation, 0.0f );
		server->m_boneMatrix = m_Animations[ 0 ].m_Bones;
		std::memcpy( server->m_flPoseParameters, player->m_flPoseParameter( ), sizeof( server->m_flPoseParameters ) );

		if ( weaponWorldModel ) {
			std::memcpy( server->m_flWorldPoses, weaponWorldModel->m_flPoseParameter( ), sizeof( server->m_flWorldPoses ) );
		}

		server->m_boneMask = BONE_USED_BY_ANYTHING & ~BONE_USED_BY_BONE_MERGE;

		Threading::QueueJobRef( MultithreadedRenderSetupBonesGeneral, server );

		this->m_vecSimulationData.clear( );
		this->m_iResolverSide = record->m_iResolverSide;
	}

	void C_AnimationData::Collect( C_CSPlayer* player ) {
		if ( player->IsDead( ) )
			player = nullptr;

		auto pThis = Encrypted_t<C_AnimationData>( this );

		if ( pThis->player != player ) {
			pThis->m_flSpawnTime = 0.0f;
			pThis->m_flSimulationTime = 0.0f;
			pThis->m_flOldSimulationTime = 0.0f;
			pThis->m_iCurrentTickCount = 0;
			pThis->m_iOldTickCount = 0;
			pThis->m_iTicksAfterDormancy = 0;
			pThis->m_vecSimulationData.clear( );
			pThis->m_AnimationRecord.clear( );
			pThis->m_bAimbotTarget = pThis->m_bSuppressAnimationResolver = pThis->m_bIsDormant = pThis->m_bBonesCalculated = false;
			pThis->player = player;
			pThis->m_bIsAlive = false;
		}

		if ( !player )
			return;

		pThis->m_bIsAlive = true;
		pThis->m_flOldSimulationTime = pThis->m_flSimulationTime;
		pThis->m_flSimulationTime = pThis->player->m_flSimulationTime( );

		if (/* pThis->m_flSimulationTime == 0.0f ||*/ pThis->player->IsDormant( ) ) {
			pThis->m_bIsDormant = true;
			return;
		}

		if ( pThis->m_flOldSimulationTime == pThis->m_flSimulationTime ) {
			return;
		}

		if ( pThis->m_bIsDormant ) {
			pThis->m_iTicksAfterDormancy = 0;
			pThis->m_AnimationRecord.clear( );
		} else if ( pThis->m_bAimbotTarget ) {
			auto tickrate = int( 1.0f / Source::m_pGlobalVars->interval_per_tick );

			if ( std::fabsf( std::remainderf( player->m_angEyeAngles( ).yaw - pThis->m_flLastScannedYaw, 360.f ) ) < 35.0f )
				pThis->m_iLastLowDeltaTick = Source::m_pGlobalVars->tickcount;

			if ( std::abs( Source::m_pGlobalVars->tickcount - pThis->m_iLastLowDeltaTick ) > 2 * tickrate )
				pThis->m_bAimbotTarget = false;
		}

		pThis->ent_index = player->m_entIndex;

		pThis->m_bUpdated = true;
		pThis->m_bIsDormant = false;

		pThis->m_iOldTickCount = pThis->m_iCurrentTickCount;
		pThis->m_iCurrentTickCount = Source::m_pGlobalVars->tickcount;

		if ( pThis->m_flSpawnTime != pThis->player->m_flSpawnTime( ) ) {
			auto animState = pThis->player->m_PlayerAnimState( );
			if ( animState ) {
				animState->m_Player = pThis->player;
				animState->Reset( );
			}

			pThis->m_flSpawnTime = pThis->player->m_flSpawnTime( );
		}

		int tickrate = int( 1.0f / Source::m_pGlobalVars->interval_per_tick );
		while ( pThis->m_AnimationRecord.size( ) > tickrate ) {
			pThis->m_AnimationRecord.pop_back( );
		}

		pThis->m_iTicksAfterDormancy++;

		Encrypted_t<C_AnimationRecord> previous_record = nullptr;
		Encrypted_t<C_AnimationRecord> penultimate_record = nullptr;

		if ( pThis->m_AnimationRecord.size( ) > 0 ) {
			previous_record = &pThis->m_AnimationRecord.front( );
			if ( pThis->m_AnimationRecord.size( ) > 1 ) {
				penultimate_record = &pThis->m_AnimationRecord.at( 1 );
			}
		}

		auto record = &pThis->m_AnimationRecord.emplace_front( );

		// fix netvar compression
		if ( player->m_angEyeAngles( ).pitch == 88.9947510f )
			player->m_angEyeAngles( ).pitch = 89.0f;

		pThis->m_vecOrigin = pThis->player->m_vecOrigin( );

		record->m_flLowerBodyYawTarget = pThis->player->m_flLowerBodyYawTarget( );
		record->m_vecOrigin = pThis->player->m_vecOrigin( );
		record->m_angEyeAngles = pThis->player->m_angEyeAngles( );
		record->m_flSimulationTime = pThis->m_flSimulationTime;

		auto weapon = ( C_WeaponCSBaseGun* )( player->m_hActiveWeapon( ).Get( ) );

		if ( weapon ) {
			auto weaponWorldModel = ( C_CSPlayer* )( ( C_BaseAttributableItem* )weapon )->m_hWeaponWorldModel( ).Get( );

			for ( int i = 0; i < player->m_AnimOverlay( ).Count( ); ++i ) {
				player->m_AnimOverlay( ).Element( i ).m_pOwner = player;
				player->m_AnimOverlay( ).Element( i ).m_pStudioHdr = player->m_pStudioHdr( );

				if ( weaponWorldModel ) {
					if ( player->m_AnimOverlay( ).Element( i ).m_nSequence < 2 || player->m_AnimOverlay( ).Element( i ).m_flWeight <= 0.0f )
						continue;

					using UpdateDispatchLayer = void( __thiscall* )( void*, C_AnimationLayer*, CStudioHdr*, int );
					Memory::VCall< UpdateDispatchLayer >( player, 241 )( player, &player->m_AnimOverlay( ).Element( i ),
																		 weaponWorldModel->m_pStudioHdr( ), player->m_AnimOverlay( ).Element( i ).m_nSequence );
				}
			}
		}

		std::memcpy( record->m_serverAnimOverlays, pThis->player->m_AnimOverlay( ).Base( ), sizeof( record->m_serverAnimOverlays ) );

		record->m_flFeetCycle = record->m_serverAnimOverlays[ 6 ].m_flCycle;
		record->m_flFeetYawRate = record->m_serverAnimOverlays[ 6 ].m_flWeight;

		record->m_fFlags = player->m_fFlags( );
		record->m_flDuckAmount = player->m_flDuckAmount( );

		record->m_bIsShoting = false;
		record->m_bFakeWalking = false;
		record->m_flShotTime = 0.0f;

		if ( previous_record.IsValid( ) ) {
			record->m_flChokeTime = pThis->m_flSimulationTime - pThis->m_flOldSimulationTime;
			record->m_iChokeTicks = TIME_TO_TICKS( record->m_flChokeTime );
		} else {
			record->m_flChokeTime = Source::m_pGlobalVars->interval_per_tick;
			record->m_iChokeTicks = 1;
		}

		if ( !previous_record.IsValid( ) ) {
			record->m_bIsInvalid = true;
			record->m_vecVelocity.Init( );
			record->m_bIsShoting = false;
			record->m_bTeleportDistance = false;

			auto animstate = player->m_PlayerAnimState( );
			if ( animstate )
				animstate->m_flAbsRotation = record->m_angEyeAngles.yaw;

			return;
		}

		auto prev_simulation_time = previous_record->m_flSimulationTime;
		auto tickcount_delta = pThis->m_iCurrentTickCount - pThis->m_iOldTickCount;
		auto simticks_delta = record->m_iChokeTicks;
		auto choked_ticks_unk = simticks_delta;
		auto shifted_tickbase = false;
		if ( pThis->m_flOldSimulationTime > pThis->m_flSimulationTime ) {
			record->m_bShiftingTickbase = true;
			record->m_iChokeTicks = tickcount_delta;
			record->m_flChokeTime = TICKS_TO_TIME( record->m_iChokeTicks );
			prev_simulation_time = record->m_flSimulationTime - record->m_flChokeTime;
			choked_ticks_unk = tickcount_delta;
			shifted_tickbase = true;
		}

		if ( shifted_tickbase || abs( simticks_delta - tickcount_delta ) <= 2 ) {
			if ( choked_ticks_unk ) {
				if ( choked_ticks_unk != 1 ) {
					pThis->m_iTicksUnknown = 0;
				} else {
					pThis->m_iTicksUnknown++;
				}
			} else {
				record->m_iChokeTicks = 1;
				record->m_flChokeTime = Source::m_pGlobalVars->interval_per_tick;

				prev_simulation_time = record->m_flSimulationTime - Source::m_pGlobalVars->interval_per_tick;

				pThis->m_iTicksUnknown++;
			}
		}

		if ( weapon ) {
			record->m_flShotTime = weapon->m_fLastShotTime( );
			record->m_bIsShoting = record->m_flSimulationTime >= record->m_flShotTime && record->m_flShotTime > previous_record->m_flSimulationTime;
		}

		record->m_bIsInvalid = false;

		// fix velocity
		auto velocity = Vector( 0.0f, 0.0f, 0.0f );
		if ( !( player->m_fEffects( ) & EF_NOINTERP ) ) {
			float maxSpeed = 250.0f;
			if ( weapon ) {
				auto weapon_data = weapon->GetCSWeaponData( );
				if ( weapon_data.IsValid( ) ) {
					maxSpeed = weapon->m_weaponMode( ) == 0 ? weapon_data->m_flMaxSpeed : weapon_data->m_flMaxSpeed2;
				}
			}

			float anim_speed = FLT_MAX;
			auto weapon = ( C_WeaponCSBaseGun* )( player->m_hActiveWeapon( ).Get( ) );
			if ( weapon && player->m_fFlags( ) & FL_ONGROUND ) {
				auto weapon_data = weapon->GetCSWeaponData( );
				if ( weapon_data.IsValid( ) ) {
					if ( *( bool* )( uintptr_t( player ) + Engine::Displacement.DT_CSPlayer.m_bIsWalking ) && record->m_serverAnimOverlays[ 6 ].m_flWeight < 0.95f ) {
						anim_speed = weapon->m_weaponMode( ) == 0 ? weapon_data->m_flMaxSpeed : weapon_data->m_flMaxSpeed2;
						anim_speed *= 0.52f;

						// current server anim speed
						anim_speed = anim_speed * record->m_serverAnimOverlays[ 6 ].m_flWeight;
					}
				}
			}

			const auto origin_delta = record->m_vecOrigin - previous_record->m_vecOrigin;
			if ( !origin_delta.IsZero( ) ) {
				velocity = origin_delta / record->m_flChokeTime;
				// calculate average direction if player is fakelagging
				if ( record->m_iChokeTicks > 1 && penultimate_record.IsValid( ) ) {
					const auto prev_origin_delta = previous_record->m_vecOrigin - penultimate_record->m_vecOrigin;
					if ( !prev_origin_delta.IsZero( ) && prev_origin_delta != origin_delta ) {
						const auto curr_direction = ToDegrees( atan2( origin_delta.y, origin_delta.x ) );
						const auto prev_direction = ToDegrees( atan2( prev_origin_delta.y, prev_origin_delta.x ) );

						auto average_direction = std::remainderf( curr_direction - prev_direction, 360.0f );
						average_direction = ToRadians( std::remainderf( curr_direction + average_direction * 0.5f, 360.0f ) );

						float dir_sin, dir_cos;
						DirectX::XMScalarSinCos( &dir_sin, &dir_cos, average_direction );

						const auto current_speed = ( anim_speed == FLT_MAX ) ? velocity.Length2D( ) : anim_speed;
						velocity.x = dir_cos * current_speed;
						velocity.y = dir_sin * current_speed;
					}
				}
			}

			// fix CGameMovement::FinishGravity
			if ( !( player->m_fFlags( ) & FL_ONGROUND ) )
				velocity.z -= g_Vars.sv_gravity->GetFloat( ) * record->m_flChokeTime * 0.5f;
			else
				velocity.z = 0.0f;


			if ( record->m_vecVelocity.Length( ) > 1.f &&
				 record->m_iChokeTicks >= 12 &&
				 record->m_serverAnimOverlays[ 12 ].m_flWeight == 0.0f &&
				 record->m_serverAnimOverlays[ 6 ].m_flWeight == 0.0f &&
				 record->m_serverAnimOverlays[ 6 ].m_flPlaybackRate < 0.0001f &&
				 ( record->m_fFlags & FL_ONGROUND ) ) {
				record->m_bFakeWalking = true;
			}
		}

		if ( !record->m_bShiftingTickbase
			 && velocity.Length2DSquared( ) > 0.1f
			 && record->m_serverAnimOverlays[ 12 ].m_flWeight == 0.0f
			 && record->m_serverAnimOverlays[ 6 ].m_flWeight <= 0.01f
			 && record->m_fFlags & FL_ONGROUND ) {
			velocity.Init( );
		}

		record->m_vecVelocity = velocity;
		record->m_bTeleportDistance = record->m_vecOrigin.DistanceSquared( previous_record->m_vecOrigin ) > 4096.0f;

		if ( record->m_iChokeTicks > 1 ) {
			// create simulation info
			Vector deltaOrigin = ( record->m_vecOrigin - previous_record->m_vecOrigin ) / record->m_iChokeTicks;
			Vector deltaVel = ( record->m_vecVelocity - previous_record->m_vecVelocity ) / record->m_iChokeTicks;
			float deltaDuck = ( record->m_flDuckAmount - previous_record->m_flDuckAmount ) / record->m_iChokeTicks;
			float deltaEye = ( record->m_angEyeAngles.yaw, previous_record->m_angEyeAngles.yaw ) / record->m_iChokeTicks;

			// TODO: calculate jump time
			// calculate landing time
			float landTime = 0.0f;
			bool jumped = false;
			bool landedOnServer = false;
			if ( record->m_serverAnimOverlays[ 4 ].m_flCycle < 0.5f && ( !( record->m_fFlags & FL_ONGROUND ) || !( previous_record->m_fFlags & FL_ONGROUND ) ) ) {
				landTime = record->m_flSimulationTime - float( record->m_serverAnimOverlays[ 4 ].m_flPlaybackRate * record->m_serverAnimOverlays[ 4 ].m_flCycle );
				landedOnServer = landTime >= previous_record->m_flSimulationTime;
			}

			bool onGround = record->m_fFlags & FL_ONGROUND;
			for ( int i = 1; i < record->m_iChokeTicks; i++ ) {
				C_SimulationInfo& data = pThis->m_vecSimulationData.emplace_back( );
				data.m_flTime = prev_simulation_time + TICKS_TO_TIME( i );

				if ( penultimate_record.IsValid( ) ) {
					// hermite spline much accurate than linear ( but still not perfect )
					float frac = float( i ) / float( record->m_iChokeTicks );
					data.m_vecOrigin = Math::Hermite_Spline( penultimate_record->m_vecOrigin, previous_record->m_vecOrigin, record->m_vecOrigin, frac );
					data.m_vecVelocity = Math::Hermite_Spline( penultimate_record->m_vecVelocity, previous_record->m_vecVelocity, record->m_vecVelocity, frac );
					data.m_flDuckAmount = Math::Hermite_Spline( penultimate_record->m_flDuckAmount, previous_record->m_flDuckAmount, record->m_flDuckAmount, frac );
					data.m_flEyeYaw = Math::Hermite_Spline( penultimate_record->m_angEyeAngles, previous_record->m_angEyeAngles, record->m_angEyeAngles, frac ).yaw;
				} else {
					// do linear interpolation
					data.m_vecOrigin = previous_record->m_vecOrigin + deltaOrigin * i;
					data.m_vecVelocity = previous_record->m_vecVelocity + deltaVel * i;
					data.m_flDuckAmount = previous_record->m_flDuckAmount + deltaDuck * i;
					data.m_flEyeYaw = previous_record->m_angEyeAngles.yaw + deltaEye * i;
				}

				// jump_fall fix
				if ( landedOnServer && !jumped ) {
					if ( landTime <= data.m_flTime ) {
						jumped = true;
						onGround = true;
					} else {
						onGround = previous_record->m_fFlags & FL_ONGROUND;
					}
				}

				data.bOnGround = onGround;
			}

			// add last tick
			C_SimulationInfo& data = pThis->m_vecSimulationData.emplace_back( );
			data.m_flTime = record->m_flSimulationTime;
			data.m_flDuckAmount = record->m_flDuckAmount;
			data.m_flEyeYaw = record->m_angEyeAngles.yaw;
			data.m_vecOrigin = record->m_vecOrigin;
			data.m_vecVelocity = record->m_vecVelocity;
			data.bOnGround = record->m_fFlags & FL_ONGROUND;
		}
	}

	void C_AnimationData::SimulateAnimations( Encrypted_t<Engine::C_AnimationRecord> current, Encrypted_t<Engine::C_AnimationRecord> previous ) {
		auto UpdateAnimations = [ ]( C_CSPlayer* player, float time ) {
			auto curtime = Source::m_pGlobalVars->curtime;
			auto frametime = Source::m_pGlobalVars->frametime;
			auto framecount = Source::m_pGlobalVars->framecount;
			auto tickcount = Source::m_pGlobalVars->tickcount;
			auto realtime = Source::m_pGlobalVars->realtime;
			auto absoluteframetime = Source::m_pGlobalVars->absoluteframetime;
			auto interpolation_amount = Source::m_pGlobalVars->interpolation_amount;

			// force to use correct abs origin and velocity ( no CalcAbsolutePosition and CalcAbsoluteVelocity calls )
			player->m_iEFlags( ) &= ~( EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY );

			int ticks = TIME_TO_TICKS( time );

			// calculate animations based on ticks aka server frames instead of render frames
			Source::m_pGlobalVars->curtime = time;
			Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;
			Source::m_pGlobalVars->realtime = time;
			Source::m_pGlobalVars->absoluteframetime = Source::m_pGlobalVars->interval_per_tick;
			Source::m_pGlobalVars->tickcount = ticks;
			Source::m_pGlobalVars->framecount = ticks;
			Source::m_pGlobalVars->interpolation_amount = 0.f;

			auto animstate = player->m_PlayerAnimState( );
			if ( animstate && animstate->m_nLastFrame == Source::m_pGlobalVars->framecount )
				animstate->m_nLastFrame = Source::m_pGlobalVars->framecount - 1;

			for ( int i = 0; i < player->m_AnimOverlay( ).Count( ); ++i ) {
				player->m_AnimOverlay( ).Base( )[ i ].m_pOwner = player;
				player->m_AnimOverlay( ).Base( )[ i ].m_pStudioHdr = player->m_pStudioHdr( );
			}


			g_Vars.globals.m_bUpdatingAnimations = true;
			player->FixedAnimationsUpdate( );
			g_Vars.globals.m_bUpdatingAnimations = false;

			Source::m_pGlobalVars->curtime = curtime;
			Source::m_pGlobalVars->realtime = realtime;
			Source::m_pGlobalVars->frametime = frametime;
			Source::m_pGlobalVars->absoluteframetime = absoluteframetime;
			Source::m_pGlobalVars->tickcount = tickcount;
			Source::m_pGlobalVars->framecount = framecount;
			Source::m_pGlobalVars->interpolation_amount = interpolation_amount;

			player->InvalidatePhysicsRecursive( 2 | 8 | 32 );
			};


		AnimationResolver( current );

		// end 
		auto animState = player->m_PlayerAnimState( );

		if ( previous.IsValid( ) ) {
			if ( previous->m_bIsInvalid && current->m_fFlags & FL_ONGROUND ) {
				animState->m_bOnGround = true;
				animState->m_bHitground = false;
			}

			animState->m_flFeetCycle = previous->m_flFeetCycle;
			animState->m_flFeetYawRate = previous->m_flFeetYawRate;
			*( float* )( uintptr_t( animState ) + 0x180 ) = previous->m_serverAnimOverlays[ 12 ].m_flWeight;

			std::memcpy( player->m_AnimOverlay( ).Base( ), previous->m_serverAnimOverlays, sizeof( previous->m_serverAnimOverlays ) );
		} else {
			animState->m_flFeetCycle = current->m_flFeetCycle;
			animState->m_flFeetYawRate = current->m_flFeetYawRate;
			*( float* )( uintptr_t( animState ) + 0x180 ) = current->m_serverAnimOverlays[ 12 ].m_flWeight;
		}

		if ( this->m_vecSimulationData.size( ) > 1 && current->m_iChokeTicks != 1 ) {
			SimulationRestore restore;
			restore.Setup( player );

			for ( auto it = this->m_vecSimulationData.begin( ); it < this->m_vecSimulationData.end( ); ++it ) {
				const auto& simData = *it;
				if ( simData.bOnGround ) {
					player->m_fFlags( ) |= FL_ONGROUND;
				} else {
					player->m_fFlags( ) &= ~FL_ONGROUND;
				}

				player->m_vecOrigin( ) = simData.m_vecOrigin;
				player->m_flDuckAmount( ) = simData.m_flDuckAmount;
				player->m_vecVelocity( ) = simData.m_vecVelocity;
				player->SetAbsVelocity( simData.m_vecVelocity );
				player->SetAbsOrigin( simData.m_vecOrigin );

				UpdateAnimations( player, simData.m_flTime );
			}

			restore.Apply( player );
		} else {
			this->player->SetAbsVelocity( current->m_vecVelocity );
			this->player->SetAbsOrigin( current->m_vecOrigin );

			UpdateAnimations( player, current->m_flSimulationTime );
		}
	}

	void C_AnimationData::AnimationResolver( Encrypted_t<Engine::C_AnimationRecord> current ) {
		g_Vars.globals.m_iResolverType[ player->entindex( ) ] = 0;
		if ( !current.Xor( ) ) {
			return;
		}

		auto AnimationData = Encrypted_t<C_AnimationData>( this );
		if ( !AnimationData.Xor( ) )
			return;

		auto& lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
		if ( !lag_data.IsValid( ) )
			return;

		player_info_t player_info;
		if ( !Source::m_pEngine->GetPlayerInfo( player->m_entIndex, &player_info ) ) {
			return;
		}

		if ( player_info.fakeplayer ) {
			g_Vars.globals.m_iResolverType[ player->entindex( ) ] = 2;
			current->m_iResolverSide = 0;
			return;
		}

		auto local = C_CSPlayer::GetLocalPlayer( );
		if ( !local || !g_Vars.globals.HackIsReady )
			return;

		/* just angle away */
		Vector angle_away;
		Math::VectorAngles( local->m_vecOrigin( ) - current->m_vecOrigin, angle_away );

		int index = this->player->entindex( );
		auto Entity = this->player;

		auto NormalizeYaw = [ ]( float yaw ) -> float {
			if ( yaw > 180 )
				yaw -= ( round( yaw / 360 ) * 360.f );
			else if ( yaw < -180 )
				yaw += ( round( yaw / 360 ) * -360.f );

			return yaw;
			};

		auto m_bIsYawSideways = [ & ]( float yaw ) -> bool {
			const auto at_target_yaw = angle_away.y;

			const float delta = Math::AngleDiff( at_target_yaw, yaw );

			return delta > 25.f && delta < 165.f;
		};

		auto m_bIsYawSideways1 = [ & ]( float yaw ) -> bool {
			const auto at_target_yaw = angle_away.y;

			const float delta = Math::AngleDiff( at_target_yaw, yaw );

			return delta > 35.f && delta < 145.f;
		};

		/* use global data*/
		auto data = g_ResolverData[ index ];

		/* lets detect lm lby */
		if ( current->m_vecVelocity.Length2D( ) > 0.1f && !current->m_bFakeWalking ) {
			data.m_flNextBodyUpdate = Entity->m_flAnimationTime( ) + 0.22f;
			data.m_bPredictingUpdates = false;
			data.m_bBrokeLby = true;

			/* store */
			data.m_iResolverMode = eResolverModes::PRED_22;
			lag_data->m_iResolverMode = eResolverModes::PRED_22;

			/* reso mode */
			g_ResolverData[ index ].m_ResolverText = "0.22";
		} else if ( ( Entity->m_flAnimationTime( ) >= data.m_flNextBodyUpdate ) && !current->m_bFakeWalking && lag_data->m_iMissedStand1 < 1 ) { // fix overspam it, go for brute
			data.m_flNextBodyUpdate = Entity->m_flAnimationTime( ) + 1.1f;
			data.m_bPredictingUpdates = true;
			data.m_bBrokeLby = true;

			/* reso mode */
			data.m_iResolverMode = eResolverModes::PRED_11;
			lag_data->m_iResolverMode = eResolverModes::PRED_11;

			current->m_angEyeAngles.y = current->m_flLowerBodyYawTarget;

			/* reso mode */
			g_ResolverData[ index ].m_ResolverText = "1.1";
		}

		// xref: abs( Entity->m_vecVelocity( ).Length2D( ) ) > 29.f
		// xref: Entity->m_fFlags( )
		// xref: current->m_flLowerBodyYawTarget
		if ( current->m_vecVelocity.Length2D( ) > 0.1f && ( current->m_fFlags & FL_ONGROUND ) ) {
			current->m_angEyeAngles.y = current->m_flLowerBodyYawTarget;

			data.m_sMoveData.m_flLowerBodyYawTarget = current->m_flLowerBodyYawTarget;
			data.m_sMoveData.m_vecOrigin = current->m_vecOrigin;
			data.m_sMoveData.m_flAnimTime = player->m_flAnimationTime( );

			data.m_bCollectedValidMoveData = false;
			data.m_bBrokeLby = false;

			g_ResolverData[ index ].m_ResolverText = "MOVING";
		} else if ( ( current->m_fFlags & FL_ONGROUND ) ) {
			/* we have a valid move record */
			if ( player->IsDormant( ) ) { /* store these shits */
				data.m_bCollectedValidMoveData = false;
			} else {
				Vector delta = data.m_sMoveData.m_vecOrigin - current->m_vecOrigin;
				if ( delta.Length( ) > 16.f ) { /* proper way? */
					data.m_bCollectedValidMoveData = true;
				} else if ( delta.Length( ) < 16.f ) { /* we weren't checking this */
					data.m_bCollectedValidMoveData = false;
				}
			}

			bool m_bIsSideways_Lamb = m_bIsYawSideways1( data.m_sMoveData.m_flLowerBodyYawTarget );
			bool m_bIsBackwards = !m_bIsSideways_Lamb || Math::AngleDiff( data.m_sMoveData.m_flLowerBodyYawTarget, angle_away.y + 180.f ) <= 60.f;

			float m_flAnimTime = player->m_flAnimationTime( ) - data.m_sMoveData.m_flAnimTime;

			if ( data.m_bCollectedValidMoveData ) {
				/* WE VE got lastmove */
				auto m_bLastMoveValid = [ & ]( )-> bool {
					const auto fl_delta = std::fabsf( Math::AngleNormalize( angle_away.y - data.m_sMoveData.m_flLowerBodyYawTarget ) );
					return fl_delta > 20.f && fl_delta < 160.f;
					};

				if ( m_bLastMoveValid( ) && ( lag_data->m_iMissedStand1 < 1 /* max 1 */ ) ) {
					current->m_angEyeAngles.y = data.m_sMoveData.m_flLowerBodyYawTarget;

					/* resolver mode */
					data.m_iResolverMode = eResolverModes::STAND_LM;
					lag_data->m_iResolverMode = eResolverModes::STAND_LM;

					g_ResolverData[ index ].m_ResolverText = "VM:LM";
				} else if ( m_flAnimTime < 0.22 && !data.m_bBrokeLby ) {
					current->m_angEyeAngles.y = current->m_flLowerBodyYawTarget;

					/* resolver mode */
					data.m_iResolverMode = eResolverModes::PRED_LBY;
					lag_data->m_iResolverMode = eResolverModes::PRED_LBY;

					g_ResolverData[ index ].m_ResolverText = "LBY(22)";
				} /* im not sure if the logic is right theere */
				else if ( lag_data->m_iMissedStand1 > 1 /* after 1 */ && lag_data->m_iMissedStand2 < 6 /* do not exceed brute elements */ ) {

					/* resolver mode */
					data.m_iResolverMode = eResolverModes::STAND_BRUTE_1;
					lag_data->m_iResolverMode = eResolverModes::STAND_BRUTE_1;

					/* brute */
					switch ( lag_data->m_iMissedStand2 % 5 ) {
						case 0:
							g_ResolverData[ index ].m_ResolverText = "BVM:LBY";
							current->m_angEyeAngles.y = current->m_flLowerBodyYawTarget;
						break;
						case 1:
							g_ResolverData[ index ].m_ResolverText = "BVM:L+";
							current->m_angEyeAngles.y = current->m_flLowerBodyYawTarget + 67.f;
						break;
						case 2: 
							g_ResolverData[ index ].m_ResolverText = "BVM:L-";
							current->m_angEyeAngles.y = current->m_flLowerBodyYawTarget - 67.f;
						break;
						case 3: 
							g_ResolverData[ index ].m_ResolverText = "BVM:RIGHT";
							current->m_angEyeAngles.y = angle_away.y + 45.0f;
						break;
						case 4: 
							g_ResolverData[ index ].m_ResolverText = "BVM:LEFT";
							current->m_angEyeAngles.y = angle_away.y - 45.0f;
						break;
						default: break;
					}
				}

			} else {
				data.m_iResolverMode = eResolverModes::STAND;
				lag_data->m_iResolverMode = eResolverModes::STAND;

				auto lby_delta = current->m_flLowerBodyYawTarget;
				float angle_lby = 0.0f;

				if ( lby_delta < 85.f && lby_delta > -85.f ) {
					angle_lby = lby_delta;
				} else if ( lby_delta > 85.f ) {
					if ( lby_delta < 180.f ) {
						g_ResolverData[ index ].m_ResolverText = "NVM:110";
						angle_lby = 110.f;
					}
				} else if ( lby_delta < 85.f ) {
					if ( lby_delta > -180.f ) {
						if ( lby_delta < -85.f ) {
							angle_lby = -110.f;
							g_ResolverData[ index ].m_ResolverText = "NVM:-110";
						} else {
							g_ResolverData[ index ].m_ResolverText = "NVM:LD:1";
							angle_lby = lby_delta;
						}
					}
				}

				current->m_angEyeAngles.y = angle_lby;
			}

		} else {
			current->m_angEyeAngles.y = current->m_flLowerBodyYawTarget;
			g_ResolverData[ index ].m_ResolverText = "AIR";

			data.m_iResolverMode = eResolverModes::AIR;
			lag_data->m_iResolverMode = eResolverModes::AIR;
		}

		/* set it */
		player->SetEyeAngles( current->m_angEyeAngles );
	}
}