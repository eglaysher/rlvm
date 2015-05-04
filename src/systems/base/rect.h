// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_RECT_H_
#define SRC_SYSTEMS_BASE_RECT_H_

#include <boost/serialization/access.hpp>
#include <iosfwd>

class Rect;
class Size;

class Point {
 public:
  Point() : x_(0), y_(0) {}
  Point(int x, int y) : x_(x), y_(y) {}

  int x() const { return x_; }
  void set_x(const int in) { x_ = in; }
  int y() const { return y_; }
  void set_y(const int in) { y_ = in; }

  bool is_empty() const { return x_ == 0 && y_ == 0; }

  Point& operator+=(const Point& rhs) {
    x_ += rhs.x_;
    y_ += rhs.y_;
    return *this;
  }

  Point& operator-=(const Point& rhs) {
    x_ -= rhs.x_;
    y_ -= rhs.y_;
    return *this;
  }

  Point operator+(const Point& rhs) const {
    return Point(x_ + rhs.x_, y_ + rhs.y_);
  }

  Point operator+(const Size& rhs) const;
  Point operator-(const Size& rhs) const;

  Size operator-(const Point& rhs) const;

  bool operator==(const Point& rhs) const {
    return x_ == rhs.x_ && y_ == rhs.y_;
  }

  bool operator!=(const Point& rhs) const {
    return x_ != rhs.x_ || y_ != rhs.y_;
  }

 private:
  int x_;
  int y_;

  // boost::serialization support
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, unsigned int version) {
    ar& x_& y_;
  }
};

class Size {
 public:
  Size() : width_(0), height_(0) {}
  Size(int width, int height) : width_(width), height_(height) {}

  int width() const { return width_; }
  int height() const { return height_; }

  void set_width(const int width) { width_ = width; }
  void set_height(const int height) { height_ = height; }

  bool is_empty() const { return width_ == 0 && height_ == 0; }

  // Returns a rect of our size that is centered in rect |r|. Can return a rect
  // larger than |r|.
  Rect CenteredIn(const Rect& r) const;

  Size& operator+=(const Size& rhs) {
    width_ += rhs.width_;
    height_ += rhs.height_;
    return *this;
  }

  Size& operator-=(const Size& rhs) {
    width_ -= rhs.width_;
    height_ -= rhs.height_;
    return *this;
  }

  Size operator+(const Size& rhs) const {
    return Size(width_ + rhs.width_, height_ + rhs.height_);
  }

  Size operator-(const Size& rhs) const {
    return Size(width_ - rhs.width_, height_ - rhs.height_);
  }

  Size operator*(float factor) const {
    return Size(static_cast<int>(width_ * factor),
                static_cast<int>(height_ * factor));
  }

  Size operator/(int denominator) const {
    return Size(width_ / denominator, height_ / denominator);
  }

  bool operator==(const Size& rhs) const {
    return width_ == rhs.width_ && height_ == rhs.height_;
  }

  bool operator!=(const Size& rhs) const {
    return width_ != rhs.width_ || height_ != rhs.height_;
  }

  // Returns a size that is the max of both size's widths and heights.
  Size SizeUnion(const Size& rhs) const;

 private:
  int width_;
  int height_;

  // boost::serialization support
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, unsigned int version) {
    ar& width_& height_;
  }
};

class Rect {
 public:
  Rect() {}
  Rect(const Point& point1, const Point& point2)
      : origin_(point1),
        size_(point2.x() - point1.x(), point2.y() - point1.y()) {}
  Rect(const Point& origin, const Size& size) : origin_(origin), size_(size) {}
  Rect(const int x, const int y, const Size& size)
      : origin_(x, y), size_(size) {}

  static Rect GRP(const int x1, const int y1, const int x2, const int y2) {
    return Rect(Point(x1, y1), Point(x2, y2));
  }

  static Rect REC(const int x, const int y, const int width, const int height) {
    return Rect(Point(x, y), Size(width, height));
  }

  int x() const { return origin_.x(); }
  void set_x(const int in) { origin_.set_x(in); }
  int y() const { return origin_.y(); }

  void set_y(const int in) { origin_.set_y(in); }
  int x2() const { return origin_.x() + size_.width(); }
  void set_x2(const int in) { size_.set_width(in - origin_.x()); }
  int y2() const { return origin_.y() + size_.height(); }
  void set_y2(const int in) { size_.set_height(in - origin_.y()); }

  int width() const { return size_.width(); }
  int height() const { return size_.height(); }
  const Point lower_right() const { return origin_ + size_; }

  const Size& size() const { return size_; }
  const Point& origin() const { return origin_; }

  bool is_empty() const { return origin_.is_empty() && size_.is_empty(); }

  // Whether |loc| is inside this Rect.
  bool Contains(const Point& loc);

  // Whether we intersect with |rhs|.
  bool Intersects(const Rect& rhs) const;

  // Contains the intersection of two overlapping (or subsumed) rectangles.
  Rect Intersection(const Rect& rhs) const;

  // Contains the union of two overlapping rectangles.
  Rect RectUnion(const Rect& rhs) const;

  // Calculate the rectangle |rhs| in terms of this rectangle as the origin.
  Rect GetInsetRectangle(const Rect& rhs) const;

  // Apply the inset rect to our rect.
  Rect ApplyInset(const Rect& inset) const;

  bool operator==(const Rect& rhs) const {
    return origin_ == rhs.origin_ && size_ == rhs.size_;
  }

  bool operator!=(const Rect& rhs) const {
    return origin_ != rhs.origin_ || size_ != rhs.size_;
  }

 private:
  Point origin_;
  Size size_;

  // boost::serialization support
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, unsigned int version) {
    ar& origin_& size_;
  }
};  // end of class Rect

// -----------------------------------------------------------------------

inline Point Point::operator+(const Size& rhs) const {
  return Point(x_ + rhs.width(), y_ + rhs.height());
}

inline Point Point::operator-(const Size& rhs) const {
  return Point(x_ - rhs.width(), y_ - rhs.height());
}

inline Size Point::operator-(const Point& rhs) const {
  return Size(x_ - rhs.x_, y_ - rhs.y_);
}

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const Size& s);
std::ostream& operator<<(std::ostream& os, const Point& p);
std::ostream& operator<<(std::ostream& os, const Rect& r);

#endif  // SRC_SYSTEMS_BASE_RECT_H_
