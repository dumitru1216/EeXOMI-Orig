#include "platform.hpp"
#include "source.hpp"

void* MemAlloc_Alloc( int size ) {
  return Source::m_pMemAlloc->Alloc( size );
}

void *MemAlloc_Realloc( void *pMemBlock, int size ) {
  return Source::m_pMemAlloc->Realloc( pMemBlock, size );
}

void MemAlloc_Free( void* pMemBlock ) {
  Source::m_pMemAlloc->Free( pMemBlock );
}
