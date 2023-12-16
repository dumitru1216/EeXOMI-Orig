#pragma once
#include "FnvHash.hpp"
#include <any>
#include <variant>
#include <map>
#include <d3d9.h>
#include "json.h"
#include "vector.hpp"
#include "qangle.hpp"
#include "CStudioRender.hpp" 

#if 1
#define DevelopMode
#endif

#if 1
#define BETA_MODE
#endif

#if 0

#endif

#include "XorStr.hpp"

class CUserCmd;

struct ImColor; 
struct ImVec4;

namespace KeyBindType
{
   enum {
	  HOLD = 0,
	  TOGGLE,
	  ALWAYS_ON
   };
};

struct KeyBind_t {
   KeyBind_t( ) { }

   int key = 0, cond = 0;
   bool enabled = false;

   void to_json( nlohmann::json& j ) {
	  j = nlohmann::json{
		 { ( XorStr( "key" ) ), key },
	  { ( XorStr( "cond" ) ), cond },
	  };
   }

   void from_json( nlohmann::json& j ) {
	  j.at( XorStr( "key" ) ).get_to( key );
	  j.at( XorStr( "cond" ) ).get_to( cond );
   }
};

extern std::vector<KeyBind_t*> g_keybinds;

class FloatColor {
public:
   FloatColor( ) = default;
   FloatColor( float _r, float _g, float _b, float _a = 1.0f ) :
	  r( _r ), g( _g ), b( _b ), a( _a ) {
   }

   FloatColor( int _r, int _g, int _b, int _a = 255 ) { SetColor( _r, _g, _b, _a ); }

   FloatColor( ImColor color );
   FloatColor( ImVec4 color );

   FloatColor Lerp( const FloatColor& c, float t ) const;

   void SetColor( float _r, float _g, float _b, float _a = 1.0f ) {
	  r = _r;
	  g = _g;
	  b = _b;
	  a = _a;
   }

   void SetColor( int _r, int _g, int _b, int _a = 255 ) {
	  r = static_cast< float >( _r ) / 255.0f;
	  g = static_cast< float >( _g ) / 255.0f;
	  b = static_cast< float >( _b ) / 255.0f;
	  a = static_cast< float >( _a ) / 255.0f;
   }

   FloatColor Alpha( float alpha ) {
	  return FloatColor( r, g, b, alpha );
   }

   uint32_t Hex( ) const {
	  union {
		 uint32_t i;
		 struct {
			uint8_t bytes[ 4 ];
		 };
	  } conv;

	  conv.bytes[ 0 ] = static_cast< int >( r * 255.0f );
	  conv.bytes[ 1 ] = static_cast< int >( g * 255.0f );
	  conv.bytes[ 2 ] = static_cast< int >( b * 255.0f );
	  conv.bytes[ 3 ] = static_cast< int >( a * 255.0f );

	  return conv.i;
   };

   void FloatColor::to_json( nlohmann::json& j ) {
	  j = nlohmann::json{
		 { ( "r" ), r },
	  {( "g" ), g },
	  {( "b" ), b },
	  {( "a" ), a }
	  };
   } 

   void FloatColor::from_json( nlohmann::json& j ) {
	  j.at( ( "r" ) ).get_to( r );
	  j.at( ( "g" ) ).get_to( g );
	  j.at( ( "b" ) ).get_to( b );
	  j.at( ( "a" ) ).get_to( a );
   }

   bool operator==( const FloatColor& clr ) const {
	  return clr.r == r && clr.g == g && clr.b == b && clr.a == a;
   };

   bool operator!=( const FloatColor& clr ) const {
	  return clr.r != r || clr.g != g || clr.b != b || clr.a != a;
   };

   FloatColor operator*( float v ) const {
	  return FloatColor( r * v, g * v, b * v, a );
   }

   operator uint32_t( ) const { return Hex( ); };

   operator ImColor( ) const;
   operator ImColor* ( );

   operator float* ( ) { return &r; };

   float r, g, b, a;

   static FloatColor Black;
   static FloatColor White;
   static FloatColor Gray;
};

#pragma region Config System
// best cfg system aka pizdoc na macrosax by soufiw

class CBaseGroup {
public: // ghetto fix, for skin changer options setup
   std::map< std::string, std::unique_ptr< std::any > > m_options;
   std::string m_name;
   nlohmann::json m_json;
   std::vector< CBaseGroup* > m_children;

   using AllowedTypes = std::variant< int, bool, float, std::string, FloatColor >;
   template < typename T >
   using IsTypeAllowed = std::enable_if_t< std::is_constructible_v< AllowedTypes, T >, T* >;

public:
   CBaseGroup( ) = default;
   CBaseGroup( std::string name, CBaseGroup* parent = nullptr ) {
	  m_name = std::string( name );
	  if ( parent )
		 parent->AddChild( this );
   }

   CBaseGroup( size_t idx, CBaseGroup* parent = nullptr ) {
	  m_name = std::string( "[" ) + std::to_string( idx ) + std::string( "]" );
	  if ( parent )
		 parent->AddChild( this );
   };

protected:
   template < typename T, class... Types >
   auto AddOption( const char* name, Types&& ... args ) -> T* {
	  auto pair = m_options.try_emplace( std::string( name ), std::make_unique< std::any >( std::make_any< T >( std::forward< Types >( args )... ) ) );

	  if ( typeid( T ).hash_code( ) == typeid( KeyBind_t ).hash_code( ) )
		 g_keybinds.push_back( reinterpret_cast< KeyBind_t* >( pair.first->second.get( ) ) );

	  return reinterpret_cast< T* >( pair.first->second.get( ) );
   };

   auto AddChild( CBaseGroup* group ) -> void { m_children.push_back( group ); };
public:
   auto GetName( ) const -> std::string { return m_name; }

   auto GetJson( ) -> nlohmann::json & { return m_json; }

   auto SetName( const std::string_view& name ) -> void { m_name = name; }

   virtual auto Save( ) -> void {
	  m_json.clear( );
	  for ( auto& [name, opt] : m_options ) {
		 // TODO: option class with virtual save function (is it good idea?)
		 auto any = *opt.get( );
		 auto hash = any.type( ).hash_code( );

		 // find out, could iterate AllowedTypes
		 if ( typeid( int ).hash_code( ) == hash )
			m_json[ name ] = std::any_cast< int >( any );
		 else if ( typeid( bool ).hash_code( ) == hash )
			m_json[ name ] = std::any_cast< bool >( any );
		 else if ( typeid( float ).hash_code( ) == hash )
			m_json[ name ] = std::any_cast< float >( any );
		 else if ( typeid( std::string ).hash_code( ) == hash )
			m_json[ name ] = std::any_cast< std::string >( any );
		 else if ( typeid( FloatColor ).hash_code( ) == hash )
			std::any_cast< FloatColor >( any ).to_json( m_json[ name ] );
		 else if ( typeid( KeyBind_t ).hash_code( ) == hash )
			std::any_cast< KeyBind_t >( any ).to_json( m_json[ name ] );
	  }

	  for ( auto& child : m_children ) {
		 child->Save( );

		 auto json = child->GetJson( );
		 m_json[ child->GetName( ) ] = json;
	  }
   }

   virtual auto Load( nlohmann::json& js ) -> void {
	  m_json.clear( );
	  m_json = js;
	  for ( auto& [name, opt] : m_options ) {
		 // TODO: option class with virtual load function (is it good idea?)
		 std::any& any = *opt.get( );
		 auto hash = any.type( ).hash_code( );

		 if ( m_json.count( name ) <= 0 )
			continue;

		 try {
			// find out, can iterate AllowedType? 
			if ( typeid( int ).hash_code( ) == hash )
			   std::any_cast< int& >( any ) = m_json[ name ];
			else if ( typeid( bool ).hash_code( ) == hash )
			   std::any_cast< bool& >( any ) = m_json[ name ];
			else if ( typeid( float ).hash_code( ) == hash )
			   std::any_cast< float& >( any ) = m_json[ name ];
			else if ( typeid( std::string ).hash_code( ) == hash )
			   std::any_cast< std::string& >( any ) = m_json[ name ];
			else if ( typeid( FloatColor ).hash_code( ) == hash )
			   std::any_cast< FloatColor& >( any ).from_json( m_json[ name ] );
			else if ( typeid( KeyBind_t ).hash_code( ) == hash )
			   std::any_cast< KeyBind_t& >( any ).from_json( m_json[ name ] );
		 } catch ( std::exception& ) {
			continue;
		 }
	  }

	  for ( auto& child : m_children ) {
		 child->Load( m_json[ child->GetName( ) ] );
	  }
   }
};

// TODO: std::map group
template < class ArrayImpl, class = std::enable_if_t< std::is_base_of< CBaseGroup, ArrayImpl >::value > >
class CArrayGroup : public CBaseGroup {
public:
   CArrayGroup( ) { }

   CArrayGroup( const std::string& name, size_t count = 0 ) :
	  CBaseGroup( name ) {
	  m_children.reserve( count );
	  for ( auto i = 0u; i < count; ++i ) {
		 m_children.emplace_back( new ArrayImpl( i ) );
	  }
   }

   auto AddEntry( ) -> size_t {
	  size_t idx = m_children.size( );
	  m_children.emplace_back( new ArrayImpl( idx ) );
	  return idx;
   }

   auto operator[]( ptrdiff_t idx ) -> ArrayImpl* {
	  return ( ArrayImpl* ) m_children[ idx ];
   }

   auto Size( ) const -> size_t {
	  return m_children.size( );
   };

   // update only children
   virtual auto Save( ) -> void {
	  m_json.clear( );
	  for ( auto& child : m_children ) {
		 child->Save( );

		 auto json = child->GetJson( );
		 m_json[ child->GetName( ) ] = json;
	  }
   }

   virtual auto Load( nlohmann::json& js ) -> void {
	  m_json.clear( );
	  m_json = js;
	  for ( auto& child : m_children ) {
		 child->Load( m_json[ child->GetName( ) ] );
	  }
   }
};

#define group_begin( group_name )                                  \
                                                                   \
  class group_name : public CBaseGroup {                           \
                                                                   \
  public:                                                          \
    group_name( CBaseGroup* parent = nullptr ) :                   \
        CBaseGroup( #group_name, parent ){};                       \
    group_name( const char* name, CBaseGroup* parent = nullptr ) : \
        CBaseGroup( name, parent ){};                              \
                                                                   \
    group_name( size_t idx, CBaseGroup* parent = nullptr ) :       \
        CBaseGroup( idx, parent ) {}

#define group_end() }

#define group_end_child( group_name, var_name ) \
  }                                             \
  ;                                             \
                                                \
  group_name var_name = group_name( XorStr(#group_name), this )

#define add_child_group( group_name, var_name ) group_name var_name = group_name( XorStr(#group_name), this )
#define add_child_group_ex( group_name, var_name ) group_name var_name = group_name( XorStr(#var_name), this )
#define add_child_group_name( group_name, var_name, name ) group_name var_name = group_name( XorStr(name), this )

#define config_option( type, name, ... ) type& name = *this->AddOption< type >( XorStr(#name), __VA_ARGS__ );
#define config_option_separate( type, name, parent, ... ) type& name = *parent.AddOption< type >( XorStr(#name), __VA_ARGS__ );

#define config_keybind( name ) config_option( KeyBind_t, name )
#pragma endregion

class ConVar;

struct SpreadRandom_t {
   float flRand1;
   float flRandPi1;
   float flRand2;
   float flRandPi2;
};

class CVariables : public CBaseGroup {
public:
   typedef struct _GLOBAL {
	  bool menuOpen = false;
	  bool d3dinitialize = false;
	  bool FixCycle;
	  bool UnknownCycleFix;
	  bool hackUnload = false;
	  float m_flLbyUpdateTime = 0.f;
	  int FakeDuckWillChoke = 0;

	  // cached random values used by spread, faster than RandomFloat call every hitchance tick ( also thread safe )
	  bool RandomInit = false;
	  SpreadRandom_t SpreadRandom[ 256 ];

	  bool WasShooting = false;
	  bool WasShootingInChokeCycle = false;

	  float flRealYaw;
	  QAngle angViewangles;

	  bool CorrectShootPosition = false;
	  Vector AimPoint;
	  Vector ShootPosition;

	  float MouseOverrideYaw = 0.0f;
	  bool MouseOverrideEnabled = false;
	  QAngle PreviousViewangles;
	  float YawDelta = 0.0f;
	  bool ResetWeapon = false;
	  float LastVelocityModifier = 0.0f;
	  float Realtime = 0.f;
	  bool HideShots = false;
	  bool IsRoundFreeze = false;
	  bool Fakeducking = false;
	  bool m_bAimbotShot = false;
	  int TickbaseAmount = 0;
	  int m_iNetworkedTick = 0;
	  int LastChokedCommands = 0;
	  Vector m_vecNetworkedOrigin;
	  float m_flPreviousDuckAmount = 0.0f;
	  bool HackIsReady = false;
	  bool RenderIsReady = false;
	  bool VoiceEnable = false;
	  Vector LagOrigin;
	  matrix3x4_t LagPosition[ 128 ];
      bool m_bUpdatingAnimations;

	  int m_iResolverType[ 65 ];
	  int m_iResolverType2[ 65 ];
      int m_iResolverSide[ 65 ];
	  int m_iRecordPriority[ 65 ];

	  int manual_aa = 1; // left, back, right

						 // real bones animation 
	  Vector   m_RealBonesPositions[ 256 ];
	  Quaternion m_RealBonesRotations[ 256 ];

	  // fake animations
	  float FakeWeaponPoses[ 24 ];
	  float m_flFakePoseParams[ 24 ] = {};
	  bool m_bFakeInit = false;
	  QAngle m_FakeAngles = QAngle( 0.0f, 0.0f, 0.0f );

	  float MenuAlpha = 1.0f;
	  IDirect3DDevice9* m_pD3D9Device;

	  uint32_t m_iShotCmdNum = {};

	  std::string c_login, c_password, server_adress;

	  int m_iServerType = 0;
	  int m_iGameMode = 0;

	  bool m_bNewMap = false;

      HINSTANCE hModule;

      std::string szLastHookCalled = XorStr( "none hook was called" );

      bool m_bLocalPlayerHarmedThisTick = false;
      bool m_bRenderingDormant[ 65 ];

      float m_flNextCmdTime = 0.f;
      int m_iLastCommandAck = 0;

      float m_flLastShotTime = 0.f;

      bool m_bInCreateMove = false;

   } GLOBAL, * PGLOBAL;

   group_begin( MENU );
   config_option( FloatColor, main_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( int, m_count, 0 );
   config_option( int, m_selected, 0 );
   group_end( );

#pragma region Skin Changer
   group_begin( skin_changer_data );
   config_option( bool, m_enabled, false );
   config_option( bool, m_custom, false );
   config_option( int, m_paint_kit, 0 );
   config_option( int, m_seed, 0 );
   config_option( int, m_stat_trak, 0 );
   config_option( float, m_wear, std::numeric_limits<float>::min( ) );
   config_option( std::string, m_custom_name, "" );

   // run-time data only
   //friend class source::feature::SkinChanger;
   //friend class OptionStorage;
   int m_paint_kit_index = 0;
   uint16_t m_definition_index = 0;
   bool m_executed = false;

   group_end( );

   group_begin( skin_changer_global_data );
   config_option( bool, m_active, false );

   config_option( bool, m_knife_changer, false );
   config_option( int, m_knife_idx, 0 );
   int m_knife_vector_idx = 0;

   config_option( bool, m_glove_changer, false );
   config_option( int, m_gloves_idx, 0 );
   int m_gloves_vector_idx = 0;

   bool m_update_gloves = false;
   bool m_update_skins = false;

   group_end( );
#pragma endregion

   group_begin( LEGIT_GENERAL );
   config_option( bool, active, false );
   config_option( bool, throughsmoke, false );
   config_option( bool, whileblind, false );
   config_option( bool, ignorejump, false );
   config_option( bool, snipers_only_scope, false );

   config_option( bool, autopistol, false );
   config_option( float, autopistol_delay, 0.0f );

   config_option( bool, position_adjustment, false );

   group_end( );

   group_begin( LEGIT );
   config_option( bool, active, false );
   config_option( bool, autofire, false );
   config_option( bool, kill_delay, false ); // delay between kills
   config_option( bool, fsd_enabled, false ); // first shot delay
   config_option( bool, rcs, false );
   config_option( bool, rcs_standalone, false );
   config_option( bool, autowall, false );
   config_option( bool, quickstop, false );

   config_option( float, fov, 0.0f );
   config_option( float, deadzone, 0.0f );
   config_option( float, smooth, 0.0f );
   config_option( float, rcs_x, 0.0f );
   config_option( float, rcs_y, 0.0f );
   config_option( float, first_shot_delay, 0.0f );
   config_option( float, kill_shot_delay, 0.0f );
   config_option( float, reaction_time, 0.0f );
   config_option( float, randomize, 0.0f );

   config_option( int, rcs_shots, 0 );
   config_option( int, hitbox, 0 ); // Head/Neck/Chest/Stomach/Pelvis
   config_option( int, hitbox_selection, 0 ); // priority/dynamic/nearest
   config_option( int, smooth_type, 0 );

   config_option( bool, auto_delay, false );

   config_option( bool, head_hitbox, false );
   config_option( bool, neck_hitbox, false );
   config_option( bool, chest_hitbox, false );
   config_option( bool, stomach_hitbox, false );
   config_option( bool, pelvis_hitbox, false );
   config_option( bool, arms_hitbox, false );
   config_option( bool, legs_hitbox, false );

   config_option( bool, trg_enabled, false );
   config_option( bool, trg_autowall, false );
   config_option( bool, trg_aimbot, false );
   config_option( float, trg_burst, 0.0f ); // 0.0-2.0
   config_option( float, trg_delay, 0.0f ); // 0.0-1.0
   config_option( float, trg_hitchance, 25.0f ); // 0.0-100.0

   config_keybind( trg_key );

   config_option( bool, trg_head_hitbox, false );
   config_option( bool, trg_chest_hitbox, false );
   config_option( bool, trg_stomach_hitbox, false );
   config_option( bool, trg_arms_hitbox, false );
   config_option( bool, trg_legs_hitbox, false );

   config_keybind( key );
   config_option( float, delay, 0 );

   int item_idx = 0;

   group_end( );

#pragma region Rage general group
   group_begin( RAGE_GENERAL );
   config_option( bool, enabled, false );
   config_option( bool, visual_resolver, false );
   config_option( bool, rage_multithread, false );
   config_option( bool, team_check, false );
   config_option( bool, experimental_resolver, false );

   config_option( bool, exploit, false );
   config_option( int, exploit_type, 0 ); // tickbase/rapid
   config_option( int, double_tap_type, 0 );

   config_keybind( double_tap_bind );
   config_keybind( hide_shots_bind );
   config_option( bool, break_lagcomp, false );

   config_keybind( prefer_body );
   config_keybind( prefer_head );
   config_keybind( prefer_safe );

   // temporary here
   config_option( bool, hide_shots, false );

   // rapid
   config_option( bool, rapid_on_shot, false );
   config_option( bool, rapid_on_peek, false );
   config_option( float, rapid_peekdist, 20.0f );
   config_option( bool, rapid_auto_peekdist, false );
   config_option( int, charge_amount, 1 );
   config_keybind( rapid_charge );
   config_keybind( rapid_release );

   config_keybind( key_dmg_override );
   config_keybind( override_key );

   group_end( );

   group_begin( RAGE );
   config_option( bool, active, false );

   config_option( float, min_damage, 0.0f );
   config_option( float, min_damage_visible, 0.0f );
   config_option( float, hitchance, 0.0f );
   config_option( float, doubletap_hitchance, 0.0f );
   config_option( float, hitchance_accuracy, 0.0f );

   config_option( bool, health_override, false );
   config_option( float, health_override_amount, 0.0f );

   config_option( bool, silent_aim, false );
   config_option( bool, aim_lock, false );
   config_option( bool, no_recoil, false );
   config_option( bool, no_spread, false );
   config_option( bool, friendly_fire, false );
   config_option( bool, autowall, false );
   config_option( bool, autoscope, false );
   config_option( int, target_selection, 0 );
   config_option( int, autostop, 0 ); // Off, Full stop, Minimal speed
   config_option( bool, ignorelimbs_ifwalking, false );
   config_option( bool, body_aim_if_lethal, false );
   config_option( bool, between_shots, false );

   config_option( bool, min_damage_override, false );
   config_option( float, min_damage_override_amount, 0.0f );

   config_option( int, hitbox_selection, 0 ); // Damage, Accuracy
   config_option( int, mp_density, 0 ); // low-high

   config_option( bool, dynamic_ps, false );
   config_option( bool, head_hitbox, false );
   config_option( bool, neck_hitbox, false );
   config_option( bool, chest_hitbox, false );
   config_option( bool, stomach_hitbox, false );
   config_option( bool, pelvis_hitbox, false );
   config_option( bool, arms_hitbox, false );
   config_option( bool, legs_hitbox, false );
   config_option( bool, feets_hitbox, false );

   config_option( float, head_ps, 0.0f );
   config_option( float, body_ps, 0.0f );

   config_option( float, prefer_threshold, 45.0f );

   config_option( bool, override_hitscan, false );
   config_option( bool, override_shot, false );
   config_option( bool, override_running, false );
   config_option( bool, override_walking, false );
   config_option( bool, override_inair, false );
   config_option( bool, override_standing, false );
   config_option( bool, override_backward, false );
   config_option( bool, override_sideways, false );
   config_option( bool, override_unresolved, false );
   config_option( bool, override_induck, false );

   config_option( bool, bt_head_hitbox, false );
   config_option( bool, bt_neck_hitbox, false );
   config_option( bool, bt_chest_hitbox, false );
   config_option( bool, bt_stomach_hitbox, false );
   config_option( bool, bt_pelvis_hitbox, false );
   config_option( bool, bt_arms_hitbox, false );
   config_option( bool, bt_legs_hitbox, false );
   config_option( bool, bt_feets_hitbox, false );

   config_option( bool, prefer_safety, false );
   config_option( bool, safe_shot, false );
   config_option( bool, safe_running, false );
   config_option( bool, safe_walking, false );
   config_option( bool, safe_inair, false );
   config_option( bool, safe_standing, false );
   config_option( bool, safe_backward, false );
   config_option( bool, safe_sideways, false );
   config_option( bool, safe_unresolved, false );
   config_option( bool, safe_lethal, false );
   config_option( bool, safe_induck, false );

   config_option( bool, prefer_head, false );
   config_option( bool, prefer_head_shot, false );
   config_option( bool, prefer_head_running, false );
   config_option( bool, prefer_head_walking, false );
   config_option( bool, prefer_head_inair, false );
   config_option( bool, prefer_head_standing, false );
   config_option( bool, prefer_head_backward, false );
   config_option( bool, prefer_head_sideways, false );
   config_option( bool, prefer_head_unresolved, false );
   config_option( bool, prefer_head_lethal, false );
   config_option( bool, prefer_head_induck, false );

   config_option( bool, prefer_body, false );
   config_option( bool, prefer_body_shot, false );
   config_option( bool, prefer_body_running, false );
   config_option( bool, prefer_body_walking, false );
   config_option( bool, prefer_body_inair, false );
   config_option( bool, prefer_body_standing, false );
   config_option( bool, prefer_body_backward, false );
   config_option( bool, prefer_body_sideways, false );
   config_option( bool, prefer_body_unresolved, false );
   config_option( bool, prefer_body_lethal, false );
   config_option( bool, prefer_body_induck, false );

   config_option( bool, mp_safety_head_hitbox, false );
   config_option( bool, mp_safety_neck_hitbox, false );
   config_option( bool, mp_safety_chest_hitbox, false );
   config_option( bool, mp_safety_stomach_hitbox, false );
   config_option( bool, mp_safety_pelvis_hitbox, false );
   config_option( bool, mp_safety_arms_hitbox, false );
   config_option( bool, mp_safety_legs_hitbox, false );
   config_option( bool, mp_safety_feets_hitbox, false );

   config_option( bool, on_shot_aa, false );
   config_option( bool, delay_shot_on_unducking, false );
   config_option( bool, delay_shot_on_peek, false );

   config_option( bool, shotdelay, 0 );
   config_option( float, shotdelay_amount, 0.0f );
   config_option( int, max_misses, 1 );

   //config_option( bool, improve_accuracy_on_shot, false );

#if 0
   config_option( bool, delay_shot, false );
   config_option( bool, lagfix, false );
#endif

   group_end( );
#pragma endregion

#pragma region Antiaim group
   group_begin( ANTIAIM_STATE );
   config_option( int, pitch, 0 );
   config_option( int, yaw, 0 );
   config_option( int, base_yaw, 0 );
   config_option( int, desync, 0 );
   config_option( float, desync_amount, 0.f );
   config_option( float, desync_amount_flipped, 0.f );

   config_option( bool, jitter, false );
   config_option( float, jitter_range, 15.0f );

   config_option( bool, spin, false );
   config_option( bool, spin_switch, false );
   config_option( float, spin_range, 30.0f );
   config_option( float, spin_speed, 4.0f );

   config_option( bool, autodirection, false );
   config_option( bool, autodirection_ignore_duck, false );

   config_option( int, extrapolation_ticks, 4 );
   config_option( float, autodirection_range, 24.0f );
   config_option( float, autodirection_delay, 0.3f );

   config_option( int, desync_autodir, 0 ); // Off/Hide Real/Hide Fake

   group_end( );

   group_begin( ANTIAIM );

   config_option( bool, enabled, false );
   config_option( bool, manual, false );

   config_keybind( desync_flip_bind );
   config_keybind( manual_left_bind );
   config_keybind( manual_right_bind );
   config_keybind( manual_back_bind );
   config_keybind( mouse_override );
   config_keybind( autodirection_override );

   config_option( bool, on_knife, false );
   config_option( bool, on_freeze_period, false );
   config_option( bool, on_ladder, false );
   config_option( bool, on_grenade, false );
   config_option( bool, on_dormant, false );

   config_option( int, break_lby, 0 );
   config_option( int, mirco_move_type, 0 );

   config_option( bool, move_sync, false );
   config_option( bool, hide_real_on_shot, false );
   config_option( bool, twist_speed, false );
   group_end( );
#pragma endregion

#pragma region Fakelag group
   group_begin( FAKELAG );

   config_option( bool, enabled, false );
   config_option( bool, auto_set, false ); // auto
   config_option( int, choke, 0 );
   config_option( float, variance, 0.f );
   config_option( int, lag_limit, 16 );
   config_option( bool, when_standing, false );
   config_option( bool, when_moving, false );
   config_option( bool, when_air, false );
   config_option( bool, when_dormant, false );
   config_option( bool, when_exploits, false );

   config_option( bool, auto_peekdist, false );
   config_option( float, peekdist, 10.0f );

   // alternative conditions
   config_option( bool, when_unducking, false );
   config_option( bool, when_land, false );
   config_option( bool, when_switching_weapon, false );
   config_option( bool, when_reloading, false );
   config_option( bool, when_peek, false );
   config_option( bool, when_velocity_change, false );
   config_option( bool, break_lag_compensation, false );
   config_option( int, alternative_choke, 0 );
   group_end( );
#pragma endregion

#pragma region ESP group
   // TODO: separate visuals from esp
   group_begin( ESP );
   config_option( bool, esp_enable, false );
   config_option( bool, box, false );
   config_option( int, box_type, 0 );
   config_option( FloatColor, box_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( bool, health, false );
   config_option( bool, animated_hp, false );
   config_option( int, health_pos, 0 );
   config_option( bool, armor_bar, false );
   config_option( int, armor_pos, 0 );

   config_option( bool, info, false );
   config_option( bool, draw_armor, false );
   config_option( bool, draw_ping, false );
   config_option( bool, draw_hit, false );
   config_option( bool, draw_taser, false );
   config_option( bool, draw_bombc4, false );
   config_option( bool, draw_scoped, false );
   config_option( bool, draw_money, false );
   config_option( bool, draw_distance, false );
   config_option( bool, draw_flashed, false );
   config_option( bool, draw_hostage, false );
   config_option( bool, draw_reloading, false );
   config_option( bool, draw_defusing, false );
   config_option( bool, team_check, false );
   config_option( bool, draw_ammo_bar, false );
   config_option( bool, name, false );
   config_option( FloatColor, name_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( bool, weapon, false );
   config_option( bool, weapon_ammo, false );
   config_option( bool, weapon_icon, false );
   config_option( bool, weapon_other, false );

   config_option( bool, skeleton, false );
   config_option( FloatColor, skeleton_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, aim_points, false );
   config_option( FloatColor, aim_points_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( int, info_pos, 0 );

   config_option( bool, chams_enabled, false );

   config_option( bool, chams_local, false );
   config_option( bool, chams_ghost, false );
   config_option( bool, chams_hands, false );
   config_option( bool, chams_weapon, false );
   config_option( bool, chams_enemy, false );
   config_option( bool, chams_teammate, false );
   config_option( bool, chams_death_enemy, false );
   config_option( bool, chams_death_teammate, false );
   config_option( bool, chams_lag, false );
   config_option( bool, chams_history, false );

   config_option( int, chams_local_mat, 0 );
   config_option( int, chams_desync_mat, 0 );
   config_option( int, hands_chams_mat, 0 );
   config_option( int, weapon_chams_mat, 0 );
   config_option( int, enemy_chams_mat, 0 );
   config_option( int, team_chams_mat, 0 );
   config_option( int, enemy_chams_death_mat, 0 );
   config_option( int, team_chams_death_mat, 0 );
   config_option( int, chams_lag_mat, 0 );
   config_option( int, chams_history_mat, 0 );
   config_option( int, chams_hitmatrix_mat, 0 );

   config_option( bool, enemy_chams_xqz, false );
   config_option( bool, enemy_chams_vis, false );
   config_option( bool, team_chams_xqz, false );
   config_option( bool, team_chams_vis, false );
   config_option( bool, enemy_death_chams_xqz, false );
   config_option( bool, enemy_death_chams_vis, false );
   config_option( bool, team_death_chams_xqz, false );
   config_option( bool, team_death_chams_vis, false );
   config_option( bool, enemy_death_chams_history_xqz, false );
   config_option( bool, enemy_death_chams_history_vis, false );

   config_option( bool, chams_local_outline, false );
   config_option( bool, chams_ghost_outline, false );
   config_option( bool, chams_hands_outline, false );
   config_option( bool, chams_weapon_outline, false );
   config_option( bool, chams_enemy_outline, false );
   config_option( bool, chams_teammate_outline, false );
   config_option( bool, chams_enemy_death_outline, false );
   config_option( bool, chams_teammate_death_outline, false );
   config_option( bool, chams_history_outline, false );
   config_option( bool, chams_lag_outline, false );
   config_option( bool, chams_hitmatrix_outline, false )

   config_option( bool, chams_local_shine, false );
   config_option( bool, chams_ghost_shine, false );
   config_option( bool, chams_hands_shine, false );
   config_option( bool, chams_weapon_shine, false );
   config_option( bool, chams_enemy_shine, false );
   config_option( bool, chams_teammate_shine, false );
   config_option( bool, chams_enemy_death_shine, false );
   config_option( bool, chams_teammate_death_shine, false );
   config_option( bool, chams_history_shine, false );
   config_option( bool, chams_lag_shine, false );
   config_option( bool, chams_hitmatrix_shine, false );

   config_option( bool, chams_local_pulse, false );
   config_option( bool, chams_ghost_pulse, false );
   config_option( bool, chams_hands_pulse, false );
   config_option( bool, chams_weapon_pulse, false );
   config_option( bool, chams_enemy_pulse, false );
   config_option( bool, chams_teammate_pulse, false );
   config_option( bool, chams_enemy_death_pulse, false );
   config_option( bool, chams_teammate_death_pulse, false );
   config_option( bool, chams_history_pulse, false );
   config_option( bool, chams_lag_pulse, false );
   config_option( bool, chams_hitmatrix_pulse, false );

   config_option( float, chams_local_outline_value, 0.f );
   config_option( float, chams_ghost_outline_value, 0.f );
   config_option( float, chams_hands_outline_value, 0.f );
   config_option( float, chams_weapon_outline_value, 0.f );
   config_option( float, chams_enemy_outline_value, 0.f );
   config_option( float, chams_teammate_outline_value, 0.f );
   config_option( float, chams_enemy_death_outline_value, 0.f );
   config_option( float, chams_teammate_death_outline_value, 0.f );
   config_option( float, chams_history_outline_value, 0.f );
   config_option( float, chams_lag_outline_value, 0.f );
   config_option( float, chams_hitmatrix_outline_value, 0.f );

   config_option( FloatColor, chams_local_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_ghost_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_hands_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_weapon_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_enemy_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_teammate_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_enemy_death_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_teammate_death_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_history_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_lag_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_hitmatrix_shine_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( FloatColor, chams_local_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_ghost_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_hands_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_weapon_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_enemy_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_teammate_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_enemy_death_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_teammate_death_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_history_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_lag_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_hitmatrix_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( FloatColor, chams_local_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_desync_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, hands_chams_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, weapon_chams_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, enemy_chams_color_vis, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, team_chams_color_vis, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, enemy_chams_color_xqz, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, team_chams_color_xqz, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, enemy_death_chams_color_vis, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, team_death_chams_color_vis, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, enemy_death_chams_color_xqz, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, team_death_chams_color_xqz, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_lag_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, chams_history_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, hitmatrix, false );
   config_option( FloatColor, hitmatrix_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( float, hitmatrix_time, 4.0f );

   config_option( bool, glow_team, false );
   config_option( bool, glow_enemy, false );
   config_option( bool, glow_weapons, false );
   config_option( bool, glow_grenade, false );
   config_option( FloatColor, glow_weapons_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, glow_team_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, glow_enemy_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, glow_grenade_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( int, glow_type, 0 );

   config_option( bool, NadePred, false );
   config_option( FloatColor, nade_pred_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, walls, false );
   config_option( bool, props, false );
   config_option( bool, skybox, false );

   config_option( FloatColor, wall_modulation, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, props_modulation, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, skybox_modulation, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, sound_esp, false );
   config_option( float, sound_esp_time, 4.f );
   config_option( float, sound_esp_radius, 10.f );
   config_option( int, sound_esp_type, 0 );
   config_option( FloatColor, sound_esp_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, beam_enabled, false );
   config_option( float, beam_life_time, 1.f );
   config_option( int, beam_type, 0 );
   config_option( float, beam_speed, 0.2f );
   config_option( float, beam_width, 1.5f );
   config_option( FloatColor, beam_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, offscren_enabled, false );
   config_option( int, offscren_size, 0 );
   config_option( int, offscren_distance, 0 );
   config_option( FloatColor, offscreen_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, offscreen_outline_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( bool, remove_scope, false );
   config_option( bool, remove_scope_zoom, false );
   config_option( bool, remove_scope_blur, false );
   config_option( bool, remove_recoil, false );
   config_option( bool, remove_flash, false );
   config_option( bool, remove_smoke, false );
   config_option( bool, remove_sleeves, false );
   config_option( bool, remove_hands, false );

   config_option( bool, preserve_killfeed, false );
   config_option( float, preserve_killfeed_time, 1.0f );

   config_option( bool, snaplines_enalbed, false );
   config_option( int, snaplines_pos, 0 );
   config_option( FloatColor, snaplines_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( bool, draw_c4_bar, false );
   config_option( bool, draw_c4, false );
   config_option( FloatColor, c4_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, autowall_crosshair, false );
   config_option( float, autowall_crosshair_height, 0.f );

   config_option( bool, skip_occulusion, false );

   config_option( bool, nades, false );
   config_option( FloatColor, nades_text_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( bool, nades_box, false );
   config_option( FloatColor, nades_box_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, dropped_weapons, false );
   config_option( int, dropped_weapons_type, 0 );
   config_option( FloatColor, dropped_weapons_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, skeleton_history, false );
   config_option( int, skeleton_history_type, 0 );
   config_option( FloatColor, skeleton_history_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( int, sky_changer, 0 );

   config_option( bool, extended_esp, false );

   config_option( bool, fov_crosshair, false );
   config_option( FloatColor, fov_crosshair_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );



   config_option( bool, force_sniper_crosshair, false );

   config_option( bool, event_bomb, false );
   config_option( bool, event_dmg, false );
   config_option( bool, event_buy, false );
   config_option( bool, event_resolver, false );
   config_option( bool, event_misc, false );
   config_option( bool, event_exploits, false );
   config_option( bool, event_console, false );
   config_option( bool, event_aimbot, false );

   config_option( bool, aa_indicator, false );
   config_option( int, aa_indicator_type, 0 );
   config_option( FloatColor, aa_indicator_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, stat_logger, false );

   config_option( bool, vizualize_hitmarker, false );
   config_option( bool, vizualize_hitmarker_damage, false );
   config_option( bool, vizualize_angles, false );

   config_option( bool, angle_lines, false );

   config_option( bool, molotov_indicator, false );
   config_option( bool, smoke_indicator, false );
   config_option( bool, warning_grenades, false );

   config_option( bool, fade_esp, false );

   config_option( bool, dynamic_scope_lines, false );

   config_option( bool, kill_effect, false );
   config_option( float, kill_effect_time, 0.5f );

   config_option( bool, zeus_distance, false );

   config_option( float, world_fov, 90.f );

   config_option( bool, draw_hitboxes, false );

   config_option( bool, blur_in_scoped, false );
   config_option( float, blur_in_scoped_value, 0.5f );

   config_option( int, event_logger_type, 0 );

   config_option( bool, watermark, false );
   config_option( bool, spectator_list, false );
   config_option( bool, keybind_list, false );
   config_option( bool, indicators_list, false );
   config_option( bool, aspect_ratio, false );
   config_option( float, aspect_ratio_value, 0.f );

   config_option( bool, zoom, false );
   config_option( float, zoom_value, 0.f );
   config_keybind( zoom_key );

   config_option( bool, remove_blur_effect, false );
   config_option( bool, remove_post_proccesing, false );

   config_option( FloatColor, server_impacts, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, client_impacts, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( FloatColor, hitboxes_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, ammo_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, pulse_offscreen_alpha, false );
   config_option( bool, offscreen_outline, false );
   config_option( bool, text_indicators, false );

   config_option( float, keybind_x, 100 );
   config_option( float, keybind_y, 100 );

   config_option( float, hitlogger_x, 200 );
   config_option( float, hitlogger_y, 200 );

   config_option( float, spectator_x, 300 );
   config_option( float, spectator_y, 300 );

   config_option( float, indicators_x, 400 );
   config_option( float, indicators_y, 400 );

   config_option( bool, fullbright, false );

   config_option( bool, bloom_effect, false );

   config_option( float, bloom_scale, -1.f );
   config_option( float, exposure_scale, -1.f );
   config_option( float, bloom_scale_minimun, -1.f );
   config_option( float, bloom_exponent, -1.f );
   config_option( float, bloom_saturation, -1.f );
   config_option( float, tonemap_percent_target, -1.f );
   config_option( float, tonemap_percent_bright_pixels, -1.f );
   config_option( float, tonemap_min_avg_lum, -1.f );
   config_option( float, tonemap_rate, -1.f );

   config_option( float, model_brightness, 0 );
   config_option( FloatColor, bloom_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( bool, dark_mode, false );
   config_option( float, dark_mode_value, 20.f );
   config_option( FloatColor, dark_mode_shade, FloatColor( 0.17f, 0.16f, 0.18f, 1.0f ) );

   config_option( bool, indicator_side, false );
   config_option( bool, indicator_lc, false );
   config_option( bool, indicator_lag, false );
   config_option( bool, indicator_exploits, false );
   config_option( bool, indicator_fake, false );
   config_option( bool, indicator_fake_duck, false );

   config_option( bool, fog_effect, false );
   config_option( bool, fog_blind, false );
   config_option( float, fog_density, 0.f );
   config_option( float, fog_hdr_scale, 0.f );
   config_option( int, fog_distance, 1000 );
   config_option( FloatColor, fog_color, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
   config_option( FloatColor, fog_color_secondary, FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

   config_option( bool, chams_on_death_enemy, false );
   config_option( bool, chams_on_death_teamate, false );

   config_option( bool, watermark_name, false );
   config_option( bool, watermark_ip, false );
   config_option( bool, watermark_fps, false );
   config_option( bool, watermark_ping, false );
   config_option( bool, watermark_time, false );

   group_end( );
#pragma endregion

#pragma region MISC group
   group_begin( MISC );

   config_option( FloatColor, menu_ascent, FloatColor( 0.0f, 0.54f, 1.0f, 1.0f ) );

   config_option( bool, active, false );
   config_option( bool, autojump, false );
   config_option( bool, autostrafer, false );
   config_option( bool, autostrafer_wasd, false );
   config_option( float, autostrafer_retrack, 2.0f );

   config_option( bool, fastduck, false );
   config_option( bool, minijump, false );
   config_option( bool, edgejump, false );
   config_keybind( edgejump_bind );
   config_option( bool, duckjump, false );
   config_option( bool, quickstop, false );
   config_option( bool, accurate_walk, false );
   config_option( bool, slide_walk, false );
   config_option( bool, fakeduck, false );
   config_keybind( fakeduck_bind );

   config_option( bool, third_person, false );
   config_option( bool, off_third_person_on_grenade, false );
   config_keybind( third_person_bind );
   config_option( float, third_person_dist, 100.f );

   config_option( bool, slow_walk, false );
   config_keybind( slow_walk_bind );
   config_option( float, slow_walk_speed, 100.f );

   config_option( float, viewmodel_fov, 68.f );
   config_option( bool, viewmodel_change, false );
   config_option( float, viewmodel_x, 2.5f );
   config_option( float, viewmodel_y, 0.f );
   config_option( float, viewmodel_z, -2.f );

   config_option( bool, indicators_enabled, false );
   config_option( bool, indicator_lc, false );
   config_option( bool, indicator_lby_update, false );

   config_option( bool, hitsound, false );
   config_option( int, hitsound_type, 0 );

   config_option( bool, clantag_changer, false );

   config_option( bool, server_crasher, false );
   config_option( int, server_crasher_bind, 0 );
   config_option( int, server_crasher_msges, 0 );
   config_option( int, server_crasher_packets, 0 );

   config_option( bool, name_changer, false );
   config_option( std::string, name_changer_str, std::string( "EeXOMI" ) );

   config_option( bool, chat_spammer, false );

   config_option( bool, lag_exploit, false );
   config_option( int, lag_exploit_key, 0 );

   config_option( bool, autobuy_enabled, false );
   config_option( int, autobuy_first_weapon, 0 );
   config_option( int, autobuy_second_weapon, 0 );
   config_option( bool, autobuy_flashbang, false );
   config_option( bool, autobuy_smokegreanade, false );
   config_option( bool, autobuy_molotovgrenade, false );
   config_option( bool, autobuy_hegrenade, false );
   config_option( bool, autobuy_decoy, false );
   config_option( bool, autobuy_armor, false );
   config_option( bool, autobuy_zeus, false );
   config_option( bool, autobuy_defusekit, false );

   config_option( bool, impacts_spoof, false );
   config_option( bool, server_impacts_spoof, false );
   config_option( bool, server_issue_impacts_spoof, false );

   config_option( bool, hit_logger_frame, false );

   config_option( bool, knife_bot, false );
   config_option( int, knife_bot_type, 0 );

   config_option( bool, sv_pure_bypass, false );

   config_option( bool, extended_backtrack, false );
   config_option( float, extended_backtrack_time, 0.2f );

   config_option( bool, anti_untrusted, true );
   config_option( bool, obs_bypass, false );

   config_option( bool, autopeek, false );
   config_option( bool, autopeek_visualise, false );
   config_keybind( autopeek_bind );
   config_option( FloatColor, autopeek_color, FloatColor( 1.0f, 1.0f, 1.0f, 0.5f ) );

   config_option( bool, skins_model, false );
   config_option( int, ct_model_type, 0 );
   config_option( int, tt_model_type, 0 );
   config_option( int, local_model_type, 0 );

   config_option( bool, money_revealer, false );
   config_option( bool, auto_accept, false );

   config_option( bool, disable_fog, false );
   config_option( bool, disable_shadows, false );
   config_option( bool, disable_teammates, false );
   config_option( bool, disable_enemies, false );
   config_option( bool, disable_weapons, false );
   config_option( bool, disable_ragdolls, false );
   config_option( bool, disable_decals, false );

   config_option( bool, unlock_inventory, false );

   config_option( int, server_region, 0 );
   config_option( int, search_max_ping, 100 );

   config_option( bool, override_player_view, false );
   config_keybind( override_player_view_key );

   config_option( bool, ingame_radar, false );

   group_end( );

#pragma endregion

   add_child_group_ex( MENU, menu );
   add_child_group_ex( RAGE_GENERAL, rage );

   RAGE* rage_option( );

   add_child_group_ex( RAGE, rage_default );
   add_child_group_ex( RAGE, rage_pistols );
   add_child_group_ex( RAGE, rage_heavypistols );
   add_child_group_ex( RAGE, rage_rifles );
   add_child_group_ex( RAGE, rage_awp );
   add_child_group_ex( RAGE, rage_scout );
   add_child_group_ex( RAGE, rage_autosnipers );
   add_child_group_ex( RAGE, rage_smgs );
   add_child_group_ex( RAGE, rage_heavys );
   add_child_group_ex( RAGE, rage_shotguns );
   add_child_group_ex( RAGE, rage_taser );

   add_child_group_ex( ANTIAIM, antiaim );
   add_child_group_ex( ANTIAIM_STATE, antiaim_stand );
   add_child_group_ex( ANTIAIM_STATE, antiaim_move );
   add_child_group_ex( ANTIAIM_STATE, antiaim_air );
   add_child_group_ex( LEGIT_GENERAL, legit );
   add_child_group_ex( FAKELAG, fakelag );
   add_child_group_ex( ESP, esp );
   add_child_group_ex( MISC, misc );
   add_child_group_ex( skin_changer_global_data, m_global_skin_changer );
   CArrayGroup<skin_changer_data> m_skin_changer;
   CArrayGroup<LEGIT> legit_weapons;

   add_child_group_ex( LEGIT, legit_pistols );
   add_child_group_ex( LEGIT, legit_heavypistols );
   add_child_group_ex( LEGIT, legit_rifles );
   add_child_group_ex( LEGIT, legit_snipers );
   add_child_group_ex( LEGIT, legit_autosnipers );
   add_child_group_ex( LEGIT, legit_smgs );
   add_child_group_ex( LEGIT, legit_heavys );
   add_child_group_ex( LEGIT, legit_shotguns );

   GLOBAL globals;

   // convars
   ConVar* sv_accelerate;
   ConVar* sv_airaccelerate;
   ConVar* sv_gravity;
   ConVar* sv_jump_impulse;

   ConVar* m_yaw;
   ConVar* m_pitch;
   ConVar* sensitivity;

   ConVar* cl_sidespeed;
   ConVar* cl_forwardspeed;
   ConVar* cl_upspeed;

   ConVar* weapon_recoil_scale;
   ConVar* view_recoil_tracking;

   ConVar* r_jiggle_bones;

   ConVar* mp_friendlyfire;

   ConVar* sv_maxunlag;
   ConVar* sv_minupdaterate;
   ConVar* sv_maxupdaterate;

   ConVar* sv_client_min_interp_ratio;
   ConVar* sv_client_max_interp_ratio;

   ConVar* cl_interp_ratio;
   ConVar* cl_interp;
   ConVar* cl_updaterate;

   ConVar* game_type;
   ConVar* game_mode; 

   ConVar* ff_damage_bullet_penetration;
   ConVar* ff_damage_reduction_bullets;

   ConVar* mp_damage_scale_ct_head;
   ConVar* mp_damage_scale_t_head;
   ConVar* mp_damage_scale_ct_body;
   ConVar* mp_damage_scale_t_body;

   ConVar* viewmodel_fov;
   ConVar* viewmodel_offset_x;
   ConVar* viewmodel_offset_y;
   ConVar* viewmodel_offset_z;

   ConVar* sv_show_impacts;

   ConVar* molotov_throw_detonate_time;
   ConVar* weapon_molotov_maxdetonateslope;
   ConVar* net_client_steamdatagram_enable_override;
   ConVar* mm_dedicated_search_maxping;

   ConVar* developer;
   ConVar* con_enable;
   ConVar* con_filter_enable;
   ConVar* con_filter_text;
   ConVar* con_filter_text_out;
   ConVar* contimes;

   ConVar* sv_clockcorrection_msecs;

   void Create( );
};

struct WeaponName_t {
   constexpr WeaponName_t( int32_t definition_index, const char* name ) :
	  definition_index( definition_index ),
	  name( name ) {
   }

   int32_t definition_index = 0;
   const char* name = nullptr;
};

extern CVariables g_Vars;
