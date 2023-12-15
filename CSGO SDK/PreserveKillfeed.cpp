#include "PreserveKillfeed.hpp"
#include "player.hpp"
#include "displacement.hpp"

class C_PreserveKillfeed : public IPreserveKillfeed {
public:
   void OnFrameStart( ) override;
   void OnPaintTraverse( ) override;

public:
   uintptr_t m_uHudDeathNotice = 0u;
   C_CSPlayer* m_pLocal = nullptr;

public:
   C_PreserveKillfeed( ) { };
   virtual ~C_PreserveKillfeed( ) { };
};

IPreserveKillfeed* IPreserveKillfeed::Get( ) {
   static C_PreserveKillfeed instance;
   return &instance;
}

void C_PreserveKillfeed::OnFrameStart( ) {
   m_pLocal = C_CSPlayer::GetLocalPlayer( );

   if ( Source::m_pEngine->IsInGame( ) && m_pLocal ) {
	  if ( m_uHudDeathNotice == 0u )
		 m_uHudDeathNotice = FindHudElement<uintptr_t>( XorStr( "CCSGO_HudDeathNotice" ) );
   }
   else {
	  m_uHudDeathNotice = 0u;
   }
}

void C_PreserveKillfeed::OnPaintTraverse( ) {
   if ( !m_pLocal )
	  return;

   static float LastSpawnTime = 0.0f;

   if ( Source::m_pEngine->IsInGame( ) && m_pLocal && !m_pLocal->IsDead( ) && m_uHudDeathNotice != 0u ) {
	  static auto had_notices = false;
	  static auto ClearNoticies = ( void( __thiscall* )( uintptr_t ) ) Engine::Displacement.Function.m_uClearDeathNotices;

	  if ( g_Vars.esp.preserve_killfeed ) {
		 *( float* ) ( m_uHudDeathNotice + 80 ) = g_Vars.esp.preserve_killfeed_time;
		 had_notices = true;

		 if ( LastSpawnTime != m_pLocal->m_flSpawnTime( ) ) {
			auto death_notices = ( ( int* ) ( ( uintptr_t ) m_uHudDeathNotice - 20 ) );

			if ( death_notices ) {
			   if ( ClearNoticies )
				  ClearNoticies( m_uHudDeathNotice - 20 );
			}

			LastSpawnTime = m_pLocal->m_flSpawnTime( );
		 }
	  }
	  else if ( had_notices ) {
		 auto death_notices = ( ( int* ) ( ( uintptr_t ) m_uHudDeathNotice - 20 ) );

		 if ( death_notices ) {
			if ( ClearNoticies )
			   ClearNoticies( m_uHudDeathNotice - 20 );

			had_notices = false;
		 }
	  }
   }
}
