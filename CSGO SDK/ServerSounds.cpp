#include "ServerSounds.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "source.hpp"

C_ServerSounds g_ServerSounds;


void C_ServerSounds::ProcessSound( int guid, StartSoundParams_t& params, char const* soundname ) {
   auto entity = ( C_BaseEntity* ) Source::m_pEntList->GetClientEntity( params.soundsource );
   if ( !entity )
      return;

   auto local = C_CSPlayer::GetLocalPlayer( );

   if ( !local )
      return;

   if ( !entity->IsPlayer( ) ) {
      auto owner = ( C_CSPlayer* ) entity->m_hOwnerEntity( ).Get( );
      if ( !owner || !owner->IsPlayer( ) )
         return;
      entity = owner;
   }

   if ( entity == local || entity->m_iTeamNum( ) == local->m_iTeamNum( ) )
      return;

   auto& infos = this->m_Players[ params.soundsource ];
   std::string name = soundname;
   if ( name.find( "footsteps" ) != std::string::npos ) {
      auto& new_sound = infos.m_Sounds.emplace_front( );

      new_sound.origin = params.origin;
      new_sound.direction = params.direction;
      new_sound.tickcount = Source::m_pGlobalVars->tickcount;
      new_sound.guid = guid;
      new_sound.name = soundname;
      new_sound.params = params;

   }
   //if ( name.find( "footsteps" ) != std::string::npos ) {
   //   Source::m_pDebugOverlay->AddBoxOverlay( params.origin, Vector( -1.f, -1.f, -1.f ), Vector( 1.f, 1.f, 1.f ),
   //                                           QAngle( ), 255, 0, 0, 150, 10.0f );
   //   printf( "name: %s, direction x:%f | y:%f | z:%f, origin x:%f | y:%f | z:%f\n", soundname, params.direction.x, params.direction.y, params.direction.z, params.origin.x, params.origin.y, params.origin.z );
   //
   //}
   //Start( );

   while ( infos.m_Sounds.size( ) > 128 )
	  infos.m_Sounds.pop_back( );
}

void C_ServerSounds::Start( ) {
   for ( int i = 1; i <= Source::m_pGlobalVars->maxClients; ++i ) {
      C_CSPlayer* player = C_CSPlayer::GetPlayerByIndex( i );
      
      if ( !player || player->IsDead( ) || !player->IsDormant( ) )
         continue;

      RestoreSounds restore_sound;
      restore_sound.m_index = i;
      restore_sound.m_vecOrigin = player->m_vecOrigin( );
      restore_sound.m_vecAbsOrigin = player->GetAbsOrigin( );
      restore_sound.m_bDormant = player->IsDormant( );

      m_RestorePlayers.push_back( restore_sound );

      static C_Sound pervious;
      auto sound = this->m_Players[ i ];
      if ( !sound.m_Sounds.empty( ) ) {
         for ( auto& data : sound.m_Sounds ) {
            std::string str = data.name;
            std::string str2( XorStr( "footsteps" ) );

            if ( data.origin.IsZero( ) )
               continue;

            if ( str.find( str2 ) != std::string::npos ) {
               Vector origin = data.origin;
               *( bool* ) ( ( uintptr_t ) player + 0xED ) = true; // dormant check
               player->SetAbsOrigin( origin );
            }
         }
      }
   }
}

void C_ServerSounds::Finish( ) {
   for ( const auto& data : m_RestorePlayers ) {
      C_CSPlayer* player = C_CSPlayer::GetPlayerByIndex( data.m_index );

      if ( !player || player->IsDead( ) )
         continue;

      *( bool* ) ( ( uintptr_t ) player + 0xED ) = data.m_bDormant; // dormant check
      player->SetAbsOrigin( data.m_vecAbsOrigin );
   }
}
