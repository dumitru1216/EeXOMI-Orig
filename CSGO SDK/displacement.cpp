#include "Displacement.hpp"
#include "PropManager.hpp"
#include "FnvHash.hpp"
#include "VMP.hpp"

DllInitializeData Engine::Displacement{};

namespace Engine
{

#ifdef DevelopMode
   __forceinline uintptr_t rel32_fix( uintptr_t ptr ) {
	  auto offset = *( uintptr_t* ) ( ptr + 0x1 );
	  return ( uintptr_t ) ( ptr + 5 + offset );
   }

   __forceinline uintptr_t relative( uintptr_t pointer, const std::size_t& amt ) {
	   if ( !pointer )
		   return {};

	   auto out = pointer + amt;
	   auto r = *( std::uint32_t* )out;

	   if ( !r )
		   return {};

	   out = ( out + 4 ) + r;

	   return ( uintptr_t )out;
   }

   void Create( ) {
      auto& pPropManager = PropManager::Instance( );

	  auto image_vstdlib = GetModuleHandleA( "vstdlib.dll" );
	  auto image_client = ( std::uintptr_t )GetModuleHandleA( "client.dll" );
	  auto image_engine = ( std::uintptr_t )GetModuleHandleA( "engine.dll" );
	  auto image_server = ( std::uintptr_t )GetModuleHandleA( "server.dll" );
	  auto image_shaderapidx9 = ( std::uintptr_t )GetModuleHandleA( "shaderapidx9.dll" );

	  // TODO: datamap
	  Displacement.C_BaseEntity.m_MoveType = pPropManager->GetOffset( "DT_BaseEntity", "m_nRenderMode" ) + 1;
	  Displacement.C_BaseEntity.m_rgflCoordinateFrame = pPropManager->GetOffset( "DT_BaseEntity", "m_CollisionGroup" ) - 0x30;

	  Displacement.DT_BaseEntity.m_iTeamNum = pPropManager->GetOffset( "DT_BaseEntity", "m_iTeamNum" );
	  Displacement.DT_BaseEntity.m_vecOrigin = pPropManager->GetOffset( "DT_BaseEntity", "m_vecOrigin" );
	  Displacement.DT_BaseEntity.m_flSimulationTime = pPropManager->GetOffset( "DT_BaseEntity", "m_flSimulationTime" );
	  Displacement.DT_BaseEntity.m_fEffects = pPropManager->GetOffset( "DT_BaseEntity", "m_fEffects" );
	  Displacement.DT_BaseEntity.m_iEFlags = Displacement.DT_BaseEntity.m_fEffects - 0x8;
	  Displacement.DT_BaseEntity.m_hOwnerEntity = pPropManager->GetOffset( "DT_BaseEntity", "m_hOwnerEntity" );
	  Displacement.DT_BaseEntity.m_nModelIndex = pPropManager->GetOffset( "DT_BaseEntity", "m_nModelIndex" );
	  Displacement.DT_BaseEntity.m_Collision = pPropManager->GetOffset( "DT_BaseEntity", "m_Collision" );
	  Displacement.DT_BaseEntity.m_flAnimTime = pPropManager->GetOffset( "DT_BaseEntity", "m_flAnimTime" );

	  auto m_nForceBone = pPropManager->GetOffset( "DT_BaseAnimating", "m_nForceBone" );
	  auto InvalidateBoneCache = Memory::Scan( image_client, "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81" );

	  Displacement.C_BaseAnimating.m_BoneAccessor = m_nForceBone + 0x1C; // todo
	  Displacement.C_BaseAnimating.m_iMostRecentModelBoneCounter = *( int* ) ( InvalidateBoneCache + 0x1B );
	  Displacement.C_BaseAnimating.m_flLastBoneSetupTime = *( int* ) ( InvalidateBoneCache + 0x11 );
	  Displacement.C_BaseAnimating.m_CachedBoneData = *( int* )( Memory::Scan( image_client, XorStr( "FF B7 ?? ?? ?? ?? 52" ) ) + 2 ) + 0x4;
	  Displacement.C_BaseAnimating.m_AnimOverlay = *( int* )( Memory::Scan( image_client, XorStr( "8B 89 ?? ?? ?? ?? 8D 0C D1" ) ) + 2 );

	  auto BoneSnapshotsCall = Memory::Scan( image_client, XorStr( "8D 8F ?? ?? ?? ?? 6A 01 C7 87" ) );
	  Displacement.C_BaseAnimating.m_pFirstBoneSnapshot = *( int* )( BoneSnapshotsCall + 0x2 );
	  Displacement.C_BaseAnimating.m_pSecondBoneSnapshot = *( int* )( BoneSnapshotsCall + 0x1B );

	  auto CacheBoneData = Memory::Scan( image_client, XorStr( "8D 87 ?? ?? ?? ?? 50 E8 ?? ?? ?? ?? 8B 06" ) );
	  Displacement.C_BaseAnimating.m_nCachedBonesPosition = *( int* )( CacheBoneData + 0x2 ) + 0x4;
	  Displacement.C_BaseAnimating.m_nCachedBonesRotation = *( int* )( CacheBoneData + 0x25 ) + 0x4;
	  Displacement.C_BaseAnimating.m_pStudioHdr = *( int* )( Memory::Scan( image_client, XorStr( "8B B7 ?? ?? ?? ?? 89 74 24 20" ) ) + 0x2 ) + 0x4;
	  Displacement.C_BaseAnimating.m_bShouldDraw = *( int* )( Memory::Scan( image_client, XorStr( "FF 15 ?? ?? ?? ?? 80 BE ?? ?? ?? ?? ?? 0F 84 ?? ?? ?? ??" ) ) + 0x8 );
	  Displacement.C_BaseAnimating.m_pBoneMerge = *( int* )( Memory::Scan( image_client, XorStr( "89 86 ?? ?? ?? ?? E8 ?? ?? ?? ?? FF 75 08" ) ) + 2 ); // 89 86 ? ? ? ? E8 ? ? ? ? FF 75 08
	  Displacement.C_BaseAnimating.m_pIk = 0x2570;// *( int* )( Memory::Scan( image_client, XorStr( "8B 8F ?? ?? ?? ?? 89 4C 24 1C" ) ) + 2 ) + 4;
	  // ( uintptr_t )player + ( m_nForceBone - 0x1C );
	  Displacement.DT_BaseAnimating.m_bClientSideAnimation = pPropManager->GetOffset( "DT_BaseAnimating", "m_bClientSideAnimation" );
	  Displacement.DT_BaseAnimating.m_flPoseParameter = pPropManager->GetOffset( "DT_BaseAnimating", "m_flPoseParameter" );
	  Displacement.DT_BaseAnimating.m_nHitboxSet = pPropManager->GetOffset( "DT_BaseAnimating", "m_nHitboxSet" );
	  Displacement.DT_BaseAnimating.m_flCycle = pPropManager->GetOffset( "DT_BaseAnimating", "m_flCycle" );
	  Displacement.DT_BaseAnimating.m_nSequence = pPropManager->GetOffset( "DT_BaseAnimating", "m_nSequence" );
	  Displacement.DT_BaseAnimating.m_flEncodedController = pPropManager->GetOffset( "DT_BaseAnimating", "m_flEncodedController" );

	  Displacement.DT_BaseCombatCharacter.m_hActiveWeapon = pPropManager->GetOffset( "DT_BaseCombatCharacter", "m_hActiveWeapon" );
	  Displacement.DT_BaseCombatCharacter.m_flNextAttack = pPropManager->GetOffset( "DT_BaseCombatCharacter", "m_flNextAttack" );
	  Displacement.DT_BaseCombatCharacter.m_hMyWeapons = pPropManager->GetOffset( "DT_BaseCombatCharacter", "m_hMyWeapons" ) / 2;
	  Displacement.DT_BaseCombatCharacter.m_hMyWearables = pPropManager->GetOffset( "DT_BaseCombatCharacter", "m_hMyWearables" );

	  Displacement.C_BasePlayer.m_pCurrentCommand = 0x3338;	
	  auto relative_call = Memory::Scan( image_client, XorStr( "E8 ? ? ? ? 83 7D D8 00 7C 0F" ) );
	  auto offset = *( uintptr_t* )( relative_call + 0x1 );

	  Displacement.C_BasePlayer.UpdateVisibilityAllEntities = ( DWORD32 ) ( relative_call + 5 + offset );

	  Displacement.DT_BasePlayer.m_aimPunchAngle = pPropManager->GetOffset( "DT_BasePlayer", "m_aimPunchAngle" );
	  Displacement.DT_BasePlayer.m_aimPunchAngleVel = pPropManager->GetOffset( "DT_BasePlayer", "m_aimPunchAngleVel" );
	  Displacement.DT_BasePlayer.m_viewPunchAngle = pPropManager->GetOffset( "DT_BasePlayer", "m_viewPunchAngle" );
	  Displacement.DT_BasePlayer.m_vecViewOffset = pPropManager->GetOffset( "DT_BasePlayer", "m_vecViewOffset[0]" );
	  Displacement.DT_BasePlayer.m_vecVelocity = pPropManager->GetOffset( "DT_BasePlayer", "m_vecVelocity[0]" );
	  Displacement.DT_BasePlayer.m_vecBaseVelocity = pPropManager->GetOffset( "DT_BasePlayer", "m_vecBaseVelocity" );
	  Displacement.DT_BasePlayer.m_flFallVelocity = pPropManager->GetOffset( "DT_BasePlayer", "m_flFallVelocity" );
	  Displacement.DT_BasePlayer.m_flDuckAmount = pPropManager->GetOffset( "DT_BasePlayer", "m_flDuckAmount" );
	  Displacement.DT_BasePlayer.m_flDuckSpeed = pPropManager->GetOffset( "DT_BasePlayer", "m_flDuckSpeed" );
	  Displacement.DT_BasePlayer.m_lifeState = pPropManager->GetOffset( "DT_BasePlayer", "m_lifeState" );
	  Displacement.DT_BasePlayer.m_nTickBase = pPropManager->GetOffset( "DT_BasePlayer", "m_nTickBase" );
	  Displacement.DT_BasePlayer.m_iHealth = pPropManager->GetOffset( "DT_BasePlayer", "m_iHealth" );
	  Displacement.DT_BasePlayer.m_iDefaultFOV = pPropManager->GetOffset( "DT_BasePlayer", "m_iDefaultFOV" );
	  Displacement.DT_BasePlayer.m_fFlags = pPropManager->GetOffset( "DT_BasePlayer", "m_fFlags" );
	  Displacement.DT_BasePlayer.m_iObserverMode = pPropManager->GetOffset( "DT_BasePlayer", "m_iObserverMode" );
	  Displacement.DT_BasePlayer.pl = pPropManager->GetOffset( "DT_BasePlayer", "pl" );
	  Displacement.DT_BasePlayer.m_hObserverTarget = pPropManager->GetOffset( "DT_BasePlayer", "m_hObserverTarget" );
	  Displacement.DT_BasePlayer.m_hViewModel = pPropManager->GetOffset( "DT_BasePlayer", "m_hViewModel[0]" );
	  Displacement.DT_BasePlayer.m_vphysicsCollisionState = pPropManager->GetOffset( "DT_BasePlayer", "m_vphysicsCollisionState" );
	  Displacement.DT_BasePlayer.m_ubEFNoInterpParity = pPropManager->GetOffset( "DT_BasePlayer", "m_ubEFNoInterpParity" );
	  Displacement.DT_BasePlayer.m_ubOldEFNoInterpParity = *( int* )( Memory::Scan( image_client, XorStr( "8A 87 ?? ?? ?? ?? 8D 5F F8" ) ) + 2 ) + 8;

	  Displacement.C_CSPlayer.m_PlayerAnimState = *( int* )( Memory::Scan( image_client, XorStr( "8B 8E ?? ?? ?? ?? 85 C9 74 3E" ) ) + 2 );
	  Displacement.C_CSPlayer.m_flSpawnTime = *( int* )( Memory::Scan( image_client, XorStr( "89 86 ?? ?? ?? ?? E8 ?? ?? ?? ?? 80 BE ?? ?? ?? ?? ??" ) ) + 2 );

	  Displacement.DT_CSPlayer.m_angEyeAngles = pPropManager->GetOffset( "DT_CSPlayer", "m_angEyeAngles[0]" );
	  Displacement.DT_CSPlayer.m_nSurvivalTeam = pPropManager->GetOffset( "DT_CSPlayer", "m_nSurvivalTeam" );
	  Displacement.DT_CSPlayer.m_bHasHelmet = pPropManager->GetOffset( "DT_CSPlayer", "m_bHasHelmet" );
	  Displacement.DT_CSPlayer.m_bHasHeavyArmor = pPropManager->GetOffset( "DT_CSPlayer", "m_bHasHeavyArmor" );
	  Displacement.DT_CSPlayer.m_ArmorValue = pPropManager->GetOffset( "DT_CSPlayer", "m_ArmorValue" );
	  Displacement.DT_CSPlayer.m_bScoped = pPropManager->GetOffset( "DT_CSPlayer", "m_bIsScoped" );
	  Displacement.DT_CSPlayer.m_bIsWalking = pPropManager->GetOffset( "DT_CSPlayer", "m_bIsWalking" );
	  Displacement.DT_CSPlayer.m_iAccount = pPropManager->GetOffset( "DT_CSPlayer", "m_iAccount" );
	  Displacement.DT_CSPlayer.m_iShotsFired = pPropManager->GetOffset( "DT_CSPlayer", "m_iShotsFired" );
	  Displacement.DT_CSPlayer.m_flFlashDuration = pPropManager->GetOffset( "DT_CSPlayer", "m_flFlashDuration" );
	  Displacement.DT_CSPlayer.m_flLowerBodyYawTarget = pPropManager->GetOffset( "DT_CSPlayer", "m_flLowerBodyYawTarget" );
	  Displacement.DT_CSPlayer.m_flVelocityModifier = pPropManager->GetOffset( "DT_CSPlayer", "m_flVelocityModifier" );
	  Displacement.DT_CSPlayer.m_bGunGameImmunity = pPropManager->GetOffset( "DT_CSPlayer", "m_bGunGameImmunity" );
	  Displacement.DT_CSPlayer.m_flHealthShotBoostExpirationTime = pPropManager->GetOffset( "DT_CSPlayer", "m_flHealthShotBoostExpirationTime" );
	  Displacement.DT_CSPlayer.m_iMatchStats_Kills = pPropManager->GetOffset( "DT_CSPlayer", "m_iMatchStats_Kills" );
	  Displacement.DT_CSPlayer.m_iMatchStats_Deaths = pPropManager->GetOffset( "DT_CSPlayer", "m_iMatchStats_Deaths" );
	  Displacement.DT_CSPlayer.m_iMatchStats_HeadShotKills = pPropManager->GetOffset( "DT_CSPlayer", "m_iMatchStats_HeadShotKills" );
	  Displacement.DT_CSPlayer.m_iMoveState = pPropManager->GetOffset( "DT_CSPlayer", "m_iMoveState" );
	  Displacement.DT_CSPlayer.m_bWaitForNoAttack = pPropManager->GetOffset( "DT_CSPlayer", "m_bWaitForNoAttack" );

	  // hope this doesnt crash
	  Displacement.DT_CSPlayer.m_bCustomPlayer = 0x39E1;// *( int* )( Memory::Scan( image_client, "80 BF ?? ?? ?? ?? ?? 0F 84 ?? ?? ?? ?? 83 BF ?? ?? ?? ?? ?? 74 7C" ) + 2 ); // hope this doesnt crash
	 
	  Displacement.DT_CSPlayer.m_iPlayerState = pPropManager->GetOffset( "DT_CSPlayer", "m_iPlayerState" );
	  Displacement.DT_CSPlayer.m_bIsDefusing = pPropManager->GetOffset( "DT_CSPlayer", "m_bIsDefusing" );
	  Displacement.DT_CSRagdoll.m_hPlayer = pPropManager->GetOffset( "DT_CSRagdoll", "m_hPlayer" );

	  Displacement.DT_BaseCombatWeapon.m_flNextPrimaryAttack = pPropManager->GetOffset( "DT_BaseCombatWeapon", "m_flNextPrimaryAttack" );
	  Displacement.DT_BaseCombatWeapon.m_flNextSecondaryAttack = pPropManager->GetOffset( "DT_BaseCombatWeapon", "m_flNextSecondaryAttack" );
	  Displacement.DT_BaseCombatWeapon.m_hOwner = pPropManager->GetOffset( "DT_BaseCombatWeapon", "m_hOwner" );
	  Displacement.DT_BaseCombatWeapon.m_iClip1 = pPropManager->GetOffset( "DT_BaseCombatWeapon", "m_iClip1" );
	  Displacement.DT_BaseCombatWeapon.m_iPrimaryReserveAmmoCount = pPropManager->GetOffset( "DT_BaseCombatWeapon", "m_iPrimaryReserveAmmoCount" );
	  Displacement.DT_BaseCombatWeapon.m_iItemDefinitionIndex = pPropManager->GetOffset( "DT_BaseCombatWeapon", "m_iItemDefinitionIndex" );
	  Displacement.DT_BaseCombatWeapon.m_hWeaponWorldModel = pPropManager->GetOffset( "DT_BaseCombatWeapon", "m_hWeaponWorldModel" );
	  Displacement.DT_BaseCombatWeapon.m_iWorldModelIndex = pPropManager->GetOffset( "DT_BaseCombatWeapon", "m_iWorldModelIndex" );
	  Displacement.DT_BaseCombatWeapon.m_iWorldDroppedModelIndex = pPropManager->GetOffset( "DT_BaseCombatWeapon", "m_iWorldDroppedModelIndex" );
	  Displacement.DT_BaseCombatWeapon.m_iViewModelIndex = pPropManager->GetOffset( "DT_BaseCombatWeapon", "m_iViewModelIndex" );

	  Displacement.DT_BaseCombatWeapon.m_CustomMaterials = ( *( int* )( Memory::Scan( image_client, XorStr( "83 BE ? ? ? ? ? 7F 67" ) ) + 0x2 ) ) - 12;
	  Displacement.DT_BaseCombatWeapon.m_bCustomMaterialInitialized = *( int* )( Memory::Scan( image_client, XorStr( "C6 86 ? ? ? ? ? FF 50 04" ) ) + 0x2 );

	  Displacement.DT_WeaponCSBase.m_flRecoilIndex = pPropManager->GetOffset( "DT_WeaponCSBase", "m_flRecoilIndex" );
	  Displacement.DT_WeaponCSBase.m_weaponMode = pPropManager->GetOffset( "DT_WeaponCSBase", "m_weaponMode" );
	  Displacement.DT_WeaponCSBase.m_flPostponeFireReadyTime = pPropManager->GetOffset( "DT_WeaponCSBase", "m_flPostponeFireReadyTime" );
	  Displacement.DT_WeaponCSBase.m_fLastShotTime = pPropManager->GetOffset( "DT_WeaponCSBase", "m_fLastShotTime" );

	  Displacement.DT_WeaponCSBaseGun.m_zoomLevel = pPropManager->GetOffset( "DT_WeaponCSBaseGun", "m_zoomLevel" );

	  Displacement.DT_BaseCSGrenade.m_bPinPulled = pPropManager->GetOffset( "DT_BaseCSGrenade", "m_bPinPulled" );
	  Displacement.DT_BaseCSGrenade.m_fThrowTime = pPropManager->GetOffset( "DT_BaseCSGrenade", "m_fThrowTime" );
	  Displacement.DT_BaseCSGrenade.m_flThrowStrength = pPropManager->GetOffset( "DT_BaseCSGrenade", "m_flThrowStrength" );

	  Displacement.DT_BaseAttributableItem.m_flFallbackWear = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_flFallbackWear" );
	  Displacement.DT_BaseAttributableItem.m_nFallbackPaintKit = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_nFallbackPaintKit" );
	  Displacement.DT_BaseAttributableItem.m_nFallbackSeed = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_nFallbackSeed" );
	  Displacement.DT_BaseAttributableItem.m_nFallbackStatTrak = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_nFallbackStatTrak" );
	  Displacement.DT_BaseAttributableItem.m_OriginalOwnerXuidHigh = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh" );
	  Displacement.DT_BaseAttributableItem.m_OriginalOwnerXuidLow = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow" );
	  Displacement.DT_BaseAttributableItem.m_szCustomName = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_szCustomName" );
	  Displacement.DT_BaseAttributableItem.m_bInitialized = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_bInitialized" );
	  Displacement.DT_BaseAttributableItem.m_iAccountID = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_iAccountID" );
	  Displacement.DT_BaseAttributableItem.m_iEntityLevel = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_iEntityLevel" );
	  Displacement.DT_BaseAttributableItem.m_iEntityQuality = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_iEntityQuality" );
	  Displacement.DT_BaseAttributableItem.m_iItemDefinitionIndex = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_iItemDefinitionIndex" );
	  Displacement.DT_BaseAttributableItem.m_Item = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_Item" );
	  Displacement.DT_BaseAttributableItem.m_iItemIDLow = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_iItemIDLow" );
	  Displacement.DT_BaseAttributableItem.m_iItemIDHigh = pPropManager->GetOffset( "DT_BaseAttributableItem", "m_iItemIDHigh" );

	  Displacement.DT_BaseViewModel.m_hOwner = pPropManager->GetOffset( "DT_BaseViewModel", "m_hOwner" );
	  Displacement.DT_BaseViewModel.m_hWeapon = pPropManager->GetOffset( "DT_BaseViewModel", "m_hWeapon" );

	  Displacement.DT_SmokeGrenadeProjectile.m_nSmokeEffectTickBegin = pPropManager->GetOffset( "DT_SmokeGrenadeProjectile", "m_nSmokeEffectTickBegin" );

	  // hope this doesnt crash
	  // [Warning] Signature '80 BF ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? F3 0F 7E 87 ?? ?? ?? ??' not found (Horizon::Memory::Horizon::Memory::Scan)
	  // Displacement.DT_SmokeGrenadeProjectile.m_SmokeParticlesSpawned = *( int* ) ( Memory::Scan( image_client, "80 BF ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? F3 0F 7E 87 ?? ?? ?? ??" ) + 2 );

	  Displacement.CBoneMergeCache.m_nConstructor = Memory::Scan( image_client, XorStr( "56 8B F1 0F 57 C0 C7 86 ?? ?? ?? ?? ?? ?? ?? ??" ) );
	  Displacement.CBoneMergeCache.m_nInit = rel32_fix( Memory::Scan( image_client, XorStr( "E8 ?? ?? ?? ?? FF 75 08 8B 8E ?? ?? ?? ??" ) ) );
	  Displacement.CBoneMergeCache.m_nUpdateCache = rel32_fix( Memory::Scan( image_client, XorStr( "E8 ?? ?? ?? ?? 83 7E 10 00 74 64" ) ) );
	  Displacement.CBoneMergeCache.m_CopyToFollow = rel32_fix( Memory::Scan( image_client, XorStr( "E8 ?? ?? ?? ?? 8B 87 ?? ?? ?? ?? 8D 8C 24 ?? ?? ?? ?? 8B 7C 24 18" ) ) ); // E8 ? ? ? ? F3 0F 10 45 ? 8D 84 24 ? ? ? ?
	  Displacement.CBoneMergeCache.m_CopyFromFollow = rel32_fix( Memory::Scan( image_client, XorStr( "E8 ?? ?? ?? ?? F3 0F 10 45 ?? 8D 84 24 ?? ?? ?? ??" ) ) );

	  // old: 53 8B D9 F6 C3 03 74 0B FF 15 ?? ?? ?? ?? 84 C0 74 01 CC C7 83 ?? ?? ?? ?? ?? ?? ?? ?? 8B CB
	  Displacement.CIKContext.m_nConstructor = Memory::Scan( image_client, "56 8B F1 6A 00 6A 00 C7" );
	  Displacement.CIKContext.m_nDestructor = Memory::Scan( image_client, XorStr( "56 8B F1 57 8D 8E ?? ?? ?? ?? E8 ?? ?? ?? ?? 8D 8E ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 BE ?? ?? ?? ?? ??" ) );
	  Displacement.CIKContext.m_nInit = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D 8F" ) );
	  Displacement.CIKContext.m_nUpdateTargets = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F0 81 EC ?? ?? ?? ?? 33 D2" ) );
	  Displacement.CIKContext.m_nSolveDependencies = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F0 81 EC ?? ?? ?? ?? 8B 81" ) );

	 // Displacement.CIKContext.m_nDestructor = Memory::Scan( image_client, XorStr( "56 8B F1 57 8D 8E ?? ?? ?? ?? E8 ?? ?? ?? ?? 8D 8E ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 BE ?? ?? ?? ?? ??" ) );
	 // Displacement.CIKContext.m_nInit = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D" ) );
	 //
	 // Displacement.CIKContext.m_nUpdateTargets = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F0 81 EC 18 01 00 00 33" ) );
	 // Displacement.CIKContext.m_nSolveDependencies = Memory::Scan( image_client, XorStr( "E8 ? ? ? ? 8B 47 FC 8D 4F FC 8B 80" ) );
	 //
	  Displacement.CBoneSetup.InitPose = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 10 53 8B D9 89 55 F8 56 57 89 5D F4 8B 0B 89 4D F0" ) );
	  Displacement.CBoneSetup.AccumulatePose = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F0 B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? A1" ) ); // 55 8B EC 83 E4 F0 B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? A1 ?? ?? ?? ??
	  Displacement.CBoneSetup.CalcAutoplaySequences = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 10 53 56 57 8B 7D 10 8B D9 F3 0F 11 5D ??" ) );
	  Displacement.CBoneSetup.CalcBoneAdj = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 81 EC ?? ?? ?? ?? 8B C1 89 54 24 04 89 44 24 2C 56 57 8B ??" ) );


	  // jmp patterns is not very reliable
	  auto CL_Predict = Memory::Scan( image_engine, XorStr( "75 30 8B 87 ?? ?? ?? ??" ) );
	  auto CL_Move = Memory::Scan( image_engine, XorStr( "74 0F 80 BF ?? ?? ?? ?? ??" ) ); 
	  Displacement.CClientState.m_nLastCommandAck = *( int* ) ( CL_Predict + 0x20 );
	  Displacement.CClientState.m_nDeltaTick = *( int* ) ( CL_Predict + 0x10 );
	  Displacement.CClientState.m_nLastOutgoingCommand = *( int* ) ( CL_Predict + 0xA );
	  Displacement.CClientState.m_nChokedCommands = *( int* ) ( CL_Predict + 0x4 );
	  Displacement.CClientState.m_bIsHLTV = *( int* ) ( CL_Move + 0x4 );

	  Displacement.DT_PlantedC4.m_flC4Blow = pPropManager->GetOffset( "DT_PlantedC4", "m_flC4Blow" );

	  Displacement.Data.m_uMoveHelper = **( std::uintptr_t** )( Memory::Scan( image_client, XorStr( "8B 0D ?? ?? ?? ?? 8B 46 08 68" ) ) + 2 );
	  Displacement.Data.m_uInput = *( std::uintptr_t* )( Memory::Scan( image_client, XorStr( "B9 ?? ?? ?? ?? F3 0F 11 04 24 FF 50 10" ) ) + 1 );
	  Displacement.Data.m_uPredictionRandomSeed = *( std::uintptr_t* )( Memory::Scan( image_client, XorStr( "8B 0D ?? ?? ?? ?? BA ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 C4" ) ) + 2 );
	  Displacement.Data.m_uPredictionPlayer = *( std::uintptr_t* )( Memory::Scan( image_client, XorStr( "89 ?? ?? ?? ?? ?? F3 0F 10 48 20" ) ) + 2 );
	  Displacement.Data.m_uModelBoneCounter = *( std::uintptr_t* )( InvalidateBoneCache + 0xA );
	  Displacement.Data.m_uClientSideAnimationList = *( std::uintptr_t* )( Memory::Scan( image_client, XorStr( "A1 ?? ?? ?? ?? F6 44 F0 04 01 74 0B" ) ) + 1 );
	  Displacement.Data.m_uGlowObjectManager = *( std::uintptr_t* )( Memory::Scan( image_client, XorStr( "0F 11 05 ?? ?? ?? ?? 83 C8 01" ) ) + 3 );
	  Displacement.Data.m_uCamThink = ( std::uintptr_t )( Memory::Scan( image_client, XorStr( "85 C0 75 30 38 86" ) ) );
	  Displacement.Data.m_uRenderBeams = ( std::uintptr_t )( Memory::Scan( image_client, XorStr( "A1 ?? ?? ?? ?? FF 10 A1 ?? ?? ?? ?? B9" ) ) + 0x1 );
	  Displacement.Data.m_uSmokeCount = *( std::uintptr_t* )( Memory::Scan( image_client, "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0" ) + 0x8 );

	  Displacement.Data.m_uCenterPrint = ( std::uintptr_t )( Memory::Scan( image_client, XorStr( "8B 35 ? ? ? ? 8D 4C 24 20" ) ) + 0x2 );
	  Displacement.Data.m_uHostFrameTicks = ( Memory::Scan( image_engine, XorStr( "03 05 ? ? ? ? 83 CF 10" ) ) + 2 );
	  Displacement.Data.m_uServerGlobals = Memory::Scan( image_server, XorStr( "8B 15 ? ? ? ? 33 C9 83 7A 18 01" ) ) + 0x2;
	  Displacement.Data.m_uServerPoseParameters = Memory::Scan( image_server, XorStr( "8D 87 ? ? ? ? 89 46 08 C7 46 ? ? ? ? ? EB 06" ) ) + 0x2;

	  Displacement.Data.m_uServerAnimState = Memory::Scan( image_server, XorStr( "8B 8F ?? ?? ?? ?? 85 C9 74 06 56" ) ) + 2;
	  Displacement.Data.m_uTicksAllowed = Memory::Scan( image_server, XorStr( "FF 86 ?? ?? ?? ?? 8B CE 89 86 ?? ?? ?? ??" ) ) + 2;
	  Displacement.Data.m_uHudElement = Memory::Scan( image_client, XorStr( "B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B 5D 08" ) ) + 1;

	  Displacement.Data.m_uListLeavesInBoxReturn = Memory::Scan( image_client, "FF 52 18 8B 7D 08 8B" ) + 3;
	  Displacement.Data.s_bAllowExtrapolation = Memory::Scan( image_client, XorStr( "A2 ? ? ? ? 8B 45 E8" ) ) + 1;
	  Displacement.Data.m_FireBulletsReturn = Memory::Scan( image_client, XorStr( "3B 3D ?? ?? ?? ?? 75 4C" ) );
	  Displacement.Data.m_D3DDevice = Memory::Scan( image_shaderapidx9, XorStr( "A1 ?? ?? ?? ?? 50 8B 08 FF 51 0C" ) ) + 1;

	  Displacement.Data.m_SoundService = Memory::Scan( image_engine, XorStr( "B9 ? ? ? ? 80 65 FC FE 6A 00" ) );
	  Displacement.Data.m_InterpolateServerEntities = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 1C 8B 0D ? ? ? ? 53 56" ) ); // xref CVProfile::EnterScope(g_VProfCurrentProfile, XorStr("C_BaseEntity::InterpolateServerEntities"), 0, XorStr("Interpolation"), 0, 4);
	  Displacement.Data.m_SendNetMsg = Memory::Scan( image_engine, XorStr( "55 8B EC 56 8B F1 8B 86 ? ? ? ? 85 C0 74 24 48 83 F8 02 77 2C 83 BE ? ? ? ? ? 8D 8E ? ? ? ? 74 06 32 C0 84 C0 EB 10 E8 ? ? ? ? 84 C0 EB 07 83 BE ? ? ? ? ? 0F 94 C0 84 C0 74 07 B0 01 5E 5D C2 0C 00" ) ); // xref volume || ConVarRef %s doesn't point to an existing ConVar\n
	  Displacement.Data.m_ModifyEyePos = rel32_fix( Memory::Scan( image_client, XorStr( "E8 ? ? ? ? 8B 06 8B CE FF 90 ? ? ? ? 85 C0 74 4E" ) ) ); // xref head_0
	  Displacement.Data.m_ResetContentsCache = Memory::Scan( image_client, XorStr( "56 8D 51 3C BE" ) );
	  Displacement.Data.m_ProcessInterpolatedList = Memory::Scan( image_client, XorStr( "0F B7 05 ? ? ? ? 3D ? ? ? ? 74 3F" ) ); // xref C_BaseEntity::InterpolateServerEntities
	  Displacement.Data.CheckReceivingListReturn = *reinterpret_cast< DWORD32* >( Memory::Scan( image_engine, XorStr( "FF 50 34 8B 1D ? ? ? ? 85 C0 74 16 FF B6" ) ) + 0x3 );
	  Displacement.Data.ReadSubChannelDataReturn = *reinterpret_cast< DWORD32* >( Memory::Scan( image_engine, XorStr( "FF 50 34 85 C0 74 12 53 FF 75 0C 68" ) ) + 0x3 );

	  Displacement.Data.SendDatagram = Memory::Scan( image_engine, XorStr( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9 89 7C 24 18" ) );
	  Displacement.Data.ProcessPacket = Memory::Scan( image_engine, XorStr( "55 8B EC 83 E4 C0 81 EC ? ? ? ? 53 56 57 8B 7D 08 8B D9" ) );
	  Displacement.Data.m_GameRules = ( Memory::Scan( image_client, XorStr( "8B 0D ?? ?? ?? ?? 85 C0 74 0A 8B 01 FF 50 78 83 C0 54" ) ) + 2 );

	  Displacement.Function.m_uRandomSeed = ( std::uintptr_t )( GetProcAddress( image_vstdlib, "RandomSeed" ) );
	  Displacement.Function.m_uRandomFloat = ( std::uintptr_t )( GetProcAddress( image_vstdlib, "RandomFloat" ) );
	  Displacement.Function.m_uRandomInt = ( std::uintptr_t )( GetProcAddress( image_vstdlib, "RandomInt" ) );

	  Displacement.Function.m_uSetAbsOrigin = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8" ) );
	  Displacement.Function.m_uSetAbsAngles = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8" ) );
	  Displacement.Function.m_uIsBreakable = Memory::Scan( image_client, XorStr( "55 8B EC 51 56 8B F1 85 F6 74 68" ) ); //xref

	  // old: 55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C 89 5D FC
	  // [Warning] Signature '55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 34' not found (Horizon::Memory::Horizon::Memory::Scan)
	  //Displacement.Function.m_uClearHudWeaponIcon = Memory::Scan( image_client, "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 34" ); // hope it doesnt crash

	  Displacement.Function.m_uLoadNamedSkys = Memory::Scan( image_engine, XorStr( "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45" ) );
	  Displacement.Function.m_uCreateAnimState = Memory::Scan( image_client, XorStr( "55 8B EC 56 8B F1 B9 ?? ?? ?? ?? C7 46" ) );
	  Displacement.Function.m_uResetAnimState = Memory::Scan( image_client, XorStr( "56 6A 01 68 ?? ?? ?? ?? 8B F1" ) );
	  Displacement.Function.m_uUpdateAnimState = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24" ) );
	  Displacement.Function.m_uClanTagChange = Memory::Scan( image_engine, XorStr( "53 56 57 8B DA 8B F9 FF 15" ) );

	  Displacement.Function.m_uGetSequenceActivity = Memory::Scan( image_client, XorStr( "55 8B EC 83 7D 08 FF 56 8B F1 74 3D" ) );
	  Displacement.Function.m_uInvalidatePhysics = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56 83 E0 04" ) );
	  Displacement.Function.m_uPostThinkVPhysics = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 8B D9 56 57 83 BB ? ? ? ? ? 75 50 8B 0D" ) );
	  Displacement.Function.m_SimulatePlayerSimulatedEntities = Memory::Scan( image_client, XorStr( "56 8B F1 57 8B BE ?? ?? ?? ?? 83 EF 01 78 72 90 8B 86" ) );
	  Displacement.Function.m_uImplPhysicsRunThink = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 10 53 56 57 8B F9 8B 87 ?? ?? ?? ?? C1 E8 16" ) );

	  Displacement.Function.m_uClearDeathNotices = Memory::Scan( image_client, "E8 ? ? ? ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 8B F0 85 F6 74 19" ); // hope it doesnt crash // i will upd it soon

	  Displacement.Function.m_uSetTimeout = Memory::Scan( image_engine, XorStr( "55 8B EC 80 7D 0C 00 F3 0F 10 4D" ) );
	  Displacement.Function.m_uFindHudElement = Memory::Scan( image_client, XorStr( "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28" ) );
	  Displacement.Function.m_SetCollisionBounds = Memory::Scan( image_client, XorStr( "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 10 56 57 8B 7B" ) );
	  Displacement.Function.m_MD5PseudoRandom = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 83 EC 70 6A 58 8D 44 24 1C 89 4C 24 08 6A 00 50" ) );
	  Displacement.Function.m_WriteUsercmd = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D" ) );
	  Displacement.Function.m_StdStringAssign = Memory::Scan( image_engine, XorStr( "55 8B EC 53 8B 5D 08 56 8B F1 85 DB 74 57 8B 4E 14 83 F9 10 72 04 8B 06 EB 02" ) );
	  Displacement.Function.m_pPoseParameter = Memory::Scan( image_client, XorStr( "55 8B EC 8B 45 08 57 8B F9 8B 4F 04 85 C9 75 15 8B" ) ); // 55 8B EC 8B 45 08 57 8B F9 8B 4F 04 85 C9 75 15 8B
	  Displacement.Function.m_AttachmentHelper = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4" ) );
	  Displacement.Function.m_LockStudioHdr = rel32_fix( Memory::Scan( image_client, XorStr( "E8 ?? ?? ?? ?? 8D 47 FC" ) ) );
	  Displacement.Function.m_LineGoesThroughSmoke = Memory::Scan( image_client, XorStr( "55 8B EC 83 EC 08 8B 15 ?? ?? ?? ?? 0F 57 C0" ) );
	  Displacement.Function.m_TraceFilterSimple = Memory::Scan( image_client, XorStr( "55 8B EC 83 E4 F0 83 EC 7C 56 52" ) ) + 0x3D; //xref : offset ??_7CTraceFilterSimple@@6B@ ; const CTraceFilterSimple::`vftable' 
   }

#else
   void Parse( void* reserved ) {
	  VMP_BEGIN_ULTRA( );
	  DllArgsToRecieve* transmitted_data = ( DllArgsToRecieve* ) reserved;
	  DllInitializeData* parsedData = ( DllInitializeData* ) transmitted_data->lpInitStruct;
	  auto& pPropManager = PropManager::Instance( );
	  auto InitData = &Displacement;
	  auto image_client = ( std::uintptr_t )GetModuleHandleA( XorStr( "client.dll" ) );
	  auto image_engine = ( std::uintptr_t )GetModuleHandleA( XorStr( "engine.dll" ) );

	  InitData->C_BaseEntity.m_MoveType = XOR_DWORD( parsedData->C_BaseEntity.m_MoveType );
	  InitData->C_BaseEntity.m_rgflCoordinateFrame = XOR_DWORD( parsedData->C_BaseEntity.m_rgflCoordinateFrame );

	  InitData->DT_BaseEntity.m_iTeamNum = XOR_DWORD( parsedData->DT_BaseEntity.m_iTeamNum );
	  InitData->DT_BaseEntity.m_vecOrigin = XOR_DWORD( parsedData->DT_BaseEntity.m_vecOrigin );
	  InitData->DT_BaseEntity.m_flSimulationTime = XOR_DWORD( parsedData->DT_BaseEntity.m_flSimulationTime );
	  InitData->DT_BaseEntity.m_fEffects = XOR_DWORD( parsedData->DT_BaseEntity.m_fEffects );
	  InitData->DT_BaseEntity.m_iEFlags = XOR_DWORD( parsedData->DT_BaseEntity.m_iEFlags );
	  InitData->DT_BaseEntity.m_hOwnerEntity = XOR_DWORD( parsedData->DT_BaseEntity.m_hOwnerEntity );
	  InitData->DT_BaseEntity.m_nModelIndex = XOR_DWORD( parsedData->DT_BaseEntity.m_nModelIndex );
	  InitData->DT_BaseEntity.m_Collision = XOR_DWORD( parsedData->DT_BaseEntity.m_Collision );

	  InitData->C_BaseAnimating.m_BoneAccessor = XOR_DWORD( parsedData->C_BaseAnimating.m_BoneAccessor );
	  InitData->C_BaseAnimating.m_iMostRecentModelBoneCounter = XOR_DWORD( parsedData->C_BaseAnimating.m_iMostRecentModelBoneCounter );
	  InitData->C_BaseAnimating.m_flLastBoneSetupTime = XOR_DWORD( parsedData->C_BaseAnimating.m_flLastBoneSetupTime );
	  InitData->C_BaseAnimating.m_CachedBoneData = XOR_DWORD( parsedData->C_BaseAnimating.m_CachedBoneData );
	  InitData->C_BaseAnimating.m_AnimOverlay = XOR_DWORD( parsedData->C_BaseAnimating.m_AnimOverlay );

	  InitData->C_BaseAnimating.m_pFirstBoneSnapshot = XOR_DWORD( parsedData->C_BaseAnimating.m_pFirstBoneSnapshot );
	  InitData->C_BaseAnimating.m_pSecondBoneSnapshot = XOR_DWORD( parsedData->C_BaseAnimating.m_pSecondBoneSnapshot );

	  InitData->C_BaseAnimating.m_nCachedBonesPosition = XOR_DWORD( parsedData->C_BaseAnimating.m_nCachedBonesPosition );
	  InitData->C_BaseAnimating.m_nCachedBonesRotation = XOR_DWORD( parsedData->C_BaseAnimating.m_nCachedBonesRotation );
	  InitData->C_BaseAnimating.m_pStudioHdr = XOR_DWORD( parsedData->C_BaseAnimating.m_pStudioHdr );
	  InitData->C_BaseAnimating.m_bShouldDraw = XOR_DWORD( parsedData->C_BaseAnimating.m_bShouldDraw );
	  InitData->C_BaseAnimating.m_pBoneMerge = XOR_DWORD( parsedData->C_BaseAnimating.m_pBoneMerge );
	  InitData->C_BaseAnimating.m_pIk = XOR_DWORD( parsedData->C_BaseAnimating.m_pIk );

	  InitData->DT_BaseAnimating.m_bClientSideAnimation = XOR_DWORD( parsedData->DT_BaseAnimating.m_bClientSideAnimation );
	  InitData->DT_BaseAnimating.m_flPoseParameter = XOR_DWORD( parsedData->DT_BaseAnimating.m_flPoseParameter );
	  InitData->DT_BaseAnimating.m_nHitboxSet = XOR_DWORD( parsedData->DT_BaseAnimating.m_nHitboxSet );

	  InitData->DT_BaseCombatCharacter.m_hActiveWeapon = XOR_DWORD( parsedData->DT_BaseCombatCharacter.m_hActiveWeapon );
	  InitData->DT_BaseCombatCharacter.m_flNextAttack = XOR_DWORD( parsedData->DT_BaseCombatCharacter.m_flNextAttack );
	  InitData->DT_BaseCombatCharacter.m_hMyWeapons = XOR_DWORD( parsedData->DT_BaseCombatCharacter.m_hMyWeapons );
	  InitData->DT_BaseCombatCharacter.m_hMyWearables = XOR_DWORD( parsedData->DT_BaseCombatCharacter.m_hMyWearables );

	  InitData->C_BasePlayer.m_pCurrentCommand = XOR_DWORD( parsedData->C_BasePlayer.m_pCurrentCommand );
	  InitData->C_BasePlayer.UpdateVisibilityAllEntities = XOR_DWORD( parsedData->C_BasePlayer.UpdateVisibilityAllEntities );

	  InitData->DT_BasePlayer.m_aimPunchAngle = XOR_DWORD( parsedData->DT_BasePlayer.m_aimPunchAngle );
	  InitData->DT_BasePlayer.m_aimPunchAngleVel = XOR_DWORD( parsedData->DT_BasePlayer.m_aimPunchAngleVel );
	  InitData->DT_BasePlayer.m_viewPunchAngle = XOR_DWORD( parsedData->DT_BasePlayer.m_viewPunchAngle );
	  InitData->DT_BasePlayer.m_vecViewOffset = XOR_DWORD( parsedData->DT_BasePlayer.m_vecViewOffset );
	  InitData->DT_BasePlayer.m_vecVelocity = XOR_DWORD( parsedData->DT_BasePlayer.m_vecVelocity );
	  InitData->DT_BasePlayer.m_vecBaseVelocity = XOR_DWORD( parsedData->DT_BasePlayer.m_vecBaseVelocity );
	  InitData->DT_BasePlayer.m_flFallVelocity = XOR_DWORD( parsedData->DT_BasePlayer.m_flFallVelocity );
	  InitData->DT_BasePlayer.m_flDuckAmount = XOR_DWORD( parsedData->DT_BasePlayer.m_flDuckAmount );
	  InitData->DT_BasePlayer.m_flDuckSpeed = XOR_DWORD( parsedData->DT_BasePlayer.m_flDuckSpeed );
	  InitData->DT_BasePlayer.m_lifeState = XOR_DWORD( parsedData->DT_BasePlayer.m_lifeState );
	  InitData->DT_BasePlayer.m_nTickBase = XOR_DWORD( parsedData->DT_BasePlayer.m_nTickBase );
	  InitData->DT_BasePlayer.m_iHealth = XOR_DWORD( parsedData->DT_BasePlayer.m_iHealth );
	  InitData->DT_BasePlayer.m_iDefaultFOV = XOR_DWORD( parsedData->DT_BasePlayer.m_iDefaultFOV );
	  InitData->DT_BasePlayer.m_fFlags = XOR_DWORD( parsedData->DT_BasePlayer.m_fFlags );
	  InitData->DT_BasePlayer.m_iObserverMode = XOR_DWORD( parsedData->DT_BasePlayer.m_iObserverMode );
	  InitData->DT_BasePlayer.pl = XOR_DWORD( parsedData->DT_BasePlayer.pl );
	  InitData->DT_BasePlayer.m_hObserverTarget = XOR_DWORD( parsedData->DT_BasePlayer.m_hObserverTarget );
	  InitData->DT_BasePlayer.m_hViewModel = XOR_DWORD( parsedData->DT_BasePlayer.m_hViewModel );
	  InitData->DT_BasePlayer.m_vphysicsCollisionState = XOR_DWORD( parsedData->DT_BasePlayer.m_vphysicsCollisionState );
	  InitData->DT_BasePlayer.m_ubEFNoInterpParity = XOR_DWORD( parsedData->DT_BasePlayer.m_ubEFNoInterpParity );
	  InitData->DT_BasePlayer.m_ubOldEFNoInterpParity = XOR_DWORD( parsedData->DT_BasePlayer.m_ubOldEFNoInterpParity );

	  InitData->C_CSPlayer.m_PlayerAnimState = XOR_DWORD( parsedData->C_CSPlayer.m_PlayerAnimState );
	  InitData->C_CSPlayer.m_flSpawnTime = XOR_DWORD( parsedData->C_CSPlayer.m_flSpawnTime );

	  InitData->DT_CSPlayer.m_angEyeAngles = XOR_DWORD( parsedData->DT_CSPlayer.m_angEyeAngles );
	  InitData->DT_CSPlayer.m_nSurvivalTeam = XOR_DWORD( parsedData->DT_CSPlayer.m_nSurvivalTeam );
	  InitData->DT_CSPlayer.m_bHasHelmet = XOR_DWORD( parsedData->DT_CSPlayer.m_bHasHelmet );
	  InitData->DT_CSPlayer.m_bHasHeavyArmor = XOR_DWORD( parsedData->DT_CSPlayer.m_bHasHeavyArmor );
	  InitData->DT_CSPlayer.m_ArmorValue = XOR_DWORD( parsedData->DT_CSPlayer.m_ArmorValue );
	  InitData->DT_CSPlayer.m_iMoveState = XOR_DWORD( parsedData->DT_CSPlayer.m_iMoveState );
	  InitData->DT_CSPlayer.m_bScoped = XOR_DWORD( parsedData->DT_CSPlayer.m_bScoped );
	  InitData->DT_CSPlayer.m_bIsWalking = XOR_DWORD( parsedData->DT_CSPlayer.m_bIsWalking );
	  InitData->DT_CSPlayer.m_iAccount = XOR_DWORD( parsedData->DT_CSPlayer.m_iAccount );
	  InitData->DT_CSPlayer.m_iShotsFired = XOR_DWORD( parsedData->DT_CSPlayer.m_iShotsFired );
	  InitData->DT_CSPlayer.m_flFlashDuration = XOR_DWORD( parsedData->DT_CSPlayer.m_flFlashDuration );
	  InitData->DT_CSPlayer.m_flLowerBodyYawTarget = XOR_DWORD( parsedData->DT_CSPlayer.m_flLowerBodyYawTarget );
	  InitData->DT_CSPlayer.m_flVelocityModifier = XOR_DWORD( parsedData->DT_CSPlayer.m_flVelocityModifier );
	  InitData->DT_CSPlayer.m_bGunGameImmunity = XOR_DWORD( parsedData->DT_CSPlayer.m_bGunGameImmunity );
	  InitData->DT_CSPlayer.m_flHealthShotBoostExpirationTime = XOR_DWORD( parsedData->DT_CSPlayer.m_flHealthShotBoostExpirationTime );
	  InitData->DT_CSPlayer.m_iMatchStats_Kills = XOR_DWORD( parsedData->DT_CSPlayer.m_iMatchStats_Kills );
	  InitData->DT_CSPlayer.m_iMatchStats_Deaths = XOR_DWORD( parsedData->DT_CSPlayer.m_iMatchStats_Deaths );
	  InitData->DT_CSPlayer.m_iMatchStats_HeadShotKills = XOR_DWORD( parsedData->DT_CSPlayer.m_iMatchStats_HeadShotKills );

	  InitData->DT_CSRagdoll.m_hPlayer = XOR_DWORD( parsedData->DT_CSRagdoll.m_hPlayer );

	  InitData->DT_BaseCombatWeapon.m_flNextPrimaryAttack = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_flNextPrimaryAttack );
	  InitData->DT_BaseCombatWeapon.m_flNextSecondaryAttack = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_flNextSecondaryAttack );
	  InitData->DT_BaseCombatWeapon.m_hOwner = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_hOwner );
	  InitData->DT_BaseCombatWeapon.m_iClip1 = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_iClip1 );
	  InitData->DT_BaseCombatWeapon.m_iPrimaryReserveAmmoCount = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_iPrimaryReserveAmmoCount );
	  InitData->DT_BaseCombatWeapon.m_hWeaponWorldModel = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_hWeaponWorldModel );
	  InitData->DT_BaseCombatWeapon.m_iWorldModelIndex = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_iWorldModelIndex );
	  InitData->DT_BaseCombatWeapon.m_iWorldDroppedModelIndex = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_iWorldDroppedModelIndex );
	  InitData->DT_BaseCombatWeapon.m_iViewModelIndex = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_iViewModelIndex );
	  InitData->DT_BaseCombatWeapon.m_CustomMaterials = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_CustomMaterials );
	  InitData->DT_BaseCombatWeapon.m_bCustomMaterialInitialized = XOR_DWORD( parsedData->DT_BaseCombatWeapon.m_bCustomMaterialInitialized );

	  InitData->DT_WeaponCSBase.m_flRecoilIndex = XOR_DWORD( parsedData->DT_WeaponCSBase.m_flRecoilIndex );
	  InitData->DT_WeaponCSBase.m_weaponMode = XOR_DWORD( parsedData->DT_WeaponCSBase.m_weaponMode );
	  InitData->DT_WeaponCSBase.m_flPostponeFireReadyTime = XOR_DWORD( parsedData->DT_WeaponCSBase.m_flPostponeFireReadyTime );
	  InitData->DT_WeaponCSBase.m_fLastShotTime = XOR_DWORD( parsedData->DT_WeaponCSBase.m_fLastShotTime );

	  InitData->DT_WeaponCSBaseGun.m_zoomLevel = XOR_DWORD( parsedData->DT_WeaponCSBaseGun.m_zoomLevel );

	  InitData->DT_BaseCSGrenade.m_bPinPulled = XOR_DWORD( parsedData->DT_BaseCSGrenade.m_bPinPulled );
	  InitData->DT_BaseCSGrenade.m_fThrowTime = XOR_DWORD( parsedData->DT_BaseCSGrenade.m_fThrowTime );
	  InitData->DT_BaseCSGrenade.m_flThrowStrength = XOR_DWORD( parsedData->DT_BaseCSGrenade.m_flThrowStrength );

	  InitData->DT_BaseAttributableItem.m_flFallbackWear = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_flFallbackWear );
	  InitData->DT_BaseAttributableItem.m_nFallbackPaintKit = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_nFallbackPaintKit );
	  InitData->DT_BaseAttributableItem.m_nFallbackSeed = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_nFallbackSeed );
	  InitData->DT_BaseAttributableItem.m_nFallbackStatTrak = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_nFallbackStatTrak );
	  InitData->DT_BaseAttributableItem.m_OriginalOwnerXuidHigh = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_OriginalOwnerXuidHigh );
	  InitData->DT_BaseAttributableItem.m_OriginalOwnerXuidLow = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_OriginalOwnerXuidLow );
	  InitData->DT_BaseAttributableItem.m_szCustomName = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_szCustomName );
	  InitData->DT_BaseAttributableItem.m_bInitialized = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_bInitialized );
	  InitData->DT_BaseAttributableItem.m_iAccountID = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_iAccountID );
	  InitData->DT_BaseAttributableItem.m_iEntityLevel = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_iEntityLevel );
	  InitData->DT_BaseAttributableItem.m_iEntityQuality = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_iEntityQuality );
	  InitData->DT_BaseAttributableItem.m_iItemDefinitionIndex = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_iItemDefinitionIndex );
	  InitData->DT_BaseCombatWeapon.m_iItemDefinitionIndex = InitData->DT_BaseAttributableItem.m_iItemDefinitionIndex;
	  InitData->DT_BaseAttributableItem.m_Item = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_Item );
	  InitData->DT_BaseAttributableItem.m_iItemIDLow = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_iItemIDLow );
	  InitData->DT_BaseAttributableItem.m_iItemIDHigh = XOR_DWORD( parsedData->DT_BaseAttributableItem.m_iItemIDHigh );

	  InitData->DT_BaseViewModel.m_hOwner = XOR_DWORD( parsedData->DT_BaseViewModel.m_hOwner );
	  InitData->DT_BaseViewModel.m_hWeapon = XOR_DWORD( parsedData->DT_BaseViewModel.m_hWeapon );

	  InitData->DT_SmokeGrenadeProjectile.m_nSmokeEffectTickBegin = XOR_DWORD( parsedData->DT_SmokeGrenadeProjectile.m_nSmokeEffectTickBegin );
	  InitData->DT_SmokeGrenadeProjectile.m_SmokeParticlesSpawned = XOR_DWORD( parsedData->DT_SmokeGrenadeProjectile.m_SmokeParticlesSpawned );

	  InitData->CBoneMergeCache.m_nConstructor = XOR_DWORD( parsedData->CBoneMergeCache.m_nConstructor );
	  InitData->CBoneMergeCache.m_nInit = XOR_DWORD( parsedData->CBoneMergeCache.m_nInit );
	  InitData->CBoneMergeCache.m_nUpdateCache = XOR_DWORD( parsedData->CBoneMergeCache.m_nUpdateCache );
	  InitData->CBoneMergeCache.m_CopyToFollow = XOR_DWORD( parsedData->CBoneMergeCache.m_CopyToFollow );
	  InitData->CBoneMergeCache.m_CopyFromFollow = XOR_DWORD( parsedData->CBoneMergeCache.m_CopyFromFollow );

	  InitData->CIKContext.m_nConstructor = XOR_DWORD( parsedData->CIKContext.m_nConstructor );
	  InitData->CIKContext.m_nInit = XOR_DWORD( parsedData->CIKContext.m_nInit );
	  InitData->CIKContext.m_nDestructor = XOR_DWORD( parsedData->CIKContext.m_nDestructor );
	  InitData->CIKContext.m_nUpdateTargets = XOR_DWORD( parsedData->CIKContext.m_nUpdateTargets );
	  InitData->CIKContext.m_nSolveDependencies = XOR_DWORD( parsedData->CIKContext.m_nSolveDependencies );

	  InitData->CBoneSetup.InitPose = XOR_DWORD( parsedData->CBoneSetup.InitPose );
	  InitData->CBoneSetup.AccumulatePose = XOR_DWORD( parsedData->CBoneSetup.AccumulatePose );
	  InitData->CBoneSetup.CalcAutoplaySequences = XOR_DWORD( parsedData->CBoneSetup.CalcAutoplaySequences );
	  InitData->CBoneSetup.CalcBoneAdj = XOR_DWORD( parsedData->CBoneSetup.CalcBoneAdj );

	  InitData->CClientState.m_nLastCommandAck = XOR_DWORD( parsedData->CClientState.m_nLastCommandAck );
	  InitData->CClientState.m_nDeltaTick = XOR_DWORD( parsedData->CClientState.m_nDeltaTick );
	  InitData->CClientState.m_nLastOutgoingCommand = XOR_DWORD( parsedData->CClientState.m_nLastOutgoingCommand );
	  InitData->CClientState.m_nChokedCommands = XOR_DWORD( parsedData->CClientState.m_nChokedCommands );
	  InitData->CClientState.m_bIsHLTV = XOR_DWORD( parsedData->CClientState.m_bIsHLTV );

	  InitData->DT_PlantedC4.m_flC4Blow = XOR_DWORD( parsedData->DT_PlantedC4.m_flC4Blow );

	  InitData->Data.m_uMoveHelper = XOR_DWORD( parsedData->Data.m_uMoveHelper );
	  InitData->Data.m_uInput = XOR_DWORD( parsedData->Data.m_uInput );
	  InitData->Data.m_uPredictionRandomSeed = XOR_DWORD( parsedData->Data.m_uPredictionRandomSeed );
	  InitData->Data.m_uPredictionPlayer = XOR_DWORD( parsedData->Data.m_uPredictionPlayer );
	  InitData->Data.m_uModelBoneCounter = XOR_DWORD( parsedData->Data.m_uModelBoneCounter );
	  InitData->Data.m_uClientSideAnimationList = XOR_DWORD( parsedData->Data.m_uClientSideAnimationList );
	  InitData->Data.m_uGlowObjectManager = XOR_DWORD( parsedData->Data.m_uGlowObjectManager );
	  InitData->Data.m_uCamThink = XOR_DWORD( parsedData->Data.m_uCamThink );
	  InitData->Data.m_uRenderBeams = XOR_DWORD( parsedData->Data.m_uRenderBeams );
	  InitData->Data.m_uSmokeCount = XOR_DWORD( parsedData->Data.m_uSmokeCount );
	  InitData->Data.m_uCenterPrint = XOR_DWORD( parsedData->Data.m_uCenterPrint );
	  InitData->Data.m_uHostFrameTicks = XOR_DWORD( parsedData->Data.m_uHostFrameTicks );
	  InitData->Data.m_uServerGlobals = XOR_DWORD( parsedData->Data.m_uServerGlobals );
	  InitData->Data.m_uServerPoseParameters = XOR_DWORD( parsedData->Data.m_uServerPoseParameters );
	  InitData->Data.m_uServerAnimState = XOR_DWORD( parsedData->Data.m_uServerAnimState );
	  InitData->Data.m_uTicksAllowed = XOR_DWORD( parsedData->Data.m_uTicksAllowed );
	  InitData->Data.m_uHudElement = XOR_DWORD( parsedData->Data.m_uHudElement );
	  InitData->Data.m_uListLeavesInBoxReturn = XOR_DWORD( parsedData->Data.m_uListLeavesInBoxReturn );
	  InitData->Data.s_bAllowExtrapolation = XOR_DWORD( parsedData->Data.s_bAllowExtrapolation );
	  InitData->Data.m_FireBulletsReturn = XOR_DWORD( parsedData->Data.m_FireBulletsReturn );
	  InitData->Data.m_D3DDevice = XOR_DWORD( parsedData->Data.m_D3DDevice );
	  InitData->Data.m_SoundService = XOR_DWORD( parsedData->Data.m_SoundService );
	  InitData->Data.m_InterpolateServerEntities = XOR_DWORD( parsedData->Data.m_InterpolateServerEntities );
	  InitData->Data.m_SendNetMsg = XOR_DWORD( parsedData->Data.m_SendNetMsg );
	  InitData->Data.m_ModifyEyePos = XOR_DWORD( parsedData->Data.m_ModifyEyePos );
	  InitData->Data.m_ResetContentsCache = XOR_DWORD( parsedData->Data.m_ResetContentsCache );
	  InitData->Data.m_ProcessInterpolatedList = XOR_DWORD( parsedData->Data.m_ProcessInterpolatedList );
	  InitData->Data.CheckReceivingListReturn = XOR_DWORD( parsedData->Data.CheckReceivingListReturn );
	  InitData->Data.ReadSubChannelDataReturn = XOR_DWORD( parsedData->Data.ReadSubChannelDataReturn );
	  InitData->Data.SendDatagram = XOR_DWORD( parsedData->Data.SendDatagram );
	  InitData->Data.ProcessPacket = XOR_DWORD( parsedData->Data.ProcessPacket );

	  InitData->Function.m_uRandomSeed = XOR_DWORD( parsedData->Function.m_uRandomSeed );
	  InitData->Function.m_uRandomFloat = XOR_DWORD( parsedData->Function.m_uRandomFloat );
	  InitData->Function.m_uRandomInt = XOR_DWORD( parsedData->Function.m_uRandomInt );
	  InitData->Function.m_uSetAbsOrigin = XOR_DWORD( parsedData->Function.m_uSetAbsOrigin );
	  InitData->Function.m_uSetAbsAngles = XOR_DWORD( parsedData->Function.m_uSetAbsAngles );
	  InitData->Function.m_uIsBreakable = XOR_DWORD( parsedData->Function.m_uIsBreakable );
	  InitData->Function.m_uClearHudWeaponIcon = XOR_DWORD( parsedData->Function.m_uClearHudWeaponIcon );
	  InitData->Function.m_uLoadNamedSkys = Memory::Scan( image_engine, "55 8B EC 81 EC ?? ?? ?? ?? 56 57 8B F9 C7 45" );//XOR_DWORD( parsedData->Function.m_uLoadNamedSkys );
	  InitData->Function.m_uCreateAnimState = XOR_DWORD( parsedData->Function.m_uCreateAnimState );
	  InitData->Function.m_uResetAnimState = XOR_DWORD( parsedData->Function.m_uResetAnimState );
	  InitData->Function.m_uUpdateAnimState = XOR_DWORD( parsedData->Function.m_uUpdateAnimState );
	  InitData->Function.m_uClanTagChange = Memory::Scan( image_engine, "53 56 57 8B DA 8B F9 FF 15" );//XOR_DWORD( parsedData->Function.m_uClanTagChange );
	  InitData->Function.m_uGetSequenceActivity = XOR_DWORD( parsedData->Function.m_uGetSequenceActivity );
	  InitData->Function.m_uInvalidatePhysics = XOR_DWORD( parsedData->Function.m_uInvalidatePhysics );
	  InitData->Function.m_uPostThinkVPhysics = XOR_DWORD( parsedData->Function.m_uPostThinkVPhysics );
	  InitData->Function.m_SimulatePlayerSimulatedEntities = XOR_DWORD( parsedData->Function.m_SimulatePlayerSimulatedEntities );
	  InitData->Function.m_uImplPhysicsRunThink = XOR_DWORD( parsedData->Function.m_uImplPhysicsRunThink );
	  InitData->Function.m_SetCollisionBounds = XOR_DWORD( parsedData->Function.m_SetCollisionBounds );
	  InitData->Function.m_MD5PseudoRandom = XOR_DWORD( parsedData->Function.m_MD5PseudoRandom );
	  InitData->Function.m_WriteUsercmd = XOR_DWORD( parsedData->Function.m_WriteUsercmd );
	  InitData->Function.m_StdStringAssign = XOR_DWORD( parsedData->Function.m_StdStringAssign );
	  InitData->Function.m_pPoseParameter = XOR_DWORD( parsedData->Function.m_pPoseParameter );
	  InitData->Function.m_AttachmentHelper = XOR_DWORD( parsedData->Function.m_AttachmentHelper );
	  InitData->Function.m_LockStudioHdr = XOR_DWORD( parsedData->Function.m_LockStudioHdr );
	  InitData->Function.m_LineGoesThroughSmoke = XOR_DWORD( parsedData->Function.m_LineGoesThroughSmoke );

	  //new
	  InitData->Function.m_uClearDeathNotices = XOR_DWORD( parsedData->Function.m_uClearDeathNotices );
	  InitData->Function.m_uSetTimeout = XOR_DWORD( parsedData->Function.m_uSetTimeout );
	  InitData->Function.m_TraceFilterSimple = XOR_DWORD( parsedData->Function.m_TraceFilterSimple );

	  InitData->Function.m_uFindHudElement = XOR_DWORD( parsedData->Function.m_uFindHudElement );
	  InitData->DT_CSPlayer.m_bWaitForNoAttack = XOR_DWORD( parsedData->DT_CSPlayer.m_bWaitForNoAttack );
	  InitData->DT_CSPlayer.m_bCustomPlayer = XOR_DWORD( parsedData->DT_CSPlayer.m_bCustomPlayer );

	  InitData->DT_BaseEntity.m_flAnimTime = XOR_DWORD( parsedData->DT_BaseEntity.m_flAnimTime );
	  InitData->DT_BaseAnimating.m_flCycle = XOR_DWORD( parsedData->DT_BaseAnimating.m_flCycle );
	  InitData->DT_BaseAnimating.m_nSequence = XOR_DWORD( parsedData->DT_BaseAnimating.m_nSequence );
	  InitData->DT_BaseAnimating.m_flEncodedController = XOR_DWORD( parsedData->DT_BaseAnimating.m_flEncodedController );

	  InitData->DT_CSPlayer.m_iPlayerState = XOR_DWORD( parsedData->DT_CSPlayer.m_iPlayerState );
	  InitData->DT_CSPlayer.m_bIsDefusing = XOR_DWORD( parsedData->DT_CSPlayer.m_bIsDefusing );
	  InitData->Data.m_GameRules = XOR_DWORD( parsedData->Data.m_GameRules );

	  VMP_END( );
   }
#endif

#define stringfy(s)      #s
#define dump_pointer(name) printf("%s: 0x%p\n", stringfy(name), (name))

#if 0
   void DumpOffsets( void* data ) {
	  dump_pointer( Displacement.DT_PlantedC4.m_flC4Blow );
	  dump_pointer( Displacement.DT_BaseViewModel.m_hOwner );
	  dump_pointer( Displacement.DT_BaseViewModel.m_hWeapon );
	  dump_pointer( Displacement.DT_SmokeGrenadeProjectile.m_nSmokeEffectTickBegin );
	  dump_pointer( Displacement.DT_SmokeGrenadeProjectile.m_SmokeParticlesSpawned );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_flFallbackWear );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_nFallbackPaintKit );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_nFallbackSeed );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_nFallbackStatTrak );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_OriginalOwnerXuidHigh );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_OriginalOwnerXuidLow );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_szCustomName );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_bInitialized );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_iAccountID );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_iEntityLevel );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_iEntityQuality );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_iItemDefinitionIndex );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_Item );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_iItemIDLow );
	  dump_pointer( Displacement.DT_BaseAttributableItem.m_iItemIDHigh );
	  dump_pointer( Displacement.DT_BaseCombatWeapon.m_flNextPrimaryAttack );
	  dump_pointer( Displacement.DT_BaseCombatWeapon.m_flNextSecondaryAttack );
	  dump_pointer( Displacement.DT_BaseCombatWeapon.m_hOwner );
	  dump_pointer( Displacement.DT_BaseCombatWeapon.m_iClip1 );
	  dump_pointer( Displacement.DT_BaseCombatWeapon.m_iPrimaryReserveAmmoCount );
	  dump_pointer( Displacement.DT_BaseCombatWeapon.m_iItemDefinitionIndex );
	  dump_pointer( Displacement.DT_BaseCombatWeapon.m_hWeaponWorldModel );
	  dump_pointer( Displacement.DT_BaseCombatWeapon.m_iWorldModelIndex );
	  dump_pointer( Displacement.DT_BaseCombatWeapon.m_iWorldDroppedModelIndex );
	  dump_pointer( Displacement.DT_BaseCombatWeapon.m_iViewModelIndex );
	  dump_pointer( Displacement.DT_WeaponCSBase.m_flRecoilIndex );
	  dump_pointer( Displacement.DT_WeaponCSBase.m_weaponMode );
	  dump_pointer( Displacement.DT_WeaponCSBase.m_fLastShotTime );
	  dump_pointer( Displacement.DT_WeaponCSBaseGun.m_zoomLevel );
	  dump_pointer( Displacement.DT_BaseCSGrenade.m_bPinPulled );
	  dump_pointer( Displacement.DT_BaseCSGrenade.m_fThrowTime );
	  dump_pointer( Displacement.DT_BaseCSGrenade.m_flThrowStrength );
	  dump_pointer( Displacement.DT_CSPlayer.m_angEyeAngles );
	  dump_pointer( Displacement.DT_CSPlayer.m_nSurvivalTeam );
	  dump_pointer( Displacement.DT_CSPlayer.m_bHasHelmet );
	  dump_pointer( Displacement.DT_CSPlayer.m_bHasHeavyArmor );
	  dump_pointer( Displacement.DT_CSPlayer.m_ArmorValue );
	  dump_pointer( Displacement.DT_CSPlayer.m_bScoped );
	  dump_pointer( Displacement.DT_CSPlayer.m_iAccount );
	  dump_pointer( Displacement.DT_CSPlayer.m_iShotsFired );
	  dump_pointer( Displacement.DT_CSPlayer.m_flFlashDuration );
	  dump_pointer( Displacement.DT_CSPlayer.m_flLowerBodyYawTarget );
	  dump_pointer( Displacement.DT_CSPlayer.m_flVelocityModifier );
	  dump_pointer( Displacement.DT_CSPlayer.m_bGunGameImmunity );
	  dump_pointer( Displacement.DT_CSPlayer.m_flHealthShotBoostExpirationTime );
	  dump_pointer( Displacement.DT_CSPlayer.m_iMatchStats_Kills );
	  dump_pointer( Displacement.DT_CSPlayer.m_iMatchStats_Deaths );
	  dump_pointer( Displacement.DT_CSPlayer.m_iMatchStats_HeadShotKills );
	  dump_pointer( Displacement.DT_CSPlayer.m_bWaitForNoAttack );
	  dump_pointer( Displacement.DT_CSPlayer.m_bCustomPlayer );
	  dump_pointer( Displacement.DT_CSRagdoll.m_hPlayer );
	  dump_pointer( Displacement.C_BaseEntity.m_MoveType );
	  dump_pointer( Displacement.C_BaseEntity.m_rgflCoordinateFrame );
	  dump_pointer( Displacement.DT_BaseEntity.m_iTeamNum );
	  dump_pointer( Displacement.DT_BaseEntity.m_vecOrigin );
	  dump_pointer( Displacement.DT_BaseEntity.m_flSimulationTime );
	  dump_pointer( Displacement.DT_BaseEntity.m_fEffects );
	  dump_pointer( Displacement.DT_BaseEntity.m_iEFlags );
	  dump_pointer( Displacement.DT_BaseEntity.m_hOwnerEntity );
	  dump_pointer( Displacement.DT_BaseEntity.m_nModelIndex );
	  dump_pointer( Displacement.DT_BaseEntity.m_Collision );
	  dump_pointer( Displacement.C_BaseAnimating.m_BoneAccessor );
	  dump_pointer( Displacement.C_BaseAnimating.m_iMostRecentModelBoneCounter );
	  dump_pointer( Displacement.C_BaseAnimating.m_flLastBoneSetupTime );
	  dump_pointer( Displacement.C_BaseAnimating.m_CachedBoneData );
	  dump_pointer( Displacement.C_BaseAnimating.m_AnimOverlay );
	  dump_pointer( Displacement.C_BaseAnimating.m_pFirstBoneSnapshot );
	  dump_pointer( Displacement.C_BaseAnimating.m_pSecondBoneSnapshot );
	  dump_pointer( Displacement.C_BaseAnimating.m_pBoneMerge );
	  dump_pointer( Displacement.C_BaseAnimating.m_pIk );
	  dump_pointer( Displacement.C_BaseAnimating.m_nCachedBonesPosition );
	  dump_pointer( Displacement.C_BaseAnimating.m_nCachedBonesRotation );
	  dump_pointer( Displacement.C_BaseAnimating.m_pStudioHdr );
	  dump_pointer( Displacement.C_BaseAnimating.m_bShouldDraw );
	  dump_pointer( Displacement.DT_BaseAnimating.m_bClientSideAnimation );
	  dump_pointer( Displacement.DT_BaseAnimating.m_flPoseParameter );
	  dump_pointer( Displacement.DT_BaseAnimating.m_nHitboxSet );
	  dump_pointer( Displacement.DT_BaseCombatCharacter.m_hActiveWeapon );
	  dump_pointer( Displacement.DT_BaseCombatCharacter.m_flNextAttack );
	  dump_pointer( Displacement.DT_BaseCombatCharacter.m_hMyWeapons );
	  dump_pointer( Displacement.DT_BaseCombatCharacter.m_hMyWearables );
	  dump_pointer( Displacement.C_BasePlayer.m_pCurrentCommand );
	  dump_pointer( Displacement.C_BasePlayer.UpdateVisibilityAllEntities );
	  dump_pointer( Displacement.DT_BasePlayer.m_aimPunchAngle );
	  dump_pointer( Displacement.DT_BasePlayer.m_aimPunchAngleVel );
	  dump_pointer( Displacement.DT_BasePlayer.m_viewPunchAngle );
	  dump_pointer( Displacement.DT_BasePlayer.m_vecViewOffset );
	  dump_pointer( Displacement.DT_BasePlayer.m_vecVelocity );
	  dump_pointer( Displacement.DT_BasePlayer.m_vecBaseVelocity );
	  dump_pointer( Displacement.DT_BasePlayer.m_flFallVelocity );
	  dump_pointer( Displacement.DT_BasePlayer.m_flDuckAmount );
	  dump_pointer( Displacement.DT_BasePlayer.m_flDuckSpeed );
	  dump_pointer( Displacement.DT_BasePlayer.m_lifeState );
	  dump_pointer( Displacement.DT_BasePlayer.m_nTickBase );
	  dump_pointer( Displacement.DT_BasePlayer.m_iHealth );
	  dump_pointer( Displacement.DT_BasePlayer.m_iDefaultFOV );
	  dump_pointer( Displacement.DT_BasePlayer.m_fFlags );
	  dump_pointer( Displacement.DT_BasePlayer.pl );
	  dump_pointer( Displacement.DT_BasePlayer.m_hObserverTarget );
	  dump_pointer( Displacement.DT_BasePlayer.m_hViewModel );
	  dump_pointer( Displacement.DT_BasePlayer.m_vphysicsCollisionState );
	  dump_pointer( Displacement.C_CSPlayer.m_PlayerAnimState );
	  dump_pointer( Displacement.C_CSPlayer.m_flSpawnTime );
	  dump_pointer( Displacement.CClientState.m_nLastCommandAck );
	  dump_pointer( Displacement.CClientState.m_nDeltaTick );
	  dump_pointer( Displacement.CClientState.m_nLastOutgoingCommand );
	  dump_pointer( Displacement.CClientState.m_nChokedCommands );
	  dump_pointer( Displacement.CClientState.m_bIsHLTV );
	  dump_pointer( Displacement.Data.m_uMoveHelper );
	  dump_pointer( Displacement.Data.m_uInput );
	  dump_pointer( Displacement.Data.m_uPredictionRandomSeed );
	  dump_pointer( Displacement.Data.m_uPredictionPlayer );
	  dump_pointer( Displacement.Data.m_uModelBoneCounter );
	  dump_pointer( Displacement.Data.m_uClientSideAnimationList );
	  dump_pointer( Displacement.Data.m_uGlowObjectManager );
	  dump_pointer( Displacement.Data.m_uCamThink );
	  dump_pointer( Displacement.Data.m_uRenderBeams );
	  dump_pointer( Displacement.Data.m_uSmokeCount );
	  dump_pointer( Displacement.Data.m_uCenterPrint );

	  dump_pointer( Displacement.Data.m_uListLeavesInBoxReturn );
	  dump_pointer( Displacement.Data.s_bAllowExtrapolation );
	  dump_pointer( Displacement.Data.m_FireBulletsReturn );
	  dump_pointer( Displacement.Data.m_D3DDevice );
	  dump_pointer( Displacement.Data.m_SoundService );
	  dump_pointer( Displacement.Data.m_InterpolateServerEntities );
	  dump_pointer( Displacement.Data.m_SendNetMsg );
	  dump_pointer( Displacement.Data.m_ModifyEyePos );
	  dump_pointer( Displacement.Data.m_ResetContentsCache );
	  dump_pointer( Displacement.Data.m_ProcessInterpolatedList );

	  dump_pointer( Displacement.Function.m_uRandomSeed );
	  dump_pointer( Displacement.Function.m_uRandomFloat );
	  dump_pointer( Displacement.Function.m_uRandomInt );
	  dump_pointer( Displacement.Function.m_uSetAbsOrigin );
	  dump_pointer( Displacement.Function.m_uSetAbsAngles );
	  dump_pointer( Displacement.Function.m_uIsBreakable );
	  dump_pointer( Displacement.Function.m_uClearHudWeaponIcon );
	  dump_pointer( Displacement.Function.m_uLoadNamedSkys );
	  dump_pointer( Displacement.Function.m_uCreateAnimState );
	  dump_pointer( Displacement.Function.m_uResetAnimState );
	  dump_pointer( Displacement.Function.m_uUpdateAnimState );
	  dump_pointer( Displacement.Function.m_uClanTagChange );
	  dump_pointer( Displacement.Function.m_uGetSequenceActivity );
	  dump_pointer( Displacement.Function.m_uInvalidatePhysics );
	  dump_pointer( Displacement.Function.m_uPostThinkVPhysics );
	  dump_pointer( Displacement.Function.m_SimulatePlayerSimulatedEntities );
	  dump_pointer( Displacement.Function.m_uImplPhysicsRunThink );

	  dump_pointer( Displacement.Function.m_SetCollisionBounds );
	  dump_pointer( Displacement.Function.m_MD5PseudoRandom );
	  dump_pointer( Displacement.Function.m_WriteUsercmd );
	  dump_pointer( Displacement.Function.m_StdStringAssign );
	  dump_pointer( Displacement.Function.m_pPoseParameter );
	  dump_pointer( Displacement.Function.m_AttachmentHelper );
	  dump_pointer( Displacement.Function.m_LockStudioHdr );
	  dump_pointer( Displacement.Function.m_LineGoesThroughSmoke );

	  dump_pointer( Displacement.CBoneMergeCache.m_nConstructor );
	  dump_pointer( Displacement.CBoneMergeCache.m_nInit );
	  dump_pointer( Displacement.CBoneMergeCache.m_nUpdateCache );
	  dump_pointer( Displacement.CBoneMergeCache.m_CopyToFollow );
	  dump_pointer( Displacement.CBoneMergeCache.m_CopyFromFollow );

	  dump_pointer( Displacement.CIKContext.m_nConstructor );
	  dump_pointer( Displacement.CIKContext.m_nDestructor );
	  dump_pointer( Displacement.CIKContext.m_nInit );
	  dump_pointer( Displacement.CIKContext.m_nUpdateTargets );
	  dump_pointer( Displacement.CIKContext.m_nSolveDependencies );

	  dump_pointer( Displacement.CBoneSetup.InitPose );
	  dump_pointer( Displacement.CBoneSetup.AccumulatePose );
	  dump_pointer( Displacement.CBoneSetup.CalcAutoplaySequences );
	  dump_pointer( Displacement.CBoneSetup.CalcBoneAdj );


   }
#endif

   bool CreateDisplacement( void* reserved ) {
   #ifdef DevelopMode
	  Create( );
   #else
	  Parse( reserved );
	  DllArgsToRecieve* transmitted_data = ( DllArgsToRecieve* ) reserved;
	  DllInitializeData* parsedData = ( DllInitializeData* ) transmitted_data->lpInitStruct;

	  VirtualFree( ( LPVOID ) transmitted_data->lpInitStruct, transmitted_data->dwMemorySize, MEM_DECOMMIT );
   #endif

	  //  DumpOffsets( reserved );
	  return true;
   }
}
