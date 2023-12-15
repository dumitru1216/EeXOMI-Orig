#pragma once

// Small helpers for drawing.
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9core.h>
#pragma comment(lib,"d3dx9.lib")
#include "InputSys.hpp"
#include "Render.hpp"
#include "FnvHash.hpp"

// All objects should be included before the menu.
#include "object.hpp"

#include "form.hpp"

#include "tab_control.hpp"
#include "tab_container.hpp"
#include "tab.hpp"

#include "checkbox.hpp"
#include "dropdown.hpp"
#include "button.hpp"
#include "slider.hpp"
#include "color_picker.hpp"

namespace Menu
{
  class c_ui {
  protected:
	 object::c_obj_vector m_container;
	 bool m_initialized;
	 void update_ui( );

  public:
	 void initialize( );
	 void on_frame( );
  }; extern c_ui UI;
}
