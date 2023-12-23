#if 0

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Render.hpp"
#include "MenuV2.hpp"
#include "source.hpp"
#include "Config.hpp"
#include "InputSys.hpp"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"
#include <functional>
#include "KitParser.hpp"
#include "RoundFireBulletsStore.hpp"
#include <initializer_list>
#include <algorithm>
#include "player.hpp"
#include "weapon.hpp"
#include "Displacement.hpp"

#include "imgui_freetype.h"
#include "MenuControls.h"
#include "MenuBackground.h"
#include "Dropdown.h"
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

#include "EventLogger.hpp"
#include <chrono>
#include "Render.hpp"


#define UNLEN 256
IDirect3DStateBlock9* state_block;
bool reverse = false;
int offset = 0;
bool show_popup = false;
static bool save_config = false;
static bool load_config = false;

extern ImFont* pMainFont;
extern  ImFont* pMainCapsFont;
extern  ImFont* pMenuFont;
extern  ImFont* pSecondFont;

extern float WorldExposure;
extern float ModelAmbient;
extern float BloomScale;

namespace Spectrum
{
   // a list of changes that I had to introduce to change the look
   // of the widgets. Collected here as const rather than being
   // magic numbers spread around imgui.cpp and imgui_widgets.cpp.
   const float CHECKBOX_BORDER_SIZE = 2.0f;

   void StyleColorsSpectrum( );

   namespace
   { // Unnamed namespace, since we only use this here. 
	  const unsigned int Color( unsigned int c ) {
		 // add alpha.
		 // also swap red and blue channel for some reason.
		 // todo: figure out why, and fix it.
		 const short a = 0xFF;
		 const short r = ( c >> 16 ) & 0xFF;
		 const short g = ( c >> 8 ) & 0xFF;
		 const short b = ( c >> 0 ) & 0xFF;
		 return( a << 24 )
			| ( r << 0 )
			| ( g << 8 )
			| ( b << 16 );
	  }
   }
   // all colors are from http://spectrum.corp.adobe.com/color.html

   inline unsigned int color_alpha( unsigned int alpha, unsigned int c ) {
	  return ( ( alpha & 0xFF ) << 24 ) | ( c & 0x00FFFFFF );
   }

   namespace Static
   { // static colors
	  const unsigned int NONE = 0x00000000; // transparent
	  const unsigned int WHITE = Color( 0xFFFFFF );
	  const unsigned int BLACK = Color( 0x000000 );
	  const unsigned int GRAY200 = Color( 0xF4F4F4 );
	  const unsigned int GRAY300 = Color( 0xEAEAEA );
	  const unsigned int GRAY400 = Color( 0xD3D3D3 );
	  const unsigned int GRAY500 = Color( 0xBCBCBC );
	  const unsigned int GRAY600 = Color( 0x959595 );
	  const unsigned int GRAY700 = Color( 0x767676 );
	  const unsigned int GRAY800 = Color( 0x505050 );
	  const unsigned int GRAY900 = Color( 0x323232 );
	  const unsigned int BLUE400 = Color( 0x378EF0 );
	  const unsigned int BLUE500 = Color( 0x2680EB );
	  const unsigned int BLUE600 = Color( 0x1473E6 );
	  const unsigned int BLUE700 = Color( 0x0D66D0 );
	  const unsigned int RED400 = Color( 0xEC5B62 );
	  const unsigned int RED500 = Color( 0xE34850 );
	  const unsigned int RED600 = Color( 0xD7373F );
	  const unsigned int RED700 = Color( 0xC9252D );
	  const unsigned int ORANGE400 = Color( 0xF29423 );
	  const unsigned int ORANGE500 = Color( 0xE68619 );
	  const unsigned int ORANGE600 = Color( 0xDA7B11 );
	  const unsigned int ORANGE700 = Color( 0xCB6F10 );
	  const unsigned int GREEN400 = Color( 0x33AB84 );
	  const unsigned int GREEN500 = Color( 0x2D9D78 );
	  const unsigned int GREEN600 = Color( 0x268E6C );
	  const unsigned int GREEN700 = Color( 0x12805C );
   }

   const unsigned int GRAY50 = Color( 0x252525 );
   const unsigned int GRAY75 = Color( 0x2F2F2F );
   const unsigned int GRAY100 = Color( 0x323232 );
   const unsigned int GRAY200 = Color( 0x393939 );
   const unsigned int GRAY300 = Color( 0x3E3E3E );
   const unsigned int GRAY400 = Color( 0x4D4D4D );
   const unsigned int GRAY500 = Color( 0x5C5C5C );
   const unsigned int GRAY600 = Color( 0x7B7B7B );
   const unsigned int GRAY700 = Color( 0x999999 );
   const unsigned int GRAY800 = Color( 0xCDCDCD );
   const unsigned int GRAY900 = Color( 0xFFFFFF );
   const unsigned int BLUE400 = Color( 0x2680EB );
   const unsigned int BLUE500 = Color( 0x378EF0 );
   const unsigned int BLUE600 = Color( 0x4B9CF5 );
   const unsigned int BLUE700 = Color( 0x5AA9FA );
   const unsigned int RED400 = Color( 0xE34850 );
   const unsigned int RED500 = Color( 0xEC5B62 );
   const unsigned int RED600 = Color( 0xF76D74 );
   const unsigned int RED700 = Color( 0xFF7B82 );
   const unsigned int ORANGE400 = Color( 0xE68619 );
   const unsigned int ORANGE500 = Color( 0xF29423 );
   const unsigned int ORANGE600 = Color( 0xF9A43F );
   const unsigned int ORANGE700 = Color( 0xFFB55B );
   const unsigned int GREEN400 = Color( 0x2D9D78 );
   const unsigned int GREEN500 = Color( 0x33AB84 );
   const unsigned int GREEN600 = Color( 0x39B990 );
   const unsigned int GREEN700 = Color( 0x3FC89C );
   const unsigned int INDIGO400 = Color( 0x6767EC );
   const unsigned int INDIGO500 = Color( 0x7575F1 );
   const unsigned int INDIGO600 = Color( 0x8282F6 );
   const unsigned int INDIGO700 = Color( 0x9090FA );
   const unsigned int CELERY400 = Color( 0x44B556 );
   const unsigned int CELERY500 = Color( 0x4BC35F );
   const unsigned int CELERY600 = Color( 0x51D267 );
   const unsigned int CELERY700 = Color( 0x58E06F );
   const unsigned int MAGENTA400 = Color( 0xD83790 );
   const unsigned int MAGENTA500 = Color( 0xE2499D );
   const unsigned int MAGENTA600 = Color( 0xEC5AAA );
   const unsigned int MAGENTA700 = Color( 0xF56BB7 );
   const unsigned int YELLOW400 = Color( 0xDFBF00 );
   const unsigned int YELLOW500 = Color( 0xEDCC00 );
   const unsigned int YELLOW600 = Color( 0xFAD900 );
   const unsigned int YELLOW700 = Color( 0xFFE22E );
   const unsigned int FUCHSIA400 = Color( 0xC038CC );
   const unsigned int FUCHSIA500 = Color( 0xCF3EDC );
   const unsigned int FUCHSIA600 = Color( 0xD951E5 );
   const unsigned int FUCHSIA700 = Color( 0xE366EF );
   const unsigned int SEAFOAM400 = Color( 0x1B959A );
   const unsigned int SEAFOAM500 = Color( 0x20A3A8 );
   const unsigned int SEAFOAM600 = Color( 0x23B2B8 );
   const unsigned int SEAFOAM700 = Color( 0x26C0C7 );
   const unsigned int CHARTREUSE400 = Color( 0x85D044 );
   const unsigned int CHARTREUSE500 = Color( 0x8EDE49 );
   const unsigned int CHARTREUSE600 = Color( 0x9BEC54 );
   const unsigned int CHARTREUSE700 = Color( 0xA3F858 );
   const unsigned int PURPLE400 = Color( 0x9256D9 );
   const unsigned int PURPLE500 = Color( 0x9D64E1 );
   const unsigned int PURPLE600 = Color( 0xA873E9 );
   const unsigned int PURPLE700 = Color( 0xB483F0 );
}

using KeyBindPair_t = std::pair< int, const char* >;
std::vector< KeyBindPair_t > ButtonNames{
   { 0, ( ( "None" ) ) },
{ 1, ( ( "Left Mouse" ) ) },
{ 2, ( ( "Right Mouse" ) ) },
{ 3, ( ( "Scroll Lock" ) ) },
{ 4, ( ( "Middle Mouse" ) ) },
{ 5, ( ( "X1 Mouse" ) ) },
{ 6, ( ( "X2 Mouse" ) ) },
{ 8, ( ( "Backspace" ) ) },
{ 9, ( ( "Tab" ) ) },
{ 12, ( ( "Num 5" ) ) },
{ 13, ( ( "Enter" ) ) },
{ 16, ( ( "Shift" ) ) },
{ 17, ( ( "Ctrl" ) ) },
{ 18, ( ( "Alt" ) ) },
{ 20, ( ( "Caps Lock" ) ) },
{ 27, ( ( "Esc" ) ) },
{ 32, ( ( "Space" ) ) },
{ 33, ( ( "Num 9" ) ) },
{ 34, ( ( "Num 3" ) ) },
{ 35, ( ( "Num 1" ) ) },
{ 36, ( ( "Num 7" ) ) },
{ 37, ( ( "Num 4" ) ) },
{ 38, ( ( "Num 8" ) ) },
{ 39, ( ( "Num 6" ) ) },
{ 40, ( ( "Num 2" ) ) },
{ 44, ( ( "Sys Req" ) ) },
{ 45, ( ( "Num 0" ) ) },
{ 46, ( ( "Num Del" ) ) },
{ 48, ( ( "0" ) ) },
{ 49, ( ( "1" ) ) },
{ 50, ( ( "2" ) ) },
{ 51, ( ( "3" ) ) },
{ 52, ( ( "4" ) ) },
{ 53, ( ( "5" ) ) },
{ 54, ( ( "6" ) ) },
{ 55, ( ( "7" ) ) },
{ 56, ( ( "8" ) ) },
{ 57, ( ( "9" ) ) },
{ 65, ( ( "A" ) ) },
{ 66, ( ( "B" ) ) },
{ 67, ( ( "C" ) ) },
{ 68, ( ( "D" ) ) },
{ 69, ( ( "E" ) ) },
{ 70, ( ( "F" ) ) },
{ 71, ( ( "G" ) ) },
{ 72, ( ( "H" ) ) },
{ 73, ( ( "I" ) ) },
{ 74, ( ( "J" ) ) },
{ 75, ( ( "K" ) ) },
{ 76, ( ( "L" ) ) },
{ 77, ( ( "M" ) ) },
{ 78, ( ( "N" ) ) },
{ 79, ( ( "O" ) ) },
{ 80, ( ( "P" ) ) },
{ 81, ( ( "Q" ) ) },
{ 82, ( ( "R" ) ) },
{ 83, ( ( "S" ) ) },
{ 84, ( ( "T" ) ) },
{ 85, ( ( "U" ) ) },
{ 86, ( ( "V" ) ) },
{ 87, ( ( "W" ) ) },
{ 88, ( ( "X" ) ) },
{ 89, ( ( "Y" ) ) },
{ 90, ( ( "Z" ) ) },
{ 96, ( ( "Num 0" ) ) },
{ 97, ( ( "Num 1" ) ) },
{ 98, ( ( "Num 2" ) ) },
{ 99, ( ( "Num 3" ) ) },
{ 100, ( ( "Num 4" ) ) },
{ 101, ( ( "Num 5" ) ) },
{ 102, ( ( "Num 6" ) ) },
{ 103, ( ( "Num 7" ) ) },
{ 104, ( ( "Num 8" ) ) },
{ 105, ( ( "Num 9" ) ) },
{ 106, ( ( "Num *" ) ) },
{ 107, ( ( "Num +" ) ) },
{ 109, ( ( "Num -" ) ) },
{ 110, ( ( "Num Del" ) ) },
{ 111, ( ( "/" ) ) },
{ 112, ( ( "F1" ) ) },
{ 113, ( ( "F2" ) ) },
{ 114, ( ( "F3" ) ) },
{ 115, ( ( "F4" ) ) },
{ 116, ( ( "F5" ) ) },
{ 117, ( ( "F6" ) ) },
{ 118, ( ( "F7" ) ) },
{ 119, ( ( "F8" ) ) },
{ 120, ( ( "F9" ) ) },
{ 121, ( ( "F10" ) ) },
{ 122, ( ( "F11" ) ) },
{ 123, ( ( "F12" ) ) },
{ 144, ( ( "Pause" ) ) },
{ 145, ( ( "Scroll Lock" ) ) },
{ 161, ( ( "Right Shift" ) ) },
{ 186, ( ( ";" ) ) },
{ 187, ( ( "=" ) ) },
{ 188, ( ( "," ) ) },
{ 189, ( ( "-" ) ) },
{ 190, ( ( "." ) ) },
{ 191, ( ( "/" ) ) },
{ 192, ( ( "`" ) ) },
{ 219, ( ( "[" ) ) },
{ 220, ( ( "\\" ) ) },
{ 221, ( ( "]" ) ) },
{ 222, ( ( "'" ) ) },
{ 226, ( ( "\\" ) ) },
};


namespace ImGui
{
   bool TextSpacingMeme = false;

   void ToClipboard( const char* text ) {
	  if ( OpenClipboard( 0 ) ) {
		 EmptyClipboard( );
		 char* clip_data = ( char* ) ( GlobalAlloc( GMEM_FIXED, MAX_PATH ) );
		 lstrcpy( clip_data, text );
		 SetClipboardData( CF_TEXT, ( HANDLE ) ( clip_data ) );
		 LCID* lcid = ( DWORD* ) ( GlobalAlloc( GMEM_FIXED, sizeof( DWORD ) ) );
		 *lcid = MAKELCID( MAKELANGID( LANG_RUSSIAN, SUBLANG_NEUTRAL ), SORT_DEFAULT );
		 SetClipboardData( CF_LOCALE, ( HANDLE ) ( lcid ) );
		 CloseClipboard( );
	  }
   }

   static auto vector_getter = [] ( void* vec, int idx, const char** out_text ) {
	  auto& vector = *static_cast< std::vector<std::string>* >( vec );
	  if ( idx < 0 || idx >= static_cast< int >( vector.size( ) ) ) { return false; }
	  *out_text = vector.at( idx ).c_str( );
	  return true;
   };

   bool Combo( const char* label, int* currIndex, std::vector<std::string>& values ) {
	  if ( values.empty( ) ) { return false; }
	  return Combo( label, currIndex, vector_getter,
					static_cast< void* >( &values ), values.size( ) );
   }

   bool ListBox( const char* label, int* currIndex, std::vector<std::string>& values, int height_in_items = -1 ) {
	  if ( values.empty( ) ) { return false; }
	  return ListBox( label, currIndex, vector_getter,
					  static_cast< void* >( &values ), values.size( ), height_in_items );
   }

   static bool ListBox( const char* label, int* current_item, std::function<const char* ( int )> lambda, int items_count, int height_in_items ) {
	  return ImGui::ListBox( label, current_item, [] ( void* data, int idx, const char** out_text ) {
		 *out_text = ( *reinterpret_cast< std::function<const char* ( int )>* >( data ) )( idx );
		 return true;
	  }, &lambda, items_count, height_in_items );
   }

   // code just for example
   void AlignToRight( ) {
	  ImGui::SameLine( ImGui::GetWindowContentRegionMax( ).x - 30.0f );
   }

   // Getter for the old Combo() API: const char*[]
   static bool Items_ArrayGetter( void* data, int idx, const char** out_text ) {
	  const char* const* items = ( const char* const* ) data;
	  if ( out_text )
		 *out_text = items[ idx ];
	  return true;
   }

   // Getter for the old Combo() API: XorStr( "item1\0item2\0item3\0" )
   static bool Items_SingleStringGetter( void* data, int idx, const char** out_text ) {
	  // FIXME-OPT: we could pre-compute the indices to fasten this. But only 1 active combo means the waste is limited.
	  const char* items_separated_by_zeros = ( const char* ) data;
	  int items_count = 0;
	  const char* p = items_separated_by_zeros;
	  while ( *p ) {
		 if ( idx == items_count )
			break;
		 p += strlen( p ) + 1;
		 items_count++;
	  }
	  if ( !*p )
		 return false;
	  if ( out_text )
		 *out_text = p;
	  return true;
   }

   // A stands for Aligned
   bool ComboA( const char* label, int* current_item, bool( *items_getter )( void* data, int idx, const char** out_text ), void* data, int items_count, int popup_max_height_in_items = -1 ) {
	  auto size = ImGui::GetContentRegionAvail( ).x;
	  auto pos = ImGui::GetCursorScreenPos( );

	  ImGui::BeginGroup( );
	  ImGui::RenderText( pos, label );

	  auto label_sz = ImGui::CalcTextSize( label );
	  ImGui::Dummy( ImVec2( 0.0f, label_sz.y + 2.0f - ImGui::GetStyle( ).ItemSpacing.y ) );

	  auto hashed = std::string( XorStr( "##" ) ) + label;

	  ImGui::PushItemWidth( size );
	  bool ret = ImGui::Combo( hashed.c_str( ), current_item, items_getter, data, items_count, popup_max_height_in_items );
	  ImGui::PopItemWidth( );
	  ImGui::EndGroup( );

	  return ret;
   }

   bool SliderFloatA( const char* label, float* v, float v_min, float v_max, const char* format = XorStr( "%.3f" ), float power = 1.0f ) {
	  auto pos = ImGui::GetCursorScreenPos( );
	  auto size = ImGui::GetContentRegionAvail( ).x;
	  auto str_label = std::string( label );
	  auto hashed = std::string( XorStr( "##" ) ) + str_label;

	  ImGui::BeginGroup( );

	  ImGui::RenderText( pos, label );

	  static char buffer[ 1024 ];
	  sprintf_s( buffer, format, *v );

	  auto value_size = ImGui::CalcTextSize( buffer );

	  ImGui::RenderText( pos, label );
	  ImGui::RenderText( pos + ImVec2( size, 0.0f ) - ImVec2( value_size.x, 0.0f ) - ImVec2( 1.0f, 0.0f ), buffer );
	  ImGui::Dummy( ImVec2( 0.0f, value_size.y + 2.0f - ImGui::GetStyle( ).ItemSpacing.y ) );

	  ImGui::PushItemWidth( size );
	  //ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 4.0f, -1.0f ) );
	  bool ret = ImGui::SliderFloat( hashed.c_str( ), v, v_min, v_max, "", power );
	  // ImGui::PopStyleVar( );
	  ImGui::PopItemWidth( );

	  ImGui::EndGroup( );

	  return ret;
   }

   bool SliderIntA( const char* label, int* v, int v_min, int v_max, const char* format = XorStr( "%d" ) ) {
	  auto pos = ImGui::GetCursorScreenPos( );
	  auto size = ImGui::GetContentRegionAvail( ).x;
	  auto str_label = std::string( label );
	  auto hashed = std::string( XorStr( "##" ) ) + str_label;

	  ImGui::BeginGroup( );

	  ImGui::RenderText( pos, label );

	  static char buffer[ 1024 ];
	  sprintf_s( buffer, format, *v );

	  auto value_size = ImGui::CalcTextSize( buffer );

	  ImGui::RenderText( pos, label );
	  ImGui::RenderText( pos + ImVec2( size, 0.0f ) - ImVec2( value_size.x, 0.0f ) - ImVec2( 1.0f, 0.0f ), buffer );
	  ImGui::Dummy( ImVec2( 0.0f, value_size.y + 2.0f - ImGui::GetStyle( ).ItemSpacing.y ) );

	  ImGui::PushItemWidth( size );
	  // ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 4.0f, -1.0f ) );
	  bool ret = ImGui::SliderInt( hashed.c_str( ), v, v_min, v_max, "" );
	  //ImGui::PopStyleVar( );
	  ImGui::PopItemWidth( );

	  ImGui::EndGroup( );

	  return ret;
   }

   bool ComboA( const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1 ) {
	  int items_count = 0;
	  const char* p = items_separated_by_zeros;       // FIXME-OPT: Avoid computing this, or at least only when combo is open
	  while ( *p ) {
		 p += strlen( p ) + 1;
		 items_count++;
	  }
	  bool value_changed = ComboA( label, current_item, Items_SingleStringGetter, ( void* ) items_separated_by_zeros, items_count, popup_max_height_in_items );
	  return value_changed;
   }

   bool ComboA( const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1 ) {
	  return ComboA( label, current_item, Items_ArrayGetter, ( void* ) items, items_count, popup_max_height_in_items );
   }

   bool ComboA( const char* label, int* currIndex, std::vector<std::string>& values ) {
	  if ( values.empty( ) ) { return false; }
	  return ComboA( label, currIndex, vector_getter,
					 static_cast< void* >( &values ), values.size( ) );
   }

   bool KeyBind( const char* name, int& keyBind ) {
	  auto result = std::find_if( ButtonNames.begin( ), ButtonNames.end( ), [&] ( const KeyBindPair_t& a ) { return a.first == keyBind; } );
	  if ( result == ButtonNames.end( ) ) {
		 result = ButtonNames.begin( );
		 keyBind = 0;
	  }

	  int key = result - ButtonNames.begin( );
	  auto comboRes = ImGui::Combo(
		 name, &key, [] ( void* data, int32_t idx, const char** out_text ) {
		 *out_text = ButtonNames[ idx ].second;
		 return true;
	  },
		 nullptr, ButtonNames.size( ) );

	  keyBind = ButtonNames[ key ].first;
	  return comboRes;
   }

   IMGUI_API bool TabLabels( const char** tabLabels, int tabSize, int& tabIndex, int* tabOrder, bool sameline = true, float custom_width = 0.0f ) {
	  ImGuiWindow* window = GetCurrentWindow( );
	  if ( window->SkipItems )
		 return false;

	  ImGuiStyle& style = ImGui::GetStyle( );
	  auto mainColor = ImColor( 236, 64, 122, 255 );

	  const ImVec2 itemSpacing = style.ItemSpacing;
	  const ImVec4 color = mainColor;
	  const ImVec4 colorActive = mainColor;
	  const ImVec4 colorHover = mainColor;
	  const ImVec4 colorText = style.Colors[ ImGuiCol_Text ];

	  style.ItemSpacing.x = 0.0f;
	  style.ItemSpacing.y = 0.0f;

	  const ImVec4 colorSelectedTab = ImVec4( color.x, color.y, color.z, color.w * 0.5f );
	  const ImVec4 colorSelectedTabHovered = ImVec4( colorHover.x, colorHover.y, colorHover.z, colorHover.w * 0.5f );
	  const ImVec4 colorSelectedTabText = ImVec4( colorText.x * 0.8f, colorText.y * 0.8f, colorText.z * 0.8f, colorText.w * 0.8f );

	  if ( tabSize > 0 && ( tabIndex < 0 || tabIndex >= tabSize ) ) {
		 if ( !tabOrder )
			tabIndex = 0;
		 else
			tabIndex = -1;
	  }

	  float windowWidth = 0.f, sumX = 0.f;
	  windowWidth = ImGui::GetContentRegionAvailWidth( );

	  if ( custom_width == 0.0f ) {
		 custom_width = ( windowWidth / tabSize );
	  }

	  const bool isMMBreleased = ImGui::IsMouseReleased( 2 );
	  int justClosedTabIndex = -1, newtabIndex = tabIndex;

	  bool selection_changed = false; bool noButtonDrawn = true;
	  bool is_active = false;
	  for ( int j = 0, i; j < tabSize; j++ ) {
		 i = tabOrder ? tabOrder[ j ] : j;
		 if ( i == -1 )
			continue;

		 if ( sumX > 0.f ) {
			sumX += style.ItemSpacing.x;
			sumX += ImGui::CalcTextSize( tabLabels[ i ] ).x + 2.f * style.FramePadding.x;

			if ( sumX > windowWidth )
			   sumX = 0.f;
			else if ( sameline )
			   ImGui::SameLine( );
		 }

		 is_active = !( i != tabIndex );

		 float height = std::fminf( 30.0f, ImGui::GetContentRegionAvail( ).y - 12.0f );

		 // Draw the button
		 ImGui::PushID( i );   // otherwise two tabs with the same name would clash.
		 {
			auto size_arg = ImVec2( custom_width, height );
			const ImVec2 label_size = CalcTextSize( tabLabels[ i ], NULL, true );
			const ImGuiID id = window->GetID( tabLabels[ i ] );
			ImVec2 size = CalcItemSize( size_arg, 0.0f, 0.0f );
			const ImRect bb( window->DC.CursorPos, window->DC.CursorPos + size );
			ItemSize( size );
			if ( !ItemAdd( bb, id ) ) {
			   ImGui::PopID( );
			   style.ItemSpacing = itemSpacing;
			   return false;
			}

			bool hovered, held;

			bool pressed = ButtonBehavior( bb, id, &hovered, &held );

			auto col = g_Vars.misc.menu_ascent;

			if ( pressed ) {
			   selection_changed = ( tabIndex != i ); newtabIndex = i;
			}

			window->DrawList->AddRectFilled( bb.Min, bb.Max, ImColor( 25, 25, 33 ) );
			RenderTextClipped( bb.Min + style.FramePadding, bb.Max - style.FramePadding, tabLabels[ i ], NULL, &label_size, style.ButtonTextAlign, &bb );

			if ( is_active ) {
			   auto a = bb.Min;
			   auto b = bb.Max;
			   b.y -= 1.0f;
			   a.y = b.y;

			   window->DrawList->AddLine( a, b, col, 1.f );
			}

		 }
		 ImGui::PopID( );

		 noButtonDrawn = false;
		 if ( sumX == 0.f )
			sumX = style.WindowPadding.x + ImGui::GetItemRectSize( ).x; // First element of a line
	  }

	  tabIndex = newtabIndex;

	  // Change selected tab when user closes the selected tab
	  if ( tabIndex == justClosedTabIndex && tabIndex >= 0 ) {
		 tabIndex = -1;
		 for ( int j = 0, i; j < tabSize; j++ ) {
			i = tabOrder ? tabOrder[ j ] : j;
			if ( i == -1 )
			   continue;
			tabIndex = i;
			break;
		 }
	  }

	  style.ItemSpacing = itemSpacing;

	  return selection_changed;
   }

   std::string GetNameFromCode( int code ) {
	  auto it = std::find_if( ButtonNames.begin( ), ButtonNames.end( ), [code] ( const KeyBindPair_t& a ) { return a.first == code; } );

	  std::string name;
	  if ( it == ButtonNames.end( ) )
		 name = XorStr( "none" );
	  else
		 name = it->second;

	  return name;
   }

   bool KeyBox( KeyBind_t* bind, int defaultCondition = -1, const ImVec2& size = { 0.f, 0.f } ) {
	  static bool key_input = false;
	  static bool cond_input = false;
	  static KeyBind_t* key_output = nullptr;

	  std::string text = XorStr( "none" );

	  if ( !cond_input && key_input && key_output == bind ) {
		 text = XorStr( "..." );

		 static const int mouse_code_array[ 5 ] = {
			1,
			2,
			4,
			5,
			6,
		 };

		 /*
		 if (msg == WM_LBUTTONUP) { button = 0; }
		 if (msg == WM_RBUTTONUP) { button = 1; }
		 if (msg == WM_MBUTTONUP) { button = 2; }
		 if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
		 io.MouseDown[button] = false;
		 */

		 for ( int i = 0; i < 5; i++ ) {
			if ( IsMouseReleased( i ) ) {
			   key_input = false;
			   bind->key = mouse_code_array[ i ];
			   break;
			}
		 }

		 for ( auto keyPair : ButtonNames ) {
			if ( IsKeyReleased( keyPair.first ) ) {
			   key_input = false;
			   bind->key = keyPair.first;
			   if ( bind->key == VirtualKeys::Escape )
				  bind->key = 0;

			   break;
			}
		 }
	  } else {
		 auto name = GetNameFromCode( bind->key );
		 text = name;
		 std::transform( text.begin( ), text.end( ), text.begin( ), ::tolower );
	  }

	  if ( defaultCondition != -1 ) {
		 bind->cond = defaultCondition;
	  }

	  ImGuiWindow* window = GetCurrentWindow( );
	  if ( !window->SkipItems ) {
		 ImGuiContext& g = *GImGui;
		 const ImGuiStyle& style = g.Style;
		 const ImGuiID id = ( ImGuiID ) ( ImGui::GetID( bind ) );
		 const ImVec2 label_size = CalcTextSize( text.c_str( ), NULL, true );

		 ImVec2 pos = window->DC.CursorPos;
		 pos.y += 3.0f;

		 ImVec2 size = CalcItemSize( ImVec2( 0.0f, 0.0f ), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 1.0f );
		 pos.x -= size.x - ( GetContentRegionAvailWidth( ) - GetFrameHeight( ) * 0.5f ) - 12;

	  #if 0
		 ImRect bb( text_pos, text_pos + text_size );
		 ItemSize( ImRect( text_pos, text_pos + ImVec2( text_size.y + g.Style.FramePadding.y * 1.f, text_size.y + g.Style.FramePadding.y * 1.f ) ), g.Style.FramePadding.y );
	  #endif
		 const ImRect bb( pos, pos + size );

		 bool hovered, held;
		 bool pressed = ButtonBehavior( bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnDoubleClick );

		 ItemSize( size, style.FramePadding.y );
		 if ( ItemAdd( bb, id ) ) {
			// Render cond_input
			RenderNavHighlight( bb, id );

		 #if 1
			const ImU32 col = GetColorU32( ( held && hovered ) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button );
			const ImRect new_bb = ImRect( ImVec2( bb.Min.x - 4, bb.Min.y - 3 ), ImVec2( bb.Max.x - 4, bb.Max.y + 1 ) );
			RenderFrame( new_bb.Min, new_bb.Max, col, true, style.FrameRounding );
		 #endif

			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.f, 1.f, 1.f, 1.0f ) );
			RenderTextClipped( bb.Min - style.FramePadding, bb.Max - style.FramePadding, text.c_str( ),
							   NULL, &label_size, style.ButtonTextAlign, &bb );
			ImGui::PopStyleColor( );

			// Automatically close popups
			//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
			//    CloseCurrentPopup();

			if ( defaultCondition == -1 && cond_input && key_output == bind ) {
			   ImGui::SetNextWindowSize( ImVec2( 70.0f, 0.0f ) );
			   if ( ImGui::BeginPopup( XorStr( "##key_bind_cond" ), ImGuiWindowFlags_AlwaysAutoResize ) ) {
				  const char* current_items[] = {
					 XorStr( "Hold" ),
					 XorStr( "Toggle" ),
					 XorStr( "Always On" )
				  };

				  for ( int i = 0; i < 3; i++ ) {
					 if ( Selectable( current_items[ i ], bind->cond == i, ImGuiSelectableFlags_DontClosePopups ) ) {
						if ( bind->cond == KeyBindType::ALWAYS_ON )
						   bind->enabled = false;

						bind->cond = i;
						cond_input = false;
						key_input = false;
						key_output = nullptr;
						ImGui::CloseCurrentPopup( );
					 }
				  }
				  ImGui::EndPopup( );
			   } else {
				  cond_input = false;
				  key_input = false;
				  key_output = nullptr;
			   }
			} else {


			   if ( defaultCondition == -1 && ImGui::IsItemHovered( ) && g.IO.MouseClicked[ 1 ] ) {
				  ImGui::CloseCurrentPopup( );
				  OpenPopup( XorStr( "##key_bind_cond" ) );

				  key_output = bind;
				  key_input = false;
				  cond_input = true;
				  MarkItemEdited( id );
			   } else {
				  bool hovered, held;
				  bool pressed = ButtonBehavior( bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnRelease );

				  if ( pressed ) {
					 MarkItemEdited( id );

					 cond_input = false;
					 key_input = true;
					 key_output = bind;
				  }
			   }
			}
		 }
	  }
	  return false;
   }

   bool KeyBox( const char* label, KeyBind_t* bind, int defaultCondition = -1, const ImVec2& size = { 0.f, 0.f } ) {
	  KeyBox( bind, defaultCondition, size );
	  return false;
   }

   // Getter for the old Combo() API: const char*[]
   static bool items_getter( void* data, int idx, const char** out_text ) {
	  const char* const* items = ( const char* const* ) data;
	  if ( out_text )
		 *out_text = items[ idx ];
	  return true;
   }

   static float CalcMaxPopupHeightFromItemCount( int items_count ) {
	  ImGuiContext& g = *GImGui;
	  if ( items_count <= 0 )
		 return FLT_MAX;
	  return ( g.FontSize + g.Style.ItemSpacing.y ) * items_count - g.Style.ItemSpacing.y + ( g.Style.WindowPadding.y * 2 );
   }

#if 1
   bool MultiCombo( const char* label, bool* current_items[], const char* items[],
					int items_count, int popup_max_height_in_items = -1, const char* default_preview = XorStr( "..." ) ) {
	  using namespace ImGui;
	  ImGuiContext& g = *GImGui;

	  // Go through all items to obtain the preview string which is a parameter to BeginCombo()
	  char preview_value[ 128 ] = { '\0' };
	  bool first_time = false;
	  for ( int i = 0; i < items_count; ++i ) {
		 if ( *current_items[ i ] ) {
			auto len = strlen( preview_value );
			auto preview_len = strlen( items[ i ] );
			if ( len + preview_len + 2 >= 128 )
			   break;

			sprintf( preview_value + len, !first_time ? XorStr( "%s" ) : XorStr( ", %s" ), items[ i ] );
			first_time = true;
		 }
	  }

	  if ( !first_time )
		 strcpy_s( preview_value, default_preview );

	  // The old Combo() API exposed XorStr( "popup_max_height_in_items" ). The new more general BeginCombo() API doesn't have/need it, but we emulate it here.
	  if ( popup_max_height_in_items != -1 && !( g.NextWindowData.SizeCond ) )
		 SetNextWindowSizeConstraints( ImVec2( 0, 0 ), ImVec2( FLT_MAX, CalcMaxPopupHeightFromItemCount( popup_max_height_in_items ) ) );

	  auto size = ImGui::GetContentRegionAvail( ).x;
	  auto pos = ImGui::GetCursorScreenPos( );

	  ImGui::BeginGroup( );

	  ImGui::RenderText( pos, label );

	  auto label_sz = ImGui::CalcTextSize( label );
	  ImGui::Dummy( ImVec2( 0.0f, label_sz.y + 2.0f - ImGui::GetStyle( ).ItemSpacing.y ) );

	  auto hashed = std::string( XorStr( "##" ) ) + label;

	  ImGui::PushItemWidth( size );
	  if ( !BeginCombo( hashed.c_str( ), preview_value, 0 ) ) {
		 ImGui::EndGroup( );
		 ImGui::PopItemWidth( );
		 return false;
	  }

	  // Display items
	  // FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
	  bool value_changed = false;
	  for ( int i = 0; i < items_count; i++ ) {
		 PushID( ( void* ) ( intptr_t ) i );
		 const bool item_selected = *current_items[ i ];
		 const char* item_text;
		 if ( !items_getter( ( void* ) items, i, &item_text ) )
			item_text = XorStr( "*Unknown item*" );

		 if ( Selectable( item_text, item_selected, ImGuiSelectableFlags_DontClosePopups ) ) {
			value_changed = true;
			*current_items[ i ] = !( *current_items[ i ] );
		 }

		 if ( item_selected )
			SetItemDefaultFocus( );

		 PopID( );
	  }
	  EndCombo( );
	  ImGui::PopItemWidth( );
	  ImGui::EndGroup( );
	  return value_changed;
   }

   void MultiComboBox( const char* label, std::vector<std::string> elements, std::initializer_list<bool*> vars ) {
	  std::string prevValue = XorStr( "..." );

	  if ( ImGui::BeginCombo( label, prevValue.c_str( ) ) ) {
		 std::vector<std::string> vec;

		 for ( size_t i = 0; i < elements.size( ); i++ ) {
			bool* var;
			for ( auto iterator = vars.begin( ); iterator != vars.end( ); ++iterator ) {
			   int index = std::distance( vars.begin( ), iterator );
			   if ( index == i )
				  var = *iterator;
			}

			ImGui::Selectable( elements[ i ].c_str( ), &*var, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups );

			if ( *var )
			   vec.emplace_back( elements[ i ].c_str( ) );
		 }

		 if ( vec.size( ) == 0 )
			prevValue = XorStr( "..." );
		 else {
			for ( size_t i = 0; i < vec.size( ); i++ ) {
			   if ( !( i == vec.size( ) - 1 ) )
				  prevValue += vec.at( i ) + XorStr( ", " );
			   else
				  prevValue += vec.at( i );
			}
		 }

		 ImGui::EndCombo( );
	  }
   }
#endif

   template<size_t N>
   void RenderTabs( const char* ( &names )[ N ], int& activetab, float w, float h, bool sameline ) {
	  for ( auto i = 0; i < N; ++i ) {
		 if ( i == activetab ) {
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.16f, 0.15f, 0.19f, 1.0f ) );
		 } else {
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.13f, 0.12f, 0.16f, 1.0f ) );
		 }

		 if ( ImGui::Button( names[ i ], ImVec2{ w, h } ) ) {
			activetab = i;
		 }

		 ImGui::PopStyleColor( );

		 if ( sameline && i < N - 1 )
			ImGui::SameLine( );
	  }
   }
}

class CMenuV2 : public MenuV2 {
public:
   void MenuRender( IDirect3DDevice9* pDevice ) override;
   void WndProcHandler( ) override;
   void Initialize( IDirect3DDevice9* pDevice ) override;
private:
   float m_alpha = 0.0f;

   std::vector<std::string> cfg_list;
   char config_name[ 256 ] = { "..." };
   int voted_cfg = 0;
};

Encrypted_t<MenuV2> MenuV2::Get( ) {
   static CMenuV2 instance;
   return &instance;
}

void CMenuV2::MenuRender( IDirect3DDevice9* pDevice ) {
   ImGui::GetIO( ).MouseDrawCursor = g_Vars.globals.menuOpen;
   const auto& io = ImGui::GetIO( );
   static int page = 0;

   constexpr auto alpha_freq = 1.0f / 0.2f;
   if ( g_Vars.globals.menuOpen ) {
	  m_alpha += alpha_freq * io.DeltaTime;
   } else {
	  m_alpha -= alpha_freq * io.DeltaTime;
   }

   time_t seconds = time( NULL );
   tm* timeinfo = localtime( &seconds );

   if ( g_Vars.globals.menuOpen || !g_Vars.globals.menuOpen && m_alpha != 0.f ) {
	  ImGuiStyle* style = &ImGui::GetStyle( );

	  m_alpha = Math::Clamp( m_alpha, 0.0f, 1.0f );
	  for ( auto& keybind : g_keybinds ) {
		 if ( keybind->cond == KeyBindType::ALWAYS_ON )
			keybind->enabled = true;
	  }

	  if ( m_alpha <= 0.f )
		 return;

	  auto meme = style->Alpha;
	  style->Alpha = m_alpha;
	  style->ColumnsMinSpacing = 0.0f;

	  {
		 ImVec4* colors = style->Colors;
		 colors[ ImGuiCol_CheckMark ] = ImVec4( g_Vars.misc.menu_ascent.r, g_Vars.misc.menu_ascent.g, g_Vars.misc.menu_ascent.b, 1.00f );
		 colors[ ImGuiCol_SliderGrab ] = ImVec4( g_Vars.misc.menu_ascent.r, g_Vars.misc.menu_ascent.g, g_Vars.misc.menu_ascent.b, 1.00f );
		 colors[ ImGuiCol_SliderGrabActive ] = ImVec4( g_Vars.misc.menu_ascent.r, g_Vars.misc.menu_ascent.g, g_Vars.misc.menu_ascent.b, 1.00f );
		 colors[ ImGuiCol_ResizeGrip ] = ImVec4( g_Vars.misc.menu_ascent.r, g_Vars.misc.menu_ascent.g, g_Vars.misc.menu_ascent.b, 0.25f );
		 colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( g_Vars.misc.menu_ascent.r, g_Vars.misc.menu_ascent.g, g_Vars.misc.menu_ascent.b, 0.67f );
		 colors[ ImGuiCol_ResizeGripActive ] = ImVec4( g_Vars.misc.menu_ascent.r, g_Vars.misc.menu_ascent.g, g_Vars.misc.menu_ascent.b, 0.95f );
	  }

	  ImGui::SetNextWindowPos( ImVec2( ImGui::GetIO( ).DisplaySize.x * 0.5f - 620 * 0.5f,
							   ImGui::GetIO( ).DisplaySize.y * 0.5f - 545 * 0.5f ), ImGuiCond_Once );

	  ImGui::SetNextWindowSize( ImVec2( 620, 545 ), ImGuiCond_Once );
	  ImGui::Begin( "EEXOMI", &g_Vars.globals.menuOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar );
	  {
		 ImColor c = ImColor( 32, 114, 247 );

		 const char* weapon_groups[] = {
		   XorStr( "Pistols" ),
		   XorStr( "Heavy pistols" ),
		   XorStr( "Rifles" ),
		   XorStr( "Snipers" ),
		   XorStr( "Auto snipers" ),
		   XorStr( "Sub machines" ),
		   XorStr( "Heavys" ),
		   XorStr( "Shotguns" )
		 };

		 const char* rage_weapon_groups[] = {
			XorStr( "Default" ),
			XorStr( "Pistols" ),
			XorStr( "Heavy pistols" ),
			XorStr( "Rifles" ),
			XorStr( "Awp" ),
			XorStr( "Scout" ),
			XorStr( "Auto snipers" ),
			XorStr( "Sub machines" ),
			XorStr( "Heavys" ),
			XorStr( "Shotguns" ),
			XorStr( "Taser" ),
		 };

		 const char* rage_select_target_names[] = {
			XorStr( "Lowest hp" ),
			XorStr( "Lowest distance" ),
			XorStr( "Lowest ping" ),
			XorStr( "Highest accuracy" ),
		 };

		 // name and index in config system
		 using group_vector = std::vector< std::pair< std::string, int >>;
		 static group_vector pistols;
		 static group_vector heavy_pistols;
		 static group_vector rifles;
		 static group_vector snipers;
		 static group_vector auto_snipers;
		 static group_vector sub_machines;
		 static group_vector heavys;
		 static group_vector shotguns;

		 CVariables::RAGE* rbot = nullptr;
		 CVariables::LEGIT* lbot = nullptr;
		 group_vector* aim_group = nullptr;

		 auto fill_group_vector = [] ( group_vector& vec, int idx ) {
			vec.push_back( std::make_pair( XorStr( "Group" ), 0 ) );

			for ( auto weapon : weapon_skins ) {
			   if ( weapon.group < 0 )
				  return;

			   if ( weapon.group == idx ) {
				  vec.push_back( std::make_pair( weapon.display_name, weapon.id ) );
			   }
			}
		 };

		 static bool init = true;
		 if ( init ) {
			fill_group_vector( pistols, WEAPONGROUP_PISTOL );
			fill_group_vector( heavy_pistols, WEAPONGROUP_HEAVYPISTOL );
			fill_group_vector( rifles, WEAPONGROUP_RIFLE );
			fill_group_vector( snipers, WEAPONGROUP_SNIPER );
			fill_group_vector( auto_snipers, WEAPONGROUP_AUTOSNIPER );
			fill_group_vector( sub_machines, WEAPONGROUP_SUBMACHINE );
			fill_group_vector( heavys, WEAPONGROUP_HEAVY );
			fill_group_vector( shotguns, WEAPONGROUP_SHOTGUN );

			init = false;
		 }

		 static int current_weapon = 0;
		 static int current_group = 0;
		 static int rage_current_group = 0;

		 // draw header
		 ImGui::BeginGroup( );
		 {
			ImVec2 p = ImGui::GetCursorScreenPos( );

			// logo
			ImGui::PushFont( pMainCapsFont );

			static float c_header_height = 45.0f;
			float logo_height = ImGui::CalcTextSize( "EEXOMI" ).y;
			c_header_height = logo_height + style->WindowPadding.y;

			FloatColor ascent = g_Vars.misc.menu_ascent;
			ascent.a = m_alpha;

			ImGui::GetWindowDrawList( )->AddRectFilled( ImVec2( p.x, p.y + c_header_height ),
														ImVec2( p.x + ImGui::GetContentRegionAvail( ).x + 2.f, p.y - 3 ), ImColor( 30, 30, 39, int( m_alpha * 255.f ) ) );
			ImGui::GetWindowDrawList( )->AddRectFilled( ImVec2( p.x, p.y + c_header_height + 2.0f ),
														ImVec2( p.x + ImGui::GetContentRegionAvail( ).x + 2.f, p.y + c_header_height ),
														ascent );

			//    ImColor( 0.23f, 0.82f, 0.86f, 1.0f )
			// g_Vars.misc.menu_ascent

			ImGui::BeginGroup( );
			{
			   ImGui::Dummy( ImVec2( 5.0f, 0.0f ) );

			   ImGui::SameLine( );

			   ImGui::Text( "EEXOMI" );
			}
			ImGui::EndGroup( );

			ImGui::PopFont( );

			ImGui::SameLine( );

			// tabs
			ImGui::PushFont( pMenuFont );
			float total_tabs_width = ImGui::GetContentRegionAvail( ).x
			   - style->WindowPadding.x;

			const int tabs_count = 7;
			const int tab_pads = 0;
			float tab_width = total_tabs_width / ( tabs_count + tab_pads );

			ImGui::SameLine( );

			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 30 / 255.0f, 30 / 255.0f, 39 / 255.0f, 1.0f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );
			ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0.0f, 6.0f ) );
			ImGui::PushStyleVar( ImGuiStyleVar_ButtonTextAlign, ImVec2( 0.5f, 0.25f ) );
			ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 0.0f );

			if ( ImGui::Button( "Rage", ImVec2( tab_width, c_header_height ) ) ) page = 0; ImGui::SameLine( 0, 0 );
			if ( ImGui::Button( "Anti-Aimbot", ImVec2( tab_width, c_header_height ) ) ) page = 1; ImGui::SameLine( 0, 0 );
			if ( ImGui::Button( "Legit", ImVec2( tab_width, c_header_height ) ) ) page = 2; ImGui::SameLine( 0, 0 );
			if ( ImGui::Button( "Players", ImVec2( tab_width, c_header_height ) ) ) page = 3; ImGui::SameLine( 0, 0 );
			if ( ImGui::Button( "Visuals", ImVec2( tab_width, c_header_height ) ) ) page = 4; ImGui::SameLine( 0, 0 );
			if ( ImGui::Button( "Misc", ImVec2( tab_width, c_header_height ) ) ) page = 5; ImGui::SameLine( 0, 0 );
			if ( ImGui::Button( "Skins", ImVec2( tab_width, c_header_height ) ) ) page = 6; ImGui::SameLine( 0, 0 );

			ImGui::PopStyleVar( 4 );
			ImGui::PopStyleColor( 3 );

			ImGui::PopFont( );

		 }
		 ImGui::EndGroup( );

		 ImGui::Dummy( ImVec2( 0.0f, 0.0f ) );

		 ImGui::PushFont( pMenuFont );

		 auto wnd_size = ImGui::GetWindowSize( );

		 ImVec2 contents;
		 ImVec2 full_window = ImGui::GetContentRegionAvail( );

		 float child_height;

		 DWORD dwFlag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

		 float total_tabs_width;
		 float tab_width;
		 switch ( page ) {
			case 0:
			{
			   ImGui::Columns( 2, NULL, false );
			   ImGui::SetColumnWidth( 0, full_window.x * 0.5f );
			   ImGui::SetColumnWidth( 1, full_window.x );

			   ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, ImVec4( 30 / 255.f, 30 / 255.f, 39 / 255.f, 1.0f ) );
			   ImGui::BeginChild( "Weapon", ImVec2( 0.0f, 160.0f ), true );
			   {
				  ImGui::Text( "Config" );
				  ImGui::Separator( );
				  ImGui::Checkbox( XorStr( "Enabled##yyyy" ), &g_Vars.rage.enabled );
				  ImGui::ComboA( XorStr( "Group" ), &rage_current_group, rage_weapon_groups, ARRAYSIZE( rage_weapon_groups ) );
				  switch ( rage_current_group - 1 ) {
					 case WEAPONGROUP_PISTOL:
					 rbot = &g_Vars.rage_pistols;
					 break;
					 case WEAPONGROUP_HEAVYPISTOL:
					 rbot = &g_Vars.rage_heavypistols;
					 break;
					 case WEAPONGROUP_SUBMACHINE + 1:
					 rbot = &g_Vars.rage_smgs;
					 break;
					 case WEAPONGROUP_RIFLE:
					 rbot = &g_Vars.rage_rifles;
					 break;
					 case WEAPONGROUP_SHOTGUN + 1:
					 rbot = &g_Vars.rage_shotguns;
					 break;
					 case WEAPONGROUP_SNIPER:
					 rbot = &g_Vars.rage_awp;
					 break;
					 case WEAPONGROUP_SNIPER + 1:
					 rbot = &g_Vars.rage_scout;
					 break;
					 case WEAPONGROUP_HEAVY + 1:
					 rbot = &g_Vars.rage_heavys;
					 break;
					 case WEAPONGROUP_AUTOSNIPER + 1:
					 rbot = &g_Vars.rage_autosnipers;
					 break;
					 case WEAPONGROUP_MAX + 1:
					 rbot = &g_Vars.rage_taser;
					 break;
					 default:
					 rbot = &g_Vars.rage_default;
					 break;
				  }

				  ImGui::ComboA( XorStr( "Target selection" ), &rbot->target_selection, rage_select_target_names, ARRAYSIZE( rage_select_target_names ) );

				  //ImGui::Checkbox( XorStr( "Friendly fire" ), &g_Vars.rage.team_check );
				  ImGui::Checkbox( XorStr( "Visual resolver" ), &g_Vars.rage.visual_resolver );
				  ImGui::Checkbox( XorStr( "Predict velocity-modifier" ), &g_Vars.rage.fix_velocity_modifier );
				  //ImGui::Checkbox( XorStr( "Multithreading" ), &g_Vars.rage.rage_multithread );

			   } ImGui::EndChild( );

			   ImGui::BeginChild( "General", ImVec2( 0.0f, 305.0f ), true );
			   {
				  ImGui::Text( "General" );
				  ImGui::Separator( );

				  ImGui::Checkbox( XorStr( "Enabled" ), &rbot->active );
				  ImGui::Checkbox( XorStr( "Silent aim" ), &rbot->silent_aim );
				 // ImGui::Checkbox( XorStr( "On shot" ), &rbot->on_shot_aa );
				  //ImGui::Checkbox( XorStr( "Accurate shot" ), &rbot->improve_accuracy_on_shot );
				  ImGui::Checkbox( XorStr( "Compensate spread" ), &rbot->no_spread );
				  ImGui::Checkbox( XorStr( "Automatic penetration" ), &rbot->autowall );

				  ImGui::SliderFloatA( XorStr( "Minimum hit chance" ), &rbot->hitchance, 0.f, 100.f, XorStr( "%.0f %%" ) );
				  ImGui::SliderFloatA( XorStr( "Accuracy boost" ), &rbot->hitchance_accuracy, 0.f, 100.f, XorStr( "%.0f %%" ) );

				  if ( rbot->autowall )
					 ImGui::SliderFloatA( XorStr( "Minimum damage - Penetration" ), &rbot->min_damage, 1.f, ( rage_current_group - 1 == WEAPONGROUP_SNIPER ) ? 150.0f : 100.f, XorStr( "%.0f hp" ) );
				  ImGui::SliderFloatA( XorStr( "Minimum damage - Visible" ), &rbot->min_damage_visible, 1.f, ( rage_current_group - 1 == WEAPONGROUP_SNIPER ) ? 150.0f : 100.f, XorStr( "%.0f hp" ) );
				  ImGui::Checkbox( XorStr( "Minimum damage override" ), &rbot->min_damage_override );

				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
				  ImGui::KeyBox( "", &g_Vars.rage.key_dmg_override );

				  if ( rbot->min_damage_override ) {
					 ImGui::SliderFloatA( XorStr( "Minimum damage - Override" ), &rbot->min_damage_override_amount, 1.f, 100.f, XorStr( "%.0f hp" ) );
				  }

				  ImGui::Checkbox( XorStr( "Health based override" ), &rbot->health_override );
				  if ( rbot->health_override ) {
					 ImGui::SliderFloatA( XorStr( "Amount##HealthBasedOverride" ), &rbot->health_override_amount, 1.f, 100.f, XorStr( "+%.0fhp" ) );
				  }

				  ImGui::Checkbox( XorStr( "Shot delay" ), &rbot->shotdelay );
				  if ( rbot->shotdelay ) {
					 ImGui::SliderFloatA( XorStr( "Amount##ShotDelayAmount" ), &rbot->shotdelay_amount, 1.f, 100.f, XorStr( "%.0f %%" ) );
				  }

				  const char* auto_stop_type[] = { XorStr( "Off" ), XorStr( "Full Stop" ), XorStr( "Minimal speed" ) };
				  ImGui::ComboA( XorStr( "Automatic stop" ), &rbot->autostop, auto_stop_type, ARRAYSIZE( auto_stop_type ) );
				  if ( rbot->autostop ) {
					 ImGui::Checkbox( XorStr( "Between shots" ), &rbot->between_shots );
				  }

				  ImGui::Checkbox( XorStr( "Automatic scope" ), &rbot->autoscope );
				  ImGui::SliderIntA( XorStr( "Max misses" ), &rbot->max_misses, 1, 6, XorStr( "%d" ) );
			   } ImGui::EndChild( );

			#ifdef BETA_MODE
			   ImGui::Text( XorStr( "eexomi.host [BETA] | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#else 
			   ImGui::Text( XorStr( "eexomi.host v2 | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#endif

			   ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );
			   ImGui::NextColumn( );
			   ImGui::PopStyleVar( );

			   ImGui::BeginChild( "Hitscan", ImVec2( 0.0f, 300.0f ), true );
			   {
				  ImGui::Text( "Hitscan" );
				  ImGui::Separator( );

				  bool* hitboxes[] = {
					 &rbot->head_hitbox	,
					 &rbot->neck_hitbox	,
					 &rbot->chest_hitbox,
					 &rbot->stomach_hitbox	,
					 &rbot->pelvis_hitbox	,
					 &rbot->arms_hitbox	,
					 &rbot->legs_hitbox	,
					 &rbot->feets_hitbox	,
				  };

				  bool* bt_hitboxes[] = {
					 &rbot->bt_head_hitbox	,
					 &rbot->bt_neck_hitbox	,
					 &rbot->bt_chest_hitbox,
					 &rbot->bt_stomach_hitbox	,
					 &rbot->bt_pelvis_hitbox	,
					 &rbot->bt_arms_hitbox	,
					 &rbot->bt_legs_hitbox	,
					 &rbot->bt_feets_hitbox	,
				  };

				  bool* mp_safety_hitboxes[] = {
					 &rbot->mp_safety_head_hitbox	,
					 &rbot->mp_safety_neck_hitbox	,
					 &rbot->mp_safety_chest_hitbox,
					 &rbot->mp_safety_stomach_hitbox	,
					 &rbot->mp_safety_pelvis_hitbox	,
					 &rbot->mp_safety_arms_hitbox	,
					 &rbot->mp_safety_legs_hitbox	,
					 &rbot->mp_safety_feets_hitbox	,
				  };

				  const char* hitboxes_str[] = {
					 XorStr( "Head" ), XorStr( "Neck" ), XorStr( "Chest" ), XorStr( "Stomach" ), XorStr( "Pelvis" ), XorStr( "Arms" ), XorStr( "Legs" ), XorStr( "Feet" )
				  };

				  bool* bt_conditions[] = {
					 &rbot->override_shot	,
					 &rbot->override_running	,
					 &rbot->override_walking	,
					 &rbot->override_inair	,
					 &rbot->override_standing	,
					 &rbot->override_backward	,
					 &rbot->override_sideways	,
					 &rbot->override_unresolved	,
				  };

				  bool* safe_conditions[] = {
					 &rbot->safe_shot	,
					 &rbot->safe_running	,
					 &rbot->safe_walking	,
					 &rbot->safe_inair	,
					 &rbot->safe_standing	,
					 &rbot->safe_backward	,
					 &rbot->safe_sideways	,
					 &rbot->safe_unresolved	,
				  };

				  bool* body_conditions[] = {
					 &rbot->prefer_body_shot	,
					 &rbot->prefer_body_running	,
					 &rbot->prefer_body_walking	,
					 &rbot->prefer_body_inair	,
					 &rbot->prefer_body_standing	,
					 &rbot->prefer_body_backward	,
					 &rbot->prefer_body_sideways	,
					 &rbot->prefer_body_unresolved,
				  };

				  bool* head_conditions[] = {
					 &rbot->prefer_head_shot	,
					 &rbot->prefer_head_running	,
					 &rbot->prefer_head_walking	,
					 &rbot->prefer_head_inair	,
					 &rbot->prefer_head_standing	,
					 &rbot->prefer_head_backward	,
					 &rbot->prefer_head_sideways	,
					 &rbot->prefer_head_unresolved	,
				  };

				  const char* conditions_str[] = {
					 XorStr( "On shot" ),
					 XorStr( "Running" ) ,
					 XorStr( "Walking" ) ,
					 XorStr( "In air" ),
					 XorStr( "Standing" ),
					 XorStr( "Backward" ),
					 XorStr( "Sideways" ),
					 XorStr( "In duck" ),
				  };

				  const char* hitbox_select_type[] = { XorStr( "Damage" ), XorStr( "Accuracy" ), XorStr( "Safety" ) };
				  ImGui::ComboA( XorStr( "Hitbox selection" ), &rbot->hitbox_selection, hitbox_select_type, ARRAYSIZE( hitbox_select_type ) );

				  ImGui::MultiCombo( XorStr( "Hitboxes" ),
									 hitboxes, hitboxes_str, 8 );

				  ImGui::MultiCombo( XorStr( "Safety hitboxes" ),
									 mp_safety_hitboxes, hitboxes_str, 8 );

				  ImGui::Checkbox( XorStr( "Override hitscan" ), &rbot->override_hitscan );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
				  ImGui::KeyBox( "", &g_Vars.rage.override_key );

				  if ( rbot->override_hitscan ) {
					 ImGui::MultiCombo( XorStr( "Override condition" ),
										bt_conditions, conditions_str, 8 );

					 ImGui::MultiCombo( XorStr( "Override hitboxes" ),
										bt_hitboxes, hitboxes_str, 8 );
				  }

				  ImGui::Checkbox( XorStr( "Prefer body" ), &rbot->prefer_body );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
				  ImGui::KeyBox( "", &g_Vars.rage.prefer_body );

				  if ( rbot->prefer_body ) {
					 ImGui::MultiCombo( XorStr( "Conditions##PreferBody" ),
										body_conditions, conditions_str, 8 );
				  }

				  ImGui::Checkbox( XorStr( "Prefer head" ), &rbot->prefer_head );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
				  ImGui::KeyBox( "", &g_Vars.rage.prefer_head );

				  if ( rbot->prefer_head ) {
					 ImGui::MultiCombo( XorStr( "Conditions##PreferHead" ),
										head_conditions, conditions_str, 8 );
				  }

				  ImGui::Checkbox( XorStr( "Prefer safety" ), &rbot->prefer_safety );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
				  ImGui::KeyBox( "", &g_Vars.rage.prefer_safe );

				  if ( rbot->prefer_safety ) {
					 ImGui::MultiCombo( XorStr( "Conditions##PreferSafety" ),
										safe_conditions, conditions_str, 8 );
				  }

				  const char* mp_density_str[] = {
					 XorStr( "Low" ), XorStr( "Medium" ), XorStr( "High" ),  XorStr( "Maximum" )
				  };

				  ImGui::Checkbox( XorStr( "Ignore limbs on move" ), &rbot->ignorelimbs_ifwalking );
				  ImGui::Checkbox( XorStr( "Delay on unducking" ), &rbot->delay_shot_on_unducking );

				  ImGui::Checkbox( XorStr( "Dynamic point scale" ), &rbot->dynamic_ps );
				  ImGui::SliderFloatA( XorStr( "Max head scale##Head" ), &rbot->head_ps, 0.f, 95.0f, XorStr( "%.0f %%" ) );
				  ImGui::SliderFloatA( XorStr( "Max body scale##Head" ), &rbot->body_ps, 0.f, 95.0f, XorStr( "%.0f %%" ) );

			   } ImGui::EndChild( );

			   ImGui::BeginChild( "Exploits", ImVec2( 0.0f, 165.0f ), true );
			   {
				  ImGui::Text( "Exploits" );
				  ImGui::Separator( );

				  ImGui::Checkbox( XorStr( "Enabled" ), &g_Vars.rage.exploit );
				  ImGui::Text( XorStr( "Double tap" ) );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
				  ImGui::KeyBox( "", &g_Vars.rage.double_tap_bind );
				  const char* dt_type[] = { XorStr( "Offensive" ), XorStr( "Instant" ) };
				  //ImGui::ComboA( XorStr( "Double tap mode" ), &g_Vars.rage.double_tap_type, dt_type, ARRAYSIZE( dt_type ) );
				  ImGui::SliderFloatA( XorStr( "Double tap hit chance" ), &rbot->doubletap_hitchance, 0.f, 100.f, XorStr( "%.0f %%" ) );

				  ImGui::Text( XorStr( "Hide shots" ) );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
				  ImGui::KeyBox( "", &g_Vars.rage.hide_shots_bind );

			   } ImGui::EndChild( );

			   static bool copy_ip = false;
			   ImGui::Text( g_Vars.globals.server_adress.c_str( ) );
			   if ( ImGui::IsItemClicked( 0 ) || copy_ip ) {
				  copy_ip = true;
				  ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
				  ImGui::OpenPopup( "Copy ip##unload_popup7" );
				  if ( ImGui::BeginPopupModal( XorStr( "Copy ip##unload_popup7" ) ), NULL, dwFlag ) {
					 ImGui::Text( XorStr( "Are you sure?" ) );
					 if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::ToClipboard( g_Vars.globals.server_adress.c_str( ) );
						ILoggerEvent::Get( )->PushEvent( XorStr( "Ip address copied!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
						copy_ip = false;
						ImGui::CloseCurrentPopup( );
					 }
					 ImGui::SameLine( );
					 if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::CloseCurrentPopup( );
						copy_ip = false;
					 }

					 ImGui::EndPopup( );
				  }
			   }

			   ImGui::PopStyleColor( 1 );

			   break;
			}
			case 1:
			{
			   ImGui::Columns( 2, NULL, false );
			   ImGui::SetColumnWidth( 0, full_window.x * 0.5f );
			   ImGui::SetColumnWidth( 1, full_window.x );

			   ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, ImVec4( 30 / 255.f, 30 / 255.f, 39 / 255.f, 1.0f ) );

			   ImGui::BeginChild( XorStr( "Anti-Aimbot" ), ImVec2( 0.0f, 470.0f ), true );
			   {
				  ImGui::Text( XorStr( "Anti-Aimbot" ) );
				  ImGui::Separator( );
				  static int antiaim_type = 0;
				  CVariables::ANTIAIM_STATE* settings = nullptr;

				  ImGui::Checkbox( XorStr( "Enabled" ), &g_Vars.antiaim.enabled );

				  ImGui::Checkbox( XorStr( "Manual" ), &g_Vars.antiaim.manual );

				  if ( g_Vars.antiaim.manual ) {
					 ImGui::Text( "Left" );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
					 ImGui::KeyBox( XorStr( "Left" ), &g_Vars.antiaim.manual_left_bind, KeyBindType::HOLD );
					 ImGui::Text( "Right" );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
					 ImGui::KeyBox( XorStr( "Right" ), &g_Vars.antiaim.manual_right_bind, KeyBindType::HOLD );
					 ImGui::Text( "Back" );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
					 ImGui::KeyBox( XorStr( "Back" ), &g_Vars.antiaim.manual_back_bind, KeyBindType::HOLD );
					 ImGui::Text( "Mouse override" );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
					 ImGui::KeyBox( XorStr( "Mouse override" ), &g_Vars.antiaim.mouse_override, KeyBindType::HOLD );
				  }


				  static bool* disable_conditions[] = {
					 &g_Vars.antiaim.on_freeze_period 	,
					 &g_Vars.antiaim.on_knife 	,
					 &g_Vars.antiaim.on_grenade 	,
					 &g_Vars.antiaim.on_ladder 	,
					 &g_Vars.antiaim.on_dormant 	,
					 &g_Vars.antiaim.on_manual_shot 	,
				  };

				  const char* disable_conditions_str[] = {
					 XorStr( "On freeze period" ),
					 XorStr( "On knife" ),
					 XorStr( "On grenade" ),
					 XorStr( "On ladder" ),
					 XorStr( "On dormant" ),
					 XorStr( "On manual shot" ),
				  };

				  ImGui::MultiCombo( XorStr( "Disable condition" ),
									 disable_conditions, disable_conditions_str, 6 );

				  const char* antiaim_type_str[] = { XorStr( "Stand" ), XorStr( "Move" ), XorStr( "Air" ) };

				  ImGui::ComboA( XorStr( "Movement type" ), &antiaim_type, antiaim_type_str, ARRAYSIZE( antiaim_type_str ) );

				  if ( antiaim_type == 0 )
					 settings = &g_Vars.antiaim_stand;
				  else if ( antiaim_type == 1 )
					 settings = &g_Vars.antiaim_move;
				  else
					 settings = &g_Vars.antiaim_air;

				  const char* yaw_str[] = { XorStr( "Forward" ), XorStr( "Backward" )};
				  const char* yaw_base_str[] = { XorStr( "View" ), XorStr( "At targets" ) };
				  const char* yaw_base_move_str[] = { XorStr( "View" ), XorStr( "At targets" ), XorStr( "Movement direction" ) };
				  const char* pitch_str[] = { XorStr( "Off" ), XorStr( "Down" ), XorStr( "Up" ), XorStr( "Zero" ) };
				  const char* auto_dir[] = { XorStr( "Off" ), XorStr( "Hide Fake" ), XorStr( "Hide Real" ) };
				//  const char* desync_str[] = { XorStr( "Off" ), XorStr( "Static" ), XorStr( "Jitter" ) };
				 // const char* lby_str[] = { XorStr( "Off" ), XorStr( "On" ), XorStr( "Sway" ), XorStr( "Low delta" ) };
				//  const char* micro_move_str[] = { XorStr( "Eye yaw" ), XorStr( "Opposite" ) };

				  auto str = antiaim_type == 0 ? yaw_base_str : yaw_base_move_str;
				  auto str_size = antiaim_type == 0 ? 2 : 3;
				  ImGui::ComboA( XorStr( "Yaw base" ), &settings->base_yaw,
								 str, str_size );

				  ImGui::ComboA( XorStr( "Pitch" ), &settings->pitch, pitch_str, ARRAYSIZE( pitch_str ) );
				  ImGui::ComboA( XorStr( "Yaw" ), &settings->yaw, yaw_str, ARRAYSIZE( yaw_str ) );

				 // ImGui::ComboA( XorStr( "Desync" ), &settings->desync, desync_str, ARRAYSIZE( desync_str ) );
				 //
				 // if ( settings->desync > 2 )
				//	 settings->desync = 2;
				 //
				 // if ( antiaim_type == 0 ) {
				//	 ImGui::ComboA( XorStr( "Body yaw" ), &g_Vars.antiaim.mirco_move_type, micro_move_str, ARRAYSIZE( micro_move_str ) );
				//	 if ( g_Vars.antiaim.mirco_move_type == 0 )
				//		ImGui::ComboA( XorStr( "Break lower body" ), &g_Vars.antiaim.break_lby, lby_str, ARRAYSIZE( lby_str ) );
				 // }
				 //
				 // if ( settings->desync != 3 ) {
				//	 ImGui::SliderFloatA( XorStr( "Desync offset" ), &settings->desync_amount, 0.0f, 100.0f, XorStr( "%.0f %%" ) );
				//	 ImGui::SliderFloatA( XorStr( "Desync offset flipped" ), &settings->desync_amount_flipped, 0.0f, 100.0f, XorStr( "%.0f %%" ) );
				 // }

#if 0
				  ImGui::Checkbox( XorStr( "Jitter" ), &settings->jitter );
				  if ( settings->jitter ) {
					 ImGui::SliderFloatA( XorStr( "Range##Jitter range" ), &settings->jitter_range, 0.0f, 90.0f, XorStr( "%.0f degrees" ) );
				  }
				  ImGui::Checkbox( XorStr( "Spin" ), &settings->spin );
				  if ( settings->spin ) {
					 ImGui::Checkbox( XorStr( "Switch" ), &settings->spin_switch );
					 ImGui::SliderFloatA( XorStr( "Speed##Spin" ), &settings->spin_speed, -50.0f, 50.0f, XorStr( "%.0f %%" ) );
					 ImGui::SliderFloatA( XorStr( "Range##Spin" ), &settings->spin_range, 1.0f, 180.0f, XorStr( "%.0f degrees" ) );
				  }
#endif

				  ImGui::Checkbox( XorStr( "Auto direction" ), &settings->autodirection );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
				  ImGui::KeyBox( "", &g_Vars.antiaim.autodirection_override );
				  if ( settings->autodirection ) {
					 ImGui::Checkbox( XorStr( "Auto direction ignore duck" ), &settings->autodirection_ignore_duck );

					 ImGui::SliderIntA( XorStr( "Extrapolation##AutoDirExtrapolation" ), &settings->extrapolation_ticks, 0, 20, XorStr( "%d ticks" ) );
					 ImGui::SliderFloatA( XorStr( "Range##AutoDirRange" ), &settings->autodirection_range, 24.0f, 64.0f, XorStr( "%.0f units" ) );

					// ImGui::ComboA( XorStr( "Desync auto direction" ), &settings->desync_autodir, auto_dir, ARRAYSIZE( auto_dir ) );
				  }

				  //ImGui::Checkbox( XorStr( "Move sync" ), &g_Vars.antiaim.move_sync );
				 // ImGui::Checkbox( XorStr( "Hide real on shot" ), &g_Vars.antiaim.hide_real_on_shot );
				  ImGui::Checkbox( XorStr( "Twist speed" ), &g_Vars.antiaim.twist_speed );
				  ImGui::Checkbox( XorStr( "Lby prediction" ), &g_Vars.antiaim.lbypred );

				  /*
				  GUI::Controls::Checkbox( XorStr( "Distortion###sse" ), &g_Vars.antiaim.distort );
					GUI::Controls::Checkbox( XorStr( "Manual override" ), &g_Vars.antiaim.distort_manual_aa );
					if( GUI::Controls::Checkbox( XorStr( "Twist" ), &g_Vars.antiaim.distort_twist ) )
						GUI::Controls::Slider( XorStr( "Speed" ), &g_Vars.antiaim.distort_speed, 1.f, 10.f, XorStr( "%.1fs" ) );
					GUI::Controls::Slider( XorStr( "Max time" ), &g_Vars.antiaim.distort_max_time, 0.f, 10.f );
					GUI::Controls::Slider( XorStr( "Range" ), &g_Vars.antiaim.distort_range, -360.f, 360.f );

					std::vector<MultiItem_t> distort_disablers = {
						{ XorStr( "Fakewalking" ), &g_Vars.antiaim.distort_disable_fakewalk },
						{ XorStr( "Running" ), &g_Vars.antiaim.distort_disable_run },
						{ XorStr( "Airborne" ), &g_Vars.antiaim.distort_disable_air },
					};

					GUI::Controls::MultiDropdown( XorStr( "Distortion disablers" ), distort_disablers );
				  
				  */

				  ImGui::Checkbox( XorStr( "Distortion" ), &g_Vars.antiaim.distort );
				  ImGui::Checkbox( XorStr( "Manual override" ), &g_Vars.antiaim.distort_manual_aa );
				  if ( ImGui::Checkbox( XorStr( "Twist" ), &g_Vars.antiaim.distort_twist ) )
					  ImGui::SliderFloat( XorStr( "Speed" ), &g_Vars.antiaim.distort_speed, 1.f, 10.f, XorStr( "%.1fs" ) );

				 ImGui::SliderFloat( XorStr( "Max time" ), &g_Vars.antiaim.distort_max_time, 0.f, 10.f );
				 ImGui::SliderFloat( XorStr( "Range" ), &g_Vars.antiaim.distort_range, -360.f, 360.f );

				 static bool* disable_distort[ ] = {
					 &g_Vars.antiaim.distort_disable_fakewalk 	,
					 & g_Vars.antiaim.distort_disable_run 	,
					 & g_Vars.antiaim.distort_disable_air 	,
				 };

				 const char* disable_dir_str[ ] = {
					XorStr( "Fakewalking ( wip )" ),
					XorStr( "Running" ),
					XorStr( "Airborne" ),
				 };

				 ImGui::MultiCombo( XorStr( "Disable distortion" ),
									disable_distort, disable_dir_str, 3 );

			   } ImGui::EndChild( );

			#ifdef BETA_MODE
			   ImGui::Text( XorStr( "eexomi.host [BETA] | %i:%i:%i |%s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#else 
			   ImGui::Text( XorStr( "eexomi.host v2 | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#endif

			   ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );
			   ImGui::NextColumn( );
			   ImGui::PopStyleVar( );

			   ImGui::BeginChild( "Fakelag", ImVec2( 0.0f, 470.0f ), true );
			   {
				  ImGui::Text( "Fakelag" );
				  ImGui::Separator( );
				  ImGui::Checkbox( XorStr( "Enabled## fake lag" ), &g_Vars.fakelag.enabled );

			   #if 1
				  static bool* conditions[] = {
					 &g_Vars.fakelag.when_standing,
					 &g_Vars.fakelag.when_moving,
					 &g_Vars.fakelag.when_air,
					 &g_Vars.fakelag.when_dormant,
					 &g_Vars.fakelag.when_exploits,
				  };

				  const char* conditions_str[] = {
					 XorStr( "Standing" ), XorStr( "Moving" ), XorStr( "Air" ),  XorStr( "Dormant" ), XorStr( "Exploits" ),
				  };
			   #endif

				  static bool* alt_conditions[] = {
					 &g_Vars.fakelag.when_peek,
					 &g_Vars.fakelag.when_unducking,
					 &g_Vars.fakelag.when_land,
					 &g_Vars.fakelag.when_switching_weapon,
					 &g_Vars.fakelag.when_reloading,
					 &g_Vars.fakelag.when_velocity_change,
					 &g_Vars.fakelag.break_lag_compensation,
				  };

				  const char* alt_conditions_str[] = {
					 XorStr( "Peek" ), XorStr( "Unducking" ), XorStr( "Land" ), XorStr( "Weapon Swap" ), XorStr( "Reload" ),
					 XorStr( "Velocity change" ), XorStr( "Break lag compensation" ),
				  };

				  ImGui::MultiCombo( XorStr( "Conditions" ), conditions, conditions_str, 4 );
				  ImGui::SliderIntA( XorStr( "Limit" ), &g_Vars.fakelag.choke, 1, 16, XorStr( "%d ticks" ) );
				  ImGui::SliderFloatA( XorStr( "Variance" ), &g_Vars.fakelag.variance, 0.0f, 100.0f, XorStr( "%.0f %%" ) );
				  ImGui::MultiCombo( XorStr( "Alternative conditions" ), alt_conditions, alt_conditions_str, 7 );
				  ImGui::SliderIntA( XorStr( "Alternative lag amount" ), &g_Vars.fakelag.alternative_choke, 1, 16, XorStr( "%d ticks" ) );
				  // ImGui::SliderIntA( XorStr( "Lag limit" ), &g_Vars.fakelag.lag_limit, 6, 62, XorStr( "%d ticks" ) );
			   } ImGui::EndChild( );

			   static bool copy_ip = false;
			   ImGui::Text( g_Vars.globals.server_adress.c_str( ) );
			   if ( ImGui::IsItemClicked( 0 ) || copy_ip ) {
				  copy_ip = true;
				  ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
				  ImGui::OpenPopup( "Copy ip##unload_popup7" );
				  if ( ImGui::BeginPopupModal( XorStr( "Copy ip##unload_popup7" ) ), NULL, dwFlag ) {
					 ImGui::Text( XorStr( "Are you sure?" ) );
					 if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::ToClipboard( g_Vars.globals.server_adress.c_str( ) );
						ILoggerEvent::Get( )->PushEvent( XorStr( "Ip address copied!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
						copy_ip = false;
						ImGui::CloseCurrentPopup( );
					 }
					 ImGui::SameLine( );
					 if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::CloseCurrentPopup( );
						copy_ip = false;
					 }

					 ImGui::EndPopup( );
				  }
			   }
			   ImGui::PopStyleColor( 1 );

			   break;
			}
			case 2:
			{
			   ImGui::Columns( 2, NULL, false );
			   ImGui::SetColumnWidth( 0, full_window.x * 0.5f );
			   ImGui::SetColumnWidth( 1, full_window.x );

			   ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, ImVec4( 30 / 255.f, 30 / 255.f, 39 / 255.f, 1.0f ) );
			   ImGui::BeginChild( "Weapon", ImVec2( 0.0f, 80.0f ), true );
			   {
				  ImGui::Text( "Config" );
				  ImGui::Separator( );

				  // ghetto-workaround cuz shitty macroces
				  auto backup = current_group;
				  ImGui::Combo( XorStr( "Group" ), &current_group, weapon_groups, ARRAYSIZE( weapon_groups ) );
				  if ( backup != current_group ) {
					 current_weapon = 0;
				  }

				  switch ( current_group ) {
					 case WEAPONGROUP_PISTOL:
					 aim_group = &pistols;
					 break;
					 case WEAPONGROUP_HEAVYPISTOL:
					 aim_group = &heavy_pistols;
					 break;
					 case WEAPONGROUP_SUBMACHINE:
					 aim_group = &sub_machines;
					 break;
					 case WEAPONGROUP_RIFLE:
					 aim_group = &rifles;
					 break;
					 case WEAPONGROUP_SHOTGUN:
					 aim_group = &shotguns;
					 break;
					 case WEAPONGROUP_SNIPER:
					 aim_group = &snipers;
					 break;
					 case WEAPONGROUP_HEAVY:
					 aim_group = &heavys;
					 break;
					 case WEAPONGROUP_AUTOSNIPER:
					 aim_group = &auto_snipers;
					 break;
				  }

				  {
					 ImGui::Combo(
						XorStr( "Weapon##group_weapons" ), &current_weapon,
						[] ( void* data, int32_t idx, const char** out_text ) {
						auto vec = reinterpret_cast< group_vector* >( data );
						*out_text = vec->at( idx ).first.c_str( );
						return true;
					 }, ( void* ) ( aim_group ), aim_group->size( ), 20 );

				  }

				  if ( current_weapon <= 0 ) {
					 switch ( current_group ) {
						case WEAPONGROUP_PISTOL:
						lbot = &g_Vars.legit_pistols;
						break;
						case WEAPONGROUP_HEAVYPISTOL:
						lbot = &g_Vars.legit_heavypistols;
						break;
						case WEAPONGROUP_SUBMACHINE:
						lbot = &g_Vars.legit_smgs;
						break;
						case WEAPONGROUP_RIFLE:
						lbot = &g_Vars.legit_rifles;
						break;
						case WEAPONGROUP_SHOTGUN:
						lbot = &g_Vars.legit_shotguns;
						break;
						case WEAPONGROUP_SNIPER:
						lbot = &g_Vars.legit_snipers;
						break;
						case WEAPONGROUP_HEAVY:
						lbot = &g_Vars.legit_heavys;
						break;
						case WEAPONGROUP_AUTOSNIPER:
						lbot = &g_Vars.legit_autosnipers;
						break;
					 }
				  } else {
					 for ( auto i = 0; i < g_Vars.legit_weapons.Size( ); ++i ) {
						auto weapon = g_Vars.legit_weapons[ i ];
						if ( weapon->item_idx == aim_group->at( current_weapon ).second ) {
						   lbot = g_Vars.legit_weapons[ i ];
						   break;
						}
					 }
				  }

			   } ImGui::EndChild( );
			   ImGui::BeginChild( "Legit", ImVec2( 0.0f, 385.0f ), true );
			   {
				  ImGui::Text( "Legit" );
				  ImGui::Separator( );
				  const char* priority_hitboxes[]{
						   XorStr( "Head" ),
						   XorStr( "Neck" ),
						   XorStr( "Chest" ),
						   XorStr( "Stomach" ),
				  };

				  const char* hitboxes[]{
					 XorStr( "Head" ),
					 XorStr( "Neck" ),
					 XorStr( "Chest" ),
					 XorStr( "Stomach" ),
					 XorStr( "Pelvis" ),
					 XorStr( "Arms" ),
					 XorStr( "Legs" ),
				  };

				  const char* hitbox_selection[]{
					 XorStr( "Priority" ),
					 XorStr( "Nearest" ),
				  };

				  const char* smooth_type[]{
					 XorStr( "Dynamic" ),
					 XorStr( "Static" ),
				  };

				  ImGui::Checkbox( XorStr( "Active" ), &lbot->active );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
				  ImGui::KeyBox( "", &lbot->key );

				  ImGui::Checkbox( XorStr( "Auto fire" ), &lbot->autofire );
				  ImGui::Checkbox( XorStr( "Auto wall" ), &lbot->autowall );
				  ImGui::Checkbox( XorStr( "Quick stop" ), &lbot->quickstop );

				  if ( current_group == 3 ) {
					 ImGui::Checkbox( XorStr( "Only in scope" ), &g_Vars.legit.snipers_only_scope );
				  }

				  ImGui::ComboA( XorStr( "Hitbox selection" ), &lbot->hitbox_selection, hitbox_selection, ARRAYSIZE( hitbox_selection ) );
				  ImGui::ComboA( XorStr( "Smooth type" ), &lbot->smooth_type, smooth_type, ARRAYSIZE( smooth_type ) );

				  if ( lbot->hitbox_selection == 0 ) {
					 ImGui::ComboA( XorStr( "Hitbox" ), &lbot->hitbox, priority_hitboxes, ARRAYSIZE( priority_hitboxes ) );
				  } else {
					 bool* opt_hitboxes[] = {
						&lbot->head_hitbox,
						&lbot->neck_hitbox,
						&lbot->chest_hitbox,
						&lbot->stomach_hitbox,
						&lbot->pelvis_hitbox,
						&lbot->arms_hitbox,
						&lbot->legs_hitbox,
					 };

					 ImGui::MultiCombo( XorStr( "Hitboxes" ), opt_hitboxes, hitboxes, 7 );
				  }

				  ImGui::NextColumn( );

				  ImGui::SliderFloatA( XorStr( "Field of view" ), &lbot->fov, 1.f, 30.0f, XorStr( "%0.2f deg" ) );
				  ImGui::SliderFloatA( XorStr( "Dead zone" ), &lbot->deadzone, 0.f, 2.0f, XorStr( "%0.2f deg" ) );
				  ImGui::SliderFloatA( XorStr( "Smooth" ), &lbot->smooth, 1.0f, 10.0f, XorStr( "%0.2f" ) );
				  ImGui::SliderFloatA( XorStr( "Randomize" ), &lbot->randomize, 0.0f, 5.0f, XorStr( "%0.1f" ) );

				  ImGui::Checkbox( XorStr( "Recoil control" ), &lbot->rcs );
				  ImGui::Checkbox( XorStr( "Standalone recoil control" ), &lbot->rcs_standalone );
				  ImGui::SliderIntA( XorStr( "Recoil start bullet" ), &lbot->rcs_shots, 0, 5, XorStr( "%d shots" ) );
				  ImGui::SliderFloatA( XorStr( "Recoil X" ), &lbot->rcs_x, 0.f, 100.f, XorStr( "%0.0f %%" ) );
				  ImGui::SliderFloatA( XorStr( "Recoil Y" ), &lbot->rcs_y, 0.f, 100.f, XorStr( "%0.0f %%" ) );

				  ImGui::Checkbox( XorStr( "Kill delay" ), &lbot->kill_delay );
				  if ( lbot->kill_delay ) {
					 ImGui::SliderFloatA( XorStr( "Amount##Kill Delay Time" ), &lbot->kill_shot_delay, 0.100f, 2.f, XorStr( "%0.3f sec" ) );
				  }

				  ImGui::Checkbox( XorStr( "First shot delay" ), &lbot->fsd_enabled );
				  if ( lbot->fsd_enabled ) {
					 ImGui::Checkbox( XorStr( "Auto##delay" ), &lbot->auto_delay );

					 if ( !lbot->auto_delay )
						ImGui::SliderFloatA( XorStr( "Amount##First shot delay" ), &lbot->first_shot_delay, 0.015f, 0.200f, XorStr( "%0.3f sec" ) );
				  }

			   } ImGui::EndChild( );

			#ifdef BETA_MODE
			   ImGui::Text( XorStr( "eexomi.host [BETA] | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#else 
			   ImGui::Text( XorStr( "eexomi.host v2 | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#endif
			   ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );
			   ImGui::NextColumn( );
			   ImGui::PopStyleVar( );

			   ImGui::BeginChild( "Trigger", ImVec2( 0.0f, 290.0f ), true );
			   {
				  ImGui::Text( "Trigger" );
				  ImGui::Separator( );
				  ImGui::Checkbox( XorStr( "Enabled" ), &lbot->trg_enabled );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
				  ImGui::KeyBox( "", &lbot->trg_key );

				  ImGui::Checkbox( XorStr( "Autowall" ), &lbot->trg_autowall );
				  ImGui::Checkbox( XorStr( "Run aimbot" ), &lbot->trg_aimbot );
				  ImGui::SliderFloatA( XorStr( "Hitchance" ), &lbot->trg_hitchance, 0.0f, 100.0f, XorStr( "%0.0f %%" ) );
				  ImGui::SliderFloatA( XorStr( "Delay" ), &lbot->trg_delay, 0.015f, 0.300f, XorStr( "%0.3f sec" ) );
				  ImGui::SliderFloatA( XorStr( "Burst" ), &lbot->trg_burst, 0.050f, 0.500f, XorStr( "%0.3f sec" ) );

				  const char* hitboxes[]{
					 XorStr( "Head" ),
					 XorStr( "Chest" ),
					 XorStr( "Stomach" ),
					 XorStr( "Arms" ),
					 XorStr( "Legs" ),
				  };

				  bool* opt_hitboxes[] = {
					 &lbot->trg_head_hitbox,
					 &lbot->trg_chest_hitbox,
					 &lbot->trg_stomach_hitbox,
					 &lbot->trg_arms_hitbox,
					 &lbot->trg_legs_hitbox,
				  };

				  ImGui::MultiCombo( XorStr( "Hitboxes" ), opt_hitboxes, hitboxes, 5 );

			   } ImGui::EndChild( );

			   ImGui::BeginChild( "Extra", ImVec2( 0.0f, 175.0f ), true );
			   {
				  ImGui::Text( "Extra" );
				  ImGui::Separator( );
				  ImGui::Checkbox( XorStr( "Autopistol" ), &g_Vars.legit.autopistol );
				  if ( g_Vars.legit.autopistol ) {
					 ImGui::SliderFloatA( XorStr( "Amount##Auto pistol delay" ), &g_Vars.legit.autopistol_delay, 0.00f, 0.100f, XorStr( "%0.3fsec" ) );
				  }

				  ImGui::Checkbox( XorStr( "Through smoke" ), &g_Vars.legit.throughsmoke );
				  ImGui::Checkbox( XorStr( "While blind" ), &g_Vars.legit.whileblind );
				  ImGui::Checkbox( XorStr( "Ignore jump" ), &g_Vars.legit.ignorejump );
				  ImGui::Checkbox( XorStr( "Position adjustment" ), &g_Vars.legit.position_adjustment );

			   } ImGui::EndChild( );

			   static bool copy_ip = false;
			   ImGui::Text( g_Vars.globals.server_adress.c_str( ) );
			   if ( ImGui::IsItemClicked( 0 ) || copy_ip ) {
				  copy_ip = true;
				  ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
				  ImGui::OpenPopup( "Copy ip##unload_popup7" );
				  if ( ImGui::BeginPopupModal( XorStr( "Copy ip##unload_popup7" ) ), NULL, dwFlag ) {
					 ImGui::Text( XorStr( "Are you sure?" ) );
					 if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::ToClipboard( g_Vars.globals.server_adress.c_str( ) );
						ILoggerEvent::Get( )->PushEvent( XorStr( "Ip address copied!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
						copy_ip = false;
						ImGui::CloseCurrentPopup( );
					 }
					 ImGui::SameLine( );
					 if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::CloseCurrentPopup( );
						copy_ip = false;
					 }

					 ImGui::EndPopup( );
				  }
			   }

			   ImGui::PopStyleColor( 1 );

			   break;
			}
			case 3:
			{
			   ImGui::Columns( 2, NULL, false );
			   ImGui::SetColumnWidth( 0, full_window.x * 0.5f );
			   ImGui::SetColumnWidth( 1, full_window.x );

			   ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, ImVec4( 30 / 255.f, 30 / 255.f, 39 / 255.f, 1.0f ) );

			   ImGui::BeginChild( "Players", ImVec2( 0.0f, 470.0f ), true );
			   {
				  ImGui::Text( "Players" );
				  ImGui::Separator( );
				  const char* esp_type_str[] = { XorStr( "Static" ), XorStr( "Dynamic" ) };

				  ImGui::Checkbox( XorStr( "Enabled" ), &g_Vars.esp.esp_enable );
				  if ( g_Vars.esp.esp_enable ) {
					 ImGui::Checkbox( XorStr( "Ignore team" ), &g_Vars.esp.team_check );
					 ImGui::Checkbox( XorStr( "Extended ESP" ), &g_Vars.esp.extended_esp );
					 ImGui::Checkbox( XorStr( "Dormant fade" ), &g_Vars.esp.fade_esp );
					 ImGui::Checkbox( XorStr( "Box" ), &g_Vars.esp.box );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Box Color" ), g_Vars.esp.box_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					 const char* box_type_str[] = { XorStr( "Bounding" ), XorStr( "Corner" ) };
					 if ( g_Vars.esp.box ) {
						ImGui::ComboA( XorStr( "Box type##box type" ), &g_Vars.esp.box_type, box_type_str, ARRAYSIZE( box_type_str ) );
					 }

					 ImGui::Checkbox( XorStr( "Name" ), &g_Vars.esp.name );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Name Color" ), g_Vars.esp.name_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Skeleton" ), &g_Vars.esp.skeleton );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Skelet Color" ), g_Vars.esp.skeleton_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					 ImGui::Checkbox( XorStr( "Skeleton history" ), &g_Vars.esp.skeleton_history );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Skelet History Color" ), g_Vars.esp.skeleton_history_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 const char* history_type_str[] = { XorStr( "Oldest Tick" ), XorStr( "All Ticks" ) };
					 if ( g_Vars.esp.skeleton_history ) {
						ImGui::ComboA( XorStr( "##Skelet History Type" ), &g_Vars.esp.skeleton_history_type, history_type_str, ARRAYSIZE( history_type_str ) );
					 }
					 ImGui::Checkbox( XorStr( "Aim points" ), &g_Vars.esp.aim_points );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Aim points Color" ), g_Vars.esp.aim_points_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Draw hitboxes" ), &g_Vars.esp.draw_hitboxes );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Draw hitboxes color" ), g_Vars.esp.hitboxes_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );


					 ImGui::Checkbox( XorStr( "Ammo bar" ), &g_Vars.esp.draw_ammo_bar );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##ammo_color color" ), g_Vars.esp.ammo_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Health" ), &g_Vars.esp.health );
					 if ( g_Vars.esp.health ) {
						ImGui::Checkbox( XorStr( "Health Animated" ), &g_Vars.esp.animated_hp );
					 }

					 ImGui::Checkbox( XorStr( "Glow team" ), &g_Vars.esp.glow_team );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Glow Team Color" ), g_Vars.esp.glow_team_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					 ImGui::Checkbox( XorStr( "Glow enemy" ), &g_Vars.esp.glow_enemy );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Glow Enemy Color" ), g_Vars.esp.glow_enemy_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					 ImGui::Checkbox( XorStr( "Glow weapons" ), &g_Vars.esp.glow_weapons );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Glow Weapon Color" ), g_Vars.esp.glow_weapons_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					 ImGui::Checkbox( XorStr( "Glow grenades" ), &g_Vars.esp.glow_grenade );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Glow grenade Color" ), g_Vars.esp.glow_grenade_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 const char* glow_type_str[] = { XorStr( "Outline" ), XorStr( "Cover" ), XorStr( "Thin" ) };

					 ImGui::ComboA( XorStr( "Glow type##Glow type" ), &g_Vars.esp.glow_type, glow_type_str, ARRAYSIZE( glow_type_str ) );

					 static bool* flags_conditions[] = {
						&g_Vars.esp.draw_scoped,
						&g_Vars.esp.draw_flashed,
						&g_Vars.esp.draw_money,
						&g_Vars.esp.draw_armor,
						&g_Vars.esp.draw_reloading,
						&g_Vars.esp.draw_ping,
						&g_Vars.esp.draw_taser,
						&g_Vars.esp.draw_bombc4,
						&g_Vars.esp.draw_distance,
						&g_Vars.esp.draw_hit,
						//&g_Vars.esp.draw_hostage,
					 };

					 const char* flags_conditions_str[] = {
						XorStr( "Scoped" ), XorStr( "Flashed" ), XorStr( "Money" ), XorStr( "Armor" ), XorStr( "Reloading" ), XorStr( "Ping" ),
						 XorStr( "Taser" ), XorStr( "Bomb" ), XorStr( "Distance" ), XorStr( "Hit" ), //XorStr( "Hostage" ),
					 };

					 ImGui::MultiCombo( XorStr( "Flags" ),
										flags_conditions, flags_conditions_str, 10 );

					 static bool* weapon_conditions[] = {
						&g_Vars.esp.weapon,
						&g_Vars.esp.weapon_ammo,
						&g_Vars.esp.weapon_icon,
						&g_Vars.esp.weapon_other,
					 };

					 const char* weapon_conditions_str[] = {
						XorStr( "Name" ), XorStr( "Ammo" ), XorStr( "Icon" ), XorStr( "Other" )
					 };

					 ImGui::MultiCombo( XorStr( "Weapon" ),
										weapon_conditions, weapon_conditions_str, 4 );

					 ImGui::Checkbox( XorStr( "Transparency in scope" ), &g_Vars.esp.blur_in_scoped );
					 ImGui::SliderFloatA( XorStr( "Transparency##Transparency In Scope" ), &g_Vars.esp.blur_in_scoped_value, 0.0f, 1.f, XorStr( "%0.01f" ) );
				  }
			   } ImGui::EndChild( );

			#ifdef BETA_MODE
			   ImGui::Text( XorStr( "eexomi.host [BETA] | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#else 
			   ImGui::Text( XorStr( "eexomi.host v2 | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#endif

			   ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );
			   ImGui::NextColumn( );
			   ImGui::PopStyleVar( );

			   ImGui::BeginChild( "Colored models", ImVec2( 0.0f, 320.0f ), true );
			   {
				  ImGui::Text( "Colored models" );
				  ImGui::Separator( );
				  ImGui::Checkbox( XorStr( "Enabled##chams" ), &g_Vars.esp.chams_enabled );
				  ImGui::Checkbox( XorStr( "Skip occlusion" ), &g_Vars.esp.skip_occulusion );

				  static int current_chams_settings_for = 0;

				  ImGui::ComboA( XorStr( "Settings for" ), &current_chams_settings_for, std::vector<std::string> {
					 XorStr( "Local" ),
						XorStr( "Ghost" ),
						XorStr( "Hands" ),
						XorStr( "Weapon" ),
						XorStr( "Enemy" ),
						XorStr( "Teammate" ),
						XorStr( "Death enemy" ),
						XorStr( "Death teammate" ),
						XorStr( "History" ),
						XorStr( "Lag" ),
				        XorStr( "On shot" ), }
				  );

				  static bool* chams_conditions[] = {
					 &g_Vars.esp.chams_local,
					 &g_Vars.esp.chams_ghost,
					 &g_Vars.esp.chams_hands,
					 &g_Vars.esp.chams_weapon,
					 &g_Vars.esp.chams_enemy,
					 &g_Vars.esp.chams_teammate,
					 &g_Vars.esp.chams_death_enemy,
					 &g_Vars.esp.chams_death_teammate,
					 &g_Vars.esp.chams_history,
					 &g_Vars.esp.chams_lag,
					 &g_Vars.esp.hitmatrix,

				  };

				  const char* chams_conditions_str[] = {
					 XorStr( "Local" ), XorStr( "Ghost" ), XorStr( "Hands" ), XorStr( "Weapon" ), XorStr( "Enemy" ), XorStr( "Teammate" ),
					  XorStr( "Death enemy" ), XorStr( "Death teammate" ), XorStr( "History" ), XorStr( "Lag" ), XorStr( "On shot" )
				  };

				  ImGui::MultiCombo( XorStr( "Chams on" ),
									 chams_conditions, chams_conditions_str, 11 );

				  const char* materials[] = { 
					 XorStr( "Off" ), XorStr( "Flat" ),  XorStr( "Material" ), XorStr( "Glow" ), XorStr( "Outline" ),
					 XorStr( "Skull" ), XorStr( "Gloss" ), XorStr( "Crystal" ), XorStr( "Tree" ), XorStr( "Glass" ),
					 XorStr( "Spech" )
				  };

				  //set local player chams
				  if ( current_chams_settings_for == 0 ) {
					 ImGui::ComboA( XorStr( "Local material##Local material" ), &g_Vars.esp.chams_local_mat, materials, ARRAYSIZE( materials ) );
					 ImGui::Text( XorStr( "Color" ) );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Local color" ), g_Vars.esp.chams_local_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_local_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Local outline color" ), g_Vars.esp.chams_local_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_local_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_local_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );
				  
					 ImGui::Checkbox( XorStr( "Pulse effect" ), &g_Vars.esp.chams_local_pulse );
				  }

				  //set local player ghost chams
				  if ( current_chams_settings_for == 1 ) {
					 ImGui::ComboA( XorStr( "Ghost material##Desync material" ), &g_Vars.esp.chams_desync_mat, materials, ARRAYSIZE( materials ) );
					 ImGui::ColorEdit4( XorStr( "##Desync color" ), g_Vars.esp.chams_desync_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				 
					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_ghost_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Local ghost outline color" ), g_Vars.esp.chams_ghost_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_ghost_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_ghost_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );
				  
					 ImGui::Checkbox( XorStr( "Pulse effect" ), &g_Vars.esp.chams_ghost_pulse );
				  }

				  //set local player hands chams
				  if ( current_chams_settings_for == 2 ) {
					 ImGui::ComboA( XorStr( "Hands material##local" ), &g_Vars.esp.hands_chams_mat, materials, ARRAYSIZE( materials ) );

					 ImGui::Text( XorStr( "Hands color" ) );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Hands Color" ), g_Vars.esp.hands_chams_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					
					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_hands_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##hands outline color" ), g_Vars.esp.chams_hands_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_hands_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_hands_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );
				  
					 ImGui::Checkbox( XorStr( "Pulse effect" ), &g_Vars.esp.chams_hands_pulse );
				  }

				  //set local player weapon chams
				  if ( current_chams_settings_for == 3 ) {
					 ImGui::ComboA( XorStr( "Weapon material##local" ), &g_Vars.esp.weapon_chams_mat, materials, ARRAYSIZE( materials ) );

					 ImGui::Text( XorStr( "Weapon color" ) );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Weapon Color" ), g_Vars.esp.weapon_chams_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				  
					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_weapon_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Local outline color" ), g_Vars.esp.chams_weapon_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_weapon_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_weapon_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );
				  
					 ImGui::Checkbox( XorStr( "Pulse effect" ), &g_Vars.esp.chams_weapon_pulse );
				  }

				  //set enemy chams
				  if ( current_chams_settings_for == 4 ) {
					 ImGui::ComboA( XorStr( "Enemy material##enemy" ), &g_Vars.esp.enemy_chams_mat, materials, ARRAYSIZE( materials ) );

					 ImGui::Checkbox( XorStr( "Enemy Vis" ), &g_Vars.esp.enemy_chams_vis );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Team Chams Color XQZ" ), g_Vars.esp.enemy_chams_color_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Enemy XQZ" ), &g_Vars.esp.enemy_chams_xqz );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Enemy Chams Color XQZ" ), g_Vars.esp.enemy_chams_color_xqz, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_enemy_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Enemy outline color" ), g_Vars.esp.chams_enemy_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_enemy_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_enemy_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );
				  
					 ImGui::Checkbox( XorStr( "Pulse effect" ), &g_Vars.esp.chams_enemy_pulse );
				  }

				  //set temmate chams
				  if ( current_chams_settings_for == 5 ) {
					 ImGui::ComboA( XorStr( "Team material##team" ), &g_Vars.esp.team_chams_mat, materials, ARRAYSIZE( materials ) );

					 ImGui::Checkbox( XorStr( "Team vis" ), &g_Vars.esp.team_chams_vis );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Team Chams Color Vis" ), g_Vars.esp.team_chams_color_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Team xqz" ), &g_Vars.esp.team_chams_xqz );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Enemy Chams Color Vis" ), g_Vars.esp.team_chams_color_xqz, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_teammate_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Local outline color" ), g_Vars.esp.chams_teammate_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_teammate_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_teammate_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );
				  
					 ImGui::Checkbox( XorStr( "Pulse effect" ), &g_Vars.esp.chams_teammate_pulse );
				  }

				  //set local death enemy chams
				  if ( current_chams_settings_for == 6 ) {
					 ImGui::ComboA( XorStr( "Death enemy material##enemy" ), &g_Vars.esp.enemy_chams_death_mat, materials, ARRAYSIZE( materials ) );

					 ImGui::Checkbox( XorStr( "Death enemy vis" ), &g_Vars.esp.enemy_death_chams_vis );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Death enemy Chams Color VIS" ), g_Vars.esp.enemy_death_chams_color_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Death enemy xqz" ), &g_Vars.esp.enemy_death_chams_xqz );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Death enemy Chams Color XQZ" ), g_Vars.esp.enemy_death_chams_color_xqz, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_enemy_death_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Local outline color" ), g_Vars.esp.chams_enemy_death_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_enemy_death_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_enemy_death_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );
				  
					 ImGui::Checkbox( XorStr( "Pulse effect" ), &g_Vars.esp.chams_enemy_death_pulse );
				  }

				  //set local death teammate chams
				  if ( current_chams_settings_for == 7 ) {
					 ImGui::ComboA( XorStr( "Death teammate material##teammate" ), &g_Vars.esp.team_chams_death_mat, materials, ARRAYSIZE( materials ) );

					 ImGui::Checkbox( XorStr( "Death teammate vis" ), &g_Vars.esp.team_death_chams_vis );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Death teammate Chams Color VIS" ), g_Vars.esp.team_death_chams_color_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Death teammate xqz" ), &g_Vars.esp.team_death_chams_xqz );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Death teammate Chams Color XQZ" ), g_Vars.esp.team_death_chams_color_xqz, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_teammate_death_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Local outline color" ), g_Vars.esp.chams_teammate_death_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_teammate_death_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_teammate_death_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );
				  
					 ImGui::Checkbox( XorStr( "Pulse effect" ), &g_Vars.esp.chams_teammate_death_pulse );
				  }

				  //set local enemy history chams
				  if ( current_chams_settings_for == 8 ) {
					 ImGui::ComboA( XorStr( "History material##ememy" ), &g_Vars.esp.chams_history_mat, materials, ARRAYSIZE( materials ) );

					 ImGui::Text( XorStr( "History color" ) );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Chams History Color" ), g_Vars.esp.chams_history_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				  
					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_history_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Local outline color" ), g_Vars.esp.chams_history_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_history_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_history_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );
				  
					 ImGui::Checkbox( XorStr( "Pulse effect" ), &g_Vars.esp.chams_history_pulse );
				  }

				  //set local lag chams
				  if ( current_chams_settings_for == 9 ) {
					 ImGui::ComboA( XorStr( "Lag material##local" ), &g_Vars.esp.chams_lag_mat, materials, ARRAYSIZE( materials ) );

					 ImGui::Text( XorStr( "Lag color" ) );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Lag color" ), g_Vars.esp.chams_lag_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					 
					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_lag_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Local outline color" ), g_Vars.esp.chams_lag_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_lag_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_lag_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );

					 ImGui::Checkbox( XorStr( "Pulse effect" ), &g_Vars.esp.chams_lag_pulse );
				  }

				  //set on shot chams
				  if ( current_chams_settings_for == 10 ) {
					 ImGui::ComboA( XorStr( "On shot material##enemy" ), &g_Vars.esp.chams_hitmatrix_mat, materials, ARRAYSIZE( materials ) );
					
					 ImGui::Text( XorStr( "On shot color" ) );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Chams Hitmatrix Color" ), g_Vars.esp.hitmatrix_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					 if ( g_Vars.esp.hitmatrix ) {
						ImGui::SliderFloatA( XorStr( "Time##chams" ), &g_Vars.esp.hitmatrix_time, 1.f, 10.f, XorStr( "%0.1f seconds" ) );
					 }

					 ImGui::Checkbox( XorStr( "Outline effect" ), &g_Vars.esp.chams_hitmatrix_outline );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Local outline color" ), g_Vars.esp.chams_hitmatrix_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.chams_hitmatrix_outline )
						ImGui::SliderFloatA( XorStr( "Outline intensive" ), &g_Vars.esp.chams_hitmatrix_outline_value, 1.f, 100.f, XorStr( "%0.0f" ) );
				  }

			   #if 0
				  const char* materials[] = { XorStr( "Off" ), XorStr( "Flat" ),  XorStr( "Material" ), XorStr( "Glow" ) };
				  const char* no_dis_materials[] = { XorStr( "Flat" ),  XorStr( "Material" ), XorStr( "Glow" ) };

				  const char* materials_wire[] = { XorStr( "Off" ), XorStr( "Flat" ), XorStr( "Flat with wireframe" ), XorStr( "Material" ), XorStr( "Material with wireframe" ), XorStr( "Glow" ) };
				  const char* no_dis_materials_wire[] = { XorStr( "Flat" ), XorStr( "Flat with wireframe" ), XorStr( "Material" ), XorStr( "Material with wireframe" ), XorStr( "Glow" ) };

				  const char* tabs[] = { XorStr( "Local" ), XorStr( "Enemy" ), XorStr( "Team" ) };
				  static int tab_id = 0;
				  static int tab_order[] = { 0, 1, 2 };

				  ImGui::TabLabels( tabs, 3, tab_id, tab_order );

				  ImGui::Dummy( ImVec2( 0, 2 ) );

				  if ( g_Vars.esp.chams_enabled ) {
					 if ( tab_id == 0 ) {
						ImGui::ComboA( XorStr( "Local material##Local material" ), &g_Vars.esp.chams_local_mat, materials, ARRAYSIZE( materials ) );
						ImGui::Text( XorStr( "Local Color" ) );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
						ImGui::ColorEdit4( XorStr( "##Local color" ), g_Vars.esp.chams_local_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						ImGui::Checkbox( XorStr( "Wireframe" ), &g_Vars.esp.wireframe_chams_l );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
						ImGui::ColorEdit4( XorStr( "##Wireframe Color" ), g_Vars.esp.wireframe_chams_color_l, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						ImGui::Checkbox( XorStr( "Hands" ), &g_Vars.esp.hands_chams );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
						ImGui::ColorEdit4( XorStr( "##Hands Color" ), g_Vars.esp.hands_chams_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						if ( g_Vars.esp.hands_chams ) {
						   ImGui::ComboA( XorStr( "Hands material##Hands Material" ), &g_Vars.esp.hands_chams_mat, no_dis_materials_wire, ARRAYSIZE( no_dis_materials_wire ) );
						}

						ImGui::Checkbox( XorStr( "Weapon" ), &g_Vars.esp.weapon_chams );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
						ImGui::ColorEdit4( XorStr( "##Weapon Color" ), g_Vars.esp.weapon_chams_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						if ( g_Vars.esp.weapon_chams ) {
						   ImGui::ComboA( XorStr( "Weapon material##Weapon Material" ), &g_Vars.esp.weapon_chams_mat, no_dis_materials_wire, ARRAYSIZE( no_dis_materials_wire ) );
						}

						ImGui::Checkbox( XorStr( "Lag chams" ), &g_Vars.esp.chams_lag );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
						ImGui::ColorEdit4( XorStr( "##Lag color" ), g_Vars.esp.chams_lag_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );


						ImGui::Checkbox( XorStr( "Desync chams" ), &g_Vars.esp.chams_desync );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
						ImGui::ColorEdit4( XorStr( "##Desync color" ), g_Vars.esp.chams_desync_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						if ( g_Vars.esp.chams_desync ) {
						   ImGui::ComboA( XorStr( "Desync material##Desync material" ), &g_Vars.esp.chams_desync_mat, no_dis_materials, ARRAYSIZE( no_dis_materials ) );
						}

						ImGui::Checkbox( XorStr( "Shine effect" ), &g_Vars.esp.second_color_l );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
						ImGui::ColorEdit4( XorStr( "##Double Coloring Color" ), g_Vars.esp.second_chams_color_l, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						if ( g_Vars.esp.second_color_l )
						   ImGui::SliderFloatA( XorStr( "Shine##chams" ), &g_Vars.esp.chams_shine_l, 1.f, 100.f, XorStr( "%0.0f" ) );

				  } else if ( tab_id == 1 ) {
					 ImGui::ComboA( XorStr( "Enemy Material##enemy" ), &g_Vars.esp.enemy_chams_mat, materials, ARRAYSIZE( materials ) );

					 ImGui::Checkbox( XorStr( "Enemy Vis" ), &g_Vars.esp.enemy_chams_vis );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Team Chams Color XQZ" ), g_Vars.esp.enemy_chams_color_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Enemy XQZ" ), &g_Vars.esp.enemy_chams_xqz );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Enemy Chams Color XQZ" ), g_Vars.esp.enemy_chams_color_xqz, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Wireframe" ), &g_Vars.esp.wireframe_chams_e );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Wireframe Color" ), g_Vars.esp.wireframe_chams_color_e, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "History" ), &g_Vars.esp.chams_history );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Chams History Color" ), g_Vars.esp.chams_history_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "On Shot" ), &g_Vars.esp.hitmatrix );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Chams Hitmatrix Color" ), g_Vars.esp.hitmatrix_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					 if ( g_Vars.esp.hitmatrix ) {
						ImGui::SliderFloatA( XorStr( "Time##chams" ), &g_Vars.esp.hitmatrix_time, 1.f, 10.f, XorStr( "%0.1f seconds" ) );
					 }

					 ImGui::Checkbox( XorStr( "Shine effect" ), &g_Vars.esp.second_color_e );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Double Coloring Color" ), g_Vars.esp.second_chams_color_e, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.second_color_e )
						ImGui::SliderFloatA( XorStr( "Shine##chams" ), &g_Vars.esp.chams_shine_e, 1.f, 100.f, XorStr( "%0.0f" ) );

					 ImGui::Checkbox( XorStr( "On death" ), &g_Vars.esp.chams_on_death_enemy );
				  } else if ( tab_id == 2 ) {
					 ImGui::ComboA( XorStr( "Team Material##team" ), &g_Vars.esp.team_chams_mat, materials, ARRAYSIZE( materials ) );

					 ImGui::Checkbox( XorStr( "Team Vis" ), &g_Vars.esp.team_chams_vis );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Team Chams Color Vis" ), g_Vars.esp.team_chams_color_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Team XQZ" ), &g_Vars.esp.team_chams_xqz );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Enemy Chams Color Vis" ), g_Vars.esp.team_chams_color_xqz, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Wireframe" ), &g_Vars.esp.wireframe_chams_t );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Wireframe Color" ), g_Vars.esp.wireframe_chams_color_t, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 ImGui::Checkbox( XorStr( "Shine effect" ), &g_Vars.esp.second_color_t );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Double Coloring Color" ), g_Vars.esp.second_chams_color_t, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 if ( g_Vars.esp.second_color_t )
						ImGui::SliderFloatA( XorStr( "Shine##chams" ), &g_Vars.esp.chams_shine_t, 1.f, 100.f, XorStr( "%0.0f" ) );

					 ImGui::Checkbox( XorStr( "On death" ), &g_Vars.esp.chams_on_death_teamate );
				  }
			   }
			   #endif 
			} ImGui::EndChild( );

			ImGui::BeginChild( "Other Esp", ImVec2( 0.0f, 145.0f ), true );
			{
			   ImGui::Text( "Other" );
			   ImGui::Separator( );
			   static bool* event_logger_cond[] = {
					 &g_Vars.esp.event_bomb,
					 &g_Vars.esp.event_dmg,
					 &g_Vars.esp.event_buy,
					 &g_Vars.esp.event_resolver,
					 &g_Vars.esp.event_misc,
					 &g_Vars.esp.event_exploits,
					 &g_Vars.esp.event_console,
					 &g_Vars.esp.event_aimbot,
			   };

			   const char* event_logger_cond_str[] = {
				  XorStr( "Bomb" ), XorStr( "Damage" ), XorStr( "Buy" ), XorStr( "Resolver" ), XorStr( "Misc" ), XorStr( "Exploits" ),
				  XorStr( "Console" ), XorStr( "Aimbot" )
			   };

			   ImGui::MultiCombo( XorStr( "Event logger" ),
								  event_logger_cond, event_logger_cond_str, 8 );

			   const char* logger_type[] = { XorStr( "Default" ), XorStr( "Nvidia" ) };

			   ImGui::ComboA( XorStr( "Event Logger Type" ), &g_Vars.esp.event_logger_type, logger_type, ARRAYSIZE( logger_type ) );

			   static bool* removeals_cond[] = {
				  &g_Vars.esp.remove_smoke,
				  &g_Vars.esp.remove_flash,
				  &g_Vars.esp.remove_scope,
				  &g_Vars.esp.remove_scope_zoom,
				  &g_Vars.esp.remove_scope_blur,
				  &g_Vars.esp.remove_recoil,
				  &g_Vars.esp.remove_sleeves,
				  &g_Vars.esp.remove_blur_effect,
				  &g_Vars.esp.remove_post_proccesing
			   };

			   const char* removeals_cond_str[] = {
				  XorStr( "Smoke" ), XorStr( "Flash" ), XorStr( "Scope overlay" ), XorStr( "Zoom" ), XorStr( "Zoom blur" ), XorStr( "Recoil" ), XorStr( "Sleeves" ),
				  XorStr( "Blur" ), XorStr( "Post Proccesing" )
			   };

			   ImGui::MultiCombo( XorStr( "Removals" ),
								  removeals_cond, removeals_cond_str, 9 );

			   static bool* no_render_cond[] = {
				  &g_Vars.misc.disable_enemies ,
				  &g_Vars.misc.disable_teammates,
				  &g_Vars.misc.disable_ragdolls,
				  &g_Vars.misc.disable_weapons,
			   };

			   const char* no_render_str[] = {
				  XorStr( "Enemies" ), XorStr( "Teammates" ), XorStr( "Ragdolls" ), XorStr( "Weapons" ),
			   };

			   ImGui::MultiCombo( XorStr( "Disable rendering" ),
								  no_render_cond, no_render_str, 4 );

			   ImGui::Checkbox( XorStr( "Visualize sounds" ), &g_Vars.esp.sound_esp );
			   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
			   ImGui::ColorEdit4( XorStr( "##SoundEsp Color" ), g_Vars.esp.sound_esp_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
			   if ( g_Vars.esp.sound_esp ) {
				  const char* sound_esp_type_str[] = { XorStr( "Beam" ), XorStr( "Default" ) };

				  ImGui::ComboA( XorStr( "Sound type##TypeSoundEsp" ), &g_Vars.esp.sound_esp_type, sound_esp_type_str, ARRAYSIZE( sound_esp_type_str ) );
				  ImGui::SliderFloatA( XorStr( "Time##SEsp" ), &g_Vars.esp.sound_esp_time, 0.f, 8.0f, XorStr( "%.1f seconds" ) );
				  ImGui::SliderFloatA( XorStr( "Radius##SEsp" ), &g_Vars.esp.sound_esp_radius, 0.f, 500.0f, XorStr( "%.1f units" ) );
			   }
			   ImGui::Checkbox( XorStr( "Offscreen arrow" ), &g_Vars.esp.offscren_enabled );
			   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
			   ImGui::ColorEdit4( XorStr( "##Offscreen Color" ), g_Vars.esp.offscreen_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
			   if ( g_Vars.esp.offscren_enabled ) {
				  ImGui::Checkbox( XorStr( "Offscreen pulse alpha" ), &g_Vars.esp.pulse_offscreen_alpha );
				  ImGui::Checkbox( XorStr( "Offscreen outline" ), &g_Vars.esp.offscreen_outline );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##Offscreen outline Color" ), g_Vars.esp.offscreen_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				  ImGui::SliderIntA( XorStr( "Size##Off" ), &g_Vars.esp.offscren_size, 0, 50, XorStr( "%i px" ) );
				  ImGui::SliderIntA( XorStr( "Distance##iOff" ), &g_Vars.esp.offscren_distance, 0, 700, XorStr( "%i px" ) );
			   }

			} ImGui::EndChild( );

			static bool copy_ip = false;
			ImGui::Text( g_Vars.globals.server_adress.c_str( ) );
			if ( ImGui::IsItemClicked( 0 ) || copy_ip ) {
			   copy_ip = true;
			   ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
			   ImGui::OpenPopup( "Copy ip##unload_popup7" );
			   if ( ImGui::BeginPopupModal( XorStr( "Copy ip##unload_popup7" ) ), NULL, dwFlag ) {
				  ImGui::Text( XorStr( "Are you sure?" ) );
				  if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
					 ImGui::ToClipboard( g_Vars.globals.server_adress.c_str( ) );
					 ILoggerEvent::Get( )->PushEvent( XorStr( "Ip address copied!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
					 copy_ip = false;
					 ImGui::CloseCurrentPopup( );
				  }
				  ImGui::SameLine( );
				  if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
					 ImGui::CloseCurrentPopup( );
					 copy_ip = false;
				  }

				  ImGui::EndPopup( );
			   }
			}

			ImGui::PopStyleColor( 1 );
			break;
		 }
			case 4:
			{
			   ImGui::Columns( 2, NULL, false );
			   ImGui::SetColumnWidth( 0, full_window.x * 0.5f );
			   ImGui::SetColumnWidth( 1, full_window.x );

			   ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, ImVec4( 30 / 255.f, 30 / 255.f, 39 / 255.f, 1.0f ) );

			   ImGui::BeginChild( "Visuals##11", ImVec2( 0.0f, 470.0f ), true );
			   {
				  ImGui::Text( "Visuals" );
				  ImGui::Separator( );

				  ImGui::Checkbox( XorStr( "Grenade prediction" ), &g_Vars.esp.NadePred );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##nade_pred_color Color" ), g_Vars.esp.nade_pred_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip ); \

					 ImGui::Checkbox( XorStr( "Bullet tracer" ), &g_Vars.esp.beam_enabled );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##BulletTracer Color" ), g_Vars.esp.beam_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

				  if ( g_Vars.esp.beam_enabled ) {
					 const char* beam_type_str[] = { XorStr( "Physbeam" ), XorStr( "Blue glow" ), XorStr( "Bubble" ), XorStr( "Glow" ), XorStr( "Purple glow" ), XorStr( "Purple laser" ), XorStr( "Radio" ), XorStr( "White" ) };

					 ImGui::ComboA( XorStr( "Tracer type##BulletTracer" ), &g_Vars.esp.beam_type, beam_type_str, ARRAYSIZE( beam_type_str ) );
					 ImGui::SliderFloatA( XorStr( "Time##BulletTracer" ), &g_Vars.esp.beam_life_time, 0.f, 10.0f, XorStr( "%.1f seconds" ) );
					 ImGui::SliderFloatA( XorStr( "Speed##BulletTracer" ), &g_Vars.esp.beam_speed, 0.0f, 10.0f, XorStr( "%.1f units" ) );
					 ImGui::SliderFloatA( XorStr( "Width##BulletTracer" ), &g_Vars.esp.beam_width, 1.f, 10.0f, XorStr( "%.1f size" ) );
				  }

				  const char* hitmarkers_conditions_str[] = { XorStr( "Lines" ), XorStr( "Damage" ) };

				  static bool* hitmarkers_conditions[] = {
					 &g_Vars.esp.vizualize_hitmarker,
					 &g_Vars.esp.vizualize_hitmarker_damage,
				  };

				  ImGui::MultiCombo( XorStr( "Hitmarker" ),
									 hitmarkers_conditions, hitmarkers_conditions_str, 2 );

				  ImGui::Checkbox( XorStr( "Hit sound" ), &g_Vars.misc.hitsound );

				  const char* hitsound_type_str[] = { XorStr( "Switch" ), XorStr( "Bubble" ), };

				  if ( g_Vars.misc.hitsound ) {
					 ImGui::ComboA( XorStr( "Hit sound type##HitSound Type##HitSound" ), &g_Vars.misc.hitsound_type, hitsound_type_str, ARRAYSIZE( hitsound_type_str ) );
				  }

				  ImGui::Checkbox( XorStr( "Snaplines" ), &g_Vars.esp.snaplines_enalbed );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##snaplines Color" ), g_Vars.esp.snaplines_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

				  const char* snaplines_str[] = { XorStr( "Left" ), XorStr( "Right" ), XorStr( "Top" ), XorStr( "Bottom" ), XorStr( "Center" ) };
				  if ( g_Vars.esp.snaplines_enalbed ) {
					 ImGui::ComboA( XorStr( "Position##snaplines pos" ), &g_Vars.esp.snaplines_pos, snaplines_str, ARRAYSIZE( snaplines_str ) );
				  }
				  ImGui::Checkbox( XorStr( "Visualization angles" ), &g_Vars.esp.vizualize_angles );
				  ImGui::Checkbox( XorStr( "Manual Anti-Aim indicators" ), &g_Vars.esp.aa_indicator );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##Manual Color" ), g_Vars.esp.aa_indicator_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

				  const char* manual_str[] = { XorStr( "Around the crosshair" ), XorStr( "Bottom" ) };
				  if ( g_Vars.esp.aa_indicator ) {
					 ImGui::ComboA( XorStr( "Manual Type##Manual Type" ), &g_Vars.esp.aa_indicator_type, manual_str, ARRAYSIZE( manual_str ) );
					 ImGui::Checkbox( XorStr( "Zeus distance" ), &g_Vars.esp.zeus_distance );
				  }
				  ImGui::Checkbox( XorStr( "Force crosshair" ), &g_Vars.esp.force_sniper_crosshair );
				  ImGui::Checkbox( XorStr( "Dark Mode" ), &g_Vars.esp.dark_mode );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##Dark mode shade Color" ), g_Vars.esp.dark_mode_shade, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				  if ( g_Vars.esp.dark_mode )
					 ImGui::SliderFloatA( XorStr( "Brightness value##Dark Mode" ), &g_Vars.esp.dark_mode_value, 0.f, 100.0f, XorStr( "%.1f" ) );

				  ImGui::Checkbox( XorStr( "Fullbright" ), &g_Vars.esp.fullbright );

				  if ( !g_Vars.esp.dark_mode ) {
					 ImGui::Checkbox( XorStr( "Walls modulation" ), &g_Vars.esp.walls );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##wall_modulation Color" ), g_Vars.esp.wall_modulation, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				  }

				  ImGui::Checkbox( XorStr( "Props modulation" ), &g_Vars.esp.props );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##props_modulation Color" ), g_Vars.esp.props_modulation, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

				  ImGui::Checkbox( XorStr( "Skybox modulation" ), &g_Vars.esp.skybox );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##skybox_modulation Color" ), g_Vars.esp.skybox_modulation, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

				  ImGui::Checkbox( XorStr( "Kill effect" ), &g_Vars.esp.kill_effect );
				  if ( g_Vars.esp.kill_effect ) {
					 ImGui::SliderFloatA( XorStr( "Effect time##kill effect" ), &g_Vars.esp.kill_effect_time, 0.f, 5.f, XorStr( "%.1f seconds" ) );
				  }

				  ImGui::Checkbox( XorStr( "Fog effect" ), &g_Vars.esp.fog_effect );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##fog color" ), g_Vars.esp.fog_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				  ImGui::Checkbox( XorStr( "Fog blind " ), &g_Vars.esp.fog_blind );;
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##fog color secondary" ), g_Vars.esp.fog_color_secondary, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				  if ( g_Vars.esp.fog_effect ) {
					 ImGui::SliderFloatA( XorStr( "Fog density" ), &g_Vars.esp.fog_density, 0.f, 100.f, XorStr( "%.1f " ) );
					 ImGui::SliderIntA( XorStr( "Fog distance" ), &g_Vars.esp.fog_distance, 1000, 5000, XorStr( "%d units" ) );
					 ImGui::SliderFloatA( XorStr( "Fog hdr scale" ), &g_Vars.esp.fog_hdr_scale, 0.f, 100.f, XorStr( "%.1f " ) );
				  }


			   } ImGui::EndChild( );

			#ifdef BETA_MODE
			   ImGui::Text( XorStr( "eexomi.host [BETA] | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#else 
			   ImGui::Text( XorStr( "eexomi.host v2 | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#endif

			   ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );
			   ImGui::NextColumn( );
			   ImGui::PopStyleVar( );

			   ImGui::BeginChild( "Visuals##22", ImVec2( 0.0f, 470.0f ), true );
			   {
				  ImGui::Text( "Visuals" );
				  ImGui::Separator( );
				  ImGui::Checkbox( XorStr( "Preserve killFeed" ), &g_Vars.esp.preserve_killfeed );
				  if ( g_Vars.esp.preserve_killfeed ) {
					 ImGui::SliderFloatA( XorStr( "Time##PreserveKillfeed" ), &g_Vars.esp.preserve_killfeed_time, 1.f, 300.f, XorStr( "%.0f ms" ) );
				  }

				  static bool* bomb_conditions[] = {
					 &g_Vars.esp.draw_c4,
					 &g_Vars.esp.draw_c4_bar,
				  };

				  const char* bomb_conditions_str[] = {
					 XorStr( "Timer" ), XorStr( "Bar" ),
				  };

				  ImGui::MultiCombo( XorStr( "Planted bomb" ),
									 bomb_conditions, bomb_conditions_str, 2 );

				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##c4 Color" ), g_Vars.esp.c4_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

				  ImGui::Checkbox( XorStr( "Nades Text" ), &g_Vars.esp.nades );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##Nades text Color" ), g_Vars.esp.nades_text_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				  ImGui::Checkbox( XorStr( "Nades Box" ), &g_Vars.esp.nades_box );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##Nades box Color" ), g_Vars.esp.nades_box_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

				  ImGui::Checkbox( XorStr( "Penetrate crosshair" ), &g_Vars.esp.autowall_crosshair );
				  if ( g_Vars.esp.autowall_crosshair ) {
					 ImGui::SliderFloatA( XorStr( "Size##iOff" ), &g_Vars.esp.autowall_crosshair_height, 1.f, 30.0f, XorStr( "%.0f px" ) );
				  }

				  ImGui::Checkbox( XorStr( "Dropped weapons" ), &g_Vars.esp.dropped_weapons );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##Dropped Weapons Color" ), g_Vars.esp.dropped_weapons_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

				  const char* weapon_type_str[] = { XorStr( "Off" ), XorStr( "2D" ), XorStr( "3D" ) };
				  if ( g_Vars.esp.dropped_weapons ) {
					 ImGui::ComboA( XorStr( "Type##Weapons Type" ), &g_Vars.esp.dropped_weapons_type, weapon_type_str, ARRAYSIZE( weapon_type_str ) );
				  }
				  const char* SkyNames[] = {
					 XorStr( "Default" ),
					 XorStr( "cs_baggage_skybox_" ),
					 XorStr( "cs_tibet" ),
					 XorStr( "embassy" ),
					 XorStr( "italy" ),
					 XorStr( "jungle" ),
					 XorStr( "nukeblank" ),
					 XorStr( "office" ),
					 XorStr( "sky_csgo_cloudy01" ),
					 XorStr( "sky_csgo_night02" ),
					 XorStr( "sky_csgo_night02b" ),
					 XorStr( "sky_dust" ),
					 XorStr( "sky_venice" ),
					 XorStr( "vertigo" ),
					 XorStr( "vietnam" ),
					 XorStr( "sky_descent" )
				  };

				  ImGui::ComboA( XorStr( "Skybox " ), &g_Vars.esp.sky_changer, SkyNames, ARRAYSIZE( SkyNames ) );
				  ImGui::Checkbox( XorStr( "Spread overlay" ), &g_Vars.esp.fov_crosshair );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##Fov Crosshair Color" ), g_Vars.esp.fov_crosshair_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				  static bool* list_conditions[] = {
			 &g_Vars.esp.stat_logger,
			 &g_Vars.esp.spectator_list,
			 &g_Vars.esp.keybind_list,
			 &g_Vars.esp.indicators_list,
				  };

				  const char* list_conditions_str[] = {
					 XorStr( "Hitlogger" ), XorStr( "Spectators" ),XorStr( "Keybinds" ), XorStr( "Indicators" ),
				  };

				  ImGui::MultiCombo( XorStr( "Lists" ), list_conditions, list_conditions_str, 4 );


				  ImGui::Checkbox( XorStr( "Watermark" ), &g_Vars.esp.watermark );

				  static bool* watermark_conditions[] = {
				&g_Vars.esp.watermark_name,
				&g_Vars.esp.watermark_ip,
				&g_Vars.esp.watermark_fps,
				&g_Vars.esp.watermark_ping,
				&g_Vars.esp.watermark_time
				  };

				  const char* watermark_conditions_str[] = {
					 XorStr( "Name" ), XorStr( "Ip" ),XorStr( "Fps" ), XorStr( "Ping" ),XorStr( "Time" )
				  };

				  if ( g_Vars.esp.watermark ) {
					 ImGui::MultiCombo( XorStr( "Watermark data" ),
										watermark_conditions, watermark_conditions_str, 5 );
				  }

				  ImGui::Checkbox( XorStr( "Zoom## priblijenye naxoy" ), &g_Vars.esp.zoom );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
				  ImGui::KeyBox( "", &g_Vars.esp.zoom_key );
				  if ( g_Vars.esp.zoom ) {
					 ImGui::SliderFloatA( XorStr( "Zoom value" ), &g_Vars.esp.zoom_value, 0.f, 100.f, XorStr( "%.2f" ) );
				  }

				  ImGui::Checkbox( XorStr( "Bloom effect" ), &g_Vars.esp.bloom_effect );
				  if ( g_Vars.esp.bloom_effect ) {
					 ImGui::SliderFloatA( XorStr( "World exposure" ), &g_Vars.esp.exposure_scale, 0.0f, 100.0f, XorStr( "%.0f" ) );
					 ImGui::SliderFloatA( XorStr( "Model ambient" ), &g_Vars.esp.model_brightness, 0.0f, 100.0f, XorStr( "%.0f" ) );
					 ImGui::SliderFloatA( XorStr( "Bloom scale" ), &g_Vars.esp.bloom_scale, 0.0f, 100.0f, XorStr( "%.0f" ) );
				  }

				  static bool* explosive_conditions[] = {
					 &g_Vars.esp.molotov_indicator,
					 &g_Vars.esp.smoke_indicator,
				  };

				  const char* explosive_conditions_str[] = {
					 XorStr( "Molotov" ), XorStr( "Smoke" )
				  };

				  ImGui::MultiCombo( XorStr( "Explosives" ),
									 explosive_conditions, explosive_conditions_str, 2 );

			   } ImGui::EndChild( );

			   static bool copy_ip = false;
			   ImGui::Text( g_Vars.globals.server_adress.c_str( ) );
			   if ( ImGui::IsItemClicked( 0 ) || copy_ip ) {
				  copy_ip = true;
				  ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
				  ImGui::OpenPopup( "Copy ip##unload_popup7" );
				  if ( ImGui::BeginPopupModal( XorStr( "Copy ip##unload_popup7" ) ), NULL, dwFlag ) {
					 ImGui::Text( XorStr( "Are you sure?" ) );
					 if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::ToClipboard( g_Vars.globals.server_adress.c_str( ) );
						ILoggerEvent::Get( )->PushEvent( XorStr( "Ip address copied!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
						copy_ip = false;
						ImGui::CloseCurrentPopup( );
					 }
					 ImGui::SameLine( );
					 if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::CloseCurrentPopup( );
						copy_ip = false;
					 }

					 ImGui::EndPopup( );
				  }
			   }
			   ImGui::PopStyleColor( 1 );

			   break;
			}
			case 5:
			{
			   ImGui::Columns( 2, NULL, false );
			   ImGui::SetColumnWidth( 0, full_window.x * 0.5f );
			   ImGui::SetColumnWidth( 1, full_window.x );

			   ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, ImVec4( 30 / 255.f, 30 / 255.f, 39 / 255.f, 1.0f ) );

			   ImGui::BeginChild( "Miscellaneous", ImVec2( 0.0f, 470.0f ), true );
			   {
				  ImGui::Text( "Miscellaneous" );
				  ImGui::Separator( );
				  ImGui::Checkbox( XorStr( "Enabled##Misc" ), &g_Vars.misc.active );
				  if ( g_Vars.misc.active ) {
					 ImGui::Checkbox( XorStr( "Auto jump" ), &g_Vars.misc.autojump );
					 ImGui::Checkbox( XorStr( "Auto strafe" ), &g_Vars.misc.autostrafer );
					 ImGui::Checkbox( XorStr( "Directional strafe" ), &g_Vars.misc.autostrafer_wasd );
					 if ( g_Vars.misc.autostrafer_wasd ) {
						ImGui::SliderFloatA( XorStr( "Turn smoothness" ), &g_Vars.misc.autostrafer_retrack, 0.0f, 8.0f, XorStr( "%.0f units" ) );
					 }
					 ImGui::Checkbox( XorStr( "In-game radar" ), &g_Vars.misc.ingame_radar );
					 ImGui::Checkbox( XorStr( "Bypass sv_pure" ), &g_Vars.misc.sv_pure_bypass );
					 ImGui::Checkbox( XorStr( "Unlock inventory" ), &g_Vars.misc.unlock_inventory );
					 ImGui::Checkbox( XorStr( "Override player view" ), &g_Vars.misc.override_player_view );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
					 ImGui::KeyBox( "", &g_Vars.misc.override_player_view_key );
					 ImGui::Checkbox( XorStr( "Fast duck" ), &g_Vars.misc.fastduck );
					 ImGui::Checkbox( XorStr( "Fake duck" ), &g_Vars.misc.fakeduck );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
					 ImGui::KeyBox( "", &g_Vars.misc.fakeduck_bind );
					 ImGui::Checkbox( XorStr( "Mini jump" ), &g_Vars.misc.minijump );
					 ImGui::Checkbox( XorStr( "Edge jump" ), &g_Vars.misc.edgejump );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
					 ImGui::KeyBox( "", &g_Vars.misc.edgejump_bind );
					 ImGui::Checkbox( XorStr( "Money revealer" ), &g_Vars.misc.money_revealer );
					 ImGui::Checkbox( XorStr( "Auto accept" ), &g_Vars.misc.auto_accept );
					 ImGui::Checkbox( XorStr( "Duck jump" ), &g_Vars.misc.duckjump );
					 ImGui::Checkbox( XorStr( "Quick stop" ), &g_Vars.misc.quickstop );
					 ImGui::Checkbox( XorStr( "Accurate walk" ), &g_Vars.misc.accurate_walk );
					 ImGui::Checkbox( XorStr( "Slide walk" ), &g_Vars.misc.slide_walk );
					 ImGui::Checkbox( XorStr( "Slow walk" ), &g_Vars.misc.slow_walk );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
					 ImGui::KeyBox( "", &g_Vars.misc.slow_walk_bind );
					 if ( g_Vars.misc.slow_walk ) {
						ImGui::SliderFloatA( XorStr( "Speed##slow_walk_speed" ), &g_Vars.misc.slow_walk_speed, 1.0f, 100.0f, XorStr( "%.0f %%" ) );
					 }

					 ImGui::Checkbox( XorStr( "Auto peek" ), &g_Vars.misc.autopeek );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
					 ImGui::KeyBox( "", &g_Vars.misc.autopeek_bind );
					 if ( g_Vars.misc.autopeek ) {
						ImGui::Checkbox( XorStr( "Auto peek visualise" ), &g_Vars.misc.autopeek_visualise );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
						ImGui::ColorEdit4( XorStr( "##Auto peek color" ), g_Vars.misc.autopeek_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					 }

					 ImGui::Checkbox( XorStr( "Knifebot" ), &g_Vars.misc.knife_bot );
					 const char* knife_bot_str[] = { XorStr( "Default" ), XorStr( "Backstab" ), XorStr( "Quick" ) };
					 if ( g_Vars.misc.knife_bot )
						ImGui::ComboA( XorStr( "Knifebot type##Knife bot type" ), &g_Vars.misc.knife_bot_type, knife_bot_str, ARRAYSIZE( knife_bot_str ) );

					 ImGui::Checkbox( XorStr( "Extended bactkrack" ), &g_Vars.misc.extended_backtrack );
					 if ( g_Vars.misc.extended_backtrack ) {
						ImGui::SliderFloatA( XorStr( "Extend window" ), &g_Vars.misc.extended_backtrack_time, 50.0f, 200.0f, XorStr( "%.0f ms" ) );
					 }

					 ImGui::Checkbox( XorStr( "Client impacts" ), &g_Vars.misc.impacts_spoof );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Client impacts color" ), g_Vars.esp.client_impacts, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
					 ImGui::Checkbox( XorStr( "Server impacts" ), &g_Vars.misc.server_impacts_spoof );
					 ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
					 ImGui::ColorEdit4( XorStr( "##Server impacts color" ), g_Vars.esp.server_impacts, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

					 g_Vars.misc.chat_spammer = false;
					 // InsertCheckbox( XorStr( "Chat spammer" ), g_Vars.misc.chat_spammer );

					 const char* clantags[] = {
						XorStr( "Static" ),
						XorStr( "Animated" ),
					 };

					 static bool* indicator_conditions[] = {
						&g_Vars.esp.indicator_side,
						&g_Vars.esp.indicator_lc,
						&g_Vars.esp.indicator_lag,
						&g_Vars.esp.indicator_exploits,
						&g_Vars.esp.indicator_fake,
						&g_Vars.esp.indicator_fake_duck,
					 };

					 const char* indicator_conditions_str[] = {
						XorStr( "Desync side" ), XorStr( "LC" ), XorStr( "Lag" ), XorStr( "Exploits" ),
						XorStr( "Fake" ), XorStr( "Fake Duck" ),
					 };

					 ImGui::MultiCombo( XorStr( "Indicators" ),
										indicator_conditions, indicator_conditions_str, 6 );
					 ImGui::Checkbox( XorStr( "Clan-tag" ), &g_Vars.misc.clantag_changer );
					 ImGui::Checkbox( XorStr( "Name changer" ), &g_Vars.misc.name_changer );

					 static char buffer[ 128 ] = { ( ( '\0' ) ) };
					 static char buffer2[ 128 ] = { ( ( '\0' ) ) };
					 if ( g_Vars.misc.name_changer ) {
						ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( );
						ImGui::SameLine( 19.f );
						ImGui::InputText( XorStr( "Name##name changer" ), buffer2, 128 );

						ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( );
						ImGui::SameLine( 19.f );
						if ( ImGui::Button( XorStr( "Apply name" ) ) ) {
						   g_Vars.misc.name_changer_str = buffer2;

						   static ConVar* cName = Source::m_pCvar->FindVar( XorStr( "name" ) );
						   *( int* ) ( ( uintptr_t ) &cName->fnChangeCallback + 0xC ) = 0;
						   cName->SetValue( buffer2 );
						}
					 }
				  }

			   } ImGui::EndChild( );

			   ImGui::Text( XorStr( "eexomi.host v2 | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );

			   ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );
			   ImGui::NextColumn( );
			   ImGui::PopStyleVar( );

			   ImGui::BeginChild( "Settings", ImVec2( 0.0f, 235.0f ), true );
			   {
				  ImGui::Text( "Settings" );
				  ImGui::Separator( );
				  ImGui::Text( "Menu accent" );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.5f );
				  ImGui::ColorEdit4( XorStr( "##menu accent" ), g_Vars.misc.menu_ascent, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
				  const char* server_for_pick_region[] = {
					 XorStr( "All" ),
					 XorStr( "Amsterdam, Netherlands" ),
					 XorStr( "Atlanta, USA" ),
					 XorStr( "Mumbai, India" ),
					 XorStr( "Dubai, UAE" ),
					 XorStr( "Frankfurt, Germany" ),
					 XorStr( "Bellevue, USA" ),
					 XorStr( "Sao Paulo, Brazil" ),
					 XorStr( "Hong Kong" ),
					 XorStr( "Washington DC, USA" ),
					 XorStr( "Cape Town, South Africa" ),
					 XorStr( "Los Angeles, USA" ),
					 XorStr( "London, UK" ),
					 XorStr( "Lima, Peru" ),
					 XorStr( "Luxembourg" ),
					 XorStr( "Chennai, India" ),
					 XorStr( "Madrid, Spain" ),
					 XorStr( "Manila, Philippines" ),
					 XorStr( "Moses Lake, USA" ),
					 XorStr( "Oklahoma, USA" ),
					 XorStr( "Chicago, USA" ),
					 XorStr( "Paris, France" ),
					 XorStr( "Santiago, Chile" ),
					 XorStr( "Seattle, USA" ),
					 XorStr( "Singapore" ),
					 XorStr( "Stockholm, Sweden" ),
					 XorStr( "Stockholm, Sweden 2" ),
					 XorStr( "Sydney, Australia" ),
					 XorStr( "Bellevue, USA" ),
					 XorStr( "Tokyo, Japan" ),
					 XorStr( "Vienna, Austria" ),
					 XorStr( "Warsaw, Poland" ),
				  };
				  ImGui::ComboA( XorStr( "Server Region" ), &g_Vars.misc.server_region, server_for_pick_region, ARRAYSIZE( server_for_pick_region ) );
				  ImGui::SliderIntA( XorStr( "Max ping " ), &g_Vars.misc.search_max_ping, 5, 350, XorStr( "%d ping" ) );

				  ImGui::Checkbox( XorStr( "Anti Untrusted" ), &g_Vars.misc.anti_untrusted );
				  ImGui::Checkbox( XorStr( "Obs bypass" ), &g_Vars.misc.obs_bypass );
				  ImGui::Checkbox( XorStr( "Third person" ), &g_Vars.misc.third_person );
				  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
				  ImGui::KeyBox( "", &g_Vars.misc.third_person_bind );
				  if ( g_Vars.misc.third_person ) {
					 ImGui::Checkbox( XorStr( "Disable on grenade" ), &g_Vars.misc.off_third_person_on_grenade );
					 ImGui::SliderFloatA( XorStr( "Distance " ), &g_Vars.misc.third_person_dist, 0, 250, XorStr( "%.0f units" ) );
				  }

				  ImGui::SliderFloatA( XorStr( "View world FOV " ), &g_Vars.esp.world_fov, 0.f, 200.f, XorStr( "%.0f degress" ) );
				  ImGui::Checkbox( XorStr( "View model changer" ), &g_Vars.misc.viewmodel_change );

				  if ( g_Vars.misc.viewmodel_change ) {
					 ImGui::SliderFloatA( XorStr( "View model FOV " ), &g_Vars.misc.viewmodel_fov, 0.f, 200.f, XorStr( "%.0f degress" ) );
					 ImGui::SliderFloatA( XorStr( "View model x" ), &g_Vars.misc.viewmodel_x, -20.f, 20.f, XorStr( "%.0f" ) );
					 ImGui::SliderFloatA( XorStr( "View model y" ), &g_Vars.misc.viewmodel_y, -20.f, 20.f, XorStr( "%.0f" ) );
					 ImGui::SliderFloatA( XorStr( "View model z" ), &g_Vars.misc.viewmodel_z, -20.f, 20.f, XorStr( "%.0f" ) );
				  }
				  ImGui::Checkbox( XorStr( "Aspect ratio" ), &g_Vars.esp.aspect_ratio );
				  if ( g_Vars.esp.aspect_ratio ) {
					 ImGui::SliderFloatA( XorStr( "Aspect ratio value" ), &g_Vars.esp.aspect_ratio_value, 0.02f, 5.f, XorStr( "%.2f" ) );
				  }
				  const char* first_weapon_str[] = {
					XorStr( "Scar 20 / G3SG1" ),
					XorStr( "Ssg 08" ),
					XorStr( "Awp" ),
				  };

				  const char* second_weapon_str[] = {
					XorStr( "Elite" ),
					XorStr( "Deagle / R8" ),
					XorStr( "CZ75 Auto" ),
					XorStr( "Tec 9" ),
				  };

				  const char* other_weapon_str[] = {
					XorStr( "Armor" ),
					XorStr( "Flashbang" ),
					XorStr( "Hegrenade" ),
					XorStr( "Molotov" ),
					XorStr( "Smoke" ),
					XorStr( "Decoy" ),
					XorStr( "Taser" ),
					XorStr( "Defuse Kit" ),
				  };

				  static bool* other_weapon_conditions[] = {
					 &g_Vars.misc.autobuy_armor	,
					 &g_Vars.misc.autobuy_flashbang,
					 &g_Vars.misc.autobuy_hegrenade	,
					 &g_Vars.misc.autobuy_molotovgrenade,
					 &g_Vars.misc.autobuy_smokegreanade,
					 &g_Vars.misc.autobuy_decoy,
					 &g_Vars.misc.autobuy_zeus,
					 &g_Vars.misc.autobuy_defusekit,
				  };

				  ImGui::Checkbox( XorStr( "Autobuy" ), &g_Vars.misc.autobuy_enabled );
				  if ( g_Vars.misc.autobuy_enabled ) {
					 ImGui::ComboA( XorStr( "First Weapon" ), &g_Vars.misc.autobuy_first_weapon, first_weapon_str, ARRAYSIZE( first_weapon_str ) );
					 ImGui::ComboA( XorStr( "Second Weapon" ), &g_Vars.misc.autobuy_second_weapon, second_weapon_str, ARRAYSIZE( first_weapon_str ) );

					 ImGui::MultiCombo( XorStr( "Other" ),
										other_weapon_conditions, other_weapon_str, 8 );
				  }

			   } ImGui::EndChild( );

			   ImGui::BeginChild( "Config", ImVec2( 0.0f, 230.0f ), true );
			   {
				  ImGui::Text( "Config" );
				  ImGui::Separator( );

				  static bool FirstTime = true;
				  if ( FirstTime || ( ImGui::GetFrameCount( ) % 100 ) == 0 ) {
					 cfg_list = ConfigManager::GetConfigs( );
					 FirstTime = false;
				  }

				  ImGui::BeginGroup( );
				  {
					 ImGui::InputText( "", config_name, 256 );
					 ImGui::ListBox( "", &voted_cfg, cfg_list, 8 );
				  } ImGui::EndGroup( );

				  ImGui::SameLine( );
				  ImGui::BeginGroup( );
				  {
					 static bool open_c = false;

					 if ( ImGui::Button( XorStr( "Open folder" ), ImVec2( -1.f, 28.f ) ) || open_c ) {
						open_c = true;
						ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
						ImGui::OpenPopup( "Open folder##unload_popup6" );
						if ( ImGui::BeginPopupModal( XorStr( "Open folder##unload_popup6" ) ), NULL, dwFlag ) {
						   ImGui::Text( XorStr( "Are you sure?" ) );
						   if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
							  ConfigManager::OpenConfigFolder( );
							  open_c = false;
							  ImGui::CloseCurrentPopup( );
						   }
						   ImGui::SameLine( );
						   if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
							  ImGui::CloseCurrentPopup( );
							  open_c = false;
						   }

						   ImGui::EndPopup( );
						}
					 }

					 static bool create_c = false;

					 if ( ImGui::Button( XorStr( "Create config" ), ImVec2( -1.f, 28.f ) ) || create_c ) {
						create_c = true;
						ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
						ImGui::OpenPopup( "Create Config##unload_popup5" );
						if ( ImGui::BeginPopupModal( XorStr( "Create Config##unload_popup5" ), NULL, dwFlag ) ) {
						   ImGui::Text( XorStr( "Are you sure?" ) );
						   if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
							  ConfigManager::CreateConfig( config_name );
							  std::string name( config_name );
							  std::string str = XorStr( "Config " ) + name + XorStr( " created!" );

							  ILoggerEvent::Get( )->PushEvent( str.c_str( ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
							  cfg_list = ConfigManager::GetConfigs( );
							  create_c = false;
							  ImGui::CloseCurrentPopup( );
						   }
						   ImGui::SameLine( );
						   if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
							  ImGui::CloseCurrentPopup( );
							  create_c = false;
						   }

						   ImGui::EndPopup( );
						}
					 };

					 static bool load_c = false;

					 if ( ImGui::Button( XorStr( "Load config" ), ImVec2( -1.f, 28.f ) ) || load_c && cfg_list.size( ) > 0 ) {
						load_c = true;
						ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
						ImGui::OpenPopup( "Load Config##unload_popup4" );
						if ( ImGui::BeginPopupModal( XorStr( "Load Config##unload_popup4" ), NULL, dwFlag ) ) {
						   ImGui::Text( XorStr( "Are you sure?" ) );
						   if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
							  ConfigManager::LoadConfig( cfg_list.at( voted_cfg ) );
							  ILoggerEvent::Get( )->PushEvent( XorStr( "Config loaded!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
							  g_Vars.m_global_skin_changer.m_update_skins = true;
							  g_Vars.m_global_skin_changer.m_update_gloves = true;
							  load_c = false;
							  ImGui::CloseCurrentPopup( );
						   }
						   ImGui::SameLine( );
						   if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
							  ImGui::CloseCurrentPopup( );
							  load_c = false;
						   }

						   ImGui::EndPopup( );
						}
					 }

					 static bool save_c = false;

					 if ( ImGui::Button( XorStr( "Save config" ), ImVec2( -1.f, 28.f ) ) || save_c && cfg_list.size( ) > 0 ) {
						save_c = true;
						ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
						ImGui::OpenPopup( "Save Config##unload_popup3" );
						if ( ImGui::BeginPopupModal( XorStr( "Save Config##unload_popup3" ), NULL, dwFlag ) ) {
						   ImGui::Text( XorStr( "Are you sure?" ) );
						   if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
							  ConfigManager::SaveConfig( cfg_list.at( voted_cfg ) );
							  ILoggerEvent::Get( )->PushEvent( XorStr( "Config saved!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
							  save_c = false;
							  ImGui::CloseCurrentPopup( );
						   }
						   ImGui::SameLine( );
						   if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
							  ImGui::CloseCurrentPopup( );
							  save_c = false;
						   }

						   ImGui::EndPopup( );
						}
					 }

					 static bool delete_c = false;

					 if ( ImGui::Button( XorStr( "Delete config" ), ImVec2( -1.f, 28.f ) ) || delete_c && cfg_list.size( ) > 0 ) {
						delete_c = true;
						ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
						ImGui::OpenPopup( "Delete Config##unload_popup2" );
						if ( ImGui::BeginPopupModal( XorStr( "Delete Config##unload_popup2" ), NULL, dwFlag ) ) {
						   ImGui::Text( XorStr( "Are you sure?" ) );
						   if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
							  ConfigManager::RemoveConfig( cfg_list.at( voted_cfg ) );
							  cfg_list = ConfigManager::GetConfigs( );
							  ILoggerEvent::Get( )->PushEvent( XorStr( "Config deleted!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
							  delete_c = false;
							  ImGui::CloseCurrentPopup( );
						   }
						   ImGui::SameLine( );
						   if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
							  ImGui::CloseCurrentPopup( );
							  delete_c = false;
						   }

						   ImGui::EndPopup( );
						}
					 }

				  } ImGui::EndGroup( );

				  static bool unload = false;

				  if ( ImGui::Button( XorStr( "Unload##123" ), ImVec2( -1.f, 22.f ) ) || unload ) {
					 unload = true;
					 ImGui::OpenPopup( XorStr( "Unload" ) );
					 ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
					 if ( ImGui::BeginPopupModal( XorStr( "Unload" ), NULL, dwFlag ) ) {
						ImGui::Text( XorStr( "Are you sure?" ) );
						if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
						   g_Vars.globals.hackUnload = true;
						   unload = false;
						   ImGui::CloseCurrentPopup( );
						}
						ImGui::SameLine( );
						if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
						   ImGui::CloseCurrentPopup( );
						   unload = false;
						}

						ImGui::EndPopup( );
					 }
				  }

			   } ImGui::EndChild( );
			   static bool copy_ip = false;
			   ImGui::Text( g_Vars.globals.server_adress.c_str( ) );
			   if ( ImGui::IsItemClicked( 0 ) || copy_ip ) {
				  copy_ip = true;
				  ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
				  ImGui::OpenPopup( "Copy ip##unload_popup7" );
				  if ( ImGui::BeginPopupModal( XorStr( "Copy ip##unload_popup7" ) ), NULL, dwFlag ) {
					 ImGui::Text( XorStr( "Are you sure?" ) );
					 if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::ToClipboard( g_Vars.globals.server_adress.c_str( ) );
						ILoggerEvent::Get( )->PushEvent( XorStr( "Ip address copied!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
						copy_ip = false;
						ImGui::CloseCurrentPopup( );
					 }
					 ImGui::SameLine( );
					 if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::CloseCurrentPopup( );
						copy_ip = false;
					 }

					 ImGui::EndPopup( );
					 }
				  }


			   ImGui::PopStyleColor( 1 );
			   break;
			   }
			case 6:
			{
			   ImGui::Columns( 2, NULL, false );
			   ImGui::SetColumnWidth( 0, full_window.x * 0.5f );
			   ImGui::SetColumnWidth( 1, full_window.x );

			   ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, ImVec4( 30 / 255.f, 30 / 255.f, 39 / 255.f, 1.0f ) );

			   ImGui::BeginChild( "Knives", ImVec2( 0.0f, 200.0f ), true );
			   {
				  ImGui::Text( "Knives" );
				  ImGui::Separator( );

				  ImGui::Checkbox( XorStr( "Override knives##knife" ), &g_Vars.m_global_skin_changer.m_knife_changer );

				  if ( k_knife_names.at( g_Vars.m_global_skin_changer.m_knife_vector_idx ).definition_index != g_Vars.m_global_skin_changer.m_knife_idx ) {
					 auto it = std::find_if( k_knife_names.begin( ), k_knife_names.end( ), [&] ( const WeaponName_t& a ) {
						return a.definition_index == g_Vars.m_global_skin_changer.m_knife_idx;
					 } );

					 if ( it != k_knife_names.end( ) )
						g_Vars.m_global_skin_changer.m_knife_vector_idx = std::distance( k_knife_names.begin( ), it );
				  }

				  ImGui::PushItemWidth( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.25f );

				  ImGui::ListBox(
					 XorStr( "##knives" ), &g_Vars.m_global_skin_changer.m_knife_vector_idx,
					 [] ( void* data, int32_t idx, const char** out_text ) {
					 auto vec = reinterpret_cast< std::vector< WeaponName_t >* >( data );
					 *out_text = vec->at( idx ).name;
					 return true;
				  },
					 ( void* ) ( &k_knife_names ), k_knife_names.size( ), 7 );

				  ImGui::PopItemWidth( );

				  g_Vars.m_global_skin_changer.m_knife_idx = k_knife_names[ g_Vars.m_global_skin_changer.m_knife_vector_idx ].definition_index;

			   } ImGui::EndChild( );

			   ImGui::BeginChild( "Gloves", ImVec2( 0.0f, 265.0f ), true );
			   {
				  ImGui::Text( "Gloves" );
				  ImGui::Separator( );
				  ImGui::Checkbox( XorStr( "Override gloves##glove" ), &g_Vars.m_global_skin_changer.m_glove_changer );

				  if ( k_glove_names.at( g_Vars.m_global_skin_changer.m_gloves_vector_idx ).definition_index != g_Vars.m_global_skin_changer.m_gloves_idx ) {
					 auto it = std::find_if( k_glove_names.begin( ), k_glove_names.end( ), [&] ( const WeaponName_t& a ) {
						return a.definition_index == g_Vars.m_global_skin_changer.m_gloves_idx;
					 } );

					 if ( it != k_glove_names.end( ) )
						g_Vars.m_global_skin_changer.m_gloves_vector_idx = std::distance( k_glove_names.begin( ), it );
				  }

				  ImGui::PushItemWidth( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.25f );

				  if ( ImGui::ListBox(
					 XorStr( "##gloves" ), &g_Vars.m_global_skin_changer.m_gloves_vector_idx,
					 [] ( void* data, int32_t idx, const char** out_text ) {
					 auto vec = reinterpret_cast< std::vector< WeaponName_t >* >( data );
					 *out_text = vec->at( idx ).name;
					 return true;
				  },
					 ( void* ) ( &k_glove_names ), k_glove_names.size( ), 11 ) ) {
					 g_Vars.m_global_skin_changer.m_update_skins = true;
					 g_Vars.m_global_skin_changer.m_update_gloves = true;
				  }

				  ImGui::PopItemWidth( );

				  g_Vars.m_global_skin_changer.m_gloves_idx = k_glove_names[ g_Vars.m_global_skin_changer.m_gloves_vector_idx ].definition_index;

			   } ImGui::EndChild( );

			#ifdef BETA_MODE
			   ImGui::Text( XorStr( "eexomi.host [BETA] | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#else 
			   ImGui::Text( XorStr( "eexomi.host v2 | %i:%i:%i | %s" ), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, g_Vars.globals.c_login.c_str( ) );
			#endif

			   ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );
			   ImGui::NextColumn( );
			   ImGui::PopStyleVar( );

			   ImGui::BeginChild( "Skin", ImVec2( 0.0f, 470.0f ), true );
			   {
				  ImGui::Text( "Skin" );
				  ImGui::Separator( );
				  ImGui::Checkbox( XorStr( "Enabled##Skins" ), &g_Vars.m_global_skin_changer.m_active );

				  static int weapon_id = 0;

				  ImGui::PushItemWidth( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.25f );

				  if ( ImGui::Combo(
					 XorStr( "##skins_weapons" ), &weapon_id,
					 [] ( void* data, int32_t idx, const char** out_text ) {
					 auto vec = reinterpret_cast< std::vector< item_skins >* >( data );
					 *out_text = vec->at( idx ).display_name.c_str( );
					 return true;
				  }, ( void* ) ( &weapon_skins ), weapon_skins.size( ), 14 ) ) {

				  }

				  ImGui::PopItemWidth( );

				  auto& current_weapon = weapon_skins[ weapon_id ];
				  auto idx = current_weapon.id;

				  auto& skin_data = g_Vars.m_skin_changer;
				  CVariables::skin_changer_data* skin = nullptr;
				  for ( size_t i = 0u; i < skin_data.Size( ); ++i ) {
					 skin = skin_data[ i ];
					 if ( skin->m_definition_index == idx ) {
						break;
					 }
				  }

				  auto& kit = current_weapon.m_kits[ skin->m_paint_kit_index ];
				  if ( kit.id != skin->m_paint_kit ) {
					 auto it = std::find_if( current_weapon.m_kits.begin( ), current_weapon.m_kits.end( ), [skin] ( paint_kit& a ) {
						return a.id == skin->m_paint_kit;
					 } );

					 if ( it != current_weapon.m_kits.end( ) )
						skin->m_paint_kit_index = std::distance( current_weapon.m_kits.begin( ), it );
				  }

				  ImGui::PushItemWidth( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 0.25f );

				  if ( ImGui::ListBox(
					 XorStr( "##paint_kits" ), &skin->m_paint_kit_index,
					 [] ( void* data, int32_t idx, const char** out_text ) {
					 auto vec = reinterpret_cast< std::vector< paint_kit >* >( data );
					 *out_text = vec->at( idx ).name.c_str( );
					 return true;
				  },
					 ( void* ) ( &current_weapon.m_kits ), current_weapon.m_kits.size( ), 16 ) ) {
					 g_Vars.m_global_skin_changer.m_update_skins = true;
					 if ( current_weapon.glove )
						g_Vars.m_global_skin_changer.m_update_gloves = true;
				  }

				  ImGui::PopItemWidth( );

				  skin->m_paint_kit = current_weapon.m_kits[ skin->m_paint_kit_index ].id;

				  skin->m_enabled = true;

				  ImGui::SliderFloatA( XorStr( "Quality" ), &skin->m_wear, 0.00000001f, 1.00f, XorStr( "%.8f %%" ) );
				  //ImGui::SliderIntA( XorStr( "Seed" ), &skin->m_seed, 1, 1000, XorStr( "%d" ) );
				  //ImGui::SliderIntA( XorStr( "Stat-trak" ), &skin->m_stat_trak, 1, 1000000, XorStr( "%d" ) );

				  if ( ImGui::Button( "Force update", ImVec2( -1.f, 22.f ) ) ) {
					  g_Vars.m_global_skin_changer.m_update_skins = true;
					  g_Vars.m_global_skin_changer.m_update_gloves = true;
				  }
				  static char buffer[ 128 ] = { ( '\0' ) };

				  //ImGui::InputText( XorStr( "Name" ), buffer, 128 );

			   } ImGui::EndChild( );

			   static bool copy_ip = false;
			   ImGui::Text( g_Vars.globals.server_adress.c_str( ) );
			   if ( ImGui::IsItemClicked( 0 ) || copy_ip ) {
				  copy_ip = true;
				  ImGui::SetNextWindowSize( ImVec2( 150.0f, 0.0f ) );
				  ImGui::OpenPopup( "Copy ip##unload_popup7" );
				  if ( ImGui::BeginPopupModal( XorStr( "Copy ip##unload_popup7" ) ), NULL, dwFlag ) {
					 ImGui::Text( XorStr( "Are you sure?" ) );
					 if ( ImGui::Button( XorStr( "Yes" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::ToClipboard( g_Vars.globals.server_adress.c_str( ) );
						ILoggerEvent::Get( )->PushEvent( XorStr( "Ip address copied!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
						copy_ip = false;
						ImGui::CloseCurrentPopup( );
					 }
					 ImGui::SameLine( );
					 if ( ImGui::Button( XorStr( "No" ), ImVec2( 65.f, 25.f ) ) ) {
						ImGui::CloseCurrentPopup( );
						copy_ip = false;
					 }

					 ImGui::EndPopup( );
				  }
			   }

			   ImGui::PopStyleColor( 1 );
			   break;
			}
			}
	  }

	  ImGui::PopFont( );

	  ImGui::End( );
   }
}

void CMenuV2::WndProcHandler( ) {
   if ( InputSys::Get( )->WasKeyPressed( VK_INSERT ) ) {
	  g_Vars.globals.menuOpen = !g_Vars.globals.menuOpen;
   }

   if ( g_Vars.antiaim.manual_left_bind.enabled ) {
	  g_Vars.globals.manual_aa = 0;
	  g_Vars.globals.MouseOverrideEnabled = false;
   }

   if ( g_Vars.antiaim.manual_right_bind.enabled ) {
	  g_Vars.globals.manual_aa = 2;
	  g_Vars.globals.MouseOverrideEnabled = false;
   }

   if ( g_Vars.antiaim.manual_back_bind.enabled ) {
	  g_Vars.globals.manual_aa = 1;
	  g_Vars.globals.MouseOverrideEnabled = false;
   }
}

void CMenuV2::Initialize( IDirect3DDevice9* pDevice ) {
   //ImGui_ImplWin32_Init( g_Vars.globals. );
   //ImGui_ImplDX9_Init( device );

   //ImGui::GetIO( ).RenderDrawListsFn = ImGui_ImplDX9_RenderDrawData;

   ImGuiStyle& style = ImGui::GetStyle( );
   style.Alpha = 1.0f;
   style.WindowPadding = ImVec2( 8, 10 );
   style.WindowMinSize = ImVec2( 32, 32 );
   style.WindowRounding = 0.0f;
   style.WindowTitleAlign = ImVec2( 0.0f, 0.5f );
   style.FramePadding = ImVec2( 4, 2 );
   style.FrameRounding = 3.0f;
   style.ItemSpacing = ImVec2( 8, 5 );
   style.ItemInnerSpacing = ImVec2( 8, 8 );
   style.TouchExtraPadding = ImVec2( 0, 0 );
   style.IndentSpacing = 20.0f;
   style.ColumnsMinSpacing = 0.0f;
   style.ScrollbarSize = 6.0f;
   style.ScrollbarRounding = 0.0f;
   style.GrabMinSize = 5.0f;
   style.GrabRounding = 3.0f;
   style.ButtonTextAlign = ImVec2( 0.5f, 0.5f );
   style.DisplayWindowPadding = ImVec2( 22, 22 );
   style.DisplaySafeAreaPadding = ImVec2( 4, 4 );
   style.AntiAliasedLines = true;
   style.CurveTessellationTol = 1.f;
   style.FrameBorderSize = 1.0f;
   style.ChildBorderSize = 1.0f;

   ImVec4* colors = ImGui::GetStyle( ).Colors;
   colors[ ImGuiCol_Text ] = ImVec4( 0.95f, 0.96f, 0.98f, 1.00f );
   colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.36f, 0.42f, 0.47f, 1.00f );
   colors[ ImGuiCol_WindowBg ] = ImVec4( 0.11f, 0.15f, 0.17f, 1.00f );
   colors[ ImGuiCol_ChildBg ] = ImVec4( 0.15f, 0.18f, 0.22f, 1.00f );
   colors[ ImGuiCol_PopupBg ] = ImVec4( 0.08f, 0.08f, 0.08f, 0.94f );
   colors[ ImGuiCol_Border ] = ImVec4( 0.08f, 0.10f, 0.12f, 1.00f );
   colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
   colors[ ImGuiCol_FrameBg ] = ImVec4( 0.20f, 0.25f, 0.29f, 1.00f );
   colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.12f, 0.20f, 0.28f, 1.00f );
   colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.09f, 0.12f, 0.14f, 1.00f );
   colors[ ImGuiCol_TitleBg ] = ImVec4( 0.09f, 0.12f, 0.14f, 0.65f );
   colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.08f, 0.10f, 0.12f, 1.00f );
   colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
   colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.15f, 0.18f, 0.22f, 1.00f );
   colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.02f, 0.02f, 0.02f, 0.39f );
   colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.20f, 0.25f, 0.29f, 1.00f );
   colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.18f, 0.22f, 0.25f, 1.00f );
   colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.09f, 0.21f, 0.31f, 1.00f );
   colors[ ImGuiCol_CheckMark ] = ImVec4( 0.23f, 0.82f, 0.86f, 1.00f );
   colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.23f, 0.82f, 0.86f, 1.00f );
   colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.23f, 0.82f, 0.86f, 1.00f );
   colors[ ImGuiCol_Button ] = ImVec4( 0.20f, 0.25f, 0.29f, 1.00f );
   colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.25f, 0.31f, 0.36f, 1.00f );
   colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.15f, 0.18f, 0.21f, 1.00f );
   colors[ ImGuiCol_Header ] = ImVec4( 0.20f, 0.25f, 0.29f, 1.00f );
   colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.25f, 0.31f, 0.36f, 1.00f );
   colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.15f, 0.18f, 0.21f, 1.00f );
   colors[ ImGuiCol_Separator ] = ImVec4( 0.20f, 0.25f, 0.29f, 1.00f );
   colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.25f, 0.31f, 0.36f, 1.00f );
   colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.15f, 0.18f, 0.21f, 1.00f );
   colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.23f, 0.82f, 0.86f, 0.25f );
   colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.23f, 0.82f, 0.86f, 0.67f );
   colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.23f, 0.82f, 0.86f, 0.95f );
   colors[ ImGuiCol_Tab ] = ImVec4( 0.11f, 0.15f, 0.17f, 1.00f );
   colors[ ImGuiCol_TabHovered ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
   colors[ ImGuiCol_TabActive ] = ImVec4( 0.20f, 0.25f, 0.29f, 1.00f );
   colors[ ImGuiCol_TabUnfocused ] = ImVec4( 0.11f, 0.15f, 0.17f, 1.00f );
   colors[ ImGuiCol_TabUnfocusedActive ] = ImVec4( 0.11f, 0.15f, 0.17f, 1.00f );
   colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
   colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
   colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
   colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
   colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
   colors[ ImGuiCol_DragDropTarget ] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
   colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
   colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
   colors[ ImGuiCol_NavWindowingDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
   colors[ ImGuiCol_ModalWindowDimBg ] = ImVec4( 0.10f, 0.10f, 0.10f, 0.35f );
}

#endif