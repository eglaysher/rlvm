/*
 * wavfile.h  WAV file´ØÏ¢
 *
 *  Copyright: wavfile.c (c) Erik de Castro Lopo  erikd@zip.com.au
 *
 *  Modified : 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *             1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
*/
#ifndef __WAVEFILE__
#define __WAVEFILE__

#define WW_BADOUTPUTFILE	1
#define WW_BADWRITEHEADER	2

#define WR_BADALLOC		3
#define WR_BADSEEK		4
#define WR_BADRIFF		5
#define WR_BADWAVE		6
#define WR_BADFORMAT		7
#define WR_BADFORMATSIZE	8

#define WR_NOTPCMFORMAT		9
#define WR_NODATACHUNK		10
#define WR_BADFORMATDATA	11

/*
 * These values represent values found in/or destined for a
 * WAV file.
 */
typedef struct {
	unsigned int	SamplingRate;			/* Sampling rate in Hz */
	int	Channels;			/* Mono or Stereo */
	unsigned short	DataBits;			/* Sample bit size (8/12/16) */
} WAVINF;

struct WAVFILE{
	WAVINF wavinfo;                         /* WAV file hdr info */
	WAVFILE(void);
	static int freq;
	static int format;
	static int channels;
	virtual ~WAVFILE() {};
	virtual int Read(char* buf, int blksize, int blklen) = 0;
	virtual void Seek(int count) = 0;
	static WAVFILE* MakeConverter(WAVFILE* new_reader);
};

struct WAVFILE_Converter : WAVFILE {
	WAVFILE* original;
	struct SDL_AudioCVT* cvt;
	int datasize;
	char* tmpbuf;
	int Read(char* buf, int blksize, int blklen);
	void Seek(int count) { original->Seek(count);}
	WAVFILE_Converter(WAVFILE* orig, struct SDL_AudioCVT* cvt);
	~WAVFILE_Converter();
};

struct WAVFILE_Stream : WAVFILE{
	char *data;                             /* real data */ 
	char *data_orig;
	int data_length;
	FILE* stream;
	int stream_length;
	int stream_length_orig;
	int stream_top;
	int Read(char* buf, int blksize, int blklen);
	void Seek(int count);
	WAVFILE_Stream(FILE* stream, int length);
	~WAVFILE_Stream();
};

struct NWAFILE : WAVFILE {
	int skip_count;
	FILE* stream;
	struct NWAData* nwa;
	char* data;
	int block_size;
	int data_len;

	NWAFILE(FILE* stream);
	~NWAFILE();
	void Seek(int count);
	int Read(char* buf, int blksize, int blklen);
	static char* ReadAll(FILE* stream, int& size);
};

struct OggFILE : WAVFILE {
	struct OggFILE_impl* pimpl;

	OggFILE(FILE* stream, int size);
	~OggFILE();
	void Seek(int count);
	int Read(char* buf, int blksize, int blklen);
};

struct MP3FILE : WAVFILE {
	struct MP3FILE_impl* pimpl;

	MP3FILE(FILE* stream, int size);
	~MP3FILE();
	void Seek(int count);
	int Read(char* buf, int blksize, int blklen);
};

#endif /* !__WAVEFILE__ */
