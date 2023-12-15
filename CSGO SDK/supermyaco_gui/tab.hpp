#pragma once
#include "sdk.hpp"
#include "Render.hpp"
#include "object.hpp"
#include "InputSys.hpp"

namespace object
{
// Actual tab control.
  class c_tab : public c_container {
  protected:
	 // Index of the current tab.
	 size_t m_index;
  public:
	 c_tab( const char *name ) {
		m_size = { 80, 15 };

		// Set up control variables.
		set_name( name );
		set_visible( true );
		set_identifier( hash_32_fnv1a_const( "TAB" ) );

		// Yeah, I was too lazy to add operators to this.
		// Suck my cock.
		m_index = g_Vars.menu.m_count;
		g_Vars.menu.m_count = g_Vars.menu.m_count + 1;
	 }

	 void update( ) {
		// A tab must have a parent.
		if( !m_parent )
		  set_visible( false );

		// The tab is NOT valid, or disabled.
		if( !is_visible( ) )
		  return;

		// Bandaid fix, I guess.
		size_t ignore_count = 0;

		for( auto &it : m_parent->container( ) ) {
		  if( it == shared_from_this( ) )
			 break;

		  if( !it->is_visible( ) )
			 ++ignore_count;
		}

		// Position the tab.
		m_pos.x = m_parent->get_pos( ).x + 10;
		m_pos.y = m_parent->get_pos( ).y + 5 + ( 16 * ( m_index - ignore_count ) );

		if( InputSys::Get( )->IsKeyDown( VK_LBUTTON ) && InputSys::Get( )->IsInBox( m_pos, m_size ) ) {
		  g_Vars.menu.m_selected = m_index;
		}
	 }

	 void draw( ) {
		// Don't draw the control if it's invisible.
		if( !is_visible( ) )
		  return;

		// Draw tab text (selectable)
		auto text_clr = FloatColor( 145, 145, 145 );
		bool selected = m_index == g_Vars.menu.m_selected;

		if( selected )
		  text_clr = g_Vars.menu.main_color;

		Render::Get( )->SetTextFont( FONT_MENU );
		Render::Get( )->AddText( m_pos, Color( 215, 215, 215 ).GetD3DColor( ), 0, m_name );

		// Draw tab container
		if( selected ) {
		  c_container::draw( );
		}
	 }
  };
}
