#pragma once

class CCSWeaponInfo {
public:
	virtual ~CCSWeaponInfo( ) { };

	char* m_consoleName;				// 0x0004
	char		pad_0008[ 12 ];				// 0x0008
	int			m_iMaxClip;					// 0x0014
	int			m_iMaxClip2;					// 0x0018
	int			m_iDefaultClip1;				// 0x001C
	int			m_iDefaultClip2;				// 0x0020
	char		pad_0024[ 8 ];				// 0x0024
	char* m_szWorldModel;				// 0x002C
	char* m_szViewModel;				// 0x0030
	char* m_szDroppedModel;				// 0x0034
	char		pad_0038[ 4 ];				// 0x0038
	char* N0000023E;					// 0x003C
	char		pad_0040[ 56 ];				// 0x0040
	char* m_szEmptySound;				// 0x0078
	char		pad_007C[ 4 ];				// 0x007C
	char* m_szBulletType;				// 0x0080
	char		pad_0084[ 4 ];				// 0x0084
	char* m_szHudName;					// 0x0088
	char* m_szWeaponName;				// 0x008C
	char		pad_0090[ 56 ];				// 0x0090
	int 		m_iWeaponType;					// 0x00C8
	int			m_m_iWeaponPrice;				// 0x00CC // unktype
	int			m_iKillAward;					// 0x00D0
	char* m_szAnimationPrefix;			// 0x00D4
	float		m_flUnknownFloat0;				// 0x00D8
	float		m_flCycleTimeAlt;				// 0x00DC
	float		m_flTimeToIdle;				// 0x00E0
	float		m_flIdleInterval;				// 0x00E4
	bool		m_ucFullAuto;					// 0x00E8
	char		pad_0x00E5[ 3 ];			// 0x00E9
	int			m_iWeaponDamage;					// 0x00EC
	float		m_flArmorRatio;				// 0x00F0
	int			m_iBullets;					// 0x00F4
	float		m_flPenetration;				// 0x00F8
	float		m_flFlinchVelocityModifierLarge;	// 0x00FC
	float		m_flFlinchVelocityModifierSmall;	// 0x0100
	float		m_flWeaponRange;					// 0x0104
	float		m_flRangeModifier;			// 0x0108
	float		m_flThrowVelocity;			// 0x010C
	char		pad_0x010C[ 12 ];			// 0x0110
	bool		m_bHasSilencer;				// 0x011C
	char		pad_0x0119[ 3 ];			// 0x011D
	char* m_pSilencerModel;				// 0x0120
	int			m_iCrosshairMinDistance;		// 0x0124
	int			m_iCrosshairDeltaDistance;	// 0x0128 - iTeam?
	float		m_flMaxSpeed;			// 0x012C
	float		m_flMaxSpeed2;		// 0x0130
	float		m_flSpread;					// 0x0134
	float		m_flSpreadAlt;				// 0x0138
	float		m_flInaccuracyCrouch;			// 0x013C
	float		m_flInaccuracyCrouchAlt;		// 0x0140
	float		m_flInaccuracyStand;			// 0x0144
	float		m_flInaccuracyStandAlt;		// 0x0148
	float		m_flInaccuracyJumpInitial;	// 0x014C
	float		m_flInaccuracyJump;			// 0x0150
	float		m_flInaccuracyJumpAlt;		// 0x0154
	float		m_flInaccuracyLand;			// 0x0158
	float		m_flInaccuracyLandAlt;		// 0x015C
	float		m_flInaccuracyLadder;			// 0x0160
	float		m_flInaccuracyLadderAlt;		// 0x0164
	float		m_flInaccuracyFire;			// 0x0168
	float		m_flInaccuracyFireAlt;		// 0x016C
	float		m_flInaccuracyMove;			// 0x0170
	float		m_flInaccuracyMoveAlt;		// 0x0174
	float		m_flInaccuracyReload;			// 0x0178
	int			m_iRecoilSeed;				// 0x017C
	float		m_flRecoilAngle;				// 0x0180
	float		m_flRecoilAngleAlt;			// 0x0184
	float		m_flRecoilAngleVariance;		// 0x0188
	float		m_flRecoilAngleVarianceAlt;	// 0x018C
	float		m_flRecoilMagnitude;			// 0x0190
	float		m_flRecoilMagnitudeAlt;		// 0x0194
	float		m_flRecoilMagnitudeVariance;	// 0x0198
	float		m_flRecoilMagnitudeVarianceAlt;	// 0x019C
	float		m_flRecoveryTimeCrouch;		// 0x01A0
	float		m_flRecoveryTimeStand;		// 0x01A4
	float		m_flRecoveryTimeCrouchFinal;	// 0x01A8
	float		m_flRecoveryTimeStandFinal;	// 0x01AC
	int			m_iRecoveryTransitionStartBullet;	// 0x01B0 
	int			m_iRecoveryTransitionEndBullet;	// 0x01B4
	bool		m_bUnzoomAfterShot;			// 0x01B8
	bool		m_bHideViewModelZoomed;		// 0x01B9
	char		pad_0x01B5[ 2 ];			// 0x01BA
	char		m_iZoomLevels[ 3 ];			// 0x01BC
	int			m_iZoomFOV[ 2 ];				// 0x01C0
	float		m_fZoomTime[ 3 ];				// 0x01C4
	char* m_szWeaponClass;				// 0x01D4
	float		m_flAddonScale;				// 0x01D8
	char		pad_0x01DC[ 4 ];			// 0x01DC
	char* m_szEjectBrassEffect;			// 0x01E0
	char* m_szTracerEffect;				// 0x01E4
	int			m_iTracerFrequency;			// 0x01E8
	int			m_iTracerFrequencyAlt;		// 0x01EC
	char* m_szMuzzleFlashEffect_1stPerson;	// 0x01F0
	char		pad_0x01F4[ 4 ];				// 0x01F4
	char* m_szMuzzleFlashEffect_3rdPerson;	// 0x01F8
	char		pad_0x01FC[ 4 ];			// 0x01FC
	char* m_szMuzzleSmokeEffect;		// 0x0200
	float		m_flHeatPerShot;				// 0x0204
	char* m_szZoomInSound;				// 0x0208
	char* m_szZoomOutSound;				// 0x020C
	float		m_flInaccuracyPitchShift;		// 0x0210
	float		m_flInaccuracySoundThreshold;	// 0x0214
	float		m_flBotAudibleRange;			// 0x0218
	char		pad_0x0218[ 8 ];			// 0x0220
	char* m_pWrongTeamMsg;				// 0x0224
	bool		m_bHasBurstMode;				// 0x0228
	char		pad_0x0225[ 3 ];			// 0x0229
	bool		m_bIsRevolver;				// 0x022C
	bool		m_bCannotShootUnderwater;		// 0x0230

}; //Size=0x083C


/*
class CCSWeaponInfo
{
public:
  char pad_0x0000[0x4]; //0x0000
  char* m_szWeaponName; //0x0004
  char pad_0x0008[0xC]; //0x0008
  __int32 m_iMaxClip; //0x0014
  char pad_0x0018[0xC]; //0x0018
  __int32 m_iMaxReservedAmmo; //0x0024
  char pad_0x0028[0x4]; //0x0028
  char* m_szWeaponMdlPath; //0x002C
  char* m_szWeaponMdlPath2; //0x0030
  char* m_szDroppedWeaponMdlPath; //0x0034
  char* m_szDefaultClip; //0x0038
  char pad_0x003C[0x44]; //0x003C
  char* m_szBulletType; //0x0080
  char pad_0x0084[0x4]; //0x0084
  char* m_szHudName; //0x0088
  char* m_szWeaponName2; //0x008C
  char pad_0x0090[0x38]; //0x0090
  __int32 m_iWeaponType; //0x00C8
  __int32 m_iWeaponType2; //0x00CC
  __int32 m_iWeaponPrice; //0x00D0
  __int32 m_iWeaponReward; //0x00D4
  char* m_szWeaponType; //0x00D8
  float m_flUnknownFloat0; //0x00DC
  char pad_0x00E0[0xC]; //0x00E0
  unsigned char m_ucFullAuto; //0x00EC
  char pad_0x00ED[0x3]; //0x00ED
  __int32 m_iWeaponDamage; //0x00F0
  float m_flArmorRatio; //0x00F4
  __int32 m_iBullets; //0x00F8
  float m_flPenetration; //0x00FC
  char pad_0x0100[0x8]; //0x0100
  float m_flWeaponRange; //0x0108
  float m_flRangeModifier; //0x010C
  float m_flThrowVelocity; //0x0110
  char pad_0x0114[0xC]; //0x0114
  unsigned char m_ucHasSilencer; //0x0120
  char pad_0x0121[0xF]; //0x0121
  float m_flMaxSpeed; //0x0130
  float m_flMaxSpeed2; //0x0134
  float m_flAttackMoveSpeedFactor; //0x0138
  float m_flSpread; //0x013C
  float m_flSpreadAlt; //0x0140
  float m_flInaccuracyCrouch; //0x0144
  float m_flInaccuracyCrouchAlt; //0x0148
  float m_flInaccuracyStand; //0x014C
  float m_flInaccuracyStandAlt; //0x0150
  float m_flInaccuracyJump; //0x0154
  float m_flInaccuracyJumpAlt; //0x0158
  char pad_0x015C[0x28]; //0x015C
  __int32 m_iRecoilSeed; //0x0184
  char pad_0x0188[0x68]; //0x0188
  char* m_szWeaponTracesType; //0x01F0
  char pad_0x01F4[0x648]; //0x01F4

}; //Size=0x083C

*/