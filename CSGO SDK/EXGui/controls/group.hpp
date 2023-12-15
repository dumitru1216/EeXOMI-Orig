#pragma once
#include "../ExGui.hpp"

class c_group : public c_element {
private:
  Vector2 cursor_pos;
  float scroll = 0.f;

public:
  c_group( const std::string &label, const Vector2 &size );

  void think( ) override;
  void draw( ) override;

  void add_child( c_element *child ) override;
  Vector2 get_child_draw_position( ) override;

  Vector2 get_cursor_pos( );
  void set_cursor_pos( const Vector2 &pos );

};

