#include "CChams.hpp"
#include "source.hpp"
#include <fstream>
#include "Player.hpp"
#include "IMaterialSystem.hpp"
#include "CVariables.hpp"
#include "LagCompensation.hpp"
#include "SetupBones.hpp"
#include "imgui.h"
#include "hooked.hpp"
#include "displacement.hpp"
#include "Resolver.hpp"
#include "Prediction.hpp"

extern float test_1;
extern float test_2;
extern float test_3;
extern float test_4;
extern C_AnimationLayer FakeAnimLayers[ 13 ];

namespace Source
{
   enum ChamsMaterials {
	  MATERIAL_OFF = 0,
	  MATERIAL_FLAT = 1,
	  MATERIAL_REGULAR,
	  MATERIAL_GLOW,
	  MATERIAL_OUTLINE,
	  MATERIAL_SKULL,
	  MATERIAL_GLOSS,
	  MATERIAL_CRYSTAL,
	  MATERIAL_TREE,
	  MATERIAL_DARUDE,
	  MATERIAL_DOUBLE_GLOW,
   };

   struct C_HitMatrixEntry {
	  int ent_index;
	  ModelRenderInfo_t info;
	  DrawModelState_t state;
	  matrix3x4_t pBoneToWorld[ 128 ] = {};
	  float time;
	  matrix3x4_t model_to_world;
   };

   class CChams : public IChams {
   public:
	  void OnDrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pBoneToWorld ) override;
	  void DrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pBoneToWorld ) override;
	  bool GetBacktrackMatrix( C_CSPlayer* player, matrix3x4_t* out );
	  void OnPostScreenEffects( ) override;
	  bool IsVisibleScan( C_CSPlayer* player );	  virtual void AddHitmatrix( C_CSPlayer* player, matrix3x4_t* bones );

	  CChams( );
	  ~CChams( );

	  virtual bool CreateMaterials( ) {
		 if ( m_bInit )
			return true;

		 m_matRegular = Source::m_pMatSystem->FindMaterial( XorStr( "debug/debugambientcube" ), nullptr );
		 m_matFlat = Source::m_pMatSystem->FindMaterial( XorStr( "debug/debugdrawflat" ), nullptr );
		 m_glow_rim3d = Source::m_pMatSystem->FindMaterial( XorStr( "dev/glow_armsrace" ), nullptr );
		 m_matSkull = Source::m_pMatSystem->FindMaterial( XorStr( "models/gibs/hgibs/skull1" ), nullptr );
		 m_matGloss = Source::m_pMatSystem->FindMaterial( XorStr( "models/inventory_items/trophy_majors/gloss" ), nullptr );
		 m_matCrystal = Source::m_pMatSystem->FindMaterial( XorStr( "models/inventory_items/trophy_majors/crystal_blue" ), nullptr );
		 m_matTree = Source::m_pMatSystem->FindMaterial( XorStr( "models/props_foliage/urban_tree03_branches" ), nullptr );
		 m_matDarude = Source::m_pMatSystem->FindMaterial( XorStr( "models/inventory_items/music_kit/darude_01/mp3_detail" ), nullptr );
		 m_matSpech = Source::m_pMatSystem->FindMaterial( XorStr( "models/extras/speech_info" ), nullptr );

		 if ( !m_matRegular || m_matRegular->IsErrorMaterial( ) )
			return false;

		 if ( !m_matFlat || m_matFlat->IsErrorMaterial( ) )
			return false;

		 if ( !m_glow_rim3d || m_glow_rim3d->IsErrorMaterial( ) )
			return false;

		 if ( !m_matSkull || m_matSkull->IsErrorMaterial( ) )
			return false;

		 if ( !m_matGloss || m_matGloss->IsErrorMaterial( ) )
			return false;

		 if ( !m_matCrystal || m_matCrystal->IsErrorMaterial( ) )
			return false;

		 if ( !m_matTree || m_matTree->IsErrorMaterial( ) )
			return false;

		 if ( !m_matDarude || m_matDarude->IsErrorMaterial( ) )
			return false;

		 if ( !m_matSpech || m_matSpech->IsErrorMaterial( ) )
			return false;

		 m_bInit = true;
		 return true;
	  }

   private:
	  void OverrideMaterial( bool ignoreZ, int type, const FloatColor& rgba, float glow_mod = 0 );

	  bool m_bInit = false;
	  IMaterial* m_matFlat = nullptr;
	  IMaterial* m_matRegular = nullptr;
	  IMaterial* m_glow_rim3d = nullptr;
	  IMaterial* m_matSkull = nullptr;
	  IMaterial* m_matGloss = nullptr;
	  IMaterial* m_matCrystal = nullptr;
	  IMaterial* m_matTree = nullptr;
	  IMaterial* m_matDarude = nullptr;
	  IMaterial* m_matSpech = nullptr;

	  std::vector<C_HitMatrixEntry> m_Hitmatrix;
   };

   Encrypted_t<IChams> IChams::Get( ) {
	  static CChams instance;
	  return &instance;
   }

   CChams::CChams( ) {

   }

   CChams::~CChams( ) {

   }

   void CChams::OnPostScreenEffects( ) {
	  if ( !g_Vars.globals.HackIsReady )
		 m_Hitmatrix.clear( );

	  if ( m_Hitmatrix.empty( ) )
		 return;

	  if ( !Source::m_pStudioRender.Xor( ) )
		 return;

	  auto drawed_shit = false;

	  auto ctx = Source::m_pMatSystem->GetRenderContext( );

	  if ( !ctx )
		 return;

	  auto it = m_Hitmatrix.begin( );
	  while ( it != m_Hitmatrix.end( ) ) {
		 if ( !it->state.m_pModelToWorld || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
			  !it->info.pRenderable || !it->info.pModelToWorld || !it->info.pModel ) {
			++it;
			continue;
		 }

		 auto alpha = 1.0f;
		 auto delta = Source::m_pGlobalVars->realtime - it->time;
		 if ( delta > 0.0f ) {
			alpha -= delta;
			if ( delta > 1.0f ) {
			   it = m_Hitmatrix.erase( it );
			   continue;
			}
		 }

		 drawed_shit = true;

		 auto color = g_Vars.esp.hitmatrix_color;
		 color.a *= alpha;

		 OverrideMaterial( true, g_Vars.esp.chams_hitmatrix_mat, color );
		 Hooked::oDrawModelExecute( Source::m_pModelRender.Xor( ), ctx, it->state, it->info, it->pBoneToWorld );
		 Source::m_pStudioRender->m_pForcedMaterial = nullptr;
		 Source::m_pStudioRender->m_nForcedMaterialType = 0;

		 ++it;
	  }
   }

   void CChams::AddHitmatrix( C_CSPlayer* player, matrix3x4_t* bones ) {
	  auto& hit = m_Hitmatrix.emplace_back( );

	  std::memcpy( hit.pBoneToWorld, bones, player->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );
	  hit.time = Source::m_pGlobalVars->realtime + g_Vars.esp.hitmatrix_time;

	  static int m_nSkin = Horizon::Memory::FindInDataMap( player->GetPredDescMap( ), XorStr( "m_nSkin" ) );
	  static int m_nBody = Horizon::Memory::FindInDataMap( player->GetPredDescMap( ), XorStr( "m_nBody" ) );

	  hit.info.origin = player->GetAbsOrigin( );
	  hit.info.angles = player->GetAbsAngles( );

	  auto renderable = player->GetClientRenderable( );

	  if ( !renderable )
		 return;

	  auto model = player->GetModel( );

	  if ( !model )
		 return;

	  auto hdr = *( studiohdr_t** ) ( player->m_pStudioHdr( ) );

	  if ( !hdr )
		 return;

	  hit.state.m_pStudioHdr = hdr;
	  hit.state.m_pStudioHWData = Source::m_pMDLCache->GetHardwareData( model->studio );
	  hit.state.m_pRenderable = renderable;
	  hit.state.m_drawFlags = 0;

	  hit.info.pRenderable = renderable;
	  hit.info.pModel = model;
	  hit.info.pLightingOffset = nullptr;
	  hit.info.pLightingOrigin = nullptr;
	  hit.info.hitboxset = player->m_nHitboxSet( );
	  hit.info.skin = *( int* ) ( uintptr_t( player ) + m_nSkin );
	  hit.info.body = *( int* ) ( uintptr_t( player ) + m_nBody );
	  hit.info.entity_index = player->m_entIndex;
	  hit.info.instance = Memory::VCall<ModelInstanceHandle_t( __thiscall* )( void* ) >( renderable, 30u )( renderable );
	  hit.info.flags = 0x1;

	  hit.info.pModelToWorld = &hit.model_to_world;
	  hit.state.m_pModelToWorld = &hit.model_to_world;

	  hit.model_to_world.AngleMatrix( hit.info.angles, hit.info.origin );
   }

   void CChams::OverrideMaterial( bool ignoreZ, int type, const FloatColor& rgba, float glow_mod ) {
	  IMaterial* material = nullptr;

	  switch ( type ) {
		 case MATERIAL_OFF: break;
		 case MATERIAL_FLAT:
		 material = m_matFlat; break;
		 case MATERIAL_REGULAR:
		 material = m_matRegular; break;
		 case MATERIAL_GLOW:
		 case MATERIAL_OUTLINE:
		 material = m_glow_rim3d; break;
		 case MATERIAL_SKULL:
		 material = m_matSkull; break;
		 case MATERIAL_GLOSS:
		 material = m_matGloss; break;
		 case MATERIAL_CRYSTAL:
		 material = m_matCrystal; break;
		 case MATERIAL_TREE:
		 material = m_matTree; break;
		 case MATERIAL_DARUDE:
		 material = m_matDarude; break;
		 case MATERIAL_DOUBLE_GLOW:
		 material = m_matSpech; break;
	  }

	  if ( !material ) {
		 Source::m_pStudioRender->m_pForcedMaterial = nullptr;
		 Source::m_pStudioRender->m_nForcedMaterialType = 0;
		 return;
	  }

	  material->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, ignoreZ );

	  if ( type == MATERIAL_GLOW ) {
		 auto var = material->FindVar( XorStr( "$envmaptint" ), nullptr );
		 if ( var )
			var->SetVecValue( rgba.r,
							  rgba.g,
							  rgba.b );

		 var = material->FindVar( XorStr( "$alpha" ), nullptr );
		 if ( var )
			var->SetFloatValue( rgba.a );

		 var = material->FindVar( XorStr( "$envmapfresnelminmaxexp" ), nullptr );
		 if ( var )
			var->SetVecValue( 0.f, 1.f, glow_mod );
	  } else if ( type == MATERIAL_OUTLINE ) {
		 auto var = material->FindVar( XorStr( "$envmaptint" ), nullptr );
		 if ( var )
			var->SetVecValue( rgba.r,
							  rgba.g,
							  rgba.b );

		 var = material->FindVar( XorStr( "$alpha" ), nullptr );
		 if ( var )
			var->SetFloatValue( rgba.a );

		 var = material->FindVar( XorStr( "$envmapfresnelminmaxexp" ), nullptr );
		 if ( var )
			var->SetVecValue( 0.f, 1.f, 8.0f );
	  } else {
		 material->AlphaModulate(
			rgba.a );

		 material->ColorModulate(
			rgba.r,
			rgba.g,
			rgba.b );
	  }

	  Source::m_pStudioRender->m_pForcedMaterial = material;
	  Source::m_pStudioRender->m_nForcedMaterialType = 0;
   }


   static bool override_material = true;
   void CChams::DrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pBoneToWorld ) {
	  if ( !Source::m_pStudioRender.IsValid( ) )
		 goto end;

	  if ( !CreateMaterials( ) )
		 goto end;

	  if ( !g_Vars.esp.chams_enabled )
		 goto end_func;

	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || !Source::m_pEngine->IsInGame( ) )
		 goto end;

	  static float pulse_alpha = 0.f;
	  static bool change_alpha = false;

	  if ( pulse_alpha <= 0.f )
		 change_alpha = true;
	  else if ( pulse_alpha >= 255.f )
		 change_alpha = false;

	  pulse_alpha = change_alpha ? pulse_alpha + 0.05f : pulse_alpha - 0.05f;

	  // STUDIO_SHADOWDEPTHTEXTURE(used for shadows)
	  if ( RenderInfo.flags & 0x40000000
		   || !override_material
		   || !RenderInfo.pRenderable
		   || !RenderInfo.pRenderable->GetIClientUnknown( ) ) {
		 goto end;
	  }

	  auto entity = ( C_CSPlayer* ) ( RenderInfo.pRenderable->GetIClientUnknown( )->GetBaseEntity( ) );
	  if ( !entity || entity->IsDead( ) )
		 goto end;

	  auto client_class = entity->GetClientClass( );
	  if ( !client_class->m_ClassID )
		 goto end;

	  // already have forced material ( glow outline ) 
	  if ( Source::m_pStudioRender->m_pForcedMaterial && client_class->m_ClassID != ClassId_t::CPredictedViewModel ) {
		 goto end;
	  }

	  if ( client_class->m_ClassID == ClassId_t::CBaseAnimating ) {
		 if ( g_Vars.esp.remove_hands )
			return;

		 if ( g_Vars.esp.remove_sleeves && strstr( DrawModelState.m_pStudioHdr->szName, XorStr( "sleeve" ) ) != nullptr )
			return;
	  }

	  auto InvalidateMaterial = [&] ( ) -> void {
		 Source::m_pStudioRender->m_pForcedMaterial = nullptr;
		 Source::m_pStudioRender->m_nForcedMaterialType = 0;
	  };

	  auto InvertValue = [ & ]( float invert_from, float invert_to, float value ) -> float {
		 float max_delta = value - invert_to;


	  };

	  if ( client_class->m_ClassID == ClassId_t::CPredictedViewModel ) {
		 if ( !g_Vars.esp.chams_weapon )
			goto end;

		 int material = g_Vars.esp.weapon_chams_mat;

		 if ( g_Vars.esp.chams_weapon_pulse ) {
			FloatColor clr = g_Vars.esp.weapon_chams_color;
			clr.a = pulse_alpha / 255.f;
			OverrideMaterial( false, material, clr );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 } else {
			OverrideMaterial( false, material, g_Vars.esp.weapon_chams_color, 0.f );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 }

		 if ( g_Vars.esp.chams_weapon_outline ) {
			OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_weapon_outline_color, g_Vars.esp.chams_weapon_outline_value );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 }

		 InvalidateMaterial( );
		 return;
	  } else if ( client_class->m_ClassID == ClassId_t::CBaseAnimating ) {
		 if ( !g_Vars.esp.chams_hands )
			goto end;

		 int material = g_Vars.esp.hands_chams_mat;

		 if ( g_Vars.esp.chams_hands_pulse ) {
			FloatColor clr = g_Vars.esp.hands_chams_color;
			clr.a = pulse_alpha / 255.f;
			OverrideMaterial( false, material, clr );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 } else {
			OverrideMaterial( false, material, g_Vars.esp.hands_chams_color, 0.f );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 }
		 if ( g_Vars.esp.chams_hands_outline ) {
			OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_hands_outline_color, g_Vars.esp.chams_hands_outline_value );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 }

		 InvalidateMaterial( );
		 return;
	  } else if ( entity
				  && entity->IsPlayer( )
				  && !entity->IsDormant( )
				  && entity->m_entIndex >= 1 && entity->m_entIndex <= Source::m_pGlobalVars->maxClients || entity->GetClientClass( )->m_ClassID == ClassId_t::CCSRagdoll ) {
		 bool is_local_player = false, is_enemy = false, is_teammate = false;

		 if ( entity->entindex( ) == local->entindex( ) )
			is_local_player = true;
		 else if ( entity->m_iTeamNum( ) != local->m_iTeamNum( ) )
			is_enemy = true;
		 else
			is_teammate = true;

		 if ( entity == local ) {
			if ( g_Vars.esp.blur_in_scoped && Source::m_pInput->m_fCameraInThirdPerson ) {
			   if ( local && !local->IsDead( ) && local->m_bIsScoped( ) ) {
				  Source::m_pRenderView->SetBlend( g_Vars.esp.blur_in_scoped_value );
			   }
			}
		 }

		 bool ragdoll = entity->GetClientClass( )->m_ClassID == ClassId_t::CCSRagdoll;

		 auto vis = is_teammate ? g_Vars.esp.team_chams_color_vis : g_Vars.esp.enemy_chams_color_vis;
		 auto xqz = is_teammate ? g_Vars.esp.team_chams_color_xqz : g_Vars.esp.enemy_chams_color_xqz;
		 bool should_xqz = is_teammate ? g_Vars.esp.team_chams_xqz : g_Vars.esp.enemy_chams_xqz;
		 bool should_vis = is_teammate ? g_Vars.esp.team_chams_vis : g_Vars.esp.enemy_chams_vis;

		 auto vis_death = is_teammate ? g_Vars.esp.team_death_chams_color_vis : g_Vars.esp.enemy_death_chams_color_vis;
		 auto xqz_death = is_teammate ? g_Vars.esp.team_death_chams_color_xqz : g_Vars.esp.enemy_death_chams_color_xqz;
		 bool should_xqz_death = is_teammate ? g_Vars.esp.team_death_chams_xqz : g_Vars.esp.enemy_death_chams_xqz;
		 bool should_vis_death = is_teammate ? g_Vars.esp.team_death_chams_vis : g_Vars.esp.enemy_death_chams_vis;

		 if ( is_local_player ) {
			//set local player ghost chams
			if ( g_Vars.esp.chams_ghost ) {
			   auto animState = local->m_PlayerAnimState( );

			   Encrypted_t<CVariables::GLOBAL> globals( &g_Vars.globals );
			   alignas( 16 ) static matrix3x4_t tmp_matrix[ 256 ];

			   {
				  float m_flOldCurtime = Source::m_pGlobalVars->curtime;
				  float m_flOldFrametime = Source::m_pGlobalVars->frametime;

				  Source::m_pGlobalVars->curtime = TICKS_TO_TIME( local->m_nTickBase( ) );
				  Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;

				  alignas( 16 ) Vector vecBones[ 256 ];
				  alignas( 16 ) Quaternion quatBones[ 256 ];

				  auto flWeightBackup = FakeAnimLayers[ 12 ].m_flWeight;
				  auto flWeight3Backup = FakeAnimLayers[ 3 ].m_flWeight;
				  auto flCycle3Backup = FakeAnimLayers[ 3 ].m_flCycle;

				  // fix legs shake in air
				  FakeAnimLayers[ 12 ].m_flWeight = 0.0f;
				  // nop 3 animlayer ( remove 979 )
				  FakeAnimLayers[ 3 ].m_flWeight = 0.f;
				  FakeAnimLayers[ 3 ].m_flCycle = 0.f;

				  // build fake bone matrix
				  Engine::C_SetupBones boneSetup;
				  boneSetup.Init( local, ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_ATTACHMENT ), tmp_matrix );
				  boneSetup.m_bShouldAttachment = false;
				  boneSetup.m_bShouldDispatch = true;
				  boneSetup.m_vecBones = vecBones;
				  boneSetup.m_quatBones = quatBones;
				  boneSetup.m_vecOrigin = local->GetAbsOrigin( );
				  boneSetup.m_angAngles = globals->m_FakeAngles;
				  boneSetup.m_animLayers = FakeAnimLayers;
				  boneSetup.m_nAnimOverlayCount = 13;
				  std::memcpy( boneSetup.m_flPoseParameters, globals->m_flFakePoseParams, sizeof( globals->m_flFakePoseParams ) );
				  std::memcpy( boneSetup.m_flWorldPoses, globals->FakeWeaponPoses, sizeof( globals->FakeWeaponPoses ) );

				  boneSetup.Setup( );

				  // restore animlayers
				  FakeAnimLayers[ 12 ].m_flWeight = flWeightBackup;
				  FakeAnimLayers[ 3 ].m_flWeight = flWeight3Backup;
				  FakeAnimLayers[ 3 ].m_flCycle = flCycle3Backup;

				  // restore globals
				  Source::m_pGlobalVars->curtime = m_flOldCurtime;
				  Source::m_pGlobalVars->frametime = m_flOldFrametime;
			   }

			   if ( g_Vars.esp.chams_ghost_pulse ) {
				  FloatColor clr = g_Vars.esp.chams_desync_color;
				  clr.a = pulse_alpha / 255.f;
				  OverrideMaterial( false, g_Vars.esp.chams_desync_mat, clr );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, tmp_matrix );
			   } else {
				  OverrideMaterial( false, g_Vars.esp.chams_desync_mat, g_Vars.esp.chams_desync_color );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, tmp_matrix );
			   }
			   
			   if ( g_Vars.esp.chams_ghost_outline ) {
				  OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_ghost_outline_color, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_ghost_outline_value ) * 0.2f, 1.f, 20.f ) );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, tmp_matrix );
			   }
			}

			//set local player lag chams
			if ( g_Vars.esp.chams_lag ) {
			   if ( g_Vars.globals.LagPosition[ 0 ].at( 3 ) != pBoneToWorld[ 0 ].at( 3 ) &&
					g_Vars.globals.LagPosition[ 1 ].at( 3 ) != pBoneToWorld[ 1 ].at( 3 ) &&
					g_Vars.globals.LagPosition[ 2 ].at( 3 ) != pBoneToWorld[ 2 ].at( 3 ) &&
					g_Vars.globals.LagOrigin.Distance( entity->m_vecOrigin( ) ) > 1.0f ) {

				  if ( g_Vars.esp.chams_lag_pulse ) {
					 FloatColor clr = g_Vars.esp.chams_lag_color;
					 clr.a = pulse_alpha / 255.f;
					 OverrideMaterial( false, g_Vars.esp.chams_lag_mat, clr );
					 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, g_Vars.globals.LagPosition );
				  } else {
					 OverrideMaterial( false, g_Vars.esp.chams_lag_mat, g_Vars.esp.chams_lag_color );
					 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, g_Vars.globals.LagPosition );
				  }
				  
				  if ( g_Vars.esp.chams_lag_outline ) {
					 OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_lag_outline_color, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_lag_outline_value ) * 0.2f, 1.f, 20.f ) );
					 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, g_Vars.globals.LagPosition );
				  }
			   }
			}

			//set local player chams
			if ( g_Vars.esp.chams_local ) {
			   if ( g_Vars.esp.chams_local_pulse ) {
				  FloatColor clr = g_Vars.esp.chams_local_color;
				  clr.a = pulse_alpha / 255.f;
				  OverrideMaterial( false, g_Vars.esp.chams_local_mat, clr );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			   } else {
				  OverrideMaterial( true, g_Vars.esp.chams_local_mat, g_Vars.esp.chams_local_color );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			   }

			   if ( g_Vars.esp.chams_local_outline ) {
				  OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_local_outline_color, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_local_outline_value ) * 0.2f, 1.f, 20.f ) );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			   }
			} else {
			   OverrideMaterial( false, 0, vis );
			   Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			}

			InvalidateMaterial( );
			return;
		 } else if ( is_enemy ) {
			if ( g_Vars.esp.chams_history ) {
			   auto data = Engine::LagCompensation::Get( )->GetLagData( entity->m_entIndex );
			   if ( data.IsValid( ) ) {
				  Engine::C_LagRecord* record = nullptr;

				  // start from begin
				  float time = 0.2f;
				  matrix3x4_t out[ 128 ];

				  for ( auto it = data->m_History.begin( ); it != data->m_History.end( ); ++it ) {
					 if ( it->player != entity )
						break;

					 std::pair< Engine::C_LagRecord*, Engine::C_LagRecord* > last;

					 if ( !Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *it, time, 0, true ) && it + 1 != data->m_History.end( ) && Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *( it + 1 ), time, 0, true ) )
						last = std::make_pair( &*( it + 1 ), &*it );

					 if ( !last.first || !last.second )
						continue;

					 const auto& first_invalid = last.first;
					 const auto& last_valid = last.second;

					 if ( last_valid->m_flSimulationTime - first_invalid->m_flSimulationTime > 0.5f )
						continue;

					 if ( !it->m_bIsValid ) {
						continue;
					 }

					 const auto next = last_valid->m_vecOrigin;
					 const auto curtime = Engine::Prediction::Instance( )->GetCurtime( );

					 auto delta = 1.f - ( curtime - last_valid->m_flInterpolateTime ) / ( last_valid->m_flSimulationTime - first_invalid->m_flSimulationTime );
					 if ( delta < 0.f || delta > 1.f )
						last_valid->m_flInterpolateTime = curtime;

					 delta = 1.f - ( curtime - last_valid->m_flInterpolateTime ) / ( last_valid->m_flSimulationTime - first_invalid->m_flSimulationTime );

					 const auto lerp = Math::Interpolate( next, first_invalid->m_vecOrigin, std::clamp( delta, 0.f, 1.f ) );

					 auto matrix =  &first_invalid->m_BoneMatrix[ 0 ];

					 /*
					 g_Vars.rage.visual_resolver ? first_invalid->GetBoneMatrix( g_BruteforceData[ entity->m_entIndex ].GetYawSide( entity->m_entIndex, *first_invalid ) ) :
					 */

					 matrix3x4_t ret[ 128 ];
					 memcpy( ret, matrix, sizeof( matrix3x4_t[ 128 ] ) );

					 for ( size_t i{}; i < 128; ++i ) {
						const auto matrix_delta = Math::MatrixGetOrigin( matrix[ i ] ) - first_invalid->m_vecOrigin;
						Math::MatrixSetOrigin( matrix_delta + lerp, ret[ i ] );
					 }

					 memcpy( out, ret, sizeof( matrix3x4_t[ 128 ] ) );

					 if ( !record
						  || record->m_vecOrigin != it->m_vecOrigin
						  || record->m_vecMaxs.z != it->m_vecMaxs.z ) {
						record = &*it;
					 }

				  }

				  if ( record && record->m_vecVelocity.Length2D( ) > 10.f ) {
					 if ( g_Vars.esp.chams_history_pulse ) {
						FloatColor clr = g_Vars.esp.chams_history_color;
						clr.a = pulse_alpha / 255.f;
						OverrideMaterial( false, g_Vars.esp.chams_history_mat, clr );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, out );
					 } else {
						OverrideMaterial( true, g_Vars.esp.chams_history_mat, g_Vars.esp.chams_history_color );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, out );
					 }

					 if ( g_Vars.esp.chams_history_outline ) {
						OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_history_outline_color, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_history_outline_value ) * 0.2f, 1.f, 20.f ) );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, out );
					 }
				  }
			   }
			}

			//set enemy chams
			if ( g_Vars.esp.chams_death_enemy ) {
			   if ( ragdoll ) {
				  int material = g_Vars.esp.enemy_chams_death_mat;
				  if ( g_Vars.esp.chams_enemy_death_pulse ) {
					 if ( should_xqz_death ) {
						FloatColor clr = xqz_death;
						clr.a = pulse_alpha / 255.f;
						OverrideMaterial( true, material, clr );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }

					 if ( should_vis_death ) {
						FloatColor clr = vis_death;
						clr.a = pulse_alpha / 255.f;
						OverrideMaterial( false, material, clr );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }
				  } else {
					 if ( should_xqz_death ) {
						OverrideMaterial( true, material, xqz_death );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }

					 if ( should_vis_death ) {
						OverrideMaterial( false, material, vis_death );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }
				  }

				  if ( g_Vars.esp.chams_enemy_death_outline ) {
					 OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_enemy_death_outline_color, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_enemy_death_outline_value ) * 0.2f, 1.f, 20.f ) );
					 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
				  }
			   }
			} else {
			   if ( ragdoll ) {
				  InvalidateMaterial( );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			   }
			}

			//set enemy chams
			if ( g_Vars.esp.chams_enemy ) {
			   if ( !ragdoll ) {
				  int material = g_Vars.esp.enemy_chams_mat;
				  if ( g_Vars.esp.chams_enemy_pulse ) {
					 if ( should_xqz ) {
						FloatColor clr = xqz;
						clr.a = pulse_alpha / 255.f;
						OverrideMaterial( true, material, clr );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }

					 if ( should_vis ) {
						FloatColor clr = vis;
						clr.a = pulse_alpha / 255.f;
						OverrideMaterial( false, material, clr );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }
				  } else {
					 if ( should_xqz ) {
						OverrideMaterial( true, material, xqz );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }

					 if ( should_vis ) {
						OverrideMaterial( false, material, vis );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }
				  }
				  
				  if ( g_Vars.esp.chams_enemy_outline ) {
					 OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_enemy_outline_color, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_enemy_outline_value ) * 0.2f, 1.f, 20.f ) );
					 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
				  }
			   }
			} else {
			   if ( !ragdoll ) {
				  InvalidateMaterial( );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			   }
			}

			InvalidateMaterial( );
			return;
		 } else if ( is_teammate ) {
			//set teammate chams
			if ( g_Vars.esp.chams_death_teammate ) {
			   if ( ragdoll ) {
				  int material = g_Vars.esp.team_chams_death_mat;
				  if ( g_Vars.esp.chams_teammate_death_pulse ) {
					 if ( should_xqz_death ) {
						FloatColor clr = xqz_death;
						clr.a = pulse_alpha / 255.f;
						OverrideMaterial( true, material, clr );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }

					 if ( should_vis_death ) {
						FloatColor clr = vis_death;
						clr.a = pulse_alpha / 255.f;
						OverrideMaterial( false, material, clr );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }
				  } else {
					 if ( should_xqz_death ) {
						OverrideMaterial( true, material, xqz_death );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }

					 if ( should_vis_death ) {
						OverrideMaterial( false, material, vis_death );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }
				  }

				  if ( g_Vars.esp.chams_teammate_death_outline ) {
					 OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_teammate_death_outline_color, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_teammate_death_outline_value ) * 0.2f, 1.f, 20.f ) );
					 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
				  }
			   }
			} else {
			   if ( ragdoll ) {
				  InvalidateMaterial( );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			   }
			}

			//set teammate chams
			if ( g_Vars.esp.chams_teammate ) {
			   if ( !ragdoll ) {
				  int material = g_Vars.esp.team_chams_mat;
				  if ( g_Vars.esp.chams_teammate_pulse ) {
					 if ( should_xqz ) {
						FloatColor clr = xqz;
						clr.a = pulse_alpha;
						OverrideMaterial( true, material, clr );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }

					 if ( should_vis ) {
						FloatColor clr = vis;
						clr.a = pulse_alpha;
						OverrideMaterial( false, material, clr );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }
				  } else {
					 if ( should_xqz ) {
						OverrideMaterial( true, material, xqz );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }

					 if ( should_vis ) {
						OverrideMaterial( false, material, vis );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					 }
				  }

				  if ( g_Vars.esp.chams_teammate_outline ) { 
					 OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_teammate_outline_color, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_teammate_outline_value ) * 0.2f, 1.f, 20.f ) );
					 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
				  }
			   }
			} else {
			   if ( !ragdoll ) {
				  InvalidateMaterial( );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			   }
			}

			InvalidateMaterial( );
			return;
		 }
	  }

   end:
	  if ( !Source::m_pEngine->IsInGame( ) || !Source::m_pEngine->IsConnected( ) || !local )
		 return;
	  else {
		 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 return;
	  }

   end_func:
	  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
	  return;

   }

   void CChams::OnDrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pBoneToWorld ) {
   #if 0
	  if ( !Source::m_pStudioRender.IsValid( ) )
		 goto end;

	  if ( !CreateMaterials( ) )
		 goto end;

	  if ( !g_Vars.esp.chams_enabled )
		 goto end_func;

	  C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	  if ( !local || !Source::m_pEngine->IsInGame( ) )
		 goto end;

	  alignas( 16 ) static matrix3x4_t tmp_matrix[ 256 ];
	  bool is_local = false;

	  // STUDIO_SHADOWDEPTHTEXTURE(used for shadows)
	  if ( RenderInfo.flags & 0x40000000
		   || !override_material
		   || !RenderInfo.pRenderable
		   || !RenderInfo.pRenderable->GetIClientUnknown( ) ) {
		 goto end;
	  }

	  auto entity = ( C_CSPlayer* ) ( RenderInfo.pRenderable->GetIClientUnknown( )->GetBaseEntity( ) );
	  if ( !entity || entity->IsDead( ) )
		 goto end;

	  auto client_class = entity->GetClientClass( );
	  if ( !client_class->m_ClassID )
		 goto end;

	  // already have forced material ( glow outline ) 
	  if ( Source::m_pStudioRender->m_pForcedMaterial && client_class->m_ClassID != ClassId_t::CPredictedViewModel ) {
		 goto end;
	  }

	  if ( client_class->m_ClassID == ClassId_t::CBaseAnimating ) {
		 if ( g_Vars.esp.remove_hands )
			return;

		 if ( g_Vars.esp.remove_sleeves && strstr( DrawModelState.m_pStudioHdr->szName, XorStr( "sleeve" ) ) != nullptr )
			return;
	  }

	  auto model_name = DrawModelState.m_pStudioHdr->szName;
	  if ( client_class->m_ClassID == ClassId_t::CPredictedViewModel ) {
		 if ( !g_Vars.esp.weapon_chams )
			goto end;

		 int material = g_Vars.esp.weapon_chams_mat + 1;

		 OverrideMaterial( false, material, g_Vars.esp.weapon_chams_color, 0.f );
		 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );

		 Source::m_pStudioRender->m_pForcedMaterial = nullptr;
		 Source::m_pStudioRender->m_nForcedMaterialType = 0;
		 return;
	  } else if ( client_class->m_ClassID == ClassId_t::CBaseAnimating ) {
		 if ( !g_Vars.esp.hands_chams )
			goto end;

		 int material = g_Vars.esp.hands_chams_mat + 1;
		 OverrideMaterial( false, material, g_Vars.esp.hands_chams_color, 0.f, true );
		 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );

		 if ( wireframe ) {
			OverrideMaterial( false, 1, wireframe, g_Vars.esp.hands_chams_color );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 }

		 Source::m_pStudioRender->m_pForcedMaterial = nullptr;
		 Source::m_pStudioRender->m_nForcedMaterialType = 0;
		 return;
	  } else if ( entity
				  && entity->IsPlayer( )
				  && !entity->IsDormant( )
				  && entity->m_entIndex >= 1 && entity->m_entIndex <= Source::m_pGlobalVars->maxClients || entity->GetClientClass( )->m_ClassID == ClassId_t::CCSRagdoll ) {



		 bool is_visible = IsVisibleScan( entity );
		 auto team = local->IsTeammate( entity );
		 if ( entity->GetClientClass( )->m_ClassID == ClassId_t::CCSRagdoll ) {
			if ( team && !g_Vars.esp.chams_on_death_teamate )
			   goto end;
			else if ( !team && !g_Vars.esp.chams_on_death_enemy )
			   goto end;
		 }

		 is_local = local->m_entIndex == entity->m_entIndex;

		 if ( !is_local && team && g_Vars.esp.team_check ) {
			goto end;
		 }

		 if ( entity == local ) {
			if ( g_Vars.esp.blur_in_scoped && Source::m_pInput->m_fCameraInThirdPerson ) {
			   if ( local && !local->IsDead( ) && local->m_bIsScoped( ) ) {
				  Source::m_pRenderView->SetBlend( g_Vars.esp.blur_in_scoped_value );
			   }
			}
		 }

	  #if 0
		 else if ( g_Vars.rage.enabled && g_Vars.rage.visual_resolver ) {
			auto data = Engine::LagCompensation::Get( )->GetLagData( entity->m_entIndex );
			if ( data.IsValid( ) && data->m_History.size( ) > 0 ) {
			   auto record = data->m_History.front( );
			   pBoneToWorld = record.GetBoneMatrix( record.m_iResolverSide );
			}
		 }
	  #endif

		 if ( is_local && g_Vars.esp.chams_desync ) {
			auto animState = local->m_PlayerAnimState( );

			Encrypted_t<CVariables::GLOBAL> globals( &g_Vars.globals );

			{
			   float m_flOldCurtime = Source::m_pGlobalVars->curtime;
			   float m_flOldFrametime = Source::m_pGlobalVars->frametime;

			   Source::m_pGlobalVars->curtime = TICKS_TO_TIME( local->m_nTickBase( ) );
			   Source::m_pGlobalVars->frametime = Source::m_pGlobalVars->interval_per_tick;

			   alignas( 16 ) Vector vecBones[ 256 ];
			   alignas( 16 ) Quaternion quatBones[ 256 ];

			   auto flWeightBackup = FakeAnimLayers[ 12 ].m_flWeight;
			   auto flWeight3Backup = FakeAnimLayers[ 3 ].m_flWeight;
			   auto flCycle3Backup = FakeAnimLayers[ 3 ].m_flCycle;

			   // fix legs shake in air
			   FakeAnimLayers[ 12 ].m_flWeight = 0.0f;
			   // nop 3 animlayer ( remove 979 )
			   FakeAnimLayers[ 3 ].m_flWeight = 0.f;
			   FakeAnimLayers[ 3 ].m_flCycle = 0.f;

			   // build fake bone matrix
			   Engine::C_SetupBones boneSetup;
			   boneSetup.Init( local, ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_ATTACHMENT ), tmp_matrix );
			   boneSetup.m_bShouldAttachment = false;
			   boneSetup.m_bShouldDispatch = true;
			   boneSetup.m_vecBones = vecBones;
			   boneSetup.m_quatBones = quatBones;
			   boneSetup.m_vecOrigin = local->GetAbsOrigin( );
			   boneSetup.m_angAngles = globals->m_FakeAngles;
			   boneSetup.m_animLayers = FakeAnimLayers;
			   boneSetup.m_nAnimOverlayCount = 13;
			   std::memcpy( boneSetup.m_flPoseParameters, globals->m_flFakePoseParams, sizeof( globals->m_flFakePoseParams ) );
			   std::memcpy( boneSetup.m_flWorldPoses, globals->FakeWeaponPoses, sizeof( globals->FakeWeaponPoses ) );

			   boneSetup.Setup( );

			   // restore animlayers
			   FakeAnimLayers[ 12 ].m_flWeight = flWeightBackup;
			   FakeAnimLayers[ 3 ].m_flWeight = flWeight3Backup;
			   FakeAnimLayers[ 3 ].m_flCycle = flCycle3Backup;

			   // restore globals
			   Source::m_pGlobalVars->curtime = m_flOldCurtime;
			   Source::m_pGlobalVars->frametime = m_flOldFrametime;
			}

			// material part
			int material = g_Vars.esp.chams_desync_mat + 1;

			OverrideMaterial( false, material, false, g_Vars.esp.chams_desync_color );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, tmp_matrix );
		 }

		 if ( g_Vars.esp.chams_history && !is_local ) {
			auto data = Engine::LagCompensation::Get( )->GetLagData( entity->m_entIndex );
			if ( data.IsValid( ) ) {
			   Engine::C_LagRecord* record = nullptr;

			   // start from begin
			   float time = 0.2f;
			   matrix3x4_t out[ 128 ];

			   for ( auto it = data->m_History.begin( ); it != data->m_History.end( ); ++it ) {
				  if ( it->player != entity )
					 break;

				  std::pair< Engine::C_LagRecord*, Engine::C_LagRecord* > last;

				  if ( !Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *it, time, 0, true ) && it + 1 != data->m_History.end( ) && Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *( it + 1 ), time, 0, true ) )
					 last = std::make_pair( &*( it + 1 ), &*it );

				  if ( !last.first || !last.second )
					 continue;

				  const auto& first_invalid = last.first;
				  const auto& last_valid = last.second;

				  /*if ( ( first_invalid->m_vecOrigin - it->player->GetAbsOrigin( ) ).Length( ) < 7.5f )
					 continue;*/

				  if ( last_valid->m_flSimulationTime - first_invalid->m_flSimulationTime > 0.5f )
					 continue;

				  if ( /*Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *it, time, 0, true )
					   ||*/ it->m_bSkipDueToResolver
					   || !it->m_bIsValid ) {
					 continue;
				  }

				  const auto next = last_valid->m_vecOrigin;
				  const auto curtime = Engine::Prediction::Instance( )->GetCurtime( );

				  auto delta = 1.f - ( curtime - last_valid->m_flInterpolateTime ) / ( last_valid->m_flSimulationTime - first_invalid->m_flSimulationTime );
				  if ( delta < 0.f || delta > 1.f )
					 last_valid->m_flInterpolateTime = curtime;

				  delta = 1.f - ( curtime - last_valid->m_flInterpolateTime ) / ( last_valid->m_flSimulationTime - first_invalid->m_flSimulationTime );

				  const auto lerp = Math::Interpolate( next, first_invalid->m_vecOrigin, std::clamp( delta, 0.f, 1.f ) );

				  auto matrix = g_Vars.rage.visual_resolver ? first_invalid->GetBoneMatrix( g_BruteforceData[ entity->m_entIndex ].GetYawSide( entity->m_entIndex, *first_invalid ) ) : &first_invalid->m_BoneMatrix[ 0 ];

				  matrix3x4_t ret[ 128 ];
				  memcpy( ret, matrix, sizeof( matrix3x4_t[ 128 ] ) );

				  for ( size_t i{}; i < 128; ++i ) {
					 const auto matrix_delta = Math::MatrixGetOrigin( matrix[ i ] ) - first_invalid->m_vecOrigin;
					 Math::MatrixSetOrigin( matrix_delta + lerp, ret[ i ] );
				  }

				  memcpy( out, ret, sizeof( matrix3x4_t[ 128 ] ) );

				  /* if ( it->m_vecVelocity.Length2D( ) < 10.f )
					  continue;*/

				  if ( !record
					   || record->m_vecOrigin != it->m_vecOrigin
					   || record->m_vecMaxs.z != it->m_vecMaxs.z ) {
					 record = &*it;
				  }

			   }

			   if ( record && record->m_vecVelocity.Length2D( ) > 10.f ) {
				  OverrideMaterial( true, 1, false, g_Vars.esp.chams_history_color );
				  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, out );
			   }
			}
		 }

		 auto vis = team ? g_Vars.esp.team_chams_color_vis : g_Vars.esp.enemy_chams_color_vis;
		 auto xqz = team ? g_Vars.esp.team_chams_color_xqz : g_Vars.esp.enemy_chams_color_xqz;
		 bool should_xqz = team ? g_Vars.esp.team_chams_xqz : g_Vars.esp.enemy_chams_xqz;
		 bool should_vis = team ? g_Vars.esp.team_chams_vis : g_Vars.esp.enemy_chams_vis;

		 if ( is_local ) {
			vis = g_Vars.esp.chams_local_color;
			should_xqz = false;

			if ( g_Vars.esp.chams_lag && g_Vars.globals.LagPosition[ 0 ].at( 3 ) != pBoneToWorld[ 0 ].at( 3 ) &&
				 g_Vars.globals.LagPosition[ 1 ].at( 3 ) != pBoneToWorld[ 1 ].at( 3 ) &&
				 g_Vars.globals.LagPosition[ 2 ].at( 3 ) != pBoneToWorld[ 2 ].at( 3 ) &&
				 g_Vars.globals.LagOrigin.Distance( entity->m_vecOrigin( ) ) > 1.0f ) {
			   OverrideMaterial( true, 1, false, g_Vars.esp.chams_lag_color );
			   Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, g_Vars.globals.LagPosition );
			}
		 }

		 int material = is_local ? ( g_Vars.esp.chams_local_mat ) : ( team ? g_Vars.esp.team_chams_mat : g_Vars.esp.enemy_chams_mat );
		 bool wireframe = is_local ? g_Vars.esp.wireframe_chams_l : team ? g_Vars.esp.wireframe_chams_t : g_Vars.esp.wireframe_chams_e;
		 if ( !is_local && should_xqz ) {
			OverrideMaterial( true, material, false, xqz );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );

			if ( g_Vars.esp.second_color_l && is_local ) {
			   OverrideMaterial( true, ChamsMaterials::MATERIAL_GLOW_, false, g_Vars.esp.second_chams_color_l, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_shine_l ) * 0.2f, 1.f, 20.f ) );
			   Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			}

			if ( g_Vars.esp.second_color_e && !team ) {
			   OverrideMaterial( true, ChamsMaterials::MATERIAL_GLOW_, false, g_Vars.esp.second_chams_color_e, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_shine_e ) * 0.2f, 1.f, 20.f ) );
			   Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			}

			if ( g_Vars.esp.second_color_t && team ) {
			   OverrideMaterial( true, ChamsMaterials::MATERIAL_GLOW_, false, g_Vars.esp.second_chams_color_t, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_shine_t ) * 0.2f, 1.f, 20.f ) );
			   Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			}
		 }

		 if ( !is_local && !should_vis )
			material = 0;

		 OverrideMaterial( false, material, false, vis );
		 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );

		 if ( wireframe && is_local ) {
			OverrideMaterial( false, 1, true, g_Vars.esp.wireframe_chams_color_l );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );

			wireframe = false;
		 }

		 if ( wireframe && !team ) {
			OverrideMaterial( false, 1, true, g_Vars.esp.wireframe_chams_color_e );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );

			wireframe = false;
		 }

		 if ( wireframe && team ) {
			OverrideMaterial( false, 1, true, g_Vars.esp.wireframe_chams_color_t );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );

			wireframe = false;
		 }

		 if ( g_Vars.esp.second_color_l && is_local ) {
			OverrideMaterial( false, ChamsMaterials::MATERIAL_GLOW_, false, g_Vars.esp.second_chams_color_l, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_shine_l ) * 0.2f, 1.f, 20.f ) );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 }

		 if ( g_Vars.esp.second_color_e && !team ) {
			OverrideMaterial( false, ChamsMaterials::MATERIAL_GLOW_, false, g_Vars.esp.second_chams_color_e, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_shine_e ) * 0.2f, 1.f, 20.f ) );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 }

		 if ( g_Vars.esp.second_color_t && team ) {
			OverrideMaterial( false, ChamsMaterials::MATERIAL_GLOW_, false, g_Vars.esp.second_chams_color_t, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_shine_t ) * 0.2f, 1.f, 20.f ) );
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 }

		 Source::m_pStudioRender->m_pForcedMaterial = nullptr;
		 Source::m_pStudioRender->m_nForcedMaterialType = 0;
		 return;
	  }

   end:
	  if ( !Source::m_pEngine->IsInGame( ) || !Source::m_pEngine->IsConnected( ) || !local )
		 return;
	  else {
		 Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		 return;
	  }

   end_func:
	  Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
	  return;
   #endif 
   }
}

#if 0
void C_Chams::Main( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pCustomBoneToWorld ) {
   if ( !g_Vars.esp.chams_enabled )
	  return;

   using Fn = void* ( __thiscall* )( void*, IMatRenderContext*, DrawModelState_t&, ModelRenderInfo_t&, matrix3x4_t* );
   auto DrawModelExecute = Source::m_pModelRenderSwap->VCall<Fn>( Index::ModelDraw::DrawModelExecute );

   static bool run_once = false;
   if ( !run_once ) {
	  if ( CreateMaterials( ) )
		 run_once = true;
   }

   if ( !run_once ) {
	  return;
   }

   //remove_sleeves
   if ( strstr( RenderInfo.pModel->name, XorStr( "models/player" ) ) != nullptr ) {
	  C_CSPlayer* pPlayer = C_CSPlayer::GetPlayerByIndex( RenderInfo.entity_index );
	  auto pLocal = C_CSPlayer::GetLocalPlayer( );

	  if ( pPlayer && !pPlayer->IsDead( ) ) {
		 auto bIsTeammate = pLocal->IsTeammate( pPlayer );
		 auto mat_idx = bIsTeammate ? g_Vars.esp.team_chams_mat : g_Vars.esp.enemy_chams_mat;
		 if ( !mat_idx )
			return;

		 if ( ( !bIsTeammate && !g_Vars.esp.enemy_chams_xqz && !g_Vars.esp.enemy_chams_vis )
			  || ( bIsTeammate && !g_Vars.esp.team_chams_xqz && !g_Vars.esp.team_chams_vis ) )
			return;

		 auto vis = bIsTeammate ? g_Vars.esp.team_chams_vis : g_Vars.esp.enemy_chams_vis;
		 auto xqz = bIsTeammate ? g_Vars.esp.team_chams_xqz : g_Vars.esp.enemy_chams_xqz;

		 auto vis_clr = bIsTeammate ? g_Vars.esp.team_chams_color_vis : g_Vars.esp.enemy_chams_color_vis;
		 auto xqz_clr = bIsTeammate ? g_Vars.esp.team_chams_color_xqz : g_Vars.esp.enemy_chams_color_xqz;
		 auto mat = mat_idx == 1 ? m_matFlat : m_matTexture;

		 Source::m_pModelRender->ForcedMaterialOverride( mat );

		 if ( xqz ) {
			mat->ColorModulate( xqz_clr.r, xqz_clr.g, xqz_clr.b );
			mat->AlphaModulate( xqz_clr.a );
			mat->SetMaterialVarFlag( MATERIAL_VAR_TRANSLUCENT, xqz_clr.a < 1.0f );
			mat->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, true );

			DrawModelExecute( Source::m_pModelRender, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

			mat->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, false );
		 }

		 // TODO: fix xqz translucent overlapping  ( one of solutions - use stencil buffer )
		 if ( vis ) {
			mat->ColorModulate( vis_clr.r, vis_clr.g, vis_clr.b );
			mat->AlphaModulate( vis_clr.a );
			mat->SetMaterialVarFlag( MATERIAL_VAR_TRANSLUCENT, vis_clr.a < 1.0f );
		 } else {
			Source::m_pModelRender->ForcedMaterialOverride( nullptr );
		 }
	  }
   } if ( g_Vars.esp.remove_sleeves && strstr( RenderInfo.pModel->name, XorStr( "sleeve" ) ) != nullptr ) {
	  m_matFlat->AlphaModulate( 0.0f );
	  Source::m_pModelRender->ForcedMaterialOverride( m_matFlat );
   } else if ( g_Vars.esp.hands_chams_mat && strstr( RenderInfo.pModel->name, XorStr( "arms" ) ) != nullptr ) {
	  auto mat = g_Vars.esp.hands_chams_mat == 1 ? m_matFlat : m_matTexture;

	  mat->ColorModulate( g_Vars.esp.hands_chams.r, g_Vars.esp.hands_chams.g, g_Vars.esp.hands_chams.b );
	  mat->AlphaModulate( g_Vars.esp.hands_chams.a );
	  mat->SetMaterialVarFlag( MATERIAL_VAR_TRANSLUCENT, g_Vars.esp.hands_chams.a < 1.0f );
	  Source::m_pModelRender->ForcedMaterialOverride( mat );
   } else if ( g_Vars.esp.weapon_chams_mat && strstr( RenderInfo.pModel->name, XorStr( "weapon" ) ) != nullptr ) {
	  auto mat = g_Vars.esp.weapon_chams_mat == 1 ? m_matFlat : m_matTexture;

	  mat->ColorModulate( g_Vars.esp.weapon_chams.r, g_Vars.esp.weapon_chams.g, g_Vars.esp.weapon_chams.b );
	  mat->AlphaModulate( g_Vars.esp.weapon_chams.a );
	  mat->SetMaterialVarFlag( MATERIAL_VAR_TRANSLUCENT, g_Vars.esp.weapon_chams.a < 1.0f );
	  Source::m_pModelRender->ForcedMaterialOverride( mat );
   }
}

bool C_Chams::CreateMaterials( ) {

   std::ofstream( XorStr( "csgo\\materials\\textured_eexomi.vmt" ) ) << RXorStr( "#(" )VertexLitGeneric"
   {
	  XorStr( "$basetexture" ) XorStr( "vgui/white_additive" )
		 XorStr( "$ignorez" )      XorStr( "0" )
		 XorStr( "$envmap" )       ""
		 XorStr( "$nofog" )        XorStr( "1" )
		 XorStr( "$model" )        XorStr( "1" )
		 XorStr( "$nocull" )       XorStr( "0" )
		 XorStr( "$selfillum" )    XorStr( "1" )
		 XorStr( "$halflambert" )  XorStr( "1" )
		 XorStr( "$znearer" )      XorStr( "0" )
		 XorStr( "$flat" )         XorStr( "1" )
   }
   )#";

   std::ofstream( XorStr( "csgo\\materials\\flat_eexomi.vmt" ) ) << RXorStr( "#(" )UnlitGeneric"
   {
	  XorStr( "$basetexture" ) XorStr( "vgui/white_additive" )
		 XorStr( "$ignorez" )      XorStr( "0" )
		 XorStr( "$envmap" )       ""
		 XorStr( "$nofog" )        XorStr( "1" )
		 XorStr( "$model" )        XorStr( "1" )
		 XorStr( "$nocull" )       XorStr( "0" )
		 XorStr( "$selfillum" )    XorStr( "1" )
		 XorStr( "$halflambert" )  XorStr( "1" )
		 XorStr( "$znearer" )      XorStr( "0" )
		 XorStr( "$flat" )         XorStr( "1" )
   }
   )#";

   m_matFlat = Source::m_pMatSystem->FindMaterial( XorStr( "flat_eexomi" ), TEXTURE_GROUP_MODEL );

   if ( !m_matFlat || m_matFlat->IsErrorMaterial( ) )
	  return false;

   m_matTexture = Source::m_pMatSystem->FindMaterial( XorStr( "textured_eexomi" ), TEXTURE_GROUP_MODEL );

   if ( !m_matTexture || m_matTexture->IsErrorMaterial( ) )
	  return false;

   return true;
}
#endif

