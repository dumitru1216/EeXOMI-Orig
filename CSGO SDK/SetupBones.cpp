#include "SetupBones.hpp"
#include "source.hpp"
#include "player.hpp"
#include "displacement.hpp"
#include "weapon.hpp"
#include "utils/mutex.h"
#include <mutex>

#pragma runtime_checks( "", off )
struct CBoneSetup {
   const CStudioHdr* m_pStudioHdr;
   int m_boneMask;
   const float* m_flPoseParameter;
   void* m_pPoseDebugger;

   void InitPose( Vector pos[], Quaternion q[], CStudioHdr* hdr ) {
	  __asm
	  {
		 mov eax, this
		 mov esi, q
		 mov edx, pos
		 push dword ptr[ hdr + 4 ]
		 mov ecx, [ eax ]
		 push esi
		 call Engine::Displacement.CBoneSetup.InitPose
		 add esp, 8
	  }
   }

   void AccumulatePose( Vector pos[], Quaternion q[], int sequence, float cycle, float flWeight, float flTime, CIKContext* pIKContext ) {
	  using AccumulatePoseFn = void( __thiscall* )( CBoneSetup*, Vector * a2, Quaternion * a3, int a4, float a5, float a6, float a7, CIKContext * a8 );
	  auto Accumulate_Pose = ( AccumulatePoseFn ) Engine::Displacement.CBoneSetup.AccumulatePose;
	  return Accumulate_Pose( this, pos, q, sequence, cycle, flWeight, flTime, pIKContext );
   }

   void CalcAutoplaySequences( Vector pos[], Quaternion q[], float flRealTime, CIKContext* pIKContext ) {
	  __asm
	  {
		 movss   xmm3, flRealTime
		 mov eax, pIKContext
		 mov ecx, this
		 push eax
		 push q
		 push pos
		 call Engine::Displacement.CBoneSetup.CalcAutoplaySequences
	  }
   }

   void CalcBoneAdj( Vector pos[], Quaternion q[], float* controllers, int boneMask ) {
	  __asm
	  {
		 mov     eax, controllers
		 mov     ecx, this
		 mov     edx, pos; a2
		 push    dword ptr[ ecx + 4 ]; a5
		 mov     ecx, [ ecx ]; a1
		 push    eax; a4
		 push    q; a3
		 call    Engine::Displacement.CBoneSetup.CalcBoneAdj
		 add     esp, 0xC
	  }
   }
};
#pragma runtime_checks( "", restore )

uintptr_t& GetBoneMerge( C_CSPlayer* player ) {
   return *( uintptr_t* ) ( ( uintptr_t ) player + Engine::Displacement.C_BaseAnimating.m_pBoneMerge );
}

struct mstudioposeparamdesc_t {
   int sznameindex;
   inline char* const pszName( void ) const { return ( ( char* ) this ) + sznameindex; }
   int flags;   // ???? ( volvo, really? )
   float start; // starting value
   float end;   // ending value
   float loop;  // looping range, 0 for no looping, 360 for rotations, etc.
};

mstudioposeparamdesc_t* pPoseParameter( CStudioHdr* hdr, int index ) {
   using poseParametorFN = mstudioposeparamdesc_t * ( __thiscall* )( CStudioHdr*, int );
   poseParametorFN p_pose_parameter = ( poseParametorFN ) Engine::Displacement.Function.m_pPoseParameter;
   return p_pose_parameter( hdr, index );
}

void UpdateCache( uintptr_t bonemerge ) {
   static auto BoneMergeUpdateCache = ( void( __thiscall* )( uintptr_t ) )Engine::Displacement.CBoneMergeCache.m_nUpdateCache;
   BoneMergeUpdateCache( bonemerge );
}

float GetPoseParamValue( CStudioHdr* hdr, int index, float flValue ) {
   if ( index < 0 || index > 24 )
	  return 0.0f;

   auto pose_param = pPoseParameter( hdr, index );
   if ( !pose_param )
	  return 0.0f;

   auto PoseParam = *pose_param;
   if ( PoseParam.loop ) {
	  float wrap = ( PoseParam.start + PoseParam.end ) / 2.0f + PoseParam.loop / 2.0f;
	  float shift = PoseParam.loop - wrap;

	  flValue = flValue - PoseParam.loop * std::floorf( ( flValue + shift ) / PoseParam.loop );
   }

   auto ctlValue = ( flValue - PoseParam.start ) / ( PoseParam.end - PoseParam.start );
   return ctlValue;
}

void MergeMatchingPoseParams( uintptr_t bonemerge, float* poses, float* target_poses ) {
   UpdateCache( bonemerge );

   if ( *( std::uintptr_t* )( bonemerge + 0x10 ) && *( std::uintptr_t* )( bonemerge + 0x8C ) ) {
	  auto index = ( int* ) ( bonemerge + 0x20 );
	  int  v4 = 0;
	  do {
		 if ( *index != -1 ) {
			auto target = *( C_CSPlayer * * ) ( bonemerge + 0x4 );
			auto hdr = target->m_pStudioHdr( );
			//if ( !hdr ) {
			//   target->LockStudioHdr( );
			//   hdr = target->m_pStudioHdr( );
			// }

			float pose_param_value = 0.0f;
			if ( hdr && *( studiohdr_t * * ) hdr && v4 >= 0 ) {
			   // C_BaseAnimating::GetPoseParameter
			   float pose = target_poses[ v4 ];
			   mstudioposeparamdesc_t* pPoseParam = pPoseParameter( hdr, v4 );
			   pose_param_value = pose * ( pPoseParam->end - pPoseParam->start ) + pPoseParam->start;
			}

			auto target2 = *( C_CSPlayer * * ) ( bonemerge );
			auto hdr2 = target2->m_pStudioHdr( );
			//if ( !hdr2 ) {
			 //  hdr2 = target2->m_pStudioHdr( );
			//}

			// Studio_SetPoseParameter
			poses[ *index ] = GetPoseParamValue( hdr2, *index, pose_param_value );
		 }
		 ++v4;
		 ++index;
	  } while ( v4 < 24 );
   }
}
// E8 ? ? ? ? 83 7E 10 00 74 64 

// std::map <int, Mutex> bone_merge_lock;



namespace Engine
{
   // server.dll E8 ? ? ? ? 68 ? ? ? ? 8B CF E8 ? ? ? ? 8B 8F ? ? ? ? rebuild
   void C_SetupBones::Setup( ) {
	  // ReevaluateAnimLOD
	  // 53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 30 56 57

	  CIKContext* m_pIk = *( CIKContext * * ) ( uintptr_t( this->m_animating ) + 0x2570 );
	  if ( !m_bShouldDoIK ) {
		 m_pIk = nullptr;
	  }

	  m_pHdr = this->m_animating->m_pStudioHdr( );

	  // 256 bits
	  uint32_t boneComputed[ 8 ] = { 0 };

	  auto SequencesAvailable = ( !*( int* ) ( ( *( uintptr_t* ) this->m_pHdr ) + 0x150 ) || *( int* ) ( uintptr_t( this->m_pHdr ) + 0x4 ) );
	  //TODO: Add sv_pvsskipanimation checks ( Skips SetupBones when npc's are outside the PVS )
	  //https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/sp/src/game/server/baseanimating.cpp#L1802

	  if ( m_pIk ) {
		 m_pIk->Init( this->m_pHdr, &this->m_angAngles, &this->m_vecOrigin, m_flCurtime, TIME_TO_TICKS( m_flCurtime ), this->m_boneMask );

		 if ( SequencesAvailable )
			GetSkeleton( );

		//typedef void( __thiscall* oUpdateIKLocks )( void*, float );
		//Memory::VCall< oUpdateIKLocks >( this->m_animating, 192 )( this->m_animating, m_flCurtime );
		//
		 m_pIk->UpdateTargets( this->m_vecBones, this->m_quatBones, this->m_boneMatrix, ( uint8_t* ) boneComputed ); // this offset is right

		// typedef void( __thiscall * oCalculateIKLocks )( void*, float );
		// Memory::VCall< oCalculateIKLocks >( this->m_animating, 193 )( this->m_animating, m_flCurtime );
		 
		 m_pIk->SolveDependencies( this->m_vecBones, this->m_quatBones, this->m_boneMatrix, ( uint8_t* ) boneComputed );
	  } else if ( SequencesAvailable ) {
		 GetSkeleton( );
	  }

	  //typedef void( __thiscall* oBuildTransformations )( void*, CStudioHdr*, Vector*, Quaternion*, const matrix3x4_t&, int32_t, byte* );
	  //Memory::VCall< oBuildTransformations >( this->m_animating, 189 )( this->m_animating, this->m_pHdr, this->m_vecBones, this->m_quatBones, *this->m_boneMatrix, this->m_boneMask, ( uint8_t* ) boneComputed );
	 
	  matrix3x4_t transform;
	  transform.AngleMatrix( this->m_angAngles, this->m_vecOrigin );

	  //missing parts with BuildMatricesWithBoneMerge
	  //https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/sp/src/game/server/baseanimating.cpp#L1836

	  Studio_BuildMatrices( this->m_pHdr, transform, this->m_vecBones, this->m_quatBones, this->m_boneMask, this->m_boneMatrix, boneComputed );

	  if ( this->m_boneMask & BONE_USED_BY_ATTACHMENT && m_bShouldAttachment )
		 AttachmentHelper( this->m_animating, this->m_pHdr );

	  this->m_animating->m_flLastBoneSetupTime( ) = m_flCurtime;
	  this->m_animating->m_BoneAccessor( ).m_ReadableBones |= this->m_boneMask;
	  this->m_animating->m_BoneAccessor( ).m_WritableBones |= this->m_boneMask;
	  this->m_animating->m_iMostRecentModelBoneCounter( ) = *( int* ) Engine::Displacement.Data.m_uModelBoneCounter;
   }

   bool CanBeAnimated( C_CSPlayer* player ) {
	  if ( !*( bool* ) ( uintptr_t( player ) + 0x39E1 ) || !player->m_PlayerAnimState( ) )
		 return false;

	  auto weapon = ( C_BaseAttributableItem* ) player->m_hActiveWeapon( ).Get( );
	  if ( !weapon )
		 return false;

	  auto weaponWorldModel = ( C_CSPlayer* ) weapon->m_hWeaponWorldModel( ).Get( );
	  if ( !weaponWorldModel || *( short* ) ( uintptr_t( weaponWorldModel ) + 0x26E ) == -1 )
		 return C_CSPlayer::GetLocalPlayer( ) == player;

	  return true;
   }

   void C_SetupBones::BuildMatricesWithBoneMerge(
	  const CStudioHdr* pStudioHdr,
	  const QAngle& angles,
	  const Vector& origin,
	  const Vector pos[ MAXSTUDIOBONES ],
	  const Quaternion q[ MAXSTUDIOBONES ],
	  matrix3x4_t bonetoworld[ MAXSTUDIOBONES ],
	  void* pParent,
	  void* pParentCache
   ) {

   }

   void C_SetupBones::GetSkeleton( ) {
	  alignas( 16 ) Vector pos2[ 256 ];
	  alignas( 16 ) Quaternion rot2[ 256 ];
	  // new: 55 8B EC 81 EC BC ? ? ? 53 56 57
	  // old: 55 8B EC 81 EC BC 00 00 00 53 56 57
	  static auto add_dependencies = Memory::Scan( XorStr( "client.dll" ), XorStr( "55 8B EC 81 EC BC ?? ?? ?? 53 56 57" ) ); // thats airflow sig

	  CIKContext* m_pIk = *( CIKContext * * ) ( uintptr_t( this->m_animating ) + 0x2570 );
	  if ( !m_bShouldDoIK )
		 m_pIk = nullptr;

	  // memalloc isnt thread safe, sooo
	  alignas( 16 ) char buffer[ 32 ];
	  alignas( 16 ) CBoneSetup* boneSetup = ( CBoneSetup* ) & buffer;
	  boneSetup->m_pStudioHdr = m_pHdr;
	  boneSetup->m_boneMask = this->m_boneMask;
	  boneSetup->m_flPoseParameter = this->m_flPoseParameters;
	  boneSetup->m_pPoseDebugger = nullptr;

	  boneSetup->InitPose( this->m_vecBones, this->m_quatBones, m_pHdr );
	  boneSetup->AccumulatePose( this->m_vecBones, this->m_quatBones, *( int* ) ( uintptr_t( this->m_animating ) + Engine::Displacement.DT_BaseAnimating.m_nSequence ), 
								 *( float* ) ( uintptr_t( this->m_animating ) + Engine::Displacement.DT_BaseAnimating.m_flCycle ),
		 1.0f, m_flCurtime, m_pIk );

	  int layer[ 15 ] = { this->m_nAnimOverlayCount };
	  for ( int i = 0; i < this->m_nAnimOverlayCount; i++ ) {
		 C_AnimationLayer& pLayer = this->m_animLayers[ i ];
		 if ( ( pLayer.m_flWeight > 0.f ) && ( pLayer.m_nOrder != 15 ) && pLayer.m_nOrder >= 0 && pLayer.m_nOrder < this->m_nAnimOverlayCount ) {
			layer[ pLayer.m_nOrder ] = i;
		 }
	  }

	  // static auto BoneMergeMatchingPoseParams = ( void( __thiscall* )( uintptr_t ) )rel32_fix( Memory::Scan( "client.dll", "E8 ? ? ? ? 8B 06 8D 4C 24 30 51" ) );
	  auto BoneMergeCopyToFollow = ( void( __thiscall* )( uintptr_t, Vector*, Quaternion*, int, Vector*, Quaternion* ) )Engine::Displacement.CBoneMergeCache.m_CopyToFollow;
	  auto BoneMergeCopyFromFollow = ( void( __thiscall* )( uintptr_t, Vector*, Quaternion*, int, Vector*, Quaternion* ) ) Displacement.CBoneMergeCache.m_CopyFromFollow;

	  char tmp_buffer[ 0x1070 ];
	  CIKContext* worldIk = ( CIKContext* ) tmp_buffer;

	  auto weapon = ( C_BaseAttributableItem* ) this->m_animating->m_hActiveWeapon( ).Get( );
	  if ( CanBeAnimated( this->m_animating ) && weapon ) {
		 auto weaponWorldModel = ( C_CSPlayer* ) weapon->m_hWeaponWorldModel( ).Get( );
		 if ( weaponWorldModel ) {
			auto bone_merge = GetBoneMerge( weaponWorldModel );
			if ( bone_merge ) {
			   auto world_hdr = weaponWorldModel->m_pStudioHdr( );
			   //accumulate_layers rebuild
			#if 0
			   bone_merge_lock[ this->m_animating->m_entIndex ].lock( );
			   std::memcpy( weaponWorldModel->m_flPoseParameter( ), this->m_flWorldPoses, sizeof( this->m_flWorldPoses ) );
			   BoneMergeMatchingPoseParams( bone_merge );
			   std::memcpy( this->m_flWorldPoses, weaponWorldModel->m_flPoseParameter( ), sizeof( this->m_flWorldPoses ) );
			   bone_merge_lock[ this->m_animating->m_entIndex ].unlock( );
			#else
			   MergeMatchingPoseParams( bone_merge, this->m_flWorldPoses, this->m_flPoseParameters );
			#endif

			   worldIk->Construct( );
			   worldIk->Init( world_hdr, &this->m_angAngles, &this->m_vecOrigin, m_flCurtime, 0, 0x40000 );

			   alignas( 16 ) char buffer2[ 32 ];
			   alignas( 16 ) CBoneSetup* worldSetup = ( CBoneSetup* ) & buffer2;
			   worldSetup->m_pStudioHdr = world_hdr;
			   worldSetup->m_boneMask = 0x40000;
			   worldSetup->m_flPoseParameter = this->m_flWorldPoses;
			   worldSetup->m_pPoseDebugger = nullptr;

			   worldSetup->InitPose( pos2, rot2, world_hdr );

			   for ( int i = 0; i < this->m_nAnimOverlayCount; ++i ) {
				  auto animLayer = &this->m_animLayers[ i ];
				  if ( animLayer && animLayer->m_nSequence > 1 && animLayer->m_flWeight > 0.0f ) {
					 if ( this->m_bShouldDispatch && this->m_animating == C_CSPlayer::GetLocalPlayer( ) ) {
						using UpdateDispatchLayer = void( __thiscall* )( void*, C_AnimationLayer*, CStudioHdr*, int );
						Memory::VCall< UpdateDispatchLayer >( this->m_animating, 241 )( this->m_animating, animLayer, world_hdr, animLayer->m_nSequence );
					 }

					 if ( !this->m_bShouldDispatch || animLayer->m_nDispatchSequence_2 <= 0 || animLayer->m_nDispatchSequence_2 >= ( *( studiohdr_t * * ) world_hdr )->numlocalseq ) {
						boneSetup->AccumulatePose( this->m_vecBones, this->m_quatBones, animLayer->m_nSequence, animLayer->m_flCycle, animLayer->m_flWeight, m_flCurtime, m_pIk );
					 } else if ( this->m_bShouldDispatch ) {
						// TODO: ik add dependecies code 55 8B EC 81 EC BC 00 00 00 53 56 57
						// E8 ? ? ? ? 8B 45 20 F3 0F 10 45 ? 
						typedef void( __thiscall* oIKAddDependencies )( CIKContext*, float, int, int, float, float ); // c_ik_context*, float, int, int, const float[], float

						BoneMergeCopyFromFollow( bone_merge, this->m_vecBones, this->m_quatBones, 0x40000, pos2, rot2 );
						if ( m_pIk ) {
							//if ( *( float* )( uintptr_t( this->m_animating ) + 0xA14 ) ) {
							//	std::cout << "valid\n";
							//}
							//
							//if ( animLayer->m_nSequence ) {
							//	std::cout << "valid1\n";
							//}
							//
							//if ( animLayer->m_flCycle ) {
							//	std::cout << "valid2\n";
							//}
							//
							//if ( boneSetup->m_flPoseParameter ) {
							//	std::cout << "valid3\n";
							//}
							//
							//if ( animLayer->m_flWeight ) {
							//	std::cout << "valid4\n";
							//}

							// oh get the fuck out u fucking piss of shit function, stop crashing fucking retarded cunt source u fucking retard
						   // oIKAddDependencies( add_dependencies ) ( m_pIk, *( float* ) ( uintptr_t( this->m_animating ) + 0xA14 ), animLayer->m_nSequence, 
							//										  animLayer->m_flCycle, boneSetup->m_flPoseParameter[ 2 ], animLayer->m_flWeight ) );
						}
						worldSetup->AccumulatePose( pos2, rot2, animLayer->m_nDispatchSequence_2, animLayer->m_flCycle, animLayer->m_flWeight, m_flCurtime, worldIk );
						BoneMergeCopyToFollow( bone_merge, pos2, rot2, 0x40000, this->m_vecBones, this->m_quatBones );

					 #if 0 //TODO: Do it this 
						if ( m_bShouldDoIK && *( CIKContext * * ) ( uintptr_t( this->m_animating ) + Engine::Displacement.C_BaseAnimating.m_pIk ) && *( int* ) ( bone_merge + 160 ) ) {
						   //bone_merge_lock[ this->m_animating->m_entIndex ].lock( );
						   worldIk->CopyTo( *( CIKContext * * ) ( uintptr_t( this->m_animating ) + Engine::Displacement.C_BaseAnimating.m_pIk ), *( int* ) ( bone_merge + 160 ) );
						   //bone_merge_lock[ this->m_animating->m_entIndex ].unlock( );
						}
					 #endif
					 }
				  }
			   }

			   worldIk->Destructor( );
			}
		 }
	  } else {
		 for ( int i = 0; i < this->m_nAnimOverlayCount; ++i ) {
			auto v46 = layer[ i ];
			if ( v46 >= 0 && v46 < this->m_nAnimOverlayCount ) {
			   auto debilLayer = &this->m_animLayers[ i ];
			   boneSetup->AccumulatePose( pos2, rot2, debilLayer->m_nSequence, debilLayer->m_flCycle, debilLayer->m_flWeight, m_flCurtime, m_pIk );
			}
		 }
	  }

	  if ( m_pIk ) {
		 worldIk->Construct( );
		 worldIk->Init( this->m_pHdr, &this->m_angAngles, &this->m_vecOrigin, m_flCurtime, 0, this->m_boneMask );
		 boneSetup->CalcAutoplaySequences( this->m_vecBones, this->m_quatBones, m_flCurtime, worldIk );
		 worldIk->Destructor( );
	  } else {
		 boneSetup->CalcAutoplaySequences( this->m_vecBones, this->m_quatBones, m_flCurtime, nullptr );
	  }

	  boneSetup->CalcBoneAdj( this->m_vecBones, this->m_quatBones, ( float* ) ( uintptr_t( this->m_animating ) + 0xA54 ), this->m_boneMask );
   }

   void C_SetupBones::Allocate( C_CSPlayer* animating, bool ik, bool merge ) {
	  CIKContext* m_pIk = *( CIKContext * * ) ( uintptr_t( animating ) + Engine::Displacement.C_BaseAnimating.m_pIk );
	  if ( !m_pIk && ik ) {
		 m_pIk = ( CIKContext* ) Source::m_pMemAlloc->Alloc( 0x1070 );
		 m_pIk->Construct( );

		 *( CIKContext * * ) ( uintptr_t( animating ) + Engine::Displacement.C_BaseAnimating.m_pIk ) = m_pIk;
	  }

	  if ( merge ) {
		 auto BoneMergeCtor = ( void( __thiscall* )( uintptr_t ) )       Engine::Displacement.CBoneMergeCache.m_nConstructor;
		 auto BoneMergeInit = ( void( __thiscall* )( uintptr_t, void* ) )Engine::Displacement.CBoneMergeCache.m_nInit;

		 auto weapon = ( C_BaseAttributableItem* ) animating->m_hActiveWeapon( ).Get( );
		 if ( weapon ) {
			auto weaponWorldModel = ( C_CSPlayer* ) weapon->m_hWeaponWorldModel( ).Get( );
			if ( weaponWorldModel ) {
			   uintptr_t& bone_merge = GetBoneMerge( weaponWorldModel );
			   if ( !bone_merge ) {
				  bone_merge = ( uintptr_t ) Source::m_pMemAlloc->Alloc( 676 );
				  BoneMergeCtor( bone_merge );
				  BoneMergeInit( bone_merge, weaponWorldModel );

				  GetBoneMerge( weaponWorldModel ) = bone_merge;
			   }

			   if ( bone_merge )
				  UpdateCache( bone_merge );
			}
		 }
	  }
   }
   // server E8 ? ? ? ? A1 ? ? ? ? 83 C4 1C B9 ? ? ? ? 
   void C_SetupBones::Studio_BuildMatrices( CStudioHdr* hdr, const matrix3x4_t& rotationmatrix, Vector* pos, Quaternion* q, int boneMask, matrix3x4_t* bonetoworld, uint32_t* boneComputed ) {
	  int i, j;

	  int					chain[ MAXSTUDIOBONES ] = {};
	  int					chainlength = 0;

	  // just stub
	  int iBone = -1;

	  studiohdr_t* pStudioHdr = *( studiohdr_t * * ) hdr;

	  if ( iBone < -1 || iBone >= pStudioHdr->numbones )
		 iBone = 0;

	  auto v10 = ( uintptr_t ) hdr;

	  auto boneFlags = ( CUtlVector<int>* ) ( v10 + 0x30 );
	  auto boneParent = ( CUtlVector<int>* ) ( v10 + 0x44 );

	  // build list of what bones to use
	  if ( iBone <= -1 ) {
		 // all bones
		 chainlength = pStudioHdr->numbones;
		 for ( i = 0; i < pStudioHdr->numbones; i++ ) {
			chain[ chainlength - i - 1 ] = i;
		 }

	  } else {
		 // only the parent bones
		 i = iBone;

		 do {
			chain[ chainlength++ ] = i;
			i = boneParent->m_Memory.Element( i );
		 } while ( i != -1 );
	  }

	  // missing some parts with 0xF flag

	  matrix3x4_t bonematrix;
	  for ( j = chainlength - 1; j >= 0; j-- ) {
		 i = chain[ j ];

		 if ( ( ( 1 << ( i & 0x1F ) ) & boneComputed[ i >> 5 ] ) )
			continue;

		 int flag = boneFlags->m_Memory.Element( i );
		 if ( flag & boneMask && q ) {
			bonematrix.QuaternionMatrix( q[ i ], pos[ i ] );

			auto parent = boneParent->m_Memory.Element( i );
			if ( parent == -1 ) {
			   bonetoworld[ i ] = rotationmatrix.ConcatTransforms( bonematrix );
			} else {
			   bonetoworld[ i ] = bonetoworld[ parent ].ConcatTransforms( bonematrix );
			}
		 }
	  }
   }

   void C_SetupBones::AttachmentHelper( C_CSPlayer* animating, CStudioHdr* hdr ) {
	  using AttachmentHelperFn = void( __thiscall* )( C_CSPlayer*, CStudioHdr* );
	  static AttachmentHelperFn attachment = ( AttachmentHelperFn ) Engine::Displacement.Function.m_AttachmentHelper;
	  attachment( animating, hdr );
   }

   void C_SetupBones::Init( C_CSPlayer* player, int boneMask, matrix3x4_t* boneMatrix, float time ) {
	  this->m_vecBones = player->m_vecBonePos( );
	  this->m_quatBones = player->m_quatBoneRot( );
	  this->m_boneMask = boneMask;
	  this->m_animLayers = player->m_AnimOverlay( ).Base( );
	  this->m_nAnimOverlayCount = player->m_AnimOverlay( ).Count( );
	  this->m_animating = player;
	  this->m_bShouldDoIK = true;
	  this->m_vecOrigin = player->m_vecOrigin( );
	  this->m_angAngles = player->GetAbsAngles( );
	  this->m_bShouldAttachment = true;
	  std::memcpy( this->m_flPoseParameters, player->m_flPoseParameter( ), sizeof( this->m_flPoseParameters ) );

	  auto weapon = ( C_BaseAttributableItem* ) this->m_animating->m_hActiveWeapon( ).Get( );
	  if ( weapon ) {
		 auto weaponWorldModel = ( C_CSPlayer* ) weapon->m_hWeaponWorldModel( ).Get( );
		 if ( weaponWorldModel ) {
			std::memcpy( this->m_flWorldPoses, weaponWorldModel->m_flPoseParameter( ), sizeof( this->m_flWorldPoses ) );
		 } else {
			std::memcpy( this->m_flWorldPoses, player->m_flPoseParameter( ), sizeof( this->m_flWorldPoses ) );
		 }
	  } else {
		 std::memcpy( this->m_flWorldPoses, player->m_flPoseParameter( ), sizeof( this->m_flWorldPoses ) );
	  }

	  if ( time == -1.0f )
		 time = player->m_flSimulationTime( );

	//  if ( !player->m_pStudioHdr( ) )
	//	 player->LockStudioHdr( );

	  this->m_pHdr = player->m_pStudioHdr( );

	  this->m_flCurtime = time;

	  if ( !boneMatrix )
		 this->m_boneMatrix = player->m_BoneAccessor( ).m_pBones;
	  else
		 this->m_boneMatrix = boneMatrix;

	  Allocate( player, true, true );
   }
}
