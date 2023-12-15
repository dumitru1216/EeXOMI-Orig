#include "CCSGO_HudDeathNotice.hpp"
#include <Windows.h>

WrapperForTableDEATHNOTICEHUD * CCSGO_DeathNoticeDown::TableAccessor2( )
{
  m_WrapTable.Think( m_vPtr, this );

  return &m_WrapTable;
}

template<typename T>
T TABLE_wrapper::GetPtrByIndex( int i )
{
  return this ( T ) [ i ];
}

void WrapperForTableDEATHNOTICEHUD::Think( TABLE_wrapper * table, CCSGO_DeathNoticeDown * _this )
{
  m_Table = table;
  this->_this = ( CCSGO_DeathNoticeDown * ) ( ( DWORD ) ( ( DWORD ) _this + 20 ) );
}

char * WrapperForTableDEATHNOTICEHUD::GetHudName( )
{
  typedef char * ( __thiscall * fn )( void * );
  return ( ( fn ) m_Table->GetHudName )( _this );
}

int WrapperForTableDEATHNOTICEHUD::EraseNotices( )
{
  *( bool * ) ( ( DWORD ) _this + 0x8 ) = 0;
  typedef int( __thiscall * fn )( void * );
  return ( ( fn ) m_Table->EraseNotice )( _this );
}