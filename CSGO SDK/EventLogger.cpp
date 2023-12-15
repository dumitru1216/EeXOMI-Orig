#include "EventLogger.hpp"
#include "source.hpp"
#include "Render.hpp"

static constexpr int MAX_EVENT_SIZE = 10;

struct MessageInfo {
   MessageInfo( std::string text, FloatColor color =   FloatColor( 1.f, 1.f, 1.f, 1.f ), int time = 5.f ) {
	  this->m_Text = text;
	  this->m_Color = color;
	  this->m_flTime = time;
   }

   std::string m_Text;
   FloatColor m_Color;
   float m_flTime;
   int m_iFadeIn;
};

std::vector<MessageInfo> eventLog;

class CLogger : public ILoggerEvent {
public:
   void Main( ) override;

   void PushEvent( std::string msg, FloatColor clr ) override;

private:
};

Encrypted_t<ILoggerEvent> ILoggerEvent::Get( ) {
   static CLogger instance;
   return &instance;
}

void CLogger::Main( ) {
   if ( eventLog.size( ) < 1 )
	  return;

   if ( eventLog.size( ) > MAX_EVENT_SIZE )
	  eventLog.erase( eventLog.begin( ) + 1 );

   Vector2D pos = Vector2D( 8.0f, 7.0f );

   Render::Get( )->SetTextFont( FONT_VERDANA );

   static float fontTall = Render::Get( )->CalcTextSize( XorStr( "ABCI0123456789" ) ).y;
   static float last_yaw = 0.f;
   // valve code at it's finest (lmao) https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/console.cpp#L978-L1010
   for ( size_t i = 0; i < eventLog.size( ); ++i ) {
	  if ( g_Vars.esp.event_logger_type == 1 ) {
		 auto curLog = eventLog.at( i );

		 float ratio = 1.f;

		 if ( eventLog.at( i ).m_flTime <= 1.f )
			ratio = eventLog.at( i ).m_flTime / 1.f;

		 float step = 255.f / 0.9f * Source::m_pGlobalVars->frametime;

		 if ( eventLog.at( i ).m_flTime >= 4.0f )
			eventLog.at( i ).m_iFadeIn = 0.f;
		 else
			eventLog.at( i ).m_iFadeIn = eventLog.at( i ).m_iFadeIn + ( step * 2.f );

		 if ( eventLog.at( i ).m_iFadeIn > 244 )
			eventLog.at( i ).m_iFadeIn = 255;

		 float new_ratio = 1.f - ratio;
		 float x = -new_ratio * 300;
		 float x2 = -new_ratio * 600;
		 int y = 12 * i * 2 - 5;
		 int y2 = 12 * i * 2 + 3;

		 if ( eventLog.size( ) > 0 ) {
		 #ifdef BETA_MODE
			std::string message = XorStr( "[eexomi] [beta] " ) + eventLog.at( i ).m_Text;
		 #else
			std::string message = XorStr( "[eexomi]" ) + eventLog.at( i ).m_Text;
		 #endif
			Render::Get( )->AddRectFilled( Vector4D( ( x ), y, x + Render::Get( )->CalcTextSize( message.c_str( ) ).x + 55, y + 24 ), FloatColor( 33, 150, 243, 255 ) );
			Render::Get( )->AddRectFilled( Vector4D( ( x2 ), y, x2 + Render::Get( )->CalcTextSize( message.c_str( ) ).x + 35, y + 24 ), FloatColor( 12, 12, 12, eventLog.at( i ).m_iFadeIn ) );
		 #ifdef BETA_MODE
			Render::Get( )->AddText( Vector2D( x2 + 5, y2 ), FloatColor( 33, 150, 243, 255 ), 0, XorStr( "[eexomi] [beta] " ) );
		 #else
			Render::Get( )->AddText( Vector2D( x2 + 5, y2 ), FloatColor( 33, 150, 243, 255 ), 0, XorStr( "[eexomi] " ) );
		 #endif

		 #ifdef BETA_MODE
			Render::Get( )->AddText( Vector2D( x2 + 130, y2 ), eventLog[ i ].m_Color, DROP_SHADOW, eventLog[ i ].m_Text.c_str( ) );
		 #else
			Render::Get( )->AddText( Vector2D( x2 + 80, y2 ), eventLog[ i ].m_Color, DROP_SHADOW, eventLog[ i ].m_Text.c_str( ) );
		 #endif
		 }

		 eventLog.at( i ).m_flTime -= 0.01f;
		 if ( eventLog.at( i ).m_flTime <= -1.f )
			eventLog.erase( eventLog.begin( ) + i );
	  } else {
		 eventLog[ i ].m_flTime -= Source::m_pGlobalVars->frametime;

		 if ( eventLog[ i ].m_flTime < 0.5f ) {
			float f = Math::Clamp( eventLog[ i ].m_flTime, 0.0f, .5f ) / .5f;

			if ( i == 0 ) {
			   eventLog[ i ].m_Color.a = f;

			   if ( f < 0.2f ) {
				  pos.y -= fontTall * ( 1.0f - f / 0.2f ) - 2.0f;
			   }
			}
		 } else {
			eventLog[ i ].m_Color.a = 1.0f;
		 }

		 if ( i == 0 && eventLog[ i ].m_Color.a <= .1f ) {
			eventLog.erase( eventLog.begin( ) + i );
			if ( eventLog.size( ) > 0 )
			   eventLog[ i ].m_flTime = 0.75f;
		 }

		 if ( eventLog.size( ) > 0 ) {
			Render::Get( )->AddText( Vector2D( pos.x, pos.y + ( i * 16 ) ), eventLog[ i ].m_Color, DROP_SHADOW, eventLog[ i ].m_Text.c_str( ) );
		 }
	  }
   }
}

void CLogger::PushEvent( std::string msg, FloatColor clr ) {
   eventLog.push_back( MessageInfo( msg, clr ) );

   if ( g_Vars.esp.event_console ) {
   #ifdef BETA_MODE
	  Source::m_pCvar->ConsoleColorPrintf( Color( 0, 191, 255 ), XorStr( "[eexomi] [beta] " ) );
   #else
	  Source::m_pCvar->ConsoleColorPrintf( Color( 0, 191, 255 ), XorStr( "[eexomi] " ) );
   #endif
	  Source::m_pCvar->ConsoleColorPrintf( Color( int( clr.r * 255.0f ), int( clr.g * 255.0f ), int( clr.b * 255.0f ), int( clr.a * 255.0f ) ), std::string( msg + XorStr( "\n" ) ).c_str( ) );
   }
}
