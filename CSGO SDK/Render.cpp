#define NOMINMAX
#define IMGUI_DEFINE_MATH_OPERATORS
#include "render.hpp"
#include <algorithm>
#include <mutex>

#include <d3d9.h>
#pragma comment( lib, "d3d9.lib" )

#include <unordered_map>
#include "std_vector.hpp"
#include "source.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "imgui_freetype.h"

#include "MenuFonts.h"

#include "XorStr.hpp"

ImFont* pMainFont = nullptr;
ImFont* pMainCapsFont = nullptr;
ImFont* pMenuFont = nullptr;
ImFont* pSecondFont = nullptr;

ImFont* boldMenuFont = nullptr;
ImFont* menuFont = nullptr;
ImFont* controlFont = nullptr;
ImFont* tabFont = nullptr;
ImFont* keybindsFont = nullptr;

using Vec4 = Vector4D;
using Vec3 = Vector;
using Vec2 = Vector2D;

inline ImVec2 ToImVec2( const Vector2D& vec ) {
   return ImVec2( vec.x, vec.y );
}

inline Vector2D ToVector2D( const ImVec2& vec ) {
   return Vector2D( vec.x, vec.y );
}

class DX9Render : public Render {
   std::mutex render_mutex;

   size_t current_font = 0u;

   ImDrawList* m_draw_list;
   ImDrawList* m_render_draw_list;
   std::mutex m_render_mutex;

   ImGuiIO* m_io;

public:
   std::vector< ImFont* > fonts;

   IDirect3DDevice9* device = nullptr;

   DX9Render( );
   virtual ~DX9Render( ) {
   };

   virtual void Initialize( IDirect3DDevice9* pDevice );
   virtual void BeginScene( );
   virtual void EndScene( );
   virtual void RenderScene( );
   virtual void OnReset( ) {

   }

   virtual void SetTextFont( size_t font ) {
	  current_font = font;
   }

   virtual Vector2D GetScreenSize( );
   virtual Vector2D CalcTextSize( const char* text );
   virtual void AddTextW( Vector2D pos, ColorU32 color, int flags, wchar_t* text, ... );
   virtual void AddText( Vector2D point, ColorU32 color, int flags, const char* format, ... );
   virtual void AddText( float x, float y, ColorU32 color, int flags, const char* format, ... );
   virtual void AddChar( Vector2D point, ColorU32 color, int flags, const wchar_t symbol );
   virtual void AddRect( const Vector2D& a, float w, float h, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0, float thickness = 1.0f );

   virtual void AddLine( const Vector2D& a, const Vector2D& b, ColorU32 col, float thickness = 1.0f );
   virtual void AddRect( const Vector2D& a, const Vector2D& b, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0, float thickness = 1.0f );
   virtual void AddRectFilled( const Vector2D& a, const Vector2D& b, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0 );
   virtual void AddRectFilledMultiColor( const Vector2D& a, const Vector2D& b, ColorU32 col_upr_left, ColorU32 col_upr_right, ColorU32 col_bot_right, ColorU32 col_bot_left );
   virtual void AddQuad( const Vector2D& a, const Vector2D& b, const Vector2D& c, const Vector2D& d, ColorU32 col, float thickness = 1.0f );
   virtual void AddQuadFilled( const Vector2D& a, const Vector2D& b, const Vector2D& c, const Vector2D& d, ColorU32 col );
   virtual void AddTriangle( const Vector2D& a, const Vector2D& b, const Vector2D& c, ColorU32 col, float thickness = 1.0f );
   virtual void AddTriangleFilled( const Vector2D& a, const Vector2D& b, const Vector2D& c, ColorU32 col );
   virtual void AddTriangleMultiColor( const Vector2D& a, const Vector2D& b, const Vector2D& c, ColorU32 a_col, ColorU32 b_col, ColorU32 c_col );
   virtual void AddCircle( const Vector2D& centre, float radius, ColorU32 col, int32_t num_segments = 12, float thickness = 1.0f );
   virtual void AddCircleFilled( const Vector2D& centre, float radius, ColorU32 col, int32_t num_segments = 12 );
   virtual void AddPolyline( const Vector2D* points, const int32_t num_points, ColorU32 col, bool closed, float thickness );
   virtual void AddConvexPolyFilled( const Vector2D* points, const int32_t num_points, ColorU32 col );
   virtual void AddBezierCurve( const Vector2D& pos0, const Vector2D& cp0, const Vector2D& cp1, const Vector2D& pos1, ColorU32 col, float thickness, int32_t num_segments = 0 );
   virtual void AddRectFilled( const Vector2D& a, float w, float h, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0 ) {
	  AddRectFilled( a, a + Vector2D( w, h ), col, rounding, rounding_corners_flags );
   };
   //virtual void AddRectTextured( const Vector2D& a, float w, float h, ColorU32 col, float rounding = 0.0f, int32_t rounding_corners_flags = ~0, float thickness = 1.0f );

   virtual void SetScissorsRect( const Vector4D& rect );
   virtual void RestoreScissorsRect( );
};

DX9Render* instance = new DX9Render( );
Render* Render::Get( ) {
   return instance;
}

DX9Render::DX9Render( ) {
}

#ifdef _DEBUG
char BufferName[ 4096 ] = { '\0' };
int FontWeight = 0;
int FontSize = 0;
int FontFlags = 0;
bool FontMax = false;
int CurrentRenderFont = 0;
std::vector<std::string> fonts_names;
#endif

void DX9Render::Initialize( IDirect3DDevice9* Device ) {
   this->device = Device;

   // init imgui
   ImGui::CreateContext( );
   ImGui_ImplWin32_Init( Source::hWindow );
   ImGui_ImplDX9_Init( device );

   m_io = &ImGui::GetIO( );

   // add font for rendering

   // init menu fonts
   ImFontConfig* cfg = new ImFontConfig( );
   cfg->PixelSnapH = true;
   cfg->OversampleH = cfg->OversampleV = 1;
   cfg->RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
   pMainFont = m_io->Fonts->AddFontFromFileTTF( XorStr( "C:\\Windows\\Fonts\\Tahoma.ttf" ), 16.0f, cfg, m_io->Fonts->GetGlyphRangesCyrillic( ) );

   cfg = new ImFontConfig( );
   cfg->PixelSnapH = true;
   cfg->OversampleH = cfg->OversampleV = 1;
   cfg->RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
   pMenuFont = m_io->Fonts->AddFontFromFileTTF( XorStr( "C:\\Windows\\Fonts\\Verdana.ttf" ), 12.0f, cfg, m_io->Fonts->GetGlyphRangesCyrillic( ) );

   cfg = new ImFontConfig( );
   cfg->PixelSnapH = true;
   cfg->OversampleH = cfg->OversampleV = 1;
   cfg->RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
   //pMainCapsFont = m_io->Fonts->AddFontFromFileTTF( XorStr( "C:\\Users\\ikfakof\\Documents\\Work\\Programming\\new indigo source\\Raleway-Regular.ttf" ), 24.0f, cfg, m_io->Fonts->GetGlyphRangesCyrillic());
   pMainCapsFont = m_io->Fonts->AddFontFromFileTTF( XorStr( "C:\\Windows\\Fonts\\Verdana.ttf" ), 24.0f, cfg, m_io->Fonts->GetGlyphRangesCyrillic( ) );

   cfg = new ImFontConfig( );
   cfg->PixelSnapH = true;
   cfg->OversampleH = cfg->OversampleV = 1;
   cfg->RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
   //  pSecondFont = m_io->Fonts->AddFontFromFileTTF( XorStr( "C:\\Users\\ikfakof\\Documents\\Work\\Programming\\new indigo source\\Raleway-Regular.ttf" ), 16.0f, cfg, m_io->Fonts->GetGlyphRangesCyrillic());
   pSecondFont = m_io->Fonts->AddFontFromFileTTF( XorStr( "C:\\Windows\\Fonts\\Verdana.ttf" ), 16.0f, cfg, m_io->Fonts->GetGlyphRangesCyrillic( ) );
   
   //fg = new ImFontConfig( );
   //fg->PixelSnapH = true;
   //fg->OversampleH = cfg->OversampleV = 1;
   //fg->RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
   //abFont = m_io->Fonts->AddFontFromMemoryTTF( &tabFont, sizeof( tabFont ), 12, cfg, m_io->Fonts->GetGlyphRangesCyrillic( ) );
   //
   //fg = new ImFontConfig( );
   //fg->PixelSnapH = true;
   //fg->OversampleH = cfg->OversampleV = 1;
   //fg->RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
   //oldMenuFont = m_io->Fonts->AddFontFromMemoryCompressedTTF( boldMenuFont, sizeof boldMenuFont, 11, cfg, m_io->Fonts->GetGlyphRangesCyrillic( ) );
   //
   //fg = new ImFontConfig( );
   //fg->PixelSnapH = true;
   //fg->OversampleH = cfg->OversampleV = 1;
   //fg->RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
   //enuFont = m_io->Fonts->AddFontFromMemoryCompressedTTF( menuFont, sizeof menuFont, 11, cfg, m_io->Fonts->GetGlyphRangesCyrillic( ) );
   //
   //fg = new ImFontConfig( );
   //fg->PixelSnapH = true;
   //fg->OversampleH = cfg->OversampleV = 1;
   //fg->RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
   //ontrolFont = m_io->Fonts->AddFontFromMemoryCompressedTTF( controlFont, sizeof controlFont, 12, cfg, m_io->Fonts->GetGlyphRangesCyrillic( ) );
   //
   //fg = new ImFontConfig( );
   //fg->PixelSnapH = true;
   //fg->OversampleH = cfg->OversampleV = 1;
   //fg->RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
   //eybindsFont = m_io->Fonts->AddFontFromMemoryTTF( keybindsFont, sizeof keybindsFont, 10.f, cfg, m_io->Fonts->GetGlyphRangesCyrillic( ) );

   static const ImWchar ranges[] =
   {
	  0x0020, 0x00FF, // Basic Latin + Latin Supplement
	  0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
	  0x2DE0, 0x2DFF, // Cyrillic Extended-A
	  0xA640, 0xA69F, // Cyrillic Extended-B
	  0x3131, 0x3163, // Korean alphabets
	  0xAC00, 0xD79D, // Korean characters
	  0,
   };

   std::tuple<const char*, float, const ImWchar*> render_fonts[] =
   {
	  std::make_tuple( XorStr( "verdana.ttf" ), 12.0f, ranges ),
	  std::make_tuple( XorStr( "verdana.ttf" ), 16.0f, ranges ),
	  std::make_tuple( XorStr( "verdanab.ttf" ), 30.0f, m_io->Fonts->GetGlyphRangesCyrillic( ) ),
	  std::make_tuple( XorStr( "verdana.ttf" ), 25.0f,  m_io->Fonts->GetGlyphRangesCyrillic( ) ),
   };

   for ( auto font : render_fonts ) {
	  std::string path = std::string( XorStr( "C:\\Windows\\Fonts\\" ) );
	  path += std::get<0>( font );

	  cfg = new ImFontConfig( );
	  cfg->PixelSnapH = true;
	  cfg->OversampleH = cfg->OversampleV = 1;
	  cfg->RasterizerFlags |= ImGuiFreeType::MonoHinting;

	  auto im_font = m_io->Fonts->AddFontFromFileTTF( path.c_str( ), std::get<1>( font ), cfg, std::get<2>( font ) );
	  this->fonts.push_back( im_font );
   }

   cfg = new ImFontConfig( );
   cfg->PixelSnapH = true;
   cfg->OversampleH = cfg->OversampleV = 1;
   cfg->RasterizerMultiply = 2.0f;
   cfg->RasterizerFlags |= ImGuiFreeType::MonoHinting;
   auto skeet_font = m_io->Fonts->AddFontFromMemoryTTF( SkeetFont, sizeof( SkeetFont ), 8.0f, cfg );

   cfg = new ImFontConfig( );
   cfg->PixelSnapH = true;
   cfg->OversampleH = cfg->OversampleV = 1;
   cfg->RasterizerMultiply = 2.0f;
   cfg->RasterizerFlags |= ImGuiFreeType::MonoHinting;
   auto porter_font = m_io->Fonts->AddFontFromMemoryTTF( PorterFont, sizeof( PorterFont ), 24.0f, cfg );

   cfg = new ImFontConfig( );
   cfg->PixelSnapH = true;
   cfg->OversampleH = cfg->OversampleV = 1;
   cfg->RasterizerMultiply = 2.0f;
   cfg->RasterizerFlags |= ImGuiFreeType::Bold;
   cfg->RasterizerFlags |= ImGuiFreeType::MonoHinting;
   auto skeet_indicator = m_io->Fonts->AddFontFromFileTTF( XorStr( "C:\\Windows\\Fonts\\verdanab.ttf" ), 22.0f, cfg, m_io->Fonts->GetGlyphRangesCyrillic( ) );

#if 0
   static const ImWchar ranges[] =
   {
	  0x0020, 0x00FF, // Basic Latin + Latin Supplement
	  0xE000, 0xE000 + 523, // All icons
	  0,
};
#endif

   cfg = new ImFontConfig( );
   cfg->PixelSnapH = true;
   cfg->OversampleH = cfg->OversampleV = 1;
   cfg->RasterizerMultiply = 2.0f;
   cfg->RasterizerFlags |= ImGuiFreeType::MonoHinting;
   auto csgo_icons_font = m_io->Fonts->AddFontFromMemoryCompressedTTF( Icons_compressed_data, Icons_compressed_size, 14.0f, cfg );

   this->fonts.push_back( skeet_font );
   this->fonts.push_back( porter_font );
   this->fonts.push_back( csgo_icons_font );
   this->fonts.push_back( skeet_indicator );
   ImGuiFreeType::BuildFontAtlas( m_io->Fonts, 0 );

   auto shared_data = ImGui::GetDrawListSharedData( );
   m_draw_list = new ImDrawList( shared_data );
   m_render_draw_list = new ImDrawList( shared_data );
}

#ifdef _DEBUG
void AddFontNew( ) {
   // add font for rendering
#if 0
   ImFontConfig * cfg = new ImFontConfig( );
   cfg->PixelSnapH = true;
   cfg->OversampleH = cfg->OversampleV = 1;

   ( ( DX9Render* ) Render::Get( ) )->fonts;
   fonts_names.push_back( BufferName );
#endif
}
#endif

void DX9Render::BeginScene( ) {
   m_draw_list->Clear( );
   m_draw_list->PushClipRectFullScreen( );
   m_draw_list->PushTextureID( m_io->Fonts->TexID );

#if 0
   float i = 20.0f;
   this->SetTextFont( 0 );
   for ( const auto& item : weapon_skins ) {
	  if ( !item.group )
		 continue;

	  this->AddText( Vector2D( 400.0f, i ), 0xFFFFFFFF, OUTLINED, u8XorStr( "%s | %s" ), item.display_name.c_str( ), name );
	  i += 12.0f;
   }

   //auto sample_text = XorStr( "123456789 Resolver Rapid Fire EeXomi Hello, world! FT HK Glock M4A1 Outline Drop Shadow" );
   // this->AddText( Vector2D( 400.0f, 200.0f ), 0xFFFFFFFF, OUTLINED, sample_text );
	//this->AddText( Vector2D( 400.0f, 220.0f ), 0xFFFFFFFF, DROP_SHADOW, sample_text );
   // this->AddText( Vector2D( 400.0f, 240.0f ), 0xFFFFFFFF, 0, sample_text );
#endif
}

void DX9Render::EndScene( ) {
   m_render_mutex.lock( );
   {
	  m_draw_list->PopTextureID( );

	  // swap buffers
	  std::swap( m_draw_list, m_render_draw_list );
   }
   m_render_mutex.unlock( );
}

void DX9Render::RenderScene( ) {
   m_render_mutex.lock( );
   {
	  // check if have primitives to draw
	  if ( m_render_draw_list->IdxBuffer.Size > 0 && m_render_draw_list->VtxBuffer.Size > 0 ) {
		 // TODO: render both menu and custom render draw data in one ImGui_ImplDX9_RenderDrawData call

		 // setup draw data
		 ImDrawData data;
		 data.CmdLists = &m_render_draw_list;
		 data.CmdListsCount = 1;
		 data.DisplaySize = m_io->DisplaySize;
		 data.DisplayPos = ImVec2( 0.0, 0.0f );
		 data.TotalIdxCount = m_render_draw_list->IdxBuffer.Size;
		 data.TotalVtxCount = m_render_draw_list->VtxBuffer.Size;
		 data.Valid = true;

		 ImGui_ImplDX9_RenderDrawData( &data );
	  }
   }
   m_render_mutex.unlock( );
}

Vector2D DX9Render::GetScreenSize( ) {
   return Vector2D( m_io->DisplaySize.x, m_io->DisplaySize.y );
}

Vector2D DX9Render::CalcTextSize( const char* text ) {
   const auto font = fonts[ current_font ];
   ImGuiContext& g = *GImGui;

   const float font_size = g.FontSize;
   ImVec2 text_size = font->CalcTextSizeA( font->FontSize, FLT_MAX, 0.0f, text, nullptr, NULL );

   // Round
   text_size.x = ( float ) ( int ) ( text_size.x + 0.95f );

   return ToVector2D( text_size );
}

void DX9Render::AddTextW( Vector2D pos, ColorU32 color, int flags, wchar_t* text, ... ) {
   static const auto MAX_BUFFER_SIZE = 4096;
   static wchar_t buffer[ MAX_BUFFER_SIZE ] = L"";

   va_list va;
   va_start( va, text );
   wvsprintfW( buffer, text, va );
   va_end( va );

   auto len = lstrlenW( buffer );
   auto text_end = buffer + len;

   if ( buffer == text_end )
	  return;

   auto font = fonts[ current_font ];

   auto font_size = font->FontSize;

   auto draw_pos = ToImVec2( pos );
   if ( flags & ( CENTER_X | CENTER_Y | ALIGN_RIGHT | ALIGN_BOTTOM ) ) {
	  auto size = font->CalcTextSizeA( font->FontSize, FLT_MAX, 0.0f, ( const char* ) buffer, ( const char* ) text_end );

	  if ( flags & CENTER_X )
		 draw_pos.x -= size.x * 0.5f;
	  else if ( flags & ALIGN_RIGHT )
		 draw_pos.x -= size.x;

	  if ( flags & CENTER_Y )
		 draw_pos.y -= size.y * 0.5f;
	  else if ( flags & ALIGN_BOTTOM )
		 draw_pos.y -= size.y;
   }

   // stub
   static ImVec4 clip_rect = ImVec4( -FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX );

   // imo ghetto solution, look how vgui surfaces doing it
   if ( flags & OUTLINED ) {
	  //auto shadow_color = ( ( ( color & 0xFF000000 ) >> 24 ) / 2 ) << 24;
	  auto shadow_color = color & 0xFF000000;

	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x + 1.0f, draw_pos.y + 1.0f ), shadow_color, clip_rect, ( const char* ) buffer, ( const char* ) text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x - 1.0f, draw_pos.y - 1.0f ), shadow_color, clip_rect, ( const char* ) buffer, ( const char* ) text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x + 1.0f, draw_pos.y - 1.0f ), shadow_color, clip_rect, ( const char* ) buffer, ( const char* ) text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x - 1.0f, draw_pos.y + 1.0f ), shadow_color, clip_rect, ( const char* ) buffer, ( const char* ) text_end );

	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x + 1.0f, draw_pos.y ), shadow_color, clip_rect, ( const char* ) buffer, ( const char* ) text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x - 1.0f, draw_pos.y ), shadow_color, clip_rect, ( const char* ) buffer, ( const char* ) text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x, draw_pos.y + 1.0f ), shadow_color, clip_rect, ( const char* ) buffer, ( const char* ) text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x, draw_pos.y - 1.0f ), shadow_color, clip_rect, ( const char* ) buffer, ( const char* ) text_end );
   } else if ( flags & DROP_SHADOW ) {
	  // auto shadow_color = static_cast< float >( ( color & 0xFF000000 ) >> 24 );
	  // shadow_color *= 0.8f;

	   //FloatColor shadow = FloatColor( 0.0f, 0.0f, 0.0f, shadow_color / 255.0f );

	  auto shadow_color = color & 0xFF000000;
	  font->RenderText( this->m_draw_list, font_size, draw_pos + ImVec2( 1.0f, 1.0f ), shadow_color, clip_rect, ( const char* ) buffer, ( const char* ) text_end );
   }

   font->RenderText( this->m_draw_list, font_size, draw_pos, color, clip_rect, ( const char* ) buffer, ( const char* ) text_end );
}

void DX9Render::AddText( Vector2D pos, ColorU32 color, int flags, const char* text, ... ) {
   static const auto MAX_BUFFER_SIZE = 4096;
   static char buffer[ MAX_BUFFER_SIZE ] = "";

   va_list va;
   va_start( va, text );
   vsnprintf_s( buffer, MAX_BUFFER_SIZE, text, va );
   va_end( va );

   auto len = strlen( buffer );
   auto text_end = buffer + len;

   if ( buffer == text_end )
	  return;

   auto font = fonts[ current_font ];

   auto font_size = font->FontSize;

   auto draw_pos = ToImVec2( pos );
   if ( flags & ( CENTER_X | CENTER_Y | ALIGN_RIGHT | ALIGN_BOTTOM ) ) {
	  auto size = font->CalcTextSizeA( font->FontSize, FLT_MAX, 0.0f, buffer, text_end );

	  if ( flags & CENTER_X )
		 draw_pos.x -= size.x * 0.5f;
	  else if ( flags & ALIGN_RIGHT )
		 draw_pos.x -= size.x;

	  if ( flags & CENTER_Y )
		 draw_pos.y -= size.y * 0.5f;
	  else if ( flags & ALIGN_BOTTOM )
		 draw_pos.y -= size.y;
   }

   // stub
   static ImVec4 clip_rect = ImVec4( -FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX );

   // imo ghetto solution, look how vgui surfaces doing it
   if ( flags & OUTLINED ) {
	  //auto shadow_color = ( ( ( color & 0xFF000000 ) >> 24 ) / 2 ) << 24;
	  auto shadow_color = color & 0xFF000000;

	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x + 1.0f, draw_pos.y + 1.0f ), shadow_color, clip_rect, buffer, text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x - 1.0f, draw_pos.y - 1.0f ), shadow_color, clip_rect, buffer, text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x + 1.0f, draw_pos.y - 1.0f ), shadow_color, clip_rect, buffer, text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x - 1.0f, draw_pos.y + 1.0f ), shadow_color, clip_rect, buffer, text_end );

	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x + 1.0f, draw_pos.y ), shadow_color, clip_rect, buffer, text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x - 1.0f, draw_pos.y ), shadow_color, clip_rect, buffer, text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x, draw_pos.y + 1.0f ), shadow_color, clip_rect, buffer, text_end );
	  font->RenderText( this->m_draw_list, font_size, ImVec2( draw_pos.x, draw_pos.y - 1.0f ), shadow_color, clip_rect, buffer, text_end );
   } else if ( flags & DROP_SHADOW ) {
	  // auto shadow_color = static_cast< float >( ( color & 0xFF000000 ) >> 24 );
	  // shadow_color *= 0.8f;

	   //FloatColor shadow = FloatColor( 0.0f, 0.0f, 0.0f, shadow_color / 255.0f );

	  auto shadow_color = color & 0xFF000000;
	  font->RenderText( this->m_draw_list, font_size, draw_pos + ImVec2( 1.0f, 1.0f ), shadow_color, clip_rect, buffer, text_end );
   }

   font->RenderText( this->m_draw_list, font_size, draw_pos, color, clip_rect, buffer, text_end );
}

void DX9Render::AddChar( Vector2D point, ColorU32 color, int flags, const wchar_t symbol ) {
}

void DX9Render::AddText( float x, float y, ColorU32 color, int flags, const char* format, ... ) {
   //int* ban = nullptr;
   //*ban = 1337;
}

void DX9Render::AddRect( const Vector2D& a, float w, float h, ColorU32 col, float rounding, int32_t rounding_corners_flags, float thickness ) {
   AddRect( a, { a.x + w, a.y + h }, col, rounding, rounding_corners_flags, thickness );
}

void DX9Render::AddLine( const Vector2D& from, const Vector2D& to, ColorU32 color, float thickness ) {
   this->m_draw_list->AddLine( ToImVec2( from ), ToImVec2( to ), color, thickness );
}

void DX9Render::AddRect( const Vector2D& min, const Vector2D& max, ColorU32 color, float rounding, int32_t rounding_corners_flags, float thickness ) {
   this->m_draw_list->AddRect( ToImVec2( min ), ToImVec2( max ), color, rounding, rounding_corners_flags, thickness );
}

void DX9Render::AddRectFilled( const Vector2D& min, const Vector2D& max, ColorU32 color, float rounding, int32_t rounding_corners ) {
   this->m_draw_list->AddRectFilled( ToImVec2( min ), ToImVec2( max ), color, rounding, rounding_corners );
}

void DX9Render::AddRectFilledMultiColor( const Vector2D& min, const Vector2D& max, ColorU32 col_upr_left, ColorU32 col_upr_right, ColorU32 col_bot_right, ColorU32 col_bot_left ) {
   this->m_draw_list->AddRectFilledMultiColor( ToImVec2( min ), ToImVec2( max ), col_upr_left, col_upr_right, col_bot_right, col_bot_left );
}

void DX9Render::AddQuad( const Vector2D& a, const Vector2D& b, const Vector2D& c, const Vector2D& d, ColorU32 color, float strokeWidth ) {
   this->m_draw_list->AddQuad( ToImVec2( a ), ToImVec2( b ), ToImVec2( c ), ToImVec2( d ), color, strokeWidth );
}

void DX9Render::AddQuadFilled( const Vector2D& a, const Vector2D& b, const Vector2D& c, const Vector2D& d, ColorU32 col ) {
   this->m_draw_list->AddQuadFilled( ToImVec2( a ), ToImVec2( b ), ToImVec2( c ), ToImVec2( d ), col );
}

void DX9Render::AddTriangle( const Vector2D& a, const Vector2D& b, const Vector2D& c, ColorU32 col, float thickness ) {
   this->m_draw_list->AddTriangle( ToImVec2( a ), ToImVec2( b ), ToImVec2( c ), col, thickness );
}

void DX9Render::AddTriangleFilled( const Vector2D& a, const Vector2D& b, const Vector2D& c, ColorU32 color ) {
   auto flags = this->m_draw_list->Flags;
   this->m_draw_list->Flags &= ~ImDrawListFlags_AntiAliasedFill;
   this->m_draw_list->AddTriangleFilled( ToImVec2( a ), ToImVec2( b ), ToImVec2( c ), color );
   this->m_draw_list->Flags = flags;
}

void DX9Render::AddTriangleMultiColor( const Vector2D& a, const Vector2D& b, const Vector2D& c, ColorU32 a_col, ColorU32 b_col, ColorU32 c_col ) {
   this->m_draw_list->AddTriangleFilled( ToImVec2( a ), ToImVec2( b ), ToImVec2( c ), a_col );
}

void DX9Render::AddCircle( const Vector2D& pos, float radius, ColorU32 color, int32_t segments, float strokeWidth ) {
   this->m_draw_list->AddCircle( ToImVec2( pos ), radius, color, segments, strokeWidth );
}

void DX9Render::AddCircleFilled( const Vector2D& centre, float radius, ColorU32 col, int32_t segments ) {
   this->m_draw_list->AddCircleFilled( ToImVec2( centre ), radius, col, segments );
}

void DX9Render::AddPolyline( const Vector2D* points, const int32_t num_points, ColorU32 col, bool closed, float thickness ) {
   this->m_draw_list->AddPolyline( ( ImVec2* ) points, num_points, col, closed, thickness );
}

void DX9Render::AddConvexPolyFilled( const Vector2D* points, const int32_t num_points, ColorU32 col ) {
}

void DX9Render::AddBezierCurve( const Vector2D& pos0, const Vector2D& cp0, const Vector2D& cp1, const Vector2D& pos1, ColorU32 col, float thickness, int32_t num_segments ) {
}

void DX9Render::SetScissorsRect( const Vector4D& rect ) {

}

void DX9Render::RestoreScissorsRect( ) {

}
