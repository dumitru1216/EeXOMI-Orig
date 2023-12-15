#include "slider.hpp"

float map_number( float value, float min_old, float max_old, float min_new, float max_new ) {
  return ( value - min_old ) / ( max_old - min_old ) * ( max_new - min_new ) + min_new;
}

c_slider::c_slider( const std::string &label, float* var, float min_value, float max_value, bool round_number, const std::string &display_format ) {
  this->label = std::move( label );
  this->flVar = var;
  this->min_value = min_value;
  this->max_value = max_value;
  this->round = round_number;
  this->display_format = std::move( display_format );
  this->size = Vector2( 180, 14 );
}


void c_slider::think( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position + Vector2( 16, 15 );
  Vector2 slider_size( ( this->get_parent( )->get_size( ) - Vector2( 32, 0 ) ).x - 31.0f + 8 - 30, 5.0f );

  if( menu->is_hovered( draw_position, draw_position + slider_size ) && menu->is_key_pressed( VK_LBUTTON ) )
	 menu->block( this );

  if( menu->is_blocking( this ) && menu->is_key_down( VK_LBUTTON ) ) {
	 float step = slider_size.x / ( this->max_value - this->min_value );
	 float offset = max( this->min_value, min( ( menu->get_mouse_pos( ).x - draw_position.x ), slider_size.x ) );

	 float new_value = map_number( offset, 0, slider_size.x, this->min_value, this->max_value );

	 if( this->round )
		new_value = std::roundf( new_value );

	 *flVar = ( new_value );
  }

  if( menu->is_blocking( this ) && !menu->is_key_down( VK_LBUTTON ) )
	 menu->block( nullptr );

  if( *flVar < this->min_value )
	 *flVar = this->min_value;
}

void c_slider::draw( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position;

  float max_width = ( this->get_parent( )->get_size( ) - Vector2( 32, 0 ) ).x - 31.0f + 8 - 30;
  float slider_width = map_number( *flVar, this->min_value, this->max_value, 0, max_width );

  Render::Get( )->AddRect( Vector2D( draw_position.x + 15, draw_position.y + 14 ), max_width + 2, 7, FloatColor( 17, 17, 17 ) );

  FloatColor colour = FloatColor(
	 g_Vars.menu.main_color.r,
	 g_Vars.menu.main_color.g,
	 g_Vars.menu.main_color.b,
	 g_Vars.menu.main_color.a
  );

  Render::Get( )->AddRectFilledMultiColor( Vector2D( draw_position.x + 16, draw_position.y + 15 ), max_width, 5,
	 FloatColor( 44, 44, 44 ), FloatColor( 44, 44, 44 ), FloatColor( 29, 29, 29 ), FloatColor( 29, 29, 29 ) );
  Render::Get( )->AddRectFilledMultiColor( Vector2D( draw_position.x + 16, draw_position.y + 15 ), slider_width, 5,
	 colour, colour,
	 colour * 0.44f, colour * 0.44f );

 // useless for now
#if 0
  wchar_t buffer[64];
  swprintf( buffer, 64, this->display_format.c_str( ), *flVar );

  if( config->get_float( this->var_name ) != this->max_value )
	 render::get( ).draw_text( draw_position.x + this->get_size( ).x - 35, draw_position.y + this->get_size( ).y - 1, render::get( ).menu_font_small, "+", false, color( 200, 200, 200 ) );

  if( config->get_float( this->var_name ) != this->min_value )
	 render::get( ).draw_text( draw_position.x + 10, draw_position.y + this->get_size( ).y - 1, render::get( ).menu_font_small, "-", false, color( 200, 200, 200 ) );

  std::wstring to_convert( buffer );
  std::string str( to_convert.begin( ), to_convert.end( ) );
#endif


  Render::Get( )->SetTextFont( FONT_MENU );
  Render::Get( )->AddText( { draw_position.x + 15, draw_position.y - 3 }, FloatColor( 220, 220, 220 ), DROP_SHADOW, this->label.c_str( ) );
  Render::Get( )->AddText( { draw_position.x + 15 + slider_width - 2, draw_position.y + 14 }, FloatColor( 220, 220, 220 ), DROP_SHADOW, this->display_format.c_str( ), *flVar );
}

c_slider_int::c_slider_int( const std::string &label, int* var, float min_value, float max_value, bool round_number, const std::string &display_format ) {
  this->label = std::move( label );
  this->iVar = var;
  this->min_value = min_value;
  this->max_value = max_value;
  this->round = round_number;
  this->display_format = std::move( display_format );
  this->size = Vector2( 180, 14 );
}


void c_slider_int::think( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position + Vector2( 16, 15 );
  Vector2 slider_size( ( this->get_parent( )->get_size( ) - Vector2( 32, 0 ) ).x - 31.0f + 8 - 30, 5.0f );

  if( menu->is_hovered( draw_position, draw_position + slider_size ) && menu->is_key_pressed( VK_LBUTTON ) )
	 menu->block( this );

  if( menu->is_blocking( this ) && menu->is_key_down( VK_LBUTTON ) ) {
	 float step = slider_size.x / ( this->max_value - this->min_value );
	 float offset = max( this->min_value, min( ( menu->get_mouse_pos( ).x - draw_position.x ), slider_size.x ) );

	 float new_value = map_number( offset, 0, slider_size.x, this->min_value, this->max_value );

	 if( this->round )
		new_value = std::roundf( new_value );

	 *iVar = ( new_value );
  }

  if( menu->is_blocking( this ) && !menu->is_key_down( VK_LBUTTON ) )
	 menu->block( nullptr );

  if( *iVar < this->min_value )
	 *iVar = this->min_value;
}

void c_slider_int::draw( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position;

  float max_width = ( this->get_parent( )->get_size( ) - Vector2( 32, 0 ) ).x - 31.0f + 8 - 30;
  float slider_width = map_number( *iVar, this->min_value, this->max_value, 0, max_width );

  Render::Get( )->AddRect( Vector2D( draw_position.x + 15, draw_position.y + 14 ), max_width + 2, 7, FloatColor( 17, 17, 17 ) );

  FloatColor colour = FloatColor(
	 g_Vars.menu.main_color.r,
	 g_Vars.menu.main_color.g,
	 g_Vars.menu.main_color.b,
	 g_Vars.menu.main_color.a
  );

  Render::Get( )->AddRectFilledMultiColor( Vector2D( draw_position.x + 16, draw_position.y + 16 ), max_width, 6,
	 FloatColor( 44, 44, 44 ), FloatColor( 44, 44, 44 ),
	 FloatColor( 29, 29, 29 ), FloatColor( 29, 29, 29 ) );
  Render::Get( )->AddRectFilledMultiColor( Vector2D( draw_position.x + 16, draw_position.y + 14 ), slider_width, 6,
	 colour, colour,
	 colour * 0.44f, colour * 0.44f );

  // useless for now
#if 0
  wchar_t buffer[64];
  swprintf( buffer, 64, this->display_format.c_str( ), *flVar );

  if( config->get_float( this->var_name ) != this->max_value )
	 render::get( ).draw_text( draw_position.x + this->get_size( ).x - 35, draw_position.y + this->get_size( ).y - 1, render::get( ).menu_font_small, "+", false, color( 200, 200, 200 ) );

  if( config->get_float( this->var_name ) != this->min_value )
	 render::get( ).draw_text( draw_position.x + 10, draw_position.y + this->get_size( ).y - 1, render::get( ).menu_font_small, "-", false, color( 200, 200, 200 ) );

  std::wstring to_convert( buffer );
  std::string str( to_convert.begin( ), to_convert.end( ) );
#endif


  Render::Get( )->SetTextFont( FONT_MENU );
  Render::Get( )->AddText( { draw_position.x + 15, draw_position.y - 3 }, FloatColor( 220, 220, 220 ), DROP_SHADOW, this->label.c_str( ) );
  Render::Get( )->AddText( { draw_position.x + 15 + slider_width - 2, draw_position.y + 14 }, FloatColor( 220, 220, 220 ), DROP_SHADOW, this->display_format.c_str( ), *iVar );
}
