#include "Miscellaneous.hpp"
#include "displacement.hpp"

#include "Source.hpp"
#include "player.hpp"

namespace Source
{
   inline float rgb_to_srgb( float flLinearValue ) {
	  return flLinearValue;
	  // float x = Math::Clamp( flLinearValue, 0.0f, 1.0f );
	  // return ( x <= 0.0031308f ) ? ( x * 12.92f ) : ( 1.055f * powf( x, ( 1.0f / 2.4f ) ) ) - 0.055f;
   }

   class C_Miscellaneous : public Miscellaneous {
   public:
	  static Miscellaneous* Get( );
	  virtual void Main( );

	  C_Miscellaneous( ) { };
	  virtual ~C_Miscellaneous( ) {
	  }

   private:
	  const char* skynames[ 16 ] = {
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

	  // wall modulation stuff
	  FloatColor walls = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
	  FloatColor props = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
	  FloatColor skybox = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );

	  // clantag
	  int clantag_step = 0;

	  virtual void ModulateWorld( );
	  virtual void ChatSpam( );
	  virtual void ClantagChanger( );
	  virtual void ViewModelChanger( );
	  virtual void SkyboxCahanger( );
	  void ServerRegionChanger( );
   };

   C_Miscellaneous g_Misc;
   Miscellaneous* C_Miscellaneous::Get( ) {
	  return &g_Misc;
   }

   void C_Miscellaneous::Main( ) {
	  ModulateWorld( );
	  ServerRegionChanger( );

	  if ( !g_Vars.globals.HackIsReady )
		 return;

	  ChatSpam( );
	  ClantagChanger( );
	  ViewModelChanger( );
	  SkyboxCahanger( );
	  
   }

   void C_Miscellaneous::ModulateWorld( ) {
	  static auto cl_modelfastpath = Source::m_pCvar->FindVar( XorStr( "cl_modelfastpath" ) );
	  cl_modelfastpath->SetValue( !g_Vars.esp.props );

	  if ( !g_Vars.globals.HackIsReady ) {
		 walls = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		 props = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		 skybox = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		 return;
	  }

	  if ( !C_CSPlayer::GetLocalPlayer( ) )
		 return;

	  auto p = g_Vars.esp.props ? g_Vars.esp.props_modulation : FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
	  auto w = g_Vars.esp.walls ? g_Vars.esp.wall_modulation : FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
	  auto s = g_Vars.esp.skybox ? g_Vars.esp.skybox_modulation : FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );

	  if ( g_Vars.esp.dark_mode ) {
		 float power = g_Vars.esp.dark_mode_value / 200.f;
		 FloatColor shade = g_Vars.esp.dark_mode_shade;

		 w = FloatColor( shade.r - power, shade.g - power, shade.b - power, 1.f );
	  }

	  if ( walls != w || props != p || skybox != s ) {
		 walls = w;
		 props = p;
		 skybox = s;

		 auto invalid_material = Source::m_pMatSystem->InvalidMaterial( );
		 for ( auto i = Source::m_pMatSystem->FirstMaterial( );
			i != invalid_material;
			i = Source::m_pMatSystem->NextMaterial( i ) ) {
			auto material = Source::m_pMatSystem->GetMaterial( i );

			if ( !material || material->IsErrorMaterial( ) )
			   continue;

			FloatColor color = walls;
			auto group = material->GetTextureGroupName( );

			if ( !material->GetName( ) )
			   continue;

			if ( *group == 'W' ) { // world textures
			   if ( group[ 4 ] != 'd' )
				  continue;
			   color = walls;
			} else if ( *group == 'S' ) { // staticprops & skybox
			   auto thirdCharacter = group[ 3 ];
			   if ( thirdCharacter == 'B' ) {
				  color = skybox;
			   } else if ( thirdCharacter == 't' && group[ 6 ] == 'P' ) {
				  color = props;
			   } else {
				  continue;
			   }
			} else {
			   continue;
			}

			color.r = rgb_to_srgb( color.r );
			color.g = rgb_to_srgb( color.g );
			color.b = rgb_to_srgb( color.b );

			material->ColorModulate( color.r, color.g, color.b );
			material->AlphaModulate( color.a );
		 }
	  }
   }

   void C_Miscellaneous::ChatSpam( ) {
	  if ( !g_Vars.misc.chat_spammer )
		 return;

	  Source::m_pEngine->ClientCmd( XorStr( "say eexomi good" ) );
   }

   void C_Miscellaneous::ClantagChanger( ) {
	  static bool run_once = false;
	  static auto fnClantagChanged = ( int( __fastcall* )( const char*, const char* ) ) Engine::Displacement.Function.m_uClanTagChange;

	  if ( !g_Vars.misc.clantag_changer ) {
		 if ( run_once ) {
			fnClantagChanged( "", "" );
			run_once = false;
		 }

		 return;
	  }

	  run_once = true;

	  int latency = 0;
	  auto tickrate = int( 1.0f / Source::m_pGlobalVars->interval_per_tick );
	  auto netchannel = Encrypted_t<INetChannel>( Source::m_pEngine->GetNetChannelInfo( ) );
	  if ( netchannel.IsValid( ) ) {
		 latency = TIME_TO_TICKS( netchannel->GetAvgLatency( 0 ) );
	  }

   #ifdef BETA_MODE
	  static std::string clantag = XorStr( "eexomi beta" );
   #else 
	  static std::string clantag = XorStr( "eexomi" );
   #endif

	  int step = ( latency + Source::m_pGlobalVars->tickcount ) / ( tickrate / 2 ) % clantag.size( );
	  if ( step != clantag_step ) {
		 clantag_step = step;
		 std::string buffer;
		 std::string buffer2;

		 auto delta = 16u - clantag.size( );
		 for ( int i = 0; i < std::min( 16u, clantag.size( ) ); ++i ) {
			buffer.push_back( clantag[ i ] );
		 }

		 for ( size_t i = 0u; i < delta; ++i ) {
			buffer.push_back( ' ' );
		 }

		 buffer2 = buffer;

		 auto delta2 = ( clantag_step - int( &buffer[ 0 ] ) );
		 for ( int i = 0; i < 15; ++i ) {
			auto tmp = &buffer[ i ];
			*tmp = buffer2[ ( int ) & tmp[ delta2 ] % 15 ];
		 }

		 fnClantagChanged( buffer.c_str( ), buffer.c_str( ) );
	  }
   }

   void C_Miscellaneous::ViewModelChanger( ) {
	  if ( g_Vars.misc.viewmodel_change ) {
		 g_Vars.viewmodel_fov->SetValue( g_Vars.misc.viewmodel_fov );
		 g_Vars.viewmodel_offset_x->SetValue( g_Vars.misc.viewmodel_x );
		 g_Vars.viewmodel_offset_y->SetValue( g_Vars.misc.viewmodel_y );
		 g_Vars.viewmodel_offset_z->SetValue( g_Vars.misc.viewmodel_z - 10 );
	  }
   }

   void C_Miscellaneous::SkyboxCahanger( ) {
	  static uintptr_t adr = 0;
	  if ( !adr )
		 adr = Memory::Scan( ( std::uintptr_t )GetModuleHandleA( XorStr( "engine.dll" ) ), XorStr( "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45" ) );
	  static auto fnLoadNamedSkys = ( void( __fastcall* )( const char* ) )adr;
	  static ConVar* default_skyname = Source::m_pCvar->FindVar( XorStr( "sv_skyname" ) );
	  if ( default_skyname ) {
		 static int iOldSky = 0;
		 char* sky_name = g_Vars.esp.sky_changer != 0 ? ( char* ) skynames[ g_Vars.esp.sky_changer ] : ( char* ) default_skyname->GetString( );

		 if ( iOldSky != g_Vars.esp.sky_changer ) {
			fnLoadNamedSkys( sky_name );
			iOldSky = g_Vars.esp.sky_changer;
		 }
	  }
   }

   void C_Miscellaneous::ServerRegionChanger( ) {
	  static int max_ping = 0;
	  static int server_region = 0;
	  const char* servers[] = {
		 XorStr( "All" ),
		 XorStr( "ams" ),
		 XorStr( "atl" ),
		 XorStr( "bom" ),
		 XorStr( "dxb" ),
		 XorStr( "fra" ),
		 XorStr( "eat" ),
		 XorStr( "gru" ),
		 XorStr( "hkg" ),
		 XorStr( "iad" ),
		 XorStr( "jnb" ),
		 XorStr( "lax" ),
		 XorStr( "lhr" ),
		 XorStr( "lim" ),
		 XorStr( "maa" ),
		 XorStr( "mad" ),
		 XorStr( "man" ),
		 XorStr( "mwh" ),
		 XorStr( "okc" ),
		 XorStr( "ord" ),
		 XorStr( "par" ),
		 XorStr( "scl" ),
		 XorStr( "sea" ),
		 XorStr( "sgp" ),
		 XorStr( "sto" ),
		 XorStr( "sto2" ),
		 XorStr( "syd" ),
		 XorStr( "tuk" ),
		 XorStr( "tyo" ),
		 XorStr( "vie" ),
		 XorStr( "waw" ),
	  };
	  if ( max_ping != g_Vars.misc.search_max_ping ) {
		 g_Vars.mm_dedicated_search_maxping->SetValue( g_Vars.misc.search_max_ping );
		 max_ping = g_Vars.misc.search_max_ping;
	  }

	  if ( server_region != g_Vars.misc.server_region && g_Vars.misc.server_region > 0 ) {
		 g_Vars.net_client_steamdatagram_enable_override->SetValue( 1 );
		 std::string str = XorStr( "sdr SDRClient_ForceRelayCluster " );
		 str += servers[ g_Vars.misc.server_region ];
		 Source::m_pEngine->ExecuteClientCmd( str.c_str( ) );
		 server_region = g_Vars.misc.server_region;
	  }
	  else {
		 if ( g_Vars.net_client_steamdatagram_enable_override->GetInt( ) != 0 )
			g_Vars.net_client_steamdatagram_enable_override->SetValue( 0 );
	  }
   }

   Miscellaneous* Miscellaneous::Get( ) {
	  static C_Miscellaneous instance;
	  return &instance;
   }
}
