#include "Hooked.hpp"
#include "Prediction.hpp"
#include "displacement.hpp"
#include "sdk.hpp"
#include "player.hpp"
#include "EventLogger.hpp"
#include "GrenadePrediction.hpp"
#include "Esp.hpp"
#include "Render.hpp"
#include "ExtendedEsp.hpp"
#include "Movement.hpp"
#include "InputSys.hpp"
#include "Exploits.hpp"
#include "Miscellaneous.hpp"
#include "SetupBones.hpp"
#include "LagCompensation.hpp"
#include "Utils/threading.h"
#include "AnimationSystem.hpp"
#include "Resolver.hpp"
#include "BulletBeamTracer.hpp"
#include "PreserveKillfeed.hpp"
#include "ServerSounds.hpp"
#include "TickbaseShift.hpp"

// #include "EXGui/ExGui.hpp"
//#define DEBUG_TICKBASE
#ifdef DEBUG_TICKBASE
extern bool debugTickbase;
UI.AddSliderFloat( 'world exposure', 0.0, 100.0 );
UI.AddSliderFloat( 'model ambient', 0.0, 100.0 );
UI.AddSliderFloat( 'bloom scale', 0.0, 100.0 );
UI.AddCheckbox( 'enable world color modulation' );
UI.AddColorPicker( 'world color' );
#endif

//#define DEBUG_SHOOTING

#ifdef DEBUG_SHOOTING
extern bool debugShoot;
#endif

float WorldExposure = 0.0f;
float ModelAmbient = 0.0f;
float BloomScale = 0.0f;

extern bool MoveJitter;
extern Vector JitterOrigin;

float RenderViewOffsetZ = 0.0f;

bool RestoreData = false;
Vector org_backup, vel_backup;

auto GetEEdict( int index ) -> std::uint8_t* {
   static uintptr_t pServerGlobals = **( uintptr_t** ) ( Engine::Displacement.Data.m_uServerGlobals );
   int iMaxClients = *( int* ) ( ( uintptr_t ) pServerGlobals + 0x18 );
   if ( iMaxClients >= index ) {
	  if ( index <= iMaxClients ) {
		 int v10 = index * 16;
		 uintptr_t v11 = *( uintptr_t* ) ( pServerGlobals + 96 );
		 if ( v11 ) {
			if ( !( ( *( uintptr_t* ) ( v11 + v10 ) >> 1 ) & 1 ) ) {
			   uintptr_t v12 = *( uintptr_t* ) ( v10 + v11 + 12 );
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

namespace Hooked
{
	enum PostProcessParameterNames_t {
		PPPN_FADE_TIME = 0,
		PPPN_LOCAL_CONTRAST_STRENGTH,
		PPPN_LOCAL_CONTRAST_EDGE_STRENGTH,
		PPPN_VIGNETTE_START,
		PPPN_VIGNETTE_END,
		PPPN_VIGNETTE_BLUR_STRENGTH,
		PPPN_FADE_TO_BLACK_STRENGTH,
		PPPN_DEPTH_BLUR_FOCAL_DISTANCE,
		PPPN_DEPTH_BLUR_STRENGTH,
		PPPN_SCREEN_BLUR_STRENGTH,
		PPPN_FILM_GRAIN_STRENGTH,

		POST_PROCESS_PARAMETER_COUNT
	};

	struct PostProcessParameters_t {
		PostProcessParameters_t( ) {
			memset( m_flParameters, 0, sizeof( m_flParameters ) );
			m_flParameters[ PPPN_VIGNETTE_START ] = 0.8f;
			m_flParameters[ PPPN_VIGNETTE_END ] = 1.1f;
		}

		float m_flParameters[ POST_PROCESS_PARAMETER_COUNT ];
	};

   struct clientanimating_t {
	  C_BaseAnimating* pAnimating;
	  unsigned int	flags;
	  clientanimating_t( C_BaseAnimating* _pAnim, unsigned int _flags ) : pAnimating( _pAnim ), flags( _flags ) { }
   };

   void __fastcall FrameStageNotify( void* ecx, void* edx, ClientFrameStage_t stage ) {
	  g_Vars.globals.szLastHookCalled = XorStr( "9" );
	  auto local = C_CSPlayer::GetLocalPlayer( );

	  // paranoic af
	  g_Vars.globals.HackIsReady = local
		 && Source::m_pEngine->IsInGame( )
		 && Source::m_pClientState->m_nDeltaTick( ) > 0
		 && local->m_flSpawnTime( ) > 0.f;

	  if ( g_Vars.esp.remove_post_proccesing ) {
		  static auto PostProcessParameters = *reinterpret_cast< PostProcessParameters_t** >( ( uintptr_t )Memory::Scan( ( "client.dll" ), ( "0F 11 05 ? ? ? ? 0F 10 87" ) ) + 3 );
		  static float backupblur = PostProcessParameters->m_flParameters[ PPPN_VIGNETTE_BLUR_STRENGTH ];

		  float blur = g_Vars.esp.remove_post_proccesing ? 0.f : backupblur;
		  if ( PostProcessParameters->m_flParameters[ PPPN_VIGNETTE_BLUR_STRENGTH ] != blur )
			  PostProcessParameters->m_flParameters[ PPPN_VIGNETTE_BLUR_STRENGTH ] = blur;
	  }

	  // cache random values cuz valve random system cause performance issues
	  if ( !g_Vars.globals.RandomInit ) {
		 std::vector<std::pair<int, float>> biggsetPi1;

		 for ( int i = 0; i < 256; ++i ) {
			RandomSeed( i + 1 );
			g_Vars.globals.SpreadRandom[ i ].flRand1 = RandomFloat( 0.0f, 1.0f );
			g_Vars.globals.SpreadRandom[ i ].flRandPi1 = RandomFloat( 0.0f, DirectX::XM_2PI );
			g_Vars.globals.SpreadRandom[ i ].flRand2 = RandomFloat( 0.0f, 1.0f );
			g_Vars.globals.SpreadRandom[ i ].flRandPi2 = RandomFloat( 0.0f, DirectX::XM_2PI );
		 }

		 g_Vars.globals.RandomInit = true;
	  }

	  if ( g_Vars.globals.HackIsReady ) {
		 static float networkedCycle = 0.0f;
		 static float animationTime = 0.f;

		 auto viewModel = local->m_hViewModel( ).Get( );

		 RenderViewOffsetZ = local->m_vecViewOffset( ).z;

		 // m_flAnimTime : 0x260
		 if ( viewModel ) {
			// onetap.su
			if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && g_Vars.globals.UnknownCycleFix && *( float* ) ( uintptr_t( viewModel ) + Engine::Displacement.DT_BaseAnimating.m_flCycle ) == 0.0f ) {
			   *( float* ) ( uintptr_t( viewModel ) + Engine::Displacement.DT_BaseAnimating.m_flCycle ) = networkedCycle;
			   *( float* ) ( uintptr_t( viewModel ) + Engine::Displacement.DT_BaseEntity.m_flAnimTime ) = animationTime;
			   g_Vars.globals.UnknownCycleFix = false;
			}

			networkedCycle = *( float* ) ( uintptr_t( viewModel ) + Engine::Displacement.DT_BaseAnimating.m_flCycle );
			animationTime = *( float* ) ( uintptr_t( viewModel ) + Engine::Displacement.DT_BaseEntity.m_flAnimTime );
		 }

	  #ifdef DEBUG_SHOOTING
		 if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END && local->m_flSimulationTime( ) != local->m_flOldSimulationTime( ) ) {
			auto weapon = ( C_WeaponCSBaseGun* ) ( local->m_hActiveWeapon( ).Get( ) );
			if ( weapon ) {
			   if ( debugShoot && weapon->m_fLastShotTime( ) >= local->m_flOldSimulationTime( ) && weapon->m_fLastShotTime( ) <= local->m_flSimulationTime( ) ) {
				  printf( XorStr( "server tick: %d\n" ), Source::m_pGlobalVars->tickcount );
				  debugShoot = false;
			   }
			}
		 }
	  #endif

	  #if 0
		 if ( local->m_flSimulationTime( ) != local->m_flOldSimulationTime( ) ) {
			auto ent = GetEEdict( local->m_entIndex );
			if ( ent ) {
			   // CBaseAnimating::DrawServerHitboxes(CBaseAnimating *this, float a2, __int64 a3)
			   // ref: mass %.1f, first call in found function - draw hitboxes

			   static auto pCall = ( uintptr_t* ) ( Memory::Scan( XorStr( "server.dll" ), XorStr( "55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE" ) ) );
			   float fDuration = local->m_flSimulationTime( ) - local->m_flOldSimulationTime( ) + Source::m_pGlobalVars->frametime * 2.0f;

			   __asm
			   {
				  pushad
				  movss xmm1, fDuration
				  push 1 //bool monoColor
				  mov ecx, ent
				  call pCall
				  popad
			   }
			}
		 }
	  #endif

		 if ( stage == FRAME_START && Source::m_pEngine->IsConnected( ) ) {
			// IPreserveKillfeed::Get( )->OnFrameStart( );
		 }

		 if ( stage == FRAME_RENDER_START && Source::m_pEngine->IsConnected( ) ) {
			 /* disable lag shit */
			 if ( g_Vars.rage.dt_lag_dis ) {
				 g_tickbase_control.skip_lag_interpolation( false );
			 }

			// disable animation interpolation 
			if ( Engine::Displacement.Data.m_uClientSideAnimationList ) {
			   auto clientAnimations = ( CUtlVector<clientanimating_t>* )Engine::Displacement.Data.m_uClientSideAnimationList;
			   for ( int i = 0; i < clientAnimations->m_Size; ++i ) {
				  auto& anim = clientAnimations->m_Memory.m_pMemory[ i ];
				  if ( !anim.pAnimating )
					 continue;

				  if ( !( anim.flags & 0x1 ) )
					 continue;

				  auto player = ToCSPlayer( anim.pAnimating );
				  if ( player && !player->IsDormant( ) && !player->IsDead( ) && player->m_CachedBoneData( ).Count( ) > 0 ) {
					 anim.flags &= ~0x1;

					 // force bone cache
					 {
						player->m_flLastBoneSetupTime( ) = FLT_MAX;
						player->m_iMostRecentModelBoneCounter( ) = ( *( int* ) Engine::Displacement.Data.m_uModelBoneCounter ) + 1;
						player->m_BoneAccessor( ).m_ReadableBones = player->m_BoneAccessor( ).m_WritableBones = 0xFFFFFFFF;
					 }
				  }
			   }
			}

			// no recoil without backup
			if ( g_Vars.esp.remove_recoil ) {
			   auto aim_punch = local->m_aimPunchAngle( ) * g_Vars.weapon_recoil_scale->GetFloat( ) * g_Vars.view_recoil_tracking->GetFloat( );
			   local->pl( ).v_angle -= local->m_viewPunchAngle( ) + aim_punch;
			   local->pl( ).v_angle.Normalize( );
			}

			if ( g_Vars.esp.remove_flash )
			   local->m_flFlashDuration( ) = 0.0f;

			// remove smoke overlay
			if ( g_Vars.esp.remove_smoke )
			   *( int* ) Engine::Displacement.Data.m_uSmokeCount = 0;


			for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
			   auto player = C_CSPlayer::GetPlayerByIndex( i );
			   auto local = C_CSPlayer::GetLocalPlayer( );

			   if ( !local )
				  continue;

			   if ( !player || player->IsDead( ) || player == local )
				  continue;

			   player_info_t player_info;
			   if ( !Source::m_pEngine->GetPlayerInfo( i, &player_info ) )
				  continue;

			   auto lagData = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
			   if ( !lagData.IsValid( ) || lagData->m_History.empty( ) )
				  continue;
			   if ( g_Vars.rage.enabled ) {
				  auto& record = lagData->m_History.front( );
				  matrix3x4_t* matrix = record.GetBoneMatrix( );

				  std::memcpy( player->m_CachedBoneData( ).Base( ), matrix,
							   player->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );

				  if ( player->m_CachedBoneData( ).Base( ) != player->m_BoneAccessor( ).m_pBones ) {
					 std::memcpy( player->m_BoneAccessor( ).m_pBones, matrix,
								  player->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );
				  }

				  // force bone cache
				  player->m_iMostRecentModelBoneCounter( ) = *( int* ) Engine::Displacement.Data.m_uModelBoneCounter;
				  player->m_BoneAccessor( ).m_ReadableBones = player->m_BoneAccessor( ).m_WritableBones = 0xFFFFFFFF;
				  player->m_flLastBoneSetupTime( ) = FLT_MAX;
				  if ( player->m_pStudioHdr( ) )
					 Engine::C_SetupBones::AttachmentHelper( player, player->m_pStudioHdr( ) );

			   }
			   player->SetAbsOrigin( player->m_vecOrigin( ) );

			   static bool reset_spotted = false;

			   if ( g_Vars.misc.ingame_radar ) {
				  *( bool* ) ( uintptr_t( player ) + 0x93D ) = true; // TODO: Remove hardcode offset DT_BaseEntity m_bSpotted
				  reset_spotted = true;
			   }
			   else {
				  if ( reset_spotted ) {
					 *( bool* ) ( uintptr_t( player ) + 0x93D ) = false;
					 reset_spotted = false;
				  }
			   }
			}
		 } else if ( stage == FRAME_NET_UPDATE_END && Source::m_pEngine->IsConnected( ) ) {
			typedef void( __thiscall* FuncSetTimeout )( void*, float seconds, bool bForceExact );
			static FuncSetTimeout SetTimeout = ( FuncSetTimeout ) Engine::Displacement.Function.m_uSetTimeout;

			if ( g_Vars.esp.event_exploits && local && local->m_flSimulationTime( ) - local->m_flOldSimulationTime( ) < 0.0f )
			   ILoggerEvent::Get( )->PushEvent( XorStr( "shifted tickbase" ), FloatColor( 255, 50, 100 ) );

			auto netchannel = Source::m_pEngine->GetNetChannelInfo( );
			if ( netchannel ) {
			#if 0
			   auto delta = Source::m_pGlobalVars->tickcount - TickcountBegin - TIME_TO_TICKS( netchannel->GetLatency( FLOW_INCOMING ) );;
			   if ( !FakelagMemes && delta > 0 && TickcountBegin != -1 && local->m_flSimulationTime( ) != local->m_flOldSimulationTime( ) ) {
				  LagLimit = delta;
				  FakelagMemes = true;
				  printf( "detected lag limit %d | server %d | client %d\n", LagLimit, Source::m_pGlobalVars->tickcount, TickcountBegin );
				  TickcountBegin = -1;

				  g_Vars.fakelag.lag_limit = LagLimit;
			   }
			#endif

			   //SetTimeout( Source::m_pEngine->GetNetChannelInfo( ), 3600.0f, false );
			}

			Engine::AnimationSystem::Get( )->CollectData( );

			Engine::C_Resolver::Get( )->Start( );

			Engine::AnimationSystem::Get( )->Update( );

			Threading::FinishQueue( true );

			Engine::LagCompensation::Get( )->Update( );
		 }

		 // fix netvar compression
		 auto& prediction = Engine::Prediction::Instance( );
		 if ( Source::m_pEngine->IsInGame( ) )
			prediction.OnFrameStageNotify( stage );
	  } else {
		 g_Vars.globals.RenderIsReady = false;
	  }

	  if ( stage == FRAME_RENDER_END ) {
		 static bool IsConnected = false;

		 if ( Source::m_pEngine->IsConnected( ) && !IsConnected ) {
			if ( !_stricmp( Source::m_pEngine->GetNetChannelInfo( )->GetAddress( ), XorStr( "loopback" ) ) )
			   g_Vars.globals.server_adress = XorStr( "local server" );
			else
			   g_Vars.globals.server_adress = Source::m_pEngine->GetNetChannelInfo( )->GetAddress( );

			IsConnected = true;
		 }

		 if ( !Source::m_pEngine->IsConnected( ) ) {
			g_Vars.globals.server_adress = XorStr( "not connected" );
			IsConnected = false;
		 }


		 if ( Source::m_pEngine->IsConnected( ) ) {
			static auto g_GameRules = *( uintptr_t** ) ( Engine::Displacement.Data.m_GameRules );
			if ( g_GameRules && *( bool* ) ( *( uintptr_t* ) g_GameRules + 0x75 ) ) {
			   g_Vars.globals.server_adress = XorStr( "valve server" );

			   if ( g_Vars.game_type->GetInt( ) == 0 ) { // classic
				  g_Vars.globals.m_iServerType = 1;
				  if ( g_Vars.game_mode->GetInt( ) == 0 ) {
					 g_Vars.globals.m_iGameMode = 1; // casual
				  } else if ( g_Vars.game_mode->GetInt( ) == 1 ) {
					 g_Vars.globals.m_iGameMode = 2; // competitive
				  } else if ( g_Vars.game_mode->GetInt( ) == 2 ) {
					 g_Vars.globals.m_iGameMode = 3; // scrimcomp2v2
				  } else if ( g_Vars.game_mode->GetInt( ) == 3 ) {
					 g_Vars.globals.m_iGameMode = 4; // scrimcomp5v5
				  }
			   } else if ( g_Vars.game_type->GetInt( ) == 1 ) { // gungame
				  g_Vars.globals.m_iServerType = 2;
				  if ( g_Vars.game_mode->GetInt( ) == 0 ) {
					 g_Vars.globals.m_iGameMode = 1; // gungameprogressive
				  } else if ( g_Vars.game_mode->GetInt( ) == 1 ) {
					 g_Vars.globals.m_iGameMode = 2; // gungametrbomb
				  } else if ( g_Vars.game_mode->GetInt( ) == 2 ) {
					 g_Vars.globals.m_iGameMode = 3; // deathmatch
				  }
			   } else if ( g_Vars.game_type->GetInt( ) == 2 ) { //training
				  g_Vars.globals.m_iServerType = 3;
				  g_Vars.globals.m_iGameMode = 0;
			   } else if ( g_Vars.game_type->GetInt( ) == 3 ) { //custom
				  g_Vars.globals.m_iServerType = 4;
				  g_Vars.globals.m_iGameMode = 0;
			   } else if ( g_Vars.game_type->GetInt( ) == 4 ) { //cooperative
				  g_Vars.globals.m_iServerType = 5;
				  g_Vars.globals.m_iGameMode = 0;
			   } else if ( g_Vars.game_type->GetInt( ) == 5 ) { //skirmish
				  g_Vars.globals.m_iServerType = 6;
				  g_Vars.globals.m_iGameMode = 0;
			   } else if ( g_Vars.game_type->GetInt( ) == 6 ) { //freeforall ( danger zone )
				  g_Vars.globals.m_iServerType = 7;
				  g_Vars.globals.m_iGameMode = 0;
			   }
			} else {
			   if ( !_stricmp( Source::m_pEngine->GetNetChannelInfo( )->GetAddress( ), XorStr( "loopback" ) ) )
				  g_Vars.globals.m_iServerType = 8; // local server
			   else
				  g_Vars.globals.m_iServerType = 9; // no valve server
			}
		 } else
			g_Vars.globals.m_iServerType = -1; // no connected

		 // draw visuals
		 Render::Get( )->BeginScene( );
		 {
			//g_ServerSounds.Start( );
			IEsp::Get( )->Main( );
			//g_ServerSounds.Finish( );
			IGrenadePrediction::Get( )->Paint( );
			ILoggerEvent::Get( )->Main( );
			Source::Miscellaneous::Get( )->Main( );
			if ( g_Vars.esp.beam_enabled )
			   IBulletBeamTracer::Get( )->Main( );

		 }
		 Render::Get( )->EndScene( );

		 if ( g_Vars.globals.HackIsReady )
			g_Vars.globals.RenderIsReady = true;
	  }

	  oFrameStageNotify( ecx, stage );

	  if ( g_Vars.globals.RenderIsReady ) {
		 if ( stage == FRAME_RENDER_START && Source::m_pEngine->IsConnected( ) ) {
			 /* disable lag shit */
			 if ( g_Vars.rage.dt_lag_dis ) {
				 g_tickbase_control.skip_lag_interpolation( true );
			 }

			local->m_vecViewOffset( ).z = RenderViewOffsetZ;

			if ( g_Vars.esp.bloom_effect ) {
			   static auto m_bUseCustomAutoExposureMin = Engine::PropManager::Instance( ).GetProp( XorStr( "DT_EnvTonemapController" ), XorStr( "m_bUseCustomAutoExposureMin" ) );
			   static auto m_bUseCustomAutoExposureMax = Engine::PropManager::Instance( ).GetProp( XorStr( "DT_EnvTonemapController" ), XorStr( "m_bUseCustomAutoExposureMax" ) );
			   static auto m_bUseCustomBloomScale = Engine::PropManager::Instance( ).GetProp( XorStr( "DT_EnvTonemapController" ), XorStr( "m_bUseCustomBloomScale" ) );

			   static auto m_flCustomAutoExposureMin = Engine::PropManager::Instance( ).GetProp( XorStr( "DT_EnvTonemapController" ), XorStr( "m_flCustomAutoExposureMin" ) );
			   static auto m_flCustomAutoExposureMax = Engine::PropManager::Instance( ).GetProp( XorStr( "DT_EnvTonemapController" ), XorStr( "m_flCustomAutoExposureMax" ) );

			   static auto m_flCustomBloomScale = Engine::PropManager::Instance( ).GetProp( XorStr( "DT_EnvTonemapController" ), XorStr( "m_flCustomBloomScale" ) );

			   for ( auto i = Source::m_pGlobalVars->maxClients + 1; i < Source::m_pEntList->GetHighestEntityIndex( ); i++ ) {
				  auto entity = ( C_BaseEntity* ) Source::m_pEntList->GetClientEntity( i );
				  if ( !entity )
					 continue;

				  auto client_class = entity->GetClientClass( );

				  if ( strcmp( client_class->m_pNetworkName, "CEnvTonemapController" ) ) {
					 continue;
				  }

				  *( bool* ) ( uintptr_t( entity ) + m_bUseCustomAutoExposureMin ) = true;
				  *( bool* ) ( uintptr_t( entity ) + m_bUseCustomAutoExposureMax ) = true;
				  *( bool* ) ( uintptr_t( entity ) + m_bUseCustomBloomScale ) = true;

				  *( float* ) ( uintptr_t( entity ) + m_flCustomAutoExposureMin ) = g_Vars.esp.exposure_scale / 10.0f;
				  *( float* ) ( uintptr_t( entity ) + m_flCustomAutoExposureMax ) = g_Vars.esp.exposure_scale / 10.0f;
				  *( float* ) ( uintptr_t( entity ) + m_flCustomBloomScale ) = g_Vars.esp.bloom_scale / 10.0f;

				  static auto r_modelAmbientMin = Source::m_pCvar->FindVar( "r_modelAmbientMin" );
				  r_modelAmbientMin->SetValue( g_Vars.esp.model_brightness / 10.0f );
			   }
			}
		 }

		 if ( stage == FRAME_NET_UPDATE_END && Source::m_pEngine->IsConnected( ) ) {
			 const auto correction_ticks = g_tickbase_control.calc_correction_ticks( );
			 if ( correction_ticks == -1 ) {
				 g_tickbase_control.m_correction_amount = 0;
			 }
			 else {
				 if ( local->m_flSimulationTime( ) > local->m_flOldSimulationTime( ) ) {
					 auto diff = TIME_TO_TICKS( local->m_flSimulationTime( ) ) - Source::m_pEngine->GetServerTick( /* should work like that */ );
					 if ( std::abs( diff ) <= correction_ticks ) {
						 g_tickbase_control.m_correction_amount = diff;
					 }

#if 0 // seems to work fine
					 ILoggerEvent::Get( )->PushEvent( XorStr( "shifter correction" ), FloatColor( 255, 0, 0 ) );
#endif
				 }
			 }
		 }

		 if ( RestoreData && stage == FRAME_RENDER_END ) {
			/*local->m_vecOrigin( ) = org_backup;
			local->m_vecVelocity( ) = vel_backup;
			RestoreData = false;*/
		 }
	  }
   }

   void __fastcall View_Render( void* ecx, void* edx, vrect_t* rect ) {
	  oView_Render( ecx, rect );
   }
}
