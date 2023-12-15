#pragma once

namespace object
{
  class c_form : public c_container {
  protected:
	 bool	m_dragging;
	 Vector2D	m_drag;

  public:
	 c_form( float x, float y, float w, float h ) {
		m_pos = { x, y };
		m_size = { w, h };

		set_visible( true );
		set_identifier( hash_32_fnv1a_const( "FORM" ) );
	 }

	 void update( ) {
		if( !m_parent )
		  set_visible( false );

		if( !is_visible( ) )
		  return;

		Vector2D titlebar{ m_size.x, 100.0f };

		// Form has been released.
		if( m_dragging && !InputSys::Get( )->IsKeyDown( VirtualKeys::LeftButton ) )
		  m_dragging = false;

		Vector2D mouse = InputSys::Get( )->GetMousePosition( );

		// Update form position.
		if( m_dragging ) {
		  m_pos.x = mouse.x - m_drag.x;
		  m_pos.y = mouse.y - m_drag.y;
		}

		// Check if we are dragging the form.
		if( InputSys::Get( )->IsKeyDown( VirtualKeys::LeftButton )
		  && InputSys::Get( )->IsInBox( m_pos, titlebar ) ) {
		  m_dragging = true;
		  m_drag.x = mouse.x - m_pos.x;
		  m_drag.y = mouse.y - m_pos.y;
		}
	 }

	 void draw( ) {
		if( !is_visible( ) )
		  return;

		// lambda to handle outlines
		auto outline = [&] ( float offset, FloatColor color ) {
		  auto box = [] ( float x, float y, float w, float h, FloatColor c ) {
			 Render::Get( )->AddLine( Vector2D( x, y ), Vector2D( x, y + h ), c );
			 Render::Get( )->AddLine( Vector2D( x, y + h ), Vector2D( x + w + 1.f, y + h ), c );
			 Render::Get( )->AddLine( Vector2D( x + w, y ), Vector2D( x + w, y + h ), c );
			 Render::Get( )->AddLine( Vector2D( x + 1.f, y ), Vector2D( x + w, y ), c );
		  };

		  box( m_pos.x - offset, m_pos.y - offset, m_size.x + offset * 2.f, m_size.y + offset * 2.f, color );
		};


		// Draw the form background.
		Render::Get( )->AddRectFilled( m_pos, m_pos + m_size, MENU_COLOR( 0.015f, 0.015f, 0.015f ) );

		// Draw the form borders.
		outline( 0, MENU_COLOR( 0.223f, 0.223f, 0.223f ) );
		outline( 1, MENU_COLOR( 0.137f, 0.137f, 0.137f ) );
		outline( 2, MENU_COLOR( 0.137f, 0.137f, 0.137f ) );
		outline( 3, MENU_COLOR( 0.137f, 0.137f, 0.137f ) );
		outline( 4, MENU_COLOR( 0.223f, 0.223f, 0.223f ) );
		outline( 5, MENU_COLOR( 0.003f, 0.003f, 0.003f ) );

		Render::Get( )->AddRectFilled( Vector2D( m_pos.x + 2.f, m_pos.y + 2.f ), m_pos.x + m_size.x - 3.f, m_pos.y + 1.f, g_Vars.menu.main_color );
		Render::Get( )->AddRectFilled( Vector2D( m_pos.x + 2.f, m_pos.y + 3.f ), m_pos.x + m_size.x - 3.f, m_pos.y + 1.f,
		  MENU_COLOR(
		  g_Vars.menu.main_color.r * 0.40f,
		  g_Vars.menu.main_color.g * 0.40f,
		  g_Vars.menu.main_color.b * 0.40f ) );


		// Draw container children.
		c_container::draw( );
	 }
  };
}
