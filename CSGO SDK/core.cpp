#include "core.hpp"
#include "sdk.hpp"
#include "source.hpp"
#include "Render.hpp"
#include <uchar.h>
#include "displacement.hpp"

std::uint8_t* GetServerEdict( int index ) {
   static uintptr_t pServerGlobals = **( uintptr_t * * ) ( Engine::Displacement.Data.m_uServerGlobals );
   int iMaxClients = *( int* ) ( ( uintptr_t ) pServerGlobals + 0x18 );
   if ( iMaxClients >= index ) {
	  if ( index <= iMaxClients ) {
		 int v10 = index * 16;
		 uintptr_t v11 = *( uintptr_t* ) ( pServerGlobals + 96 );
		 if ( v11 ) {
			if ( !( ( *( uintptr_t* ) ( v11 + v10 ) >> 1 ) & 1 ) ) {
			   uintptr_t v12 = *( uintptr_t* ) ( v10 + v11 + 12 );
			   if ( v12 ) {
				  uint8_t* pReturn = nullptr;

				  // abusing asm is not good
				  __asm
				  {
					 pushad
					 mov ecx, v12
					 mov eax, dword ptr[ ecx ]
					 call dword ptr[ eax + 0x14 ]
					 mov pReturn, eax
					 popad
				  }

				  return pReturn;
			   }
			}
		 }
	  }
   }
   return nullptr;
}

bool screen_transform( const Vector& in, Vector2D& out ) {
  static auto& w2sMatrix = Source::m_pEngine->WorldToScreenMatrix( );

  out.x = w2sMatrix.m[0][0] * in.x + w2sMatrix.m[0][1] * in.y + w2sMatrix.m[0][2] * in.z + w2sMatrix.m[0][3];
  out.y = w2sMatrix.m[1][0] * in.x + w2sMatrix.m[1][1] * in.y + w2sMatrix.m[1][2] * in.z + w2sMatrix.m[1][3];

  float w = w2sMatrix.m[3][0] * in.x + w2sMatrix.m[3][1] * in.y + w2sMatrix.m[3][2] * in.z + w2sMatrix.m[3][3];

  if( w < 0.001f ) {
	 return false;
  }

  out.x /= w;
  out.y /= w;

  return true;
}
std::u16string StringtoU16(const std::string &str) {
  std::u16string wstr = u"";
  char16_t c16str[3] = u"\0";
  mbstate_t mbs;
  for (const auto& it : str) {
	 memset(&mbs, 0, sizeof(mbs));//set shift state to the initial state
	 memmove(c16str, u"\0\0\0", 3);
	 mbrtoc16(c16str, &it, 3, &mbs);
	 wstr.append(std::u16string(c16str));
  }//for
  return wstr;
}
bool WorldToScreen( const Vector & in, Vector2D & out ) {
  if( screen_transform( in, out ) ) {
	 Vector2D screen = Render::Get( )->GetScreenSize( );

	 out.x = ( screen.x * 0.5f ) + ( out.x * screen.x ) * 0.5f;
	 out.y = ( screen.y * 0.5f ) - ( out.y * screen.y ) * 0.5f;

	 return true;
  }
  return false;
}
