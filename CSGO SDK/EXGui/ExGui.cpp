#include <windows.h>

#include "ExGui.hpp"
#include "controls/tab.hpp"
#include "../Source.hpp"
#include "../TextureForSpriteshpp.h"
#include "../InputSys.hpp"
#include <minwindef.h>

void c_sprite::invalidate( ) {
  this->texture->Release( );
  delete this->texture;
  this->sprite->Release( );
  delete this->sprite;
}

void c_sprite::draw( Vector2 pos, int alpha, float rotation ) {
  D3DXVECTOR3 position;

  position.x = pos.x;
  position.y = pos.y;
  position.z = 0.01f;

  D3DXVECTOR3 scaling( 1.f, 1.f, 1.f );
  D3DXVECTOR3 centre( 100, 100, 0 );
  D3DXMATRIX matrix;

  //D3DXMatrixTransformation( &matrix, NULL, 0, &scaling, &centre, NULL, &position );

  sprite->SetTransform( &matrix );

  sprite->Draw( texture, NULL, NULL, NULL, D3DCOLOR_RGBA( 255, 255, 255, alpha ) );
}

void c_sprite::draw( int x, int y, int alpha, float rotation ) {
  //draw( Vector2( x, y ), alpha, rotation );
}

void c_menu::poll_input( ) {
  this->previous_mouse_pos = this->mouse_pos;

  this->mouse_wheel = InputSys::Get( )->GetScrollMouse( );

  for( int i = 0; i < 256; i++ ) {
	 this->previous_key_state[i] = this->key_state[i];
	 this->key_state[i] = GetAsyncKeyState( i );
  }

  POINT mouse_point;

  GetCursorPos( &mouse_point );
  ScreenToClient( FindWindow( "Valve001", "Counter-Strike: Global Offensive" ), &mouse_point );

  this->mouse_pos = Vector2( mouse_point.x, mouse_point.y );
}

bool c_menu::is_key_down( int key ) {
  return this->key_state[key] == true;
}

bool c_menu::is_key_pressed( int key ) {
  return this->previous_key_state[key] == false && this->key_state[key] == true;
}

bool c_menu::is_key_released( int key ) {
  return this->previous_key_state[key] == true && this->key_state[key] == false;
}

void c_menu::set_mouse_wheel( int mouse_wheel ) {
  this->mouse_wheel = mouse_wheel;
}

int c_menu::get_mouse_wheel( ) {
  return this->mouse_wheel;
}

Vector2 c_menu::get_mouse_pos( ) {
  return this->mouse_pos;
}

bool c_menu::in_bounds( Vector2 pos, Vector2 size ) {
  // grab cursor
  auto cursor = get_mouse_pos( );

  // math for inbounds
  return ( cursor.x > pos.x &&
	 cursor.y > pos.y &&
	 cursor.x < pos.x + size.x &&
	 cursor.y < pos.y + size.y );
}


bool c_menu::is_hovered( Vector2 min, Vector2 max ) {
  return this->mouse_pos.x >= min.x && this->mouse_pos.y >= min.y &&
	 this->mouse_pos.x <= max.x && this->mouse_pos.y <= max.y;
}

void c_menu::add_tab( c_tab *tab ) {
  tab->set_parent( this );

  this->tabs.push_back( tab );
  this->active_tab = tab;
}

bool c_menu::is_blocking( c_element *elem ) {
  if( elem == nullptr )
	 return this->blocking != nullptr;

  return this->blocking == elem;
}

void c_menu::block( c_element *elem ) {
  this->blocking = elem;
}

c_element *c_menu::get_blocking( ) {
  return this->blocking;
}

c_tab *c_menu::get_active_tab( ) {
  return this->active_tab;
}

void c_menu::set_active_tab( c_tab *tab ) {
  this->active_tab = tab;
}

bool c_menu::get_hotkey( int key ) {
  int activation_type = 1;

  switch( activation_type ) {
  case 0:
	 return true;
	 break;
  case 1:
	 return menu->is_key_down( key );
	 break;
  case 2:
  #if 0
	 if( hotkey_states.find( value ) == hotkey_states.end( ) ) hotkey_states[value] = menu->is_key_down( key );
	 else if( menu->is_key_pressed( key ) ) hotkey_states[value] = !hotkey_states[value];
	 return hotkey_states.at( value );
  #endif
	 return false;
	 break;
  case 3:
	 return !menu->is_key_down( key );
	 break;
  }
}

void c_menu::think( ) {
  this->poll_input( );

  if( this->is_hovered( fixed_position - Vector2( 6, 6 ), fixed_position + Vector2( size.x + 6, 6 ) ) ) {
	 if( this->is_key_pressed( VK_LBUTTON ) && !this->dragging )
		this->dragging = true;
  } else if( !this->is_blocking( ) ) {
	 for( int i = 0; i < ( int )tabs.size( ); i++ ) {
		c_tab *tab = tabs[i]; 

		int tab_text_width, tab_text_height;

		Render::Get( )->SetTextFont( FONT_MENU );
		Vector2 text_size = Render::Get( )->CalcTextSize( tab->get_label( ).c_str( ) );

		const float tab_width = this->get_size( ).x / float( tabs.size( ) );

		Vector2 tab_hover_min( fixed_position.x + 19 + ( tab_width * i ), fixed_position.y + ( 20 ) );
		Vector2 tab_hover_max = tab_hover_min + Vector2( tab_width, 25 );

		if( this->is_hovered( tab_hover_min, tab_hover_max ) && this->is_key_pressed( VK_LBUTTON ) )
		  this->active_tab = tab;
	 }
  }

  if( this->is_key_down( VK_LBUTTON ) && this->dragging ) {
	 Vector2 mouse_delta = this->previous_mouse_pos - this->mouse_pos;
	 this->position -= mouse_delta;
  }

  if( !this->is_key_down( VK_LBUTTON ) && this->dragging )
	 this->dragging = false;

  this->active_tab->think( );
}

void c_menu::draw( ) {
  constexpr float fade_factor = 6.666667f;
  float fade_increment = ( fade_factor * Source::m_pGlobalVars->absoluteframetime ) * 255.0f;

  if( g_Vars.globals.menuOpen )
	 menu_alpha += ( int )fade_increment;
  else
	 menu_alpha -= ( int )fade_increment;

  menu_alpha = max( 0, min( 255, menu_alpha ) );

  if( menu_alpha == 0 )
	 return;

  Vector2D screenSIze = Render::Get( )->GetScreenSize( );
  fixed_position.x = max( 6.f, min( position.x, screenSIze.x - size.x - 6 ) );
  fixed_position.y = max( 6.f, min( position.y, screenSIze.y - size.y - 6 ) );

  Render::Get( )->AddRectFilled( fixed_position - 1, size.x + 4, size.y + 4, FloatColor( 32, 32, 32, menu_alpha ) );

  Render::Get( )->AddRect( Vector2D( fixed_position.x - 1, fixed_position.y - 1 ), size.x + 2, size.y + 2, FloatColor( 60, 60, 60, menu_alpha ) );
  Render::Get( )->AddRect( Vector2D( fixed_position.x - 2, fixed_position.y - 2 ), size.x + 4, size.y + 4, FloatColor( 40, 40, 40, menu_alpha ) );
  Render::Get( )->AddRect( Vector2D( fixed_position.x - 3, fixed_position.y - 3 ), size.x + 6, size.y + 6, FloatColor( 40, 40, 40, menu_alpha ) );
  Render::Get( )->AddRect( Vector2D( fixed_position.x - 4, fixed_position.y - 4 ), size.x + 8, size.y + 8, FloatColor( 40, 40, 40, menu_alpha ) );
  Render::Get( )->AddRect( Vector2D( fixed_position.x - 5, fixed_position.y - 5 ), size.x + 10, size.y + 10, FloatColor( 60, 60, 60, menu_alpha ) );
  Render::Get( )->AddRect( Vector2D( fixed_position.x - 6, fixed_position.y - 6 ), size.x + 12, size.y + 12, FloatColor( 31, 31, 31, menu_alpha ) );

  Render::Get( )->AddRectFilled( Vector2D( fixed_position.x + 1, fixed_position.y + 1 ), size.x - 2, 2, g_Vars.menu.main_color );
  Render::Get( )->AddLine( Vector2D( fixed_position.x + 1, fixed_position.y + 2 ), fixed_position.x + size.x - 1, fixed_position.y + 2,
	 FloatColor( 0, 0, 0, ( int )( 0.8f * g_Vars.menu.main_color.a ) ) );

  Render::Get( )->AddRectFilled( fixed_position.x + 19, fixed_position.y + this->size.y - 40, 440, 25, FloatColor( 24, 24, 24, menu_alpha ) );
  Render::Get( )->AddRect( fixed_position.x + 18, fixed_position.y + this->size.y - 41, 442, 27, FloatColor( 44, 44, 44, menu_alpha ) );

  for( int i = 0; i < ( int )tabs.size( ); i++ ) {
	 c_tab *tab = tabs[i];

	 FloatColor tab_text_color = tab == active_tab ? g_Vars.menu.main_color : FloatColor( 210, 210, 210, menu_alpha );
	 Render::Get( )->SetTextFont( tab == active_tab ? FONT_MENU_BOLD : FONT_MENU );

	 const float tab_width = ( this->get_size( ).x / ( float )tabs.size( ) );

	 int tab_center_x = fixed_position.x + 19 + ( tab_width * i ) + ( tab_width / 2 );
	 int tab_center_y = fixed_position.y + ( 20 ) + ( 25 / 2 );

	 Vector2 tab_text_size = Render::Get( )->CalcTextSize( tab->get_label( ).c_str( ) );

	 Render::Get( )->AddText( Vector2D( tab_center_x - ( tab_text_size.x * 0.5f ), tab_center_y - ( tab_text_size.y * 0.5f ) ),
		tab_text_color, DROP_SHADOW, tab->get_label( ).c_str( ) );

	 if( i != tabs.size( ) - 1 ) {
		Render::Get( )->AddLine( { float( tab_center_x + ( tab_width / 2 ) ), float( tab_center_y - 4 ) },
		  { float( tab_center_x + ( tab_width / 2 ) ), float( tab_center_y + 5 ) }, FloatColor( 44, 44, 44, menu_alpha ) );
	 }
  }

  this->active_tab->draw( );

  if( this->is_blocking( ) )
	 this->get_blocking( )->draw( );
}

Vector2 c_menu::get_child_draw_position( ) {
  return fixed_position + Vector2( 19, 22 );
}

c_menu *menu = new c_menu( );
