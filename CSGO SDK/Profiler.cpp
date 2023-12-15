#include "Profiler.hpp"

class Profiler : public IProfiler {
public:
   void Think( ) override;
   ProfileData_t GetData( int data_num ) override {
	  return data_num == 1 ? m_Data : m_UsedData;
   }
   void SetData( ProfileData_t data ) override {
	  m_UsedData = data;
   }
private:
   ProfileData_t m_Data;
   ProfileData_t m_UsedData;
};

IProfiler* IProfiler::Get( ) {
   static Profiler instance;
   return &instance;
}

void Profiler::Think( ) {
   if ( m_Data.Zero( ) ) {
	  m_Data.m_iAutowallCallsPerTick = m_UsedData.m_iAutowallCallsPerTick;
	  m_Data.m_iAutowallCallsPerSecond += m_UsedData.m_iAutowallCallsPerTick;
	  m_Data.m_iLagCompensationTicksScaned = m_UsedData.m_iLagCompensationTicksScaned;
	  m_Data.m_iPlayersCount = m_UsedData.m_iPlayersCount;
	  m_Data.m_flCurtime = m_UsedData.m_flCurtime;
	  m_Data.m_TickCount = m_UsedData.m_TickCount;
   } else {
	  if ( ( Source::m_pGlobalVars->realtime - m_Data.m_flCurtime ) > 1.0f ) {
		 m_Data.m_flCurtime = Source::m_pGlobalVars->realtime;
		// m_Data.Print( );
		 m_Data.Clear( );
	  } else if ( m_Data.m_TickCount != Source::m_pGlobalVars->tickcount ) {
		// m_Data.Print( );
		 m_Data.m_iAutowallCallsPerTick = 0;
		 m_Data.m_iLagCompensationTicksScaned = 0;
		 m_Data.m_iPlayersCount = 0;
		 m_Data.m_TickCount = Source::m_pGlobalVars->tickcount;
	  }
	  m_Data.m_iAutowallCallsPerTick = m_UsedData.m_iAutowallCallsPerTick;
	  m_Data.m_iAutowallCallsPerSecond += m_UsedData.m_iAutowallCallsPerTick;
	  m_Data.m_iLagCompensationTicksScaned = m_UsedData.m_iLagCompensationTicksScaned;
	  m_Data.m_iPlayersCount = m_UsedData.m_iPlayersCount;
	  m_Data.m_TickCount = m_UsedData.m_TickCount;
   }
   m_UsedData.Clear( );
}
