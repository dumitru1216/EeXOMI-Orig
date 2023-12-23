#pragma once

class IInput
{
public:
  virtual void Init_All( void ) = 0;
  virtual void Shutdown_All( void ) = 0;
  virtual int GetButtonBits( int ) = 0;
  virtual void CreateMove( int sequence_number, float input_sample_frametime, bool active ) = 0;
  virtual void ExtraMouseSample( float frametime, bool active ) = 0;
  virtual bool WriteUsercmdDeltaToBuffer( int nSlot,  void* buf, int from, int to, bool isnewcommand ) = 0;
  virtual void EncodeUserCmdToBuffer( void* buf, int slot ) = 0;
  virtual void DecodeUserCmdFromBuffer( void* buf, int slot ) = 0;

 //char pad_0x00[0x8];
 //bool m_fTrackIRAvailable;
 //bool m_fMouseInitialized;
 //bool m_fMouseActive;
 //bool m_fJoystickAdvancedInit;
 //char pad_0x08[0x2C];
 //void* m_pKeys;
 //char pad1[0x10];
 //float m_flKeyboardSampleTime;
 //char pad2[0x58];
 //bool m_fCameraInterceptingMouse;
 //bool m_fCameraInThirdPerson;
 //bool m_fCameraMovingWithMouse;
 //Vector m_vecCameraOffset;
 //bool m_fCameraDistanceMove;
 //int m_nCameraOldX;
 //int m_nCameraOldY;
 //int m_nCameraX;
 //int m_nCameraY;
 //bool m_CameraIsOrthographic;
 //Vector m_angPreviousViewAngles;
 //Vector m_angPreviousViewAnglesTilt;
 //float m_flLastForwardMove; // 0xEC
 //int m_nClearInputState;
 //CUserCmd* m_pCommands; // 0xF4
 //CVerifiedUserCmd* m_pVerifiedCommands;

  bool				m_pad_something;
  bool				m_fMouseInitialized;
  bool				m_fMouseActive;
  bool				pad_something01;
  char				pad_0x08[ 0x2C ];
  void* m_pKeys;
  char				pad_0x38[ 0x64 ];
  __int32				pad_0x41;
  __int32				pad_0x42;
  bool				m_fCameraInterceptingMouse;
  bool				m_fCameraInThirdPerson;
  bool                m_fCameraMovingWithMouse;
  Vector			    m_vecCameraOffset;
  bool                m_fCameraDistanceMove;
  int                 m_nCameraOldX;
  int                 m_nCameraOldY;
  int                 m_nCameraX;
  int                 m_nCameraY;
  bool                m_CameraIsOrthographic;
  Vector              m_angPreviousViewAngles;
  Vector              m_angPreviousViewAnglesTilt;
  float               m_flLastForwardMove;
  int                 m_nClearInputState;
  char                pad_0xE4[ 0x8 ];
  CUserCmd* m_pCommands;
  CVerifiedUserCmd* m_pVerifiedCommands;
};
