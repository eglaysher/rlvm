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
const boost::shared_ptr<GraphicsObject::Impl> GraphicsObject::s_empty_impl(
  new GraphicsObject::Impl);

// -----------------------------------------------------------------------
// GraphicsObject::TextProperties
// -----------------------------------------------------------------------
GraphicsObject::Impl::TextProperties::TextProperties()
  : text_size(DEFAULT_TEXT_SIZE),
    xspace(DEFAULT_TEXT_XSPACE),
    yspace(DEFAULT_TEXT_YSPACE),
    vertical(DEFAULT_TEXT_VERTICAL),
    colour(DEFAULT_TEXT_COLOUR),
    shadow_colour(DEFAULT_TEXT_SHADOWCOLOUR)
{
}

// -----------------------------------------------------------------------
// GraphicsObject
// -----------------------------------------------------------------------
GraphicsObject::GraphicsObject()
  : impl_(s_empty_impl)
{}

// -----------------------------------------------------------------------

GraphicsObject::GraphicsObject(const GraphicsObject& rhs)
  : impl_(rhs.impl_)
{
  if(rhs.object_data_)
  {
    object_data_.reset(rhs.object_data_->clone());
    object_data_->setOwnedBy(*this);
  }
  else
    object_data_.reset();
}

// -----------------------------------------------------------------------

GraphicsObject::~GraphicsObject()
{}

// -----------------------------------------------------------------------

GraphicsObject& GraphicsObject::operator=(const GraphicsObject& obj)
{
  impl_ = obj.impl_;

  if(obj.object_data_)
  {
    object_data_.reset(obj.object_data_->clone());
    object_data_->setOwnedBy(*this);
  }
  else
    object_data_.reset();

  return *this;
}

// -----------------------------------------------------------------------

void GraphicsObject::setObjectData(GraphicsObjectData* obj)
{
  object_data_.reset(obj);
  object_data_->setOwnedBy(*this);
}

// -----------------------------------------------------------------------

void GraphicsObject::setVisible(const int in)
{
  makeImplUnique();
  impl_->visible_ = in;
}

// -----------------------------------------------------------------------

void GraphicsObject::setX(const int x)
{
  makeImplUnique();
  impl_->x_ = x;
}

// -----------------------------------------------------------------------

void GraphicsObject::setY(const int y)
{
  makeImplUnique();
  impl_->y_ = y;
}

// -----------------------------------------------------------------------

int GraphicsObject::xAdjustmentSum() const
{
  return std::accumulate(impl_->adjust_x_, impl_->adjust_x_ + 8, 0);
}

// -----------------------------------------------------------------------

void GraphicsObject::setXAdjustment(int idx, int x)
{
  makeImplUnique();
  impl_->adjust_x_[idx] = x;
}

// -----------------------------------------------------------------------

int GraphicsObject::yAdjustmentSum() const
{
  return std::accumulate(impl_->adjust_y_, impl_->adjust_y_ + 8, 0);
}

// -----------------------------------------------------------------------

void GraphicsObject::setYAdjustment(int idx, int y)
{
  makeImplUnique();
  impl_->adjust_y_[idx] = y;
}

// -----------------------------------------------------------------------

void GraphicsObject::setVert(const int vert)
{
  makeImplUnique();
  impl_->whatever_adjust_vert_operates_on_ = vert;
}

// -----------------------------------------------------------------------

void GraphicsObject::setXOrigin(const int x)
{
  makeImplUnique();
  impl_->origin_x_ = x;
}

// -----------------------------------------------------------------------

void GraphicsObject::setYOrigin(const int y)
{
  makeImplUnique();
  impl_->origin_y_ = y;
}

// -----------------------------------------------------------------------

void GraphicsObject::setWidth(const int in)
{
  makeImplUnique();
  impl_->width_ = in;
}

// -----------------------------------------------------------------------

void GraphicsObject::setHeight(const int in)
{
  makeImplUnique();
  impl_->height_ = in;
}

// -----------------------------------------------------------------------

void GraphicsObject::setRotation(const int in)
{
  makeImplUnique();
  impl_->rotation_ = in;
}

// -----------------------------------------------------------------------

int GraphicsObject::pixelWidth(RLMachine& machine) const
{
  // Calculate out the pixel width of the current object taking in the
  // width() scaling.
  if(hasObjectData())
    return object_data_->pixelWidth(machine, *this);
  else
    return 0;
}

// -----------------------------------------------------------------------

int GraphicsObject::pixelHeight(RLMachine& machine) const
{
  if(hasObjectData())
    return object_data_->pixelHeight(machine, *this);
  else
    return 0;
}

// -----------------------------------------------------------------------

void GraphicsObject::setPattNo(const int in)
{ makeImplUnique(); impl_->patt_no_ = in; }
void GraphicsObject::setMono(const int in)
{ makeImplUnique(); impl_->mono_ = in; }
void GraphicsObject::setInvert(const int in)
{ makeImplUnique(); impl_->invert_ = in; }
void GraphicsObject::setLight(const int in)
{ makeImplUnique(); impl_->light_ = in; }
void GraphicsObject::setTint(const RGBColour& color)
{ makeImplUnique(); impl_->tint_ = color; }
void GraphicsObject::setTintR(const int in)
{ makeImplUnique(); impl_->tint_.setRed(in); }
void GraphicsObject::setTintG(const int in)
{ makeImplUnique(); impl_->tint_.setGreen(in); }
void GraphicsObject::setTintB(const int in)
{ makeImplUnique(); impl_->tint_.setBlue(in); }
void GraphicsObject::setColour(const RGBAColour& colour)
{ makeImplUnique(); impl_->colour_ = colour; }
void GraphicsObject::setColourR(const int in)
{ makeImplUnique(); impl_->colour_.setRed(in); }
void GraphicsObject::setColourG(const int in)
{ makeImplUnique(); impl_->colour_.setGreen(in); }
void GraphicsObject::setColourB(const int in)
{ makeImplUnique(); impl_->colour_.setBlue(in); }
void GraphicsObject::setColourLevel(const int in)
{ makeImplUnique(); impl_->colour_.setAlpha(in); }

// -----------------------------------------------------------------------

void GraphicsObject::setCompositeMode(const int in)
{
  makeImplUnique();
  impl_->composite_mode_ = in;
}

// -----------------------------------------------------------------------

void GraphicsObject::setScrollRateX(const int x)
{ makeImplUnique(); impl_->scroll_rate_x_ = x; }
void GraphicsObject::setScrollRateY(const int y)
{ makeImplUnique(); impl_->scroll_rate_y_ = y; }

// -----------------------------------------------------------------------

void GraphicsObject::setAlpha(const int alpha)
{
  makeImplUnique();
  impl_->alpha_ = alpha;
}

// -----------------------------------------------------------------------

void GraphicsObject::clearClip()
{
  makeImplUnique();
  impl_->clip_ = EMPTY_CLIP;
}

// -----------------------------------------------------------------------

void GraphicsObject::setClip(const Rect& rect)
{
  makeImplUnique();
  impl_->clip_ = rect;
}

// -----------------------------------------------------------------------

GraphicsObjectData& GraphicsObject::objectData()
{
  if(object_data_)
    return *object_data_;
  else
  {
    throw rlvm::Exception("null object data");
  }
}

// -----------------------------------------------------------------------

void GraphicsObject::setWipeCopy(const int wipe_copy)
{
  makeImplUnique();
  impl_->wipe_copy_ = wipe_copy;
}

// -----------------------------------------------------------------------

void GraphicsObject::setTextText(const std::string& utf8str)
{
  makeImplUnique();
  impl_->makeSureHaveTextProperties();
  impl_->text_properties_->value = utf8str;
}

// -----------------------------------------------------------------------

const std::string& GraphicsObject::textText() const
{
  static const std::string empty = "";

  if(impl_->text_properties_)
    return impl_->text_properties_->value;
  else
    return empty;
}

// -----------------------------------------------------------------------

int GraphicsObject::textSize() const
{
  if(impl_->text_properties_)
    return impl_->text_properties_->text_size;
  else
    return DEFAULT_TEXT_SIZE;
}

// -----------------------------------------------------------------------

int GraphicsObject::textXSpace() const
{
  if(impl_->text_properties_)
    return impl_->text_properties_->xspace;
  else
    return DEFAULT_TEXT_XSPACE;
}

// -----------------------------------------------------------------------

int GraphicsObject::textYSpace() const
{
  if(impl_->text_properties_)
    return impl_->text_properties_->yspace;
  else
    return DEFAULT_TEXT_YSPACE;
}

// -----------------------------------------------------------------------

int GraphicsObject::textVertical() const
{
  if(impl_->text_properties_)
    return impl_->text_properties_->vertical;
  else
    return DEFAULT_TEXT_VERTICAL;
}

// -----------------------------------------------------------------------

int GraphicsObject::textColour() const
{
  if(impl_->text_properties_)
    return impl_->text_properties_->colour;
  else
    return DEFAULT_TEXT_COLOUR;
}

// -----------------------------------------------------------------------

int GraphicsObject::textShadowColour() const
{
  if(impl_->text_properties_)
    return impl_->text_properties_->shadow_colour;
  else
    return DEFAULT_TEXT_SHADOWCOLOUR;
}

// -----------------------------------------------------------------------

void GraphicsObject::setTextOps(
  int size, int xspace, int yspace, int vertical, int colour, int shadow)
{
  makeImplUnique();

  impl_->makeSureHaveTextProperties();
  impl_->text_properties_->text_size = size;
  impl_->text_properties_->xspace = xspace;
  impl_->text_properties_->yspace = yspace;
  impl_->text_properties_->vertical = vertical;
  impl_->text_properties_->colour = colour;
  impl_->text_properties_->shadow_colour = shadow;
}

// -----------------------------------------------------------------------

void GraphicsObject::makeImplUnique()
{
  if(!impl_.unique())
  {
    impl_.reset(new Impl(*impl_));
  }
}

// -----------------------------------------------------------------------

void GraphicsObject::render(RLMachine& machine)
{
  if(object_data_ && visible())
  {
    object_data_->render(machine, *this);
  }
}

// -----------------------------------------------------------------------

void GraphicsObject::deleteObject()
{
  object_data_.reset();
}

// -----------------------------------------------------------------------

void GraphicsObject::clearObject()
{
  impl_ = s_empty_impl;
  object_data_.reset();
}

// -----------------------------------------------------------------------

void GraphicsObject::execute(RLMachine& machine)
{
  if(object_data_)
  {
    object_data_->execute(machine);
  }
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsObject::serialize(Archive& ar, unsigned int version)
{
  ar & impl_ & object_data_;
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
  : visible_(false),
    x_(0), y_(0),
    whatever_adjust_vert_operates_on_(0),
    origin_x_(0), origin_y_(0),
    rep_origin_x_(0), rep_origin_y_(0),

    // Width and height are percentages
    width_(100), height_(100),

    rotation_(0),

    patt_no_(0), alpha_(255),

    clip_(EMPTY_CLIP),

    mono_(0), invert_(0), light_(0),
    // Do the rest later.
    tint_(RGBColour::White()),
    colour_(RGBAColour::White()),
    composite_mode_(0),
    scroll_rate_x_(0),
    scroll_rate_y_(0),
    wipe_copy_(0)
{
  // Regretfully, we can't do this in the initializer list.
  fill(adjust_x_, adjust_x_ + 8, 0);
  fill(adjust_y_, adjust_y_ + 8, 0);
}

// -----------------------------------------------------------------------

GraphicsObject::Impl::Impl(const Impl& rhs)
  : visible_(rhs.visible_), x_(rhs.x_), y_(rhs.y_),
    whatever_adjust_vert_operates_on_(rhs.whatever_adjust_vert_operates_on_),
    origin_x_(rhs.origin_x_), origin_y_(rhs.origin_y_),
    rep_origin_x_(rhs.rep_origin_x_), rep_origin_y_(rhs.rep_origin_y_),
    width_(rhs.width_), height_(rhs.height_),
    rotation_(rhs.rotation_),
    patt_no_(rhs.patt_no_), alpha_(rhs.alpha_),
    clip_(rhs.clip_),
    mono_(rhs.mono_), invert_(rhs.invert_),
    light_(rhs.light_), tint_(rhs.tint_), colour_(rhs.colour_),
    composite_mode_(rhs.composite_mode_),
    scroll_rate_x_(rhs.scroll_rate_x_),
    scroll_rate_y_(rhs.scroll_rate_y_),
    wipe_copy_(0)
{
  if(rhs.text_properties_)
    text_properties_.reset(new TextProperties(*rhs.text_properties_));

  copy(rhs.adjust_x_, rhs.adjust_x_ + 8, adjust_x_);
  copy(rhs.adjust_y_, rhs.adjust_y_ + 8, adjust_y_);
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
    visible_ = rhs.visible_;
    x_ = rhs.x_;
    y_ = rhs.y_;

    copy(rhs.adjust_x_, rhs.adjust_x_ + 8, adjust_x_);
    copy(rhs.adjust_y_, rhs.adjust_y_ + 8, adjust_y_);

    whatever_adjust_vert_operates_on_ = rhs.whatever_adjust_vert_operates_on_;
    origin_x_ = rhs.origin_x_;
    origin_y_ = rhs.origin_y_;
    rep_origin_x_ = rhs.rep_origin_x_;
    rep_origin_y_ = rhs.rep_origin_y_;
    width_ = rhs.width_;
    height_ = rhs.height_;
    rotation_ = rhs.rotation_;

    patt_no_ = rhs.patt_no_;
    alpha_ = rhs.alpha_;
    clip_ = rhs.clip_;
    mono_ = rhs.mono_;
    invert_ = rhs.invert_;
    light_ = rhs.light_;
    tint_ = rhs.tint_;

    colour_ = rhs.colour_;

    composite_mode_ = rhs.composite_mode_;
    scroll_rate_x_ = rhs.scroll_rate_x_;
    scroll_rate_y_ = rhs.scroll_rate_y_;

    if(rhs.text_properties_)
      text_properties_.reset(new TextProperties(*rhs.text_properties_));

    wipe_copy_ = rhs.wipe_copy_;
  }

  return *this;
}

// -----------------------------------------------------------------------

void GraphicsObject::Impl::makeSureHaveTextProperties()
{
  if(!text_properties_)
  {
    text_properties_.reset(new Impl::TextProperties());
  }
}

// -----------------------------------------------------------------------

/// boost::serialization support
template<class Archive>
void GraphicsObject::Impl::serialize(Archive& ar, unsigned int version)
{
  ar & visible_ & x_ & y_ & whatever_adjust_vert_operates_on_ &
    origin_x_ & origin_y_ & rep_origin_x_ & rep_origin_y_ &
    width_ & height_ & rotation_ & patt_no_ & alpha_ &
    clip_ & mono_ & invert_ &
    tint_ & colour_ & composite_mode_ & text_properties_ & wipe_copy_;
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
  ar & value & text_size & xspace & yspace & vertical & colour &
    shadow_colour;
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void GraphicsObject::Impl::TextProperties::serialize<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version);

template void GraphicsObject::Impl::TextProperties::serialize<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);
