#pragma once
#include "sdk.hpp"

class __declspec( novtable ) ILoggerEvent : NonCopyable {
public:
  static Encrypted_t<ILoggerEvent> Get( );
  virtual void Main( ) = NULL;
  virtual void PushEvent( std::string msg, FloatColor clr ) = NULL;
protected:
  ILoggerEvent( ) { };
  virtual ~ILoggerEvent( ) { };
};
