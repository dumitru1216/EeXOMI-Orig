#include "entity.hpp"
#include "displacement.hpp"
#include "sdk.hpp"
#include "CBaseHandle.hpp"

void IHandleEntity::SetRefEHandle( const CBaseHandle& handle ) {
   using Fn = void( __thiscall* )( void*, const CBaseHandle& );
   return Memory::VCall<Fn>( this, Index::IHandleEntity::SetRefEHandle )( this, handle );
}

const CBaseHandle& IHandleEntity::GetRefEHandle( ) const {
   using Fn = const CBaseHandle& ( __thiscall* )( const IHandleEntity* );
   return Memory::VCall<Fn>( this, Index::IHandleEntity::GetRefEHandle )( this );
}

ICollideable* IClientUnknown::GetCollideable( ) {
   using Fn = ICollideable * ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientUnknown::GetCollideable )( this );
}

IClientNetworkable* IClientUnknown::GetClientNetworkable( ) {
   using Fn = IClientNetworkable * ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientUnknown::GetClientNetworkable )( this );
}

IClientRenderable* IClientUnknown::GetClientRenderable( ) {
   using Fn = IClientRenderable * ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientUnknown::GetClientRenderable )( this );
}

IClientEntity* IClientUnknown::GetIClientEntity( ) {
   using Fn = IClientEntity * ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientUnknown::GetIClientEntity )( this );
}

C_BaseEntity* IClientUnknown::GetBaseEntity( ) {
   using Fn = C_BaseEntity * ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientUnknown::GetBaseEntity )( this );
}

Vector& ICollideable::OBBMins( ) {
   using Fn = Vector & ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::ICollideable::OBBMins )( this );
}

Vector& ICollideable::OBBMaxs( ) {
   using Fn = Vector & ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::ICollideable::OBBMaxs )( this );
}

ClientClass* IClientNetworkable::GetClientClass( ) {
   using Fn = ClientClass * ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientNetworkable::GetClientClass )( this );
}

bool IClientNetworkable::IsDormant( ) {
   using Fn = bool( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientNetworkable::IsDormant )( this );
}

int IClientNetworkable::entindex( ) {
   using Fn = int( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientNetworkable::entindex )( this );
}

void IClientNetworkable::SetDestroyedOnRecreateEntities( void ) {
   using Fn = void( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, 13 )( this );
}

void IClientNetworkable::Release( void ) {
   using Fn = void( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, 1 )( this );
}

void IClientNetworkable::OnPreDataChanged( int updateType ) {
   using Fn = void( __thiscall* )( void*, int );
   return Memory::VCall<Fn>( this, 4 )( this, updateType );
}

void IClientNetworkable::OnDataChanged( int updateType ) {
   using Fn = void( __thiscall* )( void*, int );
   return Memory::VCall<Fn>( this, 5 )( this, updateType );
}

void IClientNetworkable::PreDataUpdate( int updateType ) {
   using Fn = void( __thiscall* )( void*, int );
   return Memory::VCall<Fn>( this, 6 )( this, updateType );
}

void IClientNetworkable::PostDataUpdate( int updateType ) {
   using Fn = void( __thiscall* )( void*, int );
   return Memory::VCall<Fn>( this, 7 )( this, updateType );
}

const model_t* IClientRenderable::GetModel( ) {
   using Fn = const model_t* ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientRenderable::GetModel )( this );
}

bool IClientRenderable::SetupBones( matrix3x4_t* pBoneToWorld, int nMaxBones, int boneMask, float currentTime ) {
   using Fn = bool( __thiscall* )( void*, matrix3x4_t*, int, int, float );
   return Memory::VCall<Fn>( this, Index::IClientRenderable::SetupBones )( this, pBoneToWorld, nMaxBones, boneMask, currentTime );
}

void IClientRenderable::GetRenderBounds( Vector& mins, Vector& maxs ) {
   using Fn = void( __thiscall* )( void*, Vector&, Vector& );
   return Memory::VCall<Fn>( this, Index::IClientRenderable::RenderBounds )( this, mins, maxs );
}

Vector& IClientEntity::OBBMins( ) {
   auto collideable = GetCollideable( );
   return collideable->OBBMins( );
}

Vector& IClientEntity::OBBMaxs( ) {
   auto collideable = GetCollideable( );
   return collideable->OBBMaxs( );
}

Vector& IClientEntity::GetAbsOrigin( ) {
   using Fn = Vector & ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientEntity::GetAbsOrigin )( this );
}

QAngle& IClientEntity::GetAbsAngles( ) {
   using Fn = QAngle & ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::IClientEntity::GetAbsAngles )( this );
}

ClientClass* IClientEntity::GetClientClass( ) {
   auto networkable = GetClientNetworkable( );
   return networkable->GetClientClass( );
}

bool IClientEntity::IsDormant( ) {
   auto networkable = GetClientNetworkable( );
   return networkable->IsDormant( );
}

int IClientEntity::entindex( ) {
   auto networkable = GetClientNetworkable( );
   return networkable->entindex( );
}

const model_t* IClientEntity::GetModel( ) {
   auto renderable = GetClientRenderable( );
   return renderable->GetModel( );
}

bool IClientEntity::SetupBones( matrix3x4_t* pBoneToWorld, int nMaxBones, int boneMask, float currentTime ) {
   auto renderable = GetClientRenderable( );
   return renderable->SetupBones( pBoneToWorld, nMaxBones, boneMask, currentTime );
}

#if 0
void CopyTo( CIKContext * ik, int unk ) {
   using IKCopyTo = void( __thiscall* )( CIKContext*, CIKContext*, int );
   static auto _CopyTo = ( IKCopyTo ) rel32_fix( Memory::Scan( "client.dll", "E8 ? ? ? ? 8B 4C 24 24 EB 49" ) );
   _CopyTo( this, ik, unk );
}
#endif

void CIKContext::Construct( ) {
   typedef void( __thiscall * IKConstruct )( void* );
   auto ik_ctor = ( IKConstruct ) Engine::Displacement.CIKContext.m_nConstructor;
   ik_ctor( this );
}

void CIKContext::Destructor( ) {
   typedef void( __thiscall * IKDestructor )( CIKContext* );
   auto ik_dector = ( IKDestructor ) Engine::Displacement.CIKContext.m_nDestructor;
   ik_dector( this );
}

// This somehow got inlined so we need to rebuild it
void CIKContext::ClearTargets( ) {
   static auto constexpr TARGET_SIZE = 85;

   auto i = 0;
   auto count = *reinterpret_cast< int* >( reinterpret_cast< uint32_t >( this ) + static_cast< ptrdiff_t >( 4080 ) );

   if ( count > 0 ) {
	  auto target = reinterpret_cast< int* >( reinterpret_cast< uint32_t >( this ) + static_cast< ptrdiff_t >( 208 ) );
	  do {
		 *target = -9999;
		 target += TARGET_SIZE;
		 ++i;
	  } while ( i < count );
   }
}

void CIKContext::Init( CStudioHdr* hdr, QAngle* angles, Vector* origin, float currentTime, int frames, int boneMask ) {
   typedef void( __thiscall * Init_t )( void*, CStudioHdr*, QAngle*, Vector*, float, int, int );
   auto ik_init = Engine::Displacement.CIKContext.m_nInit;
   ( ( Init_t ) ik_init )( this, hdr, angles, origin, currentTime, frames, boneMask );
}

void CIKContext::UpdateTargets( Vector* pos, Quaternion* qua, matrix3x4_t* matrix, uint8_t* boneComputed ) {
   typedef void( __thiscall * UpdateTargets_t )( void*, Vector*, Quaternion*, matrix3x4_t*, uint8_t* );
   auto  ik_update_targets = Engine::Displacement.CIKContext.m_nUpdateTargets;
   ( ( UpdateTargets_t ) ik_update_targets )( this, pos, qua, matrix, boneComputed );
}

void CIKContext::SolveDependencies( Vector* pos, Quaternion* qua, matrix3x4_t* matrix, uint8_t* boneComputed ) {
   typedef void( __thiscall * SolveDependencies_t )( void*, Vector*, Quaternion*, matrix3x4_t*, uint8_t* );
   auto  ik_solve_dependencies = Engine::Displacement.CIKContext.m_nSolveDependencies;
   ( ( SolveDependencies_t ) ik_solve_dependencies )( this, pos, qua, matrix, boneComputed );
}

bool C_BaseEntity::IsPlayer( ) {
   using Fn = bool( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::C_BaseEntity::IsPlayer )( this );
}

bool C_BaseEntity::IsWeapon( ) {
   using Fn = bool( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::C_BaseEntity::IsWeapon )( this );
}

void C_BaseEntity::SetAbsVelocity( const Vector& velocity ) {
   static auto m_vecAbsVelocity = Horizon::Memory::FindInDataMap( this->GetPredDescMap( ), "m_vecAbsVelocity" );
   *( Vector* ) ( ( uintptr_t ) this + m_vecAbsVelocity ) = velocity;
}

void C_BaseEntity::SetAbsOrigin( const Vector& origin ) {
   reinterpret_cast< void( __thiscall* )( void*, const Vector& ) >( Engine::Displacement.Function.m_uSetAbsOrigin )( this, origin );
}

void C_BaseEntity::InvalidatePhysicsRecursive( int change_flags ) {
   reinterpret_cast< void( __thiscall* )( void*, int ) >( Engine::Displacement.Function.m_uInvalidatePhysics )( this, change_flags );
}

void C_BaseEntity::SetAbsAngles( const QAngle& angles ) {
   reinterpret_cast< void( __thiscall* )( void*, const QAngle& ) >( Engine::Displacement.Function.m_uSetAbsAngles )( this, angles );
}

std::uint8_t& C_BaseEntity::m_MoveType( ) {
   return *( std::uint8_t* )( ( uintptr_t ) this + Engine::Displacement.C_BaseEntity.m_MoveType );
}

matrix3x4_t& C_BaseEntity::m_rgflCoordinateFrame( ) {
   return *( matrix3x4_t* ) ( ( uintptr_t ) this + Engine::Displacement.C_BaseEntity.m_rgflCoordinateFrame );
}

CCollisionProperty* C_BaseEntity::m_Collision( ) {
   return ( CCollisionProperty* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseEntity.m_Collision );
}

int& C_BaseEntity::m_fEffects( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseEntity.m_fEffects );
}
int& C_BaseEntity::m_iEFlags( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseEntity.m_iEFlags );
}

int& C_BaseEntity::m_iTeamNum( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseEntity.m_iTeamNum );
}

bool C_BaseEntity::IsPlantedC4( ) {
   return GetClientClass( )->m_ClassID == ClassId_t::CPlantedC4;
}

Vector& C_BaseEntity::m_vecOrigin( ) {
   return *( Vector* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseEntity.m_vecOrigin );
}

void C_BaseEntity::UpdateVisibilityAllEntities( ) {
   if ( Engine::Displacement.C_BasePlayer.UpdateVisibilityAllEntities )
	  reinterpret_cast< void( __thiscall* )( void* ) >( Engine::Displacement.C_BasePlayer.UpdateVisibilityAllEntities )( this );
}

float& C_PlantedC4::m_flC4Blow( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_PlantedC4.m_flC4Blow );
}

float& C_BaseEntity::m_flSimulationTime( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseEntity.m_flSimulationTime );
}

float& C_BaseEntity::m_flOldSimulationTime( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseEntity.m_flSimulationTime + 0x4 );
}

void C_BaseEntity::SetPredictionRandomSeed( const CUserCmd* cmd ) {
   *( int* ) ( Engine::Displacement.Data.m_uPredictionRandomSeed ) = cmd ? cmd->random_seed : -1;
}

void C_BaseEntity::SetPredictionPlayer( C_BasePlayer* player ) {
   *( C_BasePlayer * * ) ( Engine::Displacement.Data.m_uPredictionPlayer ) = player;
}

CBaseHandle& C_BaseEntity::m_hOwnerEntity( ) {
   return *( CBaseHandle* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseEntity.m_hOwnerEntity );;
}

void C_BaseAnimating::UpdateClientSideAnimation( ) {
   using Fn = void( __thiscall* )( void* );
   Memory::VCall<Fn>( this, Index::C_BaseAnimating::UpdateClientSideAnimation )( this );
}

void C_BaseAnimating::FixedAnimationsUpdate( ) {
   auto ishltv = Source::m_pClientState->m_bIsHLTV( );
   auto backup = this->m_bClientSideAnimation( );
   this->m_bClientSideAnimation( ) = true; // disable CGlobalVarsBase::curtime interpolation
   Source::m_pClientState->m_bIsHLTV( ) = true; // disable velocity and duck amount interpolation
   this->UpdateClientSideAnimation( );
   this->m_bClientSideAnimation( ) = backup;
   Source::m_pClientState->m_bIsHLTV( ) = ishltv;
}

void C_BaseAnimating::InvalidateBoneCache( ) {
   *( uint32_t* ) ( &m_flLastBoneSetupTime( ) ) = 0xFF7FFFFF;
   m_iMostRecentModelBoneCounter( ) = 0;
   m_BoneAccessor( ).m_ReadableBones = m_BoneAccessor( ).m_WritableBones = 0;
}

void C_BaseAnimating::LockStudioHdr( ) {
   auto _LockStudioHdr = ( void( __thiscall* )( void* ) )Engine::Displacement.Function.m_LockStudioHdr;
   _LockStudioHdr( this );
}

bool C_BaseAnimating::ComputeHitboxSurroundingBox( Vector& mins, Vector& maxs, const matrix3x4_t* boneTransform ) {
   auto model = GetModel( );
   if ( !model )
	  return false;

   auto hdr = Source::m_pModelInfo->GetStudiomodel( model );
   if ( !hdr )
	  return false;

   mstudiohitboxset_t* set = hdr->pHitboxSet( m_nHitboxSet( ) );
   if ( !set || !set->numhitboxes )
	  return false;

   const matrix3x4_t* bones = boneTransform ? boneTransform : this->m_BoneAccessor( ).m_pBones;
   mins.Init( FLT_MAX, FLT_MAX, FLT_MAX );
   maxs.Init( -FLT_MAX, -FLT_MAX, -FLT_MAX );
   Vector abs_min, abs_max;

   for ( int i = 0; i < set->numhitboxes; i++ ) {
	  mstudiobbox_t* pbox = set->pHitbox( i );

	  bones[ pbox->bone ].TransformAABB( pbox->bbmin, pbox->bbmax, abs_min, abs_max );

	  mins = abs_min.Min( mins );
	  maxs = abs_max.Max( maxs );
   }

   return true;
}

int C_BaseAnimating::GetSequenceActivity( int sequence ) {
   auto model = this->GetModel( );
   if ( !model )
	  return -1;

   auto hdr = Source::m_pModelInfo->GetStudiomodel( model );

   if ( !hdr )
	  return -1;

   // sig for stuidohdr_t version: 53 56 8B F1 8B DA 85 F6 74 55
   // sig for C_BaseAnimating version: 55 8B EC 83 7D 08 FF 56 8B F1 74 3D
   // c_csplayer vfunc 242, follow calls to find the function.
   return reinterpret_cast< int( __fastcall* )( void*, studiohdr_t*, int ) >( Engine::Displacement.Function.m_uGetSequenceActivity )( this, hdr, sequence );
}

int& C_BaseAnimating::m_nHitboxSet( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseAnimating.m_nHitboxSet );
}

int& C_BaseAnimating::m_iMostRecentModelBoneCounter( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.C_BaseAnimating.m_iMostRecentModelBoneCounter );
}

bool& C_BaseAnimating::m_bClientSideAnimation( ) {
   return *( bool* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseAnimating.m_bClientSideAnimation );
}

bool& C_BaseAnimating::m_bShouldDraw( ) {
   return *( bool* ) ( ( uintptr_t ) this + Engine::Displacement.C_BaseAnimating.m_bShouldDraw );
}

float& C_BaseAnimating::m_flLastBoneSetupTime( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.C_BaseAnimating.m_flLastBoneSetupTime );
}

float* C_BaseAnimating::m_flPoseParameter( ) {
   return ( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseAnimating.m_flPoseParameter );
}

CBoneAccessor& C_BaseAnimating::m_BoneAccessor( ) {
   return *( CBoneAccessor* ) ( ( uintptr_t ) this + Engine::Displacement.C_BaseAnimating.m_BoneAccessor );
}

CUtlVector<matrix3x4_t>& C_BaseAnimating::m_CachedBoneData( ) {
   return *( CUtlVector<matrix3x4_t>* )( ( uintptr_t ) this + Engine::Displacement.C_BaseAnimating.m_CachedBoneData );
}

CUtlVector<C_AnimationLayer>& C_BaseAnimating::m_AnimOverlay( ) {
   return *( CUtlVector<C_AnimationLayer>* )( ( uintptr_t ) this + 0x2970 ); // 7C
}

Vector* C_BaseAnimating::m_vecBonePos( ) {
   return ( Vector* ) ( ( uintptr_t ) this + Engine::Displacement.C_BaseAnimating.m_nCachedBonesPosition );
}

Quaternion* C_BaseAnimating::m_quatBoneRot( ) {
   return ( Quaternion* ) ( ( uintptr_t ) this + Engine::Displacement.C_BaseAnimating.m_nCachedBonesRotation );;
}

CStudioHdr* C_BaseAnimating::m_pStudioHdr( ) {
   return *( CStudioHdr * * ) ( ( uintptr_t ) this + Engine::Displacement.C_BaseAnimating.m_pStudioHdr );
}

CBaseHandle& C_BaseCombatCharacter::m_hActiveWeapon( ) {
   return *( CBaseHandle* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatCharacter.m_hActiveWeapon );
}

float& C_BaseCombatCharacter::m_flNextAttack( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatCharacter.m_flNextAttack );
}

CBaseHandle* C_BaseCombatCharacter::m_hMyWeapons( ) {
   return ( CBaseHandle* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatCharacter.m_hMyWeapons );
}

CBaseHandle* C_BaseCombatCharacter::m_hMyWearables( ) {
   return ( CBaseHandle* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatCharacter.m_hMyWearables );
}

float C_Inferno::m_flSpawnTime( ) {
   return *( float* ) ( ( uintptr_t ) this + 0x20 );
}

int C_SmokeGrenadeProjectile::m_nSmokeEffectTickBegin( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_SmokeGrenadeProjectile.m_nSmokeEffectTickBegin );
}

void CCollisionProperty::SetCollisionBounds( const Vector& mins, const Vector& maxs ) {
   using Fn = void( __thiscall* )( CCollisionProperty*, const Vector&, const Vector& );
   static auto mem = ( Fn )Memory::Scan( XorStr( "client.dll" ), XorStr( "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 10 56 57 8B 7B" ) );   
   mem( this, mins, maxs );
}

float C_BaseEntity::m_flAnimationTime( ) {
	return m_flOldSimulationTime( ) + Source::m_pGlobalVars->interval_per_tick;
}