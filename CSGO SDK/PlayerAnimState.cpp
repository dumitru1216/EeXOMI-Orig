#include "PlayerAnimState.hpp"
#include "Math.h"
#include "Displacement.hpp"
#include "player.hpp"

void CCSGOPlayerAnimState::Reset( ) {
   using ResetAnimState_t = void( __thiscall* )( CCSGOPlayerAnimState* );
   ( ( ResetAnimState_t ) Engine::Displacement.Function.m_uResetAnimState )( this );
}

void CCSGOPlayerAnimState::Update( QAngle angles ) {
   __asm
   {
	  push 0
	  mov ecx, this

	  movss xmm1, dword ptr[ angles + 4 ]
	  movss xmm2, dword ptr[ angles ]

	  call Engine::Displacement.Function.m_uUpdateAnimState
   }
}

float CCSGOPlayerAnimState::GetMaxFraction( ) {
   float speedFactor = Math::Clamp( m_flDuckingSpeed, 0.0f, 1.0f );
   float groundFraction = ( ( m_flGroundFraction * -0.3f ) - 0.2f ) * speedFactor;
   float maxFraction = groundFraction + 1.0f;

   if ( m_fDuckAmount > 0.0f ) {
	  float maxVelocity = Math::Clamp( m_flRunningSpeed, 0.0f, 1.0f );
	  float duckSpeed = m_fDuckAmount * maxVelocity;
	  maxFraction += ( duckSpeed * ( 0.5f - maxFraction ) );
   }
   return maxFraction;
}

float CCSGOPlayerAnimState::GetDesyncDelta( bool useMinYaw ) {
   float frac = GetMaxFraction( );

   return useMinYaw ? m_flMinBodyYaw * frac : m_flMaxBodyYaw * frac;
}

struct mstudioposeparamdesc_t1 {
   int sznameindex;
   inline char* const pszName( void ) const { return ( ( char* ) this ) + sznameindex; }
   int flags;   // ???? ( volvo, really? )
   float start; // starting value
   float end;   // ending value
   float loop;  // looping range, 0 for no looping, 360 for rotations, etc.
};
mstudioposeparamdesc_t1* pPoseParameter( CStudioHdr* hdr, int index ) {
   using poseParametorFN = mstudioposeparamdesc_t1 * ( __thiscall* )( CStudioHdr*, int );
   poseParametorFN p_pose_parameter = ( poseParametorFN ) Engine::Displacement.Function.m_pPoseParameter;
   return p_pose_parameter( hdr, index );
}

void animstate_pose_param_cache_t::SetValue( C_CSPlayer* player, float flValue ) {
   auto hdr = player->m_pStudioHdr( );
   if ( hdr ) {
      auto pose_param = pPoseParameter( hdr, index );
      if ( !pose_param )
         return;

      auto PoseParam = *pose_param;

      if ( PoseParam.loop ) {
         float wrap = ( PoseParam.start + PoseParam.end ) / 2.0f + PoseParam.loop / 2.0f;
         float shift = PoseParam.loop - wrap;

         flValue = flValue - PoseParam.loop * std::floorf( ( flValue + shift ) / PoseParam.loop );
      }

      auto ctlValue = ( flValue - PoseParam.start ) / ( PoseParam.end - PoseParam.start );
      player->m_flPoseParameter( )[ index ] = ctlValue;
   }
}
