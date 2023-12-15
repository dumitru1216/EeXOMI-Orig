#pragma once
#include "sdk.hpp"
#include "Render.hpp"
#include "object.hpp"
#include "InputSys.hpp"

namespace object
{
  class c_button : public c_object {
  protected:
	 std::function< void( ) > m_function;
	 bool m_clicked = false;
  public:
	 c_button( const char *name, std::function< void( ) > callback ) {
		m_function = callback;
		set_name( name );
		set_visible( true );

		m_size = { 155.0f, 20.0f };

		set_identifier( hash_32_fnv1a_const( "BUTTON" ) );
	 }

	 void update( ) {
		if( !m_parent )
		  set_visible( false );

		if( !is_visible( ) )
		  return;

		// Update the position.
		m_pos.x = m_parent->get_pos( ).x + 30.0f;
		m_pos.y = m_parent->get_pos( ).y + 10.0f;

		for( auto &it : m_parent->container( ) ) {
		  if( it == shared_from_this( ) )
			 break;

		  if( it->is_visible( ) )
			 m_pos.y += it->get_size( ).y + object_spacing;
		}

		// Don't affect the object until it's unblocked.
		if( m_blocked )
		  return;

		// See if the button was clicked.
		if( InputSys::Get( )->IsKeyDown( VK_LBUTTON ) ) {
		  if( InputSys::Get( )->IsInBox( m_pos, m_size ) && !m_clicked ) {
			 m_clicked = true;
			 m_function( );
		  }
		} else {
		  m_clicked = false;
		}
	 }

	 void draw( ) {
		if( !is_visible( ) )
		  return;

		// Draw the button.
		Render::Get( )->AddRectFilledMultiColor( m_pos, m_pos + m_size, MENU_COLOR( 0.117f, 0.117f, 0.117f ), MENU_COLOR( 0.176f, 0.176f, 0.176f ),
		  MENU_COLOR( 0.117f, 0.117f, 0.117f ), MENU_COLOR( 0.176f, 0.176f, 0.176f ) );
		Render::Get( )->AddRectFilled( m_pos, m_pos + m_size, MENU_COLOR( 0.f, 0.f, 0.f ) );

		// Draw the button name.
		Render::Get( )->SetTextFont( FONT_MENU );
		Render::Get( )->AddText( Vector2D( m_pos.x + 10, m_pos.y + 4 ), MENU_COLOR( 0.843f, 0.843f, 0.843f ), 0, m_name );
	 }
  };
}
