#pragma once
#include "sdk.hpp"

namespace Source
{
   class __declspec( novtable ) AntiAimbot : public NonCopyable {
   public:
	  static Encrypted_t<AntiAimbot> Get( );
	  virtual void Main( bool* bSendPacket, Encrypted_t<CUserCmd> cmd, bool ragebot ) = 0;
	  virtual void PrePrediction( bool* bSendPacket, Encrypted_t<CUserCmd> cmd ) = 0;
   protected:
	  AntiAimbot( ) { };
	  virtual ~AntiAimbot( ) { };
   };
}
