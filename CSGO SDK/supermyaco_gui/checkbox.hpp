#pragma once
#include "sdk.hpp"
#include "Render.hpp"
#include "object.hpp"
#include "InputSys.hpp"

namespace object
{
  class c_checkbox : public c_object {
  protected:
	 bool *m_value;

  public:
	 c_checkbox( const char *name, bool *value = nullptr ) :
		m_value( value ) {
		set_name( name );
		set_visible( true );

		m_size = { 10.0f, 10.0f };

		set_identifier( hash_32_fnv1a_const( "CHECKBOX" ) );
	 }

	 void update( ) {
		if( !m_parent )
		  set_visible( false );

		if( !is_visible( ) )
		  return;

		// Update the position.
		m_pos.x = m_parent->get_pos( ).x + 5.0f;
		m_pos.y = m_parent->get_pos( ).y + 5.0f;

		for( auto &it : m_parent->container( ) ) {
		  if( it == shared_from_this( ) )
			 break;

		  if( it->is_visible( ) )
			 m_pos.y += it->get_size( ).y + object_spacing;
		}

		// Don't affect the object until it's unblocked.
		if( m_blocked )
		  return;

		// See if the checkbox was ticked.
		if( InputSys::Get( )->WasKeyPressed( VirtualKeys::LeftButton ) && InputSys::Get( )->IsInBox( m_pos, { 150.0f, m_size.y } ) ) {
		  *m_value ^= 1;
		}
	 }

	 void draw( ) {
		if( !is_visible( ) )
		  return;

		if( *m_value ) {
		  Render::Get( )->AddRectFilledMultiColor( m_pos, m_pos + m_size, g_Vars.menu.main_color, g_Vars.menu.main_color * 0.40f, g_Vars.menu.main_color, g_Vars.menu.main_color * 0.40f );
		} else {
		  Render::Get( )->AddRectFilledMultiColor( m_pos, m_pos + m_size, MENU_COLOR( 0.282f, 0.282f, 0.282f ), MENU_COLOR( 0.176f, 0.176f, 0.176f ),
			 MENU_COLOR( 0.282f, 0.282f, 0.282f ), MENU_COLOR( 0.176f, 0.176f, 0.176f ) );
		}

		Render::Get( )->AddRect( m_pos, m_pos + m_size, MENU_COLOR( 0.f, 0.f, 0.f ) );

		Render::Get( )->SetTextFont( FONT_MENU );
		Render::Get( )->AddText( Vector2D( m_pos.x + 20, m_pos.y - 3 ), MENU_COLOR( 0.843f, 0.843f, 0.843f ), 0, m_name );
	 }
  };
}
