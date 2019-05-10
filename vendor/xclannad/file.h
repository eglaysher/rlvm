/*
 *  Copyright (C) 2000, 2007-   Kazunori Ueno(JAGARL) <jagarl@creator.club.ne.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
*/

#ifndef __KANON_FILE_H__
#define __KANON_FILE_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>

#include <vector>

#define INT_SIZE 4

class ARCINFO {
public:
	/* dest は256byte 程度の余裕があること */
	static void Extract2k(char*& dest, char*& src, char* destend, char* srcend);
};

class GRPCONV {
public:
  struct REGION {
    int x1, y1, x2, y2;
    int origin_x, origin_y;
    int Width() { return x2-x1;}
    int Height() { return y2-y1;}
    void FixVar(int& v, int& w) {
      if (v < 0) v = 0;
      if (v >= w) v = w-1;
    }
    // This only place I've found which relies on Fix() is the Kanon English
    // patch. I suspect that vaconv and other fan tools are/were broken.
    void Fix(int w, int h) {
      FixVar(x1,w);
      FixVar(x2,w);
      FixVar(y1,h);
      FixVar(y2,h);
      if (x1 > x2) x2 = x1;
      if (y1 > y2) y2 = y1;
    }

    bool operator<(const REGION& rhs) const;
  };

  std::vector<REGION> region_table;

	int width;
	int height;
	bool is_mask;

	const char* filename;
	const char* data;
	int datalen;

	int Width(void) { return width;}
	int Height(void) { return height;}
	bool IsMask(void) { return is_mask;}

	GRPCONV(void);
	virtual ~GRPCONV();
	void Init(const char* fname, const char* data, int dlen, int width, int height, bool is_mask);

	virtual bool Read(char* image) = 0;
	static GRPCONV* AssignConverter(const char* inbuf, int inlen, const char* fname);

	void CopyRGBA(char* image, const char* from);
	void CopyRGB(char* image, const char* from);
	void CopyRGBA_rev(char* image, const char* from);
	void CopyRGB_rev(char* image, const char* from);
};

#endif // !defined(__KANON_FILE_H__)
