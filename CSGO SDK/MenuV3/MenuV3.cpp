#if 1
#include "MenuV3.hpp"
#include "blur.hpp"
#include "custom.hpp"
#include "../source.hpp"
#include "../player.hpp"
#include "../weapon.hpp"
#include "../Displacement.hpp"
#include "../KitParser.hpp"
#include "../RoundFireBulletsStore.hpp"
#include "../InputSys.hpp"

IDirect3DTexture9* background = { nullptr };
IDirect3DTexture9* weather = { nullptr };

#define ALPHA    ( ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )
#define NO_ALPHA ( ImGuiColorEditFlags_NoTooltip    | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )

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

#define LBUTTON        0x01
#define RBUTTON        0x02
#define MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */
#define XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#define XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */
#define BACK           0x08
#define RMENU          0xA5

namespace ImGui {
 
    std::string GetNameFromCode( int code ) {
        auto it = std::find_if( ButtonNames.begin( ), ButtonNames.end( ), [ code ]( const KeyBindPair_t& a ) { return a.first == code; } );

        std::string name;
        if ( it == ButtonNames.end( ) )
            name = XorStr( "none" );
        else
            name = it->second;

        return name;
    }

    // Getter for the old Combo() API: const char*[]
    static bool items_getter( void* data, int idx, const char** out_text ) {
        const char* const* items = ( const char* const* )data;
        if ( out_text )
            *out_text = items[ idx ];
        return true;
    }

    bool HotkeyA( const char* label, KeyBind_t* k, int defaultCondition = -1 ) {

        ImGuiWindow* window = GetCurrentWindow( );
        if ( window->SkipItems )
            return false;

        ImGuiContext& g = *GImGui;
        ImGuiIO& io = g.IO;
        const ImGuiStyle& style = g.Style;

        const float w = GetWindowWidth( ) - 25;

        static bool key_input = false;
        static bool cond_input = false;
        static KeyBind_t* key_output = nullptr;

        std::string text = XorStr( "none" );

        if ( !cond_input && key_input && key_output == k ) {
            text = XorStr( "..." );

            static const int mouse_code_array[ 5 ] = {
               1,
               2,
               4,
               5,
               6,
            };

            for ( int i = 0; i < 5; i++ ) {
                if ( IsMouseReleased( ImGuiKey( i ) ) ) {
                    key_input = false;
                    k->key = mouse_code_array[ i ];
                    break;
                }
            }

            for ( auto keyPair : ButtonNames ) {
                if ( IsKeyReleased( keyPair.first ) ) {
                    key_input = false;
                    k->key = keyPair.first;
                    if ( k->key == VirtualKeys::Escape )
                        k->key = 0;

                    break;
                }
            }
        } else {
            auto name = GetNameFromCode( k->key );
            text = name;
            std::transform( text.begin( ), text.end( ), text.begin( ), ::tolower );
        }

        const ImGuiID id = window->GetID( label );
        const ImVec2 label_size = CalcTextSize( label, 0, 1 );

        const ImVec2 size = ImVec2( CalcTextSize( text.c_str( ) ).x + 10, 20 );
        const ImRect frame_bb( window->DC.CursorPos + ImVec2( w - size.x, 0 ), window->DC.CursorPos + ImVec2( w, size.y ) );
        const ImRect total_bb( window->DC.CursorPos, frame_bb.Max );

        ItemSize( total_bb, style.FramePadding.y );
        if ( !ItemAdd( frame_bb, id ) )
            return false;

        const bool hovered = IsItemHovered( );

        if ( hovered )
            SetHoveredID( id );

        const bool user_clicked = hovered && io.MouseClicked[ 0 ];

        if ( user_clicked ) {
            if ( g.ActiveId != id ) {
                // Start edition
                memset( io.MouseDown, 0, sizeof( io.MouseDown ) );
                memset( io.KeysDown, 0, sizeof( io.KeysDown ) );
                k->key = 0;
            }
            SetActiveID( id, window );
            FocusWindow( window );
        } else if ( io.MouseClicked[ 0 ] ) {
            // Release focus when we click outside
            if ( g.ActiveId == id )
                ClearActiveID( );
        }

        bool value_changed = false;
        int key = k->key;

        if ( g.ActiveId == id ) {
            for ( auto i = 0; i < 5; i++ ) {
                if ( io.MouseDown[ i ] ) {
                    switch ( i ) {
                        case 0:
                            key = LBUTTON;
                            break;
                        case 1:
                            key = RBUTTON;
                            break;
                        case 2:
                            key = MBUTTON;
                            break;
                        case 3:
                            key = XBUTTON1;
                            break;
                        case 4:
                            key = XBUTTON2;
                            break;
                    }
                    value_changed = true;
                    ClearActiveID( );
                }
            }
            if ( !value_changed ) {
                for ( auto i = BACK; i <= RMENU; i++ ) {
                    if ( io.KeysDown[ i ] ) {
                        key = i;
                        value_changed = true;
                        ClearActiveID( );
                    }
                }
            }

            if ( IsKeyPressedMap( ImGuiKey_Escape ) ) {
                k->key = -1;//0;
                ClearActiveID( );
            } else {
                k->key = key;
            }
        }

        // Render
        std::string buf_display = "None";

        window->DrawList->AddRect( frame_bb.Min, frame_bb.Max, ImColor( 1.f, 1.f, 1.f, 0.05f * style.Alpha ), 2 );

        if ( k->key != 0 && g.ActiveId != id ) {

            buf_display = text.c_str();
        } else if ( g.ActiveId == id ) {

            buf_display = "...";
        }

        RenderTextClipped( frame_bb.Min, frame_bb.Max, buf_display.c_str( ), NULL, NULL, ImVec2( 0.5f, 0.5f ) );

        if ( label_size.x > 0 )
            RenderText( { total_bb.Min.x, frame_bb.GetCenter( ).y - label_size.y / 2 }, label );

        if ( defaultCondition == -1 && cond_input && key_output == k ) {
            if ( GetAsyncKeyState( VK_F2 ) ) {
                printf( std::to_string( k->cond ).c_str( ) );
            }

            ImGui::SetNextWindowSize( ImVec2( 70.0f, 0.0f ) );
            if ( ImGui::BeginPopup( XorStr( "##key_bind_cond" ), ImGuiWindowFlags_AlwaysAutoResize ) ) {
                const char* current_items[ ] = {
                   XorStr( "Hold" ),
                   XorStr( "Toggle" ),
                   XorStr( "Always On" )
                };

                for ( int i = 0; i < 3; i++ ) {
                    if ( Selectable( current_items[ i ], k->cond == i, ImGuiSelectableFlags_DontClosePopups ) ) {


                        if ( k->cond == KeyBindType::ALWAYS_ON )
                            k->enabled = false; // always on  

                        k->cond = i;
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

                key_output = k;
                key_input = false;
                cond_input = true;
                MarkItemEdited( id );
            } 
            else {
                bool hovered, held;
                bool pressed = user_clicked;// ButtonBehavior( frame_bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnRelease );

                if ( pressed ) {
                    MarkItemEdited( id );

                    cond_input = false;
                    key_input = true;
                    key_output = k;
                }
            }
        }


        return false;
    }


    void MultiComboA( const char* label, bool* combos[ ], const char* items[ ], int items_count ) {
        std::vector<std::string> vec;

        char preview_value[ 128 ] = { '\0' };
        bool first_time = false;
        for ( int i = 0; i < items_count; ++i ) {
            if ( *combos[ i ] ) {
                auto len = strlen( preview_value );
                auto preview_len = strlen( items[ i ] );
                if ( len + preview_len + 2 >= 128 )
                    break;

                sprintf( preview_value + len, !first_time ? XorStr( "%s" ) : XorStr( ", %s" ), items[ i ] );
                first_time = true;
            }
        }

        if ( !first_time )
            strcpy_s( preview_value, "..." );

        bool value_changed = false;

        if ( BeginCombo( label, preview_value ) ) {
            PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );
            for ( int i = 0; i < items_count; i++ ) {
                PushID( ( void* )( intptr_t )i );
                const bool item_selected = *combos[ i ];
                const char* item_text;
                if ( !items_getter( ( void* )items, i, &item_text ) )
                    item_text = XorStr( "*Unknown item*" );

                if ( Selectable( item_text, item_selected, ImGuiSelectableFlags_DontClosePopups ) ) {
                    value_changed = true;
                    *combos[ i ] = !( *combos[ i ] );
                }

                if ( item_selected )
                    SetItemDefaultFocus( );

                PopID( );
            }
            PopStyleVar( );
            EndCombo( );
        }
    }

}

namespace nem {
	void c_menu::render( bool is_opened, IDirect3DDevice9* const device ) {
        if ( GetAsyncKeyState( VK_DELETE ) ) {
            g_Vars.globals.hackUnload = true;
        }

        auto& io = ImGui::GetIO( );
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        static bool  opened = false;
        static float alpha = 0.f, speed = 0.1f;

        alpha = ImLerp( alpha, opened ? 1.f : 0.f, 1.f * speed );

        if ( GetAsyncKeyState( VK_INSERT ) & 1 )
            opened = !opened;

        if ( InputSys::Get( )->WasKeyPressed( VK_INSERT ) ) {
            g_Vars.globals.menuOpen = !g_Vars.globals.menuOpen;
        }

        static bool checkbox[ 50 ];
        static bool multi[ 5 ];
        static int  slider = 0, combo = 0, combo_n = 0, key = 0, mode = 0;
        vector < const char* > combo_items = { "Item 1", "Item 2", "Item 3" };
        vector < const char* > multi_items = { "Item 1", "Item 2", "Item 3", "Item 4", "Item 5" };
        vector < const char* > combo_n_items = { "Enemy", "Friendly" };
        static vector < const char* > modes = { "Toggle", "Hold" };
        static char buf[ 64 ];
        static float color[ 4 ] = { 1.f, 1.f, 1.f, 1.f };

        static bool enableffi = false;
        static int  cur_lua = 0;
        static vector < const char* > lua_list = { "Weather", "Lua 1", "Lua 2", "Lua 3", "Lua 4", "Lua 5", "Lua 6", "Lua 7", "Lua 8", "Lua 9", "Lua 10", "Lua 11", "Lua 12" };

        static st_lua arr[ 50 ];

        BlurData::device = device;
        custom.m_accent_color[ 0 ] = g_Vars.misc.menu_ascent.r * 255.f;
        custom.m_accent_color[ 1 ] = g_Vars.misc.menu_ascent.g * 255.f;
        custom.m_accent_color[ 2 ] = g_Vars.misc.menu_ascent.b * 255.f;
        custom.m_accent_color[ 3 ] = g_Vars.misc.menu_ascent.a * 255.f;

        static int current_weapon = 0;
        static int current_group = 0;
        static int rage_current_group = 0;

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

        const char* rage_select_target_names[ ] = {
           XorStr( "Lowest hp" ),
           XorStr( "Lowest distance" ),
           XorStr( "Lowest ping" ),
           XorStr( "Highest accuracy" ),
        };
        CVariables::RAGE* rbot = nullptr;
        CVariables::ANTIAIM_STATE* settings = nullptr;
        static int antiaim_type = 0;

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

        group_vector* aim_group = nullptr;

        auto fill_group_vector = [ ]( group_vector& vec, int idx ) {
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

        PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
        PushStyleVar( ImGuiStyleVar_Alpha, alpha );

        if ( opened || alpha >= 0.05f ) {

            ImGui::Begin( "Hello, world!", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground );
            {

                auto window = GetCurrentWindow( );
                auto draw = window->DrawList;
                auto pos = window->Pos;
                auto size = window->Size;
                auto style = GetStyle( );
                // 740 x 593
                // SetWindowSize( ImVec2( 620, 480 ) );
                SetWindowSize( ImVec2( 740 * alpha, 593 * alpha ) );

                custom.m_anim = ImLerp( custom.m_anim, 1.f, 0.038f );
                draw->AddText( GetIO( ).Fonts->Fonts[ 2 ], GetIO( ).Fonts->Fonts[ 2 ]->FontSize, pos + ImVec2( 12, 12 ), ImColor( 1.f, 1.f, 1.f, 0.8f * GetStyle( ).Alpha ), "EEXOMI" );

                draw->AddRectFilled( pos + ImVec2( 0, 40 ), pos + ImVec2( size.x, 70 ), to_vec4( 19, 20, 23, GetStyle( ).Alpha ) );
                draw->AddText( GetIO( ).Fonts->Fonts[ 3 ], GetIO( ).Fonts->Fonts[ 3 ]->FontSize, pos + ImVec2( 10, 49 ), to_vec4( 118, 132, 151, GetStyle( ).Alpha ), "A" );
                draw->AddText( GetIO( ).Fonts->Fonts[ 0 ], GetIO( ).Fonts->Fonts[ 0 ]->FontSize - 1, pos + ImVec2( 27, 48 ), GetColorU32( ImGuiCol_Text, 0.5f ), "Welcome dutu1337!" );

                SetCursorPosX( size.x - custom.get_label_sizes( custom.tabs ) - ( 40 * custom.tabs.size( ) ) );
                BeginGroup( );

                for ( int i = 0; i < custom.tabs.size( ); ++i ) {

                    if ( custom.tab( custom.tabs_icons.at( i ), custom.tabs.at( i ), custom.m_tab == i, i == custom.tabs.size( ) - 1 ? style.WindowRounding : 0, i == custom.tabs.size( ) - 1 ? ImDrawFlags_RoundCornersTopRight : 0 ) && custom.m_tab != i )
                        custom.m_tab = i, custom.m_anim = 0.f;

                    if ( i != custom.tabs.size( ) - 1 )
                        SameLine( );

                }

                EndGroup( );

                switch ( custom.m_tab ) {

                    case 0:

                        SetCursorPos( ImVec2( size.x - custom.get_label_sizes( custom.aimbot_sub_tabs ) - ( GetStyle( ).ItemSpacing.x * custom.aimbot_sub_tabs.size( ) ), 47 ) );
                        BeginGroup( );

                        for ( int i = 0; i < custom.aimbot_sub_tabs.size( ); ++i ) {

                            if ( custom.sub_tab( custom.aimbot_sub_tabs.at( i ), custom.m_aimbot_sub_tab == i ) && custom.m_aimbot_sub_tab != i )
                                custom.m_aimbot_sub_tab = i, custom.m_anim = 0.f;

                            if ( i != custom.aimbot_sub_tabs.size( ) - 1 )
                                SameLine( );

                        }

                        EndGroup( );

                        SetCursorPos( ImVec2( 10, 80 ) );
                        BeginChild( "##aimbot.groupboxes.area", ImVec2( size.x - 20, size.y - 90 ), false );
                        {

                            custom.group_box_alternative( "##master_switch.aimbot", ImVec2( GetWindowWidth( ), 53 ) );
                            {
                                if ( custom.m_aimbot_sub_tab == 0 ) {
                                    custom.checkbox( "Master switch##aimbot", &g_Vars.rage.enabled, "Toggle on all aimbot features", GetIO( ).Fonts->Fonts[ 1 ] );
                                }  else if ( custom.m_aimbot_sub_tab == 1 ) {
                                  
                                    custom.checkbox( "Master switch##aimbot", &g_Vars.antiaim.enabled, "Toggle on all anti-aimbot features", GetIO( ).Fonts->Fonts[ 1 ] );
                                    SameLine( GetWindowWidth( ) - 120 );

                                    vector<const char*> antiaim_type_str = { XorStr( "Stand" ), XorStr( "Move" ), XorStr( "Air" ) };

                                    custom.combo( "##combaas", &antiaim_type, antiaim_type_str.data( ), antiaim_type_str.size( ) );

                                    if ( antiaim_type == 0 )
                                        settings = &g_Vars.antiaim_stand;
                                    else if ( antiaim_type == 1 )
                                        settings = &g_Vars.antiaim_move;
                                    else
                                        settings = &g_Vars.antiaim_air;
                                }

                            } custom.end_group_box_alternative( );

                           
                            custom.group_box( "General", ImVec2( GetWindowWidth( ) / 2 - GetStyle( ).ItemSpacing.x / 2, GetWindowHeight( ) - 53 - GetStyle( ).ItemSpacing.y ) );
                            {
                                if ( custom.m_aimbot_sub_tab == 0 ) {
                                    ImGui::Combo( XorStr( "Group" ), &rage_current_group, rage_weapon_groups, ARRAYSIZE( rage_weapon_groups ) );
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

                                    ImGui::Combo( XorStr( "Target selection" ), &rbot->target_selection, rage_select_target_names, ARRAYSIZE( rage_select_target_names ) );
                                    ImGui::Checkbox( XorStr( "Visual resolver" ), &g_Vars.rage.visual_resolver );
                                    ImGui::Checkbox( XorStr( "Predict velocity-modifier" ), &g_Vars.rage.fix_velocity_modifier );

                                    ImGui::Checkbox( XorStr( "Enabled" ), &rbot->active );
                                    ImGui::Checkbox( XorStr( "Silent aim" ), &rbot->silent_aim );

                                    ImGui::Checkbox( XorStr( "Automatic penetration" ), &rbot->autowall );
                                    ImGui::Checkbox( XorStr( "Shot delay" ), &rbot->shotdelay );
                                    if ( rbot->shotdelay ) {
                                        ImGui::SliderFloat( XorStr( "Amount##ShotDelayAmount" ), &rbot->shotdelay_amount, 1.f, 100.f, XorStr( "%.0f %%" ) );
                                    }

                                    const char* auto_stop_type[ ] = { XorStr( "Off" ), XorStr( "Full Stop" ), XorStr( "Minimal speed" ) };
                                    ImGui::Combo( XorStr( "Automatic stop" ), &rbot->autostop, auto_stop_type, ARRAYSIZE( auto_stop_type ) );
                                    if ( rbot->autostop ) {
                                        ImGui::Checkbox( XorStr( "Between shots" ), &rbot->between_shots );
                                    }

                                    ImGui::Checkbox( XorStr( "Automatic scope" ), &rbot->autoscope );

                                    ImGui::Text( "Exploits" );
                                    SameLine( GetWindowWidth( ) - 37 );
                                    if ( custom.settings_widget( "##pop3", 2 ) ) OpenPopup( "##pop3" );

                                    custom.prepared_popup( "##pop3", "Exploits", [ & ]( ) {
                                        ImGui::Checkbox( XorStr( "Enabled" ), &g_Vars.rage.exploit );
                                        ImGui::SliderFloat( XorStr( "Double tap hit chance" ), &rbot->doubletap_hitchance, 0.f, 100.f, XorStr( "%.0f %%" ) );
                                     } );

                                 

                                    ImGui::Text( "Hitchance" );
                                    SameLine( GetWindowWidth( ) - 37 );
                                    if ( custom.settings_widget( "##pop1", 2 ) ) OpenPopup( "##pop1" );

                                    custom.prepared_popup( "##pop1", "Hitchance", [ & ]( ) {
                                        ImGui::SliderFloat( XorStr( "Minimum hit chance" ), &rbot->hitchance, 0.f, 100.f, XorStr( "%.0f %%" ) );
                                        ImGui::SliderFloat( XorStr( "Accuracy boost" ), &rbot->hitchance_accuracy, 0.f, 100.f, XorStr( "%.0f %%" ) );
                                        ImGui::SliderInt( XorStr( "Max misses" ), &rbot->max_misses, 1, 6, XorStr( "%d" ) );
                                        ImGui::Checkbox( XorStr( "Compensate spread" ), &rbot->no_spread );
                                     } );


                                    ImGui::Text( "Minimum damage" );
                                    SameLine( GetWindowWidth( ) - 37 );
                                    if ( custom.settings_widget( "##pop2", 2 ) ) OpenPopup( "##pop2" );

                                    custom.prepared_popup( "##pop2", "Minimum damage", [ & ]( ) {
                                        ImGui::SliderFloat( XorStr( "Minimum damage - Visible" ), &rbot->min_damage_visible, 1.f, ( rage_current_group - 1 == WEAPONGROUP_SNIPER ) ? 150.0f : 100.f, XorStr( "%.0f hp" ) );
                                        ImGui::SliderFloat( XorStr( "Minimum damage - Wall" ), &rbot->min_damage, 1.f, ( rage_current_group - 1 == WEAPONGROUP_SNIPER ) ? 150.0f : 100.f, XorStr( "%.0f hp" ) );
                                        ImGui::Checkbox( XorStr( "Damage override" ), &rbot->min_damage_override );
                                        if ( rbot->min_damage_override ) {
                                            ImGui::SliderFloat( XorStr( "Minimum damage - Override" ), &rbot->min_damage_override_amount, 1.f, 100.f, XorStr( "%.0f hp" ) );
                                        }
                                        ImGui::Checkbox( XorStr( "Health based override" ), &rbot->health_override );
                                        if ( rbot->health_override ) {
                                            ImGui::SliderFloat( XorStr( "Amount##HealthBasedOverride" ), &rbot->health_override_amount, 1.f, 100.f, XorStr( "+%.0fhp" ) );
                                        }
                                      } );

                                   HotkeyA( "Double-tap key", &g_Vars.rage.double_tap_bind );
                                   HotkeyA( "Damage override key", &g_Vars.rage.key_dmg_override );

                              
                                   
                                }
                                else if ( custom.m_aimbot_sub_tab == 1 ) {
                                    //ImGui::Text( "Manual" );
                                    ImGui::Checkbox( XorStr( "Manual" ), &g_Vars.antiaim.manual );
                                    SameLine( GetWindowWidth( ) - 37 );
                                    if ( custom.settings_widget( "##pop_manual", 2 ) ) OpenPopup( "##pop_manual" );

                                    custom.prepared_popup( "##pop_manual", "Manual", [ & ]( ) {
                                        HotkeyA( XorStr( "Left" ), &g_Vars.antiaim.manual_left_bind, KeyBindType::HOLD );
                                        HotkeyA( XorStr( "Right" ), &g_Vars.antiaim.manual_right_bind, KeyBindType::HOLD );
                                        HotkeyA( XorStr( "Back" ), &g_Vars.antiaim.manual_back_bind, KeyBindType::HOLD );
                                        HotkeyA( XorStr( "Mouse override" ), &g_Vars.antiaim.mouse_override, KeyBindType::HOLD );
                                    } );


                                    ImGui::Checkbox( XorStr( "Auto direction" ), &settings->autodirection );
                                    if ( settings->autodirection ) {
                                        SameLine( GetWindowWidth( ) - 37 );
                                        if ( custom.settings_widget( "##pop_dir", 2 ) ) OpenPopup( "##pop_dir" );
                                    }

                                    custom.prepared_popup( "##pop_dir", "Auto direction", [ & ]( ) {
                                        if ( settings->autodirection ) {
                                            ImGui::Checkbox( XorStr( "Auto direction ignore duck" ), &settings->autodirection_ignore_duck );

                                            ImGui::SliderInt( XorStr( "Extrapolation##AutoDirExtrapolation" ), &settings->extrapolation_ticks, 0, 20, XorStr( "%d ticks" ) );
                                            ImGui::SliderFloat( XorStr( "Range##AutoDirRange" ), &settings->autodirection_range, 24.0f, 64.0f, XorStr( "%.0f units" ) );

                                            // ImGui::ComboA( XorStr( "Desync auto direction" ), &settings->desync_autodir, auto_dir, ARRAYSIZE( auto_dir ) );
                                        }
                                    } );
                     
                                    static bool* disable_conditions[ ] = {
                                        &g_Vars.antiaim.on_freeze_period 	,
                                        &g_Vars.antiaim.on_knife 	,
                                        &g_Vars.antiaim.on_grenade 	,
                                        &g_Vars.antiaim.on_ladder 	,
                                        &g_Vars.antiaim.on_dormant 	,
                                        &g_Vars.antiaim.on_manual_shot 	,
                                    };

                                    const char* disable_conditions_str[ ] = {
                                       XorStr( "On freeze period" ),
                                       XorStr( "On knife" ),
                                       XorStr( "On grenade" ),
                                       XorStr( "On ladder" ),
                                       XorStr( "On dormant" ),
                                       XorStr( "On manual shot" ),
                                    };

                                    ImGui::MultiComboA( XorStr( "Disable condition" ),
                                                       disable_conditions, disable_conditions_str, 6 );

                                    const char* yaw_str[ ] = { XorStr( "Forward" ), XorStr( "Backward" ) };
                                    const char* yaw_base_str[ ] = { XorStr( "View" ), XorStr( "At targets" ) };
                                    const char* yaw_base_move_str[ ] = { XorStr( "View" ), XorStr( "At targets" ), XorStr( "Movement direction" ) };
                                    const char* pitch_str[ ] = { XorStr( "Off" ), XorStr( "Down" ), XorStr( "Up" ), XorStr( "Zero" ) };
                                    const char* auto_dir[ ] = { XorStr( "Off" ), XorStr( "Hide Fake" ), XorStr( "Hide Real" ) };

                                    auto str = antiaim_type == 0 ? yaw_base_str : yaw_base_move_str;
                                    auto str_size = antiaim_type == 0 ? 2 : 3;
                                    ImGui::Combo( XorStr( "Yaw base" ), &settings->base_yaw,
                                                   str, str_size );

                                    ImGui::Combo( XorStr( "Pitch" ), &settings->pitch, pitch_str, ARRAYSIZE( pitch_str ) );
                                    ImGui::Combo( XorStr( "Yaw" ), &settings->yaw, yaw_str, ARRAYSIZE( yaw_str ) );

                                    ImGui::Checkbox( XorStr( "Twist speed" ), &g_Vars.antiaim.twist_speed );
                                    ImGui::Checkbox( XorStr( "Lby prediction" ), &g_Vars.antiaim.lbypred );


                                    ImGui::Checkbox( XorStr( "Distortion" ), &g_Vars.antiaim.distort );

                                    SameLine( GetWindowWidth( ) - 37 );
                                    if ( custom.settings_widget( "##pop_distoruit", 2 ) ) OpenPopup( "##pop_distoruit" );

                                    custom.prepared_popup( "##pop_distoruit", "Distortion", [ & ]( ) {
                                        ImGui::Checkbox( XorStr( "Manual override" ), &g_Vars.antiaim.distort_manual_aa );
                                        if ( ImGui::Checkbox( XorStr( "Twist" ), &g_Vars.antiaim.distort_twist ) )
                                            ImGui::SliderFloat( XorStr( "Speed" ), &g_Vars.antiaim.distort_speed, 1.f, 10.f, XorStr( "%.1fs" ) );

                                        ImGui::SliderFloat( XorStr( "Max time" ), &g_Vars.antiaim.distort_max_time, 0.f, 10.f );
                                        ImGui::SliderFloat( XorStr( "Range" ), &g_Vars.antiaim.distort_range, -360.f, 360.f );

                                        static bool* disable_distort[ ] = {
                                            &g_Vars.antiaim.distort_disable_fakewalk 	,
                                            &g_Vars.antiaim.distort_disable_run 	,
                                            &g_Vars.antiaim.distort_disable_air 	,
                                        };

                                        const char* disable_dir_str[ ] = {
                                           XorStr( "Fakewalking ( wip )" ),
                                           XorStr( "Running" ),
                                           XorStr( "Airborne" ),
                                        };

                                        ImGui::MultiComboA( XorStr( "Disable distortion" ),
                                                            disable_distort, disable_dir_str, 3 );
                                    } );

                               
                                    ImGui::HotkeyA( "Auto direction key", &g_Vars.antiaim.autodirection_override );
                                    
                                }
                            } custom.end_group_box( );

                            SameLine( );

                            custom.group_box( "Miscellaneous", ImVec2( GetWindowWidth( ) / 2 - GetStyle( ).ItemSpacing.x / 2, GetWindowHeight( ) - 53 - GetStyle( ).ItemSpacing.y ) );
                            {
                                if ( custom.m_aimbot_sub_tab == 0 ) {
                                    bool* hitboxes[ ] = {
                                    &rbot->head_hitbox	,
                                    &rbot->neck_hitbox	,
                                    &rbot->chest_hitbox,
                                    &rbot->stomach_hitbox	,
                                    &rbot->pelvis_hitbox	,
                                    &rbot->arms_hitbox	,
                                    &rbot->legs_hitbox	,
                                    &rbot->feets_hitbox	,
                                    };

                                    bool* bt_hitboxes[ ] = {
                                       &rbot->bt_head_hitbox	,
                                       &rbot->bt_neck_hitbox	,
                                       &rbot->bt_chest_hitbox,
                                       &rbot->bt_stomach_hitbox	,
                                       &rbot->bt_pelvis_hitbox	,
                                       &rbot->bt_arms_hitbox	,
                                       &rbot->bt_legs_hitbox	,
                                       &rbot->bt_feets_hitbox	,
                                    };

                                    bool* mp_safety_hitboxes[ ] = {
                                       &rbot->mp_safety_head_hitbox	,
                                       &rbot->mp_safety_neck_hitbox	,
                                       &rbot->mp_safety_chest_hitbox,
                                       &rbot->mp_safety_stomach_hitbox	,
                                       &rbot->mp_safety_pelvis_hitbox	,
                                       &rbot->mp_safety_arms_hitbox	,
                                       &rbot->mp_safety_legs_hitbox	,
                                       &rbot->mp_safety_feets_hitbox	,
                                    };

                                    const char* hitboxes_str[ ] = {
                                       XorStr( "Head" ), XorStr( "Neck" ), XorStr( "Chest" ), XorStr( "Stomach" ), XorStr( "Pelvis" ), XorStr( "Arms" ), XorStr( "Legs" ), XorStr( "Feet" )
                                    };

                                    bool* bt_conditions[ ] = {
                                       &rbot->override_shot	,
                                       &rbot->override_running	,
                                       &rbot->override_walking	,
                                       &rbot->override_inair	,
                                       &rbot->override_standing	,
                                       &rbot->override_backward	,
                                       &rbot->override_sideways	,
                                       &rbot->override_unresolved	,
                                    };

                                    bool* safe_conditions[ ] = {
                                       &rbot->safe_shot	,
                                       &rbot->safe_running	,
                                       &rbot->safe_walking	,
                                       &rbot->safe_inair	,
                                       &rbot->safe_standing	,
                                       &rbot->safe_backward	,
                                       &rbot->safe_sideways	,
                                       &rbot->safe_unresolved	,
                                    };

                                    bool* body_conditions[ ] = {
                                       &rbot->prefer_body_shot	,
                                       &rbot->prefer_body_running	,
                                       &rbot->prefer_body_walking	,
                                       &rbot->prefer_body_inair	,
                                       &rbot->prefer_body_standing	,
                                       &rbot->prefer_body_backward	,
                                       &rbot->prefer_body_sideways	,
                                       &rbot->prefer_body_unresolved,
                                    };

                                    bool* head_conditions[ ] = {
                                       &rbot->prefer_head_shot	,
                                       &rbot->prefer_head_running	,
                                       &rbot->prefer_head_walking	,
                                       &rbot->prefer_head_inair	,
                                       &rbot->prefer_head_standing	,
                                       &rbot->prefer_head_backward	,
                                       &rbot->prefer_head_sideways	,
                                       &rbot->prefer_head_unresolved	,
                                    };

                                    const char* conditions_str[ ] = {
                                       XorStr( "On shot" ),
                                       XorStr( "Running" ) ,
                                       XorStr( "Walking" ) ,
                                       XorStr( "In air" ),
                                       XorStr( "Standing" ),
                                       XorStr( "Backward" ),
                                       XorStr( "Sideways" ),
                                       XorStr( "In duck" ),
                                    };

                                    const char* hitbox_select_type[ ] = { XorStr( "Damage" ), XorStr( "Accuracy" ), XorStr( "Safety" ) };
                                    ImGui::Combo( XorStr( "Hitbox selection" ), &rbot->hitbox_selection, hitbox_select_type, ARRAYSIZE( hitbox_select_type ) );

                                    ImGui::MultiComboA( XorStr( "Hitboxes" ),
                                                       hitboxes, hitboxes_str, 8 );

                                    ImGui::MultiComboA( XorStr( "Safety hitboxes" ),
                                                       mp_safety_hitboxes, hitboxes_str, 8 );

                                    ImGui::Checkbox( XorStr( "Override hitscan" ), &rbot->override_hitscan );

                                    if ( rbot->override_hitscan ) {
                                        ImGui::MultiComboA( XorStr( "Override condition" ),
                                                           bt_conditions, conditions_str, 8 );

                                        ImGui::MultiComboA( XorStr( "Override hitboxes" ),
                                                           bt_hitboxes, hitboxes_str, 8 );
                                    }


                                    ImGui::Checkbox( XorStr( "Ignore limbs on move" ), &rbot->ignorelimbs_ifwalking );
                                    ImGui::Checkbox( XorStr( "Delay on unducking" ), &rbot->delay_shot_on_unducking );

                                    ImGui::Text( "Prefer" );
                                    SameLine( GetWindowWidth( ) - 37 );
                                    if ( custom.settings_widget( "##pop4", 2 ) ) OpenPopup( "##pop4" );

                                    custom.prepared_popup( "##pop4", "Prefer", [ & ]( ) {
                                        ImGui::Checkbox( XorStr( "Prefer body" ), &rbot->prefer_body );
                                        if ( rbot->prefer_body ) {
                                            ImGui::MultiComboA( XorStr( "Conditions##PreferBody" ),
                                                                body_conditions, conditions_str, 8 );
                                        }

                                        ImGui::Checkbox( XorStr( "Prefer head" ), &rbot->prefer_head );

                                        if ( rbot->prefer_head ) {
                                            ImGui::MultiComboA( XorStr( "Conditions##PreferHead" ),
                                                                head_conditions, conditions_str, 8 );
                                        }

                                        ImGui::Checkbox( XorStr( "Prefer safety" ), &rbot->prefer_safety );

                                        if ( rbot->prefer_safety ) {
                                            ImGui::MultiComboA( XorStr( "Conditions##PreferSafety" ),
                                                                safe_conditions, conditions_str, 8 );
                                        }
                                                           } );                               

                                    ImGui::Text( "Point-scale" );
                                    SameLine( GetWindowWidth( ) - 37 );
                                    if ( custom.settings_widget( "##pop5", 2 ) ) OpenPopup( "##pop5" );

                                    custom.prepared_popup( "##pop5", "Point-scale", [ & ]( ) {
                                        ImGui::Checkbox( XorStr( "Dynamic point scale" ), &rbot->dynamic_ps );
                                        ImGui::SliderFloat( XorStr( "Max head scale##Head" ), &rbot->head_ps, 0.f, 95.0f, XorStr( "%.0f %%" ) );
                                        ImGui::SliderFloat( XorStr( "Max body scale##Head" ), &rbot->body_ps, 0.f, 95.0f, XorStr( "%.0f %%" ) );
                                                           } );


                                   
                                    HotkeyA( "Override hitscan key", &g_Vars.rage.override_key );
                                    HotkeyA( "Prefer body key", &g_Vars.rage.prefer_body );
                                    HotkeyA( "Prefer head key", &g_Vars.rage.prefer_head );
                                    HotkeyA( "Prefer safety key", &g_Vars.rage.prefer_safe );
                                }
                                else if ( custom.m_aimbot_sub_tab == 1 ) {
                                    ImGui::Checkbox( XorStr( "Enable fakelag## fake lag" ), &g_Vars.fakelag.enabled );
                                    SameLine( GetWindowWidth( ) - 37 );
                                    if ( custom.settings_widget( "##pop_fakelag", 2 ) ) OpenPopup( "##pop_fakelag" );

                                    custom.prepared_popup( "##pop_fakelag", "Enable fakelag## fake lag", [ & ]( ) {
                                        static bool* conditions[ ] = {
                                        &g_Vars.fakelag.when_standing,
                                        &g_Vars.fakelag.when_moving,
                                        &g_Vars.fakelag.when_air,
                                        &g_Vars.fakelag.when_dormant,
                                        &g_Vars.fakelag.when_exploits,
                                        };

                                        const char* conditions_str[ ] = {
                                           XorStr( "Standing" ), XorStr( "Moving" ), XorStr( "Air" ),  XorStr( "Dormant" ), XorStr( "Exploits" ),
                                        };
#endif

                                        static bool* alt_conditions[ ] = {
                                           &g_Vars.fakelag.when_peek,
                                           &g_Vars.fakelag.when_unducking,
                                           &g_Vars.fakelag.when_land,
                                           &g_Vars.fakelag.when_switching_weapon,
                                           &g_Vars.fakelag.when_reloading,
                                           &g_Vars.fakelag.when_velocity_change,
                                           &g_Vars.fakelag.break_lag_compensation,
                                        };

                                        const char* alt_conditions_str[ ] = {
                                           XorStr( "Peek" ), XorStr( "Unducking" ), XorStr( "Land" ), XorStr( "Weapon Swap" ), XorStr( "Reload" ),
                                           XorStr( "Velocity change" ), XorStr( "Break lag compensation" ),
                                        };

                                        ImGui::MultiComboA( XorStr( "Conditions" ), conditions, conditions_str, 4 );
                                        ImGui::SliderInt( XorStr( "Limit" ), &g_Vars.fakelag.choke, 1, 16, XorStr( "%d ticks" ) );
                                        ImGui::SliderFloat( XorStr( "Variance" ), &g_Vars.fakelag.variance, 0.0f, 100.0f, XorStr( "%.0f %%" ) );
                                        ImGui::MultiComboA( XorStr( "Alternative conditions" ), alt_conditions, alt_conditions_str, 7 );
                                        ImGui::SliderInt( XorStr( "Alternative lag amount" ), &g_Vars.fakelag.alternative_choke, 1, 16, XorStr( "%d ticks" ) );
                                                           } );
#if 1
                                    
                                }
                                

                            } custom.end_group_box( );

                        } EndChild( );

                        break;
                        case 1:
                            SetCursorPos( ImVec2( size.x - custom.get_label_sizes( custom.visuals_subtab ) - ( GetStyle( ).ItemSpacing.x * custom.visuals_subtab.size( ) ), 47 ) );
                            BeginGroup( );

                            for ( int i = 0; i < custom.visuals_subtab.size( ); ++i ) {

                                if ( custom.sub_tab( custom.visuals_subtab.at( i ), custom.m_visuals_sub_tab == i ) && custom.m_visuals_sub_tab != i )
                                    custom.m_visuals_sub_tab = i, custom.m_anim = 0.f;

                                if ( i != custom.visuals_subtab.size( ) - 1 )
                                    SameLine( );

                            }

                            /*
                            
                             SetCursorPos( ImVec2( size.x - custom.get_label_sizes( custom.aimbot_sub_tabs ) - ( GetStyle( ).ItemSpacing.x * custom.aimbot_sub_tabs.size( ) ), 47 ) );
                        BeginGroup( );

                        for ( int i = 0; i < custom.aimbot_sub_tabs.size( ); ++i ) {

                            if ( custom.sub_tab( custom.aimbot_sub_tabs.at( i ), custom.m_aimbot_sub_tab == i ) && custom.m_aimbot_sub_tab != i )
                                custom.m_aimbot_sub_tab = i, custom.m_anim = 0.f;

                            if ( i != custom.aimbot_sub_tabs.size( ) - 1 )
                                SameLine( );

                        }
                            */

                            EndGroup( );

                            SetCursorPos( ImVec2( 10, 80 ) );
                            BeginChild( "##aimbot.groupboxes.area2", ImVec2( size.x - 20, size.y - 90 ), false );
                            {

                                custom.group_box_alternative( "##master_switch.aimbot2", ImVec2( GetWindowWidth( ), 53 ) );
                                {
                                    if ( custom.m_visuals_sub_tab == 0 ) {
                                        custom.checkbox( "Master switch##aimbot", &g_Vars.rage.enabled, "Toggle on all visuals features", GetIO( ).Fonts->Fonts[ 1 ] );

                                    } else if ( custom.m_visuals_sub_tab == 1 ) {

                                    } else if ( custom.m_visuals_sub_tab == 2 ) {

                                    } else if ( custom.m_visuals_sub_tab == 3 ) {

                                    }

                                } custom.end_group_box_alternative( );


                                custom.group_box( "Overlay", ImVec2( GetWindowWidth( ) / 2 - GetStyle( ).ItemSpacing.x / 2, GetWindowHeight( ) - 53 - GetStyle( ).ItemSpacing.y ) );
                                {
                                    if ( custom.m_visuals_sub_tab == 0 ) {
                                        ImGui::Checkbox( XorStr( "Ignore team" ), &g_Vars.esp.team_check );
                                        ImGui::Checkbox( XorStr( "Extended ESP" ), &g_Vars.esp.extended_esp );
                                        custom.checkbox( XorStr( "Dormant" ), &g_Vars.esp.fade_esp, "Sending sound info to esp-system.", GetIO( ).Fonts->Fonts[ 1 ] );
                                        ImGui::Checkbox( XorStr( "Box" ), &g_Vars.esp.box );
                                        ImGui::SameLine( ImGui::GetWindowWidth( ) - 47 );
                                        ImGui::ColorEdit4( XorStr( "##Box Color" ), g_Vars.esp.box_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip );
                                       //const char* box_type_str[ ] = { XorStr( "Bounding" ), XorStr( "Corner" ) };
                                       //if ( g_Vars.esp.box ) {
                                       //    ImGui::ComboA( XorStr( "Box type##box type" ), &g_Vars.esp.box_type, box_type_str, ARRAYSIZE( box_type_str ) );
                                       //}
                                    } else if ( custom.m_visuals_sub_tab == 1 ) {

                                    } else if ( custom.m_visuals_sub_tab == 2 ) {

                                    } else if ( custom.m_visuals_sub_tab == 3 ) {

                                    }
                                } custom.end_group_box( );

                                SameLine( );

                                if ( custom.m_visuals_sub_tab == 0 ) {
                                    custom.group_box( "Chams", ImVec2( GetWindowWidth( ) / 2 - GetStyle( ).ItemSpacing.x / 2, ( ( GetWindowHeight( ) - 53 - GetStyle( ).ItemSpacing.y ) / 2 ) - 10 ) );
                                    {

                                    } custom.end_group_box( );
                                   
                                    ImGui::SetCursorPosX( ( GetWindowWidth( ) / 2 - GetStyle( ).ItemSpacing.x / 2 ) + 10 );
                                    ImGui::SetCursorPosY( 280 );

                                    custom.group_box( "Glow", ImVec2( GetWindowWidth( ) / 2 - GetStyle( ).ItemSpacing.x / 2, ( ( GetWindowHeight( ) - 53 - GetStyle( ).ItemSpacing.y ) / 2 ) ) );
                                    {

                                    } custom.end_group_box( );
                                } else {
                                    custom.group_box( "Other", ImVec2( GetWindowWidth( ) / 2 - GetStyle( ).ItemSpacing.x / 2, ( GetWindowHeight( ) - 53 - GetStyle( ).ItemSpacing.y ) ) );
                                    {
                                        if ( custom.m_visuals_sub_tab == 1 ) {

                                        } else if ( custom.m_visuals_sub_tab == 2 ) {

                                        } else if ( custom.m_visuals_sub_tab == 3 ) {

                                        }


                                    } custom.end_group_box( );
                                }

                               



                            } EndChild( );

                            break;
                    case 3:

                        SetCursorPos( ImVec2( 10, 80 ) );
                        BeginChild( "##scripts.groupboxes.area", ImVec2( size.x - 20, size.y - 90 ) );
                        {

                            custom.group_box_alternative( "##enable_ffi.scripts", ImVec2( GetWindowWidth( ), 53 ) );
                            {

                                custom.checkbox( "Enable Foreign Function Interface##scripts", &enableffi, "Allow scripts to use FFI.", GetIO( ).Fonts->Fonts[ 1 ] );
                                SameLine( GetWindowWidth( ) - 155 );
                                custom.button( "Get more scripts", "B", ImVec2( 130, 30 ) );

                            } custom.end_group_box_alternative( );

                            custom.list_box( "Available scripts", ImVec2( GetWindowWidth( ) / 2 - GetStyle( ).ItemSpacing.x / 2 - 70, GetWindowHeight( ) - 53 - GetStyle( ).ItemSpacing.y ), [ ]( ) {

                                for ( int i = 0; i < lua_list.size( ); ++i ) {

                                    arr[ i ] = custom.lua( lua_list.at( i ), cur_lua == i );

                                    if ( arr[ i ].pressed_whole )
                                        cur_lua = i;
                                }

                                             }, ImVec2( 0, 0 ) );

                            SameLine( ), SetCursorPosY( 53 + GetStyle( ).ItemSpacing.y );

                            custom.group_box_alternative( "##lua.information", ImVec2( GetWindowWidth( ) / 2 + 66, GetWindowHeight( ) - 53 - GetStyle( ).ItemSpacing.y ), ImVec2( 1, 1 ) );
                            {

                                switch ( cur_lua ) {

                                    case 0:

                                        custom.image( weather, ImVec2( GetWindowWidth( ) - 2, 150 ), 4, ImDrawFlags_RoundCornersTop );
                                        SetCursorPos( ImVec2( GetWindowWidth( ) - 20, 5 ) );
                                        custom.button_alternative( "G", ImVec2( 15, 16 ) );

                                        SetCursorPos( ImVec2( GetWindowWidth( ) - 40, 5 ) );
                                        custom.button_alternative( "D", ImVec2( 15, 16 ) );

                                        SetCursorPos( ImVec2( 10, 160 ) );
                                        BeginChild( "##weather.child", ImVec2( GetWindowWidth( ) - 20, GetWindowHeight( ) - 160 ) );

                                        PushFont( io.Fonts->Fonts[ 4 ] );
                                        TextColored( arr[ cur_lua ].active ? custom.get_accent_color( GetStyle( ).Alpha ) : to_vec4( 116, 129, 148, style.Alpha ), arr[ cur_lua ].active ? "E" : "F" );
                                        PopFont( );

                                        SameLine( ), SetCursorPosY( GetCursorPosY( ) - 2 );

                                        PushFont( io.Fonts->Fonts[ 5 ] );
                                        Text( lua_list.at( cur_lua ) );
                                        PopFont( );

                                        SetCursorPosY( GetCursorPosY( ) - 5 );
                                        Text( "Last updated: 16.02.2022" );

                                        Text( "Makes rain" );

                                        EndChild( );

                                        break;

                                }

                            } custom.end_group_box_alternative( );

                        } EndChild( );

                        break;

                }

            } ImGui::End( );

        }

        PopStyleVar( 2 );
	}
#include "bytes.hpp"
	void c_menu::init( IDirect3DDevice9* const device ) {
        static bool o = false;
        if ( !o ) {
            if ( !weather )
                D3DXCreateTextureFromFileInMemoryEx( device, &lightning_bolts_binary, sizeof lightning_bolts_binary, 256, 256,
                                                     D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &weather );

            o = true;
        }

	}
}
#endif