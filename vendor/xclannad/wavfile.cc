/*
 * wavfile.c  WAV file check
 *
 *  Copyright: wavfile.c (c) Erik de Castro Lopo  erikd@zip.com.au
 *
 *  Modified : 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *             1998-                           <masaki-c@is.aist-nara.ac.jp>
 *             2000-     Kazunori Ueno(JAGARL) <jagarl@createor.club.ne.jp>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include        <stdarg.h>
#include  	<stdio.h>
#include  	<stdlib.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<unistd.h>
#include  	<string.h>
#include        "wavfile.h"
#include "endian.hpp"
#include <stdexcept>

#include <SDL2/SDL_mixer.h>

#define		BUFFERSIZE   		1024
#define		PCM_WAVE_FORMAT   	1

/*******************************************************
**
**	WAVE Header
*/

inline int LittleEndian_getDW(const char *b,int index) {
	int c0, c1, c2, c3;
	int d0, d1;
	c0 = *(const unsigned char*)(b + index + 0);
	c1 = *(const unsigned char*)(b + index + 1);
	c2 = *(const unsigned char*)(b + index + 2);
	c3 = *(const unsigned char*)(b + index + 3);
	d0 = c0 + (c1 << 8);
	d1 = c2 + (c3 << 8);
	return d0 + (d1 << 16);
}

inline int LittleEndian_get3B(const char *b,int index) {
	int c0, c1, c2;
	c0 = *(const unsigned char*)(b + index + 0);
	c1 = *(const unsigned char*)(b + index + 1);
	c2 = *(const unsigned char*)(b + index + 2);
	return c0 + (c1 << 8) + (c2 << 16);
}

inline int LittleEndian_getW(const char *b,int index) {
	int c0, c1;
	c0 = *(const unsigned char*)(b + index + 0);
	c1 = *(const unsigned char*)(b + index + 1);
	return c0 + (c1 << 8);
}

inline void LittleEndian_putW(int num, char *b, int index) {
	int c0, c1;
	num %= 65536;
	c0 = num % 256;
	c1 = num / 256;
	b[index] = c0; b[index+1] = c1;
}

typedef  struct
{	u_long     dwSize ;
	u_short    wFormatTag ;
	u_short    wChannels ;
	u_long     dwSamplesPerSec ;
	u_long     dwAvgBytesPerSec ;
	u_short    wBlockAlign ;
	u_short    wBitsPerSample ;
} WAVEFORMAT ;

typedef  struct
{	char    	RiffID [4] ;
	u_long    	RiffSize ;
	char    	WaveID [4] ;
	char    	FmtID  [4] ;
	u_long    	FmtSize ;
	u_short   	wFormatTag ;
	u_short   	nChannels ;
	u_long		nSamplesPerSec ;
	u_long		nAvgBytesPerSec ;
	u_short		nBlockAlign ;
	u_short		wBitsPerSample ;
	char		DataID [4] ;
	u_long		nDataBytes ;
} WAVE_HEADER ;


static void waveFormatCopy( WAVEFORMAT* wav, char *ptr );
static char*  findchunk (char* s1, const char* s2, size_t n) ;

static int  WaveHeaderCheck  (char *wave_buf,int* channels, u_long* samplerate, int* samplebits, u_long* samples,u_long* datastart)
{	
	static  WAVEFORMAT  waveformat ;
	char*   ptr ;
	u_long  databytes ;

	if (findchunk (wave_buf, "RIFF", BUFFERSIZE) != wave_buf) {
		fprintf(stderr, "Bad format: Cannot find RIFF file marker");
		return  WR_BADRIFF ;
	}

	if (! findchunk (wave_buf, "WAVE", BUFFERSIZE)) {
		fprintf(stderr, "Bad format: Cannot find WAVE file marker");
		return  WR_BADWAVE ;
	}

	ptr = findchunk (wave_buf, "fmt ", BUFFERSIZE) ;

	if (! ptr) {
		fprintf(stderr, "Bad format: Cannot find 'fmt' file marker");
		return  WR_BADFORMAT ;
	}

	ptr += 4 ;	/* Move past "fmt ".*/
	waveFormatCopy( &waveformat, ptr );
	
	if (waveformat.dwSize != (sizeof (WAVEFORMAT) - sizeof (u_long))) {
		/* fprintf(stderr, "Bad format: Bad fmt size"); */
		/* return  WR_BADFORMATSIZE ; */
	}

	if (waveformat.wFormatTag != PCM_WAVE_FORMAT) {
		fprintf(stderr, "Only supports PCM wave format");
		return  WR_NOTPCMFORMAT ;
	}

	ptr = findchunk (wave_buf, "data", BUFFERSIZE) ;

	if (! ptr) {
		fprintf(stderr,"Bad format: unable to find 'data' file marker");
		return  WR_NODATACHUNK ;
	}

	ptr += 4 ;	/* Move past "data".*/
	databytes = LittleEndian_getDW(ptr, 0);
	
	/* Everything is now cool, so fill in output data.*/

	*channels   = waveformat.wChannels;
	*samplerate = waveformat.dwSamplesPerSec ;
	*samplebits = waveformat.wBitsPerSample ;
	*samples    = databytes / waveformat.wBlockAlign ;
	
	*datastart  = (u_long)(ptr) + 4;

	if (waveformat.dwSamplesPerSec != waveformat.dwAvgBytesPerSec / waveformat.wBlockAlign) {
		fprintf(stderr, "Bad file format");
		return  WR_BADFORMATDATA ;
	}

	if (waveformat.dwSamplesPerSec != waveformat.dwAvgBytesPerSec / waveformat.wChannels / ((waveformat.wBitsPerSample == 16) ? 2 : 1)) {
		fprintf(stderr, "Bad file format");
		return  WR_BADFORMATDATA ;
	}

	return  0 ;
} ; /* WaveHeaderCheck*/


static char* findchunk  (char* pstart, const char* fourcc, size_t n)
{	char	*pend ;
	int		k, test ;

	pend = pstart + n ;

	while (pstart < pend)
	{ 
		if (*pstart == *fourcc)       /* found match for first char*/
		{	test = 1 ;
			for (k = 1 ; fourcc [k] != 0 ; k++)
				test = (test ? ( pstart [k] == fourcc [k] ) : 0) ;
			if (test)
				return  pstart ;
			} ; /* if*/
		pstart ++ ;
		} ; /* while lpstart*/

	return  NULL ;
} ; /* findchuck*/

static void waveFormatCopy( WAVEFORMAT* wav, char *ptr ) {
	wav->dwSize           = LittleEndian_getDW( ptr,  0 );
	wav->wFormatTag       = LittleEndian_getW(  ptr,  4 );
	wav->wChannels        = LittleEndian_getW(  ptr,  6 );
	wav->dwSamplesPerSec  = LittleEndian_getDW( ptr,  8 );
	wav->dwAvgBytesPerSec = LittleEndian_getDW( ptr, 12 );
	wav->wBlockAlign      = LittleEndian_getW(  ptr, 16 );
	wav->wBitsPerSample   = LittleEndian_getW(  ptr, 18 );
}

static char* WavGetInfo(WAVFILE* wfile, char *data) {
	int e;					/* Saved errno value */
	int channels;				/* Channels recorded in this wav file */
	u_long samplerate;			/* Sampling rate */
	int sample_bits;			/* data bit size (8/12/16) */
	u_long samples;				/* The number of samples in this file */
	u_long datastart;			/* The offset to the wav data */

	if ( (e = WaveHeaderCheck(data,
				  &channels,&samplerate,
				  &sample_bits,&samples,&datastart) != 0 )) {
		fprintf(stderr,"WavGetInfo(): Reading WAV header\n");
		return 0;
	}
	
	/*
	 * Copy WAV data over to WAVFILE struct:
	 */
	wfile->wavinfo.Channels = channels;

	wfile->wavinfo.SamplingRate = (unsigned int) samplerate;
	wfile->wavinfo.DataBits = (unsigned short) sample_bits;

	return (char *) datastart;
}

/************************************************************:
**
**	WAVFILE stream reader
*/

WAVFILE::WAVFILE(void) {
	wavinfo.SamplingRate=0;
	wavinfo.Channels=1;
	wavinfo.DataBits=0;
}

int WAVFILE_Stream::Read(char* in_buf, int blksize, int length) {
	/* ファイルの読み込み */
	if (data_length == 0 && stream_length == 0) return -1;
	/* wf->data にデータの残りがあればそれも読み込む */
	if (data_length > blksize*length) {
		memcpy(in_buf, data, blksize*length);
		data += blksize * length;
		data_length -= blksize * length;
		return length;
	}
	memcpy(in_buf, data, data_length);
	if (stream_length != -1 && stream_length < blksize*length-data_length) {
		length = (stream_length+data_length+blksize-1)/blksize;
	}
	int read_len = 0;
	if (blksize*length-data_length > 0) {
		read_len = fread(in_buf+data_length, 1, blksize*length-data_length, stream);
		if (stream_length != -1 && stream_length > read_len) stream_length -= read_len;
		if (feof(stream)) stream_length = 0; // end of file
	} else {
		stream_length = 0; // all data were read
	}
	int blklen = (read_len + data_length) / blksize;
	data_length = 0;
	return blklen;
}
void WAVFILE_Stream::Seek(int count) {
        int blksize = 1;
        /* block size の設定 */
	blksize *= wavinfo.Channels * (wavinfo.DataBits/8);
	data_length = 0;
	stream_length = stream_length_orig - stream_top - count*blksize;
	fseek(stream, count*blksize+stream_top, 0);
}
WAVFILE_Stream::WAVFILE_Stream(FILE* _stream, int _length) {
	stream = _stream;
	stream_length = _length;
	stream_length_orig = _length;
	data_orig = new char[1024];
	data = data_orig;
	data_length = 1024;
	if (stream_length != -1 && stream_length < data_length) {
		data_length = stream_length;
	}
	fread(data, data_length, 1, stream);
	if (stream_length != -1)
		stream_length -= data_length;
	data = WavGetInfo(this, data);
	if (data == 0) {
		stream_length = 0;
		data_length = 0;
		return;
	}
	stream_top = data - data_orig;
	data_length -= data - data_orig;
	return;
}
WAVFILE_Stream::~WAVFILE_Stream() {
	if (data_orig) delete data_orig;
	if (stream) fclose(stream);
	return;
}
/************************************************************:
**
**	WAVE format converter with SDL_audio
*/
WAVFILE* WAVFILE::MakeConverter(WAVFILE* new_reader) {
	bool need = false;
	if (new_reader->wavinfo.SamplingRate != freq) need = true;
	if (new_reader->wavinfo.Channels != channels) need = true;
	if (format == AUDIO_S8) {
		if (new_reader->wavinfo.DataBits != 8) need = true;
	} else if (format == AUDIO_S16) {
		if (new_reader->wavinfo.DataBits != 16) need = true;
	} else {
		need = true;
	}
	if (!need) return new_reader;
	/* 変換もとのフォーマットを得る */
	int from_format;
	if (new_reader->wavinfo.DataBits == 8) from_format = AUDIO_S8;
	else from_format = AUDIO_S16;
	SDL_AudioCVT* cvt = new SDL_AudioCVT;
	int ret = SDL_BuildAudioCVT(cvt, from_format, new_reader->wavinfo.Channels, freq,
		format, 2, freq);
	if (ret == -1) {
		delete cvt;
		fprintf(stderr,"Cannot make wave file converter!!!\n");
		return new_reader;
	}
	WAVFILE_Converter* conv = new WAVFILE_Converter(new_reader, cvt);
	return conv;
}
WAVFILE_Converter::WAVFILE_Converter(WAVFILE* _orig, SDL_AudioCVT* _cvt) {
	original = _orig;
	cvt = _cvt;
	//datasize = 4096*4;
	datasize = 48000;
	cvt->buf = new Uint8[datasize*cvt->len_mult];
	cvt->len = 0;
	tmpbuf = new char[datasize*cvt->len_mult + 1024];
	memset(tmpbuf, 0, datasize*cvt->len_mult+1024);
};

static int conv_wave_rate(short* in_buf, int length, int in_rate, int out_rate, char* tmpbuf);
WAVFILE_Converter::~WAVFILE_Converter() {
	if (cvt) {
		if (cvt->buf) delete cvt->buf;
		delete cvt;
		cvt = 0;
	}
	if (original) delete original;
	original = 0;
}
int WAVFILE_Converter::Read(char* buf, int blksize, int blklen) {
	if (original == 0 || cvt == 0) return -1;
	int copied_length = 0;
	if (cvt->len < blksize*blklen) {
		memcpy(buf, cvt->buf, cvt->len);
		copied_length += cvt->len;
		do {
			int cnt = original->Read((char*)cvt->buf, 1, datasize);
			if (cnt <= 0) {
				cvt->len = 0;
				break;
			}
			cvt->len = cnt;
			SDL_ConvertAudio(cvt);
			if (freq < original->wavinfo.SamplingRate) { // rate conversion は SDL_ConvertAudio ではうまく行かない
				// 48000Hz -> 44100Hz or 22050Hz などを想定
				// 長さは短くなるはずなので、特に処理はなし
				cvt->len = conv_wave_rate( (short*)(cvt->buf), cvt->len_cvt/4, original->wavinfo.SamplingRate, freq, tmpbuf);
				cvt->len *= 4;
			} else {
				cvt->len = cvt->len_cvt;
			}
			if (cvt->len+copied_length > blksize*blklen) break;
			memcpy(buf+copied_length, cvt->buf, cvt->len);
			copied_length += cvt->len;
		} while(1);
	}
	if (cvt->len == 0 && copied_length == 0) return -1;
	else if (cvt->len > 0) {
		int len = blksize * blklen - copied_length;
		memcpy(buf+copied_length, cvt->buf, len);
		memmove(cvt->buf, cvt->buf+len, cvt->len-len);
		copied_length += len;
		cvt->len -= len;
	}
	return copied_length / blksize;
}
/* format は signed, 16bit, little endian, stereo と決めうち
** 場合によっていは big endian になることもあるかも。
*/
static int conv_wave_rate(short* in_buf, int length, int in_rate, int out_rate, char* tmpbuf) {
	int input_rate = in_rate;
	int output_rate = out_rate;
	double input_rate_d = input_rate, output_rate_d = output_rate;
	double dtime; int outlen; short* out, * out_orig; int next_sample1, next_sample2;
	short* in_buf_orig = in_buf;
	int i; int time;

	if (input_rate == output_rate) return length;
	if (length <= 0) return 0;
	/* 一般の周波数変換：線型補完 */
	int& first_flag = *(int*)(tmpbuf);
	int& prev_time = *(int*)(tmpbuf+4);
	int& prev_sample1 = *(int*)(tmpbuf+8);
	int& prev_sample2 = *(int*)(tmpbuf+12);
	out = (short*)(tmpbuf+16);
	/* 初めてならデータを初期化 */
	if (first_flag == 0) {
		first_flag = 1;
		prev_time = 0;
		prev_sample1 = short(read_little_endian_short((char*)(in_buf++)));
		prev_sample2 = short(read_little_endian_short((char*)(in_buf++)));
		length--;
	}
	/* 今回作成するデータ量を得る */
	dtime = prev_time + length * output_rate_d;
	outlen = (int)(dtime / input_rate_d);
	out_orig = out;
	if (first_flag == 1) {
		write_little_endian_short((char*)out, prev_sample1);
		out++;
		write_little_endian_short((char*)out, prev_sample2);
		out++;
	}
	dtime -= input_rate_d*outlen; /* 次の prev_time */

	time=0;
	next_sample1 = short(read_little_endian_short((char*)(in_buf++)));
	next_sample2 = short(read_little_endian_short((char*)(in_buf++)));
	for (i=0; i<outlen; i++) {
		/* double で計算してみたけどそう簡単には高速化は無理らしい */
		/* なお、変換は 1分のデータに1秒程度かかる(Celeron 700MHz) */
		time += input_rate;
		while(time-prev_time>output_rate) {
			prev_sample1 = next_sample1;
			next_sample1 = short(read_little_endian_short((char*)(in_buf++)));
			prev_sample2 = next_sample2;
			next_sample2 = short(read_little_endian_short((char*)(in_buf++)));
			prev_time += output_rate;
		}
		write_little_endian_short((char*)out,
			((time-prev_time)*next_sample1 +
			(input_rate-time+prev_time)*prev_sample1) / input_rate);
		out++;
		write_little_endian_short((char*)out,
			((time-prev_time)*next_sample2 +
			(input_rate-time+prev_time)*prev_sample2) / input_rate);
		out++;
	}
	prev_time += output_rate; prev_time -= input_rate * outlen;
	prev_sample1 = next_sample1; prev_sample2 = next_sample2;
	if (first_flag == 1) {
		outlen++; first_flag = 2;
	}
	memcpy(in_buf_orig, out_orig, outlen*2*sizeof(short));
	return outlen;
}

int WAVFILE::freq = 48000;
int WAVFILE::channels = 2;
int WAVFILE::format = MIX_DEFAULT_FORMAT;

