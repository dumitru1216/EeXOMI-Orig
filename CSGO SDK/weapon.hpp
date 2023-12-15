#pragma once

#include "entity.hpp"
#include "CBaseHandle.hpp"

#pragma region decl_indices
namespace Index
{
  namespace C_WeaponCSBaseGun
  {
	 enum
	 {
		 GetSpread = 439,
		 GetCSWeaponData = 446,
		 GetInnacuracy = 469,
		 UpdateAccuracyPenalty = 471,
	 };
  }
}

#pragma endregion

class IRefCounted {
private:
  volatile long refCount;

public:
  virtual void destructor( char bDelete ) = 0;
  virtual bool OnFinalRelease( ) = 0;

  void unreference( ) {
	 if( InterlockedDecrement( &refCount ) == 0 && OnFinalRelease( ) )
		destructor( 1 );
  }
};

class C_EconItemView {
public:
  CUtlVector< IRefCounted* >& m_CustomMaterials( );
  CUtlVector< IRefCounted* >& m_VisualsDataProcessors( );

  int32_t& m_bInitialized( );
  int32_t& m_iEntityLevel( );
  int32_t& m_iAccountID( );
  int32_t& m_iItemIDLow( );
  int32_t& m_iItemIDHigh( );
  int32_t& m_iEntityQuality( );

  uint32_t& m_nFallbackPaintKit( );
  uint32_t& m_nFallbackSeed( );
  uint32_t& m_nFallbackStatTrak( );

  float& m_flFallbackWear( );

  str_32& m_szCustomName( );

  int16_t& m_iItemDefinitionIndex( );
};

class C_BaseCombatWeapon : public C_BaseEntity
{
public:
  float& m_flNextPrimaryAttack( );
  float& m_flNextSecondaryAttack( );
  float& m_flPostponeFireReadyTime( );
  CBaseHandle& m_hOwner( );
  int& m_iClip1( );
  int& m_iPrimaryReserveAmmoCount( );
  short& m_iItemDefinitionIndex( );

  CUtlVector< IRefCounted* >& m_CustomMaterials();
  bool& m_bCustomMaterialInitialized();

  C_EconItemView& m_Item( ) {
	 // Cheating. It should be this + m_Item netvar but then the netvars inside C_EconItemView wont work properly.
	 // A real fix for this requires a rewrite of the netvar manager
	 return *( C_EconItemView* )this;
  }
};

class C_WeaponCSBaseGun : public C_BaseCombatWeapon
{
public:
  float& m_flRecoilIndex( );
  float & m_fLastShotTime( );
  int& m_weaponMode( );
  int& m_zoomLevel( );
  int& m_Activity( );
public:
  float& m_flThrowStrength( );
  float& m_fThrowTime( );
  bool& m_bPinPulled( );

public:
  Encrypted_t<CCSWeaponInfo> GetCSWeaponData( );

  float GetMaxSpeed( );
  float GetSpread( );
  float GetInaccuracy( );
  void UpdateAccuracyPenalty( );

  bool IsInThrow( );
  bool IsFireTime( );
  bool IsSecondaryFireTime( );
};

class C_BaseAttributableItem : public C_BaseAnimating {

public:
  uint64_t& m_OriginalOwnerXuid( );

  int32_t& m_OriginalOwnerXuidLow( );
  int32_t& m_OriginalOwnerXuidHigh( );

  int32_t& m_nFallbackPaintKit( );
  int32_t& m_nFallbackSeed( );
  int32_t& m_nFallbackStatTrak( );
  int32_t& m_flFallbackWear( );

  str_32& m_szCustomName( );

  CHandle< C_BaseEntity > m_hWeaponWorldModel( );

  C_EconItemView& m_Item( ) {
	 // Cheating. It should be this + m_Item netvar but then the netvars inside C_EconItemView wont work properly.
	 // A real fix for this requires a rewrite of the netvar manager
	 return *( C_EconItemView* )this;
  }
};

class C_BaseViewModel : public C_BaseAttributableItem {
public:
  CHandle< C_BaseEntity > m_hOwner( ); //"DT_BaseViewModel", "m_hOwner");
  CHandle< C_BaseCombatWeapon > m_hWeapon( ); //"DT_BaseViewModel", "m_hWeapon");
  void SendViewModelMatchingSequence( int sequence );
};

