#pragma once
#include "../ExGui.hpp"

class c_colorpicker : public c_element {
private:
  bool is_inline;
  bool alpha_bar;

  bool changing_alpha = false;
  bool changing_hue = false;
  bool changing_color = false;

  FloatColor* m_varClr = nullptr;

  Vector2 cursor_pos;

  float hue;
  float alpha;

  bool context_menu_open = false;
  Vector2 context_menu_pos;

  std::vector< std::pair< std::string, std::function< void( c_colorpicker* ) > > > context_menu_actions;

  static void context_menu_copy( c_colorpicker *self );
  static void context_menu_paste( c_colorpicker *self );

public:
  c_colorpicker( const std::string &label, FloatColor* var_clr, bool is_inline = false, bool alpha_bar = true );

  void think( ) override;
  void draw( ) override;

};

