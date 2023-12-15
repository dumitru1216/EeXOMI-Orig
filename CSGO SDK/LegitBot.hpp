#pragma once
#include "sdk.hpp"

namespace Source
{
   class __declspec( novtable ) LegitBot : public NonCopyable {
   public:
	  static LegitBot* Get( );
	  virtual void Main( CUserCmd* pCmd ) = NULL;
	  virtual void GameEventCallBack( ) = NULL;
   protected:
	  LegitBot( ) {
		 
	  }
	  virtual ~LegitBot( ) {

	  }
   };
}