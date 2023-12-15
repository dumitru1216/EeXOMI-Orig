#pragma once
#include "sdk.hpp"
#include "player.hpp"

class __declspec( novtable ) IHitDamageVisualize : public NonCopyable {
public:
  static IHitDamageVisualize* Get( );
  virtual void Main( ) = NULL;
  virtual void AddHitmarker(int damage, C_CSPlayer* m_player) = NULL;
  virtual void AddHitmarker(bool headshot, C_CSPlayer* m_player) = NULL;
protected:
  IHitDamageVisualize( ) {
		
  }
  virtual ~IHitDamageVisualize( ) {

  }
};