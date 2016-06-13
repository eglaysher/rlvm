// -----------------------------------------------------------------------

// Do a more-portable Endian check
bool IsBigEndian()
{
   short word = 0x4321;
   if((*(char *)& word) != 0x21 )
     return true;
   else 
     return false;
}

int g_isBigEndian = IsBigEndian();

// -----------------------------------------------------------------------

/*  file.cc  : KANON の圧縮ファイル・PDT ファイル（画像ファイル）の展開の
 *            ためのメソッド
 *     class ARCINFO : 書庫ファイルの中の１つのファイルを扱うクラス
 *     class PDTCONV : PDT ファイルの展開を行う。
 *
 */

/*
 *
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_MMAP
#  ifdef MACOSX
#    undef HAVE_MMAP
#  endif /* MACOSX */
#endif /* HAVE_MMAP */

#include <ctype.h>
#include <fcntl.h>
#ifdef WIN32
# include <io.h>
#else
# include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#if HAVE_MMAP
#include<sys/mman.h>
#endif /* HAVE_MMAP */

#ifdef WIN32
# include <direct.h>
# include "dirent_impl.h"
# ifdef MAX_PATH
#  define PATH_MAX MAX_PATH
# else
#  define PATH_MAX 260
# endif
#else
# include <dirent.h>
# ifndef PATH_MAX
#   define PATH_MAX 1024
# endif
#endif

# define NAMLEN(dirent) strlen((dirent)->d_name)

#include "file.h"
#include "endian.hpp"

#include <set>
#include <tuple>

using namespace std;

// -----------------------------------------------------------------------

bool GRPCONV::REGION::operator<(const REGION& rhs) const {
  return
      std::tie(x1, y1, x2, y2, origin_x, origin_y) <
    std::tie(rhs.x1, rhs.y1, rhs.x2, rhs.y2, rhs.origin_x, rhs.origin_y);
}

/**********************************************
**
**	画像展開系クラスの定義、実装
**
***********************************************
*/
GRPCONV::GRPCONV(void) {
	filename = 0;
	data = 0;
}
GRPCONV::~GRPCONV() {
	if (filename) delete[] filename;
}
void GRPCONV::Init(const char* f, const char* d, int dlen, int w, int h, bool is_m) {
	if (filename) delete[] filename;
	if (f == 0) {
		char* fn = new char[1];
		fn[0] = 0;
		filename = fn;
	} else {
		char* fn = new char[strlen(f)+1];
		strcpy(fn,f);
		filename = fn;
	}

	data = d;
	datalen = dlen;
	width = w;
	height = h;
	is_mask = is_m;
}
class PDTCONV : public GRPCONV {
	bool Read_PDT10(char* image);
	bool Read_PDT11(char* image);
public:
	PDTCONV(const char* _inbuf, int inlen, const char* fname);
	~PDTCONV() {}
	bool Read(char* image);
};
class G00CONV : public GRPCONV {
	void Copy_16bpp(char* image, int x, int y, const char* src, int bpl, int h);
	void Copy_32bpp(char* image, int x, int y, const char* src, int bpl, int h);
	bool Read_Type0(char* image);
	bool Read_Type1(char* image);
	bool Read_Type2(char* image);
public:
	G00CONV(const char* _inbuf, int _inlen, const char* fname);
	~G00CONV() { }
	bool Read(char* image);
};

class BMPCONV : public GRPCONV {
public:
	BMPCONV(const char* _inbuf, int _inlen, const char* fname);
	~BMPCONV() {};
	bool Read(char* image);
};

GRPCONV* GRPCONV::AssignConverter(const char* inbuf, int inlen, const char* fname) {
	/* ファイルの内容に応じたコンバーターを割り当てる */
	GRPCONV* conv = 0;
	if (inlen < 10) return 0; /* invalid file */
	if (strncmp(inbuf, "PDT10", 5) == 0 || strncmp(inbuf, "PDT11", 5) == 0) { /* PDT10 or PDT11 */
		conv = new PDTCONV(inbuf, inlen, fname);
		if (conv->data == 0) { delete conv; conv = 0;}
	}
	if (conv == 0 && inbuf[0]=='B' && inbuf[1]=='M' && read_little_endian_int(inbuf+10)==0x36 && read_little_endian_int(inbuf+14) == 0x28) { // Windows BMP
		conv = new BMPCONV(inbuf, inlen, fname);
		if (conv->data == 0) { delete conv; conv = 0;}
	}
	if (conv == 0 && (inbuf[0] == 0 || inbuf[0] == 1 || inbuf[0] == 2)) { /* G00 */
		conv = new G00CONV(inbuf, inlen, fname);
		if (conv->data == 0) { delete conv; conv = 0;}
	}
	return conv;
}

PDTCONV::PDTCONV(const char* _inbuf, int _inlen,const char* filename) {
//	PDT FILE のヘッダ
//	+00 'PDT10'	(PDT11 は未対応)
//	+08 ファイルサイズ (無視)
//	+0C width (ほぼすべて、640)
//	+10 height(ほぼすべて、480)
//	+14 (mask の) x 座標 (実際は無視・・・全ファイルで 0 )
//	+1c (mask の) y座標 (実際は無視 ・・・全ファイルで 0 )
//	+20 mask が存在すれば、mask へのポインタ

	/* ヘッダチェック */
	if (_inlen < 0x20) {
		fprintf(stderr, "Invalid PDT file %s : size is too small\n",filename);
		return;
	}
	if (strncmp(_inbuf, "PDT10", 5) != 0 && strncmp(_inbuf, "PDT11", 5) != 0) {
		fprintf(stderr, "Invalid PDT file %s : not 'PDT10 / PDT11' file.\n", filename);
		return;
	}
	if (size_t(_inlen) != size_t(read_little_endian_int(_inbuf+0x08))) {
		fprintf(stderr, "Invalid archive file %s : invalid header.(size)\n",
			filename);
		return;
	}

	int w = read_little_endian_int(_inbuf+0x0c);
	int h = read_little_endian_int(_inbuf+0x10);
	int mask_pt = read_little_endian_int(_inbuf + 0x1c);
	Init(filename, _inbuf, _inlen, w, h, mask_pt ? true : false);

	return;
}


G00CONV::G00CONV(const char* _inbuf, int _inlen, const char* filename) {
//	G00 FILE のヘッダ
//	+00 type (1, 2)
//	+01: width(word)
//	+03: height(word)
//	type 1: (color table 付き LZ 圧縮 ; PDT11 に対応)
//		+05: 圧縮サイズ(dword) ; +5 するとデータ全体のサイズ
//		+09: 展開後サイズ(dword)
//	type 2: (マスク可、画像を矩形領域に分割してそれぞれ圧縮)
//		+05: index size
//		+09: index table(each size is 0x18)
//			+00
//			
//		+09+0x18*size+00: data size
//		+09+0x18*size+04: out size
//		+09+0x18*size+08: (data top)
//

	/* データから情報読み込み */
	int type = *_inbuf;

	int w = read_little_endian_short(_inbuf+1);
	int h = read_little_endian_short(_inbuf+3);
	if (w < 0 || h < 0) return;

	if (type == 0 || type == 1) { // color table 付き圧縮
		if (_inlen < 13) {
			fprintf(stderr, "Invalid G00 file %s : size is too small\n",filename);
			return;
		}
		int data_sz = read_little_endian_int(_inbuf+5);

		if (_inlen != data_sz+5) {
			fprintf(stderr, "Invalid archive file %s : invalid header.(size)\n",
				filename);
			return;
		}
		Init(filename, _inbuf, _inlen, w, h, false);
	} else if (type == 2) { // color table なし、マスク付き可の圧縮

		int head_size = read_little_endian_short(_inbuf+5);
		if (head_size < 0 || head_size*24 > _inlen) return;

    region_table = vector<REGION>(head_size);

    int real_region_count = 0;
    std::set<REGION> unique_regions;
    const char* head = _inbuf + 9;
    bool overlaid_image = head_size > 1;
    for (int i = 0; i < head_size; i++) {
      region_table[i].x1 = read_little_endian_int(head+0);
      region_table[i].y1 = read_little_endian_int(head+4);
      region_table[i].x2 = read_little_endian_int(head+8);
      region_table[i].y2 = read_little_endian_int(head+12);
      region_table[i].origin_x = read_little_endian_int(head+16);
      region_table[i].origin_y = read_little_endian_int(head+20);
      region_table[i].Fix(w, h);
      if (region_table[i].Width() &&
          region_table[i].Height()) {
        unique_regions.insert(region_table[i]);
        real_region_count++;
      }

      head += 24;
    }

    if (real_region_count > 1 && unique_regions.size() == 1) {
      // This is one of those newer images where each region is the size of
      // width/height and is stacked on top of each other. We therefore have to
      // munge the height and the region table so each region gets its own
      // space on the canvas.
      for (int i = 0; i < head_size; ++i) {
        region_table[i].y1 += i * h;
        region_table[i].y2 += i * h;
      }
      h = h * head_size;
    }

		const char* data_top = _inbuf + 9 + head_size*24;
		int data_sz = read_little_endian_int(data_top);
		if (_inbuf + _inlen != data_top + data_sz) {
			fprintf(stderr, "Invalid archive file %s : invalid header.(size)\n",
				filename);
			return;
		}
		Init(filename, _inbuf, _inlen, w, h, true);
	}
	return;
}

bool G00CONV::Read(char* image) {
	if (data == 0) return false;
	/* header 識別 */
	int type = *data;
	if (type == 0) return Read_Type0(image);
	else if (type == 1) return Read_Type1(image);
	else if (type == 2) return Read_Type2(image);
  return false;
}

/* 一般的な LZ 圧縮の展開ルーチン */
/* datasize はデータの大きさ、char / short / int を想定 */
/* datatype は Copy1Pixel (1データのコピー)及び ExtractData(LZ 圧縮の情報を得る
** というメソッドを実装したクラス */
static int bitrev_table[256] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff};
template<class DataType, class DataSize> inline int lzExtract(DataType& datatype,const char*& src, char*& dest, const char* srcend, char* destend) {
	int count = 0;
	const char* lsrcend = srcend; char* ldestend = destend;
	const char* lsrc = src; char* ldest = dest;

	if (lsrc+50 < lsrcend && ldest+1024 < ldestend) {
		/* まず、範囲チェックを緩くして高速なルーチンを使う */
		lsrcend -= 50;
		ldestend += 1024;
		while (ldest < ldestend && lsrc < lsrcend) {
			count += 8;
			int flag = int(*(unsigned char*)lsrc++);
			if (datatype.IsRev()) flag = bitrev_table[flag];
			int i; for (i=0; i<8; i++) {
				if (flag & 0x80) {
					datatype.Copy1Pixel(lsrc, ldest);
				} else {
					int data, size;
					datatype.ExtractData(lsrc, data, size);
					DataSize* p_dest = ((DataSize*)ldest) - data;
					int k; for (k=0; k<size; k++) {
						p_dest[data] = *p_dest;
						p_dest++;
					}
					ldest += size*sizeof(DataSize);
				}
				flag <<= 1;
			}
		}
		lsrcend += 50;
		ldestend += 1024;
	}
	/* 残りを変換 */
	while (ldest < ldestend && lsrc < lsrcend) {
		count += 8;
		int flag = int(*(unsigned char*)lsrc++);
		if (datatype.IsRev()) flag = bitrev_table[flag];
		int i; for (i=0; i<8 && ldest < ldestend && lsrc < lsrcend; i++) {
			if (flag & 0x80) {
				datatype.Copy1Pixel(lsrc, ldest);
			} else {
				int data, size;
				datatype.ExtractData(lsrc, data, size);
				DataSize* p_dest = ((DataSize*)ldest) - data;
				int k; for (k=0; k<size; k++) {
					p_dest[data] = *p_dest;
					p_dest++;
				}
				ldest += size*sizeof(DataSize);
			}
			flag <<= 1;
		}
	}
	dest=ldest; src=lsrc;
	return 0;
}
/* 引数を減らすためのwrapper */
template<class DataType, class DataSize> inline int lzExtract(DataType datatype, DataSize datasize ,const char*& src, char*& dest, const char* srcend, char* destend) {
	return lzExtract<DataType, DataSize>(datatype,src,dest,srcend,destend);
}

/* 普通の PDT */
class Extract_DataType {
public:
	static void ExtractData(const char*& lsrc, int& data, int& size) {
		data = read_little_endian_short(lsrc) & 0xffff;
		size = (data & 0x0f) + 1;
		data = (data>>4)+1;
		lsrc += 2;
	}
	static void Copy1Pixel(const char*& lsrc, char*& ldest) {
      if(g_isBigEndian)
      {
 		ldest[3] = lsrc[0];
 		ldest[2] = lsrc[1];
 		ldest[1] = lsrc[2];
 		ldest[0] = 0;
      }
      else
      {
		*(int*)ldest = read_little_endian_int(lsrc); ldest[3]=0;
      }

      lsrc += 3; ldest += 4;
	}
	static int IsRev(void) { return 0; }
};

/* PDT11 の第一段階変換 */
class Extract_DataType_PDT11 {
	int* index_table;
public:
	Extract_DataType_PDT11(int* it) { index_table = it; }
	void ExtractData(const char*& lsrc, int& data, int& size) {
		data = int(*(const unsigned char*)lsrc);
		size = (data>>4) + 2;
		data = index_table[data&0x0f];
		lsrc++;
	}
	static void Copy1Pixel(const char*& lsrc, char*& ldest) {
		*ldest = *lsrc;
		ldest++; lsrc++;
	}
	static int IsRev(void) { return 0; }
};
/* マスク用 */
class Extract_DataType_Mask {
public:
	void ExtractData(const char*& lsrc, int& data, int& size) {
		int d = read_little_endian_short(lsrc) & 0xffff;
		size = (d & 0xff) + 2;
		data = (d>>8)+1;
		lsrc += 2;
	}
	static void Copy1Pixel(const char*& lsrc, char*& ldest) {
		*ldest = *lsrc;
		ldest++; lsrc++;
	}
	static int IsRev(void) { return 0; }
};
/* avg2000 のシナリオ用 */
class Extract_DataType_SCN2k {
public:
	void ExtractData(const char*& lsrc, int& data, int& size) {
		data = read_little_endian_short(lsrc) & 0xffff;
		size = (data&0x0f) + 2;
		data = (data>>4);
		lsrc+= 2;
	}
	static void Copy1Pixel(const char*& lsrc, char*& ldest) {
		*ldest = *lsrc;
		ldest++; lsrc++;
	}
	static int IsRev(void) { return 1; }
};
/* ReadLive の type0 */
class Extract_DataType_G00Type0 {
public:
	static void ExtractData(const char*& lsrc, int& data, int& size) {
		data = read_little_endian_short(lsrc) & 0xffff;
		size = ((data & 0x0f)+ 1) * 3;
		data = (data>>4) * 3;
		lsrc += 2;
	}
	static void Copy1Pixel(const char*& lsrc, char*& ldest) {
      if(g_isBigEndian)
      {
		ldest[0] = lsrc[0];
		ldest[1] = lsrc[1];
		ldest[2] = lsrc[2];
      }
      else
      {
		*(int*)ldest = *(int*)lsrc;
      }

      lsrc += 3; ldest += 3;
	}
	static int IsRev(void) { return 1; }
};

bool PDTCONV::Read(char* image) {
	if (data == 0) return false;

	if (strncmp(data, "PDT10", 5) == 0) {
		if (! Read_PDT10(image)) return false;
	} else if (strncmp(data, "PDT11", 5) == 0) {
		if (! Read_PDT11(image)) return false;
	}
	if (! is_mask) return true;
	// マスク読み込み
	int mask_pt = read_little_endian_int(data + 0x1c);
	char* buf = new char[width*height+1024];
	const char* src = data + mask_pt;
	const char* srcend = data + datalen;
	char* dest = buf;
	char* destend = buf + width*height;
	while(lzExtract(Extract_DataType_Mask(), char(), src, dest, srcend, destend)) ;
	int i; int len = width*height;
	src = buf; dest = image;
	for (i=0; i<len; i++) {
		*(int*)dest |= int(*(unsigned char*)src) << 24;
		src++;
		dest += 4;
	}
	delete[] buf;
	return true;
}

bool PDTCONV::Read_PDT10(char* image) {
	int mask_pt = read_little_endian_int(data + 0x1c);

	const char* src = data + 0x20;
	const char* srcend;
	if (mask_pt == 0) srcend = data + datalen;
	else srcend = data + mask_pt;

	char* dest = image;
	char* destend;

	destend = image + width*height*4;
	while(lzExtract(Extract_DataType(), int(), src, dest, srcend, destend)) ;
	return true;
}

bool PDTCONV::Read_PDT11(char* image) {
	int index_table[16];
	int color_table[256];
	int i;
	for (i=0; i<16; i++)
		index_table[i] = read_little_endian_int(data + 0x420 + i*4);

	int mask_pt = read_little_endian_int(data + 0x1c);

	const char* src = data + 0x460;
	const char* srcend;
	if (mask_pt == 0) srcend = data + datalen;
	else srcend = data + mask_pt;

	char* dest = image;
	char* destend = image + width*height;

	while(lzExtract(Extract_DataType_PDT11(index_table), char(), src, dest, srcend, destend)) ;

	const char* cur = data + 0x20;
	for (i=0; i<256; i++) {
		color_table[i] =  read_little_endian_int(cur);
		cur += 4;
	}
	src = image + width*height;
	int* desti = (int*)(image + width*height*4);
	while(desti != (int*)image)
		*--desti = color_table[*(unsigned char*)--src];
	return true;
}

/* dest は dest_end よりも 256 byte 以上先まで
** 書き込み可能であること。
*/
void ARCINFO::Extract2k(char*& dest_start, char*& src_start, char* dest_end, char* src_end) {
	const char* src = src_start;
	while (lzExtract(Extract_DataType_SCN2k(), char(), src, dest_start, src_end, dest_end)) ;
	src_start = (char*)src;
	return;
}

bool G00CONV::Read_Type0(char* image) {
	int uncompress_size = read_little_endian_int(data+9);
	char* uncompress_data = new char[uncompress_size+1024];

	// まず展開
	const char* src = data + 13;
	const char* srcend = data + datalen;
	char* dest = uncompress_data;
	char* dstend = uncompress_data + uncompress_size;
	while(lzExtract(Extract_DataType_G00Type0(), char(), src, dest, srcend, dstend));
	// image にコピー
	CopyRGB(image, uncompress_data);
	delete[] uncompress_data;
	return true;
}

bool G00CONV::Read_Type1(char* image) {
	int i;
	int uncompress_size = read_little_endian_int(data+9) + 1;
	char* uncompress_data = new char[uncompress_size + 1024];

	// まず、展開
	const char* src = data + 13;
	const char* srcend = data + datalen;
	char* dest = uncompress_data;
	char* destend = uncompress_data + uncompress_size;

	while(lzExtract(Extract_DataType_SCN2k(), char(), src, dest, srcend, destend));

	int colortable[256];
	memset(colortable, 0, sizeof(int)*256);
	int colortable_len = read_little_endian_short(uncompress_data);
	if (colortable_len > 256) colortable_len = 256;
	if (colortable_len < 0) colortable_len = 0;
	for (i=0; i<colortable_len; i++) {
		colortable[i] = read_little_endian_int(uncompress_data+2+i*4);
	}
	src = uncompress_data + 2 + read_little_endian_short(uncompress_data)*4;
	srcend = uncompress_data + uncompress_size;
	dest = image; destend = image + width*height*4;
	while(dest < destend && src < srcend) {
		*(int*)dest = colortable[*(unsigned char*)src];
		dest += 4; src ++;
	}
	delete[] uncompress_data;
	return true;
}

bool G00CONV::Read_Type2(char* image) {
	memset(image, 0, width*height*4);

	int region_deal = read_little_endian_int(data+5);
	const char* head = data + 9 + (region_deal * 24);

	// 展開
	int uncompress_size = read_little_endian_int(head+4);
	char* uncompress_data = new char[uncompress_size + 1024];

	const char* src = head + 8;
	const char* srcend = data + datalen;
	char* dest = uncompress_data;
	char* destend = uncompress_data + uncompress_size;
	while(lzExtract(Extract_DataType_SCN2k(), char(), src, dest, srcend, destend));

	/* region_deal2 == region_deal のはず……*/
	int region_deal2 = read_little_endian_int(uncompress_data);
	if (region_deal > region_deal2) region_deal = region_deal2;

	for (int i = 0; i < region_deal; i++) {
		int offset = read_little_endian_int(uncompress_data + i*8 + 4);
		int length = read_little_endian_int(uncompress_data + i*8 + 8);
		src = (const char*)(uncompress_data + offset + 0x74);
		srcend = (const char*)(uncompress_data + offset + length);
		while(src < srcend) {
			int x, y, w, h;
			/* コピーする領域を得る */
			x = read_little_endian_short(src);
			y = read_little_endian_short(src+2);
			w = read_little_endian_short(src+6);
			h = read_little_endian_short(src+8);
			src += 0x5c;

			x += region_table[i].x1;
			y += region_table[i].y1;

			Copy_32bpp(image, x, y, src, w*4, h);

			src += w*h*4;
		}
	}
	delete[] uncompress_data;
	return true;
}

void G00CONV::Copy_32bpp(char* image, int x, int y, const char* src, int bpl, int h) {
	int i;
	int* dest = (int*)(image + x*4 + y*4*width);
	int w = bpl / 4;
	for (i=0; i<h; i++) {
		const char* s = src;
		int* d = dest;
		int j; for (j=0; j<w; j++) {
			*d++ = read_little_endian_int(s);
			s += 4;
		}
		src += bpl; dest += width;
	}
}

void GRPCONV::CopyRGBA_rev(char* image, const char* buf) {
	int mask = is_mask ? 0 : 0xff000000;
	/* 色変換を行う */
	int len = width * height;
	int i;
	unsigned char* s = (unsigned char*)buf;
	int* d = (int*)image;
	for(i=0; i<len; i++) {
		*d = (int(s[2])) | (int(s[1])<<8) | (int(s[0])<<16) | (int(s[3])<<24) | mask;
		d++; s += 4;
	}
	return;
}

void GRPCONV::CopyRGBA(char* image, const char* buf) {
	if (!is_mask) {
		CopyRGB(image, buf);
		return;
	}
	/* 色変換を行う */
	int len = width * height;
	int i;
	int* outbuf = (int*)image;
	for(i=0; i<len; i++) {
		*outbuf++ =  read_little_endian_int(buf);
		buf += 4;
	}
	return;
}

void GRPCONV::CopyRGB(char* image, const char* buf) {
	/* 色変換を行う */
	int len = width * height;
	int i;
	unsigned char* s = (unsigned char*)buf;
	int* d = (int*)image;
	for(i=0; i<len; i++) {
		*d = (int(s[0])) | (int(s[1])<<8) | (int(s[2])<<16) | 0xff000000;
		d++; s+=3;
	}
	return;
}

BMPCONV::BMPCONV(const char* _inbuf, int _inlen, const char* _filename) {
	/* データから情報読み込み */
	int w = read_little_endian_int(_inbuf + 0x12);
	int h = read_little_endian_int(_inbuf + 0x16);
	if (h < 0) h = -h;
	int bpp = read_little_endian_short(_inbuf + 0x1c);
  //	int comp = read_little_endian_int(_inbuf + 0x1e);
	Init(filename, _inbuf, _inlen, w, h, bpp==32 ? true : false);
	return;
}

bool BMPCONV::Read(char* image) {
	if (data == 0) return false;

	/* マスクのチェック */
	int bpp = read_little_endian_short(data+0x1c);
	int h = read_little_endian_int(data + 0x16);
	int dsz = read_little_endian_int(data + 0x22);
	bpp /= 8;

	int bpl = dsz / height;
	if (bpl == 0) bpl = bpp*width;
	bpl = bpp * width;
	bpl = (bpl+3) & (~3);
	

	int i;
	char* buf = new char[width*height*bpp+1024];
	const char* src = data + 0x36;
	char* dest = buf;
	if (h < 0) {
		for (i=0; i<height; i++) {
			memcpy(dest+i*width*bpp, src+i*bpl, width*bpp);
		}
	} else {
		for (i=0; i<height; i++) {
			memcpy(dest+i*width*bpp, src+(height-i-1)*bpl, width*bpp);
		}
	}
	if (bpp == 3) CopyRGB(image, buf);
	else /* bpp == 4 */ CopyRGBA(image, buf);
	delete[] buf;
	return true;
}

