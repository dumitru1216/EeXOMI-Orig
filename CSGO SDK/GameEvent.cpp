#include "GameEvent.hpp"
#include "EventLogger.hpp"
#include "source.hpp"
#include "FnvHash.hpp"
#include "player.hpp"
#include "LagCompensation.hpp"
#include <sstream>
#include "BulletBeamTracer.hpp"
#include "Hitmarker.hpp"
#include "RoundFireBulletsStore.hpp"
#include "HitDamageVisualize.hpp"
#include "AutoBuy.hpp"
#include "core.hpp"
#include "Hitsound.hpp"
#include "LegitBot.hpp"
#include "ESP.hpp"
#include "CChams.hpp"
#include "Resolver.hpp"
#include "TickbaseShift.hpp"
#pragma comment(lib,"Winmm.lib")

// BOLSHE VIRTUALNIX TABLICH BOGU VMT
// ahahhaahhaa good m3m3

#define ADD_GAMEEVENT(n)  Source::m_pGameEvent->AddListener(this, #n, false)

class C_GameEvent : public GameEvent {
public: // GameEvent interface
   virtual void Register( );
   virtual void Shutdown( );

   C_GameEvent( ) { };
   virtual ~C_GameEvent( ) { };
public: // IGameEventListener
   virtual void FireGameEvent( IGameEvent* event );
   virtual int  GetEventDebugID( void );
};

Encrypted_t<GameEvent> GameEvent::Get( ) {
   static C_GameEvent instance;
   return &instance;
}

void C_GameEvent::Register( ) {
   ADD_GAMEEVENT( player_hurt );
   ADD_GAMEEVENT( bullet_impact );
   ADD_GAMEEVENT( weapon_fire );
   ADD_GAMEEVENT( player_death );
   ADD_GAMEEVENT( round_start );
   ADD_GAMEEVENT( item_purchase );
   ADD_GAMEEVENT( bomb_begindefuse );
   ADD_GAMEEVENT( bomb_pickup );
   ADD_GAMEEVENT( bomb_beginplant );
   ADD_GAMEEVENT( item_pickup );
   ADD_GAMEEVENT( round_mvp );
   ADD_GAMEEVENT( grenade_thrown );
   ADD_GAMEEVENT( buytime_ended );
   ADD_GAMEEVENT( round_end );
   ADD_GAMEEVENT( game_newmap );
}

void C_GameEvent::Shutdown( ) {
   Source::m_pGameEvent->RemoveListener( this );
}

void C_GameEvent::FireGameEvent( IGameEvent* pEvent ) {
   if ( !pEvent )
	  return;

   C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );
   if ( !LocalPlayer || !Source::m_pEngine->IsInGame( ) )
	  return;

   auto event_hash = hash_32_fnv1a_const( pEvent->GetName( ) );

   IHitmarker::Get( )->GameEventCallback( pEvent );
   Engine::C_Resolver::Get( )->EventCallback( pEvent, event_hash );
   g_Vars.globals.m_bLocalPlayerHarmedThisTick = false;

   auto HitgroupToString = [] ( int hitgroup ) -> std::string {
	  switch ( hitgroup ) {
		 case Hitgroup_Generic:
			return XorStr( "generic" );
		 case Hitgroup_Head:
			return XorStr( "head" );
		 case Hitgroup_Chest:
			return XorStr( "chest" );
		 case Hitgroup_Stomach:
			return XorStr( "stomach" );
		 case Hitgroup_LeftArm:
			return XorStr( "left arm" );
		 case Hitgroup_RightArm:
			return XorStr( "right arm" );
		 case Hitgroup_LeftLeg:
			return XorStr( "left leg" );
		 case Hitgroup_RightLeg:
			return XorStr( "right leg" );
		 case Hitgroup_Gear:
			return XorStr( "gear" );
	  }
	  return XorStr( "generic" );
   };

   static auto sv_showimpacts_time = Source::m_pCvar->FindVar( XorStr( "sv_showimpacts_time" ) );

   // Force constexpr hash computing 
   switch ( event_hash ) {
	  case hash_32_fnv1a_const( "game_newmap" ):
	  {
		 Engine::LagCompensation::Get( )->ClearLagData( );
		 g_Vars.globals.m_bNewMap = true;
	  }
	  case hash_32_fnv1a_const( "bullet_impact" ):
	  {
		 if ( Source::m_pEngine->GetPlayerForUserID( pEvent->GetInt( XorStr( "userid" ) ) ) == Source::m_pEngine->GetLocalPlayer( ) && LocalPlayer && !LocalPlayer->IsDead( ) ) {
			float x = pEvent->GetFloat( XorStr( "x" ) ), y = pEvent->GetFloat( XorStr( "y" ) ), z = pEvent->GetFloat( XorStr( "z" ) );
			if ( g_Vars.esp.beam_enabled )
			   IBulletBeamTracer::Get( )->PushBeamInfo( { Source::m_pGlobalVars->curtime, Vector( x, y, z ) } );

			int color[ 4 ] = { g_Vars.esp.server_impacts.r * 255, g_Vars.esp.server_impacts.g * 255, g_Vars.esp.server_impacts.b * 255, g_Vars.esp.server_impacts.a * 255 };

			if ( g_Vars.misc.server_impacts_spoof ) // draw server impact
			   Source::m_pDebugOverlay->AddBoxOverlay( Vector( x, y, z ), Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), QAngle( 0, 0, 0 ), color[ 0 ], color[ 1 ], color[ 2 ], color[ 3 ],
				  sv_showimpacts_time->GetFloat( ) );
		 }

		 break;
	  }
	  case hash_32_fnv1a_const( "round_end" ):
	  {
		 //IRoundFireBulletsStore::Get( )->EventCallBack( pEvent, 1, nullptr );
		 break;
	  }
	  case hash_32_fnv1a_const( "round_freeze_end" ):
	  {
		 g_Vars.globals.IsRoundFreeze = false;
		 break;
	  }
	  case hash_32_fnv1a_const( "round_prestart" ):
	  {
		 g_Vars.globals.IsRoundFreeze = true;
		 break;
	  }
	  case hash_32_fnv1a_const( "player_hurt" ):
	  {
		 auto enemy = pEvent->GetInt( XorStr( "userid" ) );
		 auto attacker = pEvent->GetInt( XorStr( "attacker" ) );
		 auto remaining_health = pEvent->GetString( XorStr( "health" ) );
		 auto dmg_to_health = pEvent->GetInt( XorStr( "dmg_health" ) );
		 auto hitgroup = pEvent->GetInt( XorStr( "hitgroup" ) );

		 auto enemy_index = Source::m_pEngine->GetPlayerForUserID( enemy );
		 auto attacker_index = Source::m_pEngine->GetPlayerForUserID( attacker );
		 auto pEnemy = C_CSPlayer::GetPlayerByIndex( enemy_index );
		 auto pAttacker = C_CSPlayer::GetPlayerByIndex( attacker_index );

		 player_info_t attacker_info;
		 player_info_t enemy_info;

		 if ( pEnemy && pAttacker && Source::m_pEngine->GetPlayerInfo( attacker_index, &attacker_info ) && Source::m_pEngine->GetPlayerInfo( enemy_index, &enemy_info ) ) {
			auto local = reinterpret_cast< C_CSPlayer* >( Source::m_pEntList->GetClientEntity( Source::m_pEngine->GetLocalPlayer( ) ) );
			auto entity = reinterpret_cast< C_CSPlayer* >( Source::m_pEntList->GetClientEntity( Source::m_pEngine->GetPlayerForUserID( pEvent->GetInt( XorStr( "userid" ) ) ) ) );

			if ( !entity || !local )
			   return;

			if ( attacker_index != Source::m_pEngine->GetLocalPlayer( ) ) { 
			   if ( enemy_index == local->entindex( ) ) {
				  std::stringstream msg;
				  std::string szHitgroup = HitgroupToString( hitgroup );
				  msg << XorStr( "harmed by " ) << attacker_info.szName << XorStr( " for " ) << dmg_to_health << XorStr( " dmg" ) << XorStr( " in the " ) << szHitgroup ;
				  ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

				  g_Vars.globals.m_bLocalPlayerHarmedThisTick = true;
			   }
			} else {
			   if ( g_Vars.esp.event_dmg ) {
				  std::stringstream msg;
				  std::string szHitgroup = HitgroupToString( hitgroup );
				  msg << XorStr( "hurt " ) << enemy_info.szName << XorStr( " for " ) << dmg_to_health << XorStr( " dmg" ) << XorStr( " in the " ) << szHitgroup;
				  ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
				  auto lag_data = Engine::LagCompensation::Get( )->GetLagData( entity->entindex( ) );
				  CCSGOPlayerAnimState* animState = entity->m_PlayerAnimState( );
				  if ( animState && lag_data.IsValid( ) )
					 IRoundFireBulletsStore::Get( )->EventCallBack( pEvent, 0, new SRoundStats( dmg_to_health, entity->entindex( ), enemy_info.szName, 0, animState->m_flAbsRotation, 2,
																	0, lag_data->m_iResolverType, hitgroup ) );

			   }

			   if ( g_Vars.misc.hitsound ) {
				  if ( g_Vars.misc.hitsound_type == 0 )
					 Source::m_pSurface->PlaySound_( XorStr( "buttons\\arena_switch_press_02.wav" ) );
				  else
					 PlaySoundA( ( LPSTR ) bumble_hitsound, NULL, SND_MEMORY | SND_ASYNC );
			   }
			}
		 #if 0
			if ( g_Vars.esp.event_hitmarker ) {
			   char ch[ 1024 ] = { ( '\0' ) };
			   char healt_buf[ 1024 ] = { ( '\0' ) };
			   int health;

			   player_info_t playerinfo;
			   Source::m_pEngine->GetPlayerInfo( enemy, &playerinfo );

			   health = ( entity->m_iHealth( ) - pEvent->GetInt( XorStr( "dmg_health" ) ) );
			   sprintf( ch, XorStr( "<font size='16' color='#4682B4'>Victim:</font> <font size='16' color='#FF0000'>%s</font>\n<font size='16' color='#4682B4'>Damage:</font> <font size='16' color='#00FF00'>%i</font>" ), playerinfo.szName, pEvent->GetInt( XorStr( "dmg_health" ) ) );

			   auto str = StringtoU16( ch );
			   Source::m_pCenterPrint->ColorPrint( 0.5f, 0.5f, 0.5f, 255, ( char16_t* ) str.data( ) );
			}
		 #endif
		 }
		 break;
	  }
	  case hash_32_fnv1a_const( "item_purchase" ):
	  {
		 if ( !g_Vars.esp.event_buy )
			return;

		 auto userid = pEvent->GetInt( XorStr( "userid" ) );

		 if ( !userid )
			return;

		 int index = Source::m_pEngine->GetPlayerForUserID( userid );

		 player_info_t info;

		 if ( !Source::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		 std::stringstream msg;
		 msg << info.szName << XorStr( " purchased " ) << pEvent->GetString( XorStr( "weapon" ) );

		 ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

		 break;
	  }
	  case hash_32_fnv1a_const( "bomb_begindefuse" ):
	  {
		 if ( !g_Vars.esp.event_bomb )
			return;

		 auto userid = pEvent->GetInt( XorStr( "userid" ) );

		 if ( !userid )
			return;

		 int index = Source::m_pEngine->GetPlayerForUserID( userid );

		 player_info_t info;

		 if ( !Source::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		 bool has_defuse = pEvent->GetBool( XorStr( "haskit" ) );

		 std::stringstream msg;
		 msg << info.szName << XorStr( " has defusing the bomb" );
		 if ( has_defuse )
			msg << XorStr( " with defuse kit" );

		 ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
		 break;
	  }
	  case hash_32_fnv1a_const( "bomb_pickup" ):
	  {
		 if ( !g_Vars.esp.event_bomb )
			return;

		 auto userid = pEvent->GetInt( XorStr( "userid" ) );

		 if ( !userid )
			return;

		 int index = Source::m_pEngine->GetPlayerForUserID( userid );

		 player_info_t info;

		 if ( !Source::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		 std::stringstream msg;
		 msg << info.szName << XorStr( " has picked up the bomb" );

		 ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

		 break;
	  }
	  case hash_32_fnv1a_const( "bomb_beginplant" ):
	  {
		 if ( !g_Vars.esp.event_bomb )
			return;

		 auto userid = pEvent->GetInt( XorStr( "userid" ) );

		 if ( !userid )
			return;

		 int index = Source::m_pEngine->GetPlayerForUserID( userid );

		 player_info_t info;

		 if ( !Source::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		 std::stringstream msg;
		 msg << info.szName << XorStr( " has started planting the bomb " );

		 ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
		 break;
	  }
	  case hash_32_fnv1a_const( "item_pickup" ):
	  {
		 if ( !g_Vars.esp.event_misc )
			return;

		 auto userid = pEvent->GetInt( XorStr( "userid" ) );

		 if ( !userid )
			return;

		 int index = Source::m_pEngine->GetPlayerForUserID( userid );

		 player_info_t info;

		 if ( !Source::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		 std::stringstream msg;
		 msg << info.szName << XorStr( " has picked up weapon " ) << pEvent->GetString( XorStr( "item" ) );

		 ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
		 break;
	  }
	  case hash_32_fnv1a_const( "round_mvp" ):
	  {
		 if ( !g_Vars.esp.event_misc )
			return;

		 auto userid = pEvent->GetInt( XorStr( "userid" ) );

		 if ( !userid )
			return;

		 int index = Source::m_pEngine->GetPlayerForUserID( userid );

		 player_info_t info;

		 if ( !Source::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		 std::stringstream msg;
		 msg << info.szName << XorStr( " has getting mvp on this round " );

		 ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
		 break;
	  }
	  case hash_32_fnv1a_const( "grenade_thrown" ):
	  {
		 if ( !g_Vars.esp.event_misc )
			return;

		 auto userid = pEvent->GetInt( XorStr( "userid" ) );

		 if ( !userid )
			return;

		 int index = Source::m_pEngine->GetPlayerForUserID( userid );

		 player_info_t info;

		 if ( !Source::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		 if ( index == LocalPlayer->m_entIndex )
			g_Vars.globals.ResetWeapon = true;

		 std::stringstream msg;
		 msg << info.szName << XorStr( "  has thrown a  " ) << pEvent->GetString( XorStr( "weapon" ) );

		 ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
		 break;
	  }
	  case hash_32_fnv1a_const( ( "buytime_ended" ) ):
	  {
		 if ( !g_Vars.esp.event_misc )
			return;

		 std::stringstream msg;
		 msg << XorStr( "buy time has ended " );

		 ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
		 break;
	  }
	  case hash_32_fnv1a_const( "round_start" ):
	  {
		 IRoundFireBulletsStore::Get( )->EventCallBack( pEvent, 1, nullptr );

		 /* death notices */
		 for ( int i = 0; i < Source::g_pDeathNotices->m_vecDeathNotices.Count( ); i++ ) {
			 auto cur = &Source::g_pDeathNotices->m_vecDeathNotices[ i ];
			 if ( !cur ) {
				 continue;
			 }

			 cur->m_flStartTime = 0.f;
		 }

		 IAutoBuy::Get( )->Main( );
		 if ( !g_Vars.esp.event_misc )
			return;

		 std::stringstream msg;
		 msg << XorStr( "new round ! " );

		 ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

		 for ( size_t i = 1; i <= 64; i++ ) {
			IEsp::Get( )->SetAlpha( i );
		 }

		 break;
	  }
	  case hash_32_fnv1a_const( "player_death" ):
	  {
		 auto enemy = pEvent->GetInt( XorStr( "userid" ) );
		 auto enemy_index = Source::m_pEngine->GetPlayerForUserID( enemy );

		 IEsp::Get( )->SetAlpha( enemy_index );
		 if ( g_Vars.esp.kill_effect ) {
			auto attacker = pEvent->GetInt( XorStr( "attacker" ) );
			auto attacker_index = Source::m_pEngine->GetPlayerForUserID( attacker );
			auto pEnemy = C_CSPlayer::GetPlayerByIndex( enemy_index );
			auto pAttacker = C_CSPlayer::GetPlayerByIndex( attacker_index );

			if ( !pEnemy || !pAttacker )
			   return;

			if ( attacker_index == Source::m_pEngine->GetLocalPlayer( ) && enemy_index != Source::m_pEngine->GetLocalPlayer( ) ) {
			   LocalPlayer->m_flHealthShotBoostExpirationTime( ) = Source::m_pGlobalVars->curtime + g_Vars.esp.kill_effect_time;
			   Source::LegitBot::Get( )->GameEventCallBack( );
			}
		 }
		 break;
	  }
   }
}

int C_GameEvent::GetEventDebugID( void ) {
   return 42;
}
