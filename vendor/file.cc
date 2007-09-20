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

static bool s_isBigEndian = IsBigEndian();

// -----------------------------------------------------------------------

bool init_end=false;
/*  file.cc  : KANON の圧縮ファイル・PDT ファイル（画像ファイル）の展開の
 *            ためのメソッド
 *     class ARCFILE : 書庫ファイル全体を扱うクラス
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
# define PATH_MAX 1024
#endif

# define NAMLEN(dirent) strlen((dirent)->d_name)

#if HAVE_LIBZ
#include<zlib.h>
#endif
#if HAVE_LIBPNG
#include<png.h>
#endif
#if HAVE_LIBJPEG
extern "C" {
#include<jpeglib.h>
}
#endif

#include "file.h"
#include "file_impl.h"

#include <iostream>
using namespace std;

// -----------------------------------------------------------------------

static int read_little_endian_int(const char* buf) {
	const unsigned char *p = (const unsigned char *) buf;
	return (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
}

static int read_little_endian_short(const char* buf) {
	const unsigned char *p = (const unsigned char *) buf;
	return (p[1] << 8) | p[0];
}

static int write_little_endian_int(char* buf, int number) {
	int c = read_little_endian_int(buf);
	unsigned char *p = (unsigned char *) buf;
	unsigned int unum = (unsigned int) number;
	p[0] = unum & 255;
	unum >>= 8;
	p[1] = unum & 255;
	unum >>= 8;
	p[2] = unum & 255;
	unum >>= 8;
	p[3] = unum & 255;
	return c;
}

static int write_little_endian_short(char* buf, int number) {
	int c = read_little_endian_short(buf);
	unsigned char *p = (unsigned char *) buf;
	unsigned int unum = (unsigned int) number;
	p[0] = unum & 255;
	unum >>= 8;
	p[1] = unum & 255;
	return c;
}

// -----------------------------------------------------------------------


FILESEARCH file_searcher;
// #define delete fprintf(stderr,"file.cc: %d.",__LINE__), delete

/* FILESEARCH class の default の振る舞い */
FILESEARCH::ARCTYPE FILESEARCH::default_is_archived[TYPEMAX] = {
	ATYPE_DIR, ATYPE_DIR, ATYPE_DIR, ATYPE_DIR,
	ATYPE_ARC, ATYPE_ARC, ATYPE_ARC, ATYPE_ARC,
	ATYPE_DIR, ATYPE_DIR, ATYPE_DIR, ATYPE_DIR,
	ATYPE_DIR, ATYPE_DIR
};
char* FILESEARCH::default_dirnames[TYPEMAX] = {
	0, 0, "", "pdt", 
	"seen.txt", "allanm.anl", "allard.ard", "allcur.cur", 
	0, 0, "koe", "bgm", "mov", "gan"};

/*********************************************
**  ARCFILE / DIRFILE:
**	書庫ファイル、あるいはディレクトリの
**	全体を管理するクラス
**
**	書庫ファイルからファイルの抜き出しはFind()
**	Find したものをReadすると内容が得られる。
*/

ARCFILE::ARCFILE(char* aname) {
	struct stat sb;
	/* 変数初期化 */
	arcname = 0;
	list_point = 0;
	filenames_orig = 0;
	next = 0;
	if (aname[0] == '\0') {arcname=new char[1]; arcname[0]='\0';return;} // NULFILE
	/* ディレクトリか否かのチェック */
	if (stat(aname,&sb) == -1) { /* error */
		perror("stat");
	}
	if ( (sb.st_mode&S_IFMT) == S_IFDIR) {
		int l = strlen(aname);
		arcname = new char[l+2]; strcpy(arcname, aname);
		if (arcname[l-1] != DIR_SPLIT) {
			arcname[l] = DIR_SPLIT;
			arcname[l+1] = 0;
		}
	} else if ( (sb.st_mode&S_IFMT) == S_IFREG) {
		arcname = new char[strlen(aname)+1];
		strcpy(arcname,aname);
		if (arcname[strlen(arcname)-1] == DIR_SPLIT)
			arcname[strlen(arcname)-1] = '\0';
	}
	return;
}

void ARCFILE::Init(void) {
	int i;
	if (! arc_atom.empty()) return;
	if (arcname == 0) return;
	/* ファイル数を得る */
	int slen = CheckFileDeal();
	/* ファイル名のセット */
	ListupFiles(slen);
	if ( (!arc_atom.empty()) && arc_atom[0].filename) filenames_orig = arc_atom[0].filename;
	sort(arc_atom.begin(), arc_atom.end());
}
ARCFILE::~ARCFILE() {
	if (filenames_orig) delete[] filenames_orig;
	delete[] arcname;
}

ARCFILE::iterator ARCFILE::SearchName(const char* f, const char* ext) {
	char buf[1024]; char buf_ext[1024];
	iterator it;
	Init();
	if (f == 0) return arc_atom.end();
	if (arc_atom.empty()) return arc_atom.end();
	/* エラーチェック */
	if (strlen(f)>500) return arc_atom.end();
	if (ext && strlen(ext)>500) return arc_atom.end();

	/* 検索 */
	strncpy(buf, f, 1000);
	buf[1000]=0;
	it = lower_bound(arc_atom.begin(), arc_atom.end(), (char*)buf);
	if (it != arc_atom.end() && strcmp(it->filename_lower, buf) == 0) return it;
	// 拡張子をつけて検索
	if (ext) {
		strcpy(buf_ext, buf);
		char* ext_pt = strrchr(buf_ext, '.');
		if (ext_pt == 0 || ext_pt == buf_ext) ext_pt = buf_ext + strlen(buf_ext);
		*ext_pt++ = '.';
		while(*ext=='.') ext++;
		strcat(buf_ext, ext);
		it = lower_bound(arc_atom.begin(), arc_atom.end(), (char*)buf_ext);
		if (it != arc_atom.end() && strcmp(it->filename_lower, buf_ext) == 0) return it;
	}

	/* 小文字にして 検索 */
	int i; int l = strlen(f);
	if (l > 500) l = 500;
	for (i=0; i<l; i++)
		buf[i] = tolower(f[i]);
	buf[i++] = 0;
	it = lower_bound(arc_atom.begin(), arc_atom.end(), (char*)buf);
	if (it != arc_atom.end() && strcmp(it->filename_lower, buf) == 0) return it;

	// 拡張子をつけて検索
	if (ext == 0) return arc_atom.end();
	strcpy(buf_ext, buf);
	char* ext_pt = strrchr(buf_ext, '.');
	if (ext_pt == 0 || ext_pt == buf_ext) ext_pt = buf_ext + strlen(buf_ext);
	*ext_pt++ = '.';
	/* 拡張子の長さを得る */
	l = strlen(ext);
	for (i=0; i<l; i++)
		ext_pt[i] = tolower(*ext++);
	ext_pt[i] = 0;
	it = lower_bound(arc_atom.begin(), arc_atom.end(), (char*)buf_ext);
	if (it != arc_atom.end() && strcmp(it->filename_lower, buf_ext) == 0) return it;
	return arc_atom.end();
}

ARCINFO* ARCFILE::Find(const char* fname, const char* ext) {
	Init();
	iterator atom = SearchName(fname,ext);
	if (atom == arc_atom.end()) {
		if (next) return next->Find(fname, ext);
		else return 0;
	}
	return MakeARCINFO(*atom);
}
ARCINFO* ARCFILE::MakeARCINFO(ARCFILE_ATOM& atom) {
	if (atom.arcsize == atom.filesize)
		return new ARCINFO(arcname, atom);
	else // 圧縮付
		return new ARCINFO_AVG32(arcname, atom);
}
ARCINFO* NULFILE::MakeARCINFO(ARCFILE_ATOM& atom) {
	fprintf(stderr,"NULFILE::MakeARCINFO is invalid call!\n");
	return 0;
}
ARCINFO* SCN2kFILE::MakeARCINFO(ARCFILE_ATOM& atom) {
	return new ARCINFO2k(arcname, atom);
}
ARCINFO* DIRFILE::MakeARCINFO(ARCFILE_ATOM& atom) {
	char* name = atom.filename;
	char* new_path = new char[strlen(arcname)+strlen(name)+1];
	strcpy(new_path,arcname); strcat(new_path, name);
	ARCINFO* ret = new ARCINFO(new_path, atom);
	delete[] new_path;
	return ret;
}

FILE* DIRFILE::Open(const char* fname) {
	iterator atom = SearchName(fname);
	if (atom == arc_atom.end()) return 0;
	char* name = atom->filename;
	// make FILE*
	char* new_path = new char[strlen(arcname)+strlen(name)+1];
	strcpy(new_path,arcname); strcat(new_path, name);
	FILE* ret = fopen(new_path, "rb+");
	fseek(ret, 0, 0);
	delete[] new_path;
	return ret;
}

char* DIRFILE::SearchFile(const char* fname) {
	iterator atom = SearchName(fname);
	if (atom == arc_atom.end()) return 0;
	char* name = atom->filename;
	char* new_path = new char[strlen(arcname)+strlen(name)+1];
	strcpy(new_path,arcname); strcat(new_path, name);
	struct stat sb;
	if (stat(new_path, &sb) == 0 &&
		( (sb.st_mode&S_IFMT) == S_IFREG ||
		  (sb.st_mode&S_IFMT) == S_IFDIR)) {
		return new_path;
	}
	delete[] new_path;
	return 0;
}

void ARCFILE::ListFiles(FILE* out) {
	int i;
	Init();
	if (arc_atom.empty()) return;
	// list file name...
	fprintf(out,"%16s %10s %10s %10s\n", "Filename", 
		"pointer","arcsize", "filesize");
	vector<ARCFILE_ATOM>::iterator it;
	for (it=arc_atom.begin(); it!=arc_atom.end(); it++) {
		fprintf(out,"%16s %10d %10d %10d\n",
			it->filename,it->offset,it->arcsize,it->filesize);
	}
	return;
}

void ARCFILE::InitList(void) {
	Init();
	list_point = 0;
}
char* ARCFILE::ListItem(void) {
	if (list_point < 0) return 0;
	if (list_point >= arc_atom.size()) return 0;
	char* fname = arc_atom[list_point].filename;
	if (fname == 0) return 0;
	char* ret = new char[strlen(fname)+1];
	strcpy(ret, fname);
	list_point++;
	return ret;
}

int ARCFILE::CheckFileDeal(void) {
	char buf[0x20];
	/* ヘッダのチェック */
	FILE* stream = fopen(arcname, "rb");
	if (stream == 0) {
		fprintf(stderr, "Cannot open archive file : %s\n",arcname);
		return 0;
	}
	fseek(stream, 0, 2); size_t arc_size = ftell(stream);
	fseek(stream, 0, 0);
	if (arc_size < 0x20) {
		fclose(stream);
		return 0;
	}
	fread(buf, 0x20, 1, stream);
	if (strncmp(buf, "PACL", 4) != 0) {
		fclose(stream);
		return 0;
	}
	int len = read_little_endian_int(buf+0x10);
	if (arc_size < size_t(0x20 + len*0x20)) {
		fclose(stream);
		return 0;
	}
	int i; int slen = 0;
	for (i=0; i<len; i++) {
		fread(buf, 0x20, 1, stream);
		slen += strlen(buf)+1;
	}
	fclose(stream);
	return slen;
}
void ARCFILE::ListupFiles(int fname_len) {
	int i; char fbuf[0x20];
	fname_len *= 2;
	char* buf = new char[fname_len];
	FILE* stream = fopen(arcname, "rb");
	if (stream == 0) {
		fprintf(stderr, "Cannot open archive file : %s\n",arcname);
		return;
	}
	fread(fbuf,0x20,1,stream);
	int len = read_little_endian_int(fbuf+0x10);
	ARCFILE_ATOM atom;
	for (i=0; i<len; i++) {
		fread(fbuf, 0x20, 1, stream);
		int l = strlen(fbuf);
		if (l*2+2 > fname_len) {
			break;
		}
		atom.offset = read_little_endian_int(fbuf+0x10);
		atom.arcsize = read_little_endian_int(fbuf+0x14);
		atom.filesize = read_little_endian_int(fbuf+0x18);
		int j; for (j=0; j<l; j++) {
			buf[j] = fbuf[j];
			buf[j+l+1] = tolower(fbuf[j]);
		}
		buf[j] = buf[j+l+1] = 0;
		atom.filename = buf;
		atom.filename_lower = buf+l+1;
		arc_atom.push_back(atom);
		buf += l*2+2; fname_len -= l*2+2;
	}
	fclose(stream);
	return;
}
int DIRFILE::CheckFileDeal(void) {
	DIR* dir; struct dirent* ent;
	int flen = 0;
	dir = opendir(arcname);
	if (dir == 0) {
		fprintf(stderr, "Cannot open dir file : %s\n",arcname);
		return 0;
	}
	int count = 0;
	while( (ent = readdir(dir)) != NULL) {
		count++;
		flen += strlen(ent->d_name)+1;
	}
	closedir(dir);
	return flen;
}
void DIRFILE::ListupFiles(int fname_len) {
	DIR* dir; int i;
	fname_len *= 2;
	dir = opendir(arcname);
	if (dir == 0) { 
		fprintf(stderr, "Cannot open dir file : %s\n",arcname);
		return;
	}
	char old_dir_path[PATH_MAX];
	getcwd(old_dir_path, PATH_MAX);
	old_dir_path[PATH_MAX - 1] = 0;
	/* 一時的に arcname のディレクトリに移動する */
	int old_dir_fd = open(".",O_RDONLY);
	if (old_dir_fd < 0) {
		closedir(dir);
		return;
	}
	if (chdir(arcname) != 0) {
		fprintf(stderr, "Cannot open dir file : %s\n",arcname);
		closedir(dir);
		close(old_dir_fd);
		return;
	};
	
	char* buf = new char[fname_len];
	ARCFILE_ATOM atom;
	struct stat sb;
	struct dirent* ent;
	while( (ent = readdir(dir)) != NULL) {
		if (stat(ent->d_name, &sb) == -1) continue;
		if ( (sb.st_mode & S_IFMT) == S_IFREG) {
			atom.offset = 0;
			atom.arcsize = sb.st_size;
			atom.filesize = sb.st_size;
		} else if ( (sb.st_mode & S_IFMT) == S_IFDIR) {
			atom.offset = 0;
			atom.arcsize = atom.filesize = 0;
		} else {
			continue;
		}
		int l = strlen(ent->d_name);
		if (l*2+2 > fname_len) {
			break;
		}
		int j; for (j=0; j<l+1; j++) {
			buf[j] = ent->d_name[j];
			buf[j+l+1] = tolower(ent->d_name[j]);
		}
		atom.filename = buf; atom.filename_lower = buf+l+1;
		arc_atom.push_back(atom);
		buf += l*2+2; fname_len -= l*2+2;
	}
	/* chdir() したのを元に戻る */
	closedir(dir);
	//fchdir(old_dir_fd);
	chdir(old_dir_path);
	close(old_dir_fd);
	return;
}
int NULFILE::CheckFileDeal(void) {
	return 20;
}
void NULFILE::ListupFiles(int fname_len) {
	char* s = new char[40];
	ARCFILE_ATOM atom;
	atom.offset = 0; atom.arcsize = 0; atom.filesize = 0;
	strcpy(s, "** null dummy **");
	atom.filename = s;
	atom.filename_lower = s;
	arc_atom.push_back(atom);
}
int SCN2kFILE::CheckFileDeal(void) {
	/* ヘッダのチェック */
	FILE* stream = fopen(arcname, "rb");
	if (stream == 0) {
		fprintf(stderr, "Cannot open archive file : %s\n",arcname);
		return 0;
	}
	fseek(stream, 0, 2); size_t arc_size = ftell(stream);
	fseek(stream, 0, 0);
	if (arc_size < 10000*8) {
		fclose(stream);
		return 0;
	}
	char* buf = new char[10000*8];
	fread(buf, 10000, 8, stream);
	/* size == 0 のデータは存在しない */
	int count = 0;
	int i; for (i=0; i<10000; i++) {
		int tmp_offset = read_little_endian_int(buf+i*8);
		int tmp_size = read_little_endian_int(buf+i*8+4);
		if (tmp_size <= 0 || tmp_offset < 0 || tmp_offset+tmp_size > int(arc_size) ) continue;
		count++;
	}
	fclose(stream);
	delete[] buf;
	return count*13; /* ファイル名は seenXXXX.txt だから、一つ12文字+null */
}
void SCN2kFILE::ListupFiles(int fname_len) {
	FILE* stream = fopen(arcname, "rb");
	if (stream == 0) {
		fprintf(stderr, "Cannot open archive file : %s\n",arcname);
		return;
	}
	char* sbuf = new char[fname_len];
	char* buf = new char[10000*8];
	fread(buf, 10000, 8, stream);
	fseek(stream, 0, 2); size_t arc_size = ftell(stream);
	ARCFILE_ATOM atom;
	int i; for (i=0; i<10000; i++) {
		char header[0x200];
		int tmp_offset = read_little_endian_int(buf+i*8);
		int tmp_size = read_little_endian_int(buf+i*8+4);
		if (tmp_size <= 0 || tmp_offset < 0 || tmp_offset+tmp_size > int(arc_size) ) continue;
		/* header を得て圧縮形式などを調べる */
		fseek(stream, tmp_offset, 0);
		fread(header, 0x200, 1, stream);
		int header_top = read_little_endian_int(header+0);
		int file_version = read_little_endian_int(header+4);

		if (file_version != 0x2712) continue; /* system version が違う */

		if (header_top == 0x1cc) { /* 古い形式 : avg2000 */
			int header_size = read_little_endian_int(header+0)+read_little_endian_int(header+0x20)*4;
			int data_size = read_little_endian_int(header+0x24);
			atom.arcsize = data_size + header_size;
			atom.filesize = data_size + header_size;
			atom.private_data = header_size;

		} else if (header_top == 0x1b8) { /* 初夜献上 */
			int header_size = read_little_endian_int(header+0)+read_little_endian_int(header+0x08)*4;
			int data_size = read_little_endian_int(header+0x0c);
			int compdata_size = read_little_endian_int(header+0x10);
			atom.arcsize = compdata_size + header_size;
			atom.filesize = data_size + header_size;
			atom.private_data = header_size;
			
		} else if (header_top == 0x1d0) { /* 新しい形式： reallive */
			int header_size = read_little_endian_int(header+0x20);
			int data_size = read_little_endian_int(header+0x24);
			int compdata_size = read_little_endian_int(header+0x28);
			atom.arcsize = compdata_size + header_size;
			atom.filesize = data_size + header_size;
			atom.private_data = header_size;
		} else {
			fprintf(stderr,"invalid header top; %x : not supported\n",header_top);
			continue; /* サポートしない形式 */
		}

		atom.offset = tmp_offset;
		atom.filename = sbuf;
		atom.filename_lower = sbuf;
		arc_atom.push_back(atom);
		sprintf(sbuf, "seen%04d.txt",i); sbuf += 13;
	}
	fclose(stream);
	return;
}

/********************************************************
** FILESEARCH クラスの実装
*/

FILESEARCH::FILESEARCH(void) {
	int i;
	root_dir = 0; dat_dir = 0;
	for (i=0; i<TYPEMAX; i++) {
		searcher[i] = 0;
		filenames[i] = default_dirnames[i];
		is_archived[i] = default_is_archived[i];
	}
}
FILESEARCH::~FILESEARCH(void) {
	int i;
	for (i=0; i<TYPEMAX; i++) {
		if (filenames[i] != 0 && filenames[i] != default_dirnames[i]) delete[] filenames[i];
		if (searcher[i] && searcher[i] != dat_dir && searcher[i] != root_dir) {
			delete searcher[i];
		}
	}
	if (dat_dir && dat_dir != root_dir) delete dat_dir;
	if (root_dir) delete root_dir;
}

int FILESEARCH::InitRoot(char* root) {
	/* 必要に応じて ~/ を展開 */
	if (root[0] == '~' && root[1] == '/') {
		char* home = getenv("HOME");
		if (home != 0) {
			char* new_root = new char[strlen(home)+strlen(root)];
			strcpy(new_root, home);
			strcat(new_root, root+1);
			root = new_root;
		}
	}
	/* 古いデータを消す */
	int i;
	for (i=0; i<TYPEMAX; i++) {
		if (searcher[i] != 0 &&
			searcher[i] != root_dir &&
			searcher[i] != dat_dir) {
				delete searcher[i];
		}
		searcher[i] = 0;
	}
	if (dat_dir && root_dir != dat_dir) delete dat_dir;
	if (root_dir) delete root_dir;
	dat_dir = 0;

	/* 新しいディレクトリのもとで初期化 */
	root_dir = new DIRFILE(root);
	root_dir->Init();
	/* dat/ を検索 */
	char* dat_path = root_dir->SearchFile("dat");
	if (dat_path == 0) {
		/* 見つからなかったら root を dat の代わりにつかう */
		dat_dir = root_dir;
	} else {
		dat_dir = new DIRFILE(dat_path);
		dat_dir->Init();
	}
	searcher[ALL] = dat_dir;
	searcher[ROOT] = root_dir;
	return 0;
}

void FILESEARCH::SetFileInformation(FILETYPE tp, ARCTYPE is_arc, char* filename) {
	int type = tp;
	if (type < 0 || type >= TYPEMAX) return;
	ARCFILE* next_arc = 0;
	/* すでに searcher が存在すれば解放 */
	if (searcher[type] != 0 &&
	  searcher[type] != root_dir &&
	  searcher[type] != dat_dir) {
		next_arc = searcher[type]->Next();
		delete searcher[type];
	}
	searcher[type] = 0;
	/* 適当に初期化 */
	if (filenames[type] != 0 &&
		filenames[type] != default_dirnames[type]) delete[] filenames[type];
	filenames[type] = new char[strlen(filename)+1];
	strcpy(filenames[type], filename);
	is_archived[type] = is_arc;
	searcher[type] = MakeARCFILE(is_arc, filename);
	if (searcher[type] && next_arc)
		searcher[type]->SetNext(next_arc);
	return;
}
void FILESEARCH::AppendFileInformation(FILETYPE tp, ARCTYPE is_arc, char* filename) {
	int type = tp;
	if (type < 0 || type >= TYPEMAX) return;
	/* searcher がまだ割り当てられてない場合 */
	if (searcher[type] == 0 ||
	  searcher[type] == root_dir ||
	  searcher[type] == dat_dir) {
		searcher[type] = MakeARCFILE(is_archived[type], filenames[type]);
		if (searcher[type] == 0) { /* 作成できなかった場合 */
			/* この型情報を FileInformation とする */
			SetFileInformation(tp, is_arc, filename);
			return;
		}
	}
	/* 初期化 */
	ARCFILE* arc = MakeARCFILE(is_arc, filename);
	/* append */
	ARCFILE* cur;
	for (cur=searcher[type]; cur->Next() != 0; cur = cur->Next()) ;
	cur->SetNext(arc);
	return;
}

ARCFILE* FILESEARCH::MakeARCFILE(ARCTYPE tp, char* filename) {
	ARCFILE* arc = 0;
	char* file;
	if (filename == 0) goto err;
	if (tp == ATYPE_DIR) {
		file = root_dir->SearchFile(filename);
	} else {
		file = dat_dir->SearchFile(filename);
		if (file == 0)
			file = root_dir->SearchFile(filename);
	}
	if (file == 0) goto err;
	switch(tp) {
		case ATYPE_DIR: arc = new DIRFILE(file); break;
		case ATYPE_SCN2k:
		case ATYPE_ARC: {
			FILE* f = fopen(file, "rb");
			if (f == 0) goto err;
			char header[32];
			memset(header, 0, 32);
			fread(header, 32, 1, f);
			fclose(f);
			char magic_raf[8] = {'C','A','P','F',1,0,0,0};
			if (strncmp(header, "PACL", 4) == 0) arc = new ARCFILE(file);
			else arc = new SCN2kFILE(file);
			}
			break;
		default: fprintf(stderr,"FILESEARCH::MAKEARCFILE : invalid archive type; type %d name %s\n",tp,filename);
			delete[] file;
			goto err;
	}
	delete[] file;
	return arc;
err:
	arc = new NULFILE;
	return arc;
	
}

ARCINFO* FILESEARCH::Find(FILETYPE type, const char* fname, const char* ext) {
	if (searcher[type] == 0) {
		/* searcher 作成 */
		if (filenames[type] == 0) {
			searcher[type] = dat_dir;
		} else {
			searcher[type] = MakeARCFILE(is_archived[type], filenames[type]);
			if (searcher[type] == 0) {
				fprintf(stderr,"FILESEARCH::Find : invalid archive type; type %d name %s\n",type,fname);
				return 0;
			}
		}
	}
	return searcher[type]->Find(fname,ext);
}

char** FILESEARCH::ListAll(FILETYPE type) {
	int i;
	/* とりあえず searcher を初期化 */
	Find(type, "THIS FILENAME MAY NOT EXIST IN THE FILE SYSTEM !!!");
	if (searcher[type] == 0) return 0;
	/* 全ファイルのリストアップ */
	int deal = 0;
	ARCFILE* file;
	for (file = searcher[type]; file != 0; file = file->Next())
		deal += file->Deal();
	if (deal <= 0) return 0;
	char** ret_list = new char*[deal+1];
	int count = 0;
	for (file = searcher[type]; file != 0; file = file->Next()) {
		file->InitList();
		char* f;
		while( (f = file->ListItem() ) != 0) {
			ret_list[count] = new char[strlen(f)+1];
			strcpy(ret_list[count], f);
			count++;
		}
	}
	ret_list[count] = 0;
	return ret_list;
}

ARCINFO::ARCINFO(const char* __arcname, ARCFILE_ATOM& atom) : info(atom) {
	arcfile = new char[strlen(__arcname)+1];
	strcpy(arcfile, __arcname);
	use_mmap = false;
	mmapped_memory = 0;
	data = 0;
	fd = -1;
}

ARCINFO::~ARCINFO() {
#ifdef HAVE_MMAP
	if (mmapped_memory) munmap(mmapped_memory, info.arcsize);
#endif /* HAVE_MMAP */
	if (fd != -1) close(fd);
	if (data != mmapped_memory) delete[] data;
	delete[] arcfile;
}

int ARCINFO::Size(void) const {
	return info.filesize;
}

/* コピーを返す */
char* ARCINFO::CopyRead(void) {
	const char* d = Read();
	if (d == 0) return 0;
	int s = Size();
	if (s <= 0) return 0;
	char* ret = new char[s]; memcpy(ret, d, s);
	return ret;
}

const char* ARCINFO::Path(void) const {
	if (info.offset != 0) return 0; /* archive file なのでパスを帰せない */
	char* ret = new char[strlen(arcfile)+1];
	strcpy(ret, arcfile);
	return ret;
}
/* 互換性専用 */
FILE* ARCINFO::OpenFile(int* length) const {
	FILE* f = fopen(arcfile, "rb");
	if (info.offset) lseek(fileno(f), info.offset, SEEK_SET);
	if (length) *length = info.arcsize;
	return f;
}

// 展開処理はなし
bool ARCINFO::ExecExtract(void) {
	return true;
}
/* 読み込みを開始する */
const char* ARCINFO::Read(void) {
	// すでにデータを読み込み済みなら何もしない
	if (data) return data;

	if (info.offset < 0 || info.arcsize <= 0) {
		return 0;
	}
	/* ファイルを開く */
	fd = open(arcfile, O_RDONLY);
	if (fd < 0) {
		return 0;
	}
	if (lseek(fd, info.offset, 0) != info.offset) {
		close(fd); fd = -1; return 0;
	}
	/* mmap を試みる */
#ifdef HAVE_MMAP
	mmapped_memory = (char*)mmap(0, info.arcsize, PROT_READ, MAP_SHARED, fd, info.offset);
	if (mmapped_memory != MAP_FAILED) {
		use_mmap = true;
		data = (const char*)mmapped_memory;
	} else
#endif /* HAVE_MMAP */
	{
		/* 失敗：普通にファイルを読み込み */
		char* d = new char[info.arcsize];
		read(fd, d, info.arcsize);
		close(fd);
		fd = -1;
		use_mmap = false;
		data = d;
	}
	/* 展開する */
	if (! ExecExtract()) {
		// 失敗
#ifdef HAVE_MMAP
		if (use_mmap) {
			munmap(mmapped_memory, info.arcsize);
			if (data == (const char*)mmapped_memory) data = 0;
		}
#endif /* HAVE_MMAP */
		delete[] (char*)data;
		close(fd);
		fd = -1; data = 0;
		return 0;
	}
#ifdef HAVE_MMAP
	if (use_mmap && data != (const char*)mmapped_memory) {
		// すでに mmap は必要ない
		munmap(mmapped_memory, info.arcsize);
		close(fd);
		fd = -1;
		use_mmap = false;
	}
#endif /* HAVE_MMAP */
	return data;
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
#if HAVE_LIBPNG
class PNGCONV : public GRPCONV {
	const char* png_data;
	static void png_read(png_structp, png_bytep, png_size_t);

public:
	PNGCONV(const char* _inbuf, int _inlen, const char* fname);
	~PNGCONV() {};
	bool Read(char* image);
};
#endif

#if HAVE_LIBJPEG
class JPEGCONV : public GRPCONV {

public:
	JPEGCONV(const char* _inbuf, int _inlen, const char* fname);
	~JPEGCONV() {};
	bool Read(char* image);
	void SetupSrc(struct jpeg_decompress_struct* cinfo, const char* data, int size);
	static void init_source(j_decompress_ptr cinfo);
	static boolean fill_input_buffer(j_decompress_ptr cinfo);
	static void skip_input_data(j_decompress_ptr cinfo, long num_bytes);
	static boolean resync_to_restart(j_decompress_ptr cinfo, int desired);
	static void term_source(j_decompress_ptr cinf);
};
#endif

GRPCONV* GRPCONV::AssignConverter(const char* inbuf, int inlen, const char* fname) {
	/* ファイルの内容に応じたコンバーターを割り当てる */
	GRPCONV* conv = 0;
	if (inlen < 10) return 0; /* invalid file */
	if (conv == 0 && strncmp(inbuf, "PDT10", 5) == 0 || strncmp(inbuf, "PDT11", 5) == 0) { /* PDT10 or PDT11 */
		conv = new PDTCONV(inbuf, inlen, fname);
		if (conv->data == 0) { delete conv; conv = 0;}
	}
#if HAVE_LIBPNG
	unsigned char png_magic[4] = {0x89, 'P', 'N', 'G'};
	if (conv == 0 && memcmp(inbuf, png_magic,4) == 0) {
		conv = new PNGCONV(inbuf, inlen, fname);
		if (conv->data == 0) { delete conv; conv = 0;}
	}
#endif
#if HAVE_LIBJPEG
	if ( conv == 0 && *(unsigned char*)inbuf == 0xff && *(unsigned char*)(inbuf+1) == 0xd8 &&
		(strncmp(inbuf+6, "JFIF",4) == 0 || strncmp(inbuf+6,"Exif",4) == 0)) {
		conv = new JPEGCONV(inbuf, inlen, fname);
		if (conv->data == 0) { delete conv; conv = 0;}
	}
#endif
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
//    cerr << "Type: " << type << endl;
	if (type == 0) return Read_Type0(image);
	else if (type == 1) return Read_Type1(image);
	else if (type == 2) return Read_Type2(image);
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
      if(s_isBigEndian)
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
/* 書庫用 */
class Extract_DataType_ARC {
public:
	void ExtractData(const char*& lsrc, int& data, int& size) {
		data = read_little_endian_short(lsrc) & 0xffff;
		size = (data&0x0f) + 2;
		data = (data>>4) + 1;
		lsrc+= 2;
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
      if(s_isBigEndian)
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
void ARCINFO::Extract(char*& dest_start, char*& src_start, char* dest_end, char* src_end) {
	const char* src = src_start;
	while (lzExtract(Extract_DataType_ARC(), char(), src, dest_start, src_end, dest_end)) ;
	src_start = (char*)src;
	return;
}
void ARCINFO::Extract2k(char*& dest_start, char*& src_start, char* dest_end, char* src_end) {
	const char* src = src_start;
	while (lzExtract(Extract_DataType_SCN2k(), char(), src, dest_start, src_end, dest_end)) ;
	src_start = (char*)src;
	return;
}

bool ARCINFO_AVG32::ExecExtract(void) {
	// ヘッダのチェック
	if (strncmp(data, "PACK", 4) != 0) return false;
	if (read_little_endian_int(data+8) != info.filesize) return false;
	if (read_little_endian_int(data+12) != info.arcsize) return false;

	// ファイルを展開する
	char* ret_data = new char[info.filesize+1024];

	const char* s = data + 0x10;
	const char* send = data + info.arcsize;
	char* d = ret_data;
	char* dend = ret_data + info.filesize;
	while(lzExtract(Extract_DataType_ARC(), char(), s, d, send, dend)) ;
	if (! use_mmap) delete[] data;
	data = ret_data;
	return true;
}

char ARCINFO2k::decode_seed[256] ={
 0x8b ,0xe5 ,0x5d ,0xc3 ,0xa1 ,0xe0 ,0x30 ,0x44 ,0x00 ,0x85 ,0xc0 ,0x74 ,0x09 ,0x5f ,0x5e ,0x33
,0xc0 ,0x5b ,0x8b ,0xe5 ,0x5d ,0xc3 ,0x8b ,0x45 ,0x0c ,0x85 ,0xc0 ,0x75 ,0x14 ,0x8b ,0x55 ,0xec
,0x83 ,0xc2 ,0x20 ,0x52 ,0x6a ,0x00 ,0xe8 ,0xf5 ,0x28 ,0x01 ,0x00 ,0x83 ,0xc4 ,0x08 ,0x89 ,0x45
,0x0c ,0x8b ,0x45 ,0xe4 ,0x6a ,0x00 ,0x6a ,0x00 ,0x50 ,0x53 ,0xff ,0x15 ,0x34 ,0xb1 ,0x43 ,0x00
,0x8b ,0x45 ,0x10 ,0x85 ,0xc0 ,0x74 ,0x05 ,0x8b ,0x4d ,0xec ,0x89 ,0x08 ,0x8a ,0x45 ,0xf0 ,0x84
,0xc0 ,0x75 ,0x78 ,0xa1 ,0xe0 ,0x30 ,0x44 ,0x00 ,0x8b ,0x7d ,0xe8 ,0x8b ,0x75 ,0x0c ,0x85 ,0xc0
,0x75 ,0x44 ,0x8b ,0x1d ,0xd0 ,0xb0 ,0x43 ,0x00 ,0x85 ,0xff ,0x76 ,0x37 ,0x81 ,0xff ,0x00 ,0x00
,0x04 ,0x00 ,0x6a ,0x00 ,0x76 ,0x43 ,0x8b ,0x45 ,0xf8 ,0x8d ,0x55 ,0xfc ,0x52 ,0x68 ,0x00 ,0x00
,0x04 ,0x00 ,0x56 ,0x50 ,0xff ,0x15 ,0x2c ,0xb1 ,0x43 ,0x00 ,0x6a ,0x05 ,0xff ,0xd3 ,0xa1 ,0xe0
,0x30 ,0x44 ,0x00 ,0x81 ,0xef ,0x00 ,0x00 ,0x04 ,0x00 ,0x81 ,0xc6 ,0x00 ,0x00 ,0x04 ,0x00 ,0x85
,0xc0 ,0x74 ,0xc5 ,0x8b ,0x5d ,0xf8 ,0x53 ,0xe8 ,0xf4 ,0xfb ,0xff ,0xff ,0x8b ,0x45 ,0x0c ,0x83
,0xc4 ,0x04 ,0x5f ,0x5e ,0x5b ,0x8b ,0xe5 ,0x5d ,0xc3 ,0x8b ,0x55 ,0xf8 ,0x8d ,0x4d ,0xfc ,0x51
,0x57 ,0x56 ,0x52 ,0xff ,0x15 ,0x2c ,0xb1 ,0x43 ,0x00 ,0xeb ,0xd8 ,0x8b ,0x45 ,0xe8 ,0x83 ,0xc0
,0x20 ,0x50 ,0x6a ,0x00 ,0xe8 ,0x47 ,0x28 ,0x01 ,0x00 ,0x8b ,0x7d ,0xe8 ,0x89 ,0x45 ,0xf4 ,0x8b
,0xf0 ,0xa1 ,0xe0 ,0x30 ,0x44 ,0x00 ,0x83 ,0xc4 ,0x08 ,0x85 ,0xc0 ,0x75 ,0x56 ,0x8b ,0x1d ,0xd0
,0xb0 ,0x43 ,0x00 ,0x85 ,0xff ,0x76 ,0x49 ,0x81 ,0xff ,0x00 ,0x00 ,0x04 ,0x00 ,0x6a ,0x00 ,0x76};

bool ARCINFO2k::ExecExtract(void) {
	int i;
	char* ret_data = new char[info.filesize + 1024];
	char* decoded_data = new char[info.arcsize + 1024];
	
	/* header のコピー */
	memcpy(ret_data, data, info.private_data);

	/* まず、xor の暗号化を解く */
	const char* s; const char* send;
	char* d; char* dend;

	s = data + info.private_data;
	send = data + info.arcsize;
	d = decoded_data + info.private_data;
	dend = decoded_data + info.arcsize;
	i = 0;
	while(s != send)
		*d++ = *s++ ^ decode_seed[(i++)&0xff];

	if (info.filesize == info.arcsize) {
		memcpy(ret_data+info.private_data, decoded_data + info.private_data + 8, info.arcsize - info.private_data - 8);
	} else {
		/* 圧縮されているなら、それを展開 */
		s = (const char*)(decoded_data + info.private_data + 8);
		send = (const char*)(decoded_data + info.arcsize);
		d = ret_data + info.private_data;
		dend = ret_data + info.filesize;
		while(lzExtract(Extract_DataType_SCN2k(), char(), s, d, send, dend)) ;
	}
	delete[] decoded_data;
	if (! use_mmap) delete[] data;
	data = ret_data;
	return true;
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
	/* 分割領域を得る */
	int region_deal = read_little_endian_int(data+5);
//	REGION* region_table = new REGION[region_deal];
    region_table = vector<REGION>(region_deal);

	const char* head = data + 9;
	int i; for (i=0; i<region_deal; i++) {
		region_table[i].x1 = read_little_endian_int(head+0);
		region_table[i].y1 = read_little_endian_int(head+4);
		region_table[i].x2 = read_little_endian_int(head+8);
		region_table[i].y2 = read_little_endian_int(head+12);
		region_table[i].Fix(width, height);
		head += 24;
	}

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

	for (i=0; i < region_deal; i++) {
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

#if HAVE_LIBPNG
PNGCONV::PNGCONV(const char* _inbuf, int _inlen, const char* _filename) {
	int w,h,type;
	png_structp png_ptr = 0;
	png_infop info_ptr = 0;
	png_infop end_info = 0;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) return;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) goto err;

	end_info = png_create_info_struct(png_ptr);
	if (!end_info) goto err;

        if (setjmp(png_jmpbuf(png_ptr))) {
                /* error occured !! */
		goto err;
        }

	/* initialize I/O */
	png_data = _inbuf;
	png_set_read_fn(png_ptr, (png_voidp)this, &png_read);

	png_read_info(png_ptr, info_ptr);

	w = png_get_image_width(png_ptr, info_ptr);
	h = png_get_image_height(png_ptr, info_ptr);
	type = png_get_color_type(png_ptr, info_ptr);

	if (type == PNG_COLOR_TYPE_GRAY || type == PNG_COLOR_TYPE_GRAY_ALPHA) goto err; // not supported

	Init(filename, _inbuf, _inlen, w, h, type == PNG_COLOR_TYPE_RGB_ALPHA ? true : false);

err:
	if (png_ptr) {
		if (end_info)
			png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
		else if (info_ptr)
			png_destroy_read_struct(&png_ptr, &info_ptr,(png_infopp)0);
		else
			png_destroy_read_struct(&png_ptr, (png_infopp) 0,(png_infopp)0);
	}
	return;
}

bool PNGCONV::Read(char* image) {
	if (data == 0) return false;
	bool retcode = false;
	int bpp = is_mask ? 4 : 3;
	int i;
	char* buf;
	png_bytepp row_pointers = 0;

	png_structp png_ptr = 0;
	png_infop info_ptr = 0;
	png_infop end_info = 0;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) goto err;
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) goto err;
	end_info = png_create_info_struct(png_ptr);
	if (!end_info) goto err;

        if (setjmp(png_jmpbuf(png_ptr))) {
                /* error occured !! */
		goto err;
        }

	buf= new char[width*height*4];
	/* initialize I/O */
	png_data = data;
	png_set_read_fn(png_ptr, (png_voidp)this, &png_read);
	
	row_pointers = (png_bytepp)png_malloc(png_ptr, height*sizeof(png_bytep));
	for (i=0; i<height; i++) row_pointers[i] = (png_bytep)(buf + width*bpp*i);
	png_set_rows(png_ptr, info_ptr, row_pointers);

	png_read_png(png_ptr, info_ptr,
           PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_BGR,0);

	if (buf != image) {
		CopyRGBA(image, buf);
		delete[] buf;
	}
	png_free(png_ptr, (png_voidp)row_pointers);

	retcode = true;
err:
	if (png_ptr) {
		if (end_info)
			png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
		else if (info_ptr)
			png_destroy_read_struct(&png_ptr, &info_ptr,(png_infopp)0);
		else
			png_destroy_read_struct(&png_ptr, (png_infopp) 0,(png_infopp)0);
	}
	return retcode;
}

void PNGCONV::png_read(png_structp png_ptr, png_bytep d, png_size_t sz) {
	PNGCONV* orig = (PNGCONV*)png_get_io_ptr(png_ptr);
	memcpy(d, orig->png_data, sz);
	orig->png_data += sz;
	return;
}
#endif /* HAVE_LIBPNG */

#if HAVE_LIBJPEG
JPEGCONV::JPEGCONV(const char* _inbuf, int _inlen, const char* _filename) {
	int w,h,type;
	JSAMPARRAY rows, rows_orig; int i;
	char* buf = 0;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	cinfo.src = new jpeg_source_mgr;
	SetupSrc(&cinfo, _inbuf, _inlen);

	if (jpeg_read_header(&cinfo, TRUE) == JPEG_HEADER_OK)  {
		Init(filename, _inbuf, _inlen, cinfo.image_width, cinfo.image_height, false);
	}
	delete cinfo.src;
	cinfo.src = 0;
	jpeg_destroy_decompress(&cinfo);
	return;
}
		
bool JPEGCONV::Read(char* image) {
	if (data == 0) return false;
	bool retcode = false;
	JSAMPARRAY rows, rows_orig; int i;
	char* buf = 0;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	cinfo.src = new jpeg_source_mgr;
	SetupSrc(&cinfo, data, datalen);

	if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) goto err;

	cinfo.out_color_space = JCS_RGB;
	
	jpeg_start_decompress(&cinfo);

	rows = new JSAMPROW[height];
	rows_orig = rows;
	buf = new char[width*height*3];
	for (i=0; i<height; i++) rows[i] = (JSAMPROW)(buf+width*3*i);

	for (i=0; i<height; ) {
		int cnt = jpeg_read_scanlines(&cinfo, rows, height-i);
		rows += cnt;
		i += cnt;
	}
	delete[] rows_orig;
	CopyRGBA_rev(image, buf);
	delete[] buf;
	
	jpeg_finish_decompress(&cinfo);
	retcode = true;
err:
	delete cinfo.src;
	cinfo.src = 0;
	jpeg_destroy_decompress(&cinfo);
	return retcode;
}

void JPEGCONV::init_source(j_decompress_ptr cinfo) {
}
boolean JPEGCONV::fill_input_buffer(j_decompress_ptr cinfo) {
	static char dummy[1024];
	memset(dummy, 0, 1024);
	cinfo->src->next_input_byte = (const JOCTET*)dummy;
	cinfo->src->bytes_in_buffer = 1024;
	fprintf(stderr,"JPEGCONV::fill_input_buffer: warning corrupted jpeg stream\n");
	return TRUE;
}
void JPEGCONV::skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
	if (cinfo->src->bytes_in_buffer > num_bytes) {
		cinfo->src->next_input_byte += num_bytes;
		cinfo->src->bytes_in_buffer -= num_bytes;
	}
}
boolean JPEGCONV::resync_to_restart(j_decompress_ptr cinfo, int desired) {
	return jpeg_resync_to_restart(cinfo, desired);
}
void JPEGCONV::term_source(j_decompress_ptr cinf) {
}

void JPEGCONV::SetupSrc(struct jpeg_decompress_struct* cinfo, const char* data, int size) {
	cinfo->src->next_input_byte = (const JOCTET*) data;
	cinfo->src->bytes_in_buffer = size;
	cinfo->src->init_source = init_source;
	cinfo->src->fill_input_buffer = fill_input_buffer;
	cinfo->src->skip_input_data = skip_input_data;
	cinfo->src->resync_to_restart = resync_to_restart;
	cinfo->src->term_source = term_source;
}
#endif /* HAVE_LIBJPEG */
BMPCONV::BMPCONV(const char* _inbuf, int _inlen, const char* _filename) {
	/* データから情報読み込み */
	int w = read_little_endian_int(_inbuf + 0x12);
	int h = read_little_endian_int(_inbuf + 0x16);
	if (h < 0) h = -h;
	int bpp = read_little_endian_short(_inbuf + 0x1c);
	int comp = read_little_endian_int(_inbuf + 0x1e);
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

