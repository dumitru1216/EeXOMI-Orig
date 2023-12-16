#include "Esp.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "CVariables.hpp"
#include "source.hpp"
#include "weapon.hpp"
#include "CBaseHandle.hpp"
#include "SoundEsp.hpp"
#include "Render.hpp"
#include "Autowall.h"
#include "Hitmarker.hpp"
#include "PropManager.hpp"
#include "LagCompensation.hpp"
#include "HitDamageVisualize.hpp"
#include "EventLogger.hpp"
#include "InputSys.hpp"
#include "Ragebot.hpp"
#include <minwindef.h>
#include <math.h>
#include "ServerSounds.hpp"
#include "RoundFireBulletsStore.hpp"
#include "RayTracer.h"
#include "displacement.hpp"
#include "CChams.hpp"
#include "TickbaseShift.hpp"
#include "ExtendedEsp.hpp"
#include <sstream>
#include <ctime>

#include "Movement.hpp"

//#include XorStr( "Utils/threading.h" ) 
#if 0
extern int AutowallCalls;
#endif

extern Vector AutoPeekPos;
extern float PreviousYaw;
const char* ItemDefinitionIndexToIcon( int index );

class C_Window {
public:
   Vector2D pos;
   Vector2D size;
   Vector2D mouse_pos;
   int id;

   C_Window( ) { }
   C_Window( Vector2D _pos, Vector2D _size, int _id )
	  : pos( _pos ), size( _size ), id( _id ) {
   }

   bool IsInBox( Vector2D m_MousePos, Vector2D box_pos, Vector2D box_size ) {
	  return (
		 m_MousePos.x > box_pos.x&&
		 m_MousePos.y > box_pos.y&&
		 m_MousePos.x < box_pos.x + box_size.x &&
		 m_MousePos.y < box_pos.y + box_size.y
		 );
   }

   void Drag( ) {
	  auto current_mouse_pos = InputSys::Get( )->GetMousePosition( );
	  if ( g_Vars.globals.menuOpen
		   && InputSys::Get( )->IsKeyDown( VirtualKeys::LeftButton )
		   && ( IsInBox( current_mouse_pos, pos, size ) || IsInBox( mouse_pos, pos, size ) ) ) {
		 pos += current_mouse_pos - mouse_pos;

		 if ( id == 0 ) {
			g_Vars.esp.keybind_x = pos.x;
			g_Vars.esp.keybind_y = pos.y;
		 }

		 if ( id == 1 ) {
			g_Vars.esp.hitlogger_x = pos.x;
			g_Vars.esp.hitlogger_y = pos.y;
		 }

		 if ( id == 2 ) {
			g_Vars.esp.spectator_x = pos.x;
			g_Vars.esp.spectator_y = pos.y;
		 }

		 if ( id == 3 ) {
			g_Vars.esp.indicators_x = pos.x;
			g_Vars.esp.indicators_y = pos.y;
		 }
	  }

	  if ( id == 0 ) {
		 pos.x = g_Vars.esp.keybind_x;
		 pos.y = g_Vars.esp.keybind_y;
	  }

	  if ( id == 1 ) {
		 pos.x = g_Vars.esp.hitlogger_x;
		 pos.y = g_Vars.esp.hitlogger_y;
	  }

	  if ( id == 2 ) {
		 pos.x = g_Vars.esp.spectator_x;
		 pos.y = g_Vars.esp.spectator_y;
	  }

	  if ( id == 3 ) {
		 pos.x = g_Vars.esp.indicators_x;
		 pos.y = g_Vars.esp.indicators_y;
	  }
	  mouse_pos = InputSys::Get( )->GetMousePosition( );
   }
};

class CEsp : public IEsp {
public:
   void PenetrateCrosshair( Vector2D center );
   void DrawZeusDistance( );
   void Main( ) override;
   void SetAlpha( int idx ) override;
   float GetAlpha( int idx ) override;
private:
   struct IndicatorsInfo_t {
	  IndicatorsInfo_t( ) {

	  }

	  IndicatorsInfo_t( const char* m_szName,
						int m_iPrioirity,
						bool m_bLoading,
						float m_flLoading,
						FloatColor m_Color ) {
		 this->m_szName = m_szName;
		 this->m_iPrioirity = m_iPrioirity;
		 this->m_bLoading = m_bLoading;
		 this->m_flLoading = m_flLoading;
		 this->m_Color = m_Color;
	  }

	  const char* m_szName = "";
	  int m_iPrioirity = -1;
	  bool m_bLoading = false;
	  float m_flLoading = 0.f;
	  FloatColor m_Color = FloatColor( 0, 0, 0, 255 );
   };

   std::vector< IndicatorsInfo_t > m_vecTextIndicators;

   struct EspData_t {
	  C_CSPlayer* player;
	  bool bEnemy;
	  bool isVisible;
	  Vector2D head_pos;
	  Vector2D feet_pos;
	  Vector origin;
	  Rect2D bbox;
	  player_info_t info;
   };

   EspData_t m_Data;
   C_CSPlayer* m_LocalPlayer = nullptr;
   C_CSPlayer* m_LocalObserved = nullptr;

   int storedTick = 0;
   int crouchedTicks[ 64 ];
   float m_flAplha[ 64 ];

   float lastTime = 0.0f;
   int oldframecount = 0;
   int curfps = 0;

   bool m_bAlphaFix[ 65 ];
   bool Begin( C_CSPlayer* player );
   bool ValidPlayer( C_CSPlayer* player );
   bool IsVisibleScan( C_CSPlayer* player );
   void AmmoBar( C_CSPlayer* player, Rect2D bbox );
   void RenderDroppedWeapon( C_WeaponCSBaseGun* entity );
   void RenderNades( C_WeaponCSBaseGun* nade );
   void RenderPlantedC4( C_BaseEntity* ent, Rect2D bbox );
   void DrawAntiAimIndicator( );
   void RenderSnapline( Vector2D pos );
   void DrawBox( Rect2D bbox, const FloatColor& clr, C_CSPlayer* player );
   void DrawArmorBar( C_CSPlayer* player, Rect2D bbox );
   void DrawHealthBar( C_CSPlayer* player, Rect2D bbox );
   void DrawInfo( C_CSPlayer* player, Rect2D bbox, player_info_t player_info );
   void DrawBottomInfo( C_CSPlayer* player, Rect2D bbox, player_info_t player_info );
   void DrawName( C_CSPlayer* player, Rect2D bbox, player_info_t player_info );
   void DrawSkeleton( C_CSPlayer* player );
   void DrawAimPoints( C_CSPlayer* player );
   bool GetBBox( C_BaseEntity* player, Vector2D screen_points[], Rect2D& outRect );
   void Offscreen( );
   void OverlayInfo( );
   void DrawTextIndicators( );
   void Indicators( );
   void BloomEffect( );
   bool IsFakeDucking( C_CSPlayer* player ) {
	  float duckamount = player->m_flDuckAmount( );
	  if ( !duckamount ) {
		 crouchedTicks[ player->entindex( ) ] = 0;
		 return false;
	  }

	  float duckspeed = player->m_flDuckSpeed( );
	  if ( !duckspeed ) {
		 crouchedTicks[ player->entindex( ) ] = 0;
		 return false;
	  }

	  if ( storedTick != Source::m_pGlobalVars->tickcount ) {
		 crouchedTicks[ player->entindex( ) ]++;
		 storedTick = Source::m_pGlobalVars->tickcount;
	  }

	  if ( int( duckspeed ) == 8 && duckamount <= 0.9f && duckamount > 0.01
		   && ( player->m_fFlags( ) & FL_ONGROUND ) && ( crouchedTicks[ player->entindex( ) ] >= 5 ) )
		 return true;
	  else
		 return false;
   };

   C_Window m_KeyBinds = { Vector2D( g_Vars.esp.keybind_x, g_Vars.esp.keybind_y ), Vector2D( 180, 10 ), 0 };
   C_Window m_Hitlogger = { Vector2D( g_Vars.esp.hitlogger_x, g_Vars.esp.hitlogger_y ), Vector2D( 290, 90 ), 1 };
   C_Window m_SpecList = { Vector2D( g_Vars.esp.spectator_x, g_Vars.esp.spectator_y ), Vector2D( 150, 10 ), 2 };
   C_Window m_Indicators = { Vector2D( g_Vars.esp.keybind_x, g_Vars.esp.keybind_y ), Vector2D( 180, 10 ), 3 };

   void SpectatorList( );
   void HitLogger( );
   void Keybinds( );
};

void CEsp::BloomEffect( ) {
   static bool props = false;

   Source::m_pCvar->FindVar( XorStr( "mat_ambient_light_r" ) )->SetValue( g_Vars.esp.bloom_color.r );
   Source::m_pCvar->FindVar( XorStr( "mat_ambient_light_g" ) )->SetValue( g_Vars.esp.bloom_color.g );
   Source::m_pCvar->FindVar( XorStr( "mat_ambient_light_b" ) )->SetValue( g_Vars.esp.bloom_color.b );

   static ConVar* r_modelAmbientMin = Source::m_pCvar->FindVar( XorStr( "r_modelAmbientMin" ) );

   for ( int i = 0; i < Source::m_pEntList->GetHighestEntityIndex( ); i++ ) {
	  C_BaseEntity* pEntity = ( C_BaseEntity* ) Source::m_pEntList->GetClientEntity( i );

	  if ( !pEntity )
		 continue;

	  if ( pEntity->GetClientClass( )->m_ClassID == 69 ) {
		 auto pToneMap = ( CEnvTonemapContorller* ) pEntity;
		 if ( pToneMap ) {
			*pToneMap->m_bUseCustomAutoExposureMin( ) = true;
			*pToneMap->m_bUseCustomAutoExposureMax( ) = true;

			*pToneMap->m_flCustomAutoExposureMin( ) = 0.2f;
			*pToneMap->m_flCustomAutoExposureMax( ) = 0.2f;
			*pToneMap->m_flCustomBloomScale( ) = 10.1f;

			r_modelAmbientMin->SetValue( g_Vars.esp.model_brightness );
		 }
	  }
   }
}

void DrawWatermark( ) {
   int ping = 0;

   auto netchannel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
   if ( !netchannel.IsValid( ) )
	  ping = 0;
   else
	  ping = static_cast< int >( netchannel->GetAvgLatency( FLOW_OUTGOING ) * 1000.f );// delta between net graph and my calculate equal 8 always, pizdec
   //static_cast< int >( ( netchannel->GetLatency( FLOW_OUTGOING ) + netchannel->GetLatency( FLOW_INCOMING ) ) * 1000.0f );

   static float lastTime = 0.f;
   static int curfps = 0;
   static int oldframecount = 0;

   if ( std::fabsf( Source::m_pGlobalVars->realtime - lastTime ) >= 1.0f ) {
	  lastTime = Source::m_pGlobalVars->realtime;
	  curfps = Source::m_pGlobalVars->framecount - oldframecount;
	  oldframecount = Source::m_pGlobalVars->framecount;
   }

   time_t seconds = time( NULL );
   tm* timeinfo = localtime( &seconds );

   Vector2D screen = Render::Get( )->GetScreenSize( );

   std::stringstream str;
#ifdef BETA_MODE
   str << XorStr( "EEXOMI [BETA]" );
   if ( g_Vars.esp.watermark_name )
   str << XorStr( " | " ) << g_Vars.globals.c_login.c_str( );

   if ( g_Vars.esp.watermark_ip )
   str << XorStr( " | " ) << g_Vars.globals.server_adress.c_str( );
   
   std::string fps_str = std::to_string( curfps );
   std::string ping_str = std::to_string( ping );
   std::string hour = std::to_string( timeinfo->tm_hour );
   std::string minute = std::to_string( timeinfo->tm_min );
   std::string second = std::to_string( timeinfo->tm_sec );

   if ( timeinfo->tm_hour < 10 )
	  hour.insert( 0, "0" );

   if ( timeinfo->tm_min < 10 )
	  minute.insert( 0, "0" );

   if ( timeinfo->tm_sec < 10 )
	  second.insert( 0, "0" );

   if ( g_Vars.esp.watermark_fps )
   str << XorStr( " | fps: " ) << fps_str;
 
   if ( g_Vars.esp.watermark_ping )
   str << XorStr( " | ping: " ) << ping_str;

   if ( g_Vars.esp.watermark_time )
   str << XorStr( " | time: " ) << hour.c_str( ) << XorStr( ":" ) << minute.c_str( ) << XorStr( ":" ) << second.c_str( );
#else
   str << XorStr( "EEXOMI" );
   if ( g_Vars.esp.watermark_name )
	  str << XorStr( " | " ) << g_Vars.globals.c_login.c_str( );

   if ( g_Vars.esp.watermark_ip )
	  str << XorStr( " | " ) << g_Vars.globals.server_adress.c_str( );

   std::string fps_str = std::to_string( curfps );
   std::string ping_str = std::to_string( ping );
   std::string hour = std::to_string( timeinfo->tm_hour );
   std::string minute = std::to_string( timeinfo->tm_min );
   std::string second = std::to_string( timeinfo->tm_sec );

   if ( timeinfo->tm_hour < 10 )
	  hour.insert( 0, "0" );

   if ( timeinfo->tm_min < 10 )
	  minute.insert( 0, "0" );

   if ( timeinfo->tm_sec < 10 )
	  second.insert( 0, "0" );

   if ( g_Vars.esp.watermark_fps )
	  str << XorStr( " | fps: " ) << fps_str;

   if ( g_Vars.esp.watermark_ping )
	  str << XorStr( " | ping: " ) << ping_str;

   if ( g_Vars.esp.watermark_time )
	  str << XorStr( " | time: " ) << hour.c_str( ) << XorStr( ":" ) << minute.c_str( ) << XorStr( ":" ) << second.c_str( );
#endif

   Render::Get( )->SetTextFont( FONT_MENU );
   Vector2D vec_str = Vector2D( );
   vec_str = Render::Get( )->CalcTextSize( str.str( ).c_str( ) );

   int lenght = vec_str.Length( );

   Render::Get( )->AddRectFilled( Vector4D( screen.x - lenght - 36, 11, screen.x - 19, 30 ), Color( 81, 79, 79, 255 ).GetD3DColor( ) );
   Render::Get( )->AddRectFilled( Vector4D( screen.x - lenght - 35, 12, screen.x - 20, 29 ), Color( 70, 67, 67, 255 ).GetD3DColor( ) );
   Render::Get( )->AddRectFilled( Vector4D( screen.x - lenght - 36, 29, screen.x - 19, 31 ), g_Vars.misc.menu_ascent );

   Render::Get( )->AddText( Vector2D( screen.x - lenght - 28, 14 ), Color( 255, 255, 255, 255 ).GetD3DColor( ), 0, str.str( ).c_str( ) );
}

bool CEsp::IsVisibleScan( C_CSPlayer* player ) {
   matrix3x4_t boneMatrix[ MAXSTUDIOBONES ];
   Vector eyePos = m_LocalPlayer->GetEyePosition( );

   CGameTrace tr;
   Ray_t ray;
   CTraceFilter filter;
   filter.pSkip = m_LocalPlayer;

   if ( !player->SetupBones( boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f ) ) {
	  return false;
   }

   auto studio_model = Source::m_pModelInfo->GetStudiomodel( player->GetModel( ) );
   if ( !studio_model ) {
	  return false;
   }

   int scan_hitboxes[] = {
	  HITBOX_HEAD,
	  HITBOX_LEFT_FOOT,
	  HITBOX_RIGHT_FOOT,
	  HITBOX_LEFT_CALF,
	  HITBOX_RIGHT_CALF,
	  HITBOX_CHEST,
	  HITBOX_STOMACH
   };

   for ( int i = 0; i < ARRAYSIZE( scan_hitboxes ); i++ ) {
	  auto hitbox = studio_model->pHitboxSet( player->m_nHitboxSet( ) )->pHitbox( scan_hitboxes[ i ] );
	  if ( hitbox ) {
		 auto
			min = Vector{},
			max = Vector{};

		 Math::VectorTransform( hitbox->bbmin, boneMatrix[ hitbox->bone ], min );
		 Math::VectorTransform( hitbox->bbmax, boneMatrix[ hitbox->bone ], max );

		 ray.Init( eyePos, ( min + max ) * 0.5 );
		 Source::m_pEngineTrace->TraceRay( ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr );

		 if ( tr.hit_entity == player || tr.fraction > 0.97f )
			return true;
	  }
   }
   return false;
}

bool CEsp::Begin( C_CSPlayer* player ) {
   m_Data.player = player;
   m_Data.bEnemy = player->m_iTeamNum( ) != m_LocalPlayer->m_iTeamNum( );
   m_Data.isVisible = IsVisibleScan( player );
   m_LocalObserved = ( C_CSPlayer* ) m_LocalPlayer->m_hObserverTarget( ).Get( );

   player_info_t player_info;
   if ( !Source::m_pEngine->GetPlayerInfo( player->entindex( ), &player_info ) )
	  return false;

   m_Data.info = player_info;

   int idx = player->entindex( );
   bool playerTeam = player->m_iTeamNum( ) == 2;

   if ( !m_Data.bEnemy && g_Vars.esp.team_check )
	  return false;

   Vector2D points[ 8 ];
   Rect2D bbox;

   Vector head = player->GetAbsOrigin( ) + Vector( 0, 0, player->GetCollideable( )->OBBMaxs( ).z );
   Vector origin = player->GetAbsOrigin( );
   origin.z -= 5;

   if ( !WorldToScreen( head, m_Data.head_pos ) ||
		!WorldToScreen( origin, m_Data.feet_pos ) )
	  return false;

   auto h = fabs( m_Data.head_pos.y - m_Data.feet_pos.y ) + 2;
   auto w = h / 1.8f;

   m_Data.bbox.left = static_cast< long >( m_Data.feet_pos.x - w * 0.5f );
   m_Data.bbox.right = static_cast< long >( m_Data.bbox.left + w );
   m_Data.bbox.bottom = ( m_Data.feet_pos.y > m_Data.head_pos.y ? static_cast< long >( m_Data.feet_pos.y ) : static_cast< long >( m_Data.head_pos.y ) );
   m_Data.bbox.top = ( m_Data.feet_pos.y > m_Data.head_pos.y ? static_cast< long >( m_Data.head_pos.y ) : static_cast< long >( m_Data.feet_pos.y ) );

   return true;
}

bool CEsp::ValidPlayer( C_CSPlayer* player ) {
   if ( !player )
	  return false;

   int idx = player->entindex( );
   constexpr float frequency = 255.f / 0.2f;
   float step = frequency * Source::m_pGlobalVars->frametime;

   if ( g_Vars.globals.m_bRenderingDormant[ player->entindex( ) ] )
	  return false;

   if ( player->IsDead( ) ) {
	  m_flAplha[ idx ] = 0.f;
	  return false;
   }

   static auto g_GameRules = *( uintptr_t** ) ( Engine::Displacement.Data.m_GameRules );
   if ( *( bool* ) ( *( uintptr_t* ) g_GameRules + 0x20 ) ) {
	  if ( player->IsDormant( ) ) {
		 m_flAplha[ idx ] = 0.f;
	  }
	  return false;
   }

   if ( g_Vars.esp.fade_esp ) {
	  if ( player->IsDormant( ) ) {
		 if ( m_flAplha[ idx ] < 160.f ) {
			m_flAplha[ idx ] -= 255.f / 16.f * Source::m_pGlobalVars->frametime;
			m_flAplha[ idx ] = std::clamp( m_flAplha[ idx ], 0.f, 160.f );
		 } else {
			m_flAplha[ idx ] -= 255.f / 1.f * Source::m_pGlobalVars->frametime;
		 }
	  } else {
		 m_flAplha[ idx ] += 255.f / 0.68f * Source::m_pGlobalVars->frametime;
		 m_flAplha[ idx ] = std::clamp( m_flAplha[ idx ], 0.f, 255.f );
	  }
   } else {
	  if ( player->IsDormant( ) )
		 m_flAplha[ idx ] = 0.f;
	  else
		 m_flAplha[ idx ] = 255.f;
   }

   return ( m_flAplha[ idx ] > 0.f );
}


void CEsp::DrawTextIndicators( ) {
   Vector2D screen = Render::Get( )->GetScreenSize( );
   C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

   if ( !LocalPlayer || LocalPlayer->IsDead( ) || g_Vars.esp.text_indicators )
	  return;

   float desyncFactor = 0.1f;
   float chokeFactor = 0.1f;
   float lcFactor = 0.1f;

   CCSGOPlayerAnimState* animState = LocalPlayer->m_PlayerAnimState( );
   if ( animState ) {
	  desyncFactor = Math::Clamp<float>( animState->GetMaxFraction( ), 0.1f, animState->GetMaxFraction( ) );
	  desyncFactor = 1.0 - ( 1.0 - desyncFactor ) * 2;
   }
   chokeFactor = ( float ) ( float( Source::m_pClientState->m_nChokedCommands( ) ) / float( g_Vars.fakelag.lag_limit ) );
   chokeFactor = Math::Clamp<float>( chokeFactor, 0.1f, chokeFactor );

   if ( ( g_Vars.globals.LagOrigin - m_LocalPlayer->m_vecOrigin( ) ).LengthSquared( ) > 4096.f )
	  lcFactor = 1.f;

   float alpha = 1.f;

   if ( Source::m_pClient->IsChatRaised( ) )
	  alpha = 0.1f;

   {
	  // fill indicators container
	  float r, g, b;
	  IndicatorsInfo_t Indicator = IndicatorsInfo_t( );

	  if ( g_Vars.esp.indicator_fake && g_Vars.antiaim.enabled ) {
		 ImGui::ColorConvertHSVtoRGB( ( 82.f / 360.f ) * desyncFactor, 0.89f, 0.78f, r, g, b );

		 FloatColor fake_color = FloatColor( r, g, b, alpha );

		 Indicator = IndicatorsInfo_t( XorStr( "FAKE" ), 1, false, 0.f, fake_color );

		 m_vecTextIndicators.emplace_back( Indicator );
	  }

	  if ( g_Vars.esp.indicator_lc && !( LocalPlayer->m_fFlags( ) & FL_ONGROUND ) ) {
		 ImGui::ColorConvertHSVtoRGB( ( 82.f / 360.f ) * lcFactor, 0.89f, 0.78f, r, g, b );

		 FloatColor lc_color = FloatColor( r, g, b, alpha );

		 Indicator = IndicatorsInfo_t( XorStr( "LC" ), 2, false, 0.f, lc_color );

		 m_vecTextIndicators.emplace_back( Indicator );
	  }
	  if ( g_Vars.esp.indicator_lag ) {
		 ImGui::ColorConvertHSVtoRGB( ( 82.f / 360.f ) * chokeFactor, 0.89f, 0.78f, r, g, b );

		 FloatColor choke_color = FloatColor( r, g, b, alpha );

		 Indicator = IndicatorsInfo_t( XorStr( "LAG" ), 2, false, 0.f, choke_color );

		 m_vecTextIndicators.emplace_back( Indicator );
	  }
	  if ( g_Vars.esp.indicator_side && g_Vars.antiaim.enabled ) {
		 if ( g_Vars.antiaim.desync_flip_bind.enabled ) {
			float r, g, b;
			ImGui::ColorConvertHSVtoRGB( ( 82.f / 360.f ) * 1.f, 0.89f, 0.78f, r, g, b );

			FloatColor clr = FloatColor( r, g, b, alpha );

			Indicator = IndicatorsInfo_t( XorStr( "LEFT" ), 0, false, 0.f, clr );

			m_vecTextIndicators.emplace_back( Indicator );
		 } else {
			float r, g, b;
			ImGui::ColorConvertHSVtoRGB( ( 82.f / 360.f ) * 1.f, 0.89f, 0.78f, r, g, b );

			FloatColor clr = FloatColor( r, g, b, alpha );
			Indicator = IndicatorsInfo_t( XorStr( "RIGHT" ), 0, false, 0.f, clr );

			m_vecTextIndicators.emplace_back( Indicator );
		 }
	  }

	  if ( g_Vars.esp.indicator_fake_duck ) {
		 if ( g_Vars.misc.fakeduck && g_Vars.misc.fakeduck_bind.enabled ) {
			Indicator = IndicatorsInfo_t( XorStr( "DUCK" ), 0, false, 0.f, FloatColor( 1.f, 1.f, 1.f, alpha ) );

			m_vecTextIndicators.emplace_back( Indicator );
		 }
	  }

	  if ( g_Vars.esp.indicator_exploits ) {
		 if ( g_Vars.rage.exploit && g_Vars.rage.exploit_type == 0 && g_Vars.rage.double_tap_bind.enabled && !g_Vars.rage.break_lagcomp ) {
			//https://github.com/tickcount/gamesense-lua/blob/master/Double%20tap%20corrections.lua#L230 for color and visualize time to next dt
			Indicator = IndicatorsInfo_t( XorStr( "DT" ), 0, false, 0.f, FloatColor( 1.f, 1.f, 1.f, alpha ) );

			m_vecTextIndicators.emplace_back( Indicator );
		 }

		 if ( g_Vars.rage.exploit && g_Vars.rage.exploit_type == 0 && g_Vars.rage.hide_shots_bind.enabled && !g_Vars.rage.break_lagcomp ) {
			Indicator = IndicatorsInfo_t( XorStr( "HS" ), 0, false, 0.f, FloatColor( 1.f, 1.f, 1.f, alpha ) );

			m_vecTextIndicators.emplace_back( Indicator );
		 }
	  }
   }

   // sort indicators by priority
   {
	  std::sort( m_vecTextIndicators.begin( ), m_vecTextIndicators.end( ), [] ( IndicatorsInfo_t a, IndicatorsInfo_t b ) {
		 return a.m_iPrioirity < b.m_iPrioirity;
	  } );
   }

   // render
   {
	  for ( int i = 0; i < m_vecTextIndicators.size( ); i++ ) {
		 const auto& indicator = m_vecTextIndicators.at( i );
		 Render::Get( )->SetTextFont( FONT_VERDANA_40_BOLD );
		 Render::Get( )->AddText( Vector2D( 15, ( screen.y - 65 ) - Render::Get( )->CalcTextSize( indicator.m_szName ).y * ( i + 1 ) ), indicator.m_Color, DROP_SHADOW, indicator.m_szName );
		 if ( indicator.m_bLoading ) {
			// TODO: Soufiw render circle with loading
			// Use indicator.m_flLoading for frac
		 }
	  }
   }

   m_vecTextIndicators.clear( );
}

void CEsp::Indicators( ) {
   C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );

   if ( !pLocal || pLocal->IsDead( ) )
	  return;

   float desyncFactor = 0.1f;
   float chokeFactor = 0.1f;

   CCSGOPlayerAnimState* animState = pLocal->m_PlayerAnimState( );
   if ( animState ) {
	  desyncFactor = Math::Clamp<float>( animState->GetMaxFraction( ), 0.0f, animState->GetMaxFraction( ) );
	  desyncFactor = 1.0 - ( 1.0 - desyncFactor ) * 2;
   }
   chokeFactor = ( float ) ( float( Source::m_pClientState->m_nChokedCommands( ) ) / float( g_Vars.fakelag.lag_limit ) );
   chokeFactor = Math::Clamp<float>( chokeFactor, 0.0f, chokeFactor );

   this->m_Indicators.Drag( );

   Vector2D pos = { g_Vars.esp.indicators_x, g_Vars.esp.indicators_y };

   this->m_Indicators.size.y = 43.0f;

   const auto header = FloatColor( 0.15f, 0.15f, 0.15f, 1.0f );
   const auto entry = FloatColor( 0.10f, 0.10f, 0.10f, 1.0f );
   const auto textColor = FloatColor( 0.95f, 0.95f, 0.95f, 1.0f );

   const auto backColor = FloatColor( 0.09f, 0.09f, 0.09f, 1.0f );

   float r, g, b;
   ImGui::ColorConvertHSVtoRGB( ( 82.f / 360.f ) * chokeFactor, 0.89f, 0.78f, r, g, b );

   FloatColor choke_color = FloatColor( r, g, b );

   Render::Get( )->AddRectFilled( pos, pos +
								  this->m_Indicators.size, header );

   Render::Get( )->SetTextFont( FONT_VISITOR );

   auto saved_pos1 = pos + Vector2D( 0.0f, this->m_Indicators.size.y - 33.0f );
   auto saved_pos2 = pos + Vector2D( this->m_Indicators.size.x - 2.0f, this->m_Indicators.size.y - 34.0f );

   Render::Get( )->AddText( pos + Vector2D( 2.0f, 1.0f ), textColor, OUTLINED, XorStr( "INDICATORS" ) );
   Render::Get( )->AddLine( saved_pos1, saved_pos2, g_Vars.misc.menu_ascent );

   float lenght_1 = this->m_Indicators.size.x - 2.0f - ( Render::Get( )->CalcTextSize( XorStr( "FAKELAG  i" ) ).x + 10.f );

   Render::Get( )->AddText( pos + Vector2D( 2.0f, 15.0f ), textColor, OUTLINED, XorStr( "FAKELAG %i" ), Source::m_pClientState->m_nChokedCommands( ) );
   Render::Get( )->AddRectFilled( pos + Vector2D( Render::Get( )->CalcTextSize( XorStr( "FAKELAG  i" ) ).x + 10.f, 15.f ), pos + Vector2D( ( this->m_Indicators.size.x - 2.f ), 21.f ), backColor );
   Render::Get( )->AddRectFilled( pos + Vector2D( Render::Get( )->CalcTextSize( XorStr( "FAKELAG  i" ) ).x + 10.f, 15.f ), pos + Vector2D( ( Render::Get( )->CalcTextSize( XorStr( "FAKELAG  i" ) ).x + 8.f ) + lenght_1 * chokeFactor, 21.f ), choke_color );

   ImGui::ColorConvertHSVtoRGB( ( 82.f / 360.f ) * animState->GetMaxFraction( ), 0.89f, 0.78f, r, g, b );

   FloatColor fake_color = FloatColor( r, g, b );

   Render::Get( )->AddText( pos + Vector2D( 2.0f, 30.0f ), textColor, OUTLINED, XorStr( "DESYNC  %i" ), int( animState->m_flMaxBodyYaw * animState->GetMaxFraction( ) ) );
   Render::Get( )->AddRectFilled( pos + Vector2D( Render::Get( )->CalcTextSize( XorStr( "DESYNC   i" ) ).x + 10.f, 30.f ), pos + Vector2D( ( this->m_Indicators.size.x - 2.f ), 36.f ), backColor );
   Render::Get( )->AddRectFilled( pos + Vector2D( Render::Get( )->CalcTextSize( XorStr( "DESYNC   i" ) ).x + 10.f, 30.f ), pos + Vector2D( ( Render::Get( )->CalcTextSize( XorStr( "DESYNC   i" ) ).x + 8.f ) + lenght_1 * animState->GetMaxFraction( ), 36.f ), fake_color );
}

void CEsp::SpectatorList( ) {
   std::vector<std::string> vecNames;

   C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );

   this->m_SpecList.Drag( );

   Vector2D pos = { g_Vars.esp.spectator_x, g_Vars.esp.spectator_y };

   this->m_SpecList.size.y = 13.0f;

   const auto header = FloatColor( 0.15f, 0.15f, 0.15f, 1.0f );
   const auto entry = FloatColor( 0.10f, 0.10f, 0.10f, 1.0f );
   const auto textColor = FloatColor( 0.95f, 0.95f, 0.95f, 1.0f );

   Render::Get( )->AddRectFilled( pos, pos +
								  this->m_SpecList.size, header );

   Render::Get( )->SetTextFont( FONT_VISITOR );

   auto saved_pos1 = pos + Vector2D( 0.0f, this->m_SpecList.size.y - 2.0f );
   auto saved_pos2 = pos + Vector2D( this->m_SpecList.size.x - 1.0f, this->m_SpecList.size.y - 2.0f );

   Render::Get( )->AddText( pos + Vector2D( 2.0f, 1.0f ), textColor, OUTLINED, XorStr( "SPECTATORS" ) );

   for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; i++ ) {
	  C_CSPlayer* pEnt = C_CSPlayer::GetPlayerByIndex( i );
	  if ( !pEnt || pEnt->IsDormant( ) || !pEnt->IsDead( ) || pEnt == pLocal )
		 continue;

	  auto pObserver = pEnt->m_hObserverTarget( ).Get( );
	  if ( pObserver && pObserver == pLocal ) {
		 player_info_t info;
		 if ( !Source::m_pEngine->GetPlayerInfo( i, &info ) ) {
			continue;
		 }

		 this->m_SpecList.size.y += 13.0f;
		 auto& name = vecNames.emplace_back( info.szName );
		 while ( name.length( ) > 16 )
			name.pop_back( );
	  }
   }

   Render::Get( )->SetTextFont( FONT_VERDANA );
   if ( !vecNames.empty( ) ) {
	  Render::Get( )->AddRectFilled( pos + Vector2D{ 0.0f, 13.0f },
									 pos + Vector2D{ this->m_SpecList.size.x, this->m_SpecList.size.y + 4.0f },
									 entry );

	  float offset = 14.0f;
	  for ( auto name : vecNames ) {
		 Render::Get( )->AddText( pos + Vector2D( 2.0f, offset ), textColor, DROP_SHADOW, name.c_str( ) );
		 // Render::Get( )->AddLine( this->m_SpecList.pos + Vector2D( 0.0f, offset ), this->m_SpecList.pos + Vector2D(this->m_SpecList.size, g_Vars.misc.menu_ascent );
		 offset += 13.0f;
	  }
   }
   Render::Get( )->AddLine( saved_pos1, saved_pos2, g_Vars.misc.menu_ascent );
}

void CEsp::HitLogger( ) {
   const char* reasons_array[] = {
	  XorStr( "Fakes" ),
	  XorStr( "Spread" ),
	  XorStr( "-" ),
	  XorStr( "Server" ),
	  XorStr( "Pred D" ),
	  XorStr( "Pred S" ),
	  XorStr( "Server" ),
	  XorStr( "Occlusion" ),
	  XorStr( "Backtrack" ),
	  XorStr( "Unknown" ),
   };

   auto TranslateReason = [reasons_array] ( int index ) {
	  if ( index < 0 || index > 9 )
		 return XorStr( "Unknown" );

	  return reasons_array[ index ];
   };

   auto TranslateColor = [] ( int index ) -> FloatColor {
	  switch ( index ) {
		 case 0:
		 return FloatColor( 255, 84, 84 ); break;
		 case 1:
		 return FloatColor( 118, 171, 255 ); break;
		 case 2:
		 return FloatColor( 94, 230, 75 ); break;
		 case 3:
		 return FloatColor( 0.9f, 0.55f, 0.0f ); break;
		 case 4:
		 return FloatColor( 255, 84, 84 ); break;
		 case 5:
		 return FloatColor( 118, 171, 255 ); break;
		 case 6:
		 return FloatColor( 1.f, 1.f, 1.f ); break;
		 case 7:
		 return FloatColor( 118, 171, 255 ); break;
		 default:
		 return FloatColor( 0.f, 1.f, 0.f );
		 break;
	  }
   };

   auto TranslateSide = [] ( int index ) {
	  if ( !index )
		 return ( XorStr( "Fake" ) );
	  return index > 0 ? XorStr( "Left" ) : XorStr( "Right" );
   };

   auto TranslateHitbox = [] ( int hitbox, bool aimdata ) -> std::string {
	  if ( !aimdata ) {
		 switch ( hitbox ) {
			case Hitgroup_Generic:
			return XorStr( "Generic" );
			case Hitgroup_Head:
			return XorStr( "Head" );
			case Hitgroup_Chest:
			return XorStr( "Chest" );
			case Hitgroup_Stomach:
			return XorStr( "Stomach" );
			case Hitgroup_LeftArm:
			case Hitgroup_RightArm:
			return XorStr( "Arm" );
			case Hitgroup_LeftLeg:
			case Hitgroup_RightLeg:
			return XorStr( "Leg" );
			case Hitgroup_Gear:
			return XorStr( "Gear" );
		 }
		 return XorStr( "-" );
	  } else {
		 switch ( hitbox ) {
			case HITBOX_HEAD:
			return XorStr( "Head" );
			case HITBOX_NECK:
			return XorStr( "Neck" );
			case HITBOX_CHEST:
			case HITBOX_LOWER_CHEST:
			return XorStr( "Chest" );
			case HITBOX_RIGHT_FOOT:
			case HITBOX_RIGHT_CALF:
			case HITBOX_RIGHT_THIGH:
			case HITBOX_LEFT_FOOT:
			case HITBOX_LEFT_CALF:
			case HITBOX_LEFT_THIGH:
			return XorStr( "Leg" );
			case HITBOX_LEFT_FOREARM:
			case HITBOX_LEFT_HAND:
			case HITBOX_LEFT_UPPER_ARM:
			case HITBOX_RIGHT_FOREARM:
			case HITBOX_RIGHT_HAND:
			case HITBOX_RIGHT_UPPER_ARM:
			return XorStr( "Arm" );
			case HITBOX_STOMACH:
			return XorStr( "Stomach" );
			case HITBOX_PELVIS:
			return XorStr( "Pelvis" );
			default:
			return XorStr( "-" );
		 }
	  }
   };

   C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );

   this->m_Hitlogger.Drag( );

   Vector2D pos = { g_Vars.esp.hitlogger_x, g_Vars.esp.hitlogger_y };

   this->m_Hitlogger.size.y = 13.0f;

   const auto header = FloatColor( 0.15f, 0.15f, 0.15f, 1.0f );
   const auto entry = FloatColor( 0.10f, 0.10f, 0.10f, 0.4f );
   const auto textColor = FloatColor( 0.95f, 0.95f, 0.95f, 1.0f );

   Render::Get( )->AddRectFilled( pos, pos +
								  this->m_Hitlogger.size, header );

   Render::Get( )->SetTextFont( FONT_VISITOR );
   auto saved_pos1 = pos + Vector2D( 0.0f, this->m_Hitlogger.size.y - 2.0f );
   auto saved_pos2 = pos + Vector2D( this->m_Hitlogger.size.x - 1.0f, this->m_Hitlogger.size.y - 2.0f );

   Render::Get( )->AddText( pos + Vector2D( 2.0f, 1.0f ), textColor, OUTLINED, XorStr( "NAME" ) );
   Render::Get( )->AddText( pos + Vector2D( 100.0f + 1.0f, 1.0f ), textColor, OUTLINED, XorStr( "DMG" ) );
   Render::Get( )->AddText( pos + Vector2D( 140.0f + 1.0f, 1.0f ), textColor, OUTLINED, XorStr( "TICKS" ) );
   Render::Get( )->AddText( pos + Vector2D( 170.0f + 1.0f, 1.0f ), textColor, OUTLINED, XorStr( "HITBOX" ) );
   Render::Get( )->AddText( pos + Vector2D( 225.0f + 1.0f, 1.0f ), textColor, OUTLINED, XorStr( "REASON" ) );

   auto FixedStrLenght = [] ( std::string str ) -> std::string {
	  if ( ( int ) str[ 0 ] > 255 )
		 return rand( ) % 2 ? XorStr( "dolbaeb" ) : XorStr( "debil" );

	  if ( str.size( ) < 15 )
		 return str;

	  std::string result;
	  for ( size_t i = 0; i < 15u; i++ )
		 result.push_back( str.at( i ) );
	  return result;
   };

   if ( IRoundFireBulletsStore::Get( )->GetVecSize( ) > 0 ) {
	  this->m_Hitlogger.size.y += IRoundFireBulletsStore::Get( )->GetVecSize( ) * 13.0f;

	  Render::Get( )->AddRectFilled( pos + Vector2D{ 0.0f, 13.0f },
									 pos + Vector2D{ this->m_Hitlogger.size.x, this->m_Hitlogger.size.y + 2.0f },
									 entry );

	  Render::Get( )->SetTextFont( FONT_VERDANA );

	  for ( int i = 0; i < IRoundFireBulletsStore::Get( )->GetVecSize( ); i++ ) {
		 auto element = IRoundFireBulletsStore::Get( )->GetStats( i );
		 int damage = element->m_iDamage > 0 ? element->m_iDamage : element->m_iEventDamage;

		 std::string hitbox = TranslateHitbox( element->m_iHitbox, element->m_iDamage > 0 );
		 FloatColor col = TranslateColor( element->m_iReason );
		 Render::Get( )->AddRectFilled( pos + Vector2D{ 1.0f, 13.0f + i * 13.0f }, pos + Vector2D{ 3.0f, 25.0f + i * 13.0f }, col );
		 Render::Get( )->AddText( pos + Vector2D{ 4.0f, 13.0f + i * 13.0f }, textColor, DROP_SHADOW, FixedStrLenght( element->m_name ).c_str( ) );
		 Render::Get( )->AddText( pos + Vector2D{ 100.0f + 1.0f, 13.0f + i * 13.0f }, textColor, DROP_SHADOW, damage == 0 ? XorStr( "-" ) : std::to_string( damage ).c_str( ) );
		 Render::Get( )->AddText( pos + Vector2D{ 140.0f + 1.0f, 13.0f + i * 13.0f }, textColor, DROP_SHADOW, element->m_iLagCompTicks == 0 ? XorStr( "-" ) : std::to_string( element->m_iLagCompTicks ).c_str( ) );
		 Render::Get( )->AddText( pos + Vector2D{ 170.0f + 1.0f, 13.0f + i * 13.0f }, textColor, DROP_SHADOW, hitbox.c_str( ) );
		 Render::Get( )->AddText( pos + Vector2D{ 225.0f + 1.0f, 13.0f + i * 13.0f }, textColor, DROP_SHADOW, TranslateReason( element->m_iReason ) );
	  }
   }

   Render::Get( )->AddLine( saved_pos1, saved_pos2, g_Vars.misc.menu_ascent );
}

void CEsp::Keybinds( ) {
   std::vector<
	  std::pair<std::string, int>
   > vecNames;

   C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );

   this->m_KeyBinds.Drag( );

   Vector2D pos = { g_Vars.esp.keybind_x, g_Vars.esp.keybind_y };

   this->m_KeyBinds.size.y = 13.0f;

   const auto header = FloatColor( 0.15f, 0.15f, 0.15f, 1.0f );
   const auto entry = FloatColor( 0.10f, 0.10f, 0.10f, 1.0f );
   const auto textColor = FloatColor( 0.95f, 0.95f, 0.95f, 1.0f );

   Render::Get( )->AddRectFilled( pos, pos +
								  this->m_KeyBinds.size, header );

   Render::Get( )->SetTextFont( FONT_VISITOR );

   auto saved_pos1 = pos + Vector2D( 0.0f, this->m_KeyBinds.size.y - 2.0f );
   auto saved_pos2 = pos + Vector2D( this->m_KeyBinds.size.x - 1.0f, this->m_KeyBinds.size.y - 2.0f );

   Render::Get( )->AddText( pos + Vector2D( 2.0f, 1.0f ), textColor, OUTLINED, XorStr( "KEYBINDS" ) );

   Render::Get( )->SetTextFont( FONT_VERDANA );

   static const auto hold_size = Render::Get( )->CalcTextSize( XorStr( "[Hold]" ) );
   static const auto toggle_size = Render::Get( )->CalcTextSize( XorStr( "[Toggle]" ) );

   auto AddBind = [this, &vecNames] ( const char* name, KeyBind_t& bind ) {
	  if ( bind.cond == KeyBindType::ALWAYS_ON || !bind.enabled )
		 return;

	  vecNames.push_back( std::pair<std::string, int>( std::string( name ), bind.cond ) );
	  this->m_KeyBinds.size.y += 13.0f;
   };

   if ( !m_LocalPlayer )
	  m_LocalPlayer = C_CSPlayer::GetLocalPlayer( );

   if ( g_Vars.rage.enabled && m_LocalPlayer ) {
	  CVariables::RAGE* rbot = nullptr;
	  auto weapon = ( C_WeaponCSBaseGun* ) m_LocalPlayer->m_hActiveWeapon( ).Get( );
	  if ( weapon ) {
		 auto weaponInfo = weapon->GetCSWeaponData( );
		 if ( weaponInfo.IsValid( ) ) {
			auto id = weapon->m_iItemDefinitionIndex( );
			if ( id != WEAPON_ZEUS ) {
			   switch ( weaponInfo->m_iWeaponType ) {
				  case WEAPONTYPE_PISTOL:
				  if ( id == WEAPON_DEAGLE || id == WEAPON_REVOLVER )
					 rbot = &g_Vars.rage_heavypistols;
				  else
					 rbot = &g_Vars.rage_pistols;
				  break;
				  case WEAPONTYPE_SUBMACHINEGUN:
				  rbot = &g_Vars.rage_smgs;
				  break;
				  case WEAPONTYPE_RIFLE:
				  rbot = &g_Vars.rage_rifles;
				  break;
				  case WEAPONTYPE_SHOTGUN:
				  rbot = &g_Vars.rage_shotguns;
				  break;
				  case WEAPONTYPE_SNIPER_RIFLE:
				  if ( id == WEAPON_G3SG1 || id == WEAPON_SCAR20 )
					 rbot = &g_Vars.rage_autosnipers;
				  else
					 rbot = ( id == WEAPON_AWP ) ? &g_Vars.rage_awp : &g_Vars.rage_scout;
				  break;
				  case WEAPONTYPE_MACHINEGUN:
				  rbot = &g_Vars.rage_heavys;
				  break;
				  default:
				  rbot = &g_Vars.rage_default;
				  break;
			   }
			} else {
			   rbot = &g_Vars.rage_taser;
			}

			if ( !rbot || !rbot->active ) {
			   rbot = &g_Vars.rage_default;
			}
		 }
	  }

	  if ( g_Vars.rage.exploit ) {
		 if ( g_Vars.rage.exploit_type == 0 ) {
			AddBind( XorStr( "Double tap" ), g_Vars.rage.double_tap_bind );
			AddBind( XorStr( "Hide shots" ), g_Vars.rage.hide_shots_bind );
		 } else {
			AddBind( XorStr( "Rapid charge" ), g_Vars.rage.rapid_charge );
			AddBind( XorStr( "Rapid release" ), g_Vars.rage.rapid_release );
		 }
	  }

	  if ( rbot ) {
		 if ( rbot->min_damage_override )
			AddBind( XorStr( "Damage override" ), g_Vars.rage.key_dmg_override );

		 if ( rbot->override_hitscan )
			AddBind( XorStr( "Hitscan override" ), g_Vars.rage.override_key );

		 if ( rbot->prefer_safety )
			AddBind( XorStr( "Prefer safety" ), g_Vars.rage.prefer_safe );

		 if ( rbot->prefer_body )
			AddBind( XorStr( "Prefer body" ), g_Vars.rage.prefer_body );

		 if ( rbot->prefer_head )
			AddBind( XorStr( "Prefer head" ), g_Vars.rage.prefer_head );
	  }
   }

   if ( g_Vars.antiaim.enabled ) {
	  //autodirection_override
	  AddBind( XorStr( "Desync flip" ), g_Vars.antiaim.desync_flip_bind );
	  AddBind( XorStr( "Autodirection override" ), g_Vars.antiaim.autodirection_override );
   }

   if ( g_Vars.misc.active ) {
	  if ( g_Vars.misc.edgejump )
		 AddBind( XorStr( "Edge jump" ), g_Vars.misc.edgejump_bind );
	  if ( g_Vars.misc.fakeduck )
		 AddBind( XorStr( "Fakeduck" ), g_Vars.misc.fakeduck_bind );
	  if ( g_Vars.misc.slow_walk )
		 AddBind( XorStr( "Slow walk" ), g_Vars.misc.slow_walk_bind );

	  if ( g_Vars.misc.autopeek && !AutoPeekPos.IsZero( ) ) {
		 vecNames.push_back( std::pair<std::string, int>( std::string( "Auto peek" ), KeyBindType::TOGGLE ) );
		 this->m_KeyBinds.size.y += 13.0f;
	  }
   }

   if ( g_Vars.esp.zoom )
	  AddBind( XorStr( "Zoom" ), g_Vars.esp.zoom_key );

   if ( !vecNames.empty( ) ) {
	  Render::Get( )->AddRectFilled( pos + Vector2D{ 0.0f, 13.0f },
									 pos + Vector2D{ this->m_KeyBinds.size.x, this->m_KeyBinds.size.y + 4.0f },
									 entry );

	  float offset = 14.0f;
	  for ( auto name : vecNames ) {
		 Render::Get( )->AddText( pos + Vector2D( 2.0f, offset ), textColor, DROP_SHADOW, name.first.c_str( ) );
		 Render::Get( )->AddText( pos + Vector2D( this->m_KeyBinds.size.x - ( name.second == KeyBindType::HOLD ? hold_size.x : toggle_size.x ), offset ), textColor, DROP_SHADOW,
								  name.second == KeyBindType::HOLD ? XorStr( "Hold" ) : XorStr( "Toggle" ) );
		 offset += 13.0f;
	  }
   }

   Render::Get( )->AddLine( saved_pos1, saved_pos2, g_Vars.misc.menu_ascent );
}

float GetPenetrationDamage( C_CSPlayer* local, C_WeaponCSBaseGun* pWeapon ) {
   auto weaponInfo = pWeapon->GetCSWeaponData( );
   if ( !weaponInfo.IsValid( ) )
	  return -1.0f;

   Autowall::C_FireBulletData data;

   data.m_iPenetrationCount = 4;
   data.m_Player = local;
   data.m_TargetPlayer = nullptr;

   QAngle view_angles;
   Source::m_pEngine->GetViewAngles( view_angles );
   data.m_vecStart = local->GetEyePosition( );
   data.m_vecDirection = view_angles.ToVectors( );
   data.m_flMaxLength = data.m_vecDirection.Normalize( );
   data.m_WeaponData = weaponInfo.Xor( );
   data.m_flTraceLength = 0.0f;

   data.m_flCurrentDamage = static_cast< float >( weaponInfo->m_iWeaponDamage );

   CTraceFilter filter;
   filter.pSkip = local;

   Vector end = data.m_vecStart + data.m_vecDirection * weaponInfo->m_flWeaponRange;

   Autowall::TraceLine( data.m_vecStart, end, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &data.m_EnterTrace );
   Autowall::ClipTraceToPlayers( data.m_vecStart, end + data.m_vecDirection * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &data.m_EnterTrace );
   if ( data.m_EnterTrace.fraction == 1.f )
	  return -1.0f;

   data.m_flTraceLength += data.m_flMaxLength * data.m_EnterTrace.fraction;
   if ( data.m_flMaxLength != 0.0f && data.m_flTraceLength >= data.m_flMaxLength )
	  return data.m_flCurrentDamage;

   data.m_flCurrentDamage *= powf( weaponInfo->m_flRangeModifier, data.m_flTraceLength * 0.002f );
   data.m_EnterSurfaceData = Source::m_pPhysSurface->GetSurfaceData( data.m_EnterTrace.surface.surfaceProps );

   C_BasePlayer* hit_player = static_cast< C_BasePlayer* >( data.m_EnterTrace.hit_entity );
   bool can_do_damage = ( data.m_EnterTrace.hitgroup >= Hitgroup_Head && data.m_EnterTrace.hitgroup <= Hitgroup_Gear );
   bool hit_target = !data.m_TargetPlayer || hit_player == data.m_TargetPlayer;
   if ( can_do_damage && hit_player && hit_player->entindex( ) <= Source::m_pGlobalVars->maxClients && hit_player->entindex( ) > 0 && hit_target ) {
	  if ( pWeapon && pWeapon->m_iItemDefinitionIndex( ) == WEAPON_ZEUS )
		 return ( data.m_flCurrentDamage * 0.9f );

	  data.m_flCurrentDamage = Autowall::ScaleDamage( ( C_CSPlayer* ) hit_player, data.m_flCurrentDamage, weaponInfo->m_flArmorRatio, data.m_EnterTrace.hitgroup );
	  return data.m_flCurrentDamage;
   };

   if ( data.m_flTraceLength > 3000.0f && weaponInfo->m_flPenetration > 0.f || 0.1f > data.m_EnterSurfaceData->game.flPenetrationModifier )
	  return -1.0f;

   if ( !Autowall::HandleBulletPenetration( &data ) )
	  return -1.0f;

   return data.m_flCurrentDamage;
};

void CEsp::PenetrateCrosshair( Vector2D center ) {
   C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
   if ( !local || local->IsDead( ) )
	  return;

   C_WeaponCSBaseGun* pWeapon = ( C_WeaponCSBaseGun* ) local->m_hActiveWeapon( ).Get( );
   if ( !pWeapon )
	  return;

   auto dmg = int( GetPenetrationDamage( local, pWeapon ) );

   Render::Get( )->AddRectFilled(
	  Vector2D{ center.x - g_Vars.esp.autowall_crosshair_height, center.y - g_Vars.esp.autowall_crosshair_height },
	  Vector2D{ center.x + g_Vars.esp.autowall_crosshair_height + 1, center.y + g_Vars.esp.autowall_crosshair_height + 1 },
	  dmg >= 1.0f ? FloatColor( 0.0f, 1.0f, 0.0f, 1.0f ) : FloatColor( 1.0f, 0.0f, 0.0f, 1.0f ) );

   Render::Get( )->AddRectFilled(
	  Vector2D{ center.x - ( g_Vars.esp.autowall_crosshair_height - 1 ), center.y - ( g_Vars.esp.autowall_crosshair_height - 1 ) },
	  Vector2D{ center.x + ( g_Vars.esp.autowall_crosshair_height + 2 ), center.y + ( g_Vars.esp.autowall_crosshair_height + 2 ) },
	  FloatColor( 0.0f, 0.0f, 0.0f, 0.0f ) );

#if 0
   if ( dmg > 0.f ) {
	  Render::Get( )->SetTextFont( FONT_VISITOR );
	  Render::Get( )->AddText( { center.x, center.y + g_Vars.esp.autowall_crosshair_height + 3.0f }, FloatColor::White, CENTER_X, XorStr( "%i Damage" ), ( int ) dmg );
   }
#endif
}

#if 0
void DrawNetVars( C_CSPlayer* player ) {
   if ( player && !player->IsDead( ) ) {
	  auto& pPropManager = Engine::PropManager::Instance( );
	  auto cs_player = pPropManager->GetTable( XorStr( "DT_CSPlayer" ) );
	  auto base_player = pPropManager->GetTable( XorStr( "DT_BasePlayer" ) );
	  auto base_anim = pPropManager->GetTable( XorStr( "DT_BaseAnimating" ) );
	  auto base_entity = pPropManager->GetTable( XorStr( "DT_BaseEntity" ) );
	  auto DT_BaseCombatCharacter = pPropManager->GetTable( XorStr( "DT_BaseCombatCharacter" ) );
	  auto DT_BaseFlex = pPropManager->GetTable( XorStr( "DT_BaseFlex" ) );

	  struct cached_netvars {
		 SendPropType m_RecvType;
		 int offset;
		 const char* name;
	  };

	  static std::vector<cached_netvars> netvars;
	  static std::function<int( RecvTable * table )> add_all_netvars;
	  add_all_netvars = [] ( RecvTable* table ) {
		 int extra = 0;
		 for ( int i = 0; i < table->m_nProps; i++ ) {
			auto prop = &table->m_pProps[ i ];
			auto child = prop->m_pDataTable;

			if ( child && child->m_nProps ) {
			   int add = add_all_netvars( child );
			   if ( add )
				  extra += ( prop->m_Offset + add );
			} else if ( !isdigit( prop->m_pVarName[ 0 ] ) && prop->m_RecvType != DPT_Array && prop->m_RecvType != DPT_DataTable && prop->m_RecvType != DPT_String
						&& uint32_t( prop->m_Offset + extra ) > 0x40 ) {
			   netvars.push_back( { prop->m_RecvType, prop->m_Offset + extra, prop->m_pVarName } );
			}
		 }

		 return extra;
	  };

	  // cs_player : 050
	  if ( netvars.empty( ) ) {
		 //add_all_netvars( cs_player );
		 //add_all_netvars( DT_BaseCombatCharacter );
		 add_all_netvars( DT_BaseFlex );
		 //add_all_netvars( DT_BaseCombatCharacter );
	  }

	  Vector2D draw_pos = Vector2D( 200.0f, 10.0f );
	  float biggestWidth = 0.0f;

	  Render::Get( )->SetTextFont( FONT_VISITOR );

	  for ( auto& net : netvars ) {
		 static const auto MAX_BUFFER_SIZE = 4096;
		 static char buffer[ MAX_BUFFER_SIZE ] = u8"";

		 int kek = *( int* ) ( uintptr_t( player ) + net.offset );
		 float kek2 = *( float* ) ( uintptr_t( player ) + net.offset );
		 Vector kek3 = *( Vector* ) ( uintptr_t( player ) + net.offset );
		 Vector2D kek4 = *( Vector2D* ) ( uintptr_t( player ) + net.offset );

		 switch ( net.m_RecvType ) {
			case DPT_Int:
			// m_b
			if ( net.name[ 2 ] == XorStr( 'b' ) )
			   sprintf_s( buffer, XorStr( "%s : %s [%X]" ), net.name, ( *( bool* ) ( uintptr_t( player ) + net.offset ) ) ? XorStr( "true" ) : XorStr( "false" ), net.offset );
			else
			   sprintf_s( buffer, XorStr( "%s : %d [%X]" ), net.name, kek, net.offset );
			break;
			case DPT_Float:
			sprintf_s( buffer, XorStr( "%s : %.5f [%X]" ), net.name, kek2, net.offset );
			break;
			case DPT_Vector:
			sprintf_s( buffer, XorStr( "%s : x %.5f y %.5f z %.5f [%X]" ), net.name, kek3.x, kek3.y, kek3.z, net.offset );
			break;
			case DPT_VectorXY:
			sprintf_s( buffer, XorStr( "%s : x %.5f y %.5f [%X]" ), net.name, kek4.x, kek4.y, net.offset );
			break;
			default:
			continue;
}

		 Render::Get( )->AddText( draw_pos, 0xFFFFFFFF, OUTLINED, buffer );
		 Vector2D size = Render::Get( )->CalcTextSize( buffer );
		 biggestWidth = std::fmaxf( size.x, biggestWidth );

		 draw_pos.y += size.y + 1.0f;
		 if ( draw_pos.y >= 1050.0f ) {
			draw_pos.y = 10.0f;
			draw_pos.x += biggestWidth + 5.0f;
			biggestWidth = 0.0f;
		 }
	  }
   }
}÷
#endif

void CEsp::DrawZeusDistance( ) {
   if ( !g_Vars.esp.zeus_distance )
	  return;

   C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalPlayer( );

   if ( !pLocalPlayer || pLocalPlayer->IsDead( ) )
	  return;

   C_WeaponCSBaseGun* pWeapon = ( C_WeaponCSBaseGun* ) pLocalPlayer->m_hActiveWeapon( ).Get( );

   if ( !pWeapon )
	  return;

   auto pWeaponInfo = pWeapon->GetCSWeaponData( );
   if ( !pWeaponInfo.IsValid( ) )
	  return;

   if ( !( pWeapon->m_iItemDefinitionIndex( ) == WEAPON_ZEUS ) )
	  return;

   auto collision = pLocalPlayer->m_Collision( );
   Vector eyePos = pLocalPlayer->GetAbsOrigin( ) + ( collision->m_vecMins + collision->m_vecMaxs ) * 0.5f;

   float flBestDistance = FLT_MAX;
   C_CSPlayer* pBestPlayer = nullptr;

   for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; i++ ) {

	  C_CSPlayer* player = ( C_CSPlayer* ) ( Source::m_pEntList->GetClientEntity( i ) );

	  if ( !player || player->IsDead( ) || player->IsDormant( ) )
		 continue;

	  float Dist = pLocalPlayer->m_vecOrigin( ).Distance( player->m_vecOrigin( ) );

	  if ( Dist < flBestDistance ) {
		 flBestDistance = Dist;
		 pBestPlayer = player;
	  }
   }

   auto GetZeusRange = [&] ( C_CSPlayer* player ) -> float {
	  const float RangeModifier = 0.00490000006f, MaxDamage = 500.f;
	  return ( log( player->m_iHealth( ) / MaxDamage ) / log( RangeModifier ) ) / 0.002f;
   };

   float flRange = 0.f;
   if ( pBestPlayer ) {
	  flRange = GetZeusRange( pBestPlayer );
   }

   const int accuracy = 360;
   const float step = DirectX::XM_2PI / accuracy;
   for ( float a = 0.0f; a < DirectX::XM_2PI; a += step ) {
	  float a_c, a_s, as_c, as_s;
	  DirectX::XMScalarSinCos( &a_s, &a_c, a );
	  DirectX::XMScalarSinCos( &as_s, &as_c, a + step );

	  Vector startPos = Vector( a_c * flRange + eyePos.x, a_s * flRange + eyePos.y, eyePos.z );
	  Vector endPos = Vector( as_c * flRange + eyePos.x, as_s * flRange + eyePos.y, eyePos.z );

	  Ray_t ray;
	  CGameTrace tr;
	  CTraceFilter filter = CTraceFilter( );
	  filter.pSkip = pLocalPlayer;

	  ray.Init( eyePos, startPos );
	  Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &tr );

	  auto frac_1 = tr.fraction;
	  Vector2D start2d;
	  if ( !WorldToScreen( tr.endpos, start2d ) )
		 continue;

	  ray.Init( eyePos, endPos );
	  Source::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &tr );

	  Vector2D end2d;

	  if ( !WorldToScreen( tr.endpos, end2d ) )
		 continue;

	  auto frac_2 = tr.fraction;
	  auto clr = FloatColor( 1.f, 0.0f, 0.0f, 1.f ).Lerp( FloatColor( 1.f, 1.f, 1.f, 1.f ), ( frac_1 + frac_2 ) * 0.5f );
	  auto thickness = 1.0f + 4.0f * ( 1.0f - ( ( frac_1 + frac_2 ) * 0.5f ) );

	  Render::Get( )->AddLine( start2d, end2d, clr, thickness );
   }
}

void drawAngleLine( const Vector& origin, const Vector2D& w2sOrigin,
					const float& angle, const char* text, ColorU32 clr ) {
   Vector forward = QAngle( 0.0f, angle, 0.0f ).ToVectors( );
   float AngleLinesLength = 30.0f;

   Vector2D w2sReal;
   if ( WorldToScreen( origin + forward * AngleLinesLength, w2sReal ) ) {
	  Render::Get( )->AddLine( w2sOrigin, w2sReal, 0xFFFFFFFF );
	  Render::Get( )->AddRectFilled( { w2sReal.x - 5.0f, w2sReal.y - 5.0f }, { w2sReal.x + 5.0f, w2sReal.y + 5.0f }, 0xFFFFFFFF );

	  Render::Get( )->SetTextFont( FONT_VISITOR );
	  Render::Get( )->AddText( w2sReal, clr, OUTLINED | CENTER_X | CENTER_Y, text );
   }
};

class CFogContoler {
public:

};

int rgb_to_int( int red, int green, int blue ) {
   int r;
   int g;
   int b;

   r = red & 0xFF;
   g = green & 0xFF;
   b = blue & 0xFF;
   return ( r << 16 | g << 8 | b );
};

void CEsp::Main( ) {
   if ( !g_Vars.esp.esp_enable )
	  return;

   if ( g_Vars.esp.keybind_list )
	  Keybinds( );

   if ( g_Vars.esp.spectator_list )
	  SpectatorList( );

   if ( g_Vars.esp.stat_logger )
	  HitLogger( );

   if ( g_Vars.esp.indicators_list )
	  Indicators( );

   if ( g_Vars.esp.watermark )
	  DrawWatermark( );

   m_LocalPlayer = C_CSPlayer::GetLocalPlayer( );
   if ( !g_Vars.globals.HackIsReady || !m_LocalPlayer || !Source::m_pEngine->IsInGame( ) )
	  return;

   if ( g_Vars.esp.fog_effect ) {
	  for ( int i = 0; i < Source::m_pEntList->GetHighestEntityIndex( ); i++ ) {
		 C_BaseEntity* pEntity = ( C_BaseEntity* ) Source::m_pEntList->GetClientEntity( i );

		 if ( !pEntity )
			continue;

		 if ( pEntity->GetClientClass( )->m_ClassID == ClassId_t::CFogController ) {
			auto fog_controler = ( CFogContoler* ) pEntity;
			*( bool* ) ( uintptr_t( fog_controler ) + 0xA1C ) = true;
			*( bool* ) ( uintptr_t( fog_controler ) + 0xA1D ) = g_Vars.esp.fog_blind;
			*( float* ) ( uintptr_t( fog_controler ) + 0x9F8 ) = 0;
			*( float* ) ( uintptr_t( fog_controler ) + 0x9FC ) = g_Vars.esp.fog_distance;
			*( float* ) ( uintptr_t( fog_controler ) + 0xA04 ) = g_Vars.esp.fog_density / 100.f;
			*( float* ) ( uintptr_t( fog_controler ) + 0xA24 ) = g_Vars.esp.fog_hdr_scale / 100.f;
			*( int* ) ( uintptr_t( fog_controler ) + 0x9E8 ) = rgb_to_int( ( int ) ( g_Vars.esp.fog_color.b * 255.f ), ( int ) ( g_Vars.esp.fog_color.g * 255.f ), ( int ) ( g_Vars.esp.fog_color.r * 255.f ) );
			*( int* ) ( uintptr_t( fog_controler ) + 0x9EC ) = rgb_to_int( ( int ) ( g_Vars.esp.fog_color_secondary.b * 255.f ), ( int ) ( g_Vars.esp.fog_color_secondary.g * 255.f ), ( int ) ( g_Vars.esp.fog_color_secondary.r * 255.f ) );
		 }
	  }
   }

   OverlayInfo( );

   if ( g_Vars.misc.autopeek && g_Vars.misc.autopeek_visualise && !AutoPeekPos.IsZero( ) ) {
	  const float step = DirectX::XM_2PI / 24.0f;
	  auto radius = 24.0f;
	  Vector2D w2sCenter;
	  if ( WorldToScreen( AutoPeekPos, w2sCenter ) ) {
		 for ( float a = 0.f; a < DirectX::XM_2PI - step; a += step ) {
			float cos_start, sin_start;
			DirectX::XMScalarSinCos( &sin_start, &cos_start, a );

			Vector start( radius * cos_start + AutoPeekPos.x, radius * sin_start + AutoPeekPos.y, AutoPeekPos.z );

			Vector2D start2d, end2d;
			if ( !WorldToScreen( start, start2d ) )
			   break;

			float cos_end, sin_end;
			DirectX::XMScalarSinCos( &sin_end, &cos_end, a + step );
			Vector end( radius * cos_end + AutoPeekPos.x, radius * sin_end + AutoPeekPos.y, AutoPeekPos.z );
			if ( !WorldToScreen( end, end2d ) )
			   break;

			Render::Get( )->AddTriangleFilled( w2sCenter, start2d, end2d, g_Vars.misc.autopeek_color );
		 }
	  }
   }

#if 0
   auto weapon = ( C_WeaponCSBaseGun* ) local->m_hActiveWeapon( ).Get( );
   if ( weapon ) {
	  float spread = weapon->GetSpread( ) + weapon->GetInaccuracy( );

	  auto eye_pos = local->GetEyePosition( );

	  QAngle viewangles;
	  Source::m_pEngine->GetViewAngles( viewangles );

	  Vector right, up;
	  Vector forward = viewangles.ToVectors( &right, &up );

	  for ( int i = 1; i <= 5; ++i ) {
		 for ( int j = 0; j < i * 6; ++j ) {
			float flSpread = spread * float( float( i ) / 5.f );

			float flDirCos, flDirSin;
			DirectX::XMScalarSinCos( &flDirCos, &flDirSin, DirectX::XM_2PI * float( float( j ) / float( i * 6 ) ) );

			float spread_x = flDirCos * flSpread;
			float spread_y = flDirSin * flSpread;

			Vector direction;
			direction.x = forward.x + ( spread_x * right.x ) + ( spread_y * up.x );
			direction.y = forward.y + ( spread_x * right.y ) + ( spread_y * up.y );
			direction.z = forward.z + ( spread_x * right.z ) + ( spread_y * up.z );

			Ray_t ray;
			ray.Init( eye_pos, eye_pos + direction * 400.0f );

			CTraceFilterWorldOnly filt er;
			CGameTrace trace;
			Source::m_pEngineTrace->TraceRay( ray, MASK_PLAYERSOLID, &filter, &trace );

			Source::m_pDebugOverlay->AddBoxOverlay( trace.endpos, Vector( -1.f, -1.f, -1.f ), Vector( 1.0f, 1.0f, 1.0f ), QAngle( ), 255, 255, 255, 125,
													Source::m_pGlobalVars->frametime + Source::m_pGlobalVars->frametime / 2.0f );

			//Source::m_pDebugOverlay->AddLineOverlay( eye_pos + forward * 2.f, eye_pos + direction * 400.0f, 255, 255, 255, true, Source::m_pGlobalVars->frametime +
			//  Source::m_pGlobalVars->frametime / 4.0f );
		 }
	  }
   }
#endif

   DrawZeusDistance( );

   DrawAntiAimIndicator( );

   DrawTextIndicators( );

   if ( g_Vars.misc.lag_exploit && InputSys::Get( )->GetKeyState( g_Vars.misc.lag_exploit_key ) == KeyState::Down )
	  Render::Get( )->AddText( Vector2D( 5.0f, 450.0f ), 0xFFFFFFFF, OUTLINED, XorStr( "LagExploit Power %d\n" ), Source::m_pClientState->m_nChokedCommands( ) );

#if 0
   static bool debug_state = false;
   if ( InputSys::Get( )->WasKeyPressed( VirtualKeys::F ) )
	  debug_state = !debug_state;

   if ( debug_state ) {
	  for ( auto [idx, player] : g_ServerSounds.m_Players ) {
		 for ( auto sound : player.m_Sounds ) {
			if ( Source::m_pGlobalVars->tickcount - sound.tickcount > 250 )
			   continue;

			Vector2D w2s;
			if ( !WorldToScreen( sound.origin, w2s ) ) {
			   continue;
			}

			Render::Get( )->AddCircle( w2s, 8.0f, 0xffffffff, 32, 2.0f );
		 }

	  }
   }
#endif

#if 0
   /* Render::Get( )->AddText( Vector2D( 5.0f, 400.0f ), Color::White( ).GetD3DColor( ), OUTLINED,
							 XorStr( "angles delta %f" ), Math::AngleNormalize( std::fabsf( g_Vars.globals.flRealYaw - g_Vars.globals.m_FakeAngles.yaw ) ) );*/

   if ( m_LocalPlayer && !m_LocalPlayer->IsDead( ) ) {
	  Render::Get( )->SetTextFont( FONT_VERDANA );

   #if 1
	  static int m_iTicksAllowed = *( int* ) ( Engine::Displacement.Data.m_uTicksAllowed );

	  auto simTime = TIME_TO_TICKS( m_LocalPlayer->m_flSimulationTime( ) - m_LocalPlayer->m_flOldSimulationTime( ) );

	  //AutowallCalls %d\n IME_TO_TICKS( local->m_flNextAttack( ) ), TIME_TO_TICKS( weapon->m_flNextPrimaryAttack( )   TIME_TO_TICKS( Source::Movement::Get( )->GetLBYUpdateTime( ) 
	  // rebuild %f memory %f", eyePosRebuild.z, eyePos.z eyePosRebuild.z, eyePosMemory.z utowallCalls %d\n
	  auto weapon = ( C_WeaponCSBaseGun* ) ( m_LocalPlayer->m_hActiveWeapon( ).Get( ) );
	  auto curtime = TICKS_TO_TIME( m_LocalPlayer->m_nTickBase( ) );

	  auto ent = GetServerEdict( m_LocalPlayer->m_entIndex );
	  if ( ent )
		 Render::Get( )->AddText( Vector2D( 5.0f, 400.0f ), 0xFFFFFFFF, OUTLINED,
								  XorStr( "chokedcommands %d\nticks allowed %d\ndelta %d\ntickbase %d\n" ),
								  Source::m_pClientState->m_nChokedCommands( ), *( int* ) ( ent + m_iTicksAllowed ),
								  simTime, m_LocalPlayer->m_nTickBase( )
		 );
   #else
	  auto simTime = TIME_TO_TICKS( local->m_flSimulationTime( ) - local->m_flOldSimulationTime( ) );
	  Render::Get( )->AddText( Vector2D( 5.0f, 400.0f ), simTime < 0 ? Color::Green( ).GetD3DColor( ) : Color::White( ).GetD3DColor( ), OUTLINED,
							   XorStr( "tickbase delta %d" ), simTime );
   #endif
   }
#endif

   //DrawNetVars( local );

   auto absOrigin = m_LocalPlayer->GetAbsOrigin( );
   Vector2D w2sOrigin;
   if ( WorldToScreen( absOrigin, w2sOrigin ) && g_Vars.esp.vizualize_angles && !m_LocalPlayer->IsDead( ) ) {
	  drawAngleLine( absOrigin, w2sOrigin, g_Vars.globals.angViewangles.yaw, XorStr( "View Angles" ), FloatColor( 0.937f, 0.713f, 0.094f, 1.0f ) );
	  drawAngleLine( absOrigin, w2sOrigin, m_LocalPlayer->m_flLowerBodyYawTarget( ), XorStr( "Lower Body" ), FloatColor( 0.0f, 0.0f, 1.0f, 1.0f ) );
	  drawAngleLine( absOrigin, w2sOrigin, g_Vars.globals.flRealYaw, XorStr( "Real" ), FloatColor( 0.0f, 1.0f, 0.0f, 1.0f ) );
	  drawAngleLine( absOrigin, w2sOrigin, g_Vars.globals.m_FakeAngles.yaw, XorStr( "Fake" ), FloatColor( 1.0f, 0.0f, 0.0f, 1.0f ) );
   #if 0
	  drawAngleLine( absOrigin, w2sOrigin, PreviousYaw, XorStr( "PreviousYaw" ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

	  auto average = std::remainderf( ( g_Vars.globals.angViewangles.yaw + PreviousYaw ) * 0.5f, 360.0f );
	  drawAngleLine( absOrigin, w2sOrigin, average, XorStr( "average" ), FloatColor( 1.0f, 0.0f, 0.0f, 1.0f ) );

	  auto inversed = std::remainderf( average + 180.0f, 360.0f );
	  drawAngleLine( absOrigin, w2sOrigin, inversed, XorStr( "inversed" ), FloatColor( 0.0f, 0.0f, 1.0f, 1.0f ) );

	  auto delta = std::remainderf( inversed - g_Vars.globals.angViewangles.yaw, 360.0f );
	  auto yaw = 58.0f * ( 2 * ( delta >= 0.0f ) - 1 );

	  auto resolved = std::remainderf( yaw + g_Vars.globals.angViewangles.yaw, 360.0f );
	  drawAngleLine( absOrigin, w2sOrigin, resolved, XorStr( "resolved" ), FloatColor( 1.0f, 0.0f, 1.0f, 1.0f ) );
   #endif

   #if 0
	  Vector ang;
	  if ( AutoDirection( &ang ) )
		 drawAngleLine( absOrigin, w2sOrigin, ang.y, XorStr( "egde" ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   #endif

   #if 0
	  drawAngleLine( absOrigin, w2sOrigin, m_LocalPlayer->m_PlayerAnimState( )->m_flAbsRotation, XorStr( "Current Real" ), FloatColor( 0.0f, 1.0f, 0.0f, 1.0f ) );
	  drawAngleLine( absOrigin, w2sOrigin, m_LocalPlayer->m_PlayerAnimState( )->m_flEyeYaw, XorStr( "Current View" ), FloatColor( 1.0f, 0.0f, 0.0f, 1.0f ) );
   #endif

   #if 0
	  extern float fl_side;
	  extern float fl_forward;
	  extern QAngle q_viewAngles;

	  if ( fl_forward != 0.0f || fl_side != 0.0f ) {
		 Vector right;
		 auto forward = q_viewAngles.ToVectors( &right );

		 Vector wishvel;
		 for ( int i = 0; i < 2; i++ )       // Determine x and y parts of velocity
			wishvel[ i ] = forward[ i ] * fl_forward + right[ i ] * fl_side;

		 QAngle wishdir = wishvel.ToEulerAngles( );
		 Vector velocity = local->m_vecVelocity( );

		 float direction = RAD2DEG( atan2( velocity.y, velocity.x ) );
		 drawAngleLine( local->m_vecOrigin( ), w2sOrigin, wishdir.yaw, XorStr( "wishidr" ), FloatColor( 0.0f, 1.0f, 0.0f, 1.0f ) );
		 drawAngleLine( local->m_vecOrigin( ), w2sOrigin, direction, XorStr( "velocity" ), FloatColor( 1.0f, 0.0f, 0.0f, 1.0f ) );
	  }
   #endif
   }

   IHitmarker::Get( )->Paint( );

   if ( g_Vars.esp.sound_esp )
	  ISoundEsp::Get( )->Main( );

   Vector2D points[ 8 ];
   Vector2D center;

#ifdef MT_TEST
   struct traced_rays {
	  CGameTrace trace;
	  Ray_t ray;
	  CTraceFilter filter;
	  Vector2D w2s;
	  bool success_w2s;
   };

   static traced_rays rays[ 720 ] = { };

   static auto mt_traceray_lambda = [] ( void* _data ) {
	  traced_rays* data = ( traced_rays* ) _data;

	  Source::m_pEngineTrace->TraceRay( data->ray, MASK_SOLID, &data->filter, &data->trace );
	  data->success_w2s = WorldToScreen( data->trace.endpos, data->w2s );
	  };

   Vector eye_pos = local->GetEyePosition( );

   for ( int i = 0; i < 720; i++ ) {
	  Vector end = Vector( cos( DEG2RAD( float( i ) * 0.5f ) ) * 550.0f + eye_pos.x,
						   sin( DEG2RAD( float( i ) * 0.5f ) ) * 550.0f + eye_pos.y,
						   eye_pos.z
	  );

	  rays[ i ].filter = CTraceFilter( );
	  rays[ i ].filter.pSkip = local;
	  rays[ i ].ray.Init( eye_pos, end );
	  Threading::QueueJobRef( mt_traceray_lambda, &rays[ i ] );
   }

   Threading::FinishQueue( );

   Vector2D w2s_eye;
   if ( WorldToScreen( eye_pos, w2s_eye ) ) {
	  for ( int i = 0; i < 720; i++ ) {
		 if ( rays[ i ].success_w2s )
			Render::Get( )->AddLine( w2s_eye, rays[ i ].w2s, 0xFFFFFFFF, 1.5f );
	  }
   }
#endif

   QAngle angles;
   Source::m_pEngine->GetViewAngles( angles );

   auto local_eye_pos = m_LocalPlayer->GetEyePosition( );
   auto end = local_eye_pos + angles.ToVectors( ).Normalized( ) * 4096.0f;

   int local_idx = m_LocalPlayer->m_entIndex;

   //if ( g_Vars.esp.extended_esp )
	//  IExtendedEsp::Get( )->Start( );

   for ( int i = 1; i <= Source::m_pEntList->GetHighestEntityIndex( ); ++i ) {
	  auto entity = ( C_BaseEntity* ) Source::m_pEntList->GetClientEntity( i );

	  if ( !entity )
		 continue;

	  auto player = C_CSPlayer::GetPlayerByIndex( i );

	  if ( i < 65 && ValidPlayer( player ) ) {
		 if ( Begin( player ) ) {

			if ( g_Vars.esp.skeleton )
			   DrawSkeleton( player );

			if ( g_Vars.esp.aim_points )
			   DrawAimPoints( player );

			if ( g_Vars.esp.box )
			   DrawBox( m_Data.bbox, g_Vars.esp.box_color, player );

			if ( g_Vars.esp.health )
			   DrawHealthBar( player, m_Data.bbox );

			DrawInfo( player, m_Data.bbox, m_Data.info );

			if ( g_Vars.esp.name )
			   DrawName( player, m_Data.bbox, m_Data.info );

			if ( g_Vars.esp.draw_ammo_bar )
			   AmmoBar( player, m_Data.bbox );

			DrawBottomInfo( player, m_Data.bbox, m_Data.info );

			if ( g_Vars.esp.snaplines_enalbed )
			   RenderSnapline( Vector2D( ( m_Data.bbox.min.x + m_Data.bbox.max.x ) * 0.5f, m_Data.bbox.max.y ) );

		 #if 0
			auto lagData = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
			if ( lagData.IsValid( ) && !lagData->m_History.empty( ) ) {
			   auto record = lagData->m_History.front( );
			   for ( int x = -1; x <= 1; ++x ) {
				  auto boneMatrix = record.GetBoneMatrix( x );
				  auto hdr = Source::m_pModelInfo->GetStudiomodel( player->GetModel( ) );
				  auto hitboxSet = hdr->pHitboxSet( player->m_nHitboxSet( ) );
				  for ( int i = 0; i < hitboxSet->numhitboxes; ++i ) {
					 auto hitbox = hitboxSet->pHitbox( i );
					 if ( hitbox->m_flRadius <= 0.f )
						continue;

					 auto min = hitbox->bbmin.Transform( boneMatrix[ hitbox->bone ] );
					 auto max = hitbox->bbmax.Transform( boneMatrix[ hitbox->bone ] );

					 float frametime = std::fmaxf( Source::m_pGlobalVars->frametime, Source::m_pGlobalVars->absoluteframetime ) * 2.0f +
						Source::m_pGlobalVars->absoluteframestarttimestddev * Source::m_pGlobalVars->absoluteframestarttimestddev;

					 if ( x == -1 ) {
						Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius, 255, 0, 0, 255,
																	frametime );

					 } else if ( !x ) {
						Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius, 0, 255, 0, 255,
																	frametime );
					 } else {
						Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius, 0, 0, 255, 255,
																	frametime );
					 }
				  }
			   }
			}
		 #endif

		 #if 0
			auto hdr = Source::m_pModelInfo->GetStudiomodel( player->GetModel( ) );
			auto hitboxSet = hdr->pHitboxSet( player->m_nHitboxSet( ) );

			QAngle view;
			Source::m_pEngine->GetViewAngles( view );
			auto direction = view.ToVectors( ); // direction
			direction.Normalize( );

			// hitboxSet->numhitboxes
			for ( int i = 0; i < hitboxSet->numhitboxes; ++i ) {
			   float frametime = std::fmaxf( Source::m_pGlobalVars->frametime, Source::m_pGlobalVars->absoluteframetime ) * 2.0f +
				  Source::m_pGlobalVars->absoluteframestarttimestddev * Source::m_pGlobalVars->absoluteframestarttimestddev;

			   auto hitbox = hitboxSet->pHitbox( i );
			   auto min = hitbox->bbmin.Transform( player->m_CachedBoneData( ).Element( hitbox->bone ) );
			   auto max = hitbox->bbmax.Transform( player->m_CachedBoneData( ).Element( hitbox->bone ) );

			#if 0
			   if ( hitbox->m_flRadius <= 0.f ) {
				  bool hit = Math::IntersectionBoundingBox( local_eye_pos, end, min, max );
				  Source::m_pDebugOverlay->AddBoxOverlay( ( min + max ) * 0.5f, hitbox->bbmin, hitbox->bbmax, QAngle( ), hit ? 0 : 255, 255, hit ? 0 : 255, 255,
														  frametime );
				  continue;
			   }
			#endif

			   RayTracer::Trace tr;
			   tr.m_hit = false;

			   RayTracer::Ray ray = RayTracer::Ray( direction );
			   ray.m_startPoint = local->GetEyePosition( );
			   ray.m_length = 4096.0f;

			   RayTracer::Hitbox capsule( min, max, hitbox->m_flRadius );

			   RayTracer::TraceHitbox( ray, capsule, tr );
			   Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius,
														   tr.m_hit ? 0 : 255,
														   255,
														   tr.m_hit ? 0 : 255,
														   255,
														   frametime );
			   continue;
			}
		 #endif

		 #if 0
			auto hdr = Source::m_pModelInfo->GetStudiomodel( player->GetModel( ) );
			auto hitboxSet = hdr->pHitboxSet( player->m_nHitboxSet( ) );

			// hitboxSet->numhitboxes
			for ( int i = 0; i < 1; ++i ) {
			   float frametime = std::fmaxf( Source::m_pGlobalVars->frametime, Source::m_pGlobalVars->absoluteframetime ) * 2.0f +
				  Source::m_pGlobalVars->absoluteframestarttimestddev * Source::m_pGlobalVars->absoluteframestarttimestddev;

			   auto hitbox = hitboxSet->pHitbox( i );
			   auto min = hitbox->bbmin.Transform( player->m_CachedBoneData( ).Element( hitbox->bone ) );
			   auto max = hitbox->bbmax.Transform( player->m_CachedBoneData( ).Element( hitbox->bone ) );

			   if ( hitbox->m_flRadius <= 0.f ) {
				  bool hit = Math::IntersectionBoundingBox( local_eye_pos, end, min, max );
				  Source::m_pDebugOverlay->AddBoxOverlay( ( min + max ) * 0.5f, hitbox->bbmin, hitbox->bbmax, hitbox->m_angAngles, hit ? 0 : 255, 255, hit ? 0 : 255, 255,
														  frametime );
				  continue;
			   }

			   auto center = ( min + max ) * 0.5f;

			   RayTracer::Hitbox box( min, max, hitbox->m_flRadius );
			   RayTracer::Ray ray( local_eye_pos, center );
			   RayTracer::Trace trace;
			   RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );

			   Vector points[ 7 ];
			   points[ 0 ] = trace.m_traceEnd;

			   auto delta = center - local_eye_pos;
			   delta.Normalized( );

			   Vector right, up;
			   delta.GetVectors( right, up );

			   Vector middle = ( up + right ) * 0.866f;
			   Vector middle2 = ( up - right ) * 0.866f;

			   ray = RayTracer::Ray( local_eye_pos, center - ( up * 1000.0f ) );
			   RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
			   points[ 1 ] = trace.m_traceEnd;

			   ray = RayTracer::Ray( local_eye_pos, center + ( up * 1000.0f ) );
			   RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
			   points[ 2 ] = trace.m_traceEnd;

			   ray = RayTracer::Ray( local_eye_pos, center + ( middle * 1000.0f ) );
			   RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
			   points[ 3 ] = trace.m_traceEnd;

			   ray = RayTracer::Ray( local_eye_pos, center - ( middle * 1000.0f ) );
			   RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
			   points[ 4 ] = trace.m_traceEnd;

			   ray = RayTracer::Ray( local_eye_pos, center - ( middle2 * 1000.0f ) );
			   RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
			   points[ 5 ] = trace.m_traceEnd;

			   ray = RayTracer::Ray( local_eye_pos, center + ( middle2 * 1000.0f ) );
			   RayTracer::TraceFromCenter( ray, box, trace, RayTracer::Flags_RETURNEND );
			   points[ 6 ] = trace.m_traceEnd;

			   for ( int x = 0; x < 7; ++x ) {
				  Vector2D w2s;
				  if ( !WorldToScreen( points[ x ], w2s ) ) {
					 continue;
				  }

				  auto color = g_Vars.esp.aim_points_color;
				  color.a = ( m_flAplha[ player->entindex( ) ] / 255.f );

				  Render::Get( )->SetTextFont( 0 );
				  Render::Get( )->AddText( w2s, color, OUTLINED | CENTER_X | CENTER_Y, XorStr( "%d" ), x );
			   }

			   if ( InputSys::Get( )->IsKeyDown( VirtualKeys::F ) ) {
				  //Source::m_pDebugOverlay->AddBoxOverlay( trace.m_traceEnd, Vector( -2.0f, -2.0f, -2.0f ), Vector( 1.0f, 1.0f, 1.0f ), QAngle( ), 255, 0, 0, 255, 1.0f );
			   }

			#if 0
			   RayTracer::TraceHitbox( ray, box, trace );
			   if ( !trace.m_hit ) {
				  Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius, 255, 255, 255, 255,
															  frametime );
			   } else {
				  Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius, 0, 255, 0, 255,
															  frametime );
			   }
			#endif

			#if 0
			   Math::CapsuleCollider collide;
			   collide.min = min;
			   collide.max = max;
			   collide.radius = hitbox->m_flRadius;

			   bool hit = collide.Intersect( local_eye_pos, end );
			   if ( !hit ) {
				  Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius, 255, 255, 255, 255,
															  frametime );
			   } else {
				  Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius, 0, 255, 0, 255,
															  frametime );
			   }
			#endif

			}
		 #endif

		 #if 0
			// debug at target
			auto lagdata = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
			if ( lagdata.IsValid( ) && lagdata->m_History.size( ) > 0 ) {
			   auto record = &lagdata->m_History.at( 0 );

			   Vector2D w2sOrigin;
			   if ( WorldToScreen( player->m_vecOrigin( ), w2sOrigin ) ) {
				  auto at_target = ToDegrees( atan2( record->m_vecOrigin.y - m_LocalPlayer->m_vecOrigin( ).y,
											  record->m_vecOrigin.x - m_LocalPlayer->m_vecOrigin( ).x ) );

				  auto delta = std::remainderf( at_target - record->m_flEyeYaw, 360.0f );
				  at_target += ( delta <= 0.0f ) ? -10.0f : 10.0f;

				  delta = std::remainderf( at_target - record->m_flEyeYaw, 360.0f );

				  auto min_dsc_delta = -58.0f;
				  auto max_dsc_delta = 58.0f;
				  delta = Math::Clamp( delta, min_dsc_delta, max_dsc_delta );

				  auto flGoalFeetYaw1 = std::remainderf( record->m_flEyeYaw + delta, 360.0f );
				  drawAngleLine( player->m_vecOrigin( ), w2sOrigin, at_target, XorStr( "at_target" ), FloatColor( 0.937f, 0.713f, 0.094f, 1.0f ) );
				  drawAngleLine( player->m_vecOrigin( ), w2sOrigin, flGoalFeetYaw1, XorStr( "resolved yaw" ), FloatColor( 0.1f, 0.8f, 0.1f, 1.0f ) );

				  Render::Get( )->AddText( Vector2D( 500.0f, 100.0f ), 0xFFFFFFFF, OUTLINED, XorStr( "%.2f delta\n%d side" ), delta, 2 * ( delta >= 0.0f ) - 1 );
			   }

			}
		 #endif

		 #if 0
			auto lagdata = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
			if ( lagdata.IsValid( ) && lagdata->m_History.size( ) > 0 ) {
			   bool result = lagdata->DetectAutoDirerction( lagdata, player );

			   Vector2D w2sOrigin;
			   if ( lagdata->m_flDirection != FLT_MAX && WorldToScreen( player->m_vecOrigin( ), w2sOrigin ) ) {
				  float dir = RAD2DEG( atan2( lagdata->m_flEdges[ 3 ] - lagdata->m_flEdges[ 1 ], lagdata->m_flEdges[ 2 ] - lagdata->m_flEdges[ 0 ] ) );

				  drawAngleLine( player->m_vecOrigin( ), w2sOrigin, player->m_angEyeAngles( ).yaw, XorStr( "Viewangles" ), FloatColor( 0.937f, 0.713f, 0.094f, 1.0f ) );
				  drawAngleLine( player->m_vecOrigin( ), w2sOrigin, lagdata->m_flDirection, XorStr( "flDirection" ), FloatColor( 0.0f, 0.0f, 1.0f, 1.0f ) );
				  drawAngleLine( player->m_vecOrigin( ), w2sOrigin, dir, XorStr( "dir" ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

				  dir += lagdata->m_flDirection;
				  dir = std::remainderf( dir, 360.0f );

				  drawAngleLine( player->m_vecOrigin( ), w2sOrigin, dir, XorStr( "dir at target" ), FloatColor( 1.0f, 0.f, 1.0f, 1.0f ) );

				  float delta = std::remainderf( player->m_angEyeAngles( ).yaw - dir, 360.0f );
				  Render::Get( )->AddText( Vector2D( 500.0f, 100.0f ), 0xFFFFFFFF, OUTLINED, XorStr( "%.2f delta\n%d side" ), delta, 2 * ( delta <= 0.0f ) - 1 );
				  if ( !result )
					 Render::Get( )->AddText( Vector2D( 500.0f, 125.0f ), FloatColor( 0.0f, 1.0f, 0.0f ), OUTLINED, XorStr( "HISTORY!" ) );
		 }

	  }
		 #endif   

		 #if 0
			auto lagdata = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
			if ( lagdata.IsValid( ) && lagdata->m_History.size( ) > 0 ) {
			   Vector2D w2sOrigin;
			   if ( WorldToScreen( player->m_vecOrigin( ), w2sOrigin ) ) {
				  float dir1 = lagdata->m_Animations[ 0 ].m_animLayers[ 6 ].m_flPlaybackRate * 36000.0f;
				  float dir2 = lagdata->m_Animations[ 1 ].m_animLayers[ 6 ].m_flPlaybackRate * 36000.0f;
				  float dir3 = lagdata->m_Animations[ 2 ].m_animLayers[ 6 ].m_flPlaybackRate * 36000.0f;
				  float dir4 = lagdata->m_serverAnimLayers[ 6 ].m_flPlaybackRate * 36000.0f;

				  drawAngleLine( player->m_vecOrigin( ), w2sOrigin, dir1, XorStr( "0" ), FloatColor( 0.0f, 1.0f, 0.0f, 1.0f ) );
				  drawAngleLine( player->m_vecOrigin( ), w2sOrigin, dir2, XorStr( "1" ), FloatColor( 1.0f, 0.0f, 1.0f, 1.0f ) );
				  drawAngleLine( player->m_vecOrigin( ), w2sOrigin, dir3, XorStr( "2" ), FloatColor( 0.0f, 1.0f, 1.0f, 1.0f ) );
				  drawAngleLine( player->m_vecOrigin( ), w2sOrigin, dir4, XorStr( "server" ), FloatColor( 0.0f, 0.0f, 1.0f, 1.0f ) );
			   }
			}
		 #endif
   }
   }



	  if ( !entity->GetClientClass( ) || !entity->GetClientClass( )->m_ClassID )
		 continue;

	  if ( !_strcmpi( entity->GetClientClass( )->m_pNetworkName, ( XorStr( "CPlantedC4" ) ) ) ) {
		 auto bomb_entity = ( C_PlantedC4* ) entity;
		 static ConVar* mp_c4timer = Source::m_pCvar->FindVar( XorStr( "mp_c4timer" ) );

		 if ( g_Vars.esp.draw_c4 ) {
			float timer_bomb = 0.f;
			if ( bomb_entity->m_flC4Blow( ) - Source::m_pGlobalVars->curtime > 0.f )
			   timer_bomb = bomb_entity->m_flC4Blow( ) - Source::m_pGlobalVars->curtime;
			else
			   timer_bomb = 0.f;

			Vector2D center = Render::Get( )->GetScreenSize( );

			float ttt = center.y / mp_c4timer->GetInt( );
			float temp = ttt * timer_bomb;
			float height = center.y - temp;
			int temp_green = int( 6.f * timer_bomb );
			int temp_red = int( 255.0f - temp_green );

			auto CalculateDamage = [&] ( float Damage, int ArmorValue ) -> float {
			   float flArmorRatio = 0.5f;
			   float flArmorBonus = 0.5f;
			   if ( ArmorValue > 0 ) {
				  float flNew = Damage * flArmorRatio;
				  float flArmor = ( Damage - flNew ) * flArmorBonus;

				  if ( flArmor > static_cast< float >( ArmorValue ) ) {
					 flArmor = static_cast< float >( ArmorValue )* ( 1.f / flArmorBonus );
					 flNew = Damage - flArmor;
				  }

				  Damage = flNew;
			   }
			   return Damage;
			};

			float flDistance = m_LocalPlayer->GetAbsOrigin( ).Distance( entity->GetAbsOrigin( ) );
			float a = 450.7f;
			float b = 75.68f;
			float c = 789.2f;
			float d = ( ( flDistance - b ) / c );
			float Damage = a * exp( -d * d );
			int damage = std::max( ( int ) ceilf( CalculateDamage( Damage, m_LocalPlayer->m_ArmorValue( ) ) ), 0 );

			if ( timer_bomb > 0 ) {
			   Render::Get( )->SetTextFont( FONT_VERDANA_40_BOLD );
			   if ( damage >= m_LocalPlayer->m_iHealth( ) )
				  Render::Get( )->AddText( Vector2D( 55, 55 ), FloatColor( 244, 67, 54, 255 ), OUTLINED | CENTER_X | ALIGN_BOTTOM, XorStr( "FATAL" ) );
			   else
				  Render::Get( )->AddText( Vector2D( 41, 41 ), FloatColor( 124, 179, 66, 255 ), OUTLINED | CENTER_X | ALIGN_BOTTOM, std::to_string( damage - 1 ).c_str( ) );

			   Render::Get( )->AddRectFilled( Vector2D( 0, 0 ), Vector2D( 18, center.y ), Color( 66, 66, 66, 155 ).GetD3DColor( ) );
			   Render::Get( )->SetTextFont( FONT_VISITOR );
			   Render::Get( )->AddText( Vector2D( 3, height - 16 ), FloatColor( 255, 255, 255, 255 ), 0, XorStr( "%i" ), ( int ) timer_bomb );
			   Render::Get( )->AddRectFilled( Vector2D( 0, height ), Vector2D( 18, center.y ), Color( 0, temp_green, temp_red, 155 ).GetD3DColor( ) );
			}
		 }

	  }

	  Rect2D bbox;
	  if ( !GetBBox( entity, points, bbox ) )
		 continue;

	  RenderNades( ( C_WeaponCSBaseGun* ) entity );

	  if ( g_Vars.esp.dropped_weapons && entity->IsWeapon( ) )
		 RenderDroppedWeapon( ( C_WeaponCSBaseGun* ) entity );

	  if ( g_Vars.esp.molotov_indicator ) {
		 if ( entity->GetClientClass( )->m_ClassID == CInferno ) {
			C_Inferno* pInferno = reinterpret_cast< C_Inferno* >( entity );
			C_CSPlayer* player = ( C_CSPlayer* ) entity->m_hOwnerEntity( ).Get( );

			if ( !player )
			   return;

			FloatColor color = player->m_iTeamNum( ) == m_LocalPlayer->m_iTeamNum( ) ? FloatColor( 35, 255, 0, 255 ) : FloatColor( 255, 35, 0, 255 );

			const Vector origin = pInferno->GetAbsOrigin( );
			Vector2D screen_origin = Vector2D( );

			if ( !WorldToScreen( origin, screen_origin ) )
			   return;

			const auto spawn_time = pInferno->m_flSpawnTime( );
			const auto time = ( ( spawn_time + C_Inferno::GetExpiryTime( ) ) - Source::m_pGlobalVars->curtime );
			const auto factor = time / C_Inferno::GetExpiryTime( );

			static const auto size = Vector2D( 70.f, 6.f );

			auto new_pos = Vector2D( screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5 );

			Vector min, max;
			entity->GetClientRenderable( )->GetRenderBounds( min, max );

			auto radius = ( max - min ).Length2D( ) * 0.5f;
			Vector boundOrigin = Vector( ( min.x + max.x ) * 0.5f, ( min.y + max.y ) * 0.5f, min.z + 5 ) + origin;
			const int accuracy = 360;
			const float step = DirectX::XM_2PI / accuracy;
			for ( float a = 0.0f; a < DirectX::XM_2PI; a += step ) {
			   float a_c, a_s, as_c, as_s;
			   DirectX::XMScalarSinCos( &a_s, &a_c, a );
			   DirectX::XMScalarSinCos( &as_s, &as_c, a + step );

			   Vector startPos = Vector( a_c * radius + boundOrigin.x, a_s * radius + boundOrigin.y, boundOrigin.z );
			   Vector endPos = Vector( as_c * radius + boundOrigin.x, as_s * radius + boundOrigin.y, boundOrigin.z );

			   auto clr = FloatColor( 1.f, 1.0f, 1.0f, 1.f );

			   Vector2D start2d;
			   if ( !WorldToScreen( startPos, start2d ) )
				  continue;

			   Vector2D end2d;
			   if ( !WorldToScreen( endPos, end2d ) )
				  continue;

			   Render::Get( )->AddLine( start2d, end2d, clr, 1.f );
			}

			Render::Get( )->SetTextFont( FONT_VISITOR );
			Vector2D text_size = Render::Get( )->CalcTextSize( XorStr( "MOLLY" ) );
			Render::Get( )->AddText( Vector2D( new_pos.x + ( size.x * 0.5f ), new_pos.y - 2 ), FloatColor( 255, 255, 255, 255 ), OUTLINED | CENTER_X | ALIGN_BOTTOM, XorStr( "MOLLY" ) );
			Render::Get( )->AddRect( new_pos, new_pos + size, 10, 10, 10 );
			Render::Get( )->AddRectFilled( new_pos, new_pos + size, FloatColor( 45, 45, 45, 150 ) );
			Render::Get( )->AddRectFilled( new_pos + 2, new_pos + Vector2D( ( size.x - 2 ) * factor, size.y - 2 ), color );
			Render::Get( )->AddText( Vector2D( new_pos.x + ( size.x * 0.5f ), new_pos.y + 16 ), FloatColor( 255, 255, 255, 255 ), OUTLINED | CENTER_X | ALIGN_BOTTOM, XorStr( "%0.2f" ), time );
			Render::Get( )->SetTextFont( FONT_CSGO_ICONS2 );
			std::string name = ItemDefinitionIndexToIcon( ( ( C_CSPlayer* ) ( ( C_WeaponCSBaseGun* ) entity->m_hOwnerEntity( ).Get( ) ) )->m_iTeamNum( ) == TEAM_CT ? WEAPON_INC : WEAPON_MOLOTOV );
			Render::Get( )->AddText( Vector2D( new_pos.x + ( size.x * 0.5f ) + text_size.x - 4, new_pos.y - 1 ), FloatColor( 255, 255, 255, 255 ), OUTLINED | CENTER_X | ALIGN_BOTTOM, name.c_str( ) );
		 }
	  }

	  if ( g_Vars.esp.smoke_indicator ) {
		 C_SmokeGrenadeProjectile* pSmokeEffect = reinterpret_cast< C_SmokeGrenadeProjectile* >( entity );
		 if ( pSmokeEffect->GetClientClass( )->m_ClassID == CSmokeGrenadeProjectile ) {
			const Vector origin = pSmokeEffect->GetAbsOrigin( );
			Vector2D screen_origin = Vector2D( );

			if ( !WorldToScreen( origin, screen_origin ) )
			   return;

			const auto spawn_time = TICKS_TO_TIME( pSmokeEffect->m_nSmokeEffectTickBegin( ) );
			const auto time = ( spawn_time + C_SmokeGrenadeProjectile::GetExpiryTime( ) ) - Source::m_pGlobalVars->curtime;
			const auto factor = ( ( spawn_time + C_SmokeGrenadeProjectile::GetExpiryTime( ) ) - Source::m_pGlobalVars->curtime ) / C_SmokeGrenadeProjectile::GetExpiryTime( );

			static const auto size = Vector2D( 70.f, 6.f );

			auto new_pos = Vector2D( screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5 );
			if ( factor > 0.f ) {
			   auto radius = 120.f;

			   const int accuracy = 360;
			   const float step = DirectX::XM_2PI / accuracy;
			   for ( float a = 0.0f; a < DirectX::XM_2PI; a += step ) {
				  float a_c, a_s, as_c, as_s;
				  DirectX::XMScalarSinCos( &a_s, &a_c, a );
				  DirectX::XMScalarSinCos( &as_s, &as_c, a + step );

				  Vector startPos = Vector( a_c * radius + origin.x, a_s * radius + origin.y, origin.z + 5 );
				  Vector endPos = Vector( as_c * radius + origin.x, as_s * radius + origin.y, origin.z + 5 );

				  auto clr = FloatColor( 1.f, 1.0f, 1.0f, 1.f );

				  Vector2D start2d;
				  if ( !WorldToScreen( startPos, start2d ) )
					 continue;

				  Vector2D end2d;
				  if ( !WorldToScreen( endPos, end2d ) )
					 continue;

				  Render::Get( )->AddLine( start2d, end2d, clr, 1.f );
			   }

			   Render::Get( )->SetTextFont( FONT_VISITOR );
			   Vector2D text_size = Render::Get( )->CalcTextSize( XorStr( "SMOKE" ) );
			   Render::Get( )->AddText( Vector2D( new_pos.x + ( size.x * 0.5f ), new_pos.y - 2 ), FloatColor( 255, 255, 255, 255 ), OUTLINED | CENTER_X | ALIGN_BOTTOM, XorStr( "SMOKE" ) );
			   Render::Get( )->AddRect( new_pos, new_pos + size, 10, 10, 10 );
			   Render::Get( )->AddRectFilled( new_pos, new_pos + size, FloatColor( 45, 45, 45, 150 ) );
			   Render::Get( )->AddRectFilled( new_pos + 2, new_pos + Vector2D( ( size.x - 2 ) * factor, size.y - 2 ), FloatColor( 158, 158, 158, 255 ) );
			   Render::Get( )->AddText( Vector2D( new_pos.x + ( size.x * 0.5f ), new_pos.y + 16 ), FloatColor( 255, 255, 255, 255 ), OUTLINED | CENTER_X | ALIGN_BOTTOM, XorStr( "%0.2f" ), time );
			   Render::Get( )->SetTextFont( FONT_CSGO_ICONS2 );
			   std::string name = ItemDefinitionIndexToIcon( WEAPON_SMOKEGRENADE );
			   Render::Get( )->AddText( Vector2D( new_pos.x + ( size.x * 0.5f ) + text_size.x - 4, new_pos.y - 1 ), FloatColor( 255, 255, 255, 255 ), OUTLINED | CENTER_X | ALIGN_BOTTOM, name.c_str( ) );

			}
		 }
	  }

	  if ( !_strcmpi( entity->GetClientClass( )->m_pNetworkName, ( XorStr( "CPlantedC4" ) ) ) ) {
		 auto bomb_entity = ( C_PlantedC4* ) entity;

		 static ConVar* mp_c4timer = Source::m_pCvar->FindVar( XorStr( "mp_c4timer" ) );
		 static bool bomb_planted_tick_begin = false;
		 static float bomb_time_tick_begin = 0.f;

		 if ( g_Vars.esp.draw_c4_bar ) {
			Vector origin = bomb_entity->GetAbsOrigin( );
			Vector2D screen_origin = Vector2D( );

			if ( !WorldToScreen( origin, screen_origin ) )
			   return;

			if ( !bomb_planted_tick_begin ) {
			   bomb_time_tick_begin = Source::m_pGlobalVars->curtime;
			   bomb_planted_tick_begin = true;
			}

			float timer_bomb = 0.f;
			if ( bomb_entity->m_flC4Blow( ) - Source::m_pGlobalVars->curtime > 0.f )
			   timer_bomb = bomb_entity->m_flC4Blow( ) - Source::m_pGlobalVars->curtime;
			else
			   timer_bomb = 0.f;

			const auto spawn_time = TIME_TO_TICKS( bomb_time_tick_begin );
			const auto factor = timer_bomb / mp_c4timer->GetFloat( ); //( ( spawn_time + mp_c4timer->GetFloat( ) ) - Source::m_pGlobalVars->curtime ) / mp_c4timer->GetFloat( );
			static const auto size = Vector2D( 70.f, 6.f );

			auto new_pos = Vector2D( screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5 );

			if ( factor > 0.f ) {
			   Render::Get( )->SetTextFont( FONT_VISITOR );
			   Render::Get( )->AddText( Vector2D( new_pos.x + ( size.x * 0.5f ), new_pos.y - 2 ), FloatColor( 255, 255, 255, 255 ), OUTLINED | CENTER_X | ALIGN_BOTTOM, XorStr( "BOMB" ) );
			   Render::Get( )->AddRect( new_pos, new_pos + size, 10, 10, 10 );
			   Render::Get( )->AddRectFilled( new_pos, new_pos + size, FloatColor( 45, 45, 45, 150 ) );
			   Render::Get( )->AddRectFilled( new_pos + 2, new_pos + Vector2D( ( size.x - 2 ) * factor, size.y - 2 ), FloatColor( 158, 255, 158, 255 ) );
			   Render::Get( )->AddText( Vector2D( new_pos.x + ( size.x * 0.5f ), new_pos.y + 16 ), FloatColor( 255, 255, 255, 255 ), OUTLINED | CENTER_X | ALIGN_BOTTOM, XorStr( "%0.2f" ), timer_bomb );
			}
		 }
	  }
   }

   //if ( g_Vars.esp.extended_esp )
	 // IExtendedEsp::Get( )->Finish( );

#if 0
   extern Engine::C_LagRecord extrapolated;
   extern C_CSPlayer* extrapolatedPlayer;
   if ( extrapolated.m_bExtrapolated && extrapolatedPlayer ) {
	  auto model = extrapolatedPlayer->GetModel( );
	  if ( !model )
		 return;

	  auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
	  if ( !hdr )
		 return;

	  auto set = hdr->pHitboxSet( extrapolatedPlayer->m_nHitboxSet( ) );

	  std::pair< Vector2D, Vector2D > positions[ 32 ];
	  int point_count = 0;

	  auto chest = set->pHitbox( HITBOX_UPPER_CHEST );
	  auto neck = set->pHitbox( HITBOX_NECK );

	  auto middle = extrapolated.m_BoneMatrix[ chest->bone ].at( 3 ) +
		 extrapolated.m_BoneMatrix[ neck->bone ].at( 3 );
	  middle *= 0.5f;

	  auto upperarm_left = set->pHitbox( HITBOX_LEFT_UPPER_ARM );
	  auto upperarm_right = set->pHitbox( HITBOX_RIGHT_UPPER_ARM );

	  for ( int i = 0; i < hdr->numbones; ++i ) {
		 auto bone = hdr->pBone( i );
		 if ( ( bone->flags & BONE_USED_BY_ANYTHING ) == 0 || bone->parent < 0 )
			continue;

		 if ( bone->parent == chest->bone && i != neck->bone )
			continue;

		 Vector parentBone;
		 if ( i == upperarm_left->bone || i == upperarm_right->bone ) {
			parentBone = middle;
		 } else {
			parentBone = extrapolated.m_BoneMatrix[ bone->parent ].at( 3 );
		 }

		 Vector2D parentPos;
		 if ( !WorldToScreen( parentBone, parentPos ) )
			continue;

		 Vector2D childPos;
		 if ( !WorldToScreen( extrapolated.m_BoneMatrix[ i ].at( 3 ), childPos ) )
			continue;

		 positions[ point_count ].first = parentPos;
		 positions[ point_count ].second = childPos;
		 point_count++;

		 if ( point_count > 31 )
			break;
		 }

	  if ( point_count <= 0 )
		 return;

	  for ( const auto& pos : positions ) {
		 Render::Get( )->AddLine( pos.first, pos.second, FloatColor( 0.0f, 0.0f, 0.0f, 0.4f ), 2.5f );
	  }

	  for ( const auto& pos : positions ) {
		 Render::Get( )->AddLine( pos.first, pos.second, FloatColor( 1.0f, 0.0f, 0.0f, 1.0f ), 1.0f );
	  }
	  }
#endif
}

void CEsp::SetAlpha( int idx ) {
   m_bAlphaFix[ idx ] = true;
}

float CEsp::GetAlpha( int idx ) {
   return m_flAplha[ idx ];
}

void CEsp::AmmoBar( C_CSPlayer* player, Rect2D bbox ) {
   if ( !player )
	  return;

   C_WeaponCSBaseGun* pWeapon = ( C_WeaponCSBaseGun* ) player->m_hActiveWeapon( ).Get( );

   if ( !pWeapon )
	  return;

   int x = bbox.left;
   int y = bbox.top;
   float box_h = ( float ) fabs( bbox.bottom - bbox.top );
   float off = 8;

   int w = 4;
   int h = box_h;

   auto animLayer = player->m_AnimOverlay( ).Element( 1 );
   if ( !animLayer.m_pOwner )
	  return;

   auto activity = player->GetSequenceActivity( animLayer.m_nSequence );

   int iClip = pWeapon->m_iClip1( );
   int iClipMax = pWeapon->GetCSWeaponData( )->m_iMaxClip;

   if ( h ) {
	  float box_w = ( float ) fabsf( box_h / 2 );

	  float width;
	  if ( activity == 967 && animLayer.m_flWeight != 0.f ) {
		 float cycle = animLayer.m_flCycle; // 1 = finished 0 = just started
		 width = ( ( ( ( bbox.right - bbox.left ) * cycle ) / 1.f ) );
	  } else
		 width = ( ( ( ( bbox.right - bbox.left ) * iClip ) / iClipMax ) );

	  FloatColor col_black = FloatColor( 0, 0, 0, ( int ) ( this->m_flAplha[ player->entindex( ) ] * 0.40f ) );

	  //	Render Outlile
	  Render::Get( )->AddRectFilled( Vector4D( x - 1, y + h + 2, bbox.right + 1, y + h + 6 ), col_black );
	  //Render Main Visible rect
	  Render::Get( )->AddRectFilled( Vector4D( x, y + h + 3, bbox.left + width, y + h + 5 ), FloatColor( int( g_Vars.esp.ammo_color.r * 255.f ), int( g_Vars.esp.ammo_color.g * 255.f ), int( g_Vars.esp.ammo_color.b * 255.f ), ( int ) ( this->m_flAplha[ player->entindex( ) ] ) ) );
   }
}

const char* clean_item_name( const char* name ) {
   if ( name[ 0 ] == 'C' )
	  name++;

   auto start = strstr( name, XorStr( "Weapon" ) );
   if ( start != nullptr )
	  name = start + 6;

   return name;
}

void CEsp::RenderDroppedWeapon( C_WeaponCSBaseGun* entity ) {
   wchar_t buf[ 80 ];
   if ( entity->m_hOwnerEntity( ).IsValid( ) ||
		entity->m_vecOrigin( ) == Vector( 0, 0, 0 ) )
	  return;

   Vector2D pointsTransformed[ 8 ];
   Rect2D bbox;
   GetBBox( entity, pointsTransformed, bbox );
   if ( bbox.right == 0 || bbox.bottom == 0 )
	  return;

   auto draw_box = [&] ( Rect2D bbox, FloatColor clr ) -> void {
	  auto color = clr;
	  float
		 length_horizontal = ( bbox.right - bbox.left ) * 0.25f,
		 length_vertical = ( bbox.bottom - bbox.top ) * 0.25f;

	  FloatColor outline = FloatColor( 0.0f, 0.0f, 0.0f, color.a * 0.68f );
	  Render::Get( )->AddRect( bbox.min, bbox.max, color, 0.0f, -1, 1.0f );
	  Render::Get( )->AddRect( bbox.min - Vector2D( 1.0f, 1.0f ), bbox.max + Vector2D( 1.0f, 1.0f ), outline, 0.0f, -1, 1.0f );
	  Render::Get( )->AddRect( bbox.min + Vector2D( 1.0f, 1.0f ), bbox.max - Vector2D( 1.0f, 1.0f ), outline, 0.0f, -1, 1.0f );
   };

   switch ( g_Vars.esp.dropped_weapons_type ) {
	  case 0:
	  break;
	  case 1:
	  if ( entity )
		 draw_box( bbox, g_Vars.esp.dropped_weapons_color );
	  break;
	  case 2:
	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 0 ].x, pointsTransformed[ 0 ].y ), Vector2D( pointsTransformed[ 1 ].x, pointsTransformed[ 1 ].y ), g_Vars.esp.dropped_weapons_color );
	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 0 ].x, pointsTransformed[ 0 ].y ), Vector2D( pointsTransformed[ 6 ].x, pointsTransformed[ 6 ].y ), g_Vars.esp.dropped_weapons_color );
	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 1 ].x, pointsTransformed[ 1 ].y ), Vector2D( pointsTransformed[ 5 ].x, pointsTransformed[ 5 ].y ), g_Vars.esp.dropped_weapons_color );
	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 6 ].x, pointsTransformed[ 6 ].y ), Vector2D( pointsTransformed[ 5 ].x, pointsTransformed[ 5 ].y ), g_Vars.esp.dropped_weapons_color );

	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 2 ].x, pointsTransformed[ 2 ].y ), Vector2D( pointsTransformed[ 1 ].x, pointsTransformed[ 1 ].y ), g_Vars.esp.dropped_weapons_color );
	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 4 ].x, pointsTransformed[ 4 ].y ), Vector2D( pointsTransformed[ 5 ].x, pointsTransformed[ 5 ].y ), g_Vars.esp.dropped_weapons_color );
	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 6 ].x, pointsTransformed[ 6 ].y ), Vector2D( pointsTransformed[ 7 ].x, pointsTransformed[ 7 ].y ), g_Vars.esp.dropped_weapons_color );
	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 3 ].x, pointsTransformed[ 3 ].y ), Vector2D( pointsTransformed[ 0 ].x, pointsTransformed[ 0 ].y ), g_Vars.esp.dropped_weapons_color );

	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 3 ].x, pointsTransformed[ 3 ].y ), Vector2D( pointsTransformed[ 2 ].x, pointsTransformed[ 2 ].y ), g_Vars.esp.dropped_weapons_color );
	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 2 ].x, pointsTransformed[ 2 ].y ), Vector2D( pointsTransformed[ 4 ].x, pointsTransformed[ 4 ].y ), g_Vars.esp.dropped_weapons_color );
	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 7 ].x, pointsTransformed[ 7 ].y ), Vector2D( pointsTransformed[ 4 ].x, pointsTransformed[ 4 ].y ), g_Vars.esp.dropped_weapons_color );
	  Render::Get( )->AddLine( Vector2D( pointsTransformed[ 7 ].x, pointsTransformed[ 7 ].y ), Vector2D( pointsTransformed[ 3 ].x, pointsTransformed[ 3 ].y ), g_Vars.esp.dropped_weapons_color );
	  break;
   }

   int w = bbox.right - bbox.left;

   Render::Get( )->SetTextFont( FONT_VISITOR );

   auto name = XorStr( "UNKNOWN" );

   if ( entity->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER )
	  name = XorStr( "R8 REVOLVER" );
   else if ( entity->m_iItemDefinitionIndex( ) == WEAPON_USPS )
	  name = XorStr( "USP-S" );
   else if ( entity->m_iItemDefinitionIndex( ) == WEAPON_M4A1S )
	  name = XorStr( "M4A1-S" );
   else if ( entity->m_iItemDefinitionIndex( ) == WEAPON_P2000 )
	  name = XorStr( "P2000" );
   else {
	  char buffer[ 256 ] = { '\0' };
	  name = clean_item_name( entity->GetClientClass( )->m_pNetworkName );
	  std::transform( name, name + strlen( name ), buffer, ::toupper );
	  name = buffer;
   }

   Vector2D size = Render::Get( )->CalcTextSize( name );

   float distance = m_LocalPlayer->GetAbsOrigin( ).Distance( entity->GetAbsOrigin( ) ) * 0.01904f;

   char szDist[ 256 ] = { '\0' };

   sprintf_s( szDist, XorStr( "%i %s" ), ( int ) distance, XorStr( "FT" ) );

   Vector2D size2 = Render::Get( )->CalcTextSize( szDist );

   Render::Get( )->AddText( Vector2D( bbox.left + ( ( bbox.right - bbox.left ) / 2 ) - ( size.x / 2 ), bbox.bottom - 2 ), FloatColor( 1.f, 1.f, 1.f, 1.f ),
							OUTLINED | ALIGN_BOTTOM, name );

   Render::Get( )->AddText( Vector2D( bbox.left + ( ( bbox.right - bbox.left ) / 2 ) - ( size2.x / 2 ), bbox.bottom - 12 ), FloatColor( 1.f, 1.f, 1.f, 1.f ),
							OUTLINED | ALIGN_BOTTOM, szDist );

   int x = bbox.left;
   int y = bbox.top;
   float box_h = ( float ) fabs( bbox.bottom - bbox.top );
   float off = 8;

   int h = box_h;

   int iClip = entity->m_iClip1( );
   int iClipMax = entity->GetCSWeaponData( )->m_iMaxClip;

   if ( h ) {
	  float box_w = ( float ) fabsf( box_h / 2 );

	  float width = ( ( ( ( bbox.right - bbox.left ) * iClip ) / iClipMax ) );

	  FloatColor col_black = FloatColor( 0, 0, 0, int( g_Vars.esp.dropped_weapons_color.a * 255.f ) );
	  // Render Outlile
	  Render::Get( )->AddRectFilled( Vector4D( x - 1, y + h + 2, bbox.right + 1, y + h + 6 ), col_black );
	  //Render Main Visible rect
	  Render::Get( )->AddRectFilled( Vector4D( x, y + h + 3, bbox.left + width, y + h + 5 ), FloatColor( int( g_Vars.esp.dropped_weapons_color.r * 255.f ), int( g_Vars.esp.dropped_weapons_color.g * 255.f ), int( g_Vars.esp.dropped_weapons_color.b * 255.f ), int( g_Vars.esp.dropped_weapons_color.a * 255.f ) ) );
   }

}

const char* itemIcons[] =
{
   "`", //0 - default
   "B",
   "C",
   "D",
   "E",
   "none",
   "none",
   "F",
   "G",
   "H",

   "I", //10
   "J",
   "none",
   "K",
   "L",
   "none",
   "M",
   "N",
   "none",
   "O",

   "none", //20
   "none",
   "none",
   "z",
   "P",
   "Q",
   "R",
   "S",
   "T",
   "U",

   "V", //30
   "W",
   "\\",
   "Y",
   "Z",
   "[",
   "X",
   "none",
   "]",
   "^",

   "_", //40
   "`",
   "`",
   "a",
   "b",
   "c",
   "d",
   "e",
   "f",
   "g",

   "none", //50
   "none",
   "none",
   "none",
   "none",
   "none",
   "none",
   "0",
   "none",
   "k",

   "l", //60
   "m",
   "none",
   "n",
   "o",

   "none",
   "none",
   "none",
   "none",

   "none", // 69
   "1",
   "none",
   "2",
   "none",
   "k",
   "3",
   "4",
   "none",
   "5",
   "none",
   "none",
   "d",
   "e",
   "b"
};

const char* itemNames[] =
{
   XorStr( "Knife" ), //0 - default
   XorStr( "Deagle" ),
   XorStr( "Dual Berettas" ),
   XorStr( "FiveseveN" ),
   XorStr( "Glock" ),
   XorStr( "none" ),
   XorStr( "none" ),
   XorStr( "AK-47" ),
   XorStr( "AUG" ),
   XorStr( "AWP" ),

   XorStr( "Famas" ), //10
   XorStr( "G3SG1" ),
   XorStr( "none" ),
   XorStr( "Galil Ar" ),
   XorStr( "M249" ),
   XorStr( "none" ),
   XorStr( "M4A4" ),
   XorStr( "Mac-10" ),
   XorStr( "none" ),
   XorStr( "P90" ),

   XorStr( "none" ), //20
   XorStr( "none" ),
   XorStr( "none" ),
   XorStr( "MP5-SD" ),
   XorStr( "UMP-45" ),
   XorStr( "XM1014" ),
   XorStr( "Bizon" ),
   XorStr( "Mag7" ),
   XorStr( "Negev" ),
   XorStr( "Sawed-Off" ),

   XorStr( "Tec-9" ), //30
   XorStr( "Zeus" ),
   XorStr( "P2000" ),
   XorStr( "MP7" ),
   XorStr( "MP9" ),
   XorStr( "Nova" ),
   XorStr( "P250" ),
   XorStr( "none" ),
   XorStr( "SCAR-20" ),
   XorStr( "SG553" ),

   XorStr( "SSG-08" ), //40
   XorStr( "Knife" ),
   XorStr( "Knife" ),
   XorStr( "Flash" ),
   XorStr( "Nade" ),
   XorStr( "Smoke" ),
   XorStr( "Molotov" ),
   XorStr( "Decoy" ),
   XorStr( "Incendiary" ),
   XorStr( "C4" ),

   XorStr( "none" ), //50
   XorStr( "none" ),
   XorStr( "none" ),
   XorStr( "none" ),
   XorStr( "none" ),
   XorStr( "none" ),
   XorStr( "none" ),
   XorStr( "Medi-Shot" ),
   XorStr( "none" ),
   XorStr( "Knife" ),

   XorStr( "M4A1-S" ), //60
   XorStr( "USP-S" ),
   XorStr( "none" ),
   XorStr( "CZ75A" ),
   XorStr( "R8 Revolver" ),

   XorStr( "none" ),
   XorStr( "none" ),
   XorStr( "none" ),
   XorStr( "Tactical Grenade" ),

   XorStr( "Hands" ), // 69
   XorStr( "Breach Charge" ),
   XorStr( "none" ),
   XorStr( "Tablet" ),
   XorStr( "none" ),
   XorStr( "Knife" ),
   XorStr( "Axe" ),
   XorStr( "Hammer" ),
   XorStr( "none" ),
   XorStr( "Wrench" ),
   XorStr( "none" ),
   XorStr( "Spectral Shiv" ),
   XorStr( "Fire Bomb" ),
   XorStr( "Diversion Device" ),
   XorStr( "Frag Grenade" )
};

const char* ItemDefinitionIndexToString( int index ) {
   if ( index < 0 || index > 83 )
	  index = 0;

   return itemNames[ index ];
}

const char* ItemDefinitionIndexToIcon( int index ) {
   if ( index < 0 || index > 83 )
	  index = 0;

   return itemIcons[ index ] != "none" ? itemIcons[ index ] : itemNames[ index ];
}

void CEsp::RenderNades( C_WeaponCSBaseGun* nade ) {
   if ( !g_Vars.esp.nades && !g_Vars.esp.nades_box )
	  return;

   const model_t* model = nade->GetModel( );
   if ( !model )
	  return;

   studiohdr_t* hdr = Source::m_pModelInfo->GetStudiomodel( model );
   if ( !hdr )
	  return;

   int item_definition = 0;
   bool dont_render = false;
   C_SmokeGrenadeProjectile* pSmokeEffect = nullptr;
   Color Nadecolor;
   std::string Name = hdr->szName;
   switch ( nade->GetClientClass( )->m_ClassID ) {
	  case ClassId_t::CBaseCSGrenadeProjectile:
	  if ( Name[ 16 ] == 's' ) {
		 Name = XorStr( "FLASH" );
		 item_definition = WEAPON_FLASHBANG;
	  } else {
		 Name = XorStr( "FRAG" );
		 item_definition = WEAPON_FRAG_GRENADE;
	  }
	  break;
	  case ClassId_t::CSmokeGrenadeProjectile:
	  Name = XorStr( "SMOKE" );
	  item_definition = WEAPON_SMOKEGRENADE;
	  pSmokeEffect = reinterpret_cast< C_SmokeGrenadeProjectile* >( nade );
	  if ( pSmokeEffect ) {
		 const auto spawn_time = TICKS_TO_TIME( pSmokeEffect->m_nSmokeEffectTickBegin( ) );
		 const auto time = ( spawn_time + C_SmokeGrenadeProjectile::GetExpiryTime( ) ) - Source::m_pGlobalVars->curtime;
		 const auto factor = ( ( spawn_time + C_SmokeGrenadeProjectile::GetExpiryTime( ) ) - Source::m_pGlobalVars->curtime ) / C_SmokeGrenadeProjectile::GetExpiryTime( );

		 if ( factor > 0.0f )
			dont_render = true;
	  } else {
		 dont_render = false;
	  }
	  break;
	  case ClassId_t::CMolotovProjectile:
	  Name = XorStr( "MOLLY" );
	  item_definition = ( ( C_CSPlayer* ) ( nade->m_hOwnerEntity( ).Get( ) ) )->m_iTeamNum( ) == TEAM_CT ? WEAPON_INC : WEAPON_MOLOTOV;
	  break;
	  case ClassId_t::CDecoyProjectile:
	  Name = XorStr( "DECOY" );
	  item_definition = WEAPON_DECOY;
	  break;
	  default:
	  return;
   }

   Vector2D points_transformed[ 8 ];
   Rect2D size;

   if ( !GetBBox( nade, points_transformed, size ) || dont_render )
	  return;

   if ( size.right == 0 || size.bottom == 0 )
	  return;

   // Render::Get( )->SetTextFont( FONT_VISITOR );
   Vector2D calculated_text_size = Render::Get( )->CalcTextSize( Name.c_str( ) );

   if ( g_Vars.esp.nades_box ) {
	  Render::Get( )->AddLine( Vector2D( size.left - 1, size.bottom - 1 ), Vector2D( size.left - 1, size.top + 1 ), Color( 20, 20, 20, 240 ).GetD3DColor( ) );
	  Render::Get( )->AddLine( Vector2D( size.right + 1, size.top + 1 ), Vector2D( size.right + 1, size.bottom - 1 ), Color( 20, 20, 20, 240 ).GetD3DColor( ) );
	  Render::Get( )->AddLine( Vector2D( size.left - 1, size.top + 1 ), Vector2D( size.right + 1, size.top + 1 ), Color( 20, 20, 20, 240 ).GetD3DColor( ) );
	  Render::Get( )->AddLine( Vector2D( size.right + 1, size.bottom - 1 ), Vector2D( size.left + -1, size.bottom - 1 ), Color( 20, 20, 20, 240 ).GetD3DColor( ) );

	  Render::Get( )->AddLine( Vector2D( size.left, size.bottom ), Vector2D( size.left, size.top ), g_Vars.esp.nades_box_color );
	  Render::Get( )->AddLine( Vector2D( size.left, size.top ), Vector2D( size.right, size.top ), g_Vars.esp.nades_box_color );
	  Render::Get( )->AddLine( Vector2D( size.right, size.top ), Vector2D( size.right, size.bottom ), g_Vars.esp.nades_box_color );
	  Render::Get( )->AddLine( Vector2D( size.right, size.bottom ), Vector2D( size.left, size.bottom ), g_Vars.esp.nades_box_color );
   }

   if ( g_Vars.esp.nades ) {
	  Render::Get( )->SetTextFont( FONT_CSGO_ICONS2 );

	  std::string name = ItemDefinitionIndexToIcon( item_definition );
	  Render::Get( )->AddText( Vector2D( size.left + ( size.right - size.left ) * 0.5f,
							   size.top - 2.0f ), g_Vars.esp.nades_text_color, OUTLINED | CENTER_X | ALIGN_BOTTOM, name.c_str( ) );
   }
}

void CEsp::RenderPlantedC4( C_BaseEntity* ent, Rect2D bbox ) {
   if ( bbox.max.x == 0 || bbox.max.y == 0 )
	  return;

   DrawBox( bbox, g_Vars.esp.c4_color, ToCSPlayer( ent ) );

   int bombTimer = std::ceil( ( ( C_PlantedC4* ) ent )->m_flC4Blow( ) - Source::m_pGlobalVars->curtime );
   std::string timer = std::to_string( bombTimer );

   auto name = ( bombTimer < 0.f ) ? XorStr( "Bomb" ) : timer;

   Render::Get( )->SetTextFont( FONT_VISITOR );

   float w = bbox.max.x - bbox.min.x;
   Render::Get( )->AddText( { ( bbox.min.x + w * 0.5f ), bbox.max.y + 1 }, g_Vars.esp.c4_color, OUTLINED | CENTER_X, name.c_str( ) );
}

void CEsp::DrawAntiAimIndicator( ) {
   if ( !g_Vars.esp.aa_indicator || !Source::m_pEngine->IsInGame( ) )
	  return;

   Vector2D center = Render::Get( )->GetScreenSize( ) * 0.5f;
   Vector2D screen = Render::Get( )->GetScreenSize( );

   ColorU32 back = Color( 198, 198, 198, 192 ).GetD3DColor( ),
	  left = back, right = back;

   bool bLeft = false, bRight = false, bBack = false;
   // TODO: FIX COLORS!
   // for some reason, they are BGRA, when it should be rgba
   switch ( g_Vars.globals.manual_aa ) {
	  case 0:
	  left = g_Vars.esp.aa_indicator_color;
	  bLeft = true;
	  bRight = false;
	  bBack = false;
	  break;
	  case 1:
	  back = g_Vars.esp.aa_indicator_color;
	  bLeft = false;
	  bRight = false;
	  bBack = true;
	  break;
	  case 2:
	  right = g_Vars.esp.aa_indicator_color;
	  bLeft = false;
	  bRight = true;
	  bBack = false;
	  break;
   }

   if ( g_Vars.esp.aa_indicator_type == 0 ) {
	  //down
	  if ( bBack )
		 Render::Get( )->AddTriangleFilled( Vector2D( center.x - 10, center.y + 35 ), Vector2D( center.x, center.y + 50 ),
											Vector2D( center.x + 10, center.y + 35 ), back );

	  //left
	  if ( bLeft )
		 Render::Get( )->AddTriangleFilled( Vector2D( center.x - 50, center.y ), Vector2D( center.x - 35, center.y + 10 ),
											Vector2D( center.x - 35, center.y - 10 ), left );

	  //right 
	  if ( bRight )
		 Render::Get( )->AddTriangleFilled( Vector2D( center.x + 50, center.y ), Vector2D( center.x + 35, center.y + 10 ),
											Vector2D( center.x + 35, center.y - 10 ), right );
   } else if ( g_Vars.esp.aa_indicator_type == 1 ) {
	  //down
	  if ( bBack )
		 Render::Get( )->AddQuadFilled( Vector2D( center.x, screen.y - 30.f ), Vector2D( center.x - 15.f, screen.y - 60.f ),
										Vector2D( center.x, screen.y - 50.f ), Vector2D( center.x + 15.f, screen.y - 60.f ), back );

	  //left 
	  if ( bLeft )
		 Render::Get( )->AddQuadFilled( Vector2D( center.x - 100.f, screen.y - 45.f ), Vector2D( center.x - 65.f, screen.y - 60.f ),
										Vector2D( center.x - 75.f, screen.y - 45.f ), Vector2D( center.x - 65.f, screen.y - 30.f ), left );

	  //right
	  if ( bRight )
		 Render::Get( )->AddQuadFilled( Vector2D( center.x + 100.f, screen.y - 45.f ), Vector2D( center.x + 65.f, screen.y - 60.f ),
										Vector2D( center.x + 75.f, screen.y - 45.f ), Vector2D( center.x + 65.f, screen.y - 30.f ), right );

   }
}

void CEsp::RenderSnapline( Vector2D pos ) {
   auto screen = Render::Get( )->GetScreenSize( );

   switch ( g_Vars.esp.snaplines_pos ) {
	  case 0: // left
	  screen.x = 0.0f;
	  screen.y *= 0.5f;
	  break;
	  case 1: // right
	  screen.y *= 0.5f;
	  break;
	  case 2: // top
	  screen.x *= 0.5f;
	  screen.y = 0.f;
	  break;
	  case 3: // bottom
	  screen.x *= 0.5f;
	  break;
	  case 4: // center
	  screen *= 0.5f;
	  break;
   }

   Render::Get( )->AddLine( screen, pos, g_Vars.esp.snaplines_color );
}

void CEsp::DrawBox( Rect2D bbox, const FloatColor& clr, C_CSPlayer* player ) {
   if ( !player )
	  return;

   auto color = clr;
   color.a *= ( m_flAplha[ player->entindex( ) ] / 255.f );

   float
	  length_horizontal = ( bbox.right - bbox.left ) * 0.25f,
	  length_vertical = ( bbox.bottom - bbox.top ) * 0.25f;

   FloatColor outline = FloatColor( 0.0f, 0.0f, 0.0f, color.a * 0.68f );
   if ( g_Vars.esp.box_type == 0 ) {
	  Render::Get( )->AddRect( bbox.min, bbox.max, color, 0.0f, -1, 1.0f );
	  Render::Get( )->AddRect( bbox.min - Vector2D( 1.0f, 1.0f ), bbox.max + Vector2D( 1.0f, 1.0f ), outline, 0.0f, -1, 1.0f );
	  Render::Get( )->AddRect( bbox.min + Vector2D( 1.0f, 1.0f ), bbox.max - Vector2D( 1.0f, 1.0f ), outline, 0.0f, -1, 1.0f );
   } else if ( g_Vars.esp.box_type == 1 ) {
	  Render::Get( )->AddRectFilled( Vector4D( bbox.left - 1, bbox.top - 1, bbox.left + 1 + length_horizontal, bbox.top + 2 ), outline, 0.0f, -1 );
	  Render::Get( )->AddRectFilled( Vector4D( bbox.right - 1 - length_horizontal, bbox.top - 1, bbox.right + 1, bbox.top + 2 ), outline, 0.0f, -1 );
	  Render::Get( )->AddRectFilled( Vector4D( bbox.left - 1, bbox.bottom - 2, bbox.left + 1 + length_horizontal, bbox.bottom + 1 ), outline, 0.0f, -1 );
	  Render::Get( )->AddRectFilled( Vector4D( bbox.right - 1 - length_horizontal, bbox.bottom - 2, bbox.right + 1, bbox.bottom + 1 ), outline, 0.0f, -1 );

	  Render::Get( )->AddRectFilled( Vector4D( bbox.left - 1, bbox.top + 2, bbox.left + 2, bbox.top + 1 + length_vertical ), outline, 0.0f, -1 );
	  Render::Get( )->AddRectFilled( Vector4D( bbox.right - 2, bbox.top + 2, bbox.right + 1, bbox.top + 1 + length_vertical ), outline, 0.0f, -1 );
	  Render::Get( )->AddRectFilled( Vector4D( bbox.left - 1, bbox.bottom - 1 - length_vertical, bbox.left + 2, bbox.bottom - 2 ), outline, 0.0f, -1 );
	  Render::Get( )->AddRectFilled( Vector4D( bbox.right - 2, bbox.bottom - 1 - length_vertical, bbox.right + 1, bbox.bottom - 2 ), outline, 0.0f, -1 );

	  Render::Get( )->AddLine( Vector2D( bbox.left, bbox.top ), Vector2D( bbox.left + length_horizontal - 1, bbox.top ), color );
	  Render::Get( )->AddLine( Vector2D( bbox.right - length_horizontal, bbox.top ), Vector2D( bbox.right - 1, bbox.top ), color );
	  Render::Get( )->AddLine( Vector2D( bbox.left, bbox.bottom - 1 ), Vector2D( bbox.left + length_horizontal - 1, bbox.bottom - 1 ), color );
	  Render::Get( )->AddLine( Vector2D( bbox.right - length_horizontal, bbox.bottom - 1 ), Vector2D( bbox.right - 1, bbox.bottom - 1 ), color );

	  Render::Get( )->AddLine( Vector2D( bbox.left, bbox.top ), Vector2D( bbox.left, bbox.top + length_vertical - 1 ), color );
	  Render::Get( )->AddLine( Vector2D( bbox.right - 1, bbox.top ), Vector2D( bbox.right - 1, bbox.top + length_vertical - 1 ), color );
	  Render::Get( )->AddLine( Vector2D( bbox.left, bbox.bottom - length_vertical ), Vector2D( bbox.left, bbox.bottom - 1 ), color );
	  Render::Get( )->AddLine( Vector2D( bbox.right - 1, bbox.bottom - length_vertical ), Vector2D( bbox.right - 1, bbox.bottom - 1 ), color );
   }
}

void CEsp::DrawArmorBar( C_CSPlayer* player, Rect2D bbox ) {
   Color col_black = Color( 0, 0, 0, m_flAplha[ player->entindex( ) ] );
   int player_armor = player->m_ArmorValue( );
   int player_armor_max = 100;

   float width;

   float x;
   float y = bbox.top;

   int offset = g_Vars.esp.health ? 14 : 9;

   x = bbox.left - offset;
   width = fabsf( bbox.bottom - bbox.top ) - ( ( ( fabsf( bbox.bottom - bbox.top ) * player_armor ) / player_armor_max ) );

   float w = 4;
   float h = fabsf( bbox.bottom - bbox.top );

   FloatColor fill = FloatColor( 50, 100, 200, ( int ) ( m_flAplha[ player->entindex( ) ] ) );
   auto color = FloatColor( 255, 255, 255, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.85f ) );

   Render::Get( )->SetTextFont( FONT_VISITOR );
   Render::Get( )->AddRectFilled( Vector2D( x + 3, y - 1 ), Vector2D( x + w + 3, y + h + 1 ), FloatColor( 0.0f, 0.0f, 0.0f, m_flAplha[ player->entindex( ) ] / 357.0f ) );
   Render::Get( )->AddRectFilled( Vector2D( x + 4, y + width ), Vector2D( x + w - 1 + 3, y + h ), fill );
   if ( player_armor != 100 ) {
	  Render::Get( )->AddText( Vector2D( x + 3, y + width ), color,
							   OUTLINED | CENTER_X | CENTER_Y, XorStr( "%d" ), player_armor );
   }
}

void CEsp::DrawHealthBar( C_CSPlayer* player, Rect2D bbox ) {
   FloatColor col_black = FloatColor( 0, 0, 0, static_cast< int > ( m_flAplha[ player->entindex( ) ] ) );
   int player_hp = player->m_iHealth( );
   int player_hp_max = 100;

   static float prev_player_hp[ 65 ];
   constexpr float SPEED_FREQ = 255 / 1.0f;

   if ( g_Vars.esp.animated_hp ) {
	  if ( prev_player_hp[ player->entindex( ) ] > player_hp )
		 prev_player_hp[ player->entindex( ) ] -= SPEED_FREQ * Source::m_pGlobalVars->frametime;
	  else
		 prev_player_hp[ player->entindex( ) ] = player_hp;
   } else
	  prev_player_hp[ player->entindex( ) ] = player_hp;

   float x = bbox.left - 10, y = bbox.top, width = fabsf( bbox.bottom - bbox.top ) - ( ( ( fabsf( bbox.bottom - bbox.top ) * prev_player_hp[ player->entindex( ) ] ) / player_hp_max ) );
   float w = 4, h = fabsf( bbox.bottom - bbox.top );

   FloatColor fill = FloatColor( static_cast< int > ( ( 130.0f - player_hp * 1.3f ) ), static_cast< int > ( ( int ) ( player_hp * 2.55f ) ), 10, static_cast< int > ( ( m_flAplha[ player->entindex( ) ] ) ) );
   auto color = FloatColor( 255, 255, 255, static_cast< int > ( ( m_flAplha[ player->entindex( ) ] * 0.85f ) ) );

   Render::Get( )->SetTextFont( FONT_VISITOR );
   Render::Get( )->AddRectFilled( Vector2D( x + 4, y - 1 ), Vector2D( x + w + 4, y + h + 1 ), FloatColor( 0, 0, 0, ( int ) ( this->m_flAplha[ player->entindex( ) ] * 0.40f ) ) );
   Render::Get( )->AddRectFilled( Vector2D( x + 5, y + width /*+ 1*/ ), Vector2D( x + w - 1 + 4, y + h ), fill );

   if ( player_hp != 100 )
	  Render::Get( )->AddText( Vector2D( x + 3, y + width + 1 ), color, OUTLINED | CENTER_X | CENTER_Y, XorStr( "%d" ), player_hp );
}

void CEsp::DrawInfo( C_CSPlayer* player, Rect2D bbox, player_info_t player_info ) {
   std::vector<std::pair<FloatColor, std::string>> m_vecTextInfo;

   //auto color = FloatColor( 255, 255, 255, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) );

   auto animState = player->m_PlayerAnimState( );
   if ( !animState )
	  return;

   auto color = g_Vars.menu.main_color;
   color.a = m_flAplha[ player->entindex( ) ] / 255.0f;

   if ( g_Vars.esp.draw_money )
	  m_vecTextInfo.emplace_back( FloatColor( 133, 198, 22, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "$" ) + std::to_string( player->m_iAccount( ) ) );

   if ( g_Vars.esp.draw_armor && player->m_ArmorValue( ) > 0 ) {
	  std::string name = player->m_bHasHelmet( ) ? XorStr( "HK" ) : XorStr( "K" );
	  m_vecTextInfo.emplace_back( color, name.c_str( ) );
   }


   /*
   char buffer[ 32 ];
   sprintf_s( buffer, XorStr( "R: [%i] | [%i]" ), g_Vars.globals.m_iResolverType2[ player->entindex( ) ], g_Vars.globals.m_iResolverSide[ player->entindex( ) ] );

   m_vecTextInfo.emplace_back( FloatColor( 108, 160, 203, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), buffer );

	char buffer[ 32 ];
   sprintf_s( buffer, XorStr( "R: [%i]" ), g_Vars.globals.m_iResolverType2[ player->entindex( ) ] );

   m_vecTextInfo.emplace_back( FloatColor( 108, 160, 203, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), buffer );

   char buffer3[ 32 ];
   sprintf_s( buffer3, XorStr( "R2: [%i]" ), g_Vars.globals.m_iResolverType[ player->entindex( ) ] );

   m_vecTextInfo.emplace_back( FloatColor( 108, 160, 203, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), buffer3 );

   char buffer2[ 32 ];
   sprintf_s( buffer2, XorStr( "P: [%i]" ), g_Vars.globals.m_iRecordPriority[ player->entindex( ) ] );

   m_vecTextInfo.emplace_back( FloatColor( 108, 160, 203, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), buffer2 );

   */

   /* char buffer2[ 128 ];
	sprintf_s( buffer2, XorStr( "desync delta: [%i]" ), int( animState->GetDesyncDelta( ) ) );

	m_vecTextInfo.emplace_back( FloatColor( 108, 160, 203, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), buffer2 );*/

   if ( player->m_bIsScoped( ) && g_Vars.esp.draw_scoped )
	  m_vecTextInfo.emplace_back( FloatColor( 108, 160, 203, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "ZOOM" ) );

   if ( g_Vars.esp.draw_flashed && player->m_flFlashDuration( ) > 1.f )
	  m_vecTextInfo.emplace_back( FloatColor( 255, 216, 0, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "FLASHED" ) );

   if ( IsFakeDucking( player ) )
	  m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "FD" ) );

   int ping = ( *Source::m_pPlayerResource.Xor( ) )->GetPlayerPing( player->entindex( ) );

   if ( g_Vars.esp.draw_hit ) {
	  if ( g_Vars.globals.m_iResolverType2[ player->entindex( ) ] == 3 )
		 m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "HIT" ) );
   }

   /*if ( g_Vars.esp.draw_hostage  )
   m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "Hostage" ) );*/

   if ( g_Vars.esp.draw_distance ) {
	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( local && !local->IsDead( ) ) {
		 float distance = local->GetAbsOrigin( ).Distance( player->GetAbsOrigin( ) ) * 0.01904f;
		 std::string str = std::to_string( int( distance ) ) + XorStr( " FT" );
		 m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( str.c_str( ) ) );
	  }
   }

   auto weapons = player->m_hMyWeapons( );
   for ( size_t i = 0; i < 48; ++i ) {
	  auto weapon_handle = weapons[ i ];
	  if ( !weapon_handle.IsValid( ) )
		 break;

	  auto weapon = ( C_BaseCombatWeapon* ) weapon_handle.Get( );
	  if ( !weapon )
		 continue;

	  auto definition_index = weapon->m_Item( ).m_iItemDefinitionIndex( );

	  if ( definition_index == WEAPON_C4 && g_Vars.esp.draw_bombc4 )
		 m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "BOMB" ) );

	  if ( definition_index == WEAPON_ZEUS && g_Vars.esp.draw_taser ) {
		 C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
		 if ( !local || local->IsDead( ) )
			m_vecTextInfo.emplace_back( FloatColor( 108, 160, 203, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "TASER" ) );
		 else {
			auto weapon_data = ( ( C_WeaponCSBaseGun* ) weapon )->GetCSWeaponData( ).Xor( );

			if ( !weapon_data )
			   continue;

			float distance = local->GetAbsOrigin( ).Distance( player->GetAbsOrigin( ) );
			float maxRange = weapon_data->m_flWeaponRange;

			if ( distance < maxRange )
			   m_vecTextInfo.emplace_back( FloatColor( 229, 57, 53, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "TASER" ) );
			else
			   m_vecTextInfo.emplace_back( FloatColor( 108, 160, 203, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "TASER" ) );
		 }
	  }
   }



   if ( g_Vars.esp.draw_reloading && player->IsReloading( ) ) {
	  m_vecTextInfo.emplace_back( FloatColor( 108, 160, 203, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "R" ) );
   }

   std::string ping_str = std::to_string( ping ) + XorStr( " MS" );

   if ( g_Vars.esp.draw_ping ) {
	  if ( ping <= 60 )
		 m_vecTextInfo.emplace_back( FloatColor( 133, 198, 22, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), ping_str.c_str( ) );
	  else if ( ping > 60 && ping < 100 )
		 m_vecTextInfo.emplace_back( FloatColor( 253, 216, 53, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), ping_str.c_str( ) );
	  else if ( ping >= 100 )
		 m_vecTextInfo.emplace_back( FloatColor( 229, 57, 53, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), ping_str.c_str( ) );
   }
#if 0
   m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, 255 ), XorStr( "6 W:%f" ) + std::to_string( player->m_AnimOverlay( ).Element( 6 ).m_flWeight ) );
   m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, 255 ), XorStr( "6 R :%f" ) + std::to_string( player->m_AnimOverlay( ).Element( 6 ).m_flPlaybackRate ) );
   m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, 255 ), XorStr( "3 W:%f" ) + std::to_string( player->m_AnimOverlay( ).Element( 3 ).m_flWeight ) );
   m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, 255 ), XorStr( "3 C :%f" ) + std::to_string( player->m_AnimOverlay( ).Element( 3 ).m_flCycle ) );
   m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, 255 ), XorStr( "12 W :%f" ) + std::to_string( player->m_AnimOverlay( ).Element( 12 ).m_flWeight ) );
   m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, 255 ), XorStr( "12 C :%f" ) + std::to_string( player->m_AnimOverlay( ).Element( 12 ).m_flCycle ) );

   int delta = TIME_TO_TICKS( player->m_flSimulationTime( ) - player->m_flOldSimulationTime( ) );
   m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, 255 ), XorStr( "ticks " ) + std::to_string( delta ) );

   auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
   if ( lag_data.IsValid( ) ) {
	  m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "SIDE: " ) + std::to_string( lag_data->m_iResolverSide ) );
	  m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, ( int ) ( m_flAplha[ player->entindex( ) ] * 0.8f ) ), XorStr( "TYPE: " ) + std::to_string( lag_data->m_iResolverType ) );
   }

#endif

#if 0
   if ( C_CSPlayer::GetLocalPlayer( )->m_hActiveWeapon( ).Get( ) ) {
	  auto hdr = *( studiohdr_t** ) ( player->m_pStudioHdr( ) );
	  auto hitboxSet = hdr->pHitboxSet( 0 );
	  auto headbox = hitboxSet->pHitbox( HITBOX_HEAD );
	  auto center = ( headbox->bbmin + headbox->bbmax ) * 0.5f;
	  center = center.Transform( player->m_CachedBoneData( ).Element( headbox->bone ) );

	  Autowall::C_FireBulletData data;
	  data.m_bPenetration = true;
	  data.m_Player = C_CSPlayer::GetLocalPlayer( );
	  data.m_TargetPlayer = player;
	  data.m_WeaponData = ( ( C_WeaponCSBaseGun* ) C_CSPlayer::GetLocalPlayer( )->m_hActiveWeapon( ).Get( ) )->GetCSWeaponData( ).Xor( );
	  data.m_vecStart = C_CSPlayer::GetLocalPlayer( )->GetEyePosition( );
	  data.m_vecDirection = ( center - data.m_vecStart ).Normalized( );

	  float dmg = Autowall::FireBullets( &data );
	  if ( dmg > 0.0f )
		 m_vecTextInfo.emplace_back( FloatColor( 244, 67, 54, ( int ) ( m_flAplha[ player->entindex( ) ] * 1.0f ) ), std::to_string( dmg ) );
   }
#endif

   float i = 2.f;

   float x = bbox.right + 3.0f;

   static float min_height = Render::Get( )->CalcTextSize( XorStr( "ABCIW1234567890" ) ).y + 1;

   Render::Get( )->SetTextFont( FONT_VISITOR );
   for ( auto text : m_vecTextInfo ) {
	  Render::Get( )->AddText( Vector2D( x, ( bbox.top - 3 ) + i ), text.first, OUTLINED, text.second.c_str( ) );
	  i += min_height /*- 2.0f*/;
   }
		 }


wchar_t GetWeaponIcon( C_WeaponCSBaseGun* weapon ) {
   int item_index = ( ( C_BaseAttributableItem* ) weapon )->m_Item( ).m_iItemDefinitionIndex( );
   int code = 0xE000 + MAX( 0, MIN( WEAPON_KNIFE_PUSH, item_index ) );

   return ( wchar_t ) code;
};

void CEsp::DrawBottomInfo( C_CSPlayer* player, Rect2D bbox, player_info_t player_info ) {
   std::vector<std::pair<FloatColor, std::string>> m_vecTextInfo;
   std::vector<std::pair<FloatColor, std::string>> m_vecWeaponInfo;

   float i = g_Vars.esp.draw_ammo_bar ? 8.f : 2.f;

   auto GetVerifyName = [&] ( C_WeaponCSBaseGun* weapon ) -> std::string {
	  std::string final_str_weapon;
	  const char* name = nullptr;
	  if ( weapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER )
		 name = XorStr( "R8 REVOLVER" );
	  else if ( weapon->m_iItemDefinitionIndex( ) == WEAPON_USPS )
		 name = XorStr( "USP-S" );
	  else if ( weapon->m_iItemDefinitionIndex( ) == WEAPON_M4A1S )
		 name = XorStr( "M4A1-S" );
	  else if ( weapon->m_iItemDefinitionIndex( ) == WEAPON_P2000 )
		 name = XorStr( "P2000" );
	  else {
		 char buffer[ 256 ] = { '\0' };
		 name = clean_item_name( weapon->GetClientClass( )->m_pNetworkName );
		 std::transform( name, name + strlen( name ), buffer, ::toupper );
		 name = buffer;
	  }

	  final_str_weapon += name;
	  return final_str_weapon;
   };

   auto weapon = ( C_WeaponCSBaseGun* ) ( player->m_hActiveWeapon( ).Get( ) );
   //auto color = FloatColor( 255, 255, 255, ( int ) ( m_flAplha[ player->entindex( ) ] ) );
   auto color = g_Vars.menu.main_color;
   color.a = m_flAplha[ player->entindex( ) ] / 255.0f;
   std::string final_str_weapon;
   if ( g_Vars.esp.weapon && weapon && !g_Vars.esp.weapon_other ) {
	  final_str_weapon = GetVerifyName( weapon );
   }

   if ( g_Vars.esp.weapon_ammo && weapon && weapon->m_iClip1( ) >= 0 && !g_Vars.esp.weapon_other ) {
	  auto clip = weapon->m_iClip1( );
	  auto max_ammo = weapon->m_iPrimaryReserveAmmoCount( );
	  final_str_weapon += std::string( XorStr( " " ) ) + std::string( XorStr( "[" ) ) +
		 std::to_string( clip ) + std::string( XorStr( "/" ) ) + std::to_string( max_ammo ) + std::string( XorStr( "]" ) );
   }

   if ( final_str_weapon.size( ) )
	  m_vecTextInfo.emplace_back( color, final_str_weapon );

   std::string final_second_str_weapon;
   if ( g_Vars.esp.weapon_other ) {
	  auto weapons = player->m_hMyWeapons( );
	  for ( size_t i = 0; i < 48; ++i ) {
		 auto weapon_handle = weapons[ i ];
		 if ( !weapon_handle.IsValid( ) )
			break;

		 auto weapon = ( C_WeaponCSBaseGun* ) weapon_handle.Get( );
		 if ( !weapon )
			continue;

		 if ( g_Vars.esp.weapon_ammo && weapon->m_iClip1( ) >= 0 ) {
			final_second_str_weapon = GetVerifyName( weapon );
			auto clip = weapon->m_iClip1( );
			auto max_ammo = weapon->m_iPrimaryReserveAmmoCount( );
			final_second_str_weapon += std::string( XorStr( "[" ) ) +
			   std::to_string( clip ) + std::string( XorStr( "/" ) ) + std::to_string( max_ammo ) + std::string( XorStr( "]" ) );
		 } else {
			final_second_str_weapon = GetVerifyName( weapon );
		 }

		 if ( final_second_str_weapon.size( ) )
			m_vecTextInfo.emplace_back( color, final_second_str_weapon );
	  }
   }

#if 0
   if ( g_Vars.esp.name && player_info.fakeplayer )
	  m_vecTextInfo.emplace_back( FloatColor( 255, 255, 255, ( int ) m_flAplha[ player->entindex( ) ] ), XorStr( "BOT" ) );

   if ( Engine::LagCompensation::Get( )->m_PlayerHistory.count( player->m_entIndex ) > 0 ) {
	  auto& lag_data = Engine::LagCompensation::Get( )->m_PlayerHistory[ player->m_entIndex ];
	  if ( lag_data.m_iFakeSides & 1 ) {
		 m_vecTextInfo.emplace_back( FloatColor( 1.0f, 0.0f, 0.0f, 1.0f ), XorStr( "FAKE" ) );
	  }

	  if ( lag_data.m_iFakeSides & 2 ) {
		 m_vecTextInfo.emplace_back( FloatColor( 0.0f, 1.0f, 0.0f, 1.0f ), XorStr( "LEFT" ) );
	  }

	  if ( lag_data.m_iFakeSides & 4 ) {
		 m_vecTextInfo.emplace_back( FloatColor( 0.0f, 0.0f, 1.0f, 1.0f ), XorStr( "RIGHT" ) );
	  }
   }

   auto ent = GetServerEdict( player->m_entIndex );
   if ( ent ) {
	  Vector kek = *( Vector* ) ( uintptr_t( ent ) + 0x17C );
	  m_vecTextInfo.emplace_back( color, std::to_string( kek.Distance( player->m_vecVelocity( ) ) ) );
   }
#endif
   Render::Get( )->SetTextFont( FONT_VISITOR );
   static float min_height = Render::Get( )->CalcTextSize( XorStr( "ABCIW1234567890" ) ).y;

   for ( auto text : m_vecTextInfo ) {
	  Render::Get( )->AddText( Vector2D( ( bbox.right + bbox.left ) * 0.5f, bbox.bottom + i ), text.first, OUTLINED | CENTER_X, text.second.c_str( ) );
	  i += min_height;
   }

   if ( g_Vars.esp.weapon_icon && weapon ) {
	  Render::Get( )->SetTextFont( FONT_CSGO_ICONS2 );
	  int weapIcon = GetWeaponIcon( weapon );

	  std::string name = ItemDefinitionIndexToIcon( ( ( C_BaseAttributableItem* ) weapon )->m_Item( ).m_iItemDefinitionIndex( ) );
	  Render::Get( )->AddText( Vector2D( ( bbox.right + bbox.left ) * 0.5f, bbox.bottom + i ), color, DROP_SHADOW | CENTER_X, name.c_str( ) );
	  i += min_height;
   }
   }

void CEsp::DrawName( C_CSPlayer* player, Rect2D bbox, player_info_t player_info ) {
   auto clr = g_Vars.esp.name_color;
   clr.a = m_flAplha[ player->entindex( ) ] / 255.0f;
   Render::Get( )->SetTextFont( FONT_VERDANA );
   Render::Get( )->AddText( Vector2D( bbox.left + ( bbox.right - bbox.left ) * 0.5f, bbox.top - 2.f ),
							clr,
							CENTER_X | ALIGN_BOTTOM | DROP_SHADOW, player_info.szName );
}

void CEsp::DrawSkeleton( C_CSPlayer* player ) {
   auto model = player->GetModel( );
   if ( !model )
	  return;

   auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
   if ( !hdr )
	  return;

   auto set = hdr->pHitboxSet( player->m_nHitboxSet( ) );

   std::pair< Vector2D, Vector2D > positions[ 32 ];
   int point_count = 0;

   auto chest = set->pHitbox( HITBOX_UPPER_CHEST );
   auto neck = set->pHitbox( HITBOX_NECK );

   if ( g_Vars.esp.skeleton_history ) {
	  auto data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
	  if ( data.IsValid( ) ) {
		 auto& history = data->m_History;
		 Engine::C_LagRecord* record = nullptr;

		 // start from begin
		 float time = 0.2f;

		 float yaw = player->GetAbsAngles( ).y;

		 for ( auto it = history.begin( ); it != history.end( ); ++it ) {
			point_count = 0;
			if ( Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *it, time ) ) {
			   break;
			}

			if ( it->m_vecOrigin != player->m_vecOrigin( ) || it->m_angAngles.y != yaw || it->m_flDuckAmount != player->m_flDuckAmount( ) ) {
			   record = &*it;
			}
			if ( record && g_Vars.esp.skeleton_history_type == 1 ) {
			   auto middle = record->m_BoneMatrix[ chest->bone ].at( 3 ) + record->m_BoneMatrix[ neck->bone ].at( 3 );
			   middle *= 0.5f;

			   auto upperarm_left = set->pHitbox( HITBOX_LEFT_UPPER_ARM );
			   auto upperarm_right = set->pHitbox( HITBOX_RIGHT_UPPER_ARM );

			   for ( int i = 0; i < hdr->numbones; ++i ) {
				  auto bone = hdr->pBone( i );
				  if ( ( bone->flags & BONE_USED_BY_HITBOX ) == 0 || bone->parent < 0 )
					 continue;

				  if ( bone->parent == chest->bone && i != neck->bone )
					 continue;

				  Vector parentBone;
				  if ( i == upperarm_left->bone || i == upperarm_right->bone ) {
					 parentBone = middle;
				  } else {
					 parentBone = record->m_BoneMatrix[ bone->parent ].at( 3 );
				  }

				  Vector2D parentPos;
				  if ( !WorldToScreen( parentBone, parentPos ) )
					 continue;

				  Vector2D childPos;
				  if ( !WorldToScreen( record->m_BoneMatrix[ i ].at( 3 ), childPos ) )
					 continue;

				  positions[ point_count ].first = parentPos;
				  positions[ point_count ].second = childPos;
				  point_count++;

				  if ( point_count > 31 )
					 break;
			   }

			   if ( point_count <= 0 )
				  return;

			   for ( const auto& pos : positions ) {
				  Render::Get( )->AddLine( pos.first, pos.second, FloatColor( 0.0f, 0.0f, 0.0f, ( m_flAplha[ player->entindex( ) ] / 255.f ) * 0.4f ), 2.5f );
			   }
			   auto color = g_Vars.esp.skeleton_history_color;
			   color.a *= ( m_flAplha[ player->entindex( ) ] / 255.f );

			   for ( const auto& pos : positions ) {
				  Render::Get( )->AddLine( pos.first, pos.second, color, 1.0f );
			   }
			}
		 }
		 if ( record && g_Vars.esp.skeleton_history_type == 0 ) {
			auto middle = record->m_BoneMatrix[ chest->bone ].at( 3 ) + record->m_BoneMatrix[ neck->bone ].at( 3 );
			middle *= 0.5f;

			auto upperarm_left = set->pHitbox( HITBOX_LEFT_UPPER_ARM );
			auto upperarm_right = set->pHitbox( HITBOX_RIGHT_UPPER_ARM );

			for ( int i = 0; i < hdr->numbones; ++i ) {
			   auto bone = hdr->pBone( i );
			   if ( ( bone->flags & BONE_USED_BY_HITBOX ) == 0 || bone->parent < 0 )
				  continue;

			   if ( bone->parent == chest->bone && i != neck->bone )
				  continue;

			   Vector parentBone;
			   if ( i == upperarm_left->bone || i == upperarm_right->bone ) {
				  parentBone = middle;
			   } else {
				  parentBone = record->m_BoneMatrix[ bone->parent ].at( 3 );
			   }

			   Vector2D parentPos;
			   if ( !WorldToScreen( parentBone, parentPos ) )
				  continue;

			   Vector2D childPos;
			   if ( !WorldToScreen( record->m_BoneMatrix[ i ].at( 3 ), childPos ) )
				  continue;

			   positions[ point_count ].first = parentPos;
			   positions[ point_count ].second = childPos;
			   point_count++;

			   if ( point_count > 31 )
				  break;
			}

			if ( point_count <= 0 )
			   return;

			for ( const auto& pos : positions ) {
			   Render::Get( )->AddLine( pos.first, pos.second, FloatColor( 0.0f, 0.0f, 0.0f, ( m_flAplha[ player->entindex( ) ] / 255.f ) * 0.4f ), 2.5f );
			}

			auto color = g_Vars.esp.skeleton_history_color;
			color.a *= ( m_flAplha[ player->entindex( ) ] / 255.f );

			for ( const auto& pos : positions ) {
			   Render::Get( )->AddLine( pos.first, pos.second, color, 1.0f );
			}
		 }
	  }
   }

   auto middle = player->m_CachedBoneData( ).m_Memory.m_pMemory[ chest->bone ].at( 3 ) + player->m_CachedBoneData( ).m_Memory.m_pMemory[ neck->bone ].at( 3 );
   middle *= 0.5f;

   auto upperarm_left = set->pHitbox( HITBOX_LEFT_UPPER_ARM );
   auto upperarm_right = set->pHitbox( HITBOX_RIGHT_UPPER_ARM );

   for ( int i = 0; i < hdr->numbones; ++i ) {
	  auto bone = hdr->pBone( i );
	  if ( ( bone->flags & BONE_USED_BY_HITBOX ) == 0 || bone->parent < 0 )
		 continue;

	  if ( bone->parent == chest->bone && i != neck->bone )
		 continue;

	  Vector parentBone;
	  if ( i == upperarm_left->bone || i == upperarm_right->bone ) {
		 parentBone = middle;
	  } else {
		 parentBone = player->m_CachedBoneData( ).m_Memory.m_pMemory[ bone->parent ].at( 3 );
	  }

	  Vector2D parentPos;
	  if ( !WorldToScreen( parentBone, parentPos ) )
		 continue;

	  Vector2D childPos;
	  if ( !WorldToScreen( player->m_CachedBoneData( ).m_Memory.m_pMemory[ i ].at( 3 ), childPos ) )
		 continue;

	  positions[ point_count ].first = parentPos;
	  positions[ point_count ].second = childPos;
	  point_count++;

	  if ( point_count > 31 )
		 break;
   }

   if ( point_count <= 0 )
	  return;

   for ( const auto& pos : positions ) {
	  Render::Get( )->AddLine( pos.first, pos.second, FloatColor( 0.0f, 0.0f, 0.0f, ( m_flAplha[ player->entindex( ) ] / 255.f ) * 0.4f ), 2.5f );
   }

   auto color = g_Vars.esp.skeleton_color;
   color.a *= ( m_flAplha[ player->entindex( ) ] / 255.f );

   for ( const auto& pos : positions ) {
	  Render::Get( )->AddLine( pos.first, pos.second, color, 1.0f );
   }
}

void CEsp::DrawAimPoints( C_CSPlayer* player ) {
   if ( !g_Vars.rage.enabled || !Source::Ragebot::Get( )->SetupRageOptions( ) )
	  return;

   auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
   if ( !lag_data.IsValid( ) || lag_data->m_History.empty( ) )
	  return;

   auto renderable = player->GetClientRenderable( );
   if ( !renderable )
	  return;

   auto model = renderable->GetModel( );
   if ( !model )
	  return;

   auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
   if ( !hdr )
	  return;

   auto local = C_CSPlayer::GetLocalPlayer( );

   auto hitboxSet = hdr->pHitboxSet( player->m_nHitboxSet( ) );

   auto record = &lag_data->m_History.front( );
   auto should_override = Source::Ragebot::Get( )->OverrideHitscan( player, record, 0 );

   for ( int i = 0; i < hitboxSet->numhitboxes; i++ ) {
	  auto hitbox = hitboxSet->pHitbox( i );
   #if 0
	  if ( g_Vars.rage.ignorelimbs_ifwalking && record->m_vecVelocity.Length2DSquared( ) > 1.0f
		   && ( hitbox->group == Hitgroup_RightArm
		   || hitbox->group == Hitgroup_LeftArm
		   || hitbox->group == Hitgroup_RightLeg
		   || hitbox->group == Hitgroup_LeftLeg ) ) {
		 continue;
   }
   #endif

	  std::vector<Vector> points;
	  float ps = 0.0f;
	  if ( !Source::Ragebot::Get( )->GetBoxOption( hitbox, hitboxSet, ps, should_override ) ) {
		 continue;
	  }

	  ps *= 0.01f;
	  Source::Ragebot::Get( )->Multipoint( player, record, 0, points, hitbox, hitboxSet, ps );

	  for ( int i = 0; i < points.size( ); ++i ) {
		 Vector2D w2s;
		 if ( !WorldToScreen( points[ i ], w2s ) ) {
			continue;
		 }

		 auto color = g_Vars.esp.aim_points_color;
		 color.a = ( m_flAplha[ player->entindex( ) ] / 255.f );

	  #if 0
		 Render::Get( )->SetTextFont( 0 );
		 Render::Get( )->AddText( w2s, color, OUTLINED | CENTER_X | CENTER_Y, XorStr( "%d" ), i );
	  #else

		 Render::Get( )->AddCircle( w2s, 3.0f, FloatColor( 0.0f, 0.0f, 0.0f, ( m_flAplha[ player->entindex( ) ] / 255.f ) ), 12 );
		 Render::Get( )->AddCircleFilled( w2s, 2.5f, color, 12 );

	  #endif
	  }

   #if 0
	  auto boneMatrix = record->GetBoneMatrix( 0 );

	  auto min = hitbox->bbmin.Transform( boneMatrix[ hitbox->bone ] );
	  auto max = hitbox->bbmax.Transform( boneMatrix[ hitbox->bone ] );
	  auto center = ( min + max ) * 0.5f;

	  auto delta = center - local->GetEyePosition( );
	  delta.Normalized( );

	  auto angles = delta.ToEulerAngles( );

	  auto extend = ( max - min );
	  extend.Normalize( );

	  angles.roll = extend.ToEulerAngles( ).pitch * g_Vars.esp.glow_modifier;

	  Vector right, up;
	  delta = angles.ToVectors( &up, &right );

	  //Vector right, up;
	  //delta.GetVectors( right, up );

	  auto testPoint = center + delta * hitbox->m_flRadius;
	  auto testPoint2 = center + right * hitbox->m_flRadius;
	  auto testPoint3 = center + up * hitbox->m_flRadius;

	  Vector2D w2sCenter;
	  if ( WorldToScreen( center, w2sCenter ) ) {
		 Vector2D point;
		 if ( WorldToScreen( testPoint, point ) )
			Render::Get( )->AddLine( w2sCenter, point, FloatColor( 1.0f, 0.0f, 0.0f, 1.0f ) );

		 if ( WorldToScreen( testPoint2, point ) )
			Render::Get( )->AddLine( w2sCenter, point, FloatColor( 0.0f, 1.0f, 0.0f, 1.0f ) );

		 if ( WorldToScreen( testPoint3, point ) )
			Render::Get( )->AddLine( w2sCenter, point, FloatColor( 0.0f, 0.0f, 1.0f, 1.0f ) );
	  }
   #endif

   #if 0
	  if ( hitbox->m_flRadius <= 0.f )
		 continue;

	  auto min = hitbox->bbmin.Transform( record->m_BoneMatrix[ hitbox->bone ] );
	  auto max = hitbox->bbmax.Transform( record->m_BoneMatrix[ hitbox->bone ] );

	  Source::m_pDebugOverlay->AddCapsuleOverlay( min, max, hitbox->m_flRadius, 200, 0, 200, 200,
												  Source::m_pGlobalVars->absoluteframetime + Source::m_pGlobalVars->absoluteframestarttimestddev * 2.0f );
   #endif
	  }
}

bool WorldToScreenOffscreen( const Vector& in, Vector2D& out ) {
   static auto& w2sMatrix = Source::m_pEngine->WorldToScreenMatrix( );

   out.x = w2sMatrix.m[ 0 ][ 0 ] * in.x + w2sMatrix.m[ 0 ][ 1 ] * in.y + w2sMatrix.m[ 0 ][ 2 ] * in.z + w2sMatrix.m[ 0 ][ 3 ];
   out.y = w2sMatrix.m[ 1 ][ 0 ] * in.x + w2sMatrix.m[ 1 ][ 1 ] * in.y + w2sMatrix.m[ 1 ][ 2 ] * in.z + w2sMatrix.m[ 1 ][ 3 ];

   float w = w2sMatrix.m[ 3 ][ 0 ] * in.x + w2sMatrix.m[ 3 ][ 1 ] * in.y + w2sMatrix.m[ 3 ][ 2 ] * in.z + w2sMatrix.m[ 3 ][ 3 ];

   if ( w < 0.001f ) {
	  out.x *= 100000.0f;
	  out.y *= 100000.0f;
	  return true;
   } else {
	  out.x /= w;
	  out.y /= w;

	  Vector2D screen = Render::Get( )->GetScreenSize( );
	  out.x = ( screen.x * 0.5f ) + ( out.x * screen.x ) * 0.5f;
	  out.y = ( screen.y * 0.5f ) - ( out.y * screen.y ) * 0.5f;
	  return false;
   }
};

void CEsp::Offscreen( ) {
   C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

   if ( !LocalPlayer )
	  return;

   const auto screen = Render::Get( )->GetScreenSize( );
   const auto screen_center = screen * 0.5f;

   static auto alpha = 255.f;
   static auto plus_or_minus = false;

   if ( alpha <= 0.f )
	  plus_or_minus = false;
   else if ( alpha >= 255.f )
	  plus_or_minus = true;

   if ( plus_or_minus )
	  alpha -= ( 255.f / 1.f ) * Source::m_pGlobalVars->frametime;
   else
	  alpha += ( 255.f * 1.f ) * Source::m_pGlobalVars->frametime;

   alpha = std::clamp<float>( alpha, 0.f, 255.f );

   FloatColor col = g_Vars.esp.offscreen_color;

   if ( g_Vars.esp.pulse_offscreen_alpha )
	  col.a = alpha / 255.f;

   for ( auto i = 1; i <= Source::m_pGlobalVars->maxClients; i++ ) {
	  auto entity = C_CSPlayer::GetPlayerByIndex( i );
	  if ( !entity || !entity->IsPlayer( ) || entity == LocalPlayer || entity->IsDormant( ) || entity->IsDead( )
		   || ( g_Vars.esp.team_check && entity->m_iTeamNum( ) == LocalPlayer->m_iTeamNum( ) ) )
		 continue;

	  Vector screen_point2;

	  Vector player_pos = entity->m_vecOrigin( );
	  //bool eblo = WorldToScreenOffscreen( player_pos, screen_point );

	  Source::m_pDebugOverlay->ScreenPosition( player_pos, screen_point2 );

	  Vector2D screen_point = Vector2D( screen_point2.x, screen_point2.y );

	  // is point on screen?
	  if ( screen_point.x < 0 || screen_point.y < 0 || screen_point.x > screen.x || screen_point.y > screen.y ) {
		 const auto angle_yaw_rad = std::atan2f( screen_point.y - screen_center.y, screen_point.x - screen_center.x );
		 auto angle = ToDegrees( angle_yaw_rad );

		 const auto new_point_x = screen_center.x +
			g_Vars.esp.offscren_distance * cosf( angle_yaw_rad );
		 const auto new_point_y = screen_center.y +
			g_Vars.esp.offscren_distance * sinf( angle_yaw_rad );

		 float size = g_Vars.esp.offscren_size;

		 std::array<Vector2D, 3> points{
			Vector2D( new_point_x - size, new_point_y + size ),
			Vector2D( new_point_x, new_point_y - size ),
			Vector2D( new_point_x + size, new_point_y + size )
		 };

		 float kek = ToRadians( std::remainderf( angle + 90.0f, 360.0f ) );

		 const auto c = cosf( kek );
		 const auto s = sinf( kek );

		 const auto points_center = ( points.at( 0 ) + points.at( 1 ) + points.at( 2 ) ) / 3;
		 for ( auto& point : points ) {
			point -= points_center;

			const auto x = point.x,
			   y = point.y;

			point.x = x * c - y * s;
			point.y = x * s + y * c;

			point += points_center;
		 }

		 Render::Get( )->AddTriangleFilled( points[ 0 ], points[ 1 ], points[ 2 ], col );

		 if ( g_Vars.esp.offscreen_outline )
			Render::Get( )->AddTriangle( points[ 0 ], points[ 1 ], points[ 2 ], g_Vars.esp.offscreen_outline_color, 1.5f );
	  }
   }
}

void CEsp::OverlayInfo( ) {
   auto local = C_CSPlayer::GetLocalPlayer( );
   if ( !local )
	  return;

   C_WeaponCSBaseGun* weapon = ( C_WeaponCSBaseGun* ) local->m_hActiveWeapon( ).Get( );
   if ( !weapon )
	  return;

   auto animState = local->m_PlayerAnimState( );

   if ( g_Vars.esp.fov_crosshair ) {
	  Vector2D center = Render::Get( )->GetScreenSize( ) * 0.5f;
	  float Radius = ( weapon->GetInaccuracy( ) + weapon->GetSpread( ) ) * center.x;
	  Render::Get( )->AddCircleFilled( center, Radius, g_Vars.esp.fov_crosshair_color, 64 );
   }

#if 0
   static float Weight6 = 0.0f;
   if ( !( int ) ( float ) ( local->m_AnimOverlay( ).Element( 12 ).m_flWeight * 1000.0f )
		&& ( int ) ( float ) ( local->m_AnimOverlay( ).Element( 6 ).m_flWeight * 1000.0f ) == ( int ) ( float ) ( Weight6 * 1000.0f ) )
	  Render::Get( )->AddCircleFilled( Vector2D( 100.0f, 100.0f ), 50.0f, FloatColor::White, 64 );
   Weight6 = local->m_AnimOverlay( ).Element( 6 ).m_flWeight;
#endif

   if ( g_Vars.esp.offscren_enabled )
	  Offscreen( );

   if ( g_Vars.esp.autowall_crosshair )
	  PenetrateCrosshair( Render::Get( )->GetScreenSize( ) * 0.5f );
}

bool CEsp::GetBBox( C_BaseEntity* entity, Vector2D screen_points[], Rect2D& outRect ) {
   Rect2D rect{};
   auto collideable = entity->GetCollideable( );

   if ( !collideable )
	  return false;

   auto min = collideable->OBBMins( );
   auto max = collideable->OBBMaxs( );

   const matrix3x4_t& trans = entity->m_rgflCoordinateFrame( );

   Vector points[] =
   {
	  Vector( min.x, min.y, min.z ),
	  Vector( min.x, max.y, min.z ),
	  Vector( max.x, max.y, min.z ),
	  Vector( max.x, min.y, min.z ),
	  Vector( max.x, max.y, max.z ),
	  Vector( min.x, max.y, max.z ),
	  Vector( min.x, min.y, max.z ),
	  Vector( max.x, min.y, max.z )
   };

   for ( int i = 0; i < 8; i++ ) {
	  points[ i ] = points[ i ].Transform( trans );
   }

   for ( int i = 0; i < 8; i++ )
	  if ( !WorldToScreen( points[ i ], screen_points[ i ] ) )
		 return false;

   auto left = screen_points[ 0 ].x;
   auto top = screen_points[ 0 ].y;
   auto right = screen_points[ 0 ].x;
   auto bottom = screen_points[ 0 ].y;

   for ( int i = 1; i < 8; i++ ) {
	  if ( left > screen_points[ i ].x )
		 left = screen_points[ i ].x;
	  if ( top < screen_points[ i ].y )
		 top = screen_points[ i ].y;
	  if ( right < screen_points[ i ].x )
		 right = screen_points[ i ].x;
	  if ( bottom > screen_points[ i ].y )
		 bottom = screen_points[ i ].y;
   }

   left = std::ceilf( left );
   top = std::ceilf( top );
   right = std::floorf( right );
   bottom = std::floorf( bottom );

   outRect.Set( left, bottom, right, top );
   return true;
}

Encrypted_t<IEsp> IEsp::Get( ) {
   static CEsp instance;
   return &instance;
}