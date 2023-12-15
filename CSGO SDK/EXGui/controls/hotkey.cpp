#include <cctype>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include <string>
#include "hotkey.hpp"

const char* keys[] = {
"[-]", "[M1]", "[M2]", "[BRK]", "[M3]", "[M4]", "[M5]",
"[-]", "[BSPC]", "[TAB]", "[-]", "[-]", "[-]", "[ENTER]",
"[-]", "[-]", "[SHFT]","[CTRL]", "[ALT]","[PAUSE]","[CAPS]",
"[-]", "[-]", "[-]", "[-]", "[-]", "[-]","[ESC]", "[-]", "[-]",
"[-]", "[-]", "[SPACE]","[PGUP]", "[PGDOWN]", "[END]", "[HOME]",
"[Left]","[UP]", "[RIGHT]", "[DOWN]", "[-]", "[PRNT]", "[-]",
"[PRTSCR]", "[INS]","[DEL]", "[-]", "[0]", "[1]","[2]", "[3]",
"[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[-]", "[-]", "[-]",
"[-]", "[-]", "[-]","[-]", "[A]", "[B]", "[C]", "[D]", "[E]",
"[F]", "[G]", "[H]", "[I]", "[J]", "[K]", "[L]", "[M]", "[N]",
"[O]", "[P]", "[Q]", "[R]", "[S]", "[T]", "[U]","[V]", "[W]",
"[X]","[Y]", "[Z]", "[LFTWIN]", "[RGHTWIN]", "[-]", "[-]",
"[-]", "[NUM0]", "[NUM1]","[NUM2]", "[NUM3]", "[NUM4]", "[NUM5]",
"[NUM6]","[NUM7]", "[NUM8]", "[NUM9]", "[*]", "[+]", "[-]", "[-]",
"[.]", "[/]", "[F1]", "[F2]", "[F3]","[F4]", "[F5]", "[F6]",
"[F7]", "[F8]", "[F9]", "[F10]", "[F11]", "[F12]","[F13]",
"[F14]", "[F15]", "[F16]", "[F17]", "[F18]", "[F19]", "[F20]",
"[F21]","[F22]", "[F23]", "[F24]", "[-]", "[-]", "[-]", "[-]",
"[-]","[-]", "[-]", "[-]","[NUMLCK]", "[SCRLLCK]", "[-]", "[-]",
"[-]", "[-]", "[-]", "[-]", "[-]","[-]", "[-]","[-]", "[-]",
"[-]", "[-]", "[-]", "[LSHFT]", "[RSHFT]", "[LCTRL]","[RCTRL]",
"[LMENU]", "[RMENU]", "[-]","[-]", "[-]","[-]", "[-]", "[-]",
"[-]","[-]", "[-]", "[-]", "[NTRK]", "[PTRK]", "[STOP]", "[PLAY]",
"[-]", "[-]","[-]", "[-]", "[-]", "[-]", "[;]", "[+]", "[,]",
"[-]", "[.]", "[?]", "[~]", "[-]", "[-]","[-]", "[-]","[-]",
"[-]", "[-]", "[-]", "[-]", "[-]", "[-]","[-]", "[-]", "[-]",
"[-]", "[-]", "[-]", "[-]", "[-]","[-]","[-]", "[-]", "[-]",
"[-]", "[-]", "[-]", "[-]", "[-]", "[-]", "[-]", "[-]","[-]",
"[-]", "[-]","[-]", "[-]", "[-]", "[-]", "[-]", "[-]","[-]",
"[-]", "[-]", "[-]", "[-]", "[-]", "[-]", "[-]", "[-]","[-]", "[-]",
"[-]", "[-]", "[-]", "[-]", "[-]", "[-]", "[-]", "[-]", "[-]"
};

void c_hotkey::context_menu_always_on( c_hotkey *self ) {
	//config->set( self->var_name + "_type", 0 );
}

void c_hotkey::context_menu_on_hotkey( c_hotkey *self ) {
	//config->set( self->var_name + "_type", 1 );
}

void c_hotkey::context_menu_toggle( c_hotkey *self ) {
	//config->set( self->var_name + "_type", 2 );
}

void c_hotkey::context_menu_off_hotkey( c_hotkey *self ) {
	//config->set( self->var_name + "_type", 3 );
}

c_hotkey::c_hotkey( const std::string &label, int *key, bool is_inline ) {
  this->label = std::move( label );
  this->m_iKey = std::move( key );
  this->is_inline = is_inline;

  if( is_inline )
	 this->size = Vector2( 0, -16 );
  else
	 this->size = Vector2( 150, 27 );

  this->context_menu_actions.push_back( { "Always on", &c_hotkey::context_menu_always_on } );
  this->context_menu_actions.push_back( { "On hotkey", &c_hotkey::context_menu_on_hotkey } );
  this->context_menu_actions.push_back( { "Toggle", &c_hotkey::context_menu_toggle } );
  this->context_menu_actions.push_back( { "Off hotkey", &c_hotkey::context_menu_off_hotkey } );
}

void c_hotkey::think( ) {
  Render::Get( )->SetTextFont( FONT_MENU );
  Vector2 text_size = Render::Get( )->CalcTextSize( this->label.c_str( ) );

  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position;
  float group_width = this->get_parent( )->get_size( ).x - 46.0f + 8;

  if( menu->is_hovered( draw_position + Vector2( group_width - text_size.x, 0 ), draw_position + Vector2( group_width, text_size.y ) ) && !menu->is_blocking( ) &&
	 !menu->is_blocking( this ) && !this->context_menu_open && menu->is_key_pressed( VK_LBUTTON ) && !this->is_picking ) {
	 menu->block( this );
	 this->is_picking = true;
	 return;
  } else if( this->context_menu_open ) {
	 if( !menu->is_hovered( context_menu_pos, context_menu_pos + Vector2( 100, ( int )context_menu_actions.size( ) * 18 ) ) &&
		menu->is_blocking( this ) && menu->is_key_pressed( VK_LBUTTON ) ) {
		this->context_menu_open = false;
		menu->block( nullptr );
	 }
  }

  if( this->context_menu_open ) {
	 if( menu->is_hovered( context_menu_pos, context_menu_pos + Vector2( 100, ( int )context_menu_actions.size( ) * 18 ) ) &&
		menu->is_blocking( this ) && menu->is_key_pressed( VK_LBUTTON ) ) {
		for( int i = 0; i < context_menu_actions.size( ); i++ ) {
		  Vector2 context_menu_action_pos_min( context_menu_pos + Vector2( 1, 1 + ( 18 * i ) ) );
		  Vector2 context_menu_action_pos_max( context_menu_action_pos_min + Vector2( 98, 17 ) );

		  if( menu->is_hovered( context_menu_action_pos_min, context_menu_action_pos_max ) ) {
			 context_menu_actions[i].second( this );

			 this->context_menu_open = false;
			 menu->block( nullptr );
		  }
		}
	 }
  } else if( this->is_picking ) {
	 for( int i = 0; i < 256; i++ ) {
		if( menu->is_key_pressed( i ) ) {
		  this->is_picking = false;
		  menu->block( nullptr );

		  if( i != VK_ESCAPE )
			 *this->m_iKey = i;
		  else 
			 *this->m_iKey = 0;
		}
	 }
  }

  if( menu->is_hovered( draw_position + Vector2( group_width - 20, 0 ), draw_position + Vector2( group_width, 8 ) ) && !menu->is_blocking( ) &&
	 !menu->is_blocking( this ) && !this->context_menu_open && menu->is_key_pressed( VK_RBUTTON ) && !this->is_picking ) {
	 this->context_menu_open = true;
	 this->context_menu_pos = Vector2( draw_position.x + group_width, draw_position.y + text_size.y ) + Vector2( -100, 2 );

	 menu->block( this );
  }
}

void c_hotkey::draw( ) {
  Render::Get( )->SetTextFont( FONT_MENU );
  Vector2 text_size = Render::Get( )->CalcTextSize( this->label.c_str( ) );

  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position;
  float group_width = this->get_parent( )->get_size( ).x - 46.0f + 8;

  if( !this->is_inline )
	 Render::Get( )->AddText( { draw_position.x + 15.0f, draw_position.y - 3.0f }, FloatColor( 220, 220, 220 ), DROP_SHADOW, this->label.c_str( ) );

  Render::Get( )->AddText( { draw_position.x + group_width - text_size.x, draw_position.y - 3 }, this->is_picking ? FloatColor( 255, 0, 0 ) : FloatColor( 107, 107, 107 ), DROP_SHADOW, keys[*this->m_iKey] );

  if( menu->is_blocking( this ) && !this->context_menu_open ) {
  } else if( menu->is_blocking( this ) && this->context_menu_open ) {
	 Render::Get( )->AddRect( Vector4D( context_menu_pos.x, context_menu_pos.y, 100, 19 * context_menu_actions.size( ) ), FloatColor( 17, 17, 17 ) );
	 Render::Get( )->AddRectFilled( Vector4D( context_menu_pos.x + 1, context_menu_pos.y + 1, 98, ( 18 * context_menu_actions.size( ) ) - 2 ), FloatColor( 40, 40, 40 ) );

	 for( int i = 0; i < context_menu_actions.size( ); i++ ) {
		Vector2 context_menu_action_pos_min( context_menu_pos + Vector2( 1, 1 + ( 18 * i ) ) );
		Vector2 context_menu_action_pos_max( context_menu_action_pos_min + Vector2( 98, 17 ) );

		bool hovered = menu->is_hovered( context_menu_action_pos_min, context_menu_action_pos_max );
		bool active = *this->m_iKey == i;

		Render::Get( )->AddRectFilled( Vector4D( context_menu_action_pos_min.x, context_menu_action_pos_min.y, 98, 20 ), hovered ? FloatColor( 32, 32, 32 ) : FloatColor( 40, 40, 40 ) );

		if( hovered || active ) {
		  Render::Get( )->AddText( Vector2D( context_menu_action_pos_min.x + 5, context_menu_action_pos_min.y + 3 ), active ? g_Vars.menu.main_color : FloatColor( 220, 220, 220 ), DROP_SHADOW, context_menu_actions[i].first.c_str( ) );
		} else {
		  Render::Get( )->AddText( Vector2D( context_menu_action_pos_min.x + 5, context_menu_action_pos_min.y + 3 ), FloatColor( 220, 220, 220 ), DROP_SHADOW, context_menu_actions[i].first.c_str( ) );
		}
	 }
  }
}
