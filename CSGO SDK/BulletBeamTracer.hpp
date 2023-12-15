#pragma once
#include "sdk.hpp"

class __declspec( novtable ) IBulletBeamTracer : public NonCopyable {
public:
  struct BulletImpactInfo
  {
	 float m_flExpTime;
	 Vector m_vecHitPos;
  };

  static Encrypted_t<IBulletBeamTracer> Get( );
  virtual void Main( ) = NULL;
  virtual void PushBeamInfo( BulletImpactInfo beam_info ) = NULL;
protected:
  IBulletBeamTracer( ) { };
  virtual ~IBulletBeamTracer( ) { };
};
