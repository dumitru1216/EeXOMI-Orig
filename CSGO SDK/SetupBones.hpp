#pragma once
#include "sdk.hpp"
#include "player.hpp"

namespace Engine
{
   class C_SetupBones {
   public:
	  // if update anims true - calculate new vecBones and quatBones
	  // vecBones & quatBones - if updating anims, will output to this, and used for calculating matrix
	  // NOTE: if needed matrix for hitreg/lag comp - use hitbox and attachment mask, it will be the most accurate
	  // for render just use used by anything mask
	  // TODO: C_CSPlayer to C_BaseAnimating, like valve doing it
	  void Setup( );
	  void BuildMatricesWithBoneMerge( const CStudioHdr* pStudioHdr, const QAngle& angles, const Vector& origin, const Vector pos[ MAXSTUDIOBONES ], const Quaternion q[ MAXSTUDIOBONES ], matrix3x4_t bonetoworld[ MAXSTUDIOBONES ], void* pParent, void* pParentCache );
	  void GetSkeleton( );

	  static void Allocate( C_CSPlayer* animating, bool ik, bool merge );

	  static void Studio_BuildMatrices( CStudioHdr* hdr, const matrix3x4_t& worldTransform, Vector* pos, Quaternion* q, int boneMask, matrix3x4_t* out, uint32_t* boneComputed );
	  static void AttachmentHelper( C_CSPlayer* animating, CStudioHdr* hdr );

	  void Init( C_CSPlayer* player, int boneMask, matrix3x4_t* boneMatrix = nullptr, float time = -1.0f );

	  matrix3x4_t* m_boneMatrix = nullptr;

	  Vector m_vecOrigin{};
	  QAngle m_angAngles{};

	  CStudioHdr* m_pHdr;

	  // bones animations
	  Vector* m_vecBones = nullptr;
	  Quaternion* m_quatBones = nullptr;
	  bool m_bShouldDoIK = false;
	  bool m_bShouldAttachment = true;
	  bool m_bShouldDispatch = true;
	  int m_boneMask = 0;

	  // animstate animations
	  float m_flPoseParameters[ 24 ];
	  float m_flWorldPoses[ 24 ];
	  int m_nAnimOverlayCount = 0;
	  C_AnimationLayer* m_animLayers = nullptr;

	  float m_flCurtime = 0.0f;

	  C_CSPlayer* m_animating = nullptr;
   };
}
