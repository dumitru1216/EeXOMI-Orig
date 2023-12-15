#pragma once
#include "sdk.hpp"
#include "source.hpp"

class __declspec( novtable ) IProfiler : public NonCopyable {
public:
   struct ProfileData_t {
	  ProfileData_t( ) {
		 m_flCurtime = Source::m_pGlobalVars->curtime;
		 m_TickCount = Source::m_pGlobalVars->tickcount;
	  }

	  ProfileData_t( int AutowallCallsPerTick, int AutowallCallsPerSecond, int LagCompensationTicksScaned, int PlayersCount ) {
		 m_iAutowallCallsPerTick = AutowallCallsPerTick;
		 m_iAutowallCallsPerSecond = AutowallCallsPerSecond;
		 m_iLagCompensationTicksScaned = LagCompensationTicksScaned;
		 m_iPlayersCount = PlayersCount;
		 m_flCurtime = Source::m_pGlobalVars->curtime;
		 m_TickCount = Source::m_pGlobalVars->tickcount;
	  }

	  int m_iAutowallCallsPerTick = 0;
	  int m_iAutowallCallsPerSecond = 0;
	  int m_iLagCompensationTicksScaned = 0;
	  int m_iPlayersCount = 0;
	  int m_TickCount = 0;

	  float m_flCurtime = 0;

	  __forceinline void Clear( ) {
		 m_iAutowallCallsPerTick = 0;
		 m_iAutowallCallsPerSecond = 0;
		 m_iLagCompensationTicksScaned = 0;
		 m_iPlayersCount = 0;
		 m_TickCount = 0;
	  }

	  __forceinline void Print( ) {
		 if ( m_iAutowallCallsPerTick != 0 )
			printf( XorStr( "m_iAutowallCallsPerTick %i\n" ), m_iAutowallCallsPerTick );
		 if ( m_iAutowallCallsPerSecond != 0 )
			printf( XorStr( "m_iAutowallCallsPerSecond %i\n" ), m_iAutowallCallsPerSecond );
		 if ( m_iLagCompensationTicksScaned != 0 )
			printf( XorStr( "m_iLagCompensationTicksScaned %i\n" ), m_iLagCompensationTicksScaned );
		 if ( m_iPlayersCount != 0 )
			printf( XorStr( "m_iPlayersCount %i\n" ), m_iPlayersCount );
	  }

	  __forceinline bool Zero( ) {
		 return m_flCurtime == 0;
	  }
   };

   static IProfiler* Get( );
   virtual void Think( ) = NULL;
   virtual ProfileData_t GetData( int data_num ) = NULL;
   virtual void SetData( ProfileData_t data ) = NULL;
protected:
   IProfiler( ) {

   }
   virtual ~IProfiler( ) {

   }
};
