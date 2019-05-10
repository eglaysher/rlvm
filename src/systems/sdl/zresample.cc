// ----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2011 Fons Adriaensen <fons@linuxaudio.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------
//
// This is a modified version of zresample, with getopt() and command line
// parsing removed so it can be called from within rlvm.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <zita-resampler/resampler.h>
#include "audiofile.h"


enum { HELP, CAF, WAV, AMB, BIT16, BIT24, FLOAT, RATE, REC, TRI, LIPS, PAD };
enum { BUFFSIZE = 0x4000, FILTSIZE = 96 };


static unsigned int type = Audiofile::TYPE_WAV;
static unsigned int form = Audiofile::FORM_16BIT;
static unsigned int dith = Audiofile::DITHER_NONE;
static bool         zpad = false;

int zresample_main(const char* infile, const char* outfile, unsigned int rout) {
    Audiofile     Ainp;
    Audiofile     Aout;
    Resampler     R;
    unsigned int  k, chan, rinp, z1, z2;
    float         *inpb, *outb;
    bool          done;

    if (Ainp.open_read(infile))
    {
	fprintf (stderr, "Can't open input file '%s'.\n", infile);
	return 1;
    }

    chan = Ainp.chan ();
    rinp = Ainp.rate ();

    if (rout != rinp)
    {
        if ((rinp < 8000) || (rinp > 192000))
        {
	    fprintf (stderr, "Input sample %d rate is out of range.\n", rinp);
	    Ainp.close ();
	    return 1;
        }
        if ((rout < 8000) || (rout > 192000))
        {
	    fprintf (stderr, "Output sample rate %d is out of range.\n", rout);
    	    Ainp.close ();
	    return 1;
        }
        if (R.setup (rinp, rout, chan, FILTSIZE))
        {
	    fprintf (stderr, "Sample rate ratio %d/%d is not supported.\n", rout, rinp);
	    Ainp.close ();
	    return 1;
        }
    }

    if (Aout.open_write(outfile, type, form, rout, chan))
    {
	fprintf (stderr, "Can't open output file '%s'.\n", outfile);
	Ainp.close ();
	return 1;
    }
    if (dith != Audiofile::DITHER_NONE) 
    {
	Aout.set_dither (dith);
    }

    if (zpad)
    {
	z1 = R.inpsize () - 1;
	z2 = R.inpsize () - 1;
    }
    else
    {
	z1 = R.inpsize () / 2 - 1;
	z2 = R.inpsize () / 2;
    }

    inpb = new float [chan * BUFFSIZE];
    if (rout != rinp)
    {
        outb = new float [chan * BUFFSIZE];
	// Insert zero samples at start.
        R.inp_count = z1;
        R.inp_data = 0;
        R.out_count = BUFFSIZE;
        R.out_data = outb;
        done = false;
        while (true)
        {
            R.process ();
  	    if (R.inp_count == 0)
	    {
	        // Input buffer empty, read more samples, insert
	        // zeros at the end, or terminate.
	        if (done)
	        {
		    // We already inserted final zero samples.
		    // Write out any remaining output samples and terminate.
                    Aout.write (outb, BUFFSIZE - R.out_count);
		    break;
	        }
                k = Ainp.read (inpb, BUFFSIZE);
	        if (k)
	        {
		    // Process next 'k' input samples.
                    R.inp_count = k;
                    R.inp_data = inpb;
	        }
	        else
	        {
		    // At end of input, insert zero samples.
                    R.inp_count = z2;
                    R.inp_data = 0;
		    done = true;
	        }
	    }
	    if (R.out_count == 0)
	    {
	        // Output buffer full, write to file.
                Aout.write (outb, BUFFSIZE);
                R.out_count = BUFFSIZE;
                R.out_data = outb;
	    }
	}
	delete[] outb;
    }
    else
    {
	// No resampling, just copy.
	while (1)
	{
            k = Ainp.read (inpb, BUFFSIZE);
	    if (k) Aout.write (inpb, k);
	    else break;
	}
    }

    Ainp.close ();
    Aout.close ();
    delete[] inpb;

    return 0;
}
