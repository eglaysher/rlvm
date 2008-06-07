// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

// -----------------------------------------------------------------------

#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"
#include "Utilities.h"

#include <numeric>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>

using namespace std;

const int DEFAULT_TEXT_SIZE = 14;
const int DEFAULT_TEXT_XSPACE = 0;
const int DEFAULT_TEXT_YSPACE = 0;
const int DEFAULT_TEXT_VERTICAL = 0;
const int DEFAULT_TEXT_COLOUR = 0;
const int DEFAULT_TEXT_SHADOWCOLOUR = 0;

const Rect EMPTY_CLIP = Rect(Point(0, 0), Size(-1, -1));

//boost::shared_ptr<GraphicsObject::Impl> g_constantBaseObj( 
const boost::shared_ptr<GraphicsObject::Impl> GraphicsObject::s_emptyImpl(
  new GraphicsObject::Impl);

// -----------------------------------------------------------------------
// GraphicsObject::TextProperties
// -----------------------------------------------------------------------
GraphicsObject::Impl::TextProperties::TextProperties()
  : textSize(DEFAULT_TEXT_SIZE), 
    xspace(DEFAULT_TEXT_XSPACE), 
    yspace(DEFAULT_TEXT_YSPACE), 
    vertical(DEFAULT_TEXT_VERTICAL),
    colour(DEFAULT_TEXT_COLOUR),
    shadowColour(DEFAULT_TEXT_SHADOWCOLOUR)
{
}

// -----------------------------------------------------------------------
// GraphicsObject
// -----------------------------------------------------------------------
GraphicsObject::GraphicsObject()
  : m_impl(s_emptyImpl)
//  : m_impl(new GraphicsObject::Impl)
{}

// -----------------------------------------------------------------------

GraphicsObject::GraphicsObject(const GraphicsObject& rhs)
  : m_impl(rhs.m_impl)
{
  if(rhs.m_objectData)
  {
    m_objectData.reset(rhs.m_objectData->clone());
    m_objectData->setOwnedBy(*this);
  }
  else
    m_objectData.reset();
}

// -----------------------------------------------------------------------

GraphicsObject::~GraphicsObject()
{}

// -----------------------------------------------------------------------

GraphicsObject& GraphicsObject::operator=(const GraphicsObject& obj)
{
  m_impl = obj.m_impl;

  if(obj.m_objectData)
  {
    m_objectData.reset(obj.m_objectData->clone());
    m_objectData->setOwnedBy(*this);
  }
  else
    m_objectData.reset();

  return *this;
}

// -----------------------------------------------------------------------

void GraphicsObject::setObjectData(GraphicsObjectData* obj)
{
  m_objectData.reset(obj);
  m_objectData->setOwnedBy(*this);
}

// -----------------------------------------------------------------------

void GraphicsObject::setVisible(const int in) 
{
  makeImplUnique();
  m_impl->m_visible = in;
}

// -----------------------------------------------------------------------

void GraphicsObject::setX(const int x) 
{
  makeImplUnique();
  m_impl->m_x = x;
}

// -----------------------------------------------------------------------

void GraphicsObject::setY(const int y)
{ 
  makeImplUnique();
  m_impl->m_y = y;
}

// -----------------------------------------------------------------------

int GraphicsObject::xAdjustmentSum() const 
{
  return std::accumulate(m_impl->m_adjustX, m_impl->m_adjustX + 8, 0);
}

// -----------------------------------------------------------------------

void GraphicsObject::setXAdjustment(int idx, int x) 
{ 
  makeImplUnique();
  m_impl->m_adjustX[idx] = x; 
}

// -----------------------------------------------------------------------

int GraphicsObject::yAdjustmentSum() const
{ 
  return std::accumulate(m_impl->m_adjustY, m_impl->m_adjustY + 8, 0); 
}

// -----------------------------------------------------------------------

void GraphicsObject::setYAdjustment(int idx, int y) 
{ 
  makeImplUnique();
  m_impl->m_adjustY[idx] = y;
}

// -----------------------------------------------------------------------

void GraphicsObject::setVert(const int vert) 
{ 
  makeImplUnique(); 
  m_impl->m_whateverAdjustVertOperatesOn = vert; 
}

// -----------------------------------------------------------------------

void GraphicsObject::setXOrigin(const int x) 
{ 
  makeImplUnique(); 
  m_impl->m_originX = x; 
}

// -----------------------------------------------------------------------

void GraphicsObject::setYOrigin(const int y)
{
  makeImplUnique(); 
  m_impl->m_originY = y; 
}

// -----------------------------------------------------------------------

void GraphicsObject::setWidth(const int in)
{ 
  makeImplUnique(); 
  m_impl->m_width = in;
}

// -----------------------------------------------------------------------

void GraphicsObject::setHeight(const int in) 
{ 
  makeImplUnique(); 
  m_impl->m_height = in; 
}

// -----------------------------------------------------------------------

void GraphicsObject::setRotation(const int in)
{
  makeImplUnique(); 
  m_impl->m_rotation = in; 
}

// -----------------------------------------------------------------------

int GraphicsObject::pixelWidth(RLMachine& machine) const
{
  // Calculate out the pixel width of the current object taking in the
  // width() scaling.
  if(hasObjectData())
    return m_objectData->pixelWidth(machine, *this);
  else
    return 0;
}

// -----------------------------------------------------------------------

int GraphicsObject::pixelHeight(RLMachine& machine) const
{
  if(hasObjectData())
    return m_objectData->pixelHeight(machine, *this);
  else
    return 0;
}

// -----------------------------------------------------------------------

void GraphicsObject::setPattNo(const int in) 
{ makeImplUnique(); m_impl->m_pattNo = in; }
void GraphicsObject::setMono(const int in)
{ makeImplUnique(); m_impl->m_mono = in; }
void GraphicsObject::setInvert(const int in)
{ makeImplUnique(); m_impl->m_invert = in; }
void GraphicsObject::setLight(const int in)
{ makeImplUnique(); m_impl->m_light = in; }
void GraphicsObject::setTintR(const int in)
{ makeImplUnique(); m_impl->m_tint.setRed(in); }
void GraphicsObject::setTintG(const int in)
{ makeImplUnique(); m_impl->m_tint.setGreen(in); }
void GraphicsObject::setTintB(const int in)
{ makeImplUnique(); m_impl->m_tint.setBlue(in); }
void GraphicsObject::setColourR(const int in)
{ makeImplUnique(); m_impl->m_colour.setRed(in); }
void GraphicsObject::setColourG(const int in)
{ makeImplUnique(); m_impl->m_colour.setGreen(in); }
void GraphicsObject::setColourB(const int in)
{ makeImplUnique(); m_impl->m_colour.setBlue(in); }
void GraphicsObject::setColourLevel(const int in)
{ makeImplUnique(); m_impl->m_colour.setAlpha(in); }

// -----------------------------------------------------------------------

void GraphicsObject::setCompositeMode(const int in) 
{ 
  makeImplUnique();
  m_impl->m_compositeMode = in; 
}

// -----------------------------------------------------------------------

void GraphicsObject::setScrollRateX(const int x) 
{ makeImplUnique(); m_impl->m_scrollRateX = x; }
void GraphicsObject::setScrollRateY(const int y)
{ makeImplUnique(); m_impl->m_scrollRateY = y; }

// -----------------------------------------------------------------------

void GraphicsObject::setAlpha(const int alpha)
{   
  makeImplUnique();
  m_impl->m_alpha = alpha;
}

// -----------------------------------------------------------------------

void GraphicsObject::clearClip()
{ 
  makeImplUnique();
  m_impl->m_clip = EMPTY_CLIP;
}

// -----------------------------------------------------------------------

void GraphicsObject::setClip(const Rect& rect)
{
  makeImplUnique();
  m_impl->m_clip = rect;
}

// -----------------------------------------------------------------------

GraphicsObjectData& GraphicsObject::objectData()
{
  if(m_objectData)
    return *m_objectData;
  else
  {
    throw rlvm::Exception("null object data");
  }
}

// -----------------------------------------------------------------------

void GraphicsObject::setWipeCopy(const int wipeCopy) 
{ 
  makeImplUnique();
  m_impl->m_wipeCopy = wipeCopy;
}

// -----------------------------------------------------------------------

void GraphicsObject::setTextText(const std::string& utf8str) 
{
  makeImplUnique();
  m_impl->makeSureHaveTextProperties();
  m_impl->m_textProperties->value = utf8str; 
}

// -----------------------------------------------------------------------

const std::string& GraphicsObject::textText() const
{ 
  static const std::string empty = "";

  if(m_impl->m_textProperties)
    return m_impl->m_textProperties->value; 
  else
    return empty;
}

// -----------------------------------------------------------------------

int GraphicsObject::textSize() const
{
  if(m_impl->m_textProperties)
    return m_impl->m_textProperties->textSize; 
  else
    return DEFAULT_TEXT_SIZE;
}

// -----------------------------------------------------------------------

int GraphicsObject::textXSpace() const
{
  if(m_impl->m_textProperties)
    return m_impl->m_textProperties->xspace; 
  else
    return DEFAULT_TEXT_XSPACE;
}

// -----------------------------------------------------------------------

int GraphicsObject::textYSpace() const
{
  if(m_impl->m_textProperties)
    return m_impl->m_textProperties->yspace; 
  else
    return DEFAULT_TEXT_YSPACE;
}

// -----------------------------------------------------------------------

int GraphicsObject::textVertical() const
{
  if(m_impl->m_textProperties)
    return m_impl->m_textProperties->vertical; 
  else
    return DEFAULT_TEXT_VERTICAL;
}

// -----------------------------------------------------------------------

int GraphicsObject::textColour() const
{
  if(m_impl->m_textProperties)
    return m_impl->m_textProperties->colour; 
  else
    return DEFAULT_TEXT_COLOUR;
}

// -----------------------------------------------------------------------

int GraphicsObject::textShadowColour() const
{
  if(m_impl->m_textProperties)
    return m_impl->m_textProperties->shadowColour; 
  else
    return DEFAULT_TEXT_SHADOWCOLOUR;
}

// -----------------------------------------------------------------------

void GraphicsObject::setTextOps(
  int size, int xspace, int yspace, int vertical, int colour, int shadow)
{
  makeImplUnique();

  m_impl->makeSureHaveTextProperties();
  m_impl->m_textProperties->textSize = size;
  m_impl->m_textProperties->xspace = xspace;
  m_impl->m_textProperties->yspace = yspace;
  m_impl->m_textProperties->vertical = vertical;
  m_impl->m_textProperties->colour = colour;
  m_impl->m_textProperties->shadowColour = shadow;
}

// -----------------------------------------------------------------------

void GraphicsObject::makeImplUnique()
{
  if(!m_impl.unique())
  {
    m_impl.reset(new Impl(*m_impl));
  }
}

// -----------------------------------------------------------------------

void GraphicsObject::render(RLMachine& machine)
{
  if(m_objectData && visible())
  {
    m_objectData->render(machine, *this);
  }
}

// -----------------------------------------------------------------------

void GraphicsObject::deleteObject()
{
  m_objectData.reset();
}

// -----------------------------------------------------------------------

void GraphicsObject::clearObject()
{
  m_impl = s_emptyImpl;
  m_objectData.reset();
}

// -----------------------------------------------------------------------

void GraphicsObject::execute(RLMachine& machine)
{
  if(m_objectData)
  {
    m_objectData->execute(machine);
  }
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsObject::serialize(Archive& ar, unsigned int version)
{
  ar & m_impl & m_objectData;
}

// -----------------------------------------------------------------------

template void GraphicsObject::serialize<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version);

template void GraphicsObject::serialize<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);

// -----------------------------------------------------------------------
// GraphicsObject::Impl
// -----------------------------------------------------------------------
GraphicsObject::Impl::Impl()
  : m_visible(false), 
    m_x(0), m_y(0), 
    m_whateverAdjustVertOperatesOn(0),
    m_originX(0), m_originY(0),
    m_repOriginX(0), m_repOriginY(0),

    // Width and height are percentages
    m_width(100), m_height(100), 

    m_rotation(0),

    m_pattNo(0), m_alpha(255),

    m_clip(EMPTY_CLIP),

    m_mono(0), m_invert(0), m_light(0),
    // Do the rest later.
    m_tint(RGBColour::White()),
    m_colour(RGBAColour::White()),
    m_compositeMode(0),
    m_scrollRateX(0),
    m_scrollRateY(0),
    m_wipeCopy(0)
{
  // Regretfully, we can't do this in the initializer list.
  fill(m_adjustX, m_adjustX + 8, 0);
  fill(m_adjustY, m_adjustY + 8, 0);
}

// -----------------------------------------------------------------------

GraphicsObject::Impl::Impl(const Impl& rhs)
  : m_visible(rhs.m_visible), m_x(rhs.m_x), m_y(rhs.m_y),
    m_whateverAdjustVertOperatesOn(rhs.m_whateverAdjustVertOperatesOn),
    m_originX(rhs.m_originX), m_originY(rhs.m_originY), 
    m_repOriginX(rhs.m_repOriginX), m_repOriginY(rhs.m_repOriginY),
    m_width(rhs.m_width), m_height(rhs.m_height),
    m_rotation(rhs.m_rotation),
    m_pattNo(rhs.m_pattNo), m_alpha(rhs.m_alpha),
    m_clip(rhs.m_clip),
    m_mono(rhs.m_mono), m_invert(rhs.m_invert),
    m_light(rhs.m_light), m_tint(rhs.m_tint), m_colour(rhs.m_colour),
    m_compositeMode(rhs.m_compositeMode),
    m_scrollRateX(rhs.m_scrollRateX),
    m_scrollRateY(rhs.m_scrollRateY), 
    m_wipeCopy(0)
{   
  if(rhs.m_textProperties)
    m_textProperties.reset(new TextProperties(*rhs.m_textProperties));

  copy(rhs.m_adjustX, rhs.m_adjustX + 8, m_adjustX);
  copy(rhs.m_adjustY, rhs.m_adjustY + 8, m_adjustY);
}

// -----------------------------------------------------------------------

GraphicsObject::Impl::~Impl()
{}

// -----------------------------------------------------------------------

GraphicsObject::Impl& GraphicsObject::Impl::operator=(
  const GraphicsObject::Impl& rhs)
{
  if(this != &rhs)
  {
    m_visible = rhs.m_visible;
    m_x = rhs.m_x;
    m_y = rhs.m_y;

    copy(rhs.m_adjustX, rhs.m_adjustX + 8, m_adjustX);
    copy(rhs.m_adjustY, rhs.m_adjustY + 8, m_adjustY);

    m_whateverAdjustVertOperatesOn = rhs.m_whateverAdjustVertOperatesOn;
    m_originX = rhs.m_originX;
    m_originY = rhs.m_originY;
    m_repOriginX = rhs.m_repOriginX;
    m_repOriginY = rhs.m_repOriginY;
    m_width = rhs.m_width;
    m_height = rhs.m_height;
    m_rotation = rhs.m_rotation;

    m_pattNo = rhs.m_pattNo;
    m_alpha = rhs.m_alpha;
    m_clip = rhs.m_clip;
    m_mono = rhs.m_mono;
    m_invert = rhs.m_invert;
    m_light = rhs.m_light;
    m_tint = rhs.m_tint;

    m_colour = rhs.m_colour;

    m_compositeMode = rhs.m_compositeMode;
    m_scrollRateX = rhs.m_scrollRateX;
    m_scrollRateY = rhs.m_scrollRateY;

    if(rhs.m_textProperties)
      m_textProperties.reset(new TextProperties(*rhs.m_textProperties));

    m_wipeCopy = rhs.m_wipeCopy;
  }

  return *this;
}

// -----------------------------------------------------------------------

void GraphicsObject::Impl::makeSureHaveTextProperties()
{
  if(!m_textProperties)
  {
    m_textProperties.reset(new Impl::TextProperties());
  }
}

// -----------------------------------------------------------------------

/// boost::serialization support
template<class Archive>
void GraphicsObject::Impl::serialize(Archive& ar, unsigned int version)
{
  ar & m_visible & m_x & m_y & m_whateverAdjustVertOperatesOn &
    m_originX & m_originY & m_repOriginX & m_repOriginY &
    m_width & m_height & m_rotation & m_pattNo & m_alpha &
    m_clip & m_mono & m_invert &
    m_tint & m_colour & m_compositeMode & m_textProperties & m_wipeCopy;
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void GraphicsObject::Impl::serialize<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version);

template void GraphicsObject::Impl::serialize<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);

// -----------------------------------------------------------------------
// GraphicsObject::Impl::TextProperties
// -----------------------------------------------------------------------
template<class Archive>
void GraphicsObject::Impl::TextProperties::serialize(
  Archive& ar, unsigned int version)
{
  ar & value & textSize & xspace & yspace & vertical & colour & 
    shadowColour;
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void GraphicsObject::Impl::TextProperties::serialize<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version);

template void GraphicsObject::Impl::TextProperties::serialize<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);
