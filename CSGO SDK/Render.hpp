#pragma once
#include <string>

#include "sdk.hpp"
#include "vector4D.hpp"

using TextureID = void*;
using ColorU32 = uint32_t;
using Rect2D = Vector4D;
using FontHandle = std::size_t;

struct IDirect3DDevice9;

enum text_flags : int {
  CENTER_X = ( 1 << 0 ),
  CENTER_Y = ( 1 << 1 ),
  ALIGN_RIGHT = ( 1 << 2 ),
  ALIGN_BOTTOM = ( 1 << 3 ),
  DROP_SHADOW = ( 1 << 4 ),
  OUTLINED = ( 1 << 5 ),
};

enum : uint32_t {
  FONT_VERDANA = 0,
  FONT_MENU_BOLD = FONT_VERDANA,
  FONT_MENU = FONT_VERDANA,
  FONT_CSGO_ICONS,
  FONT_VERDANA_30_BOLD,
  FONT_VERDANA_25_REGULAR,
  FONT_VISITOR,
  FONT_PORTER,
  FONT_CSGO_ICONS2,
  FONT_VERDANA_40_BOLD,
};

class Render : public NonCopyable {
protected:
  Render( ) { };
  virtual ~Render( ) { };
public:
  static Render* Get( );

  virtual void Initialize( IDirect3DDevice9* pDevice ) = 0;
  virtual void BeginScene( ) = 0;
  virtual void EndScene( ) = 0;
  virtual void RenderScene( ) = 0;
  virtual void OnReset( ) = 0;

  virtual void SetTextFont( size_t font ) = 0;
  virtual Vector2D GetScreenSize( ) = 0;

  virtual Vector2D CalcTextSize( const char* text ) = 0;
  virtual void AddTextW( Vector2D pos, ColorU32 color, int flags, wchar_t* text, ... ) = 0;
  virtual void AddText( Vector2D point, ColorU32 color, int flags, const char* format, ... ) = 0;
  virtual void AddText( float x, float y, ColorU32 color, int flags, const char* format, ... ) = 0;
  virtual void AddRect( const Vector2D& a, float w, float h, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0, float thickness = 1.0f ) = 0;

  virtual void AddLine( const Vector2D& a, const Vector2D& b, ColorU32 col, float thickness = 1.0f ) = 0;
  virtual void AddRect( const Vector2D& a, const Vector2D& b, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0, float thickness = 1.0f ) = 0;
  virtual void AddRectFilled( const Vector2D& a, const Vector2D& b, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0 ) = 0;
  virtual void AddRectFilledMultiColor( const Vector2D& a, const Vector2D& b, ColorU32 col_upr_left, ColorU32 col_upr_right, ColorU32 col_bot_right, ColorU32 col_bot_left ) = 0;
  virtual void AddQuad( const Vector2D& a, const Vector2D& b, const Vector2D& c, const Vector2D& d, ColorU32 col, float thickness = 1.0f ) = 0;
  virtual void AddQuadFilled( const Vector2D& a, const Vector2D& b, const Vector2D& c, const Vector2D& d, ColorU32 col ) = 0;
  virtual void AddTriangle( const Vector2D& a, const Vector2D& b, const Vector2D& c, ColorU32 col, float thickness = 1.0f ) = 0;
  virtual void AddTriangleFilled( const Vector2D& a, const Vector2D& b, const Vector2D& c, ColorU32 col ) = 0;
  virtual void AddTriangleMultiColor( const Vector2D& a, const Vector2D& b, const Vector2D& c, ColorU32 a_col, ColorU32 b_col, ColorU32 c_col ) = 0;
  virtual void AddCircle( const Vector2D& centre, float radius, ColorU32 col, int32_t num_segments = 12, float thickness = 1.0f ) = 0;
  virtual void AddCircleFilled( const Vector2D& centre, float radius, ColorU32 col, int32_t num_segments = 12 ) = 0;
  virtual void AddPolyline( const Vector2D* points, const int32_t num_points, ColorU32 col, bool closed, float thickness ) = 0;
  virtual void AddConvexPolyFilled( const Vector2D* points, const int32_t num_points, ColorU32 col ) = 0;
  virtual void AddBezierCurve( const Vector2D& pos0, const Vector2D& cp0, const Vector2D& cp1, const Vector2D& pos1, ColorU32 col, float thickness, int32_t num_segments = 0 ) = 0;

  virtual void SetScissorsRect( const Vector4D& rect ) = 0;
  virtual void RestoreScissorsRect( ) = 0;

  // independent from render implentation
  // ------------------------------------

  inline void AddRect( const Vector4D& a, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0, float thickness = 1.0f ) {
	 AddRect( a.min, a.max, col, rounding, rounding_corners_flags, thickness );
  };

  inline void AddRectFilled( const Vector2D& a, float w, float h, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0 ) {
	 AddRectFilled( a, a + Vector2D( w, h ), col, rounding, rounding_corners_flags );
  };

  inline void AddRectFilled( const Vector4D& a, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0 ) {
	 AddRectFilled( a.min, a.max, col, rounding, rounding_corners_flags );
  };

  inline void AddRectFilledMultiColor( const Vector4D& a, ColorU32 col_upr_left, ColorU32 col_upr_right, ColorU32 col_bot_right, ColorU32 col_bot_left ) {
	 AddRectFilledMultiColor( a.min, a.max, col_upr_left, col_bot_right, col_bot_right, col_bot_left );
  }


  inline void AddRectFilledMultiColor( const Vector2D& a, float w, float h, ColorU32 col_upr_left, ColorU32 col_upr_right, ColorU32 col_bot_right, ColorU32 col_bot_left ) {
	 AddRectFilledMultiColor( a, a + Vector2D( w, h ), col_upr_left, col_upr_right, col_bot_right, col_bot_left );
  }

  inline void AddCornerBox( const Vector2D& a, const Vector2D& b, ColorU32 clr, float th = 1.f ) {
	 auto drawCorner = [this, th] ( const Vector2D& a, const Vector2D& b, const Vector2D& c, const Vector2D& size, ColorU32 clr ) {
		Vector2D kek[] = { a, b, c };
		AddPolyline( kek, 3, clr, false, th );
	 };

	 Vector4D bbox = Vector4D( a, b );
	 float_t w = ( bbox.max.x - bbox.min.x ) * 0.25f;
	 float_t h = ( bbox.max.y - bbox.min.y ) * 0.25f;

	 // upper-left
	 drawCorner( Vector2D( bbox.min.x + w, bbox.min.y ), Vector2D( bbox.min ), Vector2D( bbox.min.x, bbox.min.y + h ), { w, h }, clr );
	 // upper-right
	 drawCorner( Vector2D( bbox.max.x - w, bbox.min.y ), Vector2D( bbox.max.x, bbox.min.y ), Vector2D( bbox.max.x, bbox.min.y + h ), { w, h }, clr );
	 // bottom-left
	 drawCorner( Vector2D( bbox.min.x + w, bbox.max.y ), Vector2D( bbox.min.x, bbox.max.y ), Vector2D( bbox.min.x, bbox.max.y - h ), { w, h }, clr );
	 // bottom-right
	 drawCorner( Vector2D( bbox.max.x - w, bbox.max.y ), Vector2D( bbox.max ), Vector2D( bbox.max.x, bbox.max.y - h ), { w, h }, clr );
  }
};

