#pragma once
#include "sdk.hpp"

class __declspec( novtable ) GameEvent : public IGameEventListener, public NonCopyable
{
public:
  static Encrypted_t<GameEvent>  Get( );

  virtual void Register( ) = 0;
  virtual void Shutdown( ) = 0;

protected:
  GameEvent( ) { };
  virtual ~GameEvent( ) { };
};
