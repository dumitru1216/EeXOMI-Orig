#include "RoundFireBulletsStore.hpp"
#include <deque>
#include "source.hpp"
#include "player.hpp"

class CRoundFireBulletsStore : public IRoundFireBulletsStore {
private:
   std::deque<PSRoundStats> m_pvPlayersStat;

public:
   void EventCallBack( IGameEvent* pEvent, int index, PSRoundStats round_stat ) override;
   void AimCallback( int iLagCompTicks, float flHitchance, int iHitbox, int iDamage ) override;
   PSRoundStats GetStats( int element_index ) override;
   int GetVecSize( ) override;

   CRoundFireBulletsStore( ) {

   }

   ~CRoundFireBulletsStore( ) {

   }

private:
   void EventHandler( PSRoundStats round_stat );
   void OnRoundEnd( );
   void Think( );

   int m_iHitboxIndex;
   int m_flHitchance;
   int m_iLagCompTicks;
   int m_iDamage;

   bool m_bAimGiveData;
};

IRoundFireBulletsStore* IRoundFireBulletsStore::Get( ) {
   static CRoundFireBulletsStore instance;
   return &instance;
}

void CRoundFireBulletsStore::EventCallBack( IGameEvent* pEvent, int index, PSRoundStats round_stat ) {
   switch ( index ) {
	  case 0: // hit/miss
	  round_stat->m_iLagCompTicks = m_iLagCompTicks;
	  round_stat->m_flHitchance = m_flHitchance;

	  if ( m_bAimGiveData ) {
		 round_stat->m_iDamage = m_iDamage;
		 round_stat->m_iHitbox = m_iHitboxIndex;
		 round_stat->m_bAimGiveData = true;
	  } else
		 round_stat->m_iDamage = 0;

	  EventHandler( round_stat ); 
	  m_bAimGiveData = false; break;
	  case 1: // round end
	  OnRoundEnd( ); break;
   }
}

PSRoundStats CRoundFireBulletsStore::GetStats( int element_index ) {
   return m_pvPlayersStat.at( element_index );
}

void CRoundFireBulletsStore::AimCallback( int iLagCompTicks, float flHitchance, int iHitbox, int iDamage ) {
   m_iLagCompTicks = iLagCompTicks;
   m_flHitchance = flHitchance;
   m_iHitboxIndex = iHitbox;
   m_iDamage = iDamage;
   m_bAimGiveData = true;
}

void CRoundFireBulletsStore::EventHandler( PSRoundStats round_stat ) {
   m_pvPlayersStat.push_front( round_stat );
   Think( );
}

void CRoundFireBulletsStore::OnRoundEnd( ) {
   for ( int i = 0; i < m_pvPlayersStat.size( ); i++ ) {
	  delete m_pvPlayersStat.at( i );
	  m_pvPlayersStat.at( i ) = nullptr;
   }

   m_pvPlayersStat.clear( );
}

void CRoundFireBulletsStore::Think( ) {
   if ( m_pvPlayersStat.size( ) > 6 )
	  m_pvPlayersStat.pop_back( );

}

int CRoundFireBulletsStore::GetVecSize( ) {
   return m_pvPlayersStat.size( );
}
