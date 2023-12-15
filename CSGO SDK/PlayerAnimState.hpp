#pragma once
#include "sdk.hpp"

class C_CSPlayer;
class C_WeaponCSBaseGun;

struct animstate_pose_param_cache_t {
  bool valid = false;
  int index = -1;
  const char* name;
  void SetValue( C_CSPlayer* player, float flValue );
};

class CCSGOPlayerAnimState {
public:
  char pad[3];
  char bUnknown;									  // 0x03
  bool m_bInvalid;								  // 0x04
  char pad2[72];
  int m_iModelIndex;
  char pad_[11];
  C_CSPlayer* m_Player;                     // 0x60
  C_WeaponCSBaseGun* m_ActiveWeapon;        // 0x64
  C_WeaponCSBaseGun* m_LastActiveWeapon;	  // 0x68
  float m_flLastUpdateTime;                 // 0x6C
  int m_nLastFrame;                         // 0x70
  float m_flFrametime;                      // 0x74
  float m_flEyeYaw;                         // 0x78
  float m_flPitch;                          // 0x7C
  float m_flAbsRotation;                    // 0x80
  float m_flOldAbsRotation;                 // 0x84
  float m_flCurrentTorsoYaw;                // 0x88
  float m_flUnknownVelocityLean;            // 0x8C changes when moving/jumping/hitting ground
  float m_flLeanAmount;                     // 0x90
  float m_flUnknown1;                       // 0x94
  float m_flFeetCycle;                      // 0x98 0 to 1
  float m_flFeetYawRate;                    // 0x9C 0 to 1
  float m_fUnknown2;								  // 0xA0
  float m_fDuckAmount;							  // 0xA4
  float m_fLandingDuckAdditiveSomething;	  // 0xA8
  float m_fUnknown3;								  // 0xAC
  Vector m_vOrigin;								  // 0xB0, 0xB4, 0xB8
  Vector m_vLastOrigin;							  // 0xBC, 0xC0, 0xC4
  Vector m_vecVelocity;							  // 0xC8
  Vector m_vecNormalizedVelocity;			  // 0xD4 velocity * (1.0 / velocity_length)
  Vector m_vecNormalizedMovingVelocity;	  // 0xE0
  float m_velocity;								  // 0xEC
  float flUpVelocity;							  // 0xF0
  float m_flSpeedNormalized;					  // 0xF4 //from 0 to 1
  float m_flDuckingSpeed;						  // 0xF8 calculation: (1.923077 / flMaxPlayerSpeed) * velocity;
  float m_flRunningSpeed;						  // 0xFC calculation: (2.9411764 / flMaxPlayerSpeed) * velocity; 1.0 / 2.9411764 = 0.34
  float m_flTimeSinceStartedMoving;			  // 0x100
  float m_flTimeSinceStoppedMoving;			  // 0x104
  bool m_bOnGround;								  // 0x108
  bool m_bHitground;								  // 0x109
  float m_flLastBodyUpdate;					  // 0x10C
  float m_flAirTime;								  // 0x110
  float m_fUnknown4;								  // 0x114
  float m_flLastOriginZ;						  // 0x118
  float m_flGroundFraction;					  // 0x11C decreasing if velocity smaller 135.2, and vice versa
  float m_flStopToFullRunningFraction;		  // 0x120
  float m_flProceduralFootPlant;				  // 0x124
  float m_flUnknownFraction;					  // 0x128
  float m_fUnknown5;                        // 0x12C
  float m_flUnknown3;							  // 0x130
  bool m_StartedMoving;							  // 0x134
  char pad10[ 35 ];                               // 0x157
  float m_unkLeanTime;                            // 0x15B
  Vector m_vecLeanVelocity;                       // 0x16D
  Vector m_velocityUnk2;                          // 0x17F
  Vector m_leanVelocity1;                         // 0x191
  Vector m_leanVelocity2;
  char pad11[32];
  bool m_bUnknown01;
  bool m_bIsAccelerating;
  animstate_pose_param_cache_t lean_yaw; // 0x1B0
  animstate_pose_param_cache_t speed;
  animstate_pose_param_cache_t ladder_speed;
  animstate_pose_param_cache_t ladder_yaw;
  animstate_pose_param_cache_t move_yaw;
  animstate_pose_param_cache_t run;
  animstate_pose_param_cache_t body_yaw;
  animstate_pose_param_cache_t body_pitch;
  animstate_pose_param_cache_t death_yaw;
  animstate_pose_param_cache_t stand;
  animstate_pose_param_cache_t jump_fall;
  animstate_pose_param_cache_t aim_blend_stand_idle;
  animstate_pose_param_cache_t aim_blend_crouch_idle;
  animstate_pose_param_cache_t strafe_yaw;
  animstate_pose_param_cache_t aim_blend_stand_walk;
  animstate_pose_param_cache_t aim_blend_stand_run;
  animstate_pose_param_cache_t aim_blend_crouch_walk;
  animstate_pose_param_cache_t move_blend_walk;
  animstate_pose_param_cache_t move_blend_run;
  animstate_pose_param_cache_t move_blend_crouch;
  float unk_speed_01;
  float m_flVelocityUnknown;
  char pad12[136]; // those vars used in DoProceduralFootPlant, so idk what is it

  // Used for pose calculations ( body_yaw and body_pitch )
  float m_flMinBodyYaw; // 0x330
  float m_flMaxBodyYaw; // 0x334
  float m_flMinBodyPitch;
  float m_flMaxBodyPitch;
  int anim_state_version; // currently equals 2

  void Reset();
  void Update( QAngle angles );
  float GetMaxFraction( );
  float GetDesyncDelta( bool useMinYaw = false );
};//Size=0x344
