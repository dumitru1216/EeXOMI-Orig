#include "SoundEsp.hpp"
#include "source.hpp"
#include "player.hpp"
#include "entity.hpp"
#include "CBaseHandle.hpp"
#include "Render.hpp"

bool should_draw_beam = false;

struct SoundInfo_t {
  int guid;
  float soundTime;
  float alpha;
  Vector soundPos;
  C_CSPlayer* player;
  bool no_use;
};

class CSoundEsp : public ISoundEsp {
public:
  void Main( ) override;
private:
  void DrawSoundRingBeam( Vector Origin );
  void UpdateSounds( );
  void RenderSounds( );
  std::map< int, std::vector< SoundInfo_t > > m_Sounds;
};

ISoundEsp * ISoundEsp::Get( ) {
  static CSoundEsp instance;
  return &instance;
}

void CSoundEsp::Main( ) {
  RenderSounds( );
}

void Add3DCircle( const Vector& position, ColorU32 color, float radius ) {
  float precision = 24.0f;
  if( radius >= 60.0f )
	 precision = 48.0f;
  else if( radius >= 30.0f )
	 precision = 36.0f;

  const float step = DirectX::XM_2PI / precision;

  for( float a = 0.f; a < DirectX::XM_2PI; a += step ) {
	 Vector start( radius * cosf( a ) + position.x, radius * sinf( a ) + position.y, position.z );
	 Vector end( radius * cosf( a + step ) + position.x, radius * sinf( a + step ) + position.y, position.z );

	 Vector2D start2d, end2d;
	 if( !WorldToScreen( start, start2d ) || !WorldToScreen( end, end2d ) )
		return;

	 Render::Get( )->AddLine( start2d, end2d, color );
  }
}
void CSoundEsp::DrawSoundRingBeam( Vector Origin ) {
  auto color = g_Vars.esp.sound_esp_color;

  BeamInfo_t beamInfo;
  beamInfo.m_nType = TE_BEAMRINGPOINT;
  beamInfo.m_pszModelName = XorStr( "sprites/purplelaser1.vmt" );
  beamInfo.m_nModelIndex = Source::m_pModelInfo->GetModelIndex( XorStr( "sprites/purplelaser1.vmt" ) );
  beamInfo.m_pszHaloName = XorStr( "sprites/purplelaser1.vmt" );
  beamInfo.m_nHaloIndex = Source::m_pModelInfo->GetModelIndex( XorStr( "sprites/purplelaser1.vmt" ) );
  beamInfo.m_flHaloScale = 5;
  beamInfo.m_flLife = g_Vars.esp.sound_esp_time;
  beamInfo.m_flWidth = 10.f;
  beamInfo.m_flFadeLength = 1.0f;
  beamInfo.m_flAmplitude = 0.f;
  beamInfo.m_flRed = color.r * 255.f;
  beamInfo.m_flGreen = color.g * 255.f;
  beamInfo.m_flBlue = color.b * 255.f;
  beamInfo.m_flBrightness = 255.f;
  beamInfo.m_flSpeed = 10.f;
  beamInfo.m_nStartFrame = 0;
  beamInfo.m_flFrameRate = 60.f;
  beamInfo.m_nSegments = -1;
  beamInfo.m_nFlags = 0;
  beamInfo.m_vecCenter = Origin + Vector( 0, 0, 5 );
  beamInfo.m_vecCenter.z += beamInfo.m_flWidth * 0.5f;
  beamInfo.m_flStartRadius = 1;
  beamInfo.m_flEndRadius = g_Vars.esp.sound_esp_radius;

  auto beam = Source::m_pRenderBeams->CreateBeamRingPoint( beamInfo );

  if( beam )
	 Source::m_pRenderBeams->DrawBeam( beam );
}
void CSoundEsp::UpdateSounds( ) {
  CUtlVector< SndInfo_t > sounds;
  Source::m_pEngineSound->GetActiveSounds( sounds );
  if( sounds.Count( ) < 1 )
	 return;

  C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalPlayer( );

  if( !pLocalPlayer )
	 return;

  Vector eye_pos = pLocalPlayer->GetEyePosition( );
  for( int i = 0; i < sounds.Count( ); ++i ) {
	 SndInfo_t& sound = sounds.Element( i );
	 if( sound.m_nSoundSource < 1 )
		continue;

	 C_CSPlayer* player = C_CSPlayer::GetPlayerByIndex( sound.m_nSoundSource );
	 if( !player )
		continue;

	 if( player->m_hOwnerEntity( ).IsValid( ) && player->IsWeapon( ) ) {
		player = ( C_CSPlayer* )player->m_hOwnerEntity( ).Get( );
	 }

	 if( !player->IsPlayer( ) || player->IsDead( ) )
		continue;

	 if( pLocalPlayer->entindex( ) == player->entindex( ) || ( g_Vars.esp.team_check && pLocalPlayer->IsTeammate( player ) ) )
		continue;

	 auto& player_sound = m_Sounds[player->entindex( )];
	 if( player_sound.size( ) > 0 ) {
		bool should_break = false;
		for( const auto& snd : player_sound ) {
		  if( snd.guid == sound.m_nGuid ) {
			 should_break = true;
			 break;
		  }
		}

		if( should_break )
		  continue;
	 }

	 SoundInfo_t& snd = player_sound.emplace_back( );
	 snd.guid = sound.m_nGuid;
	 snd.soundPos = *sound.m_pOrigin;
	 snd.soundTime = Source::m_pGlobalVars->realtime;
	 snd.alpha = 1.0f;
	 snd.player = player;

	 if( g_Vars.esp.sound_esp_type == 0 )
		DrawSoundRingBeam( *sound.m_pOrigin );
  }
}

void CSoundEsp::RenderSounds( ) {
   UpdateSounds( );

   C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalPlayer( );

   if ( !pLocalPlayer )
	  return;

   for ( auto& [entIndex, sound] : m_Sounds ) {
	  if ( sound.empty( ) )
		 continue;

	  for ( auto& info : sound ) {
		 if ( info.soundTime + g_Vars.esp.sound_esp_time < Source::m_pGlobalVars->realtime )
			info.alpha -= Source::m_pGlobalVars->absoluteframetime * 1.1f;

		 if ( info.alpha <= 0.0f )
			continue;

		 float deltaTime = Source::m_pGlobalVars->realtime - info.soundTime;

		 auto factor = deltaTime / g_Vars.esp.sound_esp_time;
		 if ( factor > 1.0f )
			factor = 1.0f;

		 float radius = g_Vars.esp.sound_esp_radius * factor;
		 auto color = g_Vars.esp.sound_esp_color;
		 color.a = info.alpha;

		 if ( g_Vars.esp.sound_esp_type == 1 )
			Add3DCircle( info.soundPos, FloatColor( color.r, color.g, color.b, color.a ), radius );
	  }


	/*  if ( !sound.front( ).player && !sound.front( ).player->IsDead( ) && !pLocalPlayer->IsDead( ) )
		continue;

	 g_Vars.globals.m_bRenderingDormant[ sound.front( ).player->m_entIndex ] = false;

	 bool dormant = *( bool* ) ( uintptr_t( sound.front( ).player ) + 0xED );*/

	/* if ( !dormant ) {
		Vector2D points[ 8 ];
		Rect2D bbox;

		Vector2D head_pos, feet_pos;

		Vector head = sound.front( ).soundPos + Vector( 0, 0, sound.front( ).player->GetCollideable( )->OBBMaxs( ).z );
		Vector origin = sound.front( ).soundPos;
		origin.z -= 5;

		if ( !WorldToScreen( head, head_pos ) ||
			 !WorldToScreen( origin, feet_pos ) )
		   continue;

		player_info_t info;

		if ( Source::m_pEngine->GetPlayerInfo( sound.front( ).player->m_entIndex, &info ) )
		   continue;

		g_Vars.globals.m_bRenderingDormant[ sound.front( ).player->m_entIndex ] = true;

		auto h = fabs( head_pos.y - feet_pos.y );
		auto w = h / 1.65f;

		bbox.left = static_cast< long >( feet_pos.x - w * 0.5f );
		bbox.right = static_cast< long >( bbox.left + w );
		bbox.bottom = ( feet_pos.y > head_pos.y ? static_cast< long >( feet_pos.y ) : static_cast< long >( head_pos.y ) );
		bbox.top = ( feet_pos.y > head_pos.y ? static_cast< long >( head_pos.y ) : static_cast< long >( feet_pos.y ) );

		{
		   float
			  length_horizontal = ( bbox.right - bbox.left ) * 0.25f,
			  length_vertical = ( bbox.bottom - bbox.top ) * 0.25f;

		   FloatColor outline = FloatColor( 0.0f, 0.0f, 0.0f, g_Vars.esp.box_color.a * 0.68f );

		   auto color = g_Vars.esp.box_color;

		   color.a *= 0.5f;

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

		{
		   auto clr = g_Vars.esp.name_color;
		   clr.a *= 0.5;
		   Render::Get( )->SetTextFont( FONT_VERDANA );
		   Render::Get( )->AddText( Vector2D( bbox.left + ( bbox.right - bbox.left ) * 0.5f, bbox.top - 2.f ),
									clr,
									CENTER_X | ALIGN_BOTTOM | DROP_SHADOW, info.szName );
		}

	 }*/

	 while( !sound.empty( ) ) {
		auto& back = sound.back( );
		if( sound.size( ) > 256 )
		  sound.pop_back( );
		else
		  break;
	 }
  }
}
