#include "BulletBeamTracer.hpp"
#include "source.hpp"
#include "CVariables.hpp"
#include "player.hpp"

class CBulletBeamTracer : public IBulletBeamTracer {
public:
  void Main( ) override;
  void PushBeamInfo( BulletImpactInfo beam_info ) override;
  const char * GetBeamName( );
private:
  void DrawBeam( );
  std::vector<BulletImpactInfo> bulletImpactInfo;
};

Encrypted_t<IBulletBeamTracer> IBulletBeamTracer::Get( ) {
  static CBulletBeamTracer instance;
  return &instance;
}

void CBulletBeamTracer::Main( ) {
  DrawBeam( );
}

void CBulletBeamTracer::PushBeamInfo( BulletImpactInfo beam_info ) {
  bulletImpactInfo.emplace_back( beam_info );
}

const char* CBulletBeamTracer::GetBeamName( ) {
  switch( g_Vars.esp.beam_type ) {
  case 1:
	 return XorStr( "sprites/blueglow1.vmt" );
	 break;
  case 2:
	 return XorStr( "sprites/bubble.vmt" );
	 break;
  case 3:
	 return XorStr( "sprites/glow01.vmt" );
	 break;
  case 4:
	 return XorStr( "sprites/purpleglow1.vmt" );
	 break;
  case 5:
	 return XorStr( "sprites/purplelaser1.vmt" );
	 break;
  case 6:
	 return XorStr( "sprites/radio.vmt" );
	 break;
  case 7:
	 return XorStr( "sprites/white.vmt" );
	 break;
  case 0:
  default:
	 return XorStr( "sprites/physbeam.vmt" );
	 break;
  }
}
void CBulletBeamTracer::DrawBeam( ) {

  C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

  if( !LocalPlayer || LocalPlayer->IsDead( ) )
	 return;

  std::vector<BulletImpactInfo> &impacts = bulletImpactInfo;

  if( impacts.empty( ) )
	 return;

  for( size_t i = 0; i < impacts.size( ); i++ ) {

	 auto current_impact = impacts.at( i );

	 BeamInfo_t beamInfo{ };
	 beamInfo.m_nType = TE_BEAMPOINTS;
	 beamInfo.m_pszModelName = GetBeamName( );
	 beamInfo.m_nModelIndex = -1;
	 beamInfo.m_flHaloScale = 0.0f;
	 beamInfo.m_flLife = g_Vars.esp.beam_life_time;
	 beamInfo.m_flWidth = g_Vars.esp.beam_width;
	 beamInfo.m_flEndWidth = 1.5f;
	 beamInfo.m_flFadeLength = 1.0f;
	 beamInfo.m_flAmplitude = 2.0f;
	 beamInfo.m_flBrightness = 255.f;
	 beamInfo.m_flSpeed = g_Vars.esp.beam_speed;
	 beamInfo.m_nStartFrame = 0;
	 beamInfo.m_flFrameRate = 0.f;
	 beamInfo.m_flRed = g_Vars.esp.beam_color.r * 255;
	 beamInfo.m_flGreen = g_Vars.esp.beam_color.g * 255;
	 beamInfo.m_flBlue = g_Vars.esp.beam_color.b * 255;
	 beamInfo.m_nSegments = 2;
	 beamInfo.m_bRenderable = true;
	 beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

	 beamInfo.m_vecStart = LocalPlayer->GetEyePosition( );
	 beamInfo.m_vecEnd = current_impact.m_vecHitPos;

	 auto beam = Source::m_pRenderBeams->CreateBeamPoints( beamInfo );

	 if( beam ) {
		Source::m_pRenderBeams->DrawBeam( beam );
	 }

	 impacts.erase( impacts.begin( ) + i );
  }
}
