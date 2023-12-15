  #include "checkbox.hpp"

c_checkbox::c_checkbox( const std::string &label, bool* var ) {
  this->label = std::move( label );
  this->m_bVar = var;
  this->size = Vector2( 7, 3 );
}

void c_checkbox::think( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position;

  Render::Get( )->SetTextFont( FONT_MENU );
  auto text_size = Render::Get( )->CalcTextSize( this->label.c_str( ) );

  if( menu->is_hovered( draw_position, draw_position + Vector2( 7, 7 ) ) && menu->is_key_pressed( VK_LBUTTON ) )
	 *m_bVar ^= 1;
  else if( menu->is_hovered( draw_position + Vector2( 15, -2 ), draw_position + Vector2( 15, -2 ) + Vector2( text_size.x, text_size.y ) ) && menu->is_key_pressed( VK_LBUTTON ) )
	 *m_bVar ^= 1;
}

void c_checkbox::draw( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position;

  Render::Get( )->AddRect( Vector2D( draw_position.x, draw_position.y ), 9.f, 9.f, FloatColor( 17, 17, 17 ) );

  FloatColor colour = FloatColor(
	 g_Vars.menu.main_color.r,
	 g_Vars.menu.main_color.g,
	 g_Vars.menu.main_color.b,
	 g_Vars.menu.main_color.a
  );

  if( *m_bVar )
	 Render::Get( )->AddRectFilledMultiColor( Vector2D( draw_position.x + 1, draw_position.y + 1 ), draw_position + Vector2D( 9.f, 9.f ),
	 colour, colour, colour * 0.44f, colour * 0.44f );
  else
	 Render::Get( )->AddRectFilled( Vector2D( draw_position.x + 1, draw_position.y + 1 ), 7.f, 7.f, FloatColor( 44, 44, 44 ) );

  Render::Get( )->AddText( { draw_position.x + 15.0f, draw_position.y - 2.0f }, FloatColor( 220, 220, 220 ), DROP_SHADOW, this->label.c_str( ) );
}


