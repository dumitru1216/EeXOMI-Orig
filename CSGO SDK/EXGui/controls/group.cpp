#include "group.hpp"
#include "../../source.hpp"

Vector2 c_group::get_cursor_pos( ) {
  return this->cursor_pos;
}

void c_group::set_cursor_pos( const Vector2 &pos ) {
  this->cursor_pos = pos;
}

c_group::c_group( const std::string &label, const Vector2 &_size ) {
  this->label = label;
  this->size = _size;
}

void c_group::think( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position;
  bool only_update_blocking = menu->is_blocking( );

  float content_height = this->cursor_pos.y - 16;

  if( menu->get_mouse_wheel( ) != 0 && menu->is_hovered( draw_position, draw_position + size ) ) {
	 if( content_height > this->size.y - 36 ) {
		if( !menu->is_blocking( ) )
		  this->scroll += menu->get_mouse_wheel( ) * 12;

		menu->set_mouse_wheel( 0 );

		if( this->scroll > 0 )
		  this->scroll = 0;
		else if( this->scroll < ( size.y - 36 ) - content_height )
		  this->scroll = ( size.y - 36 ) - content_height;
	 }
  }

  for( c_element *child : this->children ) {
	 if( ( !only_update_blocking && menu->is_hovered( draw_position, draw_position + size ) ) ||
		( only_update_blocking && menu->is_blocking( child ) ) ) {
		child->think( );
	 }
  }
}

void c_group::draw( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position + Vector2( 0, 40 );

  constexpr float fade_factor = 6.666667f;
  float fade_increment = ( fade_factor * Source::m_pGlobalVars->absoluteframetime ) * 255.0f;
  static int menu_alpha = 0;
  if( g_Vars.globals.menuOpen )
	 menu_alpha += ( int )fade_increment;
  else
	 menu_alpha -= ( int )fade_increment;

  menu_alpha = max( 0, min( 255, menu_alpha ) );

  if( menu_alpha == 0 )
	 return;

  // DRAW EBIC SHADOW
  {
	 float shadow_size = 5.0f;
	 const float pad_size = 0.0f;

	 const auto shadow = FloatColor( 24, 24, 24, menu_alpha );
	 auto hidden = FloatColor( 0, 0, 0, 0 );

	 // left
	 Render::Get( )->AddRectFilledMultiColor( draw_position - Vector2D( shadow_size, 0.0f ), draw_position + Vector2D( -pad_size, size.y ),
		hidden, shadow,
		shadow, hidden );

	 // up
	 Render::Get( )->AddRectFilledMultiColor( draw_position - Vector2D( 0.0f, shadow_size ), draw_position + Vector2D( size.x, -pad_size ),
		hidden, hidden,
		shadow, shadow );

	 // right
	 Render::Get( )->AddRectFilledMultiColor( draw_position + Vector2D( size.x + pad_size, 0.0f ), draw_position + Vector2D( size.x + shadow_size, size.y ),
		shadow, hidden,
		hidden, shadow );

	 // bottom
	 Render::Get( )->AddRectFilledMultiColor( draw_position + Vector2D( 0.0f, size.y + pad_size ), draw_position + Vector2D( size.x, size.y + shadow_size ),
		shadow, shadow,
		hidden, hidden );

	 // upper left
	 Render::Get( )->AddRectFilledMultiColor( draw_position - Vector2D( shadow_size, shadow_size ), draw_position - Vector2D( pad_size, pad_size ),
		hidden, hidden,
		shadow, hidden );

	 // upper right
	 Render::Get( )->AddRectFilledMultiColor( draw_position + Vector2D( size.x + shadow_size, -shadow_size ), draw_position + Vector2D( size.x + pad_size, 0.0f ),
		hidden, hidden,
		shadow, hidden );

	 // bottom left
	 Render::Get( )->AddRectFilledMultiColor( draw_position + Vector2D( -shadow_size, size.y + shadow_size ), draw_position + Vector2D( -pad_size, size.y + pad_size ),
		hidden, hidden,
		shadow, hidden );

	 // bottom right
	 Render::Get( )->AddRectFilledMultiColor( draw_position + size + Vector2D( pad_size, pad_size ), draw_position + size + Vector2D( shadow_size, shadow_size ),
		shadow, hidden,
		hidden, hidden );
  }

  Render::Get( )->AddRectFilled( Vector2D( draw_position.x, draw_position.y ), size.x, size.y, FloatColor( 48, 48, 48, menu_alpha ) );
  Render::Get( )->AddRect( Vector2D( draw_position.x - 1.f, draw_position.y - 1.f ), size.x + 2.0f, size.y + 2.0f, FloatColor( 44, 44, 44, menu_alpha ), 2.0f );

  Render::Get( )->SetTextFont( FONT_MENU_BOLD );
  Vector2 text_size = Render::Get( )->CalcTextSize( this->label.c_str( ) );

  //Render::Get( )->AddLine( Vector2D( draw_position.x + 6, draw_position.y - 1 ), Vector2D( draw_position.x + text_size.x + 9, draw_position.y - 1 ), FloatColor( 17, 17, 17, menu_alpha ) );
  // Render::Get( )->AddLine( Vector2D( draw_position.x + 6, draw_position.y - 2 ), Vector2D( draw_position.x + text_size.x + 9, draw_position.y - 2 ), FloatColor( 17, 17, 17, menu_alpha ) );

  Render::Get( )->AddText( { draw_position.x + 8, draw_position.y - 1 - ( text_size.y / 2 ) }, FloatColor( 255, 255, 255 ), DROP_SHADOW,
	 this->label.c_str( ) );

  Render::Get( )->SetScissorsRect( Rect2D( draw_position, draw_position + size ) );

  for( c_element *child : this->children ) {
	 if( !menu->is_blocking( ) || ( menu->is_blocking( ) && !menu->is_blocking( child ) ) )
		child->draw( );
  }
#if 0
  Render::Get( )->AddRectFilledMultiColor( draw_position, size.x, size.y,
	 FloatColor( 48, 48, 48, menu_alpha ), FloatColor( 48, 48, 48, menu_alpha ),
	 FloatColor( 48, 48, 48, 0 ), FloatColor( 48, 48, 48, 0 ) );

  if( this->cursor_pos.y - 16 > this->size.y - 36 )
	 Render::Get( )->AddRectFilledMultiColor( draw_position, size.x, size.y,
	 FloatColor( 48, 48, 48, menu_alpha ), FloatColor( 48, 48, 48, menu_alpha ),
	 FloatColor( 48, 48, 48, 0 ), FloatColor( 48, 48, 48, 0 ) );
#endif
  Render::Get( )->RestoreScissorsRect( );
}

void c_group::add_child( c_element *child ) {
  c_element::add_child( child );

  child->set_position( this->cursor_pos );
  this->cursor_pos.y += child->get_size( ).y + 16;
}

Vector2 c_group::get_child_draw_position( ) {
  return this->get_parent( )->get_child_draw_position( ) + this->position + Vector2( 16.0f, 50.0f + this->scroll );
}

