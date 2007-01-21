
#include "GraphicsObject.hpp"

#include <iostream>
#include <algorithm>

using namespace std;

GraphicsObject::GraphicsObject()
  : m_visible(false), 
    m_x(0), m_y(0), 
    m_whateverAdjustVertOperatesOn(0),
    m_repOriginX(0), m_repOriginY(0),

    // Width and height are percentages
    m_width(100), m_height(100), 

    m_rotation(0),

    m_pattNo(0), m_alpha(255), m_mono(0), m_invert(0),
    // Do the rest later.
    m_colourR(255), m_colourG(255), m_colourB(255), m_colourLevel(255)
{
  // Regretfully, we can't do this in the initializer list.
  fill(m_adjustX, m_adjustX + 8, 0);
  fill(m_adjustY, m_adjustY + 8, 0);
}

// -----------------------------------------------------------------------

void GraphicsObject::render(RLMachine& machine) {
  if(m_objectData && visible()) {
//    cerr << "Rendering object!" << endl;
    m_objectData->render(machine, *this);
  }
}
