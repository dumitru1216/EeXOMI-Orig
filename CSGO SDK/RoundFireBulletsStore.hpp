#pragma once
#include "sdk.hpp"

typedef struct _SRoundStats
{
  _SRoundStats( int damage, int index, std::string name, int ticks, float AbsRotation, int Reason, int Side, int Type, int Hitbox ) : m_iEventDamage( damage ), m_iClientIndex( index ), m_name( name ), m_iLagCompTicks( ticks ), m_flAbsRotation( AbsRotation ), m_iReason( Reason ), m_iResolverSide( Side ), m_iResolverType( Type ), m_iHitbox( Hitbox ) {
	 m_bAimGiveData = false;
  }
  int m_iDamage;
  int m_iEventDamage;
  int m_iClientIndex;
  int m_iLagCompTicks;
  int m_iReason;
  int m_iResolverSide;
  int m_iResolverType;
  int m_iHitbox;

  bool m_bAimGiveData;

  float m_flHitchance;
  float m_flAbsRotation;

  std::string m_name;
}SRoundStats, *PSRoundStats;

class __declspec( novtable ) IRoundFireBulletsStore : public NonCopyable {
public:
  static IRoundFireBulletsStore* Get( );
  virtual void EventCallBack( IGameEvent* pEvent, int index, PSRoundStats round_stat ) = NULL;
  virtual void AimCallback( int iLagCompTicks, float flHitchance, int iHitbox, int iDamage ) = NULL;
  virtual PSRoundStats GetStats( int element_index ) = NULL;
  virtual int GetVecSize( ) = NULL;
protected:
  IRoundFireBulletsStore( ) {

  }
  virtual ~IRoundFireBulletsStore( ) {

  }
};