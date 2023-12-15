#include "Hitmarker.hpp"
#include "player.hpp"
#include <chrono>
#include "CVariables.hpp"
#include "Render.hpp"

class CHitmarker : public IHitmarker {
public:
   void GameEventCallback( IGameEvent* pEvent ) override;
   void Paint( ) override;
private:
   void PlayerHurt( IGameEvent* pEvent );
   void BulletImpact( IGameEvent* pEvent );

   std::vector<ImpactInfo> pImpacts;
   std::vector<HitmarkerInfo> pHitmarkers;
};

IHitmarker* IHitmarker::Get( ) {
   static CHitmarker instance;
   return &instance;
}

void CHitmarker::GameEventCallback( IGameEvent* pEvent ) {
   if ( !g_Vars.esp.vizualize_hitmarker && !g_Vars.esp.vizualize_hitmarker_damage )
	  return;

   C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

   if ( !LocalPlayer )
	  return;

   if ( !Source::m_pEngine->IsInGame( ) )
	  return;

   if ( !pEvent )
	  return;

   auto event_hash = hash_32_fnv1a_const( pEvent->GetName( ) );

   if ( event_hash == hash_32_fnv1a_const( "player_hurt" ) )
	  PlayerHurt( pEvent );
   else if ( event_hash == hash_32_fnv1a_const( "bullet_impact" ) )
	  BulletImpact( pEvent );
}

void CHitmarker::Paint( ) {
   if ( !g_Vars.esp.vizualize_hitmarker && !g_Vars.esp.vizualize_hitmarker_damage )
	  return;

   C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );
   if ( !Source::m_pEngine->IsInGame( ) || !LocalPlayer || LocalPlayer->IsDead( ) ) {
	  if ( !pImpacts.empty( ) ) {
		 pImpacts.clear( );
	  }

	  if ( !pHitmarkers.empty( ) ) {
		 pHitmarkers.clear( );
	  }

	  return;
   }

   std::vector<HitmarkerInfo>::iterator pIter;

   for ( pIter = pHitmarkers.begin( ); pIter != pHitmarkers.end( ); ) {
	  static auto line_size = 6;

	  const auto step = 255.f / 1.f * Source::m_pGlobalVars->frametime;
	  const auto step_move = 30.f / 1.5f * Source::m_pGlobalVars->frametime;
	  const auto multiplicator = 0.3f;

	  if ( pIter->pImpact.time + 0.5f <= Source::m_pGlobalVars->curtime && pIter->alpha > 0.0f )
		 pIter->alpha -= step;

	  pIter->moved -= step_move;

	  if ( pIter->pImpact.time + 0.5f <= Source::m_pGlobalVars->curtime && pIter->alpha2 > 0 )
		 pIter->alpha2 -= 5;

	  if ( pIter->alpha <= 0 && pIter->alpha2 <= 0 ) {
		 pIter= pHitmarkers.erase( pIter );
		 continue;
	  }

	  FloatColor pWhite2 = FloatColor( 255, 255, 255, pIter->alpha2 );

	  Vector pos3D = Vector( pIter->pImpact.x, pIter->pImpact.y, pIter->pImpact.z );

	  Vector2D center = Render::Get( )->GetScreenSize( ) * 0.5f;
	  if ( g_Vars.esp.vizualize_hitmarker ) {
		 Render::Get( )->AddLine( Vector2D( center.x - 8, center.y - 8 ), Vector2D( center.x - 4, center.y - 4 ), pWhite2 );
		 Render::Get( )->AddLine( Vector2D( center.x - 8, center.y + 8 ), Vector2D( center.x - 4, center.y + 4 ), pWhite2 );
		 Render::Get( )->AddLine( Vector2D( center.x + 8, center.y - 8 ), Vector2D( center.x + 4, center.y - 4 ), pWhite2 );
		 Render::Get( )->AddLine( Vector2D( center.x + 8, center.y + 8 ), Vector2D( center.x + 4, center.y + 4 ), pWhite2 );
	  }

	  const auto int_alpha = static_cast< int >( pIter->alpha );

	  Vector2D pos2D;
	  if ( WorldToScreen( pos3D, pos2D ) && g_Vars.esp.vizualize_hitmarker_damage ) {
		 FloatColor col = FloatColor( 255, 255, 255, int_alpha );
		 Render::Get( )->AddLine( Vector2D( pos2D.x - line_size * multiplicator, pos2D.y - line_size * multiplicator ), Vector2D( pos2D.x - line_size, pos2D.y - line_size ), col );
		 Render::Get( )->AddLine( Vector2D( pos2D.x - line_size * multiplicator, pos2D.y + line_size * multiplicator ), Vector2D( pos2D.x - line_size, pos2D.y + line_size ), col );
		 Render::Get( )->AddLine( Vector2D( pos2D.x + line_size * multiplicator, pos2D.y + line_size * multiplicator ), Vector2D( pos2D.x + line_size, pos2D.y + line_size ), col );
		 Render::Get( )->AddLine( Vector2D( pos2D.x + line_size * multiplicator, pos2D.y - line_size * multiplicator ), Vector2D( pos2D.x + line_size, pos2D.y - line_size ), col );

		 Render::Get( )->SetTextFont( FONT_VERDANA );
		 col = FloatColor( 255, 0, 0, int_alpha );
		 Render::Get( )->AddText( Vector2D( pos2D.x + 8, pos2D.y - 12 + pIter->moved ), col, DROP_SHADOW, XorStr( "- %i" ), pIter->damage );
	  }

	  ++pIter;
   }
}

void CHitmarker::PlayerHurt( IGameEvent* pEvent ) {

   C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

   if ( !LocalPlayer )
	  return;

   int attacker_index = Source::m_pEngine->GetPlayerForUserID( pEvent->GetInt( XorStr( "attacker" ) ) );

   if ( !attacker_index )
	  return;

   int victim_index = Source::m_pEngine->GetPlayerForUserID( pEvent->GetInt( XorStr( "userid" ) ) );

   if ( !victim_index )
	  return;

   C_CSPlayer* pAttacker = C_CSPlayer::GetPlayerByIndex( attacker_index );
   C_CSPlayer* pVictim = C_CSPlayer::GetPlayerByIndex( victim_index );

   if ( !pAttacker || !pVictim || pAttacker != LocalPlayer )
	  return;

   Vector pEnemyPos = pVictim->GetAbsOrigin( );
   ImpactInfo BestImpact;

   float pBestImpactDistance = -1;

   std::vector<ImpactInfo>::iterator pIter;

   for ( pIter = pImpacts.begin( ); pIter != pImpacts.end( ); ) {
	  if ( Source::m_pGlobalVars->curtime > pIter->time + 1.f ) {
		 pIter = pImpacts.erase( pIter );
		 continue;
	  }

	  Vector Position = Vector( pIter->x, pIter->y, pIter->z );

	  float pDistance = Position.Distance( Vector( pEnemyPos.x, pEnemyPos.y, pEnemyPos.z ) );

	  if ( pDistance < pBestImpactDistance || pBestImpactDistance == -1 ) {

		 pBestImpactDistance = pDistance;
		 BestImpact = *pIter;
	  }
	  pIter++;
   }

   if ( pBestImpactDistance == -1 )
	  return;

   HitmarkerInfo pInfo;
   pInfo.pImpact = BestImpact;
   pInfo.alpha = pInfo.alpha2 = 255;
   pInfo.damage = pEvent->GetInt( XorStr( "dmg_health" ) );
   pHitmarkers.emplace_back( pInfo );
}

void CHitmarker::BulletImpact( IGameEvent* pEvent ) {

   C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

   if ( !LocalPlayer )
	  return;

   int index = Source::m_pEngine->GetPlayerForUserID( pEvent->GetInt( XorStr( "userid" ) ) );

   if ( !index )
	  return;

   C_CSPlayer* pShooter = C_CSPlayer::GetPlayerByIndex( index );

   if ( !pShooter || pShooter != LocalPlayer )
	  return;

   ImpactInfo Info;
   Info.x = pEvent->GetFloat( XorStr( "x" ) );
   Info.y = pEvent->GetFloat( XorStr( "y" ) );
   Info.z = pEvent->GetFloat( XorStr( "z" ) );
   Info.time = Source::m_pGlobalVars->curtime;
   pImpacts.emplace_back( Info );
}
