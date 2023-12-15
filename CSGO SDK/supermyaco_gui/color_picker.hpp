#pragma once
#include "sdk.hpp"
#include "Render.hpp"
#include "object.hpp"
#include "InputSys.hpp"

#define get_red(col) (((col)&0x00ff0000)>>16)
#define get_green(col) (((col)&0x0000ff00)>>8)
#define get_blue(col) ((col)&0x000000ff)

namespace object {
  inline IDirect3DTexture9* colorpicker_texture;
  inline LPD3DXSPRITE colorpicker_sprite;
  namespace helper {
	 // I hate pasting, but this function is too much of a pain
	 // to clean up -_-.
	 inline D3DCOLOR hsl_to_rgb( float h, float s, float l ) {

		float q;

		if ( l < 0.5f )
		  q = l * ( s + 1.0f );
		else
		  q = l + s - ( l * s );

		float p = 2 * l - q;

		float rgb[ 3 ];

		rgb[ 0 ] = h + ( 1.0f / 3.0f );
		rgb[ 1 ] = h;
		rgb[ 2 ] = h - ( 1.0f / 3.0f );

		for ( int i = 0; i < 3; ++i ) {

		  if ( rgb[ i ] < 0 )
			 rgb[ i ] += 1.0f;

		  if ( rgb[ i ] > 1 )
			 rgb[ i ] -= 1.0f;

		  if ( rgb[ i ] < ( 1.0f / 6.0f ) )
			 rgb[ i ] = p + ( ( q - p ) * 6 * rgb[ i ] );

		  else if ( rgb[ i ] < 0.5f )
			 rgb[ i ] = q;

		  else if ( rgb[ i ] < ( 2.0f / 3.0f ) )
			 rgb[ i ] = p + ( ( q - p ) * 6 * ( ( 2.0f / 3.0f ) - rgb[ i ] ) );
		  else
			 rgb[ i ] = p;
		}

		return D3DCOLOR_XRGB( int( rgb[ 0 ] * 255.f ), int( rgb[ 1 ] * 255.f ), int( rgb[ 2 ] * 255.f ) );
	 }

	 inline void destroy( ) {
		if ( colorpicker_sprite != NULL ) {
		  colorpicker_sprite->Release( );
		  colorpicker_sprite = NULL;
		}

		if ( colorpicker_texture != NULL ) {
		  colorpicker_texture->Release( );
		  colorpicker_texture = NULL;
		}
	 }

	 inline HRESULT setup( LPDIRECT3DDEVICE9 p_device, int width, int height ) {
		bool bits_32 = true;

		HRESULT return_value;

		return_value = p_device->CreateTexture( width, height, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &colorpicker_texture, 0 );

		if ( FAILED( return_value ) )
		{
		  bits_32 = false;

		  return_value = p_device->CreateTexture( width, height, 1, 0, D3DFMT_X4R4G4B4, D3DPOOL_MANAGED, &colorpicker_texture, 0 );

		  if ( FAILED( return_value ) )
			 return return_value;
		}

		D3DLOCKED_RECT palette;

		return_value = colorpicker_texture->LockRect( 0, &palette, 0, 0 );

		if ( FAILED( return_value ) ) {
		  destroy( );
		  return return_value;
		}

		float h = 0;
		float s = 0.99f;
		float l = 1.0f;

		D3DCOLOR Color;

		BYTE R;
		BYTE G;
		BYTE B;

		DWORD* Colors32 = ( ( DWORD* )palette.pBits ) - 1;
		WORD* Colors = ( ( WORD* )palette.pBits ) - 1;

		for ( int i = 0; i < width; ++i ) {

		  for ( int j = 0; j < height; ++j ) {

			 Color = hsl_to_rgb( h, s, l );

			 if ( bits_32 ) {
				Colors32++;
				*Colors32 = Color;
			 }

			 else {

				R = ( ( Color >> 16 ) & 0xFF ) / 0x10;
				G = ( ( Color >> 8 ) & 0xFF ) / 0x10;
				B = ( ( Color >> 0 ) & 0xFF ) / 0x10;

				Colors++;

				*Colors = ( 0xFF << 12 ) | ( R << 8 ) | ( G << 4 ) | ( B << 0 );
			 }

			 h += ( 1.0f / width );
		  }

		  l -= ( 1.0f / height );
		  h = 0.0f;
		}

		colorpicker_texture->UnlockRect( 0 );

		if ( !colorpicker_sprite )
		  D3DXCreateSprite( p_device, &colorpicker_sprite );

		return S_OK;
	 }

	 inline LPDIRECT3DTEXTURE9 get_texture( ) {
		return colorpicker_texture;
	 }

	 inline void draw( int x, int y, int alpha, float rotation ) {
		D3DXVECTOR3 position;

		position.x = ( float )x;
		position.y = ( float )y;
		position.z = 0.f;

		D3DXVECTOR2 scaling( 1.f, 1.f );

		D3DXVECTOR2 position2;
		position2.x = ( float )x;
		position2.y = ( float )y;

		D3DXVECTOR2 spriteCentre = D3DXVECTOR2( 100, 100 );

		D3DXMATRIX matrix;

		D3DXMatrixTransformation2D( &matrix, NULL, 0, &scaling, &spriteCentre, D3DXToRadian( rotation ), &position2 );

		colorpicker_sprite->SetTransform( &matrix );

		colorpicker_sprite->Begin( D3DXSPRITE_SORT_TEXTURE | D3DXSPRITE_ALPHABLEND );
		colorpicker_sprite->Draw( get_texture( ), NULL, NULL, NULL, D3DCOLOR_RGBA( 255, 255, 255, alpha ) );
		colorpicker_sprite->End( );
	 }

	 inline void draw_texture( int x, int y ) {
		if ( !colorpicker_sprite )
		  setup( g_Vars.globals.m_pD3D9Device, 200, 200 );

		if ( colorpicker_sprite != NULL ) {
		  draw( x, y, g_Vars.globals.MenuAlpha, 0.f );
		}
	 }

	 inline D3DCOLOR get_picked_colour( int x, int y ) {

		float h = x * ( 1.0f / 200 );
		float s = 0.99f;
		float l = 1.0f - y * ( 1.0f / 200 );

		return hsl_to_rgb( h, s, l );
	 }
  }

  class c_colorpicker : public c_object {
  protected:
	 LPD3DXSPRITE m_sprite = nullptr;

	 FloatColor *m_value;

	 bool m_opened = false;
	 bool m_clicked = false;
	 bool m_held = false;
  public:
	 c_colorpicker( const char *name, FloatColor *value ) :
		m_value( value ) {
		set_name( name );

		set_visible( true );

		m_size = { 7, 7 };

		set_identifier( hash_32_fnv1a_const( "COLORPICKER" ) );
	 }

	 void update( ) {
		if ( !m_parent )
		  set_visible( false );

		if ( !is_visible( ) )
		  return;

		// Update the position.
		m_pos.x = m_parent->get_pos( ).x + 25;
		m_pos.y = m_parent->get_pos( ).y + 5;

		for ( auto &it : m_parent->container( ) ) {
		  if ( it == shared_from_this( ) )
			 break;

		  if ( it->is_visible( ) )
			 m_pos.y += it->get_size( ).y + object_spacing;
		}

		// Don't affect the object until it's unblocked.
		if ( m_blocked )
		  return;

		// Restore blocked state if the colorpicker is closed.
		if ( !m_opened ) {
		  set_blocked_r( false );
		}

		// See if the colorpicker was clicked.
		if ( m_held && !InputSys::Get()->IsKeyDown( VK_LBUTTON ) ) {
		  m_clicked = true;
		}
		else {
		  m_clicked = false;
		}

		if ( m_clicked && InputSys::Get()->IsInBox( { m_pos.x + 135, m_pos.y }, { 20, 10 } ) ) {
		  m_opened = true;
		}

		if ( m_opened ) {
		  set_blocked_r( true );

		  if ( m_held && InputSys::Get()->IsInBox( { m_pos.x + 160, m_pos.y }, { 200, 200 } ) ) {
			 Vector2D pos = { m_pos.x + 160, m_pos.y };
			 Vector2D delta = { InputSys::Get()->GetMousePosition().x - pos.x, InputSys::Get()->GetMousePosition().y - pos.y };

			 D3DCOLOR clr = helper::get_picked_colour( delta.x, delta.y );
			 *m_value = FloatColor( get_red( clr ), get_green( clr ), get_blue( clr ),255 );

		  }
		  else if ( m_held && !InputSys::Get()->IsInBox( { m_pos.x + 135, m_pos.y }, { 20, 7 } ) ) {
			 m_opened = false;
		  }
		}

		m_held = InputSys::Get()->IsKeyDown( VK_LBUTTON );
	 }

	 void draw( ) {
		if ( !is_visible( ) )
		  return;

		// The colorpicker name.
		Render::Get( )->SetTextFont( FONT_MENU );
		Render::Get( )->AddText( Vector2D( m_pos.x, m_pos.y - 2 ), MENU_COLOR( 0.843f, 0.843f, 0.843f ), 0, m_name );

		// The little color box.
		Vector2D pos_color = { m_pos.x + 135, m_pos.y };
		Vector2D pos_sprite = { m_pos.x + 160, m_pos.y };

		FloatColor clr = FloatColor( *m_value );

		Render::Get( )->AddRectFilledMultiColor( Vector2D(pos_color.x, pos_color.y),pos_color + Vector2D(19, 7), MENU_COLOUR( clr ), MENU_COLOR( clr.r * 0.60f, clr.g * 0.60f, clr.b * 0.60f ),
		  MENU_COLOR( 0.282f, 0.282f, 0.282f ), MENU_COLOR( 0.176f, 0.176f, 0.176f ) );
	
		Render::Get( )->AddRect( Vector2D(pos_color.x, pos_color.y),pos_color + Vector2D(19, 7), MENU_COLOR( 0.f, 0.f, 0.f ) );

		if ( m_opened ) {
		  Render::Get( )->AddRect( Vector2D(pos_sprite.x, pos_sprite.y), pos_color + Vector2D(201, 201), MENU_COLOR( 0.f, 0.f, 0.f ) );
		  helper::draw_texture( pos_sprite.x + 1, pos_sprite.y + 1 );
		}
	 }
  };
}
