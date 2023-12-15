#include "button.hpp"

c_button::c_button( const std::string &label, std::function< void( ) > callback, const Vector2 &size ) {
  this->label = std::move( label );
  this->active = false;
  this->callback = callback;
  this->size = size - Vector2( 0, 4 );
}

void c_button::think( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position + Vector2( 16, 0 );
  Vector2 button_size( size + Vector2( 0, 4 ) );

  Vector2 item_min( draw_position );
  Vector2 item_max( item_min + Vector2( button_size.x, button_size.y ) - Vector2( 0, 1 ) );

  if( menu->is_hovered( item_min, item_max ) && menu->is_key_pressed( VK_LBUTTON ) && !active ) {
	 menu->block( this );
	 active = true;
  } else if( menu->is_hovered( item_min, item_max ) && menu->is_key_released( VK_LBUTTON ) && active ) {
	 callback( );
	 menu->block( nullptr );
	 active = false;
  } else if( !menu->is_hovered( item_min, item_max ) && menu->is_key_released( VK_LBUTTON ) && active ) {
	 menu->block( nullptr );
	 active = false;
  }
}

void c_button::draw( ) {
  if( size.x == 0 ) {
	 float group_width = this->get_parent( )->get_size( ).x - 61.0f + 8 - 30;
	 size.x = group_width - 2;
  }

  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position + Vector2( 16, 0 );
  Vector2 button_size( size + Vector2( 0, 4 ) );

  Vector2 item_min( draw_position );
  Vector2 item_max( item_min + Vector2( button_size.x, button_size.y ) - Vector2( 0, 1 ) );

  Render::Get( )->AddRect( Vector2D( item_min.x - 1, item_min.y - 1 ), button_size.x + 2, button_size.y + 2, FloatColor( 17, 17, 17 ) );

  if( active && menu->is_hovered( item_min, item_max ) )
	 Render::Get( )->AddRectFilledMultiColor( item_min.x, item_min.y, button_size.x, button_size.y, FloatColor( 30, 30, 30 ), FloatColor( 15, 15, 15 ) );
  else if( menu->is_hovered( item_min, item_max ) )
	 Render::Get( )->AddRectFilledMultiColor( Vector2D( item_min.x, item_min.y ), button_size.x, button_size.y,
	 FloatColor( 40, 40, 40 ), FloatColor( 40, 40, 40 ), FloatColor( 30, 30, 30 ), FloatColor( 30, 30, 30 ) );
  else
	 Render::Get( )->AddRectFilledMultiColor( Vector2D( item_min.x, item_min.y ),
	 button_size.x, button_size.y, FloatColor( 35, 35, 35 ), FloatColor( 35, 35, 35 ), FloatColor( 25, 25, 25 ), FloatColor( 25, 25, 25 ) );

  Render::Get( )->SetTextFont( FONT_MENU_BOLD );
  Vector2 label_size = Render::Get( )->CalcTextSize( get_label( ).c_str( ) );
  Render::Get( )->AddText( item_min + ( button_size / 2 ) - ( label_size / 2 ), FloatColor( 255, 255, 255 ),
	 DROP_SHADOW, get_label( ).c_str( ) );
}
