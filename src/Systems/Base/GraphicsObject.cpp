
#include "GraphicsObject.hpp"

GraphicsObject::GraphicsObject()
  : m_visible(false), m_x(0), m_y(0), m_width(0), m_height(0), 
    m_whateverAdjustVertOperatesOn(0), m_rotation(0), m_originX(0),
    m_originY(0), m_type2g00region(0)
{
  // Regretfully, we can't do this in the initializer list.
  fill(m_adjustX, m_adjustX + 8, 0);
  fill(m_adjustY, m_adjustY + 8, 0);
}
