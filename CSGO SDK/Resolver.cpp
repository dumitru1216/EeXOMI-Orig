#include "Resolver.hpp"
#include "EventLogger.hpp"
#include "RoundFireBulletsStore.hpp"
#include <sstream>
#include "Autowall.h"

std::vector<int> YawDefaultResolverSides = { 0, 1, 2 };
std::vector<int> YawBruhResolver = { 0, 1, 2 };

int C_BruteforceData::ConvertSideIndex( const int value, bool side_to_index ) {
	if ( !side_to_index ) {
		if ( value == 2 )
			return -1;

		if ( value == 1 )
			return 1;

		return 0;
	} else {
		if ( value == -1 )
			return 2;

		if ( value == 1 )
			return 1;

		return 0;
	}
}

int C_BruteforceData::GetYawSide( int index, Engine::C_LagRecord record ) {
	return 0;
}

C_BruteforceData g_BruteforceData[ 65 ] = {};
ResolverData_t g_ResolverData[ 65 ] = {};

namespace Engine {
	struct C_TraceData {
		int yaw_side;
		bool is_resolver_issue;
		bool is_correct;
	};

	void TraceMatrix( const Vector& start, const Vector& end, C_LagRecord* record, C_CSPlayer* player, std::vector<C_TraceData>& data, int side, bool did_hit ) {
		auto& trace = data.emplace_back( );

		Ray_t ray;
		ray.Init( start, end );

		record->Apply( player );

		CGameTrace tr;
		Source::m_pEngineTrace->ClipRayToEntity( ray, 0x4600400B, player, &tr );
		trace.is_resolver_issue = tr.hit_entity == player;
		trace.is_correct = trace.is_resolver_issue == did_hit;
		trace.yaw_side = side;
		if ( trace.yaw_side == -1 )
			trace.yaw_side = 2;
	}

	Encrypted_t<C_Resolver> C_Resolver::Get( ) {
		static C_Resolver instance;
		return &instance;
	}

	void C_Resolver::Start( ) {
		auto netchannel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
		if ( !netchannel.IsValid( ) ) {
			return;
		}

		ProcessEvents( );

		auto latency = netchannel->GetAvgLatency( FLOW_OUTGOING ) * 1000.f;//TIME_TO_TICKS( netchannel->GetAvgLatency( FLOW_INCOMING ) + netchannel->GetAvgLatency( FLOW_OUTGOING ) ) + 1;

		auto it = this->m_Shapshots.begin( );
		while ( it != this->m_Shapshots.end( ) ) {
			if ( !it->haveReference && it->correctSequence && Source::m_pClientState->m_nLastCommandAck( ) >= it->outSequence + latency ) {
				if ( g_Vars.esp.event_resolver ) {
					ILoggerEvent::Get( )->PushEvent( XorStr( "missed shot due to latency" ), FloatColor( 0.5f, 0.5f, 0.5f ) );
				}
				it = this->m_Shapshots.erase( it );
			} else {
				it++;
			}
		}
	}

	void C_Resolver::ProcessEvents( ) {
		if ( !this->m_GetEvents ) {
			return;
		}

		this->m_GetEvents = false;

		if ( this->m_Shapshots.empty( ) ) {
			this->m_Weaponfire.clear( );
			return;
		}

		if ( this->m_Weaponfire.empty( ) ) {
			this->m_Shapshots.clear( );
			return;
		}

		try {
			auto it = this->m_Weaponfire.begin( );
			while ( it != this->m_Weaponfire.end( ) ) {
				if ( this->m_Shapshots.empty( ) || this->m_Weaponfire.empty( ) ) {
					this->m_Weaponfire.clear( );
					//#ifdef DevelopMode
					if ( g_Vars.esp.event_resolver )
						ILoggerEvent::Get( )->PushEvent( XorStr( "[Resolver] snapshot or weponfire empty!" ), FloatColor( 255, 50, 100 ) );
					//#endif
					break;
				}

				auto snapshot = it->snapshot;

				if ( !( &it->snapshot ) || !&( *it->snapshot ) ) {
					//#ifdef DevelopMode
					if ( g_Vars.esp.event_resolver )
						ILoggerEvent::Get( )->PushEvent( XorStr( "[Resolver] Unknown snapshot!" ), FloatColor( 255, 50, 100 ) );
					//#endif

					it = this->m_Weaponfire.erase( it );
					continue;
				}

				if ( snapshot == this->m_Shapshots.end( ) ) {
					//#ifdef DevelopMode
					if ( g_Vars.esp.event_resolver )
						ILoggerEvent::Get( )->PushEvent( XorStr( "[Resolver] Unknown snapshot!" ), FloatColor( 255, 50, 100 ) );
					//#endif

					it = this->m_Weaponfire.erase( it );
					continue;
				}

				auto player = snapshot->player;
				if ( !player ) {
					//#ifdef DevelopMode
					if ( g_Vars.esp.event_resolver )
						ILoggerEvent::Get( )->PushEvent( XorStr( "[Resolver] Snapshot entity invalid!" ), FloatColor( 255, 50, 100 ) );
					//#endif

					this->m_Shapshots.erase( it->snapshot );
					it = this->m_Weaponfire.erase( it );
					continue;
				}

				if ( player != C_CSPlayer::GetPlayerByIndex( it->snapshot->playerIdx ) ) {
					//#ifdef DevelopMode
					if ( g_Vars.esp.event_resolver )
						ILoggerEvent::Get( )->PushEvent( XorStr( "[Resolver] Snapshot entity invalid!" ), FloatColor( 255, 50, 100 ) );
					//#endif
					this->m_Shapshots.erase( it->snapshot );
					it = this->m_Weaponfire.erase( it );
					continue;
				}

				auto anim_data = AnimationSystem::Get( )->GetAnimationData( player->m_entIndex );
				if ( !anim_data ) {
					//#ifdef DevelopMode
					if ( g_Vars.esp.event_resolver )
						ILoggerEvent::Get( )->PushEvent( XorStr( "[Resolver] Empty snapshot data!" ), FloatColor( 255, 50, 100 ) );
					//#endif

					this->m_Shapshots.erase( it->snapshot );
					it = this->m_Weaponfire.erase( it );
					continue;
				}

				if ( it->impacts.empty( ) ) {
					//#ifdef DevelopMode
					if ( g_Vars.esp.event_resolver )
						ILoggerEvent::Get( )->PushEvent( XorStr( "[Resolver] Empty snapshot data!" ), FloatColor( 255, 50, 100 ) );
					//#endif

					this->m_Shapshots.erase( it->snapshot );
					it = this->m_Weaponfire.erase( it );
					continue;
				}

				auto& lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
				if ( player->IsDead( ) ) {
					// g_BruteforceData[ player->m_entIndex ].SideIndex = it->snapshot->SideIndex;

					this->m_Shapshots.erase( it->snapshot );
					it = this->m_Weaponfire.erase( it );
					continue;
				}

				auto did_hit = it->damage.size( ) > 0;

				// last reseived impact
				auto last_impact = it->impacts.back( );

				C_BaseLagRecord backup;
				backup.Setup( player );

				std::vector<C_TraceData> trace_data;
				TraceMatrix( it->snapshot->eye_pos, last_impact, &it->snapshot->resolve_record, player, trace_data, 0, did_hit );
				TraceMatrix( it->snapshot->eye_pos, last_impact, &it->snapshot->resolve_record, player, trace_data, 1, did_hit );
				TraceMatrix( it->snapshot->eye_pos, last_impact, &it->snapshot->resolve_record, player, trace_data, -1, did_hit );

				backup.Apply( player );

				auto CanHitPlayer = [ player ]( C_LagRecord* record, int side, const Vector& eyePos, const Vector& end ) {
					Ray_t ray;
					ray.Init( eyePos, end );

					record->Apply( player );

					CGameTrace tr;
					Source::m_pEngineTrace->ClipRayToEntity( ray, 0x4600400B, player, &tr );

					return tr.DidHit( ) && tr.hit_entity == player;
					};

				if ( !did_hit ) {
					auto aimpoint_distance = it->snapshot->eye_pos.Distance( it->snapshot->AimPoint );
					auto impact_distance = it->snapshot->eye_pos.Distance( last_impact );
					auto side = it->snapshot->resolverIdx;
					if ( side == -1 )
						side = 2;

					auto td = &trace_data[ side ];

					// resolver issue
					if ( td->is_resolver_issue ) {
						auto resolver_data = g_ResolverData[ player->entindex( ) ];

						lag_data->m_iMissedShots++;

						if ( lag_data->m_iResolverMode == eResolverModes::STAND_LM )
							lag_data->m_iMissedStand1++;
						else if ( lag_data->m_iResolverMode == eResolverModes::STAND_BRUTE_1 )
							lag_data->m_iMissedStand2++;						
						else if ( lag_data->m_iResolverMode == eResolverModes::PRED_SIDE_LM )
							lag_data->m_iMissedStand3++;						
						else if ( lag_data->m_iResolverMode == eResolverModes::PRED_BW )
							lag_data->m_iMissedStand4++;						
						else if ( lag_data->m_iResolverMode == eResolverModes::STAND_BRUTE_2 )
							lag_data->m_iMissedStand5++;

						if ( snapshot->resolve_record.m_iRecordPriority == 3 )
							anim_data->m_bSuppressAnimationResolver = true;


						if ( g_Vars.esp.event_resolver ) {
							std::stringstream msg;

							msg << XorStr( "missed shot due to resolver" );
							ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 0.5f, 0.5f, 0.5f ) );
						}

						// custom miss log shit
						player_info_t info;
						if ( Source::m_pEngine->GetPlayerInfo( player->entindex( ), &info ) ) {
							CCSGOPlayerAnimState* animState = player->m_PlayerAnimState( );
							if ( animState )
								IRoundFireBulletsStore::Get( )->EventCallBack( nullptr, 0, new SRoundStats( 0, player->entindex( ), info.szName, 0, animState->m_flAbsRotation, 0, snapshot->resolverIdx, 0, 0 ) );
						}
					} else if ( aimpoint_distance > impact_distance ) { // occulusion issue
						g_BruteforceData[ player->m_entIndex ].SideIndex = it->snapshot->SideIndex;

						if ( snapshot->resolve_record.m_iRecordPriority == 3 )
							anim_data->m_bSuppressAnimationResolver = true;

						if ( g_BruteforceData[ player->m_entIndex ].IsResolved && g_BruteforceData[ player->m_entIndex ].LastHitIndex == it->snapshot->SideIndex )
							g_BruteforceData[ player->m_entIndex ].IsResolved = false;

						if ( g_Vars.esp.event_resolver ) {
							std::stringstream msg;

							msg << XorStr( "missed shot due to occlusion" );
							ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 0.5f, 0.5f, 0.5f ) );
						}
						player_info_t info;
						if ( Source::m_pEngine->GetPlayerInfo( player->entindex( ), &info ) ) {
							CCSGOPlayerAnimState* animState = player->m_PlayerAnimState( );
							if ( animState )
								IRoundFireBulletsStore::Get( )->EventCallBack( nullptr, 0, new SRoundStats( 0, player->entindex( ), info.szName, 0, animState->m_flAbsRotation, 7, snapshot->resolverIdx, 0, 0 ) );
						}
					} else { // spread issue
						if ( g_Vars.esp.event_resolver ) {
							std::stringstream msg;

							msg << XorStr( "missed shot due to spread " );
							ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 0.5f, 0.5f, 0.5f ) );
						}

						player_info_t info;
						if ( Source::m_pEngine->GetPlayerInfo( player->entindex( ), &info ) ) {
							CCSGOPlayerAnimState* animState = player->m_PlayerAnimState( );
							if ( animState )
								IRoundFireBulletsStore::Get( )->EventCallBack( nullptr, 0, new SRoundStats( 0, player->entindex( ), info.szName, 0, animState->m_flAbsRotation, 1, snapshot->resolverIdx, 0, 0 ) );
						}
					}
				} else {
					bool shoud_break = false;
					auto best_damage = it->damage.end( );
					auto dmg = it->damage.begin( );
					while ( dmg != it->damage.end( ) ) {
						shoud_break = true;
						if ( best_damage == it->damage.end( )
							 || dmg->damage > best_damage->damage ) {
							best_damage = dmg;
						}

						dmg++;
					}

					if ( shoud_break ) {
						auto resolved_side = INT_MAX;
						if ( it->impacts.size( ) > 0 ) { /* delete this later if it creates problems */
							auto record = &it->snapshot->resolve_record;

							auto center = record->m_vecOrigin;
							center.z += record->m_vecMaxs.z * 0.5f;

							auto eyePos = snapshot->eye_pos;
							auto end = Vector( );
							if ( it->impacts.size( ) > 1 ) {
								auto distance = FLT_MAX;
								for ( auto& pos : it->impacts ) {
									auto dist = pos.Distance( center );
									if ( distance > dist ) {
										end = pos;
										distance = dist;
									}
								}
							} else {
								end = it->impacts.back( );
							}

							C_BaseLagRecord backup;
							backup.Setup( player );

							auto best_distance = FLT_MAX;
							for ( int i = -1; i < 1; ++i ) {
								Ray_t ray;
								ray.Init( eyePos, end );

								record->Apply( player );

								CGameTrace tr;
								Source::m_pEngineTrace->ClipRayToEntity( ray, 0x4600400B, player, &tr );
								if ( tr.hit_entity == player && tr.hitgroup == best_damage->hitgroup ) {
									auto dst = tr.endpos.Distance( end );
									if ( best_distance > dst && dst <= 4.0f ) {
										resolved_side = i;
										best_distance = dst;
									}
								}
							}

							backup.Apply( player );
						}

					}
				}

				this->m_Shapshots.erase( it->snapshot );
				it = this->m_Weaponfire.erase( it );
			}
		} catch ( const std::exception& ) {
			return;
		}
	}

	void C_Resolver::EventCallback( IGameEvent* gameEvent, uint32_t hash ) {
		if ( this->m_Shapshots.size( ) <= 0 )
			return;

		auto net_channel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
		if ( !net_channel.IsValid( ) ) {
			this->m_Shapshots.clear( );
			return;
		}

		C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
		auto name = gameEvent->GetName( );
		if ( !local )
			return;

		switch ( hash ) {
			case hash_32_fnv1a_const( "player_death" ):
			{
				int id = Source::m_pEngine->GetPlayerForUserID( gameEvent->GetInt( XorStr( "userid" ) ) );
				for ( int i = 1; i < Source::m_pGlobalVars->maxClients; ++i ) {
					auto player = C_CSPlayer::GetPlayerByIndex( i );

					if ( !player || player == local )
						continue;

					auto lagData = Engine::LagCompensation::Get( )->GetLagData( i );
					if ( lagData.IsValid( ) ) {
						lagData->m_iMissedShots = 0;
						lagData->m_iMissedStand1 = 0;
						lagData->m_iMissedStand2 = 0;
						lagData->m_iMissedStand3 = 0;
						lagData->m_iMissedStand4 = 0;
						lagData->m_iMissedStand5 = 0;

						g_ResolverData[ i ].Reset( );

						lagData->m_iResolverMode = eResolverModes::NONE;
					}
				}
			}
			case hash_32_fnv1a_const( "round_start" ):
			{
				for ( int i = 1; i < Source::m_pGlobalVars->maxClients; ++i ) {
					auto lagData = Engine::LagCompensation::Get( )->GetLagData( i );
					if ( lagData.IsValid( ) ) {
						lagData->m_iMissedShots = 0;
						lagData->m_iMissedStand1 = 0;
						lagData->m_iMissedStand2 = 0;
						lagData->m_iMissedStand3 = 0;
						lagData->m_iMissedStand4 = 0;
						lagData->m_iMissedStand5 = 0;

						g_ResolverData[ i ].Reset( );

						lagData->m_iResolverMode = eResolverModes::NONE;
					}
				}

				break;
			}
		}

		if ( local->IsDead( ) ) {
			this->m_Shapshots.clear( );
			return;
		}

		C_WeaponCSBaseGun* weapon = ( C_WeaponCSBaseGun* )( local->m_hActiveWeapon( ).Get( ) );
		if ( !weapon ) {
			this->m_Shapshots.clear( );
			return;
		}

		auto weapon_data = weapon->GetCSWeaponData( );
		if ( !weapon_data.IsValid( ) ) {
			this->m_Shapshots.clear( );
			return;
		}

		auto it = this->m_Shapshots.begin( );
		while ( it != this->m_Shapshots.end( ) ) {
			// unhandled snapshots
			if ( std::fabsf( it->time - Source::m_pGlobalVars->realtime ) >= 2.5f ) {
				//#ifdef DevelopMode
				ILoggerEvent::Get( )->PushEvent( XorStr( "[Resolver] Unhandled snapshot!" ), FloatColor( 0.5f, 0.5f, 0.5f ) );
				//#endif
				it = this->m_Shapshots.erase( it );
			} else {
				it++;
			}
		}

		if ( this->m_Shapshots.size( ) <= 0 )
			return;

		auto snapshot = this->m_Shapshots.end( );

		switch ( hash ) {
			case hash_32_fnv1a_const( "player_hurt" ):
			{
				if ( this->m_Weaponfire.empty( ) || Source::m_pEngine->GetPlayerForUserID( gameEvent->GetInt( XorStr( "attacker" ) ) ) != local->entindex( ) )
					return;

				// TODO: check if need backtrack
				auto target = C_CSPlayer::GetPlayerByIndex( Source::m_pEngine->GetPlayerForUserID( gameEvent->GetInt( XorStr( "userid" ) ) ) );
				if ( !target || target == local || local->IsTeammate( target ) || target->IsDormant( ) )
					return;

				auto& player_damage = this->m_Weaponfire.back( ).damage.emplace_back( );
				player_damage.playerIdx = target->m_entIndex;
				player_damage.player = target;
				player_damage.damage = gameEvent->GetInt( XorStr( "dmg_health" ) );
				player_damage.hitgroup = gameEvent->GetInt( XorStr( "hitgroup" ) );
				break;
			}
			case hash_32_fnv1a_const( "bullet_impact" ):
			{
				if ( this->m_Weaponfire.empty( ) || Source::m_pEngine->GetPlayerForUserID( gameEvent->GetInt( XorStr( "userid" ) ) ) != local->entindex( ) )
					return;

				this->m_Weaponfire.back( ).impacts.emplace_back( gameEvent->GetFloat( XorStr( "x" ) ), gameEvent->GetFloat( XorStr( "y" ) ), gameEvent->GetFloat( XorStr( "z" ) ) );
				break;
			}
			case hash_32_fnv1a_const( "weapon_fire" ):
			{
				if ( Source::m_pEngine->GetPlayerForUserID( gameEvent->GetInt( XorStr( "userid" ) ) ) != local->entindex( ) )
					return;

				// will get iBullets weapon_fire events
				int to_element = this->m_Weaponfire.size( ) / weapon_data->m_iBullets;

				if ( to_element <= this->m_Shapshots.size( ) ) {
					snapshot = this->m_Shapshots.begin( ) + to_element;

					auto& fire = this->m_Weaponfire.emplace_back( );
					fire.snapshot = snapshot;
				}
				break;
			}
		}

		this->m_GetEvents = true;
	}

	void C_Resolver::CreateSnapshot( C_CSPlayer* player, const Vector& shootPosition, const Vector& aimPoint, Engine::C_LagRecord* record, int resolverSide, int hitgroup ) {
		auto& snapshot = this->m_Shapshots.emplace_back( );

		snapshot.playerIdx = player->m_entIndex;
		snapshot.resolverIdx = resolverSide;
		snapshot.player = player;
		snapshot.resolve_record = *record;
		snapshot.eye_pos = shootPosition;
		snapshot.time = Source::m_pGlobalVars->realtime;
		snapshot.correctSequence = false;
		snapshot.correctEyePos = false;
		snapshot.haveReference = false;

		snapshot.AimPoint = aimPoint;
		snapshot.Hitgroup = hitgroup;

		snapshot.IsPrioritySide = g_BruteforceData[ player->m_entIndex ].LastResolverAttempt == Source::m_pGlobalVars->tickcount;

		if ( g_BruteforceData[ player->m_entIndex ].ResolvedSides.empty( ) || g_BruteforceData[ player->m_entIndex ].IsResolved ) {
			snapshot.IsSwitchedSide = false;
			snapshot.SideIndex = g_BruteforceData[ player->m_entIndex ].SideIndex;
		} else {
			snapshot.IsSwitchedSide = true;
			snapshot.SideIndex = g_BruteforceData[ player->m_entIndex ].ResolvedSides.front( );

			g_BruteforceData[ player->m_entIndex ].ResolvedSides.erase( g_BruteforceData[ player->m_entIndex ].ResolvedSides.begin( ) );
		}

		auto data = AnimationSystem::Get( )->GetAnimationData( player->m_entIndex );
		if ( data ) {
			data->m_bAimbotTarget = true;
			data->m_flLastScannedYaw = record->m_flEyeYaw;
		}

		if ( record->m_iRecordPriority < 2 )
			record->m_iRecordPriority = 2;
	}

	void C_Resolver::CorrectSnapshots( bool is_sending_packet ) {
		auto local = C_CSPlayer::GetLocalPlayer( );
		if ( !local )
			return;

		auto netchannel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
		if ( !netchannel.IsValid( ) )
			return;

		for ( auto& snapshot : this->m_Shapshots ) {
			if ( !snapshot.correctEyePos ) {
				snapshot.eye_pos = local->GetEyePosition( );
				snapshot.correctEyePos = true;
			}

			if ( is_sending_packet && !snapshot.correctSequence ) {
				snapshot.outSequence = Source::m_pClientState->m_nLastOutgoingCommand( ) + Source::m_pClientState->m_nChokedCommands( ) + 1;
				snapshot.correctSequence = true;
			}
		}
	}
}

