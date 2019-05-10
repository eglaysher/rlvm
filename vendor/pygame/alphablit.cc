/*
    pygame - Python Game Library
    Copyright (C) 2000-2001  Pete Shinners
    Copyright (C) 2006  Rene Dudfield

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

    Pete Shinners
    pete@shinners.org
*/

#include <SDL.h>


#define PYGAME_BLEND_ADD  0x1
#define PYGAME_BLEND_SUB  0x2
#define PYGAME_BLEND_MULT 0x3
#define PYGAME_BLEND_MIN  0x4
#define PYGAME_BLEND_MAX  0x5



/* The structure passed to the low level blit functions */
typedef struct {
        Uint8 *s_pixels;
        int s_width;
        int s_height;
        int s_skip;
        Uint8 *d_pixels;
        int d_width;
        int d_height;
        int d_skip;
        void *aux_data;
        SDL_PixelFormat *src;
        Uint8 *table;
        SDL_PixelFormat *dst;
} SDL_BlitInfo;
static void alphablit_alpha(SDL_BlitInfo *info);
static void alphablit_colorkey(SDL_BlitInfo *info);
static void alphablit_solid(SDL_BlitInfo *info);

static int SoftBlitPyGame(SDL_Surface *src, SDL_Rect *srcrect,
                          SDL_Surface *dst, SDL_Rect *dstrect, int the_args);
extern int SDL_RLESurface(SDL_Surface *surface);
extern void SDL_UnRLESurface(SDL_Surface *surface, int recode);












static int SoftBlitPyGame(SDL_Surface *src, SDL_Rect *srcrect,
                        SDL_Surface *dst, SDL_Rect *dstrect, int the_args)
{
        int okay;
        int src_locked;
        int dst_locked;

    /* Everything is okay at the beginning...  */
        okay = 1;

        /* Lock the destination if it's in hardware */
        dst_locked = 0;
        if ( SDL_MUSTLOCK(dst) ) {
                if ( SDL_LockSurface(dst) < 0 )
                        okay = 0;
                else
                        dst_locked = 1;
        }
        /* Lock the source if it's in hardware */
        src_locked = 0;
        if ( SDL_MUSTLOCK(src) ) {
                if ( SDL_LockSurface(src) < 0 )
                        okay = 0;
                else
                        src_locked = 1;
        }

        /* Set up source and destination buffer pointers, and BLIT! */
        if ( okay  && srcrect->w && srcrect->h ) {
                SDL_BlitInfo info;

                /* Set up the blit information */
                info.s_pixels = (Uint8 *)src->pixels +
                                (Uint16)srcrect->y*src->pitch +
                                (Uint16)srcrect->x*src->format->BytesPerPixel;
                info.s_width = srcrect->w;
                info.s_height = srcrect->h;
                info.s_skip=src->pitch-info.s_width*src->format->BytesPerPixel;
                info.d_pixels = (Uint8 *)dst->pixels +
                                (Uint16)dstrect->y*dst->pitch +
                                (Uint16)dstrect->x*dst->format->BytesPerPixel;
                info.d_width = dstrect->w;
                info.d_height = dstrect->h;
                info.d_skip=dst->pitch-info.d_width*dst->format->BytesPerPixel;
                info.src = src->format;
                info.dst = dst->format;

                switch(the_args) {
                    case 0:
                    {
                        // TODO(erg): We used to check SDL_SRCALPHA here.
                      //                        if(src->format->Amask)
                            alphablit_alpha(&info);
                        // else if(SDL_GetColorKey(src, NULL) > 0)
                        //     alphablit_colorkey(&info);
                        // else
                        //     alphablit_solid(&info);
                        break;
                    }
                    default:
                    {
                        SDL_SetError("Invalid argument passed to blit.");
                        okay = 0;
                        break;
                    }
                }

        }

        /* We need to unlock the surfaces if they're locked */
        if ( dst_locked )
                SDL_UnlockSurface(dst);
        if ( src_locked )
                SDL_UnlockSurface(src);
        /* Blit is done! */
        return(okay ? 0 : -1);
}



/*
 * Macros below are for the blit blending functions.
 */


#define GET_PIXEL(buf, bpp, fmt, pixel)                    \
do {                                                                       \
        switch (bpp) {                                                           \
                case 1:                                                           \
                        pixel = *((Uint8 *)(buf));                           \
                break;                                                       \
                case 2:                                                           \
                        pixel = *((Uint16 *)(buf));                           \
                break;                                                           \
                case 4:                                                           \
                        pixel = *((Uint32 *)(buf));                           \
                break;                                                           \
                default:        {/* case 3: FIXME: broken code (no alpha) */                   \
                        Uint8 *b = (Uint8 *)buf;                           \
                        if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {                   \
                                pixel = b[0] + (b[1] << 8) + (b[2] << 16); \
                        } else {                                           \
                                pixel = (b[0] << 16) + (b[1] << 8) + b[2]; \
                        }                                                   \
                }                                                           \
                break;                                                           \
        }                                                                   \
} while(0)


#define DISEMBLE_RGBA(buf, bpp, fmt, pixel, R, G, B, A)                    \
do {                                                                       \
        if(bpp==1){\
            pixel = *((Uint8 *)(buf));                           \
            R = fmt->palette->colors[pixel].r; \
            G = fmt->palette->colors[pixel].g; \
            B = fmt->palette->colors[pixel].b; \
            A = 255; \
        } else { \
        switch (bpp) {                                                           \
                case 2:                                                           \
                        pixel = *((Uint16 *)(buf));                           \
                break;                                                           \
                case 4:                                                           \
                        pixel = *((Uint32 *)(buf));                           \
                break;                                                           \
                default:        {/* case 3: FIXME: broken code (no alpha) */                   \
                        Uint8 *b = (Uint8 *)buf;                           \
                        if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {                   \
                                pixel = b[0] + (b[1] << 8) + (b[2] << 16); \
                        } else {                                           \
                                pixel = (b[0] << 16) + (b[1] << 8) + b[2]; \
                        }                                                   \
                }                                                           \
                break;                                                           \
            }                                                                   \
            R = ((pixel&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss;                 \
            G = ((pixel&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss;                 \
            B = ((pixel&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss;                 \
            A = ((pixel&fmt->Amask)>>fmt->Ashift)<<fmt->Aloss;                 \
        }\
} while(0)


#define DISEMBLE_RGBA4(buf, bpp, fmt, pixel, R, G, B, A)                    \
                        pixel = *((Uint32 *)(buf));                           \
            R = ((pixel&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss;                 \
            G = ((pixel&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss;                 \
            B = ((pixel&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss;                 \
            A = ((pixel&fmt->Amask)>>fmt->Ashift)<<fmt->Aloss;                 \


#define PIXEL_FROM_RGBA(pixel, fmt, r, g, b, a)                         \
{                                                                       \
        pixel = ((r>>fmt->Rloss)<<fmt->Rshift)|                                \
                ((g>>fmt->Gloss)<<fmt->Gshift)|                                \
                ((b>>fmt->Bloss)<<fmt->Bshift)|                                \
                ((a<<fmt->Aloss)<<fmt->Ashift);                                \
}
#define ASSEMBLE_RGBA(buf, bpp, fmt, r, g, b, a)                        \
{                                                                       \
        switch (bpp) {                                                        \
                case 2: {                                                \
                        Uint16 pixel;                                        \
                        PIXEL_FROM_RGBA(pixel, fmt, r, g, b, a);        \
                        *((Uint16 *)(buf)) = pixel;                        \
                }                                                        \
                break;                                                        \
                case 4: {                                                \
                        Uint32 pixel;                                        \
                        PIXEL_FROM_RGBA(pixel, fmt, r, g, b, a);        \
                        *((Uint32 *)(buf)) = pixel;                        \
                }                                                        \
                break;                                                        \
        }                                                                \
}


#define ASSEMBLE_RGBA4(buf, bpp, fmt, r, g, b, a)                        \
                        Uint32 pixel;                                        \
                        PIXEL_FROM_RGBA(pixel, fmt, r, g, b, a);        \
                        *((Uint32 *)(buf)) = pixel;                        \



#if 0
#define ALPHA_BLEND(sR, sG, sB, sA, dR, dG, dB, dA)  \
do {                                            \
        dR = (((sR-dR)*(sA))>>8)+dR;                \
        dG = (((sG-dG)*(sA))>>8)+dG;                \
        dB = (((sB-dB)*(sA))>>8)+dB;                \
        dA = sA+dA - ((sA*dA)/255);                \
} while(0)
#else
#define ALPHA_BLEND(sR, sG, sB, sA, dR, dG, dB, dA)  \
do {   if(dA){\
        dR = ((dR<<8) + (sR-dR)*sA + sR) >> 8;	   \
        dG = ((dG<<8) + (sG-dG)*sA + sG) >> 8;     \
        dB = ((dB<<8) + (sB-dB)*sA + sB) >> 8;	   \
        dA = sA+dA - ((sA*dA)/255);                \
    }else{\
        dR = sR;                \
        dG = sG;                \
        dB = sB;                \
        dA = sA;               \
    }\
} while(0)
#endif

static void alphablit_alpha(SDL_BlitInfo *info)
{
        int n;
        int width = info->d_width;
        int height = info->d_height;
        Uint8 *src = info->s_pixels;
        int srcskip = info->s_skip;
        Uint8 *dst = info->d_pixels;
        int dstskip = info->d_skip;
        SDL_PixelFormat *srcfmt = info->src;
        SDL_PixelFormat *dstfmt = info->dst;
        int srcbpp = srcfmt->BytesPerPixel;
        int dstbpp = dstfmt->BytesPerPixel;
        int dR, dG, dB, dA, sR, sG, sB, sA;

        while ( height-- )
        {
            for(n=width; n>0; --n)
            {
                Uint32 pixel;
                DISEMBLE_RGBA(src, srcbpp, srcfmt, pixel, sR, sG, sB, sA);
                DISEMBLE_RGBA(dst, dstbpp, dstfmt, pixel, dR, dG, dB, dA);
                ALPHA_BLEND(sR, sG, sB, sA, dR, dG, dB, dA);
                ASSEMBLE_RGBA(dst, dstbpp, dstfmt, dR, dG, dB, dA);
                src += srcbpp;
                dst += dstbpp;
            }
            src += srcskip;
            dst += dstskip;
        }
}

// static void alphablit_colorkey(SDL_BlitInfo *info)
// {
//         int n;
//         int width = info->d_width;
//         int height = info->d_height;
//         Uint8 *src = info->s_pixels;
//         int srcskip = info->s_skip;
//         Uint8 *dst = info->d_pixels;
//         int dstskip = info->d_skip;
//         SDL_PixelFormat *srcfmt = info->src;
//         SDL_PixelFormat *dstfmt = info->dst;
//         int srcbpp = srcfmt->BytesPerPixel;
//         int dstbpp = dstfmt->BytesPerPixel;
//         int dR, dG, dB, dA, sR, sG, sB, sA;
//         int alpha = srcfmt->alpha;
//         Uint32 colorkey = srcfmt->colorkey;

//         while ( height-- )
//         {
//             for(n=width; n>0; --n)
//             {
//                 Uint32 pixel;
//                 DISEMBLE_RGBA(dst, dstbpp, dstfmt, pixel, dR, dG, dB, dA);
//                 DISEMBLE_RGBA(src, srcbpp, srcfmt, pixel, sR, sG, sB, sA);
//                 sA = (pixel == colorkey) ? 0 : alpha;
//                 ALPHA_BLEND(sR, sG, sB, sA, dR, dG, dB, dA);
//                 ASSEMBLE_RGBA(dst, dstbpp, dstfmt, dR, dG, dB, dA);
//                 src += srcbpp;
//                 dst += dstbpp;
//             }
//             src += srcskip;
//             dst += dstskip;
//         }
// }


// static void alphablit_solid(SDL_BlitInfo *info)
// {
//         int n;
//         int width = info->d_width;
//         int height = info->d_height;
//         Uint8 *src = info->s_pixels;
//         int srcskip = info->s_skip;
//         Uint8 *dst = info->d_pixels;
//         int dstskip = info->d_skip;
//         SDL_PixelFormat *srcfmt = info->src;
//         SDL_PixelFormat *dstfmt = info->dst;
//         int srcbpp = srcfmt->BytesPerPixel;
//         int dstbpp = dstfmt->BytesPerPixel;
//         int dR, dG, dB, dA, sR, sG, sB, sA;
//         int alpha = srcfmt->alpha;

//         while ( height-- )
//         {
//             for(n=width; n>0; --n)
//             {
//                 int pixel;
//                 DISEMBLE_RGBA(dst, dstbpp, dstfmt, pixel, dR, dG, dB, dA);
//                 DISEMBLE_RGBA(src, srcbpp, srcfmt, pixel, sR, sG, sB, sA);
//                 ALPHA_BLEND(sR, sG, sB, alpha, dR, dG, dB, dA);
//                 ASSEMBLE_RGBA(dst, dstbpp, dstfmt, dR, dG, dB, dA);
//                 src += srcbpp;
//                 dst += dstbpp;
//             }
//             src += srcskip;
//             dst += dstskip;
//         }
// }



/*we assume the "dst" has pixel alpha*/
int pygame_Blit(SDL_Surface *src, SDL_Rect *srcrect,
                   SDL_Surface *dst, SDL_Rect *dstrect, int the_args)
{
        SDL_Rect fulldst;
        int srcx, srcy, w, h;

        /* Make sure the surfaces aren't locked */
        if ( ! src || ! dst ) {
                SDL_SetError("SDL_UpperBlit: passed a NULL surface");
                return(-1);
        }
        if ( src->locked || dst->locked ) {
                SDL_SetError("Surfaces must not be locked during blit");
                return(-1);
        }

        /* If the destination rectangle is NULL, use the entire dest surface */
        if ( dstrect == NULL ) {
                fulldst.x = fulldst.y = 0;
                dstrect = &fulldst;
        }

        /* clip the source rectangle to the source surface */
        if(srcrect) {
                int maxw, maxh;

                srcx = srcrect->x;
                w = srcrect->w;
                if(srcx < 0) {
                        w += srcx;
                        dstrect->x -= srcx;
                        srcx = 0;
                }
                maxw = src->w - srcx;
                if(maxw < w)
                        w = maxw;

                srcy = srcrect->y;
                h = srcrect->h;
                if(srcy < 0) {
                        h += srcy;
                        dstrect->y -= srcy;
                        srcy = 0;
                }
                maxh = src->h - srcy;
                if(maxh < h)
                        h = maxh;

        } else {
                srcx = srcy = 0;
                w = src->w;
                h = src->h;
        }

        /* clip the destination rectangle against the clip rectangle */
        {
                SDL_Rect *clip = &dst->clip_rect;
                int dx, dy;

                dx = clip->x - dstrect->x;
                if(dx > 0) {
                        w -= dx;
                        dstrect->x += dx;
                        srcx += dx;
                }
                dx = dstrect->x + w - clip->x - clip->w;
                if(dx > 0)
                        w -= dx;

                dy = clip->y - dstrect->y;
                if(dy > 0) {
                        h -= dy;
                        dstrect->y += dy;
                        srcy += dy;
                }
                dy = dstrect->y + h - clip->y - clip->h;
                if(dy > 0)
                        h -= dy;
        }

        if(w > 0 && h > 0) {
                SDL_Rect sr;
                sr.x = srcx;
                sr.y = srcy;
                sr.w = dstrect->w = w;
                sr.h = dstrect->h = h;
                return SoftBlitPyGame(src, &sr, dst, dstrect, the_args);
        }
        dstrect->w = dstrect->h = 0;
        return 0;
}


int pygame_AlphaBlit (SDL_Surface *src, SDL_Rect *srcrect,
                   SDL_Surface *dst, SDL_Rect *dstrect)
{
    return pygame_Blit(src, srcrect, dst, dstrect, 0);
}

void pygame_stretch(SDL_Surface *src, SDL_Surface *dst) {
	int looph, loopw;

	Uint8* srcrow = (Uint8*)src->pixels;
	Uint8* dstrow = (Uint8*)dst->pixels;

	int srcpitch = src->pitch;
	int dstpitch = dst->pitch;

	int dstwidth = dst->w;
	int dstheight = dst->h;
	int dstwidth2 = dst->w << 1;
	int dstheight2 = dst->h << 1;

	int srcwidth2 = src->w << 1;
	int srcheight2 = src->h << 1;

	int w_err, h_err = srcheight2 - dstheight2;


	switch (src->format->BytesPerPixel) {
	case 1:
		for (looph = 0; looph < dstheight; ++looph) {
			Uint8 *srcpix = (Uint8*)srcrow, *dstpix = (Uint8*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for (loopw = 0; loopw < dstwidth; ++ loopw) {
				*dstpix++ = *srcpix;
				while (w_err >= 0) {++srcpix; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while (h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	case 2:
		for (looph = 0; looph < dstheight; ++looph) {
			Uint16 *srcpix = (Uint16*)srcrow, *dstpix = (Uint16*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for (loopw = 0; loopw < dstwidth; ++ loopw) {
				*dstpix++ = *srcpix;
				while (w_err >= 0) {++srcpix; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while (h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	case 3:
		for (looph = 0; looph < dstheight; ++looph) {
			Uint8 *srcpix = (Uint8*)srcrow, *dstpix = (Uint8*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for (loopw = 0; loopw < dstwidth; ++ loopw) {
				dstpix[0] = srcpix[0]; dstpix[1] = srcpix[1]; dstpix[2] = srcpix[2];
				dstpix += 3;
				while (w_err >= 0) {srcpix+=3; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while (h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	default: /*case 4:*/
		for (looph = 0; looph < dstheight; ++looph) {
			Uint32 *srcpix = (Uint32*)srcrow, *dstpix = (Uint32*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for (loopw = 0; loopw < dstwidth; ++ loopw) {
				*dstpix++ = *srcpix;
				while (w_err >= 0) {++srcpix; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while (h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	}
}
