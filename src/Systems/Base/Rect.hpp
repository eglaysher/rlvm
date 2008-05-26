// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//

#ifndef __Rect_hpp__
#define __Rect_hpp__

class Point
{
public:
  Point() : x_(0), y_(0) {}
  Point(int x, int y) : x_(x), y_(y) {}

  int x() const { return x_; }
  int y() const { return y_; }

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

  Point operator-(const Point& rhs) const {
    return Point(x_ - rhs.x_, y_ - rhs.y_);
  }

  bool operator==(const Point& rhs) const {
    return x_ == rhs.x_ && y_ == rhs.y_;
  }

  bool operator!=(const Point& rhs) const {
    return x_ != rhs.x_ && y_ != rhs.y_;
  }

private:
  int x_;
  int y_;
};

/**
 * Backend independent representation of a size.
 */
class Size
{
public:
  Size() : width_(0), height_(0) {}
  Size(int width, int height) : width_(width), height_(height) {}

  int width() const { return width_; }
  int height() const { return height_; }

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

  bool operator==(const Size& rhs) const {
    return width_ == rhs.width_ && height_ == rhs.height_;
  }

  bool operator!=(const Size& rhs) const {
    return width_ != rhs.width_ && height_ != rhs.height_;
  }

private:
  int width_;
  int height_;
};

/**
 *
 */
class Rect
{
public: 
  Rect();
  Rect(const Point& point1, const Point& point2);
  Rect(const Point& point, const Size& size);
  ~Rect();

  int x() const { return origin_.x(); }
  int y() const { return origin_.y(); }

  int width() const { return size_.width(); }
  int height() const { return size_.height(); }
  const Size& size() const { return size_; }

private:
  Point origin_;
  Size size_;
};	// end of class Rect


#endif
