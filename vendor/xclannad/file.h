/*  file.h  : KANON の圧縮ファイル・PDT ファイル（画像ファイル）の展開の
 *            ためのクラス
 *     class ARCINFO : 書庫ファイルの中の１つのファイルを扱うクラス
 *     class PDTCONV : PDT ファイルの展開を行う。
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/

#ifndef __KANON_FILE_H__
#define __KANON_FILE_H__

#ifndef DIR_SPLIT
#define DIR_SPLIT '/'	/* UNIX */
#endif

// read 'KANON' compressed file

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>

#include <vector>

#define INT_SIZE 4


/*********************************************
**  FILESEARCH:
**	書庫ファイル／ディレクトリを含め、
**	全ファイルの管理を行う。
**
**	最初に、設定ファイルからファイルの種類ごとに
**	実際に入っているディレクトリ、書庫を設定する
**
**	以降はFind() メソッドで実際のファイルの内容を得る
**
*/

/* ARCFILE と DIRFILE はファイル種類ごとの情報 */
class ARCFILE;
class DIRFILE;
class SCN2kFILE;
class RaffresiaFILE;
/* ARCINFO はファイルを読み込むために必要 */
class ARCINFO;
struct ARCFILE_ATOM;

class ARCINFO {
protected:
	/* ファイルそのものの情報 */
	ARCFILE_ATOM& info;
	char* arcfile;
	/* mmap している場合、その情報 */
	bool use_mmap;
	char* mmapped_memory;
	int fd;
	/* ファイル内容の入っているバッファ */
	const char* data;

protected:
	ARCINFO(const char* arcfile, ARCFILE_ATOM& from); // only from ARCFILE
	friend class ARCFILE;
	friend class DIRFILE;

	virtual bool ExecExtract(void);
public:
	/* dest は256byte 程度の余裕があること */
	static void Extract(char*& dest, char*& src, char* destend, char* srcend);
	static void Extract2k(char*& dest, char*& src, char* destend, char* srcend);
	virtual ~ARCINFO();
	/* 必要なら Read 前に呼ぶことで処理を分割できる */
	int Size(void) const;
	char* CopyRead(void); /* Read() して内容のコピーを返す */
	const char* Read(void);
	/* ファイルが regular file の場合、ファイル名を帰す */
	/* そうでないなら 0 を帰す */
	const char* Path(void) const;
	FILE* OpenFile(int* length=0) const; /* 互換性のため：raw file の場合、ファイルを開く */
};

class GRPCONV {
public:
	struct REGION {
      int x1, y1, x2, y2;
      int origin_x, origin_y;
		int Width() { return x2-x1+1;}
		int Height() { return y2-y1+1;}
		void FixVar(int& v, int& w) {
			if (v < 0) v = 0;
			if (v >= w) v = w-1;
		}
		void Fix(int w, int h) {
			FixVar(x1,w);
			FixVar(x2,w);
			FixVar(y1,h);
			FixVar(y2,h);
			if (x1 > x2) x2 = x1;
			if (y1 > y2) y2 = y1;
		}
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
	static GRPCONV* AssignConverter(ARCINFO* info) {
		const char* dat = info->Read();
		if (dat == 0) return 0;
		return AssignConverter(dat, info->Size(), "???");
	}
	void CopyRGBA(char* image, const char* from);
	void CopyRGB(char* image, const char* from);
	void CopyRGBA_rev(char* image, const char* from);
	void CopyRGB_rev(char* image, const char* from);
};

#endif // !defined(__KANON_FILE_H__)
