#pragma once

#include "sdk.hpp"
#include "UtlVector.hpp"

#pragma region decl_indices
namespace Index
{
   namespace IHandleEntity
   {
	  enum {
		 SetRefEHandle = 1,
		 GetRefEHandle = 2,
	  };
   }
   namespace IClientUnknown
   {
	  enum {
		 GetCollideable = 3,
		 GetClientNetworkable = 4,
		 GetClientRenderable = 5,
		 GetIClientEntity = 6,
		 GetBaseEntity = 7,
	  };
   }
   namespace ICollideable
   {
	  enum {
		 OBBMins = 1,
		 OBBMaxs = 2,
	  };
   }
   namespace IClientNetworkable
   {
	  enum {
		 GetClientClass = 2,
		 IsDormant = 9,
		 entindex = 10,
	  };
   }
   namespace IClientRenderable
   {
	  enum {
		 GetModel = 8,
		 SetupBones = 13,
		 RenderBounds = 17,
	  };
   }
   namespace IClientEntity
   {
	  enum {
		 GetAbsOrigin = 10,
		 GetAbsAngles = 11,
	  };
   }
   namespace C_BaseEntity
   {
	  enum {
		 IsPlayer = 152,
		 IsWeapon = 160,
	  };
   }
   namespace C_BaseAnimating
   {
	  enum {
		 UpdateClientSideAnimation = 218,
	  };
   }
}
#pragma endregion

class IHandleEntity {
public:
   void SetRefEHandle( const CBaseHandle& handle );
   const CBaseHandle& GetRefEHandle( ) const;
};

class IClientUnknown : public IHandleEntity {
public:
   ICollideable* GetCollideable( );
   IClientNetworkable* GetClientNetworkable( );
   IClientRenderable* GetClientRenderable( );
   IClientEntity* GetIClientEntity( );
   C_BaseEntity* GetBaseEntity( );
};

class ICollideable {
public:
   Vector& OBBMins( );
   Vector& OBBMaxs( );
};

class IClientNetworkable {
public:
   ClientClass* GetClientClass( );
   bool IsDormant( );
   int entindex( );
   void SetDestroyedOnRecreateEntities( void ); //13
   void Release( void ); // 1
   void OnPreDataChanged( int updateType ); //4
   void OnDataChanged( int updateType ); //5
   void PreDataUpdate( int updateType ); //6
   void PostDataUpdate( int updateType ); //7
};

class IClientRenderable {
public:
   virtual IClientUnknown* GetIClientUnknown( ) = 0;
   const model_t* GetModel( );
   bool SetupBones( matrix3x4_t* pBoneToWorld, int nMaxBones, int boneMask, float currentTime );
   void GetRenderBounds( Vector& mins, Vector& maxs );
};

class CCollisionProperty : public ICollideable {
public:
   void* vtable;
   C_BaseEntity* m_pEntity;
   Vector m_vecMins;
   Vector m_vecMaxs;
   unsigned short m_usSolidFlags;
   unsigned short m_nSolidType;
   float m_flRadius;

   unsigned short m_Partition; // SpatialPartitionHandle_t
   unsigned char m_nSurroundType;
   unsigned char m_triggerBloat;

   Vector m_vecSpecifiedSurroundingMins;
   Vector m_vecSpecifiedSurroundingMaxs;
   Vector m_vecSurroundingMins;
   Vector m_vecSurroundingMaxs;

   void SetCollisionBounds( const Vector& mins, const Vector& maxs );
};

class IClientEntity : public IClientUnknown {
public:
   horizon_pad( 0x64 );
   int m_entIndex;

   Vector& OBBMins( );
   Vector& OBBMaxs( );

   Vector& GetAbsOrigin( );
   QAngle& GetAbsAngles( );

   ClientClass* GetClientClass( );
   bool IsDormant( );
   int entindex( );

   const model_t* GetModel( );
   bool SetupBones( matrix3x4_t* pBoneToWorld, int nMaxBones, int boneMask, float currentTime );
};

class CBoneAccessor {
public:
   inline matrix3x4_t* GetBoneArrayForWrite( ) { return m_pBones; }

   inline void SetBoneArrayForWrite( matrix3x4_t* bone_array ) { m_pBones = bone_array; }

   inline int GetReadableBones( ) {
	  return m_ReadableBones;
   }

   inline void SetReadableBones( int flags ) {
	  m_ReadableBones = flags;
   }

   inline int GetWritableBones( ) {
	  return m_WritableBones;
   }

   inline void SetWritableBones( int flags ) {
	  m_WritableBones = flags;
   }

   alignas( 16 ) matrix3x4_t* m_pBones;
   int m_ReadableBones; // Which bones can be read.
   int m_WritableBones; // Which bones can be written.
};

class CIKContext {
public:
   void Construct( );
   void Destructor( );

   void ClearTargets( );
   void Init( CStudioHdr* hdr, QAngle* angles, Vector* origin, float currentTime, int frames, int boneMask );
   void UpdateTargets( Vector* pos, Quaternion* qua, matrix3x4_t* matrix, uint8_t* boneComputed );
   void SolveDependencies( Vector* pos, Quaternion* qua, matrix3x4_t* matrix, uint8_t* boneComputed );
};

class C_AnimationLayer {
public:
	bool m_bClientBlend;				 //0x0000
	float m_flBlendIn;					 //0x0004
	void* m_pStudioHdr;					 //0x0008
	int m_nDispatchSequence;     //0x000C
	int m_nDispatchSequence_2;   //0x0010
	uint32_t m_nOrder;           //0x0014
	uint32_t m_nSequence;        //0x0018
	float_t m_flPrevCycle;       //0x001C
	float_t m_flWeight;          //0x0020
	float_t m_flWeightDeltaRate; //0x0024
	float_t m_flPlaybackRate;    //0x0028
	float_t m_flCycle;           //0x002C
	void* m_pOwner;              //0x0030 // player's thisptr
	char pad_0038[ 4 ];            //0x0034
};                    //Size: 0x0038

class C_BaseEntity : public IClientEntity {
public:
   datamap_t* GetDataDescMap( ) {
	  typedef datamap_t* ( __thiscall * o_GetPredDescMap )( void* );
	  return Memory::VCall<o_GetPredDescMap>( this, 15 )( this );
   }

   datamap_t* GetPredDescMap( ) {
	  typedef datamap_t* ( __thiscall * o_GetPredDescMap )( void* );
	  return Memory::VCall<o_GetPredDescMap>( this, 17 )( this );
   }
   
   void SetModelIndex( const int index ) {
	  using Fn = void( __thiscall* )( C_BaseEntity*, int );
	  return Memory::VCall<Fn>( this, 75 )( this, index );
   }

public:
   bool IsPlayer( );
   bool IsWeapon( );
   bool IsPlantedC4( );

   void SetAbsVelocity( const Vector& velocity );
   void SetAbsOrigin( const Vector& origin );
   void InvalidatePhysicsRecursive( int change_flags );
   void SetAbsAngles( const QAngle& angles );

   std::uint8_t& m_MoveType( );
   matrix3x4_t& m_rgflCoordinateFrame( );

   CCollisionProperty* m_Collision( );

   int& m_fEffects( );
   int& m_iEFlags( );
   int& m_iTeamNum( );

   Vector& m_vecOrigin( );
   void UpdateVisibilityAllEntities( );

   float& m_flSimulationTime( );
   float& m_flOldSimulationTime( );
   float m_flAnimationTime( );
public:
   static void SetPredictionRandomSeed( const CUserCmd* cmd );
   static void SetPredictionPlayer( C_BasePlayer* player );
   CBaseHandle& m_hOwnerEntity( );
   int& m_nModelIndex( );

};

class C_PlantedC4 : public C_BaseEntity {
public:
   float& m_flC4Blow( );
};

class C_BaseAnimating : public C_BaseEntity {
public:
   void UpdateClientSideAnimation( );
   void FixedAnimationsUpdate( );
   void InvalidateBoneCache( );
   void LockStudioHdr( );
   bool ComputeHitboxSurroundingBox( Vector& VecWorldMins, Vector& VecWorldMaxs, const matrix3x4_t* boneTransform = nullptr );
   int GetSequenceActivity( int sequence );

public:
   int& m_nHitboxSet( );
   int& m_iMostRecentModelBoneCounter( );
   bool& m_bClientSideAnimation( );
   bool& m_bShouldDraw( );
   float& m_flLastBoneSetupTime( );
   float* m_flPoseParameter( );

public:
   CBoneAccessor& m_BoneAccessor( );
   CUtlVector<matrix3x4_t>& m_CachedBoneData( );
   CUtlVector<C_AnimationLayer>& m_AnimOverlay( );

   Vector* m_vecBonePos( );
   Quaternion* m_quatBoneRot( );

   CStudioHdr* m_pStudioHdr( );

};

class C_BaseCombatCharacter : public C_BaseAnimating {
public:
   CBaseHandle& m_hActiveWeapon( );
   float& m_flNextAttack( );

   CBaseHandle* m_hMyWeapons( );
   CBaseHandle* m_hMyWearables( );
};

class C_SmokeGrenadeProjectile : public C_BaseEntity {
public:
   int m_nSmokeEffectTickBegin( );

   static float GetExpiryTime( ) {
	  return 19.f;
   }
};

class C_Inferno : public C_BaseEntity {
public:
   float m_flSpawnTime( ); // 0x20

   static float GetExpiryTime( ) {
	  return 7.f;
   }
};
