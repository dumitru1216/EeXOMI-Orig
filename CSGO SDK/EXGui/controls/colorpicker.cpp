#include <cctype>
#include <iomanip>
#include <sstream>
#include <windows.h>

#include "Colorpicker.hpp"

struct c_rgb { double r, g, b; }; // a fraction between 0 and 1
struct c_hsv { double h, s, v; }; // a fraction between 0 and 1, but hue is in degrees (0 to 360)

c_hsv rgb_to_hsv( const c_rgb &in ) {
  c_hsv result;
  double min, max, delta;

  min = in.r < in.g ? in.r : in.g;
  min = min < in.b ? min : in.b;

  max = in.r > in.g ? in.r : in.g;
  max = max > in.b ? max : in.b;

  result.v = max;
  delta = max - min;

  if( delta < 0.0001 ) {
	 result.s = 0;
	 result.h = 0;

	 return result;
  }

  if( max > 0 ) {
	 result.s = ( delta / max );
  } else {
	 result.s = 0;
	 result.h = NAN;

	 return result;
  }

  if( in.r >= max ) {
	 result.h = ( in.g - in.b ) / delta;
  } else {
	 if( in.g >= max ) {
		result.h = 2 + ( in.b - in.r ) / delta;
	 } else {
		result.h = 4 + ( in.r - in.g ) / delta;
	 }
  }

  result.h *= 60;

  if( result.h < 0 )
	 result.h += 360;

  return result;
}
c_rgb hsv_to_rgb( const c_hsv &in ) {
  c_rgb result;

  double hh, p, q, t, ff;
  long i;

  if( in.s <= 0 ) {
	 result.r = in.v;
	 result.g = in.v;
	 result.b = in.v;

	 return result;
  }

  hh = ( in.h >= 360 ? 0 : in.h ) / 60;
  i = ( long )hh;

  ff = hh - i;

  p = in.v * ( 1 - in.s );
  q = in.v * ( 1 - ( in.s * ff ) );
  t = in.v * ( 1 - ( in.s * ( 1 - ff ) ) );

  switch( i ) {
  case 0:
	 result.r = in.v;
	 result.g = t;
	 result.b = p;

	 break;

  case 1:
	 result.r = q;
	 result.g = in.v;
	 result.b = p;

	 break;

  case 2:
	 result.r = p;
	 result.g = in.v;
	 result.b = t;

	 break;

  case 3:
	 result.r = p;
	 result.g = q;
	 result.b = in.v;

	 break;

  case 4:
	 result.r = t;
	 result.g = p;
	 result.b = in.v;

	 break;

  case 5:
  default:
	 result.r = in.v;
	 result.g = p;
	 result.b = q;

	 break;

  }

  return result;
}

static FloatColor
hue_bar_Colors[7] = {
	FloatColor( 255, 0, 0, 255 ),
	FloatColor( 255, 255, 0, 255 ),
	FloatColor( 0, 255, 0, 255 ),
	FloatColor( 0, 255, 255, 255 ),
	FloatColor( 0, 0, 255, 255 ),
	FloatColor( 255, 0, 255, 255 ),
	FloatColor( 255, 0, 0, 255 )
};

void c_colorpicker::context_menu_copy( c_colorpicker *self ) {
  FloatColor col = *self->m_varClr;
  std::stringstream Color_hex;

  Color_hex << "#";
  Color_hex << std::hex << std::setw( 2 ) << std::setfill( '0' ) << std::uppercase << col.r;
  Color_hex << std::hex << std::setw( 2 ) << std::setfill( '0' ) << std::uppercase << col.g;
  Color_hex << std::hex << std::setw( 2 ) << std::setfill( '0' ) << std::uppercase << col.b;

  if( self->alpha_bar )
	 Color_hex << std::hex << std::setw( 2 ) << std::setfill( '0' ) << std::uppercase << col.a;

  if( OpenClipboard( nullptr ) ) {
	 EmptyClipboard( );

	 HGLOBAL clipboard_buffer = GlobalAlloc( GMEM_DDESHARE, Color_hex.str( ).size( ) + 1 );
	 char *buffer = ( char* )GlobalLock( clipboard_buffer );
	 strcpy( buffer, Color_hex.str( ).c_str( ) );

	 GlobalUnlock( clipboard_buffer );
	 SetClipboardData( CF_TEXT, clipboard_buffer );
	 CloseClipboard( );
  }
}

void c_colorpicker::context_menu_paste( c_colorpicker *self ) {
  if( OpenClipboard( nullptr ) ) {
	 std::string input( ( char* )GetClipboardData( CF_TEXT ) );

	 input.erase( input.begin( ), std::find_if( input.begin( ), input.end( ), [] ( int ch ) {
		return !std::isspace( ch );
	 } ) );
	 input.erase( std::find_if( input.rbegin( ), input.rend( ), [] ( int ch ) {
		return !std::isspace( ch );
	 } ).base( ), input.end( ) );

	 if( input.at( 0 ) != '#' )
		return;

	 int component_r = std::stoi( input.substr( 1, 2 ), 0, 16 );
	 int component_g = std::stoi( input.substr( 3, 2 ), 0, 16 );
	 int component_b = std::stoi( input.substr( 5, 2 ), 0, 16 );
	 int component_a = input.size( ) > 7 ? std::stoi( input.substr( 7, 2 ), 0, 16 ) : 255;

	 *self->m_varClr = FloatColor( component_r, component_g, component_b, component_a );

	 c_hsv Color_hsv = rgb_to_hsv( {
		 ( double )( ( float )component_r / 255.f ),
		 ( double )( ( float )component_g / 255.f ),
		 ( double )( ( float )component_b / 255.f )
		} );

	 self->hue = ( float )Color_hsv.h / 360.f;
	 self->cursor_pos.x = ( float )Color_hsv.s * 150.f;
	 self->cursor_pos.y = 150.f - ( ( float )Color_hsv.v * 150.f );
	 self->alpha = ( float )component_a / 255.f;

	 c_colorpicker::context_menu_copy( self );
  }
}

c_colorpicker::c_colorpicker( const std::string &label, FloatColor* var_clr, bool is_inline, bool alpha_bar ) {
  this->label = std::move( label );
  this->m_varClr = std::move( m_varClr );
  this->is_inline = is_inline;
  this->alpha_bar = alpha_bar;

  c_hsv Color_hsv = rgb_to_hsv( {
	  ( double )( ( float )m_varClr->r ),
	  ( double )( ( float )m_varClr->g ),
	  ( double )( ( float )m_varClr->b )
	 } );

  this->hue = ( float )Color_hsv.h / 360.f;
  this->cursor_pos.x = ( float )Color_hsv.s * 150.f;
  this->cursor_pos.y = 150.f - ( ( float )Color_hsv.v * 150.f );

  if( is_inline )
	 this->size = Vector2( 0, -16 );
  else
	 this->size = Vector2( 150, 27 );

  if( alpha_bar )
	 this->alpha = ( float )m_varClr->a;
  else
	 this->alpha = 1.0f;

  this->context_menu_actions.push_back( { "copy", &c_colorpicker::context_menu_copy } );
  this->context_menu_actions.push_back( { "paste", &c_colorpicker::context_menu_paste } );

}

void c_colorpicker::think( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position;
  float group_width = this->get_parent( )->get_size( ).x - 46.f + 8;

  Vector2 picker_pos( draw_position + Vector2( group_width - 21.f, 12.f ) );
  Vector2 picker_size( 174.f, this->alpha_bar ? 173.f : 158.f );

  if( menu->is_hovered( draw_position + Vector2( group_width - 20, 0 ), draw_position + Vector2( group_width, 8 ) ) && !menu->is_blocking( ) &&
	 !menu->is_blocking( this ) && !this->context_menu_open && menu->is_key_pressed( VK_LBUTTON ) ) {
	 menu->block( this );
  } else if( this->context_menu_open ) {
	 if( !menu->is_hovered( context_menu_pos, context_menu_pos + Vector2( 100.f, ( float )context_menu_actions.size( ) * 18.f ) ) &&
		menu->is_blocking( this ) && menu->is_key_pressed( VK_LBUTTON ) ) {
		this->context_menu_open = false;
		menu->block( nullptr );
	 }
  } else if( !menu->is_hovered( picker_pos, picker_pos + picker_size ) && menu->is_blocking( this ) && menu->is_key_pressed( VK_LBUTTON ) ) {
	 this->context_menu_open = false;
	 menu->block( nullptr );
  }

  if( menu->is_blocking( this ) && !context_menu_open && !changing_alpha && !changing_color && !changing_hue ) {
	 if( menu->is_key_down( VK_CONTROL ) && menu->is_key_pressed( 'C' ) )
		c_colorpicker::context_menu_copy( this );
	 else if( menu->is_key_down( VK_CONTROL ) && menu->is_key_pressed( 'V' ) )
		c_colorpicker::context_menu_paste( this );
  }

  if( menu->is_hovered( picker_pos, picker_pos + picker_size ) && menu->is_blocking( this ) && menu->is_key_pressed( VK_LBUTTON ) && !this->context_menu_open ) {
	 if( !this->changing_alpha && !this->changing_hue && !this->changing_color ) {
		if( menu->is_hovered( picker_pos + Vector2( 2, 2 ), picker_pos + Vector2( 152, 152 ) ) ) {
		  this->changing_color = true;
		} else if( menu->is_hovered( picker_pos + Vector2( 161.f, 6.f ), picker_pos + Vector2( 172.f, 156.f ) ) ) {
		  this->changing_hue = true;
		} else if( menu->is_hovered( picker_pos + Vector2( 6.f, 160.f ), picker_pos + Vector2( 156.f, 171.f ) ) && this->alpha_bar ) {
		  this->changing_alpha = true;
		}
	 }
  } else if( this->context_menu_open ) {
	 if( menu->is_hovered( context_menu_pos, context_menu_pos + Vector2( 100.f, context_menu_actions.size( ) * 18.f ) ) &&
		menu->is_blocking( this ) && menu->is_key_pressed( VK_LBUTTON ) ) {
		for( int i = 0; i < ( int )context_menu_actions.size( ); i++ ) {
		  Vector2 context_menu_action_pos_min( context_menu_pos + Vector2( 1.f, 1.f + ( 18.f * ( float )i ) ) );
		  Vector2 context_menu_action_pos_max( context_menu_action_pos_min + Vector2( 98.f, 17.f ) );

		  if( menu->is_hovered( context_menu_action_pos_min, context_menu_action_pos_max ) ) {
			 context_menu_actions[i].second( this );

			 this->context_menu_open = false;
			 menu->block( nullptr );
		  }
		}
	 }
  }

  if( menu->is_hovered( draw_position + Vector2( group_width - 20, 0 ), draw_position + Vector2( group_width, 8.f ) ) && !menu->is_blocking( ) &&
	 !menu->is_blocking( this ) && !this->context_menu_open && menu->is_key_pressed( VK_RBUTTON ) ) {
	 this->context_menu_open = true;
	 this->context_menu_pos = picker_pos + Vector2( -100 + 21, 2 );

	 menu->block( this );
  }

  if( menu->is_blocking( this ) && !this->context_menu_open && ( this->changing_alpha || this->changing_color || this->changing_hue ) ) {
	 if( !menu->is_key_down( VK_LBUTTON ) ) {
		this->changing_alpha = false;
		this->changing_color = false;
		this->changing_hue = false;
	 } else {
		if( this->alpha_bar && this->changing_alpha ) {
		  this->alpha = ( menu->get_mouse_pos( ).x - ( picker_pos.x + 6.f ) ) / 150.f;

		  if( this->alpha < 0 ) this->alpha = 0;
		  if( this->alpha > 1 ) this->alpha = 1;
		} else if( this->changing_color ) {
		  this->cursor_pos.x = menu->get_mouse_pos( ).x - ( picker_pos.x + 6.f );
		  this->cursor_pos.y = menu->get_mouse_pos( ).y - ( picker_pos.y + 6.f );

		  if( this->cursor_pos.x < 0.f ) this->cursor_pos.x = 0.f;
		  if( this->cursor_pos.x > 150.f ) this->cursor_pos.x = 150.f;

		  if( this->cursor_pos.y < 0.f ) this->cursor_pos.y = 0.f;
		  if( this->cursor_pos.y > 150.f ) this->cursor_pos.y = 150.f;
		} else if( this->changing_hue ) {
		  this->hue = ( menu->get_mouse_pos( ).y - ( picker_pos.y + 6.f ) ) / 150.f;

		  if( this->hue < 0.f ) this->hue = 0.f;
		  if( this->hue > 1.f ) this->hue = 1.f;
		}

		float saturation = this->cursor_pos.x / 150.f;
		float brightness = 1.f - ( this->cursor_pos.y / 150.f );

		c_rgb new_Color = hsv_to_rgb( { this->hue * 360.f, saturation, brightness } );

		*m_varClr = FloatColor(
		  ( float )( new_Color.r ),
		  ( float )( new_Color.g ),
		  ( float )( new_Color.b ),
		  ( float )( this->alpha )
		);
	 }
  }
}

void c_colorpicker::draw( ) {
  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position;
  float group_width = this->get_parent( )->get_size( ).x - 46.f + 8;

  Vector2 picker_pos( draw_position + Vector2( group_width - 21.f, 12.f ) );
  Vector2 picker_size( 174.f, this->alpha_bar ? 173.f : 158.f );
  FloatColor preview = *m_varClr;
  preview.a = 0.0f;

  Render::Get( )->AddRectFilled( Vector2D( draw_position.x + group_width - 20, draw_position.y ), 20, 10, FloatColor( 17, 17, 17 ) );
  Render::Get( )->AddRectFilledMultiColor( Vector2D( draw_position.x + group_width - 19, draw_position.y + 1 ), 18, 8, preview, preview,
	 FloatColor( preview * 0.65f ), FloatColor( preview * 0.65f ) );

  Render::Get( )->SetTextFont( FONT_MENU );
  if( !this->is_inline )
	 Render::Get( )->AddText( Vector2D( draw_position.x + 15, draw_position.y - 3 ), FloatColor( 220, 220, 220 ),
	 DROP_SHADOW, this->label.c_str( ) );

  if( menu->is_blocking( this ) && !this->context_menu_open ) {
	 FloatColor picked_Color = *m_varClr;
	 c_rgb hue_Color = hsv_to_rgb( { ( double )this->hue * 360.f, 1.f, 1.f } );

	 Render::Get( )->AddRect( Vector2D( picker_pos.x, picker_pos.y ), picker_size.x + 4, picker_size.y + 4, FloatColor( 17, 17, 17 ) );
	 Render::Get( )->AddRect( Vector2D( picker_pos.x + 1, picker_pos.y + 1 ), picker_size.x + 2, picker_size.y + 2, FloatColor( 60, 60, 60 ) );
	 Render::Get( )->AddRectFilled( Vector2D( picker_pos.x + 2, picker_pos.y + 2 ), picker_size.x, picker_size.y, FloatColor( 44, 44, 44 ) );

	 Render::Get( )->AddRect( picker_pos.x + 5, picker_pos.y + 5, 152, 152, FloatColor( 0, 0, 0 ) );
	 Render::Get( )->AddRectFilledMultiColor( Vector2D( picker_pos.x + 6, picker_pos.y + 6 ), 150, 150,
		FloatColor( ( float )( hue_Color.r ), ( float )( hue_Color.g ), ( float )( hue_Color.b ) ),
		FloatColor( 255, 255, 255 ),
		FloatColor( ( float )( hue_Color.r ), ( float )( hue_Color.g ), ( float )( hue_Color.b ) ),
		FloatColor( 255, 255, 255 ) );

	 Render::Get( )->AddRect( picker_pos.x + 160, picker_pos.y + 5, 13, 152, FloatColor( 0, 0, 0 ) );

	 for( int i = 0; i < 6; ++i )
		Render::Get( )->AddRectFilledMultiColor( Vector2D( picker_pos.x + 161, picker_pos.y + 6 + ( 25 * i ) ), 11, 25,
		hue_bar_Colors[i], hue_bar_Colors[i], hue_bar_Colors[i + 1], hue_bar_Colors[i + 1] );

	 Render::Get( )->AddRect( picker_pos.x + 5 + float( ( 148.f * ( this->cursor_pos.x / 150.0f ) ) ),
		picker_pos.y + 5 + float( ( 148.f * ( this->cursor_pos.y / 150.f ) ) ), 4, 4, FloatColor( 0, 0, 0 ) );

	 if( this->alpha_bar ) {
		Render::Get( )->AddRect( picker_pos.x + 5, picker_pos.y + 159, 152, 13, FloatColor( 0, 0, 0 ) );
		Render::Get( )->AddRectFilled( picker_pos.x + 6, picker_pos.y + 160, 150, 11, FloatColor( picked_Color.r, picked_Color.g, picked_Color.b, picked_Color.a ) );

		Render::Get( )->AddRect( picker_pos.x + 6 + float( ( 146.f * this->alpha ) ), picker_pos.y + 160, 4, 11, FloatColor( 0, 0, 0 ) );
	 }

	 Render::Get( )->AddRect( picker_pos.x + 161, float( picker_pos.y + 6 + ( 146.f * this->hue ) ), 11, 4, FloatColor( 0, 0, 0 ) );
  } else if( menu->is_blocking( this ) && this->context_menu_open ) {
	 Render::Get( )->AddRect( context_menu_pos.x, context_menu_pos.y, 100, 20 * context_menu_actions.size( ), FloatColor( 17, 17, 17 ) );
	 Render::Get( )->AddRectFilled( int( context_menu_pos.x ) + 1, int( context_menu_pos.y ) + 1, 98, ( 18 * context_menu_actions.size( ) ) - 2, FloatColor( 40, 40, 40 ) );

	 for( int i = 0; i < context_menu_actions.size( ); i++ ) {
		Vector2 context_menu_action_pos_min( context_menu_pos + Vector2( 1.f, 1.f + ( 18.f * ( float )i ) ) );
		Vector2 context_menu_action_pos_max( context_menu_action_pos_min + Vector2( 98, 17 ) );

		bool hovered = menu->is_hovered( context_menu_action_pos_min, context_menu_action_pos_max );

		Render::Get( )->AddRectFilled( context_menu_action_pos_min.x, context_menu_action_pos_min.y, 98, 20, hovered ? FloatColor( 32, 32, 32 ) : FloatColor( 40, 40, 40 ) );

		if( hovered ) {
		  Render::Get( )->SetTextFont( FONT_MENU_BOLD );
		  Render::Get( )->AddText( Vector2D( context_menu_action_pos_min.x + 5, context_menu_action_pos_min.y + 3 ),
			 FloatColor( 220, 220, 220 ), DROP_SHADOW, context_menu_actions[i].first.c_str( ) );
		} else {
		  Render::Get( )->AddText( Vector2D( context_menu_action_pos_min.x + 5, context_menu_action_pos_min.y + 3 ),
			 FloatColor( 220, 220, 220 ), DROP_SHADOW, context_menu_actions[i].first.c_str( ) );

		}
	 }
  }
}


