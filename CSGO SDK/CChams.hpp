#pragma once
#include "sdk.hpp"

namespace Source
{

   class __declspec( novtable ) IChams : public NonCopyable {
   public:
	  static Encrypted_t<IChams> Get( );
	  virtual void OnDrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pCustomBoneToWorld ) = 0;
	  virtual void DrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pCustomBoneToWorld ) = 0;
	  virtual bool CreateMaterials( ) = 0;
	  virtual void OnPostScreenEffects( ) = 0;
	  virtual void AddHitmatrix( C_CSPlayer* player, matrix3x4_t* bones ) = 0;
   protected:
	  IChams( ) { };
	  virtual ~IChams( ) { };
   };

}
