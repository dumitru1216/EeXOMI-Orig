#include "weapon.hpp"
#include "displacement.hpp"
#include "source.hpp"
#include "PropManager.hpp"


float& C_BaseCombatWeapon::m_flNextPrimaryAttack( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatWeapon.m_flNextPrimaryAttack );
}

float& C_BaseCombatWeapon::m_flNextSecondaryAttack( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatWeapon.m_flNextSecondaryAttack );
}

float& C_BaseCombatWeapon::m_flPostponeFireReadyTime( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_WeaponCSBase.m_flPostponeFireReadyTime );
}

CBaseHandle& C_BaseCombatWeapon::m_hOwner( ) {
   return *( CBaseHandle* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatWeapon.m_hOwner );
}

int& C_BaseCombatWeapon::m_iClip1( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatWeapon.m_iClip1 );
}

int& C_BaseCombatWeapon::m_iPrimaryReserveAmmoCount( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatWeapon.m_iPrimaryReserveAmmoCount );
}

short& C_BaseCombatWeapon::m_iItemDefinitionIndex( ) {
   return *( short* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatWeapon.m_iItemDefinitionIndex );
}

CUtlVector<IRefCounted*>& C_BaseCombatWeapon::m_CustomMaterials( ) {
   return *( CUtlVector< IRefCounted* >* )( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatWeapon.m_CustomMaterials );
}

bool& C_BaseCombatWeapon::m_bCustomMaterialInitialized( ) {
   return *( bool* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCombatWeapon.m_bCustomMaterialInitialized );
}

float& C_WeaponCSBaseGun::m_flRecoilIndex( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_WeaponCSBase.m_flRecoilIndex );
}

float& C_WeaponCSBaseGun::m_fLastShotTime( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_WeaponCSBase.m_fLastShotTime );
}

int& C_WeaponCSBaseGun::m_weaponMode( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_WeaponCSBase.m_weaponMode );
}

int& C_WeaponCSBaseGun::m_zoomLevel( ) {
   return *( int* ) ( ( uintptr_t ) this + Engine::Displacement.DT_WeaponCSBaseGun.m_zoomLevel );
}

int& C_WeaponCSBaseGun::m_Activity( ) {
   static unsigned int m_activity = Memory::FindInDataMap( GetPredDescMap( ), "m_Activity" );
   return *( int* ) ( ( uintptr_t ) this + m_activity );
}

float& C_WeaponCSBaseGun::m_flThrowStrength( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCSGrenade.m_flThrowStrength );
}

float& C_WeaponCSBaseGun::m_fThrowTime( ) {
   return *( float* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCSGrenade.m_fThrowTime );
}

bool& C_WeaponCSBaseGun::m_bPinPulled( ) {
   return *( bool* ) ( ( uintptr_t ) this + Engine::Displacement.DT_BaseCSGrenade.m_bPinPulled );
}

Encrypted_t<CCSWeaponInfo> C_WeaponCSBaseGun::GetCSWeaponData( ) {
   using Fn = CCSWeaponInfo * ( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::C_WeaponCSBaseGun::GetCSWeaponData )( this );
}
//8B 81 ? ? ? ? 85 C0 0F 84 ? ? ? ? C3
//
//10B8F32C
//
//10B8FA44

float C_WeaponCSBaseGun::GetMaxSpeed( ) {
   auto pWeaponData = GetCSWeaponData( );
   if ( !pWeaponData.IsValid( ) )
	  return 260.0f;

   if ( m_weaponMode( ) == 0 )
	  return pWeaponData->m_flMaxSpeed;

   return pWeaponData->m_flMaxSpeed2;
}

float C_WeaponCSBaseGun::GetSpread( ) {
   using Fn = float( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::C_WeaponCSBaseGun::GetSpread )( this );
}

float C_WeaponCSBaseGun::GetInaccuracy( ) {
   using Fn = float( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::C_WeaponCSBaseGun::GetInnacuracy )( this );
}

void C_WeaponCSBaseGun::UpdateAccuracyPenalty( ) {
   using Fn = void( __thiscall* )( void* );
   return Memory::VCall<Fn>( this, Index::C_WeaponCSBaseGun::UpdateAccuracyPenalty )( this );
}

bool C_WeaponCSBaseGun::IsFireTime( ) {
   return ( Source::m_pGlobalVars->curtime >= m_flNextPrimaryAttack( ) );
}

bool C_WeaponCSBaseGun::IsSecondaryFireTime( ) {
   return ( Source::m_pGlobalVars->curtime >= m_flNextSecondaryAttack( ) );
}

bool C_WeaponCSBaseGun::IsInThrow( ) {
   if ( !m_bPinPulled( ) ) {
	  float throwTime = m_fThrowTime( );

	  if ( throwTime > 0 )
		 return true;
   }
   return false;
}

CUtlVector<IRefCounted*>& C_EconItemView::m_CustomMaterials( ) {
   static auto _m_CustomMaterials = Engine::Displacement.DT_BaseAttributableItem.m_Item + 0x14;
   return *( CUtlVector< IRefCounted* >* )( ( uintptr_t ) this + _m_CustomMaterials );
}

CUtlVector<IRefCounted*>& C_EconItemView::m_VisualsDataProcessors( ) {
   static auto _m_CustomMaterials = Engine::Displacement.DT_BaseAttributableItem.m_Item + 0x230;
   return *( CUtlVector< IRefCounted* >* )( ( uintptr_t ) this + _m_CustomMaterials );
}

int32_t& C_EconItemView::m_bInitialized( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_bInitialized );
}

int32_t& C_EconItemView::m_iEntityLevel( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_iEntityLevel );
}

int32_t& C_EconItemView::m_iAccountID( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_iAccountID );
}

int32_t& C_EconItemView::m_iItemIDLow( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_iItemIDLow );
}

int32_t& C_EconItemView::m_iItemIDHigh( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_iItemIDHigh );
}

int32_t& C_EconItemView::m_iEntityQuality( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_iEntityQuality );
}

uint32_t& C_EconItemView::m_nFallbackPaintKit( ) {
   return *( uint32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_nFallbackPaintKit );
}

uint32_t& C_EconItemView::m_nFallbackSeed( ) {
   return *( uint32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_nFallbackSeed );
}

uint32_t& C_EconItemView::m_nFallbackStatTrak( ) {
   return *( uint32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_nFallbackStatTrak );
}

float& C_EconItemView::m_flFallbackWear( ) {
   return *( float* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_flFallbackWear );
}

str_32& C_EconItemView::m_szCustomName( ) {
   return *( str_32* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_szCustomName );
}

int16_t& C_EconItemView::m_iItemDefinitionIndex( ) {
   return *( int16_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_iItemDefinitionIndex );
}

uint64_t& C_BaseAttributableItem::m_OriginalOwnerXuid( ) {
   return *( uint64_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_OriginalOwnerXuidLow );
}

int32_t& C_BaseAttributableItem::m_OriginalOwnerXuidLow( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_OriginalOwnerXuidLow );
}

int32_t& C_BaseAttributableItem::m_OriginalOwnerXuidHigh( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_OriginalOwnerXuidHigh );
}

int32_t& C_BaseAttributableItem::m_nFallbackPaintKit( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_nFallbackPaintKit );
}

int32_t& C_BaseAttributableItem::m_nFallbackSeed( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_nFallbackSeed );
}

int32_t& C_BaseAttributableItem::m_nFallbackStatTrak( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_nFallbackStatTrak );
}

int32_t& C_BaseAttributableItem::m_flFallbackWear( ) {
   return *( int32_t* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_flFallbackWear );
}

str_32& C_BaseAttributableItem::m_szCustomName( ) {
   return *( str_32* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseAttributableItem.m_szCustomName );
}

CHandle<C_BaseEntity> C_BaseAttributableItem::m_hWeaponWorldModel( ) {
   return *( CHandle<C_BaseEntity>* )( ( int32_t ) this + Engine::Displacement.DT_BaseCombatWeapon.m_hWeaponWorldModel );
}

CHandle<C_BaseEntity> C_BaseViewModel::m_hOwner( ) {
   return *( CHandle<C_BaseEntity>* )( ( int32_t ) this + Engine::Displacement.DT_BaseViewModel.m_hOwner );
}

CHandle<C_BaseCombatWeapon> C_BaseViewModel::m_hWeapon( ) {
   return *( CHandle<C_BaseCombatWeapon>* )( ( int32_t ) this + Engine::Displacement.DT_BaseViewModel.m_hWeapon );
}

void C_BaseViewModel::SendViewModelMatchingSequence( int sequence ) {
   using Fn = void( __thiscall* )( void*, int );
   return  Memory::VCall<Fn>( this, 246 )( this, sequence );
}

int& C_BaseEntity::m_nModelIndex( ) {
   return *( int* ) ( ( int32_t ) this + Engine::Displacement.DT_BaseEntity.m_nModelIndex );
}
