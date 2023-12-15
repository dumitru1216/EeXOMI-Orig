#include "Hooked.hpp"
#include "displacement.hpp"
#include "player.hpp"
#include "SetupBones.hpp"
#include "LagCompensation.hpp"

namespace Hooked
{
   void __cdecl InterpolateServerEntities( ) {
	  g_Vars.globals.szLastHookCalled = XorStr( "11" );
	  if ( !g_Vars.globals.RenderIsReady )
		 return oInterpolateServerEntities( );

	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || !Source::m_pEngine->IsInGame( ) )
		 return oInterpolateServerEntities( );

	  oInterpolateServerEntities( );

	  if ( local && !local->IsDead( ) && local->m_CachedBoneData( ).Count( ) > 0 && ( g_Vars.misc.third_person && g_Vars.misc.third_person_bind.enabled || Source::m_pInput->m_fCameraInThirdPerson ) ) {
		 local->SetAbsAngles( QAngle( 0.0f, g_Vars.globals.flRealYaw, 0.0f ) );

		 // correct render bone matrix
		 {
			matrix3x4_t world_matrix{};
			world_matrix.AngleMatrix( QAngle( 0.f, g_Vars.globals.flRealYaw, 0.f ),
			   local->GetAbsOrigin( ) );

			uint32_t bone_computed[ 8 ] = { 0 };
			auto hdr = local->m_pStudioHdr( );
			Engine::C_SetupBones::Studio_BuildMatrices( hdr, world_matrix, g_Vars.globals.m_RealBonesPositions, g_Vars.globals.m_RealBonesRotations,
			   BONE_USED_BY_ANYTHING, local->m_CachedBoneData( ).Base( ), bone_computed );

			local->m_flLastBoneSetupTime( ) = FLT_MAX;
			local->m_iMostRecentModelBoneCounter( ) = ( *( int* ) Engine::Displacement.Data.m_uModelBoneCounter );
			local->m_BoneAccessor( ).m_ReadableBones = local->m_BoneAccessor( ).m_WritableBones = 0xFFFFFFFF;

			auto backup = local->m_BoneAccessor( ).m_pBones;
			local->m_BoneAccessor( ).m_pBones = local->m_CachedBoneData( ).Base( );

			Engine::C_SetupBones::AttachmentHelper( local, hdr );

			local->m_BoneAccessor( ).m_pBones = backup;

		 #if 0
			typedef void( __thiscall * o_BuildTransformations )( C_CSPlayer*, CStudioHdr*, Vector*, Quaternion*, const matrix3x4_t&, int32_t, uint8_t* );
			Memory::VCall< o_BuildTransformations >( local, 188 )( local,
			   local->m_pStudioHdr( ),
			   g_Vars.globals.m_RealBonesPositions,
			   g_Vars.globals.m_RealBonesRotations,
			   world_matrix,
			   BONE_USED_BY_ANYTHING,
			   bone_computed );

			Studio_BuildMatrices( local->m_pStudioHdr( ), QAngle( 0.f, g_Vars.globals.flRealYaw, 0.f ),
			   local->GetAbsOrigin( ),
			   g_Vars.globals.m_RealBonesPositions,
			   g_Vars.globals.m_RealBonesRotations, -1, 1.0f, local->m_CachedBoneData( ).Base( ), BONE_USED_BY_ANYTHING );
		 #endif
		 }
	  }
   }
}