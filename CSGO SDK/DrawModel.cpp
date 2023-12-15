#include "hooked.hpp"
#include "CChams.hpp"
#include "player.hpp"
//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
struct matrix3x4_t;

struct MDLSquenceLayer_t {
   int		m_nSequenceIndex;
   float	m_flWeight;
   bool	m_bNoLoop;
   float	m_flCycleBeganAt;
};

uintptr_t* rel32( uintptr_t ptr ) {
   auto offset = *( uintptr_t* ) ( ptr + 0x1 );
   return ( uintptr_t* ) ( ptr + 5 + offset );
}

class CMDLAttachmentData {
public:
   matrix3x4_t	m_AttachmentToWorld;
   bool m_bValid;
};

//-----------------------------------------------------------------------------
// Class containing simplistic MDL state for use in rendering
//-----------------------------------------------------------------------------
class CMDL {
public:
	CMDL( ) {
		static auto ctor = ( void( __thiscall* )( void* ) )rel32( Memory::Scan( "client.dll", "E8 ?? ?? ?? ?? 56 8D 4C 24 24" ) );
		ctor( this );
	}

	~CMDL( ) {
		static auto dector = ( void( __thiscall* )( void* ) )rel32( Memory::Scan( "client.dll", "E8 ?? ?? ?? ?? 8D 8E ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B CF" ) );
		dector( this );
	}

	//E8 ? ? ? ? 85 FF 74 42 
	void SetMDL( MDLHandle_t h ) {
		static auto _SetMDL = ( void( __thiscall* )( void*, MDLHandle_t ) )rel32( Memory::Scan( "client.dll", "E8 ?? ?? ?? ?? 85 FF 74 42" ) );
		_SetMDL( this, h );
	}

   //MDLHandle_t GetMDL( ) const;

   // Simple version of drawing; sets up bones for you
   // void Draw( const matrix3x4_t& rootToWorld );

   //NOTE: This version of draw assumes you've filled in the bone to world
   //matrix yourself by calling IStudioRender::LockBoneMatrices. The pointer
   //returned by that method needs to be passed into here
   //@param flags allows you to specify additional STUDIORENDER_ flags -- usually never necessary
   //       unless you need to (eg) forcibly disable shadows for some reason.
	void Draw( const matrix3x4_t& rootToWorld, const matrix3x4_t* pBoneToWorld, int flags = 0 ) {
		static auto _Draw = ( void( __thiscall* )( void*, const matrix3x4_t&, const matrix3x4_t*, int ) )rel32( Memory::Scan( "client.dll", "E8 ?? ?? ?? ?? 8B FB 8D 4D F4" ) );
		_Draw( this, rootToWorld, pBoneToWorld, flags );
	}

	void SetUpBones( const matrix3x4_t& shapeToWorld, int nMaxBoneCount, matrix3x4_t* pOutputMatrices, const float* pPoseParameters = NULL, MDLSquenceLayer_t* pSequenceLayers = NULL, int nNumSequenceLayers = 0 ) {
		static auto _SetUpBones = ( void( __thiscall* )( void*, const matrix3x4_t&, int, matrix3x4_t*, const float*, MDLSquenceLayer_t*, int ) )rel32( Memory::Scan( "client.dll", "E8 ?? ?? ?? ?? 8B CB 8B 01" ) );
		_SetUpBones( this, shapeToWorld, nMaxBoneCount, pOutputMatrices, pPoseParameters, pSequenceLayers, nNumSequenceLayers );
	}

   //void SetupBonesWithBoneMerge( const CStudioHdr *pMergeHdr, matrix3x4_t *pMergeBoneToWorld,
   // const CStudioHdr *pFollow, const matrix3x4_t *pFollowBoneToWorld, const matrix3x4_t &matModelToWorld );

   // studiohdr_t *GetStudioHdr( );
   virtual bool GetAttachment( int number, matrix3x4_t& matrix ) { return false; };

private:
   // void UnreferenceMDL( );
   CUtlVector<CMDLAttachmentData> m_Attachments;
public:
   MDLHandle_t	m_MDLHandle;
   Color		m_Color;
   int			m_nSkin;
   int			m_nBody;
   int			m_nSequence;
   int			m_nLOD;
   float		m_flPlaybackRate;
   float		m_flTime;
   float		m_pFlexControls[ 96 * 4 ];
   Vector		m_vecViewTarget;
   bool	m_bWorldSpaceViewTarget;
   char pad[ 0x420 ]; // for safety, cuz lazy to reverse
};


void __fastcall Hooked::DrawModelExecute( void* ECX, void* EDX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pCustomBoneToWorld ) {
   g_Vars.globals.szLastHookCalled = XorStr( "6" );
   if ( !MatRenderContext || !pCustomBoneToWorld || !g_Vars.globals.RenderIsReady || ECX != Source::m_pModelRender.Xor( ) )
	  return oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

   C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
   if ( !local || !Source::m_pEngine->IsInGame( ) )
	  return oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

   static auto disable_post_processing_override = Source::m_pCvar->FindVar( XorStr( "mat_postprocess_enable" ) );

   if ( g_Vars.esp.remove_post_proccesing && disable_post_processing_override )
	  disable_post_processing_override->SetValue( 0 );
   else
	  disable_post_processing_override->SetValue( 1 );

   static bool run_once = false;
   static bool run_once2 = false;

   if ( g_Vars.esp.remove_blur_effect ) {
	  if ( !run_once ) {
		 auto blur_overlay = Source::m_pMatSystem->FindMaterial( XorStr( "dev/scope_bluroverlay" ), XorStr( TEXTURE_GROUP_OTHER ) );
		 auto lens_dirt = Source::m_pMatSystem->FindMaterial( XorStr( "models/weapons/shared/scope/scope_lens_dirt" ), XorStr( TEXTURE_GROUP_OTHER ) );
		 auto xblur_mat = Source::m_pMatSystem->FindMaterial( XorStr( "dev/blurfilterx_nohdr" ), XorStr( TEXTURE_GROUP_OTHER ) );
		 auto yblur_mat = Source::m_pMatSystem->FindMaterial( XorStr( "dev/blurfiltery_nohdr" ), XorStr( TEXTURE_GROUP_OTHER ) );

		 xblur_mat->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, true );
		 yblur_mat->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, true );
		 blur_overlay->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, true );
		 lens_dirt->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, true );
		 run_once = true;
		 run_once2 = false;
	  }
   } else {
	  if ( !run_once2 ) {
		 auto blur_overlay = Source::m_pMatSystem->FindMaterial( XorStr( "dev/scope_bluroverlay" ), XorStr( TEXTURE_GROUP_OTHER ) );
		 auto lens_dirt = Source::m_pMatSystem->FindMaterial( XorStr( "models/weapons/shared/scope/scope_lens_dirt" ), XorStr( TEXTURE_GROUP_OTHER ) );
		 auto xblur_mat = Source::m_pMatSystem->FindMaterial( XorStr( "dev/blurfilterx_nohdr" ), XorStr( TEXTURE_GROUP_OTHER ) );
		 auto yblur_mat = Source::m_pMatSystem->FindMaterial( XorStr( "dev/blurfiltery_nohdr" ), XorStr( TEXTURE_GROUP_OTHER ) );

		 xblur_mat->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, false );
		 yblur_mat->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, false );
		 blur_overlay->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, false );
		 lens_dirt->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, false );
		 run_once = false;
		 run_once2 = true;
	  }
   }

   Source::IChams::Get( )->DrawModel( ECX, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );
}
