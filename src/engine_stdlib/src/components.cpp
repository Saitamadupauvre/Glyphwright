#include "gw/Collider2D.hpp"
#include "gw/reflect_macro.hpp"
#include "gw/Sprite.hpp"
#include "gw/Velocity.hpp"

ENGINE_REFLECT(gw::Sprite, texture_id, offset_x, offset_y, layer)
ENGINE_REFLECT(gw::Velocity, x, y)
ENGINE_REFLECT(gw::Collider2D, width, height, is_trigger)
