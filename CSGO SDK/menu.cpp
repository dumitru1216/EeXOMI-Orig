#if 0
#define IMGUI_DEFINE_MATH_OPERATORS
#include "Menu.hpp"
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
#include "Render.hpp"


class CMenu : public IMenu {
public:
   void StatFrameRender( ) override;
   void IndicatorsFrame( ) override;
   void Main( IDirect3DDevice9* pDevice ) override;
   void WndProcHandler( ) override;
   void Initialize( IDirect3DDevice9* pDevice ) override;
private:
   ImFont* hack_font;
   ImFont* hack_font_bold;
#if 0
   ImFont * font_main;
   ImFont* font_menu;
   ImFont* font_tabs;
   ImFont* font_main_caps;
#endif

   float m_alpha = 0.0f;

   IDirect3DTexture9* menuBg = nullptr;

   std::vector<std::string> cfg_list;
   char config_name[ 256 ] = { "..." };
   int voted_cfg = 0;
};

#define REMOVE_WINDOW_PADDING() ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f))
#define RESTORE_WINDOW_PADDING() ImGui::PopStyleVar()

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
		 * out_text = items[ idx ];
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
		 * out_text = p;
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

		 float height = std::fminf( 40.0f, ImGui::GetContentRegionAvail( ).y - 2.0f );

		 // Draw the button
		 ImGui::PushID( i );   // otherwise two tabs with the same name would clash.
		 {
			auto size_arg = ImVec2( custom_width, height );
			const ImVec2 label_size = CalcTextSize( tabLabels[ i ], NULL, true );
			const ImGuiID id = window->GetID( tabLabels[ i ] );
			ImVec2 size = CalcItemSize( size_arg, 0.0f, 0.0f );
			const ImRect bb( window->DC.CursorPos, window->DC.CursorPos + size );
			ItemSize( size );
			if ( !ItemAdd( bb, id ) )
			   return false;

			bool hovered, held;
			bool pressed = ButtonBehavior( bb, id, &hovered, &held );

			auto col = Spectrum::RED700;

			if ( pressed ) {
			   selection_changed = ( tabIndex != i ); newtabIndex = i;
			}

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

   bool KeyBox( KeyBind_t* bind, int defaultCondition = -1, const ImVec2 & size = { 0.f, 0.f } ) {
	  static bool key_input = false;
	  static bool cond_input = false;
	  static KeyBind_t* key_output = nullptr;

	  std::string text = XorStr( "none" );

	  if ( !cond_input && key_input && key_output == bind ) {
		 text = XorStr( "[...]" );

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
		 text = ( '[' ) + name + ( ']' );
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
		 pos.x -= size.x;

	  #if 0
		 ImRect bb( text_pos, text_pos + text_size );
		 ItemSize( ImRect( text_pos, text_pos + ImVec2( text_size.y + g.Style.FramePadding.y * 1.f, text_size.y + g.Style.FramePadding.y * 1.f ) ), g.Style.FramePadding.y );
	  #endif

		 const ImRect bb( pos, pos + size );
		 ItemSize( size, style.FramePadding.y );
		 if ( ItemAdd( bb, id ) ) {
			// Render cond_input
			RenderNavHighlight( bb, id );

		 #if 0
			const ImU32 col = GetColorU32( ( held && hovered ) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button );
			RenderFrame( bb.Min, bb.Max, col, true, style.FrameRounding );
		 #endif

			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 92.0f / 255.0f, 92.0f / 255.0f, 92.0f / 255.0f, 1.0f ) );
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
			#if 0
			   bool hovered, held;
			   bool pressed = ButtonBehavior( bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnDoubleClick );
			#endif

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

   bool KeyBox( const char* label, KeyBind_t* bind, int defaultCondition = -1, const ImVec2 & size = { 0.f, 0.f } ) {
	  KeyBox( bind, defaultCondition, size );
	  return false;
   }

   // Getter for the old Combo() API: const char*[]
   static bool items_getter( void* data, int idx, const char** out_text ) {
	  const char* const* items = ( const char* const* ) data;
	  if ( out_text )
		 * out_text = items[ idx ];
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
	  char preview_value[ 128 ] = { XorStr( '\0' ) };
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
	  if ( popup_max_height_in_items != -1 /*&& !( g.NextWindowData.SizeConstraintCond )*/ )
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

#if 0
auto GetServerEdictKek( int index ) -> std::uint8_t * {
   static uintptr_t pServerGlobals = **( uintptr_t * * ) ( Engine::Displacement.Data.m_uServerGlobals );
   int iMaxClients = *( int* ) ( ( uintptr_t ) pServerGlobals + 0x18 );
   if ( iMaxClients >= index ) {
	  if ( index <= iMaxClients ) {
		 int v10 = index * 16;
		 uintptr_t v11 = *( uintptr_t* ) ( pServerGlobals + 96 );
		 if ( v11 ) {
			if ( !( ( *( uintptr_t* ) ( v11 + v10 ) >> 1 ) & 1 ) ) {
			   uintptr_t v12 = *( uintptr_t* ) ( v10 + v11 + 12 );
			   if ( v12 ) {
				  uint8_t* pReturn = nullptr;

				  // abusing asm is not good
				  __asm
				  {
					 pushad
					 mov ecx, v12
					 mov eax, dword ptr[ ecx ]
					 call dword ptr[ eax + 0x14 ]
					 mov pReturn, eax
					 popad
				  }

				  return pReturn;
			   }
			}
		 }
	  }
   }
   return nullptr;
}
#endif

void CMenu::StatFrameRender( ) {
#if 0
   static int size_y = 0;

   auto TranslateReason = [] ( int index ) {
	  switch ( index ) {
		 case 0:
		 return ( XorStr( "Desync" ) ); break;
		 case 1:
		 return ( XorStr( "Spread" ) ); break;
		 case 2:
		 return ( XorStr( "Hit" ) ); break;
		 case 3:
		 return ( XorStr( "LC" ) ); break;
		 case 4:
		 return ( XorStr( "Pred D" ) ); break;
		 case 5:
		 return ( XorStr( "Pred S" ) ); break;
		 case 6:
		 return ( XorStr( "Server" ) ); break;
		 default:
		 return XorStr( "Unknown" );
		 break;
	  }
   };

   auto TranslateSide = [] ( int index ) {
	  if ( !index )
		 return ( XorStr( "Fake" ) );
	  return index > 0 ? XorStr( "Left" ) : XorStr( "Right" );
   };

   auto FixedStrLenght = [] ( std::string str ) -> std::string {
	  std::string result;
	  for ( size_t i = 0; i < std::min( 12u, str.size( ) ); i++ ) {
		 result.push_back( str.at( i ) );
	  }
	  return result;
   };

   ImGuiStyle* style = &ImGui::GetStyle( );

   style->WindowPadding = ImVec2( 6, 6 );

   ImGui::TextSpacingMeme = true;
   ImGui::SetNextWindowSize( ImVec2( 326, size_y == 0.f ? 72.f : size_y + 12.f ), ImGuiCond_FirstUseEver );
   if ( ImGui::BeginMenuBackground( XorStr( "Hit logger" ), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar ) ) {
	  style->ItemSpacing = ImVec2( 4, 2 );
	  style->WindowPadding = ImVec2( 4, 4 );

	  ImGui::SetWindowSize( ImVec2( 326, size_y == 0.f ? 72.f : size_y + 12.f ) );
	  ImGui::SameLine( 6.f );

	  ImGui::BeginChild( XorStr( "Menu Contents" ), ImVec2( 314, size_y == 0.f ? 65.f : size_y ), false );
	  {
		 ImGui::ColorBar( XorStr( "unicorn" ), ImVec2( 326.f, 2.f ) );

		 size_y = ( IRoundFireBulletsStore::Get( )->GetVecSize( ) * 15.f ) + 18.f;
		 ImGui::SetWindowSize( ImVec2( 314, size_y == 0.f ? 65.f : size_y ) );
		 ImGui::SameLine( 5.f );
		 ImGui::Text( XorStr( "id" ) );
		 ImGui::SameLine( 30.f );
		 ImGui::Text( XorStr( "name" ) );
		 ImGui::SameLine( 120.f );
		 ImGui::Text( XorStr( "damage" ) );
		 ImGui::SameLine( 170.f );
		 ImGui::Text( XorStr( "ticks" ) );
		 ImGui::SameLine( 200.f );
		 ImGui::Text( XorStr( "reason" ) );
		 ImGui::SameLine( 250.f );
		 ImGui::Text( XorStr( "side" ) );
		 ImGui::SameLine( 285.f );
		 ImGui::Text( XorStr( "type" ) );
		 ImGui::Spacing( );

		 for ( int i = 0; i < IRoundFireBulletsStore::Get( )->GetVecSize( ); i++ ) {
			auto element = IRoundFireBulletsStore::Get( )->GetStats( i );

			ImGui::NewLine( );
			ImGui::SameLine( 5.f );
			ImGui::Text( std::to_string( element->m_iClientIndex ).c_str( ) );
			ImGui::SameLine( 30.f );
			ImGui::Text( FixedStrLenght( element->m_name ).c_str( ) );
			ImGui::SameLine( 120.f );
			ImGui::Text( element->m_iDamage == 0 ? XorStr( "-" ) : std::to_string( element->m_iDamage ).c_str( ) );
			ImGui::SameLine( 180.f );
			ImGui::Text( element->m_iLagCompTicks == 0 ? XorStr( "-" ) : std::to_string( element->m_iLagCompTicks ).c_str( ) );
			ImGui::SameLine( 200.f );
			ImGui::Text( TranslateReason( element->m_iReason ) );
			ImGui::SameLine( 250.f );
			ImGui::Text( TranslateSide( element->m_iResolverSide ) );
			ImGui::SameLine( 285.f );
			ImGui::Text( std::to_string( element->m_iResolverType ).c_str( ) );
		 }
	  }
	  ImGui::EndChild( );

	  style->ItemSpacing = ImVec2( 0, 0 );
	  style->WindowPadding = ImVec2( 6, 6 );

	  ImGui::End( );
   }
   ImGui::TextSpacingMeme = false;
#endif

#if 0
   struct __declspec( align( 4 ) ) CServerCSGOPlayerAnimState {
	  char pad[ 3 ];
	  char bUnknown;
	  char pad2[ 75 ];
	  void* m_player;
	  void* m_active_weapon;
	  void* m_last_active_weapon;
	  float m_flLastUpdateTime;
	  int m_iLastFrame;
	  float m_flFrametime;
	  float m_flEyeYaw;
	  float m_flPitch;
	  float m_flGoalFeetYaw;
	  float m_flOldFeetYaw;
	  float m_flCurrentTorsoYaw;
	  float m_flUnknownVelocityLean;
	  float m_flLeanAmount;
	  float m_flUnknown1;
	  float m_flFeetCycle;
	  float m_flFeetYawRate;
	  float m_fUnknown2;
	  float m_fDuckAmount;
	  float m_fLandingDuckAdditiveSomething;
	  float m_fUnknown3;
	  Vector m_vOrigin;
	  Vector m_vLastOrigin;
	  Vector m_vecVelocity;
	  Vector m_vecNormalizedVelocity;
	  Vector m_vecNormalizedMovingVelocity;
	  float m_velocity;
	  float flUpVelocity;
	  float m_flSpeedNormalized;
	  float m_flWalkingSpeed;
	  float m_flPerfectAccuracySpeed;
	  float m_flTimeSinceStartedMoving;
	  float m_flTimeSinceStoppedMoving;
	  unsigned __int8 m_bOnGround;
	  unsigned __int8 m_987Act;
	  float m_flLowerBodyUpdateTime;
	  char m_bHitground;
	  char pad7[ 6 ];
	  float m_flAirTime;
	  float m_flHeadHeightOrOffsetFromHittingGroundAnimation;
	  float m_flStopToFullRunningFraction;
	  float m_flGroundFriction;
	  char m_bLanding;
	  char m_bJumped;
	  char m_flagsChanged[ 2 ];
	  float m_flFeetlWeight;
	  char m_bOnLadder;
	  char pad10[ 11 ];
	  bool is_walking;
	  char pad10_[ 3 ];
	  char m_bIdleAdjust;
	  char pad11[ 28 ];
	  Vector m_vecLeanVelocity;
	  char pad12[ 74 ];
	  animstate_pose_param_cache_t lean_yaw;
	  animstate_pose_param_cache_t speed;
	  animstate_pose_param_cache_t ladder_speed;
	  animstate_pose_param_cache_t ladder_yaw;
	  animstate_pose_param_cache_t move_yaw;
	  animstate_pose_param_cache_t run;
	  animstate_pose_param_cache_t body_yaw;
	  animstate_pose_param_cache_t body_pitch;
	  animstate_pose_param_cache_t death_yaw;
	  animstate_pose_param_cache_t stand;
	  animstate_pose_param_cache_t jump_fall;
	  animstate_pose_param_cache_t aim_blend_stand_idle;
	  animstate_pose_param_cache_t aim_blend_crouch_idle;
	  animstate_pose_param_cache_t strafe_yaw;
	  animstate_pose_param_cache_t aim_blend_stand_walk;
	  animstate_pose_param_cache_t aim_blend_stand_run;
	  animstate_pose_param_cache_t aim_blend_crouch_walk;
	  animstate_pose_param_cache_t move_blend_walk;
	  animstate_pose_param_cache_t move_blend_run;
	  animstate_pose_param_cache_t move_blend_crouch;
	  char pad__111[ 8 ];
	  float m_flunkownspeed;
	  float m_flSomeSpeed;
	  float m_flSomeSpeed2;
	  char pad__112[ 4 ];
	  float max_body_yaw_lower;
	  float max_body_yaw_upper;
	  float max_body_pitch_lower;
	  float max_body_pitch_upper;
	  int anim_state_version;
   };

   static uintptr_t animStateOffset = *( uintptr_t* ) ( Engine::Displacement.Data.m_uServerAnimState );

   C_CSPlayer* player = nullptr;
   for ( int i = 1; i <= 64; ++i ) {
	  auto _player = C_CSPlayer::GetPlayerByIndex( i );
	  if ( !_player || _player->IsDormant( ) || _player->IsDead( ) || _player == C_CSPlayer::GetLocalPlayer( ) )
		 continue;

	  player = _player;
	  break;
   }

   if ( !player )
	  return;

   auto ent = GetServerEdictKek( player->m_entIndex );
   if ( !ent )
	  return;

   auto animState = player->m_PlayerAnimState( );

   ImGui::SetNextWindowSize( ImVec2( 450, 420 ) );
   if ( ImGui::BeginMenuBackground( XorStr( "Debug Window Poses" ), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar ) ) {
	  static uintptr_t poseParamOffset = *( uintptr_t* ) ( Engine::Displacement.Data.m_uServerPoseParameters );

	  auto server_poses = ( float* ) ( uintptr_t( ent ) + poseParamOffset );
	  auto client_poses = player->m_flPoseParameter( );

	  ImGui::Text( XorStr( "name client - server; delta" ) );
	  for ( auto i = &animState->lean_yaw; i != ( animstate_pose_param_cache_t* ) & animState->unk_speed_01; ++i ) {
		 if ( !i->name || !i->valid )
			continue;

		 ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), i->name, server_poses[ i->index ], client_poses[ i->index ], server_poses[ i->index ] - client_poses[ i->index ] );
	  }
	  ImGui::End( );
   }

   if ( !animStateOffset )
	  return;

   auto serverAnimState = *( CServerCSGOPlayerAnimState * * ) ( uintptr_t( ent ) + animStateOffset );
   if ( !serverAnimState )
	  return;

   auto weapon = ( C_WeaponCSBaseGun* ) ( player->m_hActiveWeapon( ).Get( ) );

   auto velocity = *( Vector* ) ( uintptr_t( ent ) + 0x1F4 );
   auto velocity_delta = player->m_vecVelocity( ) - velocity;

   ImGui::SetNextWindowSize( ImVec2( 450, 500 ) );
   if ( ImGui::BeginMenuBackground( XorStr( "Debug Window Animation State" ), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar ) ) {
	  ImGui::Text( XorStr( "name client - server; delta" ) );

	  ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), XorStr( "Abs rotation" ), animState->m_flAbsRotation, serverAnimState->m_flGoalFeetYaw,
				   std::remainderf( animState->m_flAbsRotation - serverAnimState->m_flGoalFeetYaw, 360.0f ) );

	  ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), XorStr( "Move yaw" ), animState->m_flCurrentTorsoYaw, serverAnimState->m_flCurrentTorsoYaw,
				   std::remainderf( animState->m_flCurrentTorsoYaw - serverAnimState->m_flCurrentTorsoYaw, 360.0f ) );

	  ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), XorStr( "Feet cycle" ), animState->m_flFeetCycle, serverAnimState->m_flFeetCycle,
				   animState->m_flFeetCycle - serverAnimState->m_flFeetCycle );

	  ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), XorStr( "Feet yaw rate" ), animState->m_flFeetYawRate, serverAnimState->m_flFeetYawRate,
				   animState->m_flFeetYawRate - serverAnimState->m_flFeetYawRate );

	  ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), XorStr( "Lean Amount" ), animState->m_flLeanAmount, serverAnimState->m_flLeanAmount,
				   animState->m_flLeanAmount - serverAnimState->m_flLeanAmount );

	  ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), XorStr( "Unknown1" ), animState->m_flUnknown1, serverAnimState->m_flUnknown1,
				   animState->m_flUnknown1 - serverAnimState->m_flUnknown1 );

	  ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), XorStr( "Unknown2" ), animState->m_fUnknown2, serverAnimState->m_fUnknown2,
				   animState->m_fUnknown2 - serverAnimState->m_fUnknown2 );

	  ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), XorStr( "Unknown5" ), animState->m_fUnknown5, *( float* ) ( uintptr_t( serverAnimState ) + 0x124 ),
				   animState->m_fUnknown5 - *( float* ) ( uintptr_t( serverAnimState ) + 0x124 ) );

	  ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), XorStr( "Weight Lean" ), *( float* ) ( uintptr_t( animState ) + 0x180 ), *( float* ) ( uintptr_t( serverAnimState ) + 0x178 ),
				   *( float* ) ( uintptr_t( animState ) + 0x180 ) - *( float* ) ( uintptr_t( serverAnimState ) + 0x178 ) );

	  ImGui::Text( XorStr( "Physical velocity" ) );
	  ImGui::Text( XorStr( "client: X %.3f Y %.3f Z %.3f" ), player->m_vecVelocity( ).x, player->m_vecVelocity( ).y, player->m_vecVelocity( ).z );
	  ImGui::Text( XorStr( "server: X %.3f Y %.3f Z %.3f" ), velocity.x, velocity.y, velocity.z );
	  ImGui::Text( XorStr( "clients speed: %.1f | server speed: %.1f | %.1f" ), player->m_vecVelocity( ).Length( ), velocity.Length( ), player->m_vecVelocity( ).Length( ) - velocity.Length( ) );
	  ImGui::Text( XorStr( "delta: X %.3f Y %.3f Z %.3f" ), velocity_delta.x, velocity_delta.y, velocity_delta.z );
	  ImGui::Text( XorStr( "delta len: %.5f" ), velocity_delta.Length( ) );

	  ImGui::Text( XorStr( "%s \t %.5f - %.5f;\tdelta %.5f" ), XorStr( "2D Speed" ), animState->m_velocity, serverAnimState->m_velocity,
				   animState->m_velocity - serverAnimState->m_velocity );

	  if ( weapon ) {
		 auto weapon_data = weapon->GetCSWeaponData( );
		 if ( weapon_data.IsValid( ) && serverAnimState->m_velocity > 1.0f && serverAnimState->m_velocity <= weapon_data->m_flMaxSpeed * 0.52f ) {
			float speed = player->m_AnimOverlay( ).Element( 6 ).m_flWeight * ( weapon_data->m_flMaxSpeed * 0.52f );
			ImGui::Text( XorStr( "estimated speed: %.5f | real speed: %.5f | delta: %.5f" ), speed, serverAnimState->m_velocity,
						 speed - serverAnimState->m_velocity );
		 }
	  }

	  ImGui::End( );
   }

   ImGui::SetNextWindowSize( ImVec2( 750, 500 ) );
   if ( ImGui::BeginMenuBackground( XorStr( "Debug Window Animoverlays" ), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar ) ) {
	  for ( int i = 0; i < player->m_AnimOverlay( ).Count( ); ++i ) {
		 auto layer = &player->m_AnimOverlay( ).Element( i );
		 ImGui::Text( XorStr( "order: %d | weight: %.5f | cycle: %.5f | playback: %.5f | deltarate: %.5f | seq: %d" ), i, layer->m_flWeight, layer->m_flCycle,
					  layer->m_flPlaybackRate, layer->m_flWeightDeltaRate,
					  layer->m_nSequence );
	  }
	  ImGui::Text( XorStr( "on ground: %s" ), ( player->m_fFlags( ) & FL_ONGROUND ) != 0 ? XorStr( "true" ) : XorStr( "false" ) );

	  ImGui::End( );
   }

#endif
}

void AddBind( const char* name, const KeyBind_t& bind ) {
   if ( bind.cond == KeyBindType::ALWAYS_ON )
	  return;

   static const auto hold_size = ImGui::CalcTextSize( XorStr( "[Hold]" ) );
   static const auto toggle_size = ImGui::CalcTextSize( XorStr( "[Toggle]" ) );

   // ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) 

   if ( bind.enabled ) {
	  ImGui::NewLine( );
	  ImGui::SameLine( );
	  ImGui::Text( name );
	  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ( bind.cond == KeyBindType::HOLD ? hold_size.x : toggle_size.x ) - 5.0f );
	  ImGui::Text( bind.cond == KeyBindType::HOLD ? XorStr( "Hold" ) : XorStr( "Toggle" ) );
   }
}

void CMenu::IndicatorsFrame( ) {
#if 0
   static int size_y = 0;
   ImGuiStyle* style = &ImGui::GetStyle( );

   style->WindowPadding = ImVec2( 6, 6 );

   ImGui::PushFont( menuFont );

   if ( g_Vars.esp.spectator_list ) {
	  std::vector<std::string> vecNames;

	  int iObservers = 0;

	  for ( int i = 1; i <= 64; i++ ) {
		 C_CSPlayer* pEnt = C_CSPlayer::GetPlayerByIndex( i );
		 C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );
		 if ( pEnt && !pEnt->IsDormant( ) && pLocal && !pLocal->IsDead( ) && pEnt != pLocal ) {
			auto pObserver = pEnt->m_hObserverTarget( ).Get( );
			if ( pObserver && pObserver == pLocal ) {
			   player_info_t info;
			   if ( Source::m_pEngine->GetPlayerInfo( i, &info ) ) {
				  vecNames.push_back( info.szName );
				  iObservers++;
				  size_y = 18 * iObservers;
			   }
			}
		 }
	  }

	  if ( vecNames.size( ) > 0 ) {
		 ImGui::SetNextWindowSize( ImVec2( 200, size_y + 20 ) );
		 if ( ImGui::BeginMenuBackground( XorStr( "Spectators" ), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar ) ) {

			ImGui::BeginChild( XorStr( "Menu Contents" ), ImVec2( 188, size_y == 0 ? 17 : size_y + 7 ), false );
			{
			   for ( const auto& name : vecNames ) {
				  ImGui::Text( name.c_str( ) );
			   }
			} ImGui::EndChild( );

			ImGui::End( );
		 }
	  }
   }

   if ( g_Vars.esp.keybind_list ) {
	  ImGui::SetNextWindowSize( ImVec2( 200, 250 ) );
	  if ( ImGui::BeginMenuBackground( XorStr( "KeyBinds" ), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar ) ) {
		 ImGui::BeginChild( XorStr( "Menu Contents" ), ImVec2( 188, 240 ), false );
		 {
			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );

			if ( g_Vars.rage.enabled ) {
			   if ( g_Vars.rage.exploit ) {
				  if ( g_Vars.rage.exploit_type == 0 ) {
					 AddBind( XorStr( "Double tap" ), g_Vars.rage.double_tap_bind );
					 AddBind( XorStr( "Hide shots" ), g_Vars.rage.hide_shots_bind );
				  } else {
					 AddBind( XorStr( "Rapid charge" ), g_Vars.rage.rapid_charge );
					 AddBind( XorStr( "Rapid release" ), g_Vars.rage.rapid_release );
				  }
			   }

			   AddBind( XorStr( "Damage override" ), g_Vars.rage.key_dmg_override );
			   AddBind( XorStr( "Hitscan override" ), g_Vars.rage.override_key );
			   AddBind( XorStr( "Safe point" ), g_Vars.rage.optimal_point_key );
			}

			if ( g_Vars.esp.zoom )
			   AddBind( XorStr( "Zoom" ), g_Vars.esp.zoom_key );

			if ( g_Vars.misc.active ) {
			   if ( g_Vars.misc.edgejump )
				  AddBind( XorStr( "Edge jump" ), g_Vars.misc.edgejump_bind );
			   if ( g_Vars.misc.fakeduck )
				  AddBind( XorStr( "Fakeduck" ), g_Vars.misc.fakeduck_bind );
			   if ( g_Vars.misc.slow_walk )
				  AddBind( XorStr( "Slow walk" ), g_Vars.misc.slow_walk_bind );
			}

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		 } ImGui::EndChild( );

		 ImGui::End( );
	  }
   }

   ImGui::PopFont( );
#endif
}

void CMenu::Main( IDirect3DDevice9* pDevice ) {
   ImGuiStyle* style = &ImGui::GetStyle( );
   const auto& io = ImGui::GetIO( );

   constexpr auto alpha_freq = 1.0f / 0.2f;
   if ( g_Vars.globals.menuOpen ) {
	  m_alpha += alpha_freq * io.DeltaTime;
   } else {
	  m_alpha -= alpha_freq * io.DeltaTime;
   }

   m_alpha = Math::Clamp( m_alpha, 0.0f, 1.0f );
   for ( auto& keybind : g_keybinds ) {
	  if ( keybind->cond == KeyBindType::ALWAYS_ON )
		 keybind->enabled = true;
   }

   if ( m_alpha <= 0.f )
	  return;

   auto meme = style->Alpha;
   style->Alpha = m_alpha;
   style->Colors[ ImGuiCol_MenuTheme ] = ImVec4( g_Vars.misc.menu_ascent.r, g_Vars.misc.menu_ascent.g, g_Vars.misc.menu_ascent.b, g_Vars.misc.menu_ascent.a );

   static int tab = 0;
   style->WindowPadding = ImVec2( 6, 6 );

   ImGui::PushFont( menuFont );

   ImGui::SetNextWindowSize( ImVec2( 574.f, 640.f ) );
   ImGui::BeginMenuBackground( XorStr( "pasted russki p2c hake" ), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar );
   {
	  ImGui::BeginChild( XorStr( "Complete Border" ), ImVec2( 562.f, 628.f ), false );
	  {
		 ImGui::Image( menuBg, ImVec2( 562.f, 628.f ) );
	  } ImGui::EndChild( );

	  ImGui::SameLine( 6.f );

	  style->Colors[ ImGuiCol_ChildBg ] = ImColor( 0, 0, 0, 0 );

	  enum TABS_INDEXES {
		 RAGEBOT_TAB = 0,
		 LEGITBOT_TAB,
		 ANTIAIM_TAB,
		 PLAYERS_TAB,
		 VISUALS_TAB,
		 SKINCHANGER_TAB,
		 MISC_TAB
	  };

	  ImGui::BeginChild( XorStr( "Menu Contents" ), ImVec2( 558.0f, 628.f ), false, ImGuiWindowFlags_NoScrollbar );
	  {
		 //ImGui::ColorBar( XorStr( "unicorn" ), ImVec2( 557.f, 2.f ) );

		 style->ItemSpacing = ImVec2( 0.f, -1.f );

		 // 562
		 ImGui::BeginTabs( XorStr( "Tabs" ), ImVec2( 558.0f, 30.f ), false, ImGuiWindowFlags_NoScrollbar );
		 {
			style->ItemSpacing = ImVec2( 0.f, 0.f );

			style->ButtonTextAlign = ImVec2( 0.5f, 0.47f );

			ImGui::PopFont( );
			ImGui::PushFont( menuFont );

			float tab_width = 80.0f;

			const char* tabs_names[] = {
			   XorStr( "ragebot" ),
			   XorStr( "legitbot" ),
			   XorStr( "antiaim" ),
			   XorStr( "players" ),
			   XorStr( "visuals" ),
			   XorStr( "skinchanger" ),
			   XorStr( "misc" )
			};

			ImGui::TabSpacer( XorStr( "##Top Spacer" ), ImVec2( tab_width, 1.f ) );
			constexpr int max_tabs = sizeof( tabs_names ) / sizeof( const char* );
			for ( int i = 0; i < max_tabs; ++i ) {
			   if ( tab == i ) {
				  if ( ImGui::SelectedTab( tabs_names[ i ], ImVec2( tab_width, 30.f ) ) )
					 tab = i;
			   } else {
				  if ( ImGui::Tab( tabs_names[ i ], ImVec2( tab_width, 30.f ) ) )
					 tab = i;
			   }

			   if ( i + 1 != max_tabs )
				  ImGui::SameLine( );
			}
			ImGui::TabSpacer2( XorStr( "##Bottom Spacer" ), ImVec2( tab_width, 7.f ) );

			ImGui::PopFont( );
			ImGui::PushFont( menuFont );

			style->ButtonTextAlign = ImVec2( 0.5f, 0.5f );

		 } ImGui::EndTabs( );

		 /*ImGui::SameLine( 75.f );*/

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

		 ImGui::BeginChild( XorStr( "Tab Contents" ), ImVec2( 562.f, 628.f ), false );
		 {
			style->Colors[ ImGuiCol_Border ] = ImColor( 0, 0, 0, 0 );

			switch ( tab ) {
			   case RAGEBOT_TAB:
			   {
				  InsertSpacer( XorStr( "Top Spacer" ) );

				  ImGui::Columns( 2, NULL, false );
				  {
					 InsertGroupBoxLeft( XorStr( "Config" ), 125.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Enabled" ), g_Vars.rage.enabled );
						InsertCombo( XorStr( "Group" ), rage_current_group, rage_weapon_groups );
						InsertCheckbox( XorStr( "Friendly fire" ), g_Vars.rage.team_check );
						InsertCheckbox( XorStr( "Visual resolver" ), g_Vars.rage.visual_resolver );
						InsertCheckbox( XorStr( "Multithreading" ), g_Vars.rage.rage_multithread );
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

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 }
					 InsertEndGroupBoxLeft( XorStr( "Config Cover" ), XorStr( "Config" ) );

					 InsertSpacer( XorStr( "Config - Aimbot Spacer" ) );

					 InsertGroupBoxLeft( XorStr( "Aimbot" ), 388.0f + 46.0f - 15.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						const char* auto_stop_type[] = { XorStr( "Off" ), XorStr( "Full Stop" ), XorStr( "Minimal speed" ) };

						//InsertCheckbox( XorStr( "Fake lag correction" ), rbot->lagfix );
						//InsertCheckbox( XorStr( "Aim lock" ), rbot->aim_lock );

						InsertCheckbox( XorStr( "Enabled" ), rbot->active );
						InsertCheckbox( XorStr( "Silent aim" ), rbot->silent_aim );
						InsertCheckbox( XorStr( "Compensate spread" ), rbot->no_spread );

						InsertSlider( XorStr( "Minimum hit chance" ), rbot->hitchance, 0.f, 100.f, XorStr( "%.0f %%" ) );
						// InsertSlider( XorStr( "Hit chance accuracy" ), rbot->hitchance_accuracy, 0.f, 100.f, XorStr( "%.0f %%" ) );
						InsertSlider( XorStr( "Minimum damage - Penetration" ), rbot->min_damage, 1.f, ( rage_current_group - 1 == WEAPONGROUP_SNIPER ) ? 150.0f : 100.f, XorStr( "%.0f hp" ) );
						InsertSlider( XorStr( "Minimum damage - Visible" ), rbot->min_damage_visible, 1.f, ( rage_current_group - 1 == WEAPONGROUP_SNIPER ) ? 150.0f : 100.f, XorStr( "%.0f hp" ) );

						InsertCheckbox( XorStr( "Shot delay" ), rbot->shotdelay );
						if ( rbot->shotdelay ) {
						   InsertSliderWithoutText( XorStr( "##ShotDelayAmount" ), rbot->shotdelay_amount, 1.f, 100.f, XorStr( "%.0f %%" ) );
						}

						InsertCheckbox( XorStr( "Health based override" ), rbot->health_override );
						if ( rbot->health_override ) {
						   InsertSliderWithoutText( XorStr( "##HealthBasedOverride" ), rbot->health_override_amount, 1.f, 100.f, XorStr( "+%.0fhp" ) );
						}

						InsertCheckbox( XorStr( "Minimum damage override" ), rbot->min_damage_override );

						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						InsertKeyBox( "", g_Vars.rage.key_dmg_override );

						if ( rbot->min_damage_override ) {
						   InsertSlider( XorStr( "Minimum damage - Override" ), rbot->min_damage_override_amount, 1.f, 100.f, XorStr( "%.0f hp" ) );
						}

						InsertCheckbox( XorStr( "Automatic penetration" ), rbot->autowall );
						InsertCheckbox( XorStr( "Automatic scope" ), rbot->autoscope );
						InsertCombo( XorStr( "Automatic stop" ), rbot->autostop, auto_stop_type );

						if ( rbot->autostop ) {
						   InsertCheckbox( XorStr( "Between shots" ), rbot->between_shots );
						}

						InsertSliderInt( XorStr( "Max misses" ), rbot->max_misses, 1, 6, XorStr( "%d" ) );

					 #if 0
						InsertCheckbox( XorStr( "Unlag delay" ), rbot->delay_shot );
						InsertCheckbox( XorStr( "Shot delay" ), rbot->shotdelay );
						InsertSliderWithoutText( XorStr( "##shot_delay" ), rbot->shotdelay_amount, 0.f, 100.f, XorStr( "%.0f %%" ) );
					 #endif

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 } InsertEndGroupBoxLeft( XorStr( "Aimbot Cover" ), XorStr( "Aimbot" ) );
				  }
				  ImGui::NextColumn( );
				  {
					 InsertGroupBoxRight( XorStr( "Hitscan" ), 326.0f + 30.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						const char* hitbox_select_type[] = { XorStr( "Damage" ), XorStr( "Accuracy" ) };

						InsertCombo( XorStr( "Hitbox selection" ), rbot->hitbox_selection, hitbox_select_type );

					 #if 0
						if ( rbot->hitbox_selection == 0 ) {
						   InsertCheckbox( XorStr( "Prefer body aim" ), rbot->body_aim_if_lethal );
						}
					 #endif

						InsertCheckbox( XorStr( "Ignore limbs on move" ), rbot->ignorelimbs_ifwalking );

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
						   XorStr( "Unresolved" ),
						};

						/*
						const char* label, bool* current_items[], const char* items[],
					int items_count, int popup_max_height_in_items = -1, const char* default_preview = XorStr( "..." ) ) {
						*/

						ImGui::MultiCombo( XorStr( "Hitboxes" ),
										   hitboxes, hitboxes_str, 8 );

						InsertMultiCombo( XorStr( "Multipoint safety" ),
										  hitboxes_str, mp_safety_hitboxes, 8 );

						InsertCheckbox( XorStr( "Override hitscan" ), rbot->override_hitscan );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						InsertKeyBox( "", g_Vars.rage.override_key );

						if ( rbot->override_hitscan ) {
						   InsertMultiCombo( XorStr( "Override condition" ),
											 conditions_str, bt_conditions, 8 );

						   InsertMultiCombo( XorStr( "Override hitboxes" ),
											 hitboxes_str, bt_hitboxes, 8 );
						}

						InsertCheckbox( XorStr( "Prefer body" ), rbot->prefer_body );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						InsertKeyBox( "", g_Vars.rage.prefer_body );

						if ( rbot->prefer_body ) {
						   InsertMultiComboWithoutText( XorStr( "##PreferBody" ),
														conditions_str, body_conditions, 8 );
						}

						InsertCheckbox( XorStr( "Prefer head" ), rbot->prefer_head );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						InsertKeyBox( "", g_Vars.rage.prefer_head );

						if ( rbot->prefer_head ) {
						   InsertMultiComboWithoutText( XorStr( "##PreferHead" ),
														conditions_str, head_conditions, 8 );
						}

						InsertCheckbox( XorStr( "Prefer safety" ), rbot->prefer_safety );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						InsertKeyBox( "", g_Vars.rage.prefer_safe );

						if ( rbot->prefer_safety ) {
						   InsertMultiComboWithoutText( XorStr( "##PreferSafety" ),
														conditions_str, safe_conditions, 8 );
						}

						// InsertSlider( XorStr( "Prefer angle threshold##Head" ), rbot->prefer_threshold, 1.f, 90.0f, XorStr( "%.0f degrees" ) );

						const char* mp_density_str[] = {
						   XorStr( "Low" ), XorStr( "Medium" ), XorStr( "High" ),  XorStr( "Maximum" )
						};

						//InsertCombo( XorStr( "Multipoint density" ), rbot->mp_density, mp_density_str );

						InsertCheckbox( XorStr( "Dynamic point scale" ), rbot->dynamic_ps );
						if ( !rbot->dynamic_ps ) {
						   InsertSlider( XorStr( "Max head scale##Head" ), rbot->head_ps, 0.f, 95.0f, XorStr( "%.0f %%" ) );
						   InsertSlider( XorStr( "Max body scale##Head" ), rbot->body_ps, 0.f, 95.0f, XorStr( "%.0f %%" ) );
						}
						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 } InsertEndGroupBoxRight( XorStr( "Hitscan Cover" ), XorStr( "Hitscan" ) );

					 InsertSpacer( XorStr( "Hitscan - Exploits Spacer" ) );

					 InsertGroupBoxRight( XorStr( "Exploits" ), 162.0f + 26.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );

						ImGui::CustomSpacing( 9.f );

						const char* exploits_type[] = { XorStr( "Tickbase" ), XorStr( "Rapid" ) };
						InsertCheckbox( XorStr( "Enabled" ), g_Vars.rage.exploit );
						InsertCombo( XorStr( "Exploits" ), g_Vars.rage.exploit_type, exploits_type );

						if ( g_Vars.rage.exploit_type == 1 ) {
						   bool* rapid_conditions[] = {
							  &g_Vars.rage.rapid_on_shot,
							  &g_Vars.rage.rapid_on_peek,
						   };

						   const char* rapid_conditions_str[] = {
							  XorStr( "On shot" ),
							  XorStr( "On peek" ),
						   };

						   InsertMultiCombo( XorStr( "Conditions" ),
											 rapid_conditions_str, rapid_conditions, 2 );

						   InsertKeyBox( XorStr( "Rapid charge" ), g_Vars.rage.rapid_charge );
						   InsertKeyBox( XorStr( "Rapid release" ), g_Vars.rage.rapid_release );

						   InsertSliderInt( XorStr( "Charge amount" ), g_Vars.rage.charge_amount, 4, 15, XorStr( "%d ticks" ) );

						} else {
						   InsertKeyBox( XorStr( "Hide shots" ), g_Vars.rage.hide_shots_bind );
						   InsertKeyBox( XorStr( "Double tap" ), g_Vars.rage.double_tap_bind );
						   // InsertCheckbox( XorStr( "Break lag comp" ), g_Vars.rage.break_lagcomp );

						   g_Vars.rage.break_lagcomp = false;

						#if 0
						   ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 19.f );
						   ImGui::Text( XorStr( "Not available currently" ) );
						#endif
						}

					 #if 0

						if ( g_Vars.rage.exploit_type == 1 ) {


						}
					 #endif

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 }
					 InsertEndGroupBoxRight( XorStr( "Exploits Cover" ), XorStr( "Exploits" ) );
				  }
			   }
			   break;
			   case ANTIAIM_TAB:
			   {
				  InsertSpacer( XorStr( "Top Spacer" ) );

				  ImGui::Columns( 2, NULL, false );
				  {
					 InsertGroupBoxLeft( XorStr( "Anti-Aimbot" ), 506.f + 56.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						static int antiaim_type = 0;
						CVariables::ANTIAIM_STATE* settings = nullptr;

						InsertCheckbox( XorStr( "Enabled" ), g_Vars.antiaim.enabled );

						InsertCheckbox( XorStr( "Manual" ), g_Vars.antiaim.manual );
						if ( g_Vars.antiaim.manual ) {
						   InsertKeyBoxDefCond( XorStr( "Left" ), g_Vars.antiaim.manual_left_bind, KeyBindType::HOLD );
						   InsertKeyBoxDefCond( XorStr( "Right" ), g_Vars.antiaim.manual_right_bind, KeyBindType::HOLD );
						   InsertKeyBoxDefCond( XorStr( "Back" ), g_Vars.antiaim.manual_back_bind, KeyBindType::HOLD );
						   InsertKeyBoxDefCond( XorStr( "Mouse override" ), g_Vars.antiaim.mouse_override, KeyBindType::HOLD );
						}
						InsertKeyBoxDefCond( XorStr( "Desync flip" ), g_Vars.antiaim.desync_flip_bind, KeyBindType::TOGGLE );

						static bool* disable_conditions[] = {
						   &g_Vars.antiaim.on_freeze_period 	,
						   &g_Vars.antiaim.on_knife 	,
						   &g_Vars.antiaim.on_grenade 	,
						   &g_Vars.antiaim.on_ladder 	,
						   &g_Vars.antiaim.on_dormant 	,
						};

						const char* disable_conditions_str[] = {
						   XorStr( "On freeze period" ),
						   XorStr( "On knife" ),
						   XorStr( "On grenade" ),
						   XorStr( "On ladder" ),
						   XorStr( "On dormant" ),
						};

						InsertMultiCombo( XorStr( "Disable condition" ),
										  disable_conditions_str, disable_conditions, 5 );

						const char* antiaim_type_str[] = { XorStr( "Stand" ), XorStr( "Move" ), XorStr( "Air" ) };

						InsertCombo( XorStr( "Movement type" ), antiaim_type, antiaim_type_str );

						if ( antiaim_type == 0 )
						   settings = &g_Vars.antiaim_stand;
						else if ( antiaim_type == 1 )
						   settings = &g_Vars.antiaim_move;
						else
						   settings = &g_Vars.antiaim_air;

						const char* yaw_base_str[] = { XorStr( "View" ), XorStr( "At targets" ) };
						const char* yaw_base_move_str[] = { XorStr( "View" ), XorStr( "At targets" ), XorStr( "Movement direction" ) };
						const char* pitch_str[] = { XorStr( "Off" ), XorStr( "Down" ), XorStr( "Up" ), XorStr( "Zero" ) };
						const char* auto_dir[] = { XorStr( "Off" ), XorStr( "Hide Fake" ), XorStr( "Hide Real" ) };
						const char* desync_str[] = { XorStr( "Off" ), XorStr( "Static" ), XorStr( "Jitter" ) };
						const char* lby_str[] = { XorStr( "Off" ), XorStr( "On" ), XorStr( "Sway" ), XorStr( "Low delta" ) };

						ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
						auto str = antiaim_type == 0 ? yaw_base_str : yaw_base_move_str;
						auto str_size = antiaim_type == 0 ? 2 : 3;
						ImGui::Combo( XorStr( "Yaw base" ), &settings->base_yaw,
									  str, str_size );

						ImGui::PopItemWidth( ); ImGui::CustomSpacing( 1.f );

						InsertCombo( XorStr( "Pitch" ), settings->pitch, pitch_str );

						InsertCombo( XorStr( "Desync" ), settings->desync, desync_str );

						if ( settings->desync > 2 )
						   settings->desync = 2;

						if ( antiaim_type == 0 ) {
						   InsertCombo( XorStr( "Break lower body" ), g_Vars.antiaim.break_lby, lby_str );
						}

						if ( settings->desync != 3 ) {
						   InsertSlider( XorStr( "Desync offset" ), settings->desync_amount, 0.0f, 100.0f, XorStr( "%.0f %%" ) );
						   InsertSlider( XorStr( "Desync offset flipped" ), settings->desync_amount_flipped, 0.0f, 100.0f, XorStr( "%.0f %%" ) );
						}

						InsertCheckbox( XorStr( "Jitter" ), settings->jitter );
						if ( settings->jitter ) {
						   InsertSliderWithoutText( XorStr( "##Jitter range" ), settings->jitter_range, 0.0f, 90.0f, XorStr( "%.0f degrees" ) );
						}

						InsertCheckbox( XorStr( "Spin" ), settings->spin );
						if ( settings->spin ) {
						   InsertCheckbox( XorStr( "Switch" ), settings->spin_switch );
						   InsertSlider( XorStr( "Speed##Spin" ), settings->spin_speed, -50.0f, 50.0f, XorStr( "%.0f %%" ) );
						   InsertSlider( XorStr( "Range##Spin" ), settings->spin_range, 1.0f, 180.0f, XorStr( "%.0f degrees" ) );
						}

						InsertCheckbox( XorStr( "Auto direction" ), settings->autodirection );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						InsertKeyBox( "", g_Vars.antiaim.autodirection_override );
						if ( settings->autodirection ) {
						   InsertCheckbox( XorStr( "Auto direction ignore duck" ), settings->autodirection_ignore_duck );

						   InsertSliderInt( XorStr( "Extrapolation##AutoDirExtrapolation" ), settings->extrapolation_ticks, 0, 20, XorStr( "%d ticks" ) );
						   InsertSlider( XorStr( "Range##AutoDirRange" ), settings->autodirection_range, 24.0f, 64.0f, XorStr( "%.0f units" ) );
						   //InsertSlider( XorStr( "Delay##AutoDirDelay" ), settings->autodirection_delay, 0.00f, 2.00f, XorStr( "%.2f seconds" ) );

						   InsertCombo( XorStr( "Desync auto direction" ), settings->desync_autodir, auto_dir );
						}
						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 } InsertEndGroupBoxLeft( XorStr( "Anti-Aimbot Cover" ), XorStr( "Anti-Aimbot" ) );
				  }
				  ImGui::NextColumn( );
				  {
					 InsertGroupBoxRight( XorStr( "Fakelag" ), 506.f + 56.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Enabled## fake lag" ), g_Vars.fakelag.enabled );

					 #if 1
						static bool* conditions[] = {
						   &g_Vars.fakelag.when_standing,
						   &g_Vars.fakelag.when_moving,
						   &g_Vars.fakelag.when_air,
						   &g_Vars.fakelag.when_dormant,
						};

						const char* conditions_str[] = {
						   XorStr( "Standing" ), XorStr( "Moving" ), XorStr( "Air" ),  XorStr( "Dormant" ),
						};
					 #endif

						static bool* alt_conditions[] = {
						   &g_Vars.fakelag.when_peek,
						   &g_Vars.fakelag.when_unducking,
						   &g_Vars.fakelag.when_land,
						   &g_Vars.fakelag.when_switching_weapon,
						   &g_Vars.fakelag.when_reloading,
						};

						const char* alt_conditions_str[] = {
						   XorStr( "Peek" ), XorStr( "Unducking" ), XorStr( "Land" ), XorStr( "Weapon Swap" ), XorStr( "Reload" )
						};

						InsertMultiCombo( XorStr( "Conditions" ), conditions_str, conditions, 4 );

						// InsertCheckbox( XorStr( "Latency compensation" ), g_Vars.fakelag.auto_set );

						const char* mode[] = { XorStr( "Off" ), XorStr( "Passive" ), XorStr( "Adaptive" ), XorStr( "Step" ), XorStr( "Dynamic" ) };

						//if ( g_Vars.fakelag.mode > 4 )
						//   g_Vars.fakelag.mode = 4;

						//InsertCombo( XorStr( "Mode" ), g_Vars.fakelag.mode, mode );
						InsertSliderInt( XorStr( "Limit" ), g_Vars.fakelag.choke, 1, g_Vars.fakelag.lag_limit, XorStr( "%d ticks" ) );
						InsertSlider( XorStr( "Variance" ), g_Vars.fakelag.variance, 0.0f, 100.0f, XorStr( "%.0f %%" ) );

						InsertMultiCombo( XorStr( "Alternative conditions" ), alt_conditions_str, alt_conditions, 5 );
						InsertSliderInt( XorStr( "Alternative lag amount" ), g_Vars.fakelag.alternative_choke, 0, g_Vars.fakelag.lag_limit, XorStr( "%d ticks" ) );
						// InsertCheckbox( XorStr( "Auto distance" ), g_Vars.fakelag.auto_peekdist );
						// if ( !g_Vars.fakelag.auto_peekdist ) {
						//    InsertSlider( XorStr( "Peek distance" ), g_Vars.fakelag.peekdist, 20.0f, 60.0f, XorStr( "%.0f %%" ) );
						// }
						InsertSliderInt( XorStr( "Lag limit" ), g_Vars.fakelag.lag_limit, 6, 62, XorStr( "%d ticks" ) );

						//	ImGui::Checkbox( XorStr( "Server crasher" ), &g_Vars.misc.server_crasher );
						//	ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
						//	ImGui::KeyBox( &g_Vars.misc.server_crasher_bind );

						//	ImGui::SliderIntA( XorStr( "Messages" ), &g_Vars.misc.server_crasher_msges, 1, 500, XorStr( "%d" ) );
						//	ImGui::SliderIntA( XorStr( "Packets" ), &g_Vars.misc.server_crasher_packets, 1, 5, XorStr( "%d" ) );

						// std::min( 5, g_Vars.fakelag.choke + 3 );

						// for test only
						//ImGui::SliderIntA( XorStr( "Shift" ), &g_Vars.globals.FakeDuckWillChoke, 0, 17, XorStr( "%d ticks" ) );
						// ImGui::SliderIntA( XorStr( "Min delta" ), &g_Vars.globals.SavedCommandNr, 0, 17, XorStr( "%d ticks" ) );

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 } InsertEndGroupBoxRight( XorStr( "Fakelag Cover" ), XorStr( "Fakelag" ) );
				  }
				  break;
			   }
			   case LEGITBOT_TAB:
			   {
				  InsertSpacer( XorStr( "Top Spacer" ) );

				  InsertGroupBoxTop( XorStr( "Weapon Selection" ), ImVec2( 531.f, 61.f ) );
				  {
					 style->ItemSpacing = ImVec2( 4, 2 );
					 style->WindowPadding = ImVec2( 4, 4 );

					 ImGui::BeginGroup( );
					 {
						// ghetto-workaround cuz shitty macroces
						auto backup = current_group;
						InsertCombo( XorStr( "Group" ), current_group, weapon_groups );
						if ( backup != current_group ) {
						   current_weapon = 0;
						}
					 }
					 ImGui::EndGroup( );

					 ImGui::SameLine( );

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

					 ImGui::BeginGroup( );
					 {
						ImGui::CustomSpacing( 11.f );
						ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
						{
						   ImGui::Combo(
							  XorStr( "Weapon##group_weapons" ), &current_weapon,
							  [] ( void* data, int32_t idx, const char** out_text ) {
							  auto vec = reinterpret_cast< group_vector* >( data );
							  *out_text = vec->at( idx ).first.c_str( );
							  return true;
						   }, ( void* ) ( aim_group ), aim_group->size( ), 20 );

						}
						ImGui::PopItemWidth( ); ImGui::CustomSpacing( 1.f );
					 }
					 ImGui::EndGroup( );

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

				  #if 0
					 WEAPONGROUP_PISTOL,
						WEAPONGROUP_RIFLE,
						WEAPONGROUP_SNIPER,
						WEAPONGROUP_AUTOSNIPER,
						WEAPONGROUP_SUBMACHINE,
						WEAPONGROUP_HEAVY,
						WEAPONGROUP_SHOTGUN,
						auto name = XorStr( "none" );
					 switch ( item.group ) {
						case WEAPONGROUP_PISTOL:
						name = XorStr( "Pistol" );
						break;
						case WEAPONGROUP_SUBMACHINE:
						name = XorStr( "SMG" );
						break;
						case WEAPONGROUP_RIFLE:
						name = XorStr( "Rifle" );
						break;
						case WEAPONGROUP_SHOTGUN:
						name = XorStr( "Shotgun" );
						break;
						case WEAPONGROUP_SNIPER:
						name = XorStr( "Sniper-Rifle" );
						break;
						case WEAPONGROUP_HEAVY:
						name = XorStr( "Heavy" );
						break;
						case WEAPONGROUP_AUTOSNIPER:
						name = XorStr( "Auto-Sniper" );
						break;
					 }
				  #endif

					 style->ItemSpacing = ImVec2( 0, 0 );
					 style->WindowPadding = ImVec2( 6, 6 );
				  } InsertEndGroupBoxTop( XorStr( "Weapon Selection Cover" ), XorStr( "Weapon Selection" ), ImVec2( 536.f, 11.f ) );

				  InsertSpacer( XorStr( "Weapon Selection - Main Group boxes Spacer" ) );

				  ImGui::Columns( 2, NULL, false );
				  {
					 InsertGroupBoxLeft( XorStr( "Aimbot" ), 484.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );
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

						InsertCheckbox( XorStr( "Active" ), lbot->active );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						InsertKeyBox( "", lbot->key );

						InsertCheckbox( XorStr( "Auto fire" ), lbot->autofire );
						InsertCheckbox( XorStr( "Auto wall" ), lbot->autowall );
						InsertCheckbox( XorStr( "Quick stop" ), lbot->quickstop );

						if ( current_group == 3 ) {
						   InsertCheckbox( XorStr( "Only in scope" ), g_Vars.legit.snipers_only_scope );
						}

						InsertCombo( XorStr( "Hitbox selection" ), lbot->hitbox_selection, hitbox_selection );
						InsertCombo( XorStr( "Smooth type" ), lbot->smooth_type, smooth_type );

						if ( lbot->hitbox_selection == 0 ) {
						   InsertCombo( XorStr( "Hitbox" ), lbot->hitbox, priority_hitboxes );
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

						   InsertMultiCombo( XorStr( "Hitboxes" ), hitboxes, opt_hitboxes, 7 );
						}

						ImGui::NextColumn( );

						InsertSlider( XorStr( "Field of view" ), lbot->fov, 1.f, 30.0f, XorStr( "%0.2fdeg" ) );
						InsertSlider( XorStr( "Dead zone" ), lbot->deadzone, 0.f, 2.0f, XorStr( "%0.2fdeg" ) );
						InsertSlider( XorStr( "Smooth" ), lbot->smooth, 1.0f, 10.0f, XorStr( "%0.2f" ) );
						InsertSlider( XorStr( "Randomize" ), lbot->randomize, 0.0f, 5.0f, XorStr( "%0.1f" ) );

						InsertCheckbox( XorStr( "Recoil control" ), lbot->rcs );
						InsertCheckbox( XorStr( "Standalone recoil control" ), lbot->rcs_standalone );
						InsertSliderInt( XorStr( "Recoil start bullet" ), lbot->rcs_shots, 0, 5, XorStr( "%d shots" ) );
						InsertSlider( XorStr( "Recoil X" ), lbot->rcs_x, 0.f, 100.f, XorStr( "%0.0f %%" ) );
						InsertSlider( XorStr( "Recoil Y" ), lbot->rcs_y, 0.f, 100.f, XorStr( "%0.0f %%" ) );

						InsertCheckbox( XorStr( "Kill delay" ), lbot->kill_delay );
						if ( lbot->kill_delay ) {
						   InsertSliderWithoutText( XorStr( "##Kill Delay Time" ), lbot->kill_shot_delay, 0.100f, 2.f, XorStr( "%0.3f sec" ) );
						}

						InsertCheckbox( XorStr( "First shot delay" ), lbot->fsd_enabled );
						if ( lbot->fsd_enabled ) {
						   InsertCheckbox( XorStr( "Auto##delay" ), lbot->auto_delay );

						   if ( !lbot->auto_delay )
							  InsertSliderWithoutText( XorStr( "##First shot delay" ), lbot->first_shot_delay, 0.015f, 0.200f, XorStr( "%0.3f sec" ) );
						}

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );

					 } InsertEndGroupBoxLeft( XorStr( "Aimbot Cover" ), XorStr( "Aimbot" ) );
				  }
				  ImGui::NextColumn( );
				  {
					 InsertGroupBoxRight( XorStr( "Triggerbot" ), 277.f + 56.0f / 2.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Enabled" ), lbot->trg_enabled );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						InsertKeyBox( "", lbot->trg_key );

						InsertCheckbox( XorStr( "Autowall" ), lbot->trg_autowall );
						InsertCheckbox( XorStr( "Run aimbot" ), lbot->trg_aimbot );
						InsertSlider( XorStr( "Hitchance" ), lbot->trg_hitchance, 0.0f, 100.0f, XorStr( "%0.0f %%" ) );
						InsertSlider( XorStr( "Delay" ), lbot->trg_delay, 0.015f, 0.300f, XorStr( "%0.3f sec" ) );
						InsertSlider( XorStr( "Burst" ), lbot->trg_burst, 0.050f, 0.500f, XorStr( "%0.3f sec" ) );

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

						InsertMultiCombo( XorStr( "Hitboxes" ), hitboxes, opt_hitboxes, 5 );

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 } InsertEndGroupBoxRight( XorStr( "Triggerbot Cover" ), XorStr( "Triggerbot" ) );

					 InsertSpacer( XorStr( "Triggerbot - Other Spacer" ) );

					 InsertGroupBoxRight( XorStr( "Extra" ), 132.f + 56.0f / 2.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Autopistol" ), g_Vars.legit.autopistol );
						if ( g_Vars.legit.autopistol ) {
						   InsertSliderWithoutText( XorStr( "##Auto pistol delay" ), g_Vars.legit.autopistol_delay, 0.00f, 0.100f, XorStr( "%0.3fsec" ) );
						}

						InsertCheckbox( XorStr( "Through smoke" ), g_Vars.legit.throughsmoke );
						InsertCheckbox( XorStr( "While blind" ), g_Vars.legit.whileblind );
						InsertCheckbox( XorStr( "Ignore jump" ), g_Vars.legit.ignorejump );
						InsertCheckbox( XorStr( "Position adjustment" ), g_Vars.legit.position_adjustment );

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 } InsertEndGroupBoxRight( XorStr( "Extra Cover" ), XorStr( "Extra" ) );
				  }
			   }
			   break;
			   case PLAYERS_TAB:
			   {
				  InsertSpacer( XorStr( "Top Spacer" ) );

				  ImGui::Columns( 2, NULL, false );
				  {
					 InsertGroupBoxLeft( XorStr( "Player ESP" ), 331.f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						const char* esp_type_str[] = { XorStr( "Static" ), XorStr( "Dynamic" ) };

						InsertCheckbox( XorStr( "Enabled" ), g_Vars.esp.esp_enable );
						if ( g_Vars.esp.esp_enable ) {
						   InsertCheckbox( XorStr( "Ignore team" ), g_Vars.esp.team_check );
						   InsertCheckbox( XorStr( "Extended ESP" ), g_Vars.esp.extended_esp );
						   InsertCheckbox( XorStr( "Dormant fade" ), g_Vars.esp.fade_esp );
						   InsertCheckbox( XorStr( "Box" ), g_Vars.esp.box );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Box Color" ), g_Vars.esp.box_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						   const char* box_type_str[] = { XorStr( "Bounding" ), XorStr( "Corner" ) };
						   if ( g_Vars.esp.box ) {
							  InsertComboWithoutText( XorStr( "##box type" ), g_Vars.esp.box_type, box_type_str );
						   }

						   InsertCheckbox( XorStr( "Name" ), g_Vars.esp.name );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Name Color" ), g_Vars.esp.name_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						   InsertCheckbox( XorStr( "Skeleton" ), g_Vars.esp.skeleton );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Skelet Color" ), g_Vars.esp.skeleton_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						   InsertCheckbox( XorStr( "Skeleton history" ), g_Vars.esp.skeleton_history );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Skelet History Color" ), g_Vars.esp.skeleton_history_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						   const char* history_type_str[] = { XorStr( "Oldest Tick" ), XorStr( "All Ticks" ) };
						   if ( g_Vars.esp.skeleton_history ) {
							  InsertComboWithoutText( XorStr( "##Skelet History Type" ), g_Vars.esp.skeleton_history_type, history_type_str );
						   }
						   InsertCheckbox( XorStr( "Aim points" ), g_Vars.esp.aim_points );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Aim points Color" ), g_Vars.esp.aim_points_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						   InsertCheckbox( XorStr( "Draw hitboxes" ), g_Vars.esp.draw_hitboxes );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Draw hitboxes color" ), g_Vars.esp.hitboxes_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );


						   InsertCheckbox( XorStr( "Ammo bar" ), g_Vars.esp.draw_ammo_bar );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##ammo_color color" ), g_Vars.esp.ammo_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						   InsertCheckbox( XorStr( "Health" ), g_Vars.esp.health );
						   if ( g_Vars.esp.health ) {
							  InsertCheckbox( XorStr( "Health Animated" ), g_Vars.esp.animated_hp );
						   }
						   static bool* flags_conditions[] = {
							  &g_Vars.esp.draw_scoped,
							  &g_Vars.esp.draw_flashed,
							  &g_Vars.esp.draw_money,
							  &g_Vars.esp.draw_armor,
							  &g_Vars.esp.draw_reloading,
							  &g_Vars.esp.draw_ping,
							  &g_Vars.esp.draw_hit,
							  &g_Vars.esp.draw_taser,
							  &g_Vars.esp.draw_bombc4,
							  &g_Vars.esp.draw_distance,
							  //&g_Vars.esp.draw_hostage,
						   };

						   const char* flags_conditions_str[] = {
							  XorStr( "Scoped" ), XorStr( "Flashed" ), XorStr( "Money" ), XorStr( "Armor" ), XorStr( "Reloading" ), XorStr( "Ping" ),
							  XorStr( "Hit" ), XorStr( "Taser" ), XorStr( "Bomb" ), XorStr( "Distance" ), //XorStr( "Hostage" ),
						   };

						   InsertMultiCombo( XorStr( "Flags" ),
											 flags_conditions_str, flags_conditions, 9 );

						   static bool* weapon_conditions[] = {
							  &g_Vars.esp.weapon,
							  &g_Vars.esp.weapon_ammo,
							  &g_Vars.esp.weapon_icon,
							  &g_Vars.esp.weapon_other,
						   };

						   const char* weapon_conditions_str[] = {
							  XorStr( "Name" ), XorStr( "Ammo" ), XorStr( "Icon" ), XorStr( "Other" )
						   };

						   InsertMultiCombo( XorStr( "Weapon" ),
											 weapon_conditions_str, weapon_conditions, 4 );
						}

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );

					 } InsertEndGroupBoxLeft( XorStr( "Player ESP Cover" ), XorStr( "Player ESP" ) );

					 InsertSpacer( XorStr( "Player ESP - Colored models Spacer" ) );

					 InsertGroupBoxLeft( XorStr( "Effects" ), 157.f + 56.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Transparency in scope" ), g_Vars.esp.blur_in_scoped );
						InsertSliderWithoutText( XorStr( "##Transparency In Scope" ), g_Vars.esp.blur_in_scoped_value, 0.0f, 1.f, XorStr( "%0.01f" ) );

						InsertCheckbox( XorStr( "Glow team" ), g_Vars.esp.glow_team );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##Glow Team Color" ), g_Vars.esp.glow_team_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						InsertCheckbox( XorStr( "Glow enemy" ), g_Vars.esp.glow_enemy );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##Glow Enemy Color" ), g_Vars.esp.glow_enemy_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						InsertCheckbox( XorStr( "Glow weapons" ), g_Vars.esp.glow_weapons );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##Glow Weapon Color" ), g_Vars.esp.glow_weapons_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						InsertCheckbox( XorStr( "Glow grenades" ), g_Vars.esp.glow_grenade );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##Glow grenade Color" ), g_Vars.esp.glow_grenade_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						const char* glow_type_str[] = { XorStr( "Outline" ), XorStr( "Cover" ), XorStr( "Thin" ) };

						InsertComboWithoutText( XorStr( "##Glow type" ), g_Vars.esp.glow_type, glow_type_str );

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );

					 } InsertEndGroupBoxLeft( XorStr( "Effects Cover" ), XorStr( "Effects" ) );
				  }
				  ImGui::NextColumn( );
				  {
					 InsertGroupBoxRight( XorStr( "Other ESP" ), 199.f + 56.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						static bool* event_logger_cond[] = {
						   &g_Vars.esp.event_bomb,
						   &g_Vars.esp.event_dmg,
						   &g_Vars.esp.event_buy,
						   &g_Vars.esp.event_resolver,
						   &g_Vars.esp.event_misc,
						   &g_Vars.esp.event_exploits,
						   &g_Vars.esp.event_console,
						};

						const char* event_logger_cond_str[] = {
						   XorStr( "Bomb" ), XorStr( "Damage" ), XorStr( "Buy" ), XorStr( "Resolver" ), XorStr( "Misc" ), XorStr( "Exploits" ),
						   XorStr( "Console" )
						};

						InsertMultiCombo( XorStr( "Event logger" ),
										  event_logger_cond_str, event_logger_cond, 7 );

						const char* logger_type[] = { XorStr( "Default" ), XorStr( "Nvidia" ) };

						InsertCombo( XorStr( "Event Logger Type" ), g_Vars.esp.event_logger_type, logger_type );

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
						   XorStr( "Smoke" ), XorStr( "Flash" ), XorStr( "Scope Overlay" ), XorStr( "Zoom" ), XorStr( "Zoom blur" ), XorStr( "Recoil" ), XorStr( "Sleeves" ),
						   XorStr( "Blur" ), XorStr( "Post Proccesing" )
						};

						InsertMultiCombo( XorStr( "Removals" ),
										  removeals_cond_str, removeals_cond, 9 );

						InsertCheckbox( XorStr( "Visualize sounds" ), g_Vars.esp.sound_esp );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##SoundEsp Color" ), g_Vars.esp.sound_esp_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						if ( g_Vars.esp.sound_esp ) {
						   const char* sound_esp_type_str[] = { XorStr( "Beam" ), XorStr( "Default" ) };

						   InsertComboWithoutText( XorStr( "##TypeSoundEsp" ), g_Vars.esp.sound_esp_type, sound_esp_type_str );
						   InsertSlider( XorStr( "Time##SEsp" ), g_Vars.esp.sound_esp_time, 0.f, 8.0f, XorStr( "%.1f seconds" ) );
						   InsertSlider( XorStr( "Radius##SEsp" ), g_Vars.esp.sound_esp_radius, 0.f, 500.0f, XorStr( "%.1f units" ) );
						}
						InsertCheckbox( XorStr( "Offscreen arrow" ), g_Vars.esp.offscren_enabled );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##Offscreen Color" ), g_Vars.esp.offscreen_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						if ( g_Vars.esp.offscren_enabled ) {
						   InsertCheckbox( XorStr( "Offscreen pulse alpha" ), g_Vars.esp.pulse_offscreen_alpha );
						   InsertCheckbox( XorStr( "Offscreen outline" ), g_Vars.esp.offscreen_outline );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Offscreen outline Color" ), g_Vars.esp.offscreen_outline_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						   InsertSliderInt( XorStr( "Size##Off" ), g_Vars.esp.offscren_size, 0, 50, XorStr( "%i px" ) );
						   InsertSliderInt( XorStr( "Distance##iOff" ), g_Vars.esp.offscren_distance, 0, 700, XorStr( "%i px" ) );
						}
						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );

					 } InsertEndGroupBoxRight( XorStr( "Other ESP Cover" ), XorStr( "Other ESP" ) );

					 InsertSpacer( XorStr( "Other ESP - Effects Spacer" ) );

					 InsertGroupBoxRight( XorStr( "Colored models" ), 289.f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Enabled##chams" ), g_Vars.esp.chams_enabled );
						if ( g_Vars.esp.chams_enabled ) {
						   const char* materials[] = { XorStr( "Off" ), XorStr( "Flat" ), XorStr( "Flat with wireframe" ), XorStr( "Metallic" ), XorStr( "Metalic with wireframe" ), XorStr( "Glow" ) };
						   const char* no_dis_materials[] = { XorStr( "Flat" ), XorStr( "Flat with wireframe" ), XorStr( "Metallic" ), XorStr( "Metalic with wireframe" ), XorStr( "Glow" ) };

						   InsertCombo( XorStr( "Enemy Material##enemy" ), g_Vars.esp.enemy_chams_mat, materials );
						   InsertCombo( XorStr( "Team Material##team" ), g_Vars.esp.team_chams_mat, materials );

						  //InsertCheckbox( XorStr( "Hands" ), g_Vars.esp.hands_chams );
						  //if ( g_Vars.esp.hands_chams ) {
							//  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
							//  ImGui::ColorEdit4( XorStr( "##Hands Color" ), g_Vars.esp.hands_chams_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
							//  InsertComboWithoutText( XorStr( "##Hands Material" ), g_Vars.esp.hands_chams_mat, no_dis_materials );
						  //}
						  //
						  //InsertCheckbox( XorStr( "Weapon" ), g_Vars.esp.weapon_chams );
						  //if ( g_Vars.esp.weapon_chams ) {
							//  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
							//  ImGui::ColorEdit4( XorStr( "##Weapon Color" ), g_Vars.esp.weapon_chams_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
							//  InsertComboWithoutText( XorStr( "##Weapon Material" ), g_Vars.esp.weapon_chams_mat, no_dis_materials );
						  //}

						   InsertCheckbox( XorStr( "Lag chams" ), g_Vars.esp.chams_lag );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Lag color" ), g_Vars.esp.chams_lag_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						   InsertCheckbox( XorStr( "Local player" ), g_Vars.esp.chams_local );
						   if ( g_Vars.esp.chams_local ) {
							  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
							  ImGui::ColorEdit4( XorStr( "##Local color" ), g_Vars.esp.chams_local_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
							  InsertComboWithoutText( XorStr( "##Local material" ), g_Vars.esp.chams_local_mat, no_dis_materials );
						   }

						  //InsertCheckbox( XorStr( "Desync chams" ), g_Vars.esp.chams_desync );
						  //if ( g_Vars.esp.chams_desync ) {
							//  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
							//  ImGui::ColorEdit4( XorStr( "##Desync color" ), g_Vars.esp.chams_desync_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
							//  InsertComboWithoutText( XorStr( "##Desync material" ), g_Vars.esp.chams_desync_mat, no_dis_materials );
						  //}

						   InsertCheckbox( XorStr( "Enemy Vis" ), g_Vars.esp.enemy_chams_vis );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Team Chams Color XQZ" ), g_Vars.esp.enemy_chams_color_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						   InsertCheckbox( XorStr( "Enemy XQZ" ), g_Vars.esp.enemy_chams_xqz );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Enemy Chams Color XQZ" ), g_Vars.esp.enemy_chams_color_xqz, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						   InsertCheckbox( XorStr( "Team Vis" ), g_Vars.esp.team_chams_vis );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Team Chams Color Vis" ), g_Vars.esp.team_chams_color_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						   InsertCheckbox( XorStr( "Team XQZ" ), g_Vars.esp.team_chams_xqz );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Enemy Chams Color Vis" ), g_Vars.esp.team_chams_color_xqz, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						   InsertCheckbox( XorStr( "Shadow" ), g_Vars.esp.chams_history );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Chams History Color" ), g_Vars.esp.chams_history_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						  //InsertCheckbox( XorStr( "Double Coloring" ), g_Vars.esp.second_color );
						  //ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						  //ImGui::ColorEdit4( XorStr( "##Double Coloring Color" ), g_Vars.esp.second_chams_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						  //
						  //InsertSlider( XorStr( "Reflectivity##chams" ), g_Vars.esp.chams_reflectivity, 0.f, 1.f, XorStr( "%0.2f" ) );
						  ////InsertSlider( XorStr( "Contrast##chams" ), g_Vars.esp.chams_contrast, 0.f, 50.f, XorStr( "%0.1f" ) );
						  //InsertSlider( XorStr( "Shine##chams" ), g_Vars.esp.chams_shine, 0.f, 20.f, XorStr( "%0.1f" ) );
						  //InsertSlider( XorStr( "Rim##chams" ), g_Vars.esp.chams_depth, 1.0f, 100.f, XorStr( "%0.0f" ) );
						  //InsertSlider( XorStr( "Brightness" ), g_Vars.esp.chams_brightness, 0.f, 5.0f, XorStr( "%.2f" ) );
						  //InsertSlider( XorStr( "Border" ), g_Vars.esp.chams_border, 0.f, 20.0f, XorStr( "%.1f" ) );
						  //InsertSlider( XorStr( "Outline" ), g_Vars.esp.chams_outline, 0.f, 20.0f, XorStr( "%.1f" ) );
						  //InsertSlider( XorStr( "Embossing" ), g_Vars.esp.chams_embossing, 0.f, 10.0f, XorStr( "%.1f" ) );

						   InsertCheckbox( XorStr( "Skip occlusion" ), g_Vars.esp.skip_occulusion );

						  // InsertSlider( XorStr( "Glow Modifier##glow chams" ), g_Vars.esp.glow_modifier, 0.f, 1.f, XorStr( "%0.3f" ) );
						}
						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );

					 } InsertEndGroupBoxRight( XorStr( "Colored models Cover" ), XorStr( "Colored models" ) );
				  }
			   }
			   break;
			   case VISUALS_TAB:
			   {
				  InsertSpacer( XorStr( "Top Spacer" ) );

				  ImGui::Columns( 2, NULL, false );
				  {
					 InsertGroupBoxLeft( XorStr( "Part 1" ), 506.f + 56.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Grenade prediction" ), g_Vars.esp.NadePred );
						//if ( g_Vars.esp.NadePred ) {
						//   InsertCheckbox( XorStr( "Only when pin pulled" ), g_Vars.esp.grenade_pred_only_pin_pull );
						//}
						InsertCheckbox( XorStr( "Bullet tracer" ), g_Vars.esp.beam_enabled );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##BulletTracer Color" ), g_Vars.esp.beam_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						if ( g_Vars.esp.beam_enabled ) {
						   const char* beam_type_str[] = { XorStr( "Physbeam" ), XorStr( "Blue glow" ), XorStr( "Bubble" ), XorStr( "Glow" ), XorStr( "Purple glow" ), XorStr( "Purple laser" ), XorStr( "Radio" ), XorStr( "White" ) };

						   InsertComboWithoutText( XorStr( "##Type##BulletTracer" ), g_Vars.esp.beam_type, beam_type_str );
						   InsertSlider( XorStr( "Time##BulletTracer" ), g_Vars.esp.beam_life_time, 0.f, 10.0f, XorStr( "%.1f seconds" ) );
						   InsertSlider( XorStr( "Speed##BulletTracer" ), g_Vars.esp.beam_speed, 0.0f, 10.0f, XorStr( "%.1f units" ) );
						   InsertSlider( XorStr( "Width##BulletTracer" ), g_Vars.esp.beam_width, 1.f, 10.0f, XorStr( "%.1f size" ) );
						}

						static bool* hitmarkers_conditions[] = {
						   &g_Vars.esp.vizualize_hitmarker	,
						  // &g_Vars.esp.hitmarker,
						  // &g_Vars.esp.vizualize_hitmarker_text,
						};

						const char* hitmarkers_conditions_str[] = {
						   XorStr( "World" ), XorStr( "Crosshair" ), XorStr( "Text" )
						};

						InsertMultiCombo( XorStr( "Hit marker" ),
										  hitmarkers_conditions_str, hitmarkers_conditions, 3 );

						InsertCheckbox( XorStr( "Hit sound" ), g_Vars.misc.hitsound );

						const char* hitsound_type_str[] = { XorStr( "Switch" ), XorStr( "Bubble" ), };

						if ( g_Vars.misc.hitsound ) {
						   InsertComboWithoutText( XorStr( "##HitSound Type##HitSound" ), g_Vars.misc.hitsound_type, hitsound_type_str );
						}

						InsertCheckbox( XorStr( "Snaplines" ), g_Vars.esp.snaplines_enalbed );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##snaplines Color" ), g_Vars.esp.snaplines_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						const char* snaplines_str[] = { XorStr( "Left" ), XorStr( "Right" ), XorStr( "Top" ), XorStr( "Bottom" ), XorStr( "Center" ) };
						if ( g_Vars.esp.snaplines_enalbed ) {
						   InsertCombo( XorStr( "Position##snaplines pos" ), g_Vars.esp.snaplines_pos, snaplines_str );
						}
						InsertCheckbox( XorStr( "Visualise angles" ), g_Vars.esp.vizualize_angles );
						InsertCheckbox( XorStr( "Manual Anti-Aim indicators" ), g_Vars.esp.aa_indicator );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##Manual Color" ), g_Vars.esp.aa_indicator_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						const char* manual_str[] = { XorStr( "Around the crosshair" ), XorStr( "Bottom" ) };
						if ( g_Vars.esp.aa_indicator ) {
						   InsertCombo( XorStr( "Manual Type##Manual Type" ), g_Vars.esp.aa_indicator_type, manual_str );
						   InsertCheckbox( XorStr( "Zeus distance" ), g_Vars.esp.zeus_distance );
						}
						InsertCheckbox( XorStr( "Force crosshair" ), g_Vars.esp.force_sniper_crosshair );
						InsertCheckbox( XorStr( "Dark Mode" ), g_Vars.esp.dark_mode );
						InsertCheckbox( XorStr( "Fullbright" ), g_Vars.esp.fullbright );

						InsertCheckbox( XorStr( "Walls modulation" ), g_Vars.esp.walls );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##wall_modulation Color" ), g_Vars.esp.wall_modulation, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						InsertCheckbox( XorStr( "Props modulation" ), g_Vars.esp.props );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##props_modulation Color" ), g_Vars.esp.props_modulation, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						InsertCheckbox( XorStr( "Skybox modulation" ), g_Vars.esp.skybox );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##skybox_modulation Color" ), g_Vars.esp.skybox_modulation, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						InsertCheckbox( XorStr( "Kill effect" ), g_Vars.esp.kill_effect );
						if ( g_Vars.esp.kill_effect ) {
						   InsertSliderWithoutText( XorStr( "##kill effect" ), g_Vars.esp.kill_effect_time, 0.f, 5.f, XorStr( "%.1f seconds" ) );
						}
						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 } InsertEndGroupBoxLeft( XorStr( "Part 1 Cover" ), XorStr( "Part 1" ) );
				  }
				  ImGui::NextColumn( );
				  {
					 InsertGroupBoxRight( XorStr( "Part 2" ), 506.f + 56.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Preserve killFeed" ), g_Vars.esp.preserve_killfeed );
						if ( g_Vars.esp.preserve_killfeed ) {
						   InsertSlider( XorStr( "Time##PreserveKillfeed" ), g_Vars.esp.preserve_killfeed_time, 1.f, 300.f, XorStr( "%.0f ms" ) );
						}

						//InsertCheckbox( XorStr( "Overlay Info" ), g_Vars.misc.overlay_info );

						static bool* bomb_conditions[] = {
						   &g_Vars.esp.draw_c4,
						   &g_Vars.esp.draw_c4_bar,

						};

						const char* bomb_conditions_str[] = {
						   XorStr( "Timer" ), XorStr( "Bar" ),
						};

						InsertMultiCombo( XorStr( "Planted bomb" ),
										  bomb_conditions_str, bomb_conditions, 2 );

						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##c4 Color" ), g_Vars.esp.c4_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						InsertCheckbox( XorStr( "Nades Text" ), g_Vars.esp.nades );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##Nades text Color" ), g_Vars.esp.nades_text_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						InsertCheckbox( XorStr( "Nades Box" ), g_Vars.esp.nades_box );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##Nades box Color" ), g_Vars.esp.nades_box_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						InsertCheckbox( XorStr( "Penetrate crosshair" ), g_Vars.esp.autowall_crosshair );
						if ( g_Vars.esp.autowall_crosshair ) {
						   InsertSlider( XorStr( "Size##iOff" ), g_Vars.esp.autowall_crosshair_height, 1.f, 30.0f, XorStr( "%.0f px" ) );
						}

						//InsertCheckbox( XorStr( "Crosshair" ), g_Vars.esp.crosshair );
						//ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						//ImGui::ColorEdit4( XorStr( "##Crosshair Color" ), g_Vars.esp.crosshair_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						//if ( g_Vars.esp.crosshair ) {
						//   InsertSliderInt( XorStr( "Size##Crosshair" ), g_Vars.esp.crosshair_size, 1.f, 15.f, XorStr( "%d" ) );
						//}
						InsertCheckbox( XorStr( "Dropped weapons" ), g_Vars.esp.dropped_weapons );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##Dropped Weapons Color" ), g_Vars.esp.dropped_weapons_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						const char* weapon_type_str[] = { XorStr( "Off" ), XorStr( "2D" ), XorStr( "3D" ) };
						if ( g_Vars.esp.dropped_weapons ) {
						   InsertComboWithoutText( XorStr( "##Weapons Type" ), g_Vars.esp.dropped_weapons_type, weapon_type_str );
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

						InsertCombo( XorStr( "Skybox " ), g_Vars.esp.sky_changer, SkyNames );
						InsertCheckbox( XorStr( "Spread overlay" ), g_Vars.esp.fov_crosshair );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##Fov Crosshair Color" ), g_Vars.esp.fov_crosshair_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						InsertCheckbox( XorStr( "Hit logger" ), g_Vars.esp.stat_logger );
						InsertCheckbox( XorStr( "Spectator list" ), g_Vars.esp.spectator_list );
						InsertCheckbox( XorStr( "Keybind list" ), g_Vars.esp.keybind_list );
						InsertCheckbox( XorStr( "Indicators list" ), g_Vars.esp.indicators_list );
						InsertCheckbox( XorStr( "Watermark" ), g_Vars.esp.watermark );
						//InsertCheckbox( XorStr( "Show indicators" ), g_Vars.misc.indicator_frame );

						InsertCheckbox( XorStr( "Zoom## priblijenye naxoy" ), g_Vars.esp.zoom );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
						InsertKeyBox( "", g_Vars.esp.zoom_key );
						if ( g_Vars.esp.zoom ) {
						   InsertSlider( XorStr( "Zoom value" ), g_Vars.esp.zoom_value, 0.f, 100.f, XorStr( "%.2f" ) );
						}
						/*InsertCheckbox( XorStr( "Tonemap" ), g_Vars.esp.bloom_effect );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##bloom color" ), g_Vars.esp.bloom_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						InsertSlider( XorStr( "Bloom scale" ), g_Vars.esp.bloom_scale, -1.f, 2500.f, XorStr( "%.2f" ) );
						InsertSlider( XorStr( "Bloom scale min" ), g_Vars.esp.bloom_scale_minimun, -1.f, 2500.f, XorStr( "%.2f" ) );
						InsertSlider( XorStr( "Bloom exponent" ), g_Vars.esp.bloom_exponent, -1.f, 500.f, XorStr( "%.2f" ) );
						InsertSlider( XorStr( "Bloom saturation" ), g_Vars.esp.bloom_saturation, -1.f, 500.f, XorStr( "%.2f" ) );
						InsertSlider( XorStr( "Percent Target" ), g_Vars.esp.tonemap_percent_target, -1.f, 500.f, XorStr( "%.2f" ) );
						InsertSlider( XorStr( "Percent Bright Pixels" ), g_Vars.esp.tonemap_percent_bright_pixels, -1.f, 500.f, XorStr( "%.2f" ) );
						InsertSlider( XorStr( "Min avg lum" ), g_Vars.esp.tonemap_min_avg_lum, -1.f, 500.f, XorStr( "%.2f" ) );
						InsertSlider( XorStr( "Rate" ), g_Vars.esp.tonemap_rate, -1.f, 500.f, XorStr( "%.2f" ) );
						InsertSlider( XorStr( "Exposure scale" ), g_Vars.esp.exposure_scale, -1.f, 2000.f, XorStr( "%.2f" ) );
						InsertSliderInt( XorStr( "Model brightness" ), g_Vars.esp.model_brightness, 0, 1000, XorStr( "%d" ) );*/

						static bool* explosive_conditions[] = {
						   &g_Vars.esp.molotov_indicator,
						   &g_Vars.esp.smoke_indicator,
						};

						const char* explosive_conditions_str[] = {
						   XorStr( "Molotov" ), XorStr( "Smoke" )
						};

						InsertMultiCombo( XorStr( "Exsplosive" ),
										  explosive_conditions_str, explosive_conditions, 2 );

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 } InsertEndGroupBoxRight( XorStr( "Part 2 Cover" ), XorStr( "Part 2" ) );
				  }
			   }
			   break;
			   case SKINCHANGER_TAB:
			   {
				  InsertSpacer( XorStr( "Top Spacer" ) );
				  ImGui::Columns( 2, NULL, false );
				  {
					 InsertGroupBoxLeft( XorStr( "Knife options" ), 338.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Override knives##knife" ), g_Vars.m_global_skin_changer.m_knife_changer );

						ImGui::PushItemWidth( ImGui::GetContentRegionAvailWidth( ) - style->WindowPadding.x );

						if ( k_knife_names.at( g_Vars.m_global_skin_changer.m_knife_vector_idx ).definition_index != g_Vars.m_global_skin_changer.m_knife_idx ) {
						   auto it = std::find_if( k_knife_names.begin( ), k_knife_names.end( ), [&] ( const WeaponName_t& a ) {
							  return a.definition_index == g_Vars.m_global_skin_changer.m_knife_idx;
						   } );

						   if ( it != k_knife_names.end( ) )
							  g_Vars.m_global_skin_changer.m_knife_vector_idx = std::distance( k_knife_names.begin( ), it );
						}

						ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
						ImGui::ListBox(
						   XorStr( "##knives" ), &g_Vars.m_global_skin_changer.m_knife_vector_idx,
						   [] ( void* data, int32_t idx, const char** out_text ) {
						   auto vec = reinterpret_cast< std::vector< WeaponName_t >* >( data );
						   *out_text = vec->at( idx ).name;
						   return true;
						},
						   ( void* ) ( &k_knife_names ), k_knife_names.size( ), 21 );
						ImGui::PopItemWidth( ); ImGui::CustomSpacing( 1.f );

						g_Vars.m_global_skin_changer.m_knife_idx = k_knife_names[ g_Vars.m_global_skin_changer.m_knife_vector_idx ].definition_index;

						ImGui::PopItemWidth( );

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );

					 } InsertEndGroupBoxLeft( XorStr( "Knife options Cover" ), XorStr( "Knife options" ) );

					 InsertSpacer( XorStr( "Skins - Other Spacer" ) );

					 InsertGroupBoxLeft( XorStr( "Glove options" ), 150.f + 56.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Override gloves##glove" ), g_Vars.m_global_skin_changer.m_glove_changer );

						ImGui::PushItemWidth( ImGui::GetContentRegionAvailWidth( ) - style->WindowPadding.x );

						if ( k_glove_names.at( g_Vars.m_global_skin_changer.m_gloves_vector_idx ).definition_index != g_Vars.m_global_skin_changer.m_gloves_idx ) {
						   auto it = std::find_if( k_glove_names.begin( ), k_glove_names.end( ), [&] ( const WeaponName_t& a ) {
							  return a.definition_index == g_Vars.m_global_skin_changer.m_gloves_idx;
						   } );

						   if ( it != k_glove_names.end( ) )
							  g_Vars.m_global_skin_changer.m_gloves_vector_idx = std::distance( k_glove_names.begin( ), it );
						}

						ImGui::Spacing( ); ImGui::NewLine( );  ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
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
						ImGui::PopItemWidth( ); ImGui::CustomSpacing( 1.f );

						g_Vars.m_global_skin_changer.m_gloves_idx = k_glove_names[ g_Vars.m_global_skin_changer.m_gloves_vector_idx ].definition_index;

						ImGui::PopItemWidth( );

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );

					 } InsertEndGroupBoxLeft( XorStr( "Glove options Cover" ), XorStr( "Glove options" ) );
				  }
				  ImGui::NextColumn( );
				  {
					 InsertGroupBoxRight( XorStr( "Skin" ), 506.f + 56.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Enabled##Skins" ), g_Vars.m_global_skin_changer.m_active );

						static int weapon_id = 0;

						ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
						if ( ImGui::Combo(
						   XorStr( "##skins_weapons" ), &weapon_id,
						   [] ( void* data, int32_t idx, const char** out_text ) {
						   auto vec = reinterpret_cast< std::vector< item_skins >* >( data );
						   *out_text = vec->at( idx ).display_name.c_str( );
						   return true;
						}, ( void* ) ( &weapon_skins ), weapon_skins.size( ), 30 ) ) {

						}
						ImGui::PopItemWidth( ); ImGui::CustomSpacing( 1.f );

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

						ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
						if ( ImGui::ListBox(
						   XorStr( "##paint_kits" ), &skin->m_paint_kit_index,
						   [] ( void* data, int32_t idx, const char** out_text ) {
						   auto vec = reinterpret_cast< std::vector< paint_kit >* >( data );
						   *out_text = vec->at( idx ).name.c_str( );
						   return true;
						},
						   ( void* ) ( &current_weapon.m_kits ), current_weapon.m_kits.size( ), 36 ) ) {
						   g_Vars.m_global_skin_changer.m_update_skins = true;
						   if ( current_weapon.glove )
							  g_Vars.m_global_skin_changer.m_update_gloves = true;
						}
						ImGui::PopItemWidth( ); ImGui::CustomSpacing( 1.f );

						skin->m_paint_kit = current_weapon.m_kits[ skin->m_paint_kit_index ].id;

						skin->m_enabled = true;
						// InsertCheckbox( XorStr( "On" ), skin->m_enabled );
					 #if 1
						ImGui::NewLine( );
						InsertSlider( XorStr( "Quality" ), skin->m_wear, 0.00000001f, 1.00f, XorStr( "%.8f %%" ) );
						InsertSliderInt( XorStr( "Seed" ), skin->m_seed, 1, 1000, XorStr( "%d" ) );
						InsertSliderInt( XorStr( "Stat-trak" ), skin->m_stat_trak, 1, 1000000, XorStr( "%d" ) );
						static char buffer[ 128 ] = { ( '\0' ) };

						//ImGui::InputText( XorStr( "Name" ), buffer, 128 );

						//ImGui::PopItemWidth( );

						auto size = ImGui::GetContentRegionAvail( );
						/*ImGui::NewLine( );
						if ( ImGui::Button( XorStr( "Update skins" ), ImVec2( size.x - style->FramePadding.x, 40 ) ) ) {
						   g_Vars.m_global_skin_changer.m_update_skins = true;
						}*/
					 #endif

						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );
					 } InsertEndGroupBoxRight( XorStr( "Skin Cover" ), XorStr( "Skin" ) );
				  }
			   }
			   break;
			   case MISC_TAB:
			   {
				  InsertSpacer( XorStr( "Top Spacer" ) );

				  ImGui::Columns( 2, NULL, false );
				  {
					 InsertGroupBoxLeft( XorStr( "Miscellaneous" ), 506.f + 56.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						InsertCheckbox( XorStr( "Enabled##Misc" ), g_Vars.misc.active );
						if ( g_Vars.misc.active ) {
						   InsertCheckbox( XorStr( "Auto jump" ), g_Vars.misc.autojump );
						   InsertCheckbox( XorStr( "Auto strafe" ), g_Vars.misc.autostrafer );
						   InsertCheckbox( XorStr( "Directional strafe" ), g_Vars.misc.autostrafer_wasd );
						   if ( g_Vars.misc.autostrafer_wasd ) {
							  InsertSlider( XorStr( "Turn smoothness" ), g_Vars.misc.autostrafer_retrack, 0.0f, 8.0f, XorStr( "%.0f units" ) );
						   }
						   InsertCheckbox( XorStr( "Bypass sv_pure" ), g_Vars.misc.sv_pure_bypass );
						   InsertCheckbox( XorStr( "Fast duck" ), g_Vars.misc.fastduck );
						   InsertCheckbox( XorStr( "Fake duck" ), g_Vars.misc.fakeduck );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
						   InsertKeyBox( "", g_Vars.misc.fakeduck_bind );
						  // if ( g_Vars.misc.fakeduck ) {
							//  InsertSliderInt( XorStr( "Fake duck ticks##fakeduck_ticks" ), g_Vars.misc.fakeduck_ticks, 8, 15, XorStr( "%d ticks" ) );
						  // }
						   InsertCheckbox( XorStr( "Mini jump" ), g_Vars.misc.minijump );
						   InsertCheckbox( XorStr( "Edge jump" ), g_Vars.misc.edgejump );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
						   InsertKeyBox( "", g_Vars.misc.edgejump_bind );
						   InsertCheckbox( XorStr( "Money revealer" ), g_Vars.misc.money_revealer );
						   InsertCheckbox( XorStr( "Auto accept" ), g_Vars.misc.auto_accept );
						   InsertCheckbox( XorStr( "Duck jump" ), g_Vars.misc.duckjump );
						   InsertCheckbox( XorStr( "Quick stop" ), g_Vars.misc.quickstop );
						   InsertCheckbox( XorStr( "Accurate walk" ), g_Vars.misc.accurate_walk );
						   InsertCheckbox( XorStr( "Slide walk" ), g_Vars.misc.slide_walk );
						   InsertCheckbox( XorStr( "Slow walk" ), g_Vars.misc.slow_walk );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
						   InsertKeyBox( "", g_Vars.misc.slow_walk_bind );
						   if ( g_Vars.misc.slow_walk ) {
							  InsertSlider( XorStr( "Speed##slow_walk_speed" ), g_Vars.misc.slow_walk_speed, 1.0f, 100.0f, XorStr( "%.0f %%" ) );
						   }

						   InsertCheckbox( XorStr( "Auto peek" ), g_Vars.misc.autopeek );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
						   InsertKeyBoxDefCond( "", g_Vars.misc.autopeek_bind, KeyBindType::HOLD );
						   if ( g_Vars.misc.autopeek ) {
							  InsertCheckbox( XorStr( "Auto peek visualise" ), g_Vars.misc.autopeek_visualise );
							  ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) - 11.f );
							  ImGui::ColorEdit4( XorStr( "##Auto peek color" ), g_Vars.misc.autopeek_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						   }

						   InsertCheckbox( XorStr( "Knifebot" ), g_Vars.misc.knife_bot );
						   const char* knife_bot_str[] = { XorStr( "Default" ), XorStr( "Backstab" ), XorStr( "Quick" ) };
						   if ( g_Vars.misc.knife_bot )
							  InsertComboWithoutText( XorStr( "##Knife bot type" ), g_Vars.misc.knife_bot_type, knife_bot_str );

						   InsertCheckbox( XorStr( "Extended bactkrack" ), g_Vars.misc.extended_backtrack );
						   if ( g_Vars.misc.extended_backtrack ) {
							  InsertSlider( XorStr( "Extend window" ), g_Vars.misc.extended_backtrack_time, 50.0f, 200.0f, XorStr( "%.0f ms" ) );
						   }

						   InsertCheckbox( XorStr( "Client impacts" ), g_Vars.misc.impacts_spoof );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Client impacts color" ), g_Vars.esp.client_impacts, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						   InsertCheckbox( XorStr( "Server impacts" ), g_Vars.misc.server_impacts_spoof );
						   ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						   ImGui::ColorEdit4( XorStr( "##Server impacts color" ), g_Vars.esp.server_impacts, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );

						   //InsertCheckbox( XorStr( "Unregistered impacts" ), g_Vars.misc.server_issue_impacts_spoof );

						   g_Vars.misc.chat_spammer = false;
						   // InsertCheckbox( XorStr( "Chat spammer" ), g_Vars.misc.chat_spammer );

						   const char* clantags[] = {
							  XorStr( "Static" ),
							  XorStr( "Animated" ),
						   };

						   const char* models_to_change[] = {
							   XorStr( "Default" ),
							   XorStr( "tm_balkan_varianta.mdl" ),
							   XorStr( "tm_balkan_variantb.mdl" ),
							   XorStr( "tm_balkan_variantc.mdl" ),
							   XorStr( "tm_balkan_variantd.mdl" ),
							   XorStr( "ctm_fbi_variantb.mdl" ),
							   XorStr( "ctm_fbi_variantf.mdl" ),
							   XorStr( "ctm_fbi_variantg.mdl" ),
							   XorStr( "ctm_fbi_varianth.mdl" ),
							   XorStr( "ctm_heavy.mdl" ),
							   XorStr( "ctm_st6.mdl" ),
							   XorStr( "ctm_st6_varianta.mdl" ),
							   XorStr( "ctm_st6_variantb.mdl" ),
							   XorStr( "ctm_st6_variantc.mdl" ),
							   XorStr( "ctm_st6_variantd.mdl" ),
							   XorStr( "tm_balkan_variantg.mdl" ),
							   XorStr( "tm_balkan_varianth.mdl" ),
							   XorStr( "tm_balkan_varianti.mdl" ),
							   XorStr( "tm_balkan_variantj.mdl" ),
							   XorStr( "tm_phoenix_variantf.mdl" ),
							   XorStr( "tm_phoenix_variantg.mdl" ),
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

						   InsertMultiCombo( XorStr( "Indicators" ),
											 indicator_conditions_str, indicator_conditions, 6 );

						   InsertCheckbox( XorStr( "Models skin" ), g_Vars.misc.skins_model );
						   if ( g_Vars.misc.skins_model ) {
							  InsertCombo( XorStr( "CT Local Model" ), g_Vars.misc.tt_model_type, models_to_change );
							  InsertCombo( XorStr( "TT Local Model" ), g_Vars.misc.ct_model_type, models_to_change );
						   }


						   InsertCheckbox( XorStr( "Clan-tag" ), g_Vars.misc.clantag_changer );
						   InsertCheckbox( XorStr( "Name changer" ), g_Vars.misc.name_changer );

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
								 *( int* ) ( ( uintptr_t ) & cName->fnChangeCallback + 0xC ) = 0;
								 cName->SetValue( buffer2 );
							  }
						   }
						}
						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );

					 } InsertEndGroupBoxLeft( XorStr( "Miscellaneous Cover" ), XorStr( "Miscellaneous" ) );

				  }
				  ImGui::NextColumn( );
				  {
					 InsertGroupBoxRight( XorStr( "Settings" ), 156.f + 56.0f );
					 {
						style->ItemSpacing = ImVec2( 4, 2 );
						style->WindowPadding = ImVec2( 4, 4 );
						ImGui::CustomSpacing( 9.f );

						//ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(19.f); ImGui::Text( XorStr( "Menu ascent" ) );

						ImGui::NewLine( ); ImGui::SameLine( 47.f ); ImGui::Text( XorStr( "Menu accent" ) ); ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) * 2.0f );
						//ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) - ImGui::GetFrameHeight( ) );
						ImGui::ColorEdit4( XorStr( "##menu ascent" ), g_Vars.misc.menu_ascent, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
						InsertCheckbox( XorStr( "Anti Untrusted" ), g_Vars.misc.anti_untrusted );
						InsertCheckbox( XorStr( "Obs bypass" ), g_Vars.misc.obs_bypass );
						InsertCheckbox( XorStr( "Third person" ), g_Vars.misc.third_person );
						ImGui::SameLine( ImGui::GetContentRegionAvailWidth( ) );
						InsertKeyBox( "", g_Vars.misc.third_person_bind );
						if ( g_Vars.misc.third_person ) {
						   InsertCheckbox( XorStr( "Disable on grenade" ), g_Vars.misc.off_third_person_on_grenade );
						   InsertSlider( XorStr( "Distance " ), g_Vars.misc.third_person_dist, 0, 250, XorStr( "%.0f units" ) );
						}

						InsertSlider( XorStr( "View world FOV " ), g_Vars.esp.world_fov, 0.f, 200.f, XorStr( "%.0f degress" ) );
						InsertCheckbox( XorStr( "View model changer" ), g_Vars.misc.viewmodel_change );

						if ( g_Vars.misc.viewmodel_change ) {
						   InsertSlider( XorStr( "View model FOV " ), g_Vars.misc.viewmodel_fov, 0.f, 200.f, XorStr( "%.0f degress" ) );
						   InsertSlider( XorStr( "View model x" ), g_Vars.misc.viewmodel_x, -20.f, 20.f, XorStr( "%.0f" ) );
						   InsertSlider( XorStr( "View model y" ), g_Vars.misc.viewmodel_y, -20.f, 20.f, XorStr( "%.0f" ) );
						   InsertSlider( XorStr( "View model z" ), g_Vars.misc.viewmodel_z, -20.f, 20.f, XorStr( "%.0f" ) );
						}
						InsertCheckbox( XorStr( "Aspect ratio" ), g_Vars.esp.aspect_ratio );
						if ( g_Vars.esp.aspect_ratio ) {
						   InsertSlider( XorStr( "Aspect ratio value" ), g_Vars.esp.aspect_ratio_value, 0.02f, 5.f, XorStr( "%.2f" ) );
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

						InsertCheckbox( XorStr( "Autobuy" ), g_Vars.misc.autobuy_enabled );
						if ( g_Vars.misc.autobuy_enabled ) {
						   InsertCombo( XorStr( "First Weapon" ), g_Vars.misc.autobuy_first_weapon, first_weapon_str );
						   InsertCombo( XorStr( "Second Weapon" ), g_Vars.misc.autobuy_second_weapon, second_weapon_str );

						   InsertMultiCombo( XorStr( "Other" ),
											 other_weapon_str, other_weapon_conditions, 8 );
						}


						style->ItemSpacing = ImVec2( 0, 0 );
						style->WindowPadding = ImVec2( 6, 6 );

					 } InsertEndGroupBoxRight( XorStr( "Settings Cover" ), XorStr( "Settings" ) );

					 InsertSpacer( XorStr( "Settings - Other Spacer" ) );

					 InsertGroupBoxRight( XorStr( "Other" ), 332.f );
					 {
						ImGui::CustomSpacing( 9.f );

						ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( );
						ImGui::SameLine( 45.f );
						ImGui::InputText( "", config_name, 256 );
						ImGui::NewLine( );
						ImGui::SameLine( 45.f );
						ImGui::ListBox( "", &voted_cfg, cfg_list );
						style->ItemSpacing = ImVec2( 4, 2 );
						ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( );
						ImGui::SameLine( 45.f );

						style->FrameBorderSize = 1.f;

						//    ImGui::Button( XorStr( "Update configs" ), ImVec2( 167.f, 25 ) )
						static bool FirstTime = true;
						if ( FirstTime || ( ImGui::GetFrameCount( ) % 100 ) == 0 ) {
						   cfg_list = ConfigManager::GetConfigs( );
						   FirstTime = false;
						}

						if ( ImGui::Button( XorStr( "Open folder" ), ImVec2( 167.f, 25 ) ) ) {
						   ConfigManager::OpenConfigFolder( );
						}

						ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( );
						ImGui::SameLine( 45.f );

						if ( ImGui::Button( XorStr( "Create config" ), ImVec2( 167.f, 25 ) ) ) {
						   ConfigManager::CreateConfig( config_name );
						   std::string name( config_name );
						   std::string str = XorStr( "Config " ) + name + XorStr( " created!" );

						   ILoggerEvent::Get( )->PushEvent( str.c_str( ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
						   cfg_list = ConfigManager::GetConfigs( );
						}

						ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( );
						ImGui::SameLine( 45.f );

						if ( ImGui::Button( XorStr( "Load config" ), ImVec2( 167.f, 25 ) ) && cfg_list.size( ) > 0 ) {
						   ConfigManager::LoadConfig( cfg_list.at( voted_cfg ) );
						   ILoggerEvent::Get( )->PushEvent( XorStr( "Config loaded!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
						   g_Vars.m_global_skin_changer.m_update_skins = true;
						   g_Vars.m_global_skin_changer.m_update_gloves = true;
						}

						ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( );
						ImGui::SameLine( 45.f );

						if ( ImGui::Button( XorStr( "Save config" ), ImVec2( 167.f, 25 ) ) && cfg_list.size( ) > 0 ) {
						   ConfigManager::SaveConfig( cfg_list.at( voted_cfg ) );
						   ILoggerEvent::Get( )->PushEvent( XorStr( "Config saved!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
						}

						ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( );
						ImGui::SameLine( 45.f );

						if ( ImGui::Button( XorStr( "Delete config" ), ImVec2( 167.f, 25 ) ) && cfg_list.size( ) > 0 ) {
						   ConfigManager::RemoveConfig( cfg_list.at( voted_cfg ) );
						   cfg_list = ConfigManager::GetConfigs( );
						   ILoggerEvent::Get( )->PushEvent( XorStr( "Config deleted!" ), FloatColor( 1.f, 1.f, 1.f, 1.f ) );
						}

						ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( );
						ImGui::SameLine( 45.f );

						if ( ImGui::Button( XorStr( "Unload" ), ImVec2( 167.f, 25 ) ) && cfg_list.size( ) > 0 ) {
						   g_Vars.globals.hackUnload = true;
						}

						style->FrameBorderSize = 0.f;
						style->ItemSpacing = ImVec2( 0, 0 );
					 } InsertEndGroupBoxRight( XorStr( "Other Cover" ), XorStr( "Other" ) );
				  }
			   }
			   break;
			   default:
			   break;
			}

			style->Colors[ ImGuiCol_Border ] = ImColor( 10, 10, 10, 255 );

		 } ImGui::EndChild( );

		 style->ItemSpacing = ImVec2( 4.f, 4.f );
		 style->Colors[ ImGuiCol_ChildBg ] = ImColor( 17, 17, 17, 255 );

	  } ImGui::EndChild( );

	  ImGui::End( );
   }

   style->Alpha = meme;

#ifdef _DEBUG
   ImGui::ShowStyleEditor( );
#endif

   ImGui::PopFont( );
}

void CMenu::Initialize( IDirect3DDevice9* pDevice ) {
   ImGuiStyle& style = ImGui::GetStyle( );

   style.ScrollbarSize = 6.0f;
   style.ItemSpacing = ImVec2( 5, 4 );
   style.ItemInnerSpacing = ImVec2( 8, 8 );
   style.TouchExtraPadding = ImVec2( 0, 0 );
   style.FramePadding = ImVec2( 5, 4 );
   style.WindowRounding = 0.0f;
   style.FrameRounding = 0.0f;
   style.IndentSpacing = 20.0f;
   style.GrabMinSize = 0.0f;

   /*  style.Alpha = 1.0f;
   style.WindowPadding = ImVec2( 10, 10 );
   style.WindowMinSize = ImVec2( 32, 32 );
   style.WindowRounding = 0.0f;
   style.WindowTitleAlign = ImVec2( 0.0f, 0.5f );
   style.FramePadding = ImVec2( 5, 1 );
   style.FrameRounding = 2.0f;


   style.ColumnsMinSpacing = 0.0f;
   style.ScrollbarSize = 6.0f;
   style.ScrollbarRounding = 0.0f;

   style.GrabRounding = 2.0f;
   style.ButtonTextAlign = ImVec2( 0.5f, 0.5f );
   style.DisplayWindowPadding = ImVec2( 22, 22 );
   style.DisplaySafeAreaPadding = ImVec2( 200, 150 );
   style.AntiAliasedLines = true;
   style.CurveTessellationTol = 1.f;
   style.FrameBorderSize = 0.0f;
   style.ChildBorderSize = 1.0f;*/

#if 0
   ImVec4 element_color_active = ImVec4( 0.1f, 0.46f, 0.82f, 1.f );
   ImVec4 element_color = ImVec4( 0.08f, 0.40f, 0.75f, 1.f );

   ImVec4* colors = ImGui::GetStyle( ).Colors;
   colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
   colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
   colors[ ImGuiCol_WindowBg ] = ImVec4( 0.13f, 0.12f, 0.16f, 1.0f );
   colors[ ImGuiCol_ChildBg ] = ImVec4( 0.16f, 0.15f, 0.19f, 1.00f );
   colors[ ImGuiCol_PopupBg ] = ImVec4( 0.08f, 0.08f, 0.08f, 0.94f );
   colors[ ImGuiCol_Border ] = ImVec4( 0.43f, 0.43f, 0.50f, 0.50f );
   colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
   colors[ ImGuiCol_FrameBg ] = ImVec4( 0.25f, 0.27f, 0.28f, 0.54f );
   colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.45f, 0.46f, 0.47f, 0.40f );
   colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.19f, 0.19f, 0.20f, 0.67f );
   colors[ ImGuiCol_TitleBg ] = ImVec4( 0.04f, 0.04f, 0.04f, 1.00f );
   colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.29f, 0.29f, 0.29f, 1.00f );
   colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
   colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
   colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
   colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.31f, 1.00f );
   colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
   colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
   colors[ ImGuiCol_CheckMark ] = ImVec4( 0.94f, 0.94f, 0.94f, 1.00f );
   colors[ ImGuiCol_SliderGrab ] = element_color;
   colors[ ImGuiCol_SliderGrabActive ] = element_color_active;
   colors[ ImGuiCol_Button ] = ImVec4( 0.13f, 0.12f, 0.16f, 1.0f );
   colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.16f, 0.15f, 0.19f, 1.00f );
   colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.16f, 0.15f, 0.19f, 1.00f );
   colors[ ImGuiCol_Header ] = element_color;
   colors[ ImGuiCol_HeaderHovered ] = element_color_active;
   colors[ ImGuiCol_HeaderActive ] = element_color_active;
   colors[ ImGuiCol_Separator ] = ImVec4( 0.43f, 0.43f, 0.50f, 0.50f );
   colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.72f, 0.72f, 0.72f, 0.78f );
   colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
   colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.91f, 0.91f, 0.91f, 0.25f );
   colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.81f, 0.81f, 0.81f, 0.67f );
   colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.46f, 0.46f, 0.46f, 0.95f );
   colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
   colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
   colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.73f, 0.60f, 0.15f, 1.00f );
   colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
   colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.87f, 0.87f, 0.87f, 0.35f );
   //colors[ ImGuiCol_ModalWindowDarkening ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );
   colors[ ImGuiCol_DragDropTarget ] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
   colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );
   colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
#endif

   ImGui::StyleColorsDark( &ImGui::GetStyle( ) );

   using namespace ImGui;

   hack_font = menuFont;

#if 0
   ImFontConfig * cfg = new ImFontConfig( );
   cfg->OversampleH = cfg->OversampleV = 1;
   cfg->PixelSnapH = true;
   hack_font = ImGui::GetIO( ).Fonts->AddFontFromFileTTF( XorStr( "C:\\Windows\\Fonts\\Verdana.ttf" ), 13.0f,
														  cfg, ImGui::GetIO( ).Fonts->GetGlyphRangesCyrillic( ) );
#endif

   if ( menuBg == nullptr ) {
	  D3DXCreateTextureFromFileInMemoryEx( pDevice,
										   &menuBackground, sizeof( menuBackground ),
										   1920, 1080, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &menuBg );
   }

   /*font_tabs = font_main_caps = ImGui::GetIO( ).Fonts->AddFontFromFileTTF(
   XorStr( "C:\\Windows\\Fonts\\Raleway-Regular.ttf" ), 16.0f );

   font_main = ImGui::GetIO( ).Fonts->AddFontFromFileTTF( XorStr( "C:\\Windows\\Fonts\\Tahoma.ttf" ), 18 );
   font_menu = ImGui::GetIO( ).Fonts->AddFontFromFileTTF( XorStr( "C:\\Windows\\Fonts\\Verdana.ttf" ), 12 );
   font_main_caps = ImGui::GetIO( ).Fonts->AddFontFromFileTTF(
   XorStr( "C:\\Windows\\Fonts\\Raleway-Regular.ttf" ), 24.0f );*/
}

void CMenu::WndProcHandler( ) {
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

IMenu* IMenu::Get( ) {
   static CMenu instance;
   return &instance;
}
#endif