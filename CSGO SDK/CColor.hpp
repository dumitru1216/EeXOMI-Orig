#pragma once

class Color {
public:
   unsigned char RGBA[ 4 ];

   Color( ) {
	  RGBA[ 0 ] = 255;
	  RGBA[ 1 ] = 255;
	  RGBA[ 2 ] = 255;
	  RGBA[ 3 ] = 255;
   }
   Color( int r, int g, int b, int a = 255 ) {
	  RGBA[ 0 ] = r;
	  RGBA[ 1 ] = g;
	  RGBA[ 2 ] = b;
	  RGBA[ 3 ] = a;
   }

   bool operator!=( Color color ) { return RGBA[ 0 ] != color.RGBA[ 0 ] || RGBA[ 1 ] != color.RGBA[ 1 ] || RGBA[ 2 ] != color.RGBA[ 2 ] || RGBA[ 3 ] != color.RGBA[ 3 ]; }
   bool operator==( Color color ) { return RGBA[ 0 ] == color.RGBA[ 0 ] && RGBA[ 1 ] == color.RGBA[ 1 ] && RGBA[ 2 ] == color.RGBA[ 2 ] && RGBA[ 3 ] == color.RGBA[ 3 ]; }

   // returns the color from 0.f - 1.f
   static float Base( const unsigned char col ) {
	  return col / 255.f;
   }

   float* Base( ) {
	  float clr[ 3 ];

	  clr[ 0 ] = RGBA[ 0 ] / 255.0f;
	  clr[ 1 ] = RGBA[ 1 ] / 255.0f;
	  clr[ 2 ] = RGBA[ 2 ] / 255.0f;

	  return &clr[ 0 ];
   }

   static Color Inverse( const Color color ) {
	  return Color( 255 - color.RGBA[ 0 ], 255 - color.RGBA[ 1 ], 255 - color.RGBA[ 2 ] );
   }

   int GetD3DColor( ) const {
	  return ( ( int ) ( ( ( ( RGBA[ 3 ] ) & 0xff ) << 24 ) | ( ( ( RGBA[ 0 ] ) & 0xff ) << 16 ) | ( ( ( RGBA[ 1 ] ) & 0xff ) << 8 ) | ( ( RGBA[ 2 ] ) & 0xff ) ) );
   }

   static Color Red( ) {
	  return Color( 255, 0, 0 );
   }

   static Color Green( ) {
	  return Color( 0, 255, 0 );
   }

   static Color Blue( ) {
	  return Color( 0, 0, 255 );
   }

   static Color LightBlue( ) {
	  return Color( 100, 100, 255 );
   }

   static Color Grey( ) {
	  return Color( 128, 128, 128 );
   }

   static Color DarkGrey( ) {
	  return Color( 45, 45, 45 );
   }

   static Color Black( ) {
	  return Color( 0, 0, 0 );
   }

   static Color White( ) {
	  return Color( 255, 255, 255 );
   }

   static Color Purple( ) {
	  return Color( 220, 0, 220 );
   }

   float Difference( const Color color ) const // from 0.f - 1.f with 1.f being the most different
   {
	  float red_diff = std::fabs( Base( RGBA[ 0 ] ) - Base( color.RGBA[ 0 ] ) );
	  float green_diff = std::fabs( Base( RGBA[ 1 ] ) - Base( color.RGBA[ 1 ] ) );
	  float blue_diff = std::fabs( Base( RGBA[ 2 ] ) - Base( color.RGBA[ 2 ] ) );
	  float alpha_diff = std::fabs( Base( RGBA[ 3 ] ) - Base( color.RGBA[ 3 ] ) );

	  return ( red_diff + green_diff + blue_diff + alpha_diff ) * 0.25f;
   }

   // RGB -> HSB
   static float Hue( const Color color ) {
	  float R = Base( color.RGBA[ 0 ] );
	  float G = Base( color.RGBA[ 1 ] );
	  float B = Base( color.RGBA[ 2 ] );

	  float mx = std::max( R, std::max( G, B ) );
	  float mn = std::min( R, std::min( G, B ) );
	  if ( mx == mn )
		 return 0.f;

	  float delta = mx - mn;

	  float hue = 0.f;
	  if ( mx == R )
		 hue = ( G - B ) / delta;
	  else if ( mx == G )
		 hue = 2.f + ( B - R ) / delta;
	  else
		 hue = 4.f + ( R - G ) / delta;

	  hue *= 60.f;
	  if ( hue < 0.f )
		 hue += 360.f;

	  return hue / 360.f;
   }

   static float Saturation( const Color color ) {
	  float R = Base( color.RGBA[ 0 ] );
	  float G = Base( color.RGBA[ 1 ] );
	  float B = Base( color.RGBA[ 2 ] );

	  float mx = std::max( R, std::max( G, B ) );
	  float mn = std::min( R, std::min( G, B ) );

	  float delta = mx - mn;

	  if ( mx == 0.f )
		 return delta;

	  return delta / mx;
   }

   static float Brightness( const Color color ) {
	  float R = Base( color.RGBA[ 0 ] );
	  float G = Base( color.RGBA[ 1 ] );
	  float B = Base( color.RGBA[ 2 ] );

	  return  std::max( R, std::max( G, B ) );
   }

   float Hue( ) const {
	  return Hue( *this );
   }

   float Saturation( ) const {
	  return Saturation( *this );
   }

   float Brightness( ) const {
	  return Brightness( *this );
   }

   // HSB -> RGB
   static Color HSBtoRGB( float hue /* 0.f - 1.f*/,
	  float saturation /* 0.f - 1.f */,
	  float brightness /* 0.f - 1.f */,
	  int alpha = 255 ) {
	  hue = std::clamp( hue, 0.f, 1.f );
	  saturation = std::clamp( saturation, 0.f, 1.f );
	  brightness = std::clamp( brightness, 0.f, 1.f );

	  float h = ( hue == 1.f ) ? 0.f : ( hue * 6.f );
	  float f = h - static_cast< int >( h );
	  float p = brightness * ( 1.f - saturation );
	  float q = brightness * ( 1.f - saturation * f );
	  float t = brightness * ( 1.f - ( saturation * ( 1.f - f ) ) );

	  if ( h < 1.f )
		 return Color( brightness * 255, t * 255, p * 255, alpha );
	  else if ( h < 2.f )
		 return Color( q * 255, brightness * 255, p * 255, alpha );
	  else if ( h < 3.f )
		 return Color( p * 255, brightness * 255, t * 255, alpha );
	  else if ( h < 4 )
		 return Color( p * 255, q * 255, brightness * 255, alpha );
	  else if ( h < 5 )
		 return Color( t * 255, p * 255, brightness * 255, alpha );
	  else
		 return Color( brightness * 255, p * 255, q * 255, alpha );
   }
   __inline void SetColor( int _r, int _g, int _b, int _a ) {
	  RGBA[ 0 ] = ( unsigned char ) _r;
	  RGBA[ 1 ] = ( unsigned char ) _g;
	  RGBA[ 2 ] = ( unsigned char ) _b;
	  RGBA[ 3 ] = ( unsigned char ) _a;
   }
   __inline void SetColor( float _r, float _g, float _b, float _a ) {
	  RGBA[ 0 ] = static_cast< unsigned char >( _r * 255.0f );
	  RGBA[ 1 ] = static_cast< unsigned char >( _g * 255.0f );
	  RGBA[ 2 ] = static_cast< unsigned char >( _b * 255.0f );
	  RGBA[ 3 ] = static_cast< unsigned char >( _a * 255.0f );
   }
   void SetColor( float* color ) {
	  if ( !color )
		 return;

	  RGBA[ 0 ] = ( unsigned char ) ( color[ 0 ] * 255.f );
	  RGBA[ 1 ] = ( unsigned char ) ( color[ 1 ] * 255.f );
	  RGBA[ 2 ] = ( unsigned char ) ( color[ 2 ] * 255.f );
	  RGBA[ 3 ] = ( unsigned char ) ( color[ 3 ] * 255.f );
   }

   void SetAlpha( int alpha ) {
	  RGBA[ 3 ] = alpha;
   }
};
