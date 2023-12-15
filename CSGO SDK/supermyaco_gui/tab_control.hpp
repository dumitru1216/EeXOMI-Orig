#pragma once
#include "sdk.hpp"
#include "Render.hpp"
#include "object.hpp"
#include "InputSys.hpp"

namespace object
{
  class c_tab_control : public c_container {
  public:
	 c_tab_control( ) {
		// The tab control is always 100px wide.
		// Change this if you think it's too large for your menu.
		m_size.x = 100;

		set_visible( true );
		set_identifier( hash_32_fnv1a_const( "TAB_CONTROLLER" ) );
	 }

	 void update( ) {
		// A tab control must have a parent.
		if( !m_parent )
		  set_visible( false );

		// The tab control is NOT valid, or disabled.
		if( !is_visible( ) )
		  return;

		// 20px from the top, 20px from the bottom.
		m_size.y = m_parent->get_size( ).y - 30;

		m_pos.x = m_parent->get_pos( ).x + 15;
		m_pos.y = m_parent->get_pos( ).y + 15;
	 }

	 void draw( ) {
		// Don't draw the control if it's invisible.
		if( !is_visible( ) )
		  return;

		// Draw background.
		Render::Get( )->AddRectFilled( m_pos, m_pos + m_size, MENU_COLOR( 0.03f, 0.03f, 0.03f ) );

		Render::Get( )->AddRect( m_pos, m_pos + m_size, MENU_COLOR( 0.f, 0.f, 0.f ) );
		Render::Get( )->AddRect( Vector2D( m_pos.x + 1, m_pos.y + 1 ), m_pos + Vector2D( m_size.x - 1, m_size.y - 1 ), MENU_COLOR( 0.176f, 0.176f, 0.176f ) );


		// Draw any children that the tab control has.
		c_container::draw( );
	 }
  };
}
