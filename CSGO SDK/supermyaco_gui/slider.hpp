#pragma once
#include "sdk.hpp"
#include "Render.hpp"
#include "object.hpp"
#include "InputSys.hpp"

namespace object
{
  template < typename T = int >
  class c_slider : public c_object {
  protected:
	 T	*m_value;
	 T	 m_min;
	 T	 m_max;
	 const char *m_name;
	 bool m_held = false;

	 char m_measure[16]{};

  public:
	 c_slider( const char *name, T min, T max, T *value ) :
		m_name( name ), m_min( min ), m_max( max ), m_value( value ) {
		set_name( name );
		set_visible( true );

		m_size = { 155, 22 };

		set_identifier( hash_32_fnv1a_const( "SLIDER" ) );
	 }

	 void set_measure( const char *measure ) {
		strcpy_s< 16 >( m_measure, measure );
	 }

	 void update( ) {
		if( !m_parent )
		  set_visible( false );

		if( !is_visible( ) )
		  return;

		// Update the position.
		m_pos.x = m_parent->get_pos( ).x + 25;
		m_pos.y = m_parent->get_pos( ).y + 5;

		for( auto &it : m_parent->container( ) ) {
		  if( it == shared_from_this( ) )
			 break;

		  if( it->is_visible( ) ) {
			 m_pos.y += it->get_size( ).y + object_spacing;
		  }
		}

		// Don't affect the object until it's unblocked.
		if( m_blocked )
		  return;

		// Update slider.
		if( m_held ) {
		  set_blocked_r( true );

		  Vector2D mouse = InputSys::Get( )->GetMousePosition( );

		  const float delta = Math::Clamp< float >(
			 mouse.x - m_pos.x, 0, m_size.x
			 );

		  const float ratio = delta / m_size.x;

		  const T new_value = ( T )( m_min + ( m_max - m_min ) * ratio );

		  *m_value = new_value;
		} else {
		  set_blocked_r( false );
		}

		if( InputSys::Get( )->IsInBox( { m_pos.x, m_pos.y + 15.0f }, { m_size.x, m_size.y } ) ) {
		  if( InputSys::Get( )->IsKeyDown( VirtualKeys::LeftButton ) ) {
			 m_held = true;
		  }
		}

		if( !InputSys::Get( )->IsKeyDown( VirtualKeys::LeftButton ) ) {
		  m_held = false;
		}
	 }

	 void draw( ) {
		if( !is_visible( ) )
		  return;

		// The slider name.
		Render::Get( )->SetTextFont( FONT_MENU );
		Render::Get( )->AddText( Vector2D( m_pos.x, m_pos.y ), FloatColor( 215, 215, 215 ), 0, m_name );

		// The slider value.
		bool round = ( T )( 0.1f ) != 0.1f;

		char value[16];

		if( m_measure ) {
		  sprintf_s( value, round ? "%d%s" : "%.2f%s", *m_value, m_measure );
		} else {
		  sprintf_s( value, round ? "%d" : "%.2f", *m_value );
		}
		// The slider itself.				
		auto ratio = ( ( float )( *m_value ) - ( float )( m_min ) ) / float( m_max - m_min );

		g_Vars.menu.main_color.r = Math::Clamp( g_Vars.menu.main_color.r, 0.f, 1.0f );
		g_Vars.menu.main_color.g = Math::Clamp( g_Vars.menu.main_color.g, 0.f, 1.0f );
		g_Vars.menu.main_color.b = Math::Clamp( g_Vars.menu.main_color.b, 0.f, 1.0f );

		Render::Get( )->AddRectFilledMultiColor( Vector2D( m_pos.x, m_pos.y + 16.0f ), m_pos + Vector2D( m_size.x, m_size.y ), MENU_COLOR( 0.27f, 0.27f, 0.27f ), MENU_COLOR( 0.176f, 0.176f, 0.176f ), MENU_COLOR( 0.27f, 0.27f, 0.27f ), MENU_COLOR( 0.176f, 0.176f, 0.176f ) );
		Render::Get( )->AddRectFilledMultiColor( Vector2D( m_pos.x, m_pos.y + 16.0f ), m_pos + Vector2D( m_size.x * ratio, m_size.y ), g_Vars.menu.main_color, g_Vars.menu.main_color, g_Vars.menu.main_color* 0.4f, g_Vars.menu.main_color* 0.4f );

		Render::Get( )->AddRect( Vector2D( m_pos.x, m_pos.y + 16.0f ), m_pos + Vector2D( m_size.x, m_size.y ), MENU_COLOR( 0.f, 0.f, 0.f ) );

		Render::Get( )->SetTextFont( FONT_MENU );
		Render::Get( )->AddText( Vector2D( m_pos.x + m_size.x * ratio - ( Render::Get( )->CalcTextSize( value ).x  * 0.5f ),
		  m_pos.y + m_size.y - 2 ), MENU_COLOR( 0.843f, 0.843f, 0.843f ), 0, value );

	 }
  };

  template < typename T = int >
  class _c_slider : public c_object {
  protected:
	 T	*m_value;
	 T	 m_min;
	 T	 m_max;
	 bool m_held = false;

	 char m_measure[16]{};

  public:
	 _c_slider( T min, T max, T *value ) :
		m_min( min ), m_max( max ), m_value( value ) {
		set_visible( true );

		m_size = { 155, 7 };

		set_identifier( g_fnv_str( "_SLIDER" ) );
	 }

	 void set_measure( const char *measure ) {
		strcpy_s< 16 >( m_measure, measure );
	 }

	 void update( ) {
		if( !m_parent )
		  set_visible( false );

		if( !is_visible( ) )
		  return;

		// Update the position.
		m_pos.x = m_parent->get_pos( ).x + 25;
		m_pos.y = m_parent->get_pos( ).y + 5;

		for( auto &it : m_parent->container( ) ) {
		  if( it == shared_from_this( ) )
			 break;

		  if( it->is_visible( ) ) {
			 m_pos.y += it->get_size( ).y + object_spacing;
		  }
		}

		// Don't affect the object until it's unblocked.
		if( m_blocked )
		  return;

		// Update slider.
		if( m_held ) {
		  set_blocked_r( true );

		  const float delta = std::clamp< float >(
			 g_input.m_cursor.x - m_pos.x, 0, m_size.x
			 );

		  const float ratio = delta / m_size.x;

		  const T new_value = ( T )( m_min + ( m_max - m_min ) * ratio );

		  *m_value = new_value;
		} else {
		  set_blocked_r( false );
		}

		if( g_input.is_in_box( { m_pos.x, m_pos.y }, { m_size.x, m_size.y } ) ) {
		  if( g_input.key_down( VK_LBUTTON ) ) {
			 m_held = true;
		  }
		}

		if( !g_input.key_down( VK_LBUTTON ) ) {
		  m_held = false;
		}
	 }

	 void draw( ) {
		if( !is_visible( ) )
		  return;

		// The slider value.
		bool round = ( T )( 0.1f ) != 0.1f;

		char value[16];

		if( m_measure ) {
		  sprintf_s( value, round ? "%d%s" : "%.2f%s", *m_value, m_measure );
		} else {
		  sprintf_s( value, round ? "%d" : "%.2f", *m_value );
		}
		// The slider itself.				
		auto ratio = ( ( float )( *m_value ) - ( float )( m_min ) ) / float( m_max - m_min );

		std::clamp( ( int )g_config.menu.m_accent.r, 0, 255 );
		std::clamp( ( int )g_config.menu.m_accent.g, 0, 255 );
		std::clamp( ( int )g_config.menu.m_accent.b, 0, 255 );

		render::gradient( m_pos.x, m_pos.y, m_size.x, m_size.y, MENU_COLOR( 69, 69, 69 ), MENU_COLOR( 45, 45, 45 ), true );
		render::gradient( m_pos.x, m_pos.y, m_size.x * ratio, m_size.y,
		  MENU_COLOUR( g_config.menu.m_accent ),
		  MENU_COLOR( g_config.menu.m_accent.r * 0.40f, g_config.menu.m_accent.g * 0.40f, g_config.menu.m_accent.b * 0.40f ), true );
		render::bordered_box( m_pos.x, m_pos.y, m_size.x, m_size.y, MENU_COLOR( 0, 0, 0 ) );

		render::text( render::fonts::menu, m_pos.x + m_size.x * ratio - ( render::get_text_size( value, render::fonts::menu, true ).x / 2 ),
		  m_pos.y + m_size.y - 6, MENU_COLOR( 215, 215, 215 ), render::alignment::left, value );

	 }
  };
}
