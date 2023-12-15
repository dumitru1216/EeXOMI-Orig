#pragma once
// Generated using ReClass 2016

class CCSGO_DeathNoticeDown;
class N00000116;
class N00000229;
class CPanoramaPanel;
class N0000024F;
class N00000252;
class N00000256;
class N0000048C;
class N0000049E;
class CCSGO_HudDeathNotice;
class N000004C2;
class N000004D9;
class TABLE;
class N00000510;
class N00000524;
class N00000528;
class N0000053A;
class N0000054D;
class N00000550;
class N0000055E;
class N00000561;
class N00000573;
class TABLE_wrapper;
class N000005BF;

class TABLE_wrapper
{
public:
private:
  char pad_0x0000 [ 0x1C ]; //0x0000
public:
  void * EraseNotice; //0x001C 
private:
  char pad_0x0020 [ 0x10 ]; //0x0020
public:
  void * GetHudName; //0x0030 
private:
  char pad_0x0034 [ 0x50 ]; //0x0034
public:
  template <typename T>
  T GetPtrByIndex( int i );
}; //Size=0x0084

class WrapperForTableDEATHNOTICEHUD {
private:
  TABLE_wrapper * m_Table;
  CCSGO_DeathNoticeDown * _this;

public:
  void Think( TABLE_wrapper * table, CCSGO_DeathNoticeDown * _this );

public:
  char * GetHudName( );

  int EraseNotices( );
};

class CCSGO_DeathNoticeDown
{
public:
private:
  char pad_0x0000 [ 0x14 ]; //0x0000
public:
  TABLE_wrapper * m_vPtr; //0x0014 
private:
  char pad_0x0018 [ 0x1C ]; //0x0018
public:
  char * m_szNameOfHud; //0x0034 
private:
  char pad_0x0038 [ 0x20 ]; //0x0038
public:
  N00000116 * SomeInstance; //0x0058 
private:
  char pad_0x005C [ 0x4 ]; //0x005C
public:
  float m_flTime; //0x0060 
  float m_flLocalTime; //0x0064 
  float m_flUnknown; //0x0068 
private:
  char pad_0x006C [ 0x3D4 ]; //0x006C
  WrapperForTableDEATHNOTICEHUD m_WrapTable;
public:
  WrapperForTableDEATHNOTICEHUD * TableAccessor2( );

}; //Size=0x0440

class N00000116
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000
public:
  N00000229 * N00000118; //0x0004 
private:
  char pad_0x0008 [ 0x43C ]; //0x0008

}; //Size=0x0444

class N00000229
{
public:
  CPanoramaPanel * N0000022A; //0x0000 
private:
  char pad_0x0004 [ 0x28 ]; //0x0004
public:
  __int32 m_iUnknownNotChange; //0x002C 
private:
  char pad_0x0030 [ 0x1C ]; //0x0030

}; //Size=0x004C

class CPanoramaPanel
{
public:
private:
  char pad_0x0000 [ 0xC0 ]; //0x0000
public:
  void * GetUnknown; //0x00C0 
private:
  char pad_0x00C4 [ 0x7E7 ]; //0x00C4

}; //Size=0x08AB

class N0000024F
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x0004

class N00000252
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x0004

class N00000256
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x0004

class N0000048C
{
public:
  N0000049E * N0000048D; //0x0000 
private:
  char pad_0x0004 [ 0x3C ]; //0x0004

}; //Size=0x0040

class N0000049E
{
public:
private:
  char pad_0x0000 [ 0x44 ]; //0x0000

}; //Size=0x0044

class CCSGO_HudDeathNotice
{
public:
  N000004C2 * N000004B1; //0x0000 
private:
  char pad_0x0004 [ 0x3C ]; //0x0004

}; //Size=0x0040

class N000004C2
{
public:
private:
  char pad_0x0000 [ 0x30 ]; //0x0000
public:
  N000004D9 * N000004CF; //0x0030 
private:
  char pad_0x0034 [ 0x14 ]; //0x0034
public:
  N00000510 * N000004D5; //0x0048 
private:
  char pad_0x004C [ 0x88 ]; //0x004C

}; //Size=0x00D4

class N000004D9
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x00 04



class N00000510
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x0004

class N00000524
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x0004


class N0000053A
{
public:
private:
  char pad_0x0000 [ 0x44 ]; //0x0000

}; //Size=0x0044

class N0000054D
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x0004

class N00000550
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x0004



class N00000561
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x0004

class N00000573
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x0004


class N000005BF
{
public:
private:
  char pad_0x0000 [ 0x4 ]; //0x0000

}; //Size=0x0004