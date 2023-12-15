#include "vmt_swap.hpp"

bool Horizon::Memory::VmtSwap::Create( const void * instance ) {
  m_ppInstance = ( std::uintptr_t** )instance;
  m_pRestore = *m_ppInstance;

  if( !m_pRestore )
	 return false;

  while( m_pRestore[m_nSize] )
	 m_nSize++;

  if( !m_nSize )
	 return false;

  m_nSize++;
  m_pReplace = std::make_unique<std::uintptr_t[]>( m_nSize );
  memcpy( m_pReplace.get( ), &m_pRestore[-1], m_nSize * sizeof( std::uintptr_t ) );

  Replace( );

  return true;
}

void Horizon::Memory::VmtSwap::Destroy( ) {
  Restore( );

  m_ppInstance = nullptr;
  m_pRestore = nullptr;
  m_pReplace.reset( );
  m_nSize = 0u;
}

void Horizon::Memory::VmtSwap::Replace( ) {
  if( !m_ppInstance || !m_pReplace )
	 return;

  *m_ppInstance = &m_pReplace.get( )[1];
}

void Horizon::Memory::VmtSwap::Restore( ) {
  if( !m_ppInstance || !m_pRestore )
	 return;

  *m_ppInstance = m_pRestore;
}

void Horizon::Memory::VmtSwap::Hook( const void * hooked, const std::uint32_t index ) {
  if( !m_pReplace )
	 return;

  m_pReplace[index + 1] = ( std::uintptr_t )hooked;
}
