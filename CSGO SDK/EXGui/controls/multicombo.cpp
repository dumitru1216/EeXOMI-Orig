#include "multicombo.hpp"

std::string construct_label( const std::vector<std::string> &items, const std::vector<bool*> &values ) {
  std::string result;

  for( int i = 0; i < items.size( ); i++ ) {
	 bool length_exceeded = result.length( ) >= 16 || result.empty( ) && items.size( ) == i + 1;
	 auto first_item = result.empty( );

	 if( *values.at( i ) && !length_exceeded ) {
		if( !first_item )
		  result.append( ", " );

		result.append( items[i] );
	 } else if( length_exceeded ) {
		result.append( "..." );
		break;
	 }
  }

  return result;
}

c_multicombo::c_multicombo( const std::string &label, std::vector<bool*> var, const std::vector<std::string> &items ) {
  this->label = std::move( label );
  this->m_vecVar = std::move( var );
  this->items = std::move( items );
  this->size = Vector2( 150, 27 );
}

void c_multicombo::think( ) {
  float group_width = this->get_parent( )->get_size( ).x - 61.0f + 8 - 30;

  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position + Vector2( 16, 15 );
  Vector2 combo_size( group_width - 2, 18 );

  Vector2 first_item_min( draw_position + Vector2( 16, 23 ) );
  Vector2 last_item_max( first_item_min + Vector2( combo_size.x, combo_size.y * this->items.size( ) ) - Vector2( 0, 1 ) );

  if( menu->is_hovered( draw_position, draw_position + combo_size ) && menu->is_key_pressed( VK_LBUTTON ) && !menu->is_blocking( this ) ) {
	 menu->block( this );
  } else if( !menu->is_hovered( first_item_min, last_item_max ) && menu->is_key_pressed( VK_LBUTTON ) && menu->is_blocking( this ) ) {
	 menu->block( nullptr );
  }

  if( menu->is_blocking( this ) && menu->is_key_pressed( VK_LBUTTON ) ) {

	 for( int i = 0; i < this->items.size( ); i++ ) {
		Vector2 item_min( draw_position + Vector2( 16, 23 + ( combo_size.y * i ) ) );
		Vector2 item_max( item_min + combo_size - Vector2( 0, 1 ) );

		if( menu->is_hovered( item_min, item_max ) ) {
		  *this->m_vecVar[i] ^= 1;
		}
	 }
  }
}

void c_multicombo::draw( ) {
  float group_width = this->get_parent( )->get_size( ).x - 61.0f + 8 - 30;

  Vector2 draw_position = this->get_parent( )->get_child_draw_position( ) + this->position;
  Vector2 combo_size( group_width - 2, 18 );

  Render::Get( )->AddRect( Vector2D( draw_position.x + 15, draw_position.y + 14 ), combo_size.x + 2, combo_size.y + 2, FloatColor( 17, 17, 17 ) );

  if( menu->is_hovered( draw_position + Vector2( 16, 15 ), draw_position + Vector2( 16, 15 ) + combo_size ) && !menu->is_blocking( this ) )
	 Render::Get( )->AddRectFilledMultiColor( Vector2D( draw_position.x + 16, draw_position.y + 15 ), combo_size.x, combo_size.y,
	 FloatColor( 54, 54, 54 ), FloatColor( 54, 54, 54 ),
	 FloatColor( 50, 50, 50 ), FloatColor( 50, 50, 50 ) );
  else
	 Render::Get( )->AddRectFilledMultiColor( Vector2D( draw_position.x + 16, draw_position.y + 15 ), combo_size.x, combo_size.y,
	 FloatColor( 50, 50, 50 ), FloatColor( 50, 50, 50 ),
	 FloatColor( 40, 40, 40 ), FloatColor( 40, 40, 40 ) );

  auto arrow = [] ( float x, float y, FloatColor clr ) {
	 for( auto i = 5.0f; i >= 2.0f; --i ) {
		auto offset = 5.0f - i;

		Render::Get( )->AddLine( Vector2D( x + offset, y + offset ), Vector2D( x + offset + Math::Clamp( i - offset, 0.0f, 5.0f ), y + offset ), clr );
	 }
  };

  arrow( draw_position.x + combo_size.x + 4, draw_position.y + ( combo_size.y + 4 ), FloatColor( 0, 0, 0 ) );
  arrow( draw_position.x + combo_size.x + 4, draw_position.y + ( combo_size.y + 5 ), FloatColor( 151, 151, 151 ) );

  Render::Get( )->SetTextFont( FONT_MENU );

  Render::Get( )->AddText( Vector2D{ draw_position.x + 15, draw_position.y - 3 }, FloatColor( 220, 220, 220 ), DROP_SHADOW, this->label.c_str( ) );
  Render::Get( )->AddText( Vector2D{ draw_position.x + 21, draw_position.y + 17 }, FloatColor( 220, 220, 220 ),
	 DROP_SHADOW, construct_label( this->items, this->m_vecVar ).c_str( ) );

  if( menu->is_blocking( this ) ) {
	 Render::Get( )->AddRect( Vector2D( draw_position.x + 15, draw_position.y + 19 + combo_size.y ), combo_size.x + 2, ( combo_size.y * this->items.size( ) ) + 2, FloatColor( 17, 17, 17 ) );

	 for( int i = 0; i < this->items.size( ); i++ ) {
		Vector2 item_min( draw_position + Vector2( 16, 20 + ( combo_size.y * ( i + 1 ) ) ) );
		Vector2 item_max( item_min + combo_size - Vector2( 0, 1 ) );

		bool is_hovered = menu->is_hovered( item_min, item_max );

		Render::Get( )->AddRectFilled( Vector2D( item_min.x, item_min.y ), combo_size.x, combo_size.y, is_hovered ? FloatColor( 60, 60, 60 ) : FloatColor( 44, 44, 44 ) );

		if( *m_vecVar[i] || is_hovered ) {
		  Render::Get( )->SetTextFont( FONT_MENU_BOLD );
		  Render::Get( )->AddText( { item_min.x + 5, item_min.y + 2 },
			 *m_vecVar[i] ? g_Vars.menu.main_color : FloatColor( 220, 220, 220 ), DROP_SHADOW, this->items[i].c_str( ) );
		} else {
		  Render::Get( )->AddText( { item_min.x + 5, item_min.y + 2 },
			 FloatColor{ 220, 220, 220 }, DROP_SHADOW, this->items[i].c_str( ) );
		}
	 }
  }
}
