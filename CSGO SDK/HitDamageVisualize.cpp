#include "HitDamageVisualize.hpp"
#include "source.hpp"
#include "Render.hpp"

class CHitDamageVisualize : public IHitDamageVisualize {
public:
  void Main( ) override;
  void AddHitmarker( int damage, C_CSPlayer* m_player ) override;
  void AddHitmarker( bool headshot, C_CSPlayer* m_player ) override;
private:
  struct Hitmarker {
	 Hitmarker( float time, int damage, int player ) {
		m_time = time;
		m_damage = damage;
		m_player = player;
	 }

	 Hitmarker( float time, bool headshot, int player ) {
		headshoted = headshot;
		m_time = time;
		m_player = player;
	 }

	 bool headshoted;
	 float m_time;
	 int m_damage;
	 int m_player;
  };

  std::vector<Hitmarker> m_vecHitmarkes;
  const float damage_fade_time = 2.5f;
  const int height = 70;
};

IHitDamageVisualize * IHitDamageVisualize::Get( ) {
  static CHitDamageVisualize instance;
  return &instance;
}

void CHitDamageVisualize::Main( ) {
  if( !m_vecHitmarkes.size( ) )
	 return;

  auto it = m_vecHitmarkes.begin( );
  while( it != m_vecHitmarkes.end( ) ) {
	 if( fabs( Source::m_pGlobalVars->curtime - it->m_time ) > damage_fade_time
		|| !C_CSPlayer::GetPlayerByIndex( it->m_player ) )
		it = m_vecHitmarkes.erase( it );
	 else
		it++;
  }

  if( !m_vecHitmarkes.size( ) )
	 return;

  for( const auto hitmarker : m_vecHitmarkes ) {
	 player_info_t ent_info;
	 Vector min, max;

	 auto player = C_CSPlayer::GetPlayerByIndex( hitmarker.m_player );
	 if( !player || !player->ComputeHitboxSurroundingBox( min, max ) )
		continue;

	 Vector pos3D = player->m_vecOrigin( ) - Vector( 0.f, 0.f, 80.f );
	 Vector top3D = pos3D + Vector( 0.f, 0.f, max.z /*+ 80.f*/ );

	 Source::m_pEngine->GetPlayerInfo( player->entindex( ), &ent_info );

	 const float time_delta = fabs( Source::m_pGlobalVars->curtime - hitmarker.m_time );
	 const float extra_height = ( time_delta / damage_fade_time ) * height;

	 const int alpha = 255 - ( ( time_delta / damage_fade_time ) * 255.f );
	 const Color color = Color( std::fminf( 255, ( hitmarker.m_damage * 0.01f ) * 255.f ), std::fmaxf( 0, 255 - ( ( hitmarker.m_damage * 0.01f ) * 255.f ) ), 0, alpha );

	 Vector2D pos, top;
	 if( WorldToScreen( pos3D, pos ) && WorldToScreen( top3D, top ) ) {
		int height = ( pos.y - top.y );
		int width = height;

		top.y -= extra_height;
		if( hitmarker.headshoted == true ) {
		  Render::Get( )->SetTextFont( FONT_VERDANA_40_BOLD );
		  Render::Get( )->AddText( Vector2D( pos.x, top.y ), Color( 66, 124, 179, alpha ).GetD3DColor( ), 0, XorStr( "HEADSHOT" ) );
		} else {
		  Render::Get( )->SetTextFont( FONT_VERDANA_40_BOLD );
		  Render::Get( )->AddText( Vector2D( pos.x, top.y ), color.GetD3DColor( ), 0, std::to_string( hitmarker.m_damage ).c_str( ) );
		}
	 }
  }
}

void CHitDamageVisualize::AddHitmarker( int damage, C_CSPlayer * m_player ) {
  m_vecHitmarkes.insert( m_vecHitmarkes.begin( ), Hitmarker( Source::m_pGlobalVars->curtime, damage, m_player->m_entIndex ) );
}

void CHitDamageVisualize::AddHitmarker( bool headshot, C_CSPlayer * m_player ) {
  m_vecHitmarkes.insert( m_vecHitmarkes.begin( ), Hitmarker( Source::m_pGlobalVars->curtime, headshot, m_player->m_entIndex ) );
}
