#pragma once
#include "sdk.hpp"

namespace Source
{
  class __declspec( novtable ) KnifeBot : public NonCopyable {
  public:
	 static KnifeBot* Get( );
	 virtual void Main(  Encrypted_t<CUserCmd> pCmd, bool* sendPacket) = NULL;
  protected:
	 KnifeBot( ) {

	 }
	 virtual ~KnifeBot( ) {

	 }
  };
}
