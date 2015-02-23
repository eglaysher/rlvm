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

#include <SDL/SDL_mixer.h>

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


/************************************************************:
**
**	MP3FILE stream reader
*/

int WAVFILE::freq = 48000;
int WAVFILE::channels = 2;
int WAVFILE::format = MIX_DEFAULT_FORMAT;

#if HAVE_LIBMAD

#include<mad.h>
#define MPEG_BUFSZ       40000   /* 2.5 s at 128 kbps; 1 s at 320 kbps */
struct MP3FILE_impl {
	enum { PREPARE, RUN, WRITE, DONE} status;
	struct mad_decoder decoder;
	char* data;
	int data_len;
	char* write_data;
	unsigned int write_data_len;
	unsigned int write_pointer;
	unsigned int src_pointer;
	FILE* stream;
	MP3FILE_impl(FILE*);
	~MP3FILE_impl();
	static enum mad_flow callback_read(void *data, struct mad_stream *stream);
	static enum mad_flow callback_error(void *data, struct mad_stream *stream, struct mad_frame *frame);
	static enum mad_flow callback_write(void *data, struct mad_header const *header, struct mad_pcm *pcm);
	enum mad_flow callback_write_impl(struct mad_pcm *pcm);
	void run(void);
};

MP3FILE_impl::MP3FILE_impl(FILE* _stream) {
	stream = _stream;
	data = new char[MPEG_BUFSZ];
	data_len = 0;
	src_pointer = 0;
	write_data = 0;
	write_data_len = 0;
	write_pointer = 0;

	/* initialize decoder */
	mad_decoder_init(&decoder, (void*)this, callback_read, 0 /* header */, 0 /* filter */, callback_write,
 			callback_error, 0 /* message */);
	/* prepare stream */
	status = PREPARE;
	*(void**)(&decoder.sync) = malloc(sizeof(*decoder.sync));

	mad_stream_init(&decoder.sync->stream);
	mad_frame_init(&decoder.sync->frame);
	mad_synth_init(&decoder.sync->synth);

	mad_stream_options(&decoder.sync->stream, decoder.options);

	while(status != WRITE && status != DONE) run();
}
MP3FILE_impl::~MP3FILE_impl() {
	free(decoder.sync);
	mad_decoder_finish(&decoder);
	delete[] data;
	return;
}

void MP3FILE_impl::run(void) {
	if (status == DONE) return;
	struct mad_stream *stream = &decoder.sync->stream;
	struct mad_frame *frame = &decoder.sync->frame;
	struct mad_synth *synth = &decoder.sync->synth;
	if (status == PREPARE) {
		switch (decoder.input_func(decoder.cb_data, stream)) {
		case MAD_FLOW_STOP:
		case MAD_FLOW_BREAK:
			goto done;
		case MAD_FLOW_CONTINUE:
			status = RUN;
		case MAD_FLOW_IGNORE:
			break;
		}
		return;
	}
	if (status == RUN) {
		if (mad_frame_decode(frame, stream) == -1) {
			if (!MAD_RECOVERABLE(stream->error)) {
				status = PREPARE;
				return;
			}
			switch (decoder.error_func((void*)this, stream, frame)) {
			case MAD_FLOW_STOP:
			case MAD_FLOW_BREAK:
				goto done;
			case MAD_FLOW_IGNORE:
				status = PREPARE;
				return;
			case MAD_FLOW_CONTINUE:
			default:
				return;
			}
		}

		mad_synth_frame(synth, frame);
		src_pointer = 0;
		status = WRITE;
		return;
	}
	if (status == WRITE) {
		switch (decoder.output_func(decoder.cb_data, &frame->header, &synth->pcm)) {
		case MAD_FLOW_STOP:
		case MAD_FLOW_BREAK:
			goto done;
		case MAD_FLOW_IGNORE:
			return;
		case MAD_FLOW_CONTINUE:
			status = RUN;
			break;
		}
		if (stream->error == MAD_ERROR_BUFLEN) {
			stream->error = MAD_ERROR_NONE;
			status = PREPARE;
		}
		return;
	}
done:
	status = DONE;
	mad_synth_finish(&decoder.sync->synth);
	mad_frame_finish(&decoder.sync->frame);
	mad_stream_finish(&decoder.sync->stream);
	return;
}

enum mad_flow MP3FILE_impl::callback_read(void *data, struct mad_stream *stream)
{
	MP3FILE_impl* impl = (MP3FILE_impl*)data;
	if (stream->next_frame) {
		impl->data_len -= (char*)stream->next_frame - impl->data;
		memmove(impl->data, (char*)stream->next_frame, impl->data_len);
	} else {
		impl->data_len = 0;
	}
	int count;
	if (feof(impl->stream)) {
		if (stream->next_frame && (char*)stream->next_frame - impl->data > 0) {
			// There is under processing data
			count = 0;
		} else {
			// all data were processed
			return MAD_FLOW_STOP;
		}
	} else {
		count = fread(impl->data + impl->data_len, 1, MPEG_BUFSZ-impl->data_len, impl->stream);
		if (count <= 0) {
			return MAD_FLOW_BREAK;
		}
	}
	impl->data_len += count;
	if (impl->data_len < MPEG_BUFSZ) {
		memset(impl->data + impl->data_len, 0, MPEG_BUFSZ-impl->data_len);
	}
	mad_stream_buffer(stream, (unsigned char*)impl->data, impl->data_len);
	return MAD_FLOW_CONTINUE;
}

enum mad_flow MP3FILE_impl::callback_error(void *data, struct mad_stream *stream, struct mad_frame *frame)
{
	MP3FILE_impl* impl = (MP3FILE_impl*)data;
	fprintf(stdout, "decoding error 0x%04x (%s) at byte offset %u\n",
	  stream->error, mad_stream_errorstr(stream),
	  ftell(impl->stream) - ((impl->data+impl->data_len)-(char*)stream->this_frame));
	/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */
	return MAD_FLOW_CONTINUE;
}
signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}
enum mad_flow MP3FILE_impl::callback_write(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
	MP3FILE_impl* pimpl = (MP3FILE_impl*)data;
	return pimpl->callback_write_impl(pcm);
}
enum mad_flow MP3FILE_impl::callback_write_impl(struct mad_pcm *pcm)
{
	if (write_data_len == 0) return MAD_FLOW_IGNORE;
	mad_fixed_t const *left_ch = pcm->samples[0] + src_pointer;
	mad_fixed_t const *right_ch = pcm->samples[1] + src_pointer;

	unsigned int nchannels = pcm->channels;
	unsigned int nsamples = pcm->length - src_pointer;
	if (write_pointer + nsamples * nchannels * 2 > write_data_len) {
		nsamples = (write_data_len - write_pointer) / nchannels / 2;
	}
	write_data_len &= ~(nchannels*2-1);	/* write_data_len はあらかじめ丸めておく */
	src_pointer += nsamples;
	if (write_data == 0) { // skip data write
		write_pointer += nsamples*2*2;
	} else while(nsamples--) {
		signed int sample = scale(*left_ch++);
		write_data[write_pointer++] = sample & 0xff;
		write_data[write_pointer++] = (sample>>8) & 0xff;
		if (nchannels == 2) {
			sample = scale(*right_ch++);
		}
		write_data[write_pointer++] = sample & 0xff;
		write_data[write_pointer++] = (sample>>8) & 0xff;
	}
	if (write_pointer >= write_data_len) return MAD_FLOW_IGNORE;
	else return MAD_FLOW_CONTINUE;
}

MP3FILE::MP3FILE(FILE* stream, int len) {
	pimpl = new MP3FILE_impl(stream);
	if (pimpl->status == MP3FILE_impl::DONE) {
		delete pimpl;
		pimpl = 0;
		fclose(stream);
		return;
	}
	wavinfo.SamplingRate = pimpl->decoder.sync->synth.pcm.samplerate;
	wavinfo.Channels = 2;
	wavinfo.DataBits = 16;
}
MP3FILE::~MP3FILE() {
	if (pimpl) {
		FILE* s = pimpl->stream;
		delete pimpl;
		fclose(s);
	}
	pimpl = 0;
}
int MP3FILE::Read(char* buf, int blksize, int blklen) {
	if (pimpl == 0) return -1;
	pimpl->write_data = buf;
	pimpl->write_data_len = blksize*blklen;
	pimpl->write_pointer = 0;
	do {
		pimpl->run();
	} while(pimpl->status != MP3FILE_impl::DONE && pimpl->write_pointer < pimpl->write_data_len);
	return pimpl->write_pointer / blksize;
}
void MP3FILE::Seek(int count) {
	FILE* stream = pimpl->stream;
	delete pimpl;
	fseek(stream,0,0);
	pimpl = new MP3FILE_impl(stream);
	if (pimpl->status == MP3FILE_impl::DONE) {
		delete pimpl;
		pimpl = 0;
		fclose(stream);
		return;
	}
	int blksize = 1;
	blksize *= wavinfo.Channels * (wavinfo.DataBits/8);
	pimpl->write_data = 0;
	pimpl->write_data_len = count * blksize;
	pimpl->write_pointer = 0;
	do {
		pimpl->run();
	} while(pimpl->status != MP3FILE_impl::DONE && pimpl->write_pointer < pimpl->write_data_len);
	return;
}
#elif HAVE_LIBSMPEG
#include<smpeg/smpeg.h>

struct MP3FILE_impl {
	SMPEG* info;
	FILE* stream;
	MP3FILE_impl(FILE*);
};

MP3FILE_impl::MP3FILE_impl(FILE* _stream) {
	stream = _stream;
	info = SMPEG_new_descr(fileno(stream), NULL, 0);
fprintf(stderr,"mp3 %x\n",info);
	if (info && SMPEG_error(info) ) info = 0;
	SMPEG_enableaudio(info, 0);
	SMPEG_enableaudio(info, 1);
	SMPEG_play(info);
}

MP3FILE::MP3FILE(FILE* stream, int len) {
	pimpl = new MP3FILE_impl(stream);
	if (pimpl->info == 0) {
		delete pimpl;
		fclose(stream);
		return;
	}
	SDL_AudioSpec fmt;
	SMPEG_wantedSpec(pimpl->info, &fmt);
	wavinfo.SamplingRate = fmt.freq;
	wavinfo.Channels = fmt.channels;
	wavinfo.DataBits = (fmt.format == AUDIO_S8) ? 8:16;
}
MP3FILE::~MP3FILE() {
	if (pimpl && pimpl->info) {
		if (SMPEG_status(pimpl->info) == SMPEG_PLAYING) SMPEG_stop(pimpl->info);
		SMPEG_delete(pimpl->info);
	}
	if (pimpl) {
		fclose(pimpl->stream);
		delete pimpl;
		pimpl = 0;
	}
}
int MP3FILE::Read(char* buf, int blksize, int blklen) {
	if (pimpl == 0 || pimpl->info == 0) return -1;
	int r = SMPEG_playAudio(pimpl->info, (Uint8*)buf, blksize*blklen);
	if (r <= 0) { // end of file
		return -1;
	}
	return r / blksize;
}
void MP3FILE::Seek(int count) {
	if (pimpl == 0 || pimpl->info == 0) return;
	SMPEG_stop(pimpl->info);
	SMPEG_rewind(pimpl->info);
	SMPEG_play(pimpl->info);
	count /= 4;
	count *= 4; // reduce noise; possibly SMPEG error
	char* d = new char[count*channels*2];
	Read(d,count,channels*2);
	delete[] d;
	return;
}
#else /* SMPEG */
MP3FILE::MP3FILE(FILE* stream, int len) {
  throw std::runtime_error("MP3 support not compiled in!");
}
MP3FILE::~MP3FILE(){}
void MP3FILE::Seek(int count){}
int MP3FILE::Read(char* buf, int blksize, int blklen){return -1;}
#endif /* SMPEG */
