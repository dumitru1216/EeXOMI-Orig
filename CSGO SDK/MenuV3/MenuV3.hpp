#if 1
#define sal
#include "../imgui.h"
#include<string>
#include<sstream>
#include<vector>
#include<math.h>

#include <d3dx9.h>
#include <d3d9.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

using namespace ImGui;

namespace nem {
	class c_menu {
	public:
		void init( IDirect3DDevice9* const device );
		void render( bool is_opened, IDirect3DDevice9* const device );
	};

	inline const auto g_menu = std::make_unique< c_menu >( );
}
#endif