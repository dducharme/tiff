/* $Header: /cvsroot/osrs/libtiff/tools/tiffsplit.c,v 1.3 2003/07/26 03:46:08 warmerda Exp $ */

/*
 * Copyright (c) 1992-1997 Sam Leffler
 * Copyright (c) 1992-1997 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiffio.h"

#define	streq(a,b)	(strcmp(a,b) == 0)
#define	CopyField(tag, v) \
    if (TIFFGetField(in, tag, &v)) TIFFSetField(out, tag, v)
#define	CopyField2(tag, v1, v2) \
    if (TIFFGetField(in, tag, &v1, &v2)) TIFFSetField(out, tag, v1, v2)
#define	CopyField3(tag, v1, v2, v3) \
    if (TIFFGetField(in, tag, &v1, &v2, &v3)) TIFFSetField(out, tag, v1, v2, v3)

static	char fname[1024+1];

static	int tiffcp(TIFF*, TIFF*);
static	void newfilename(void);
static	int cpStrips(TIFF*, TIFF*);
static	int cpTiles(TIFF*, TIFF*);

int
main(int argc, char* argv[])
{
	TIFF *in, *out;

	if (argc < 2) {
		fprintf(stderr, "usage: tiffsplit input.tif [prefix]\n");
		return (-3);
	}
	if (argc > 2)
		strcpy(fname, argv[2]);
	in = TIFFOpen(argv[1], "r");
	if (in != NULL) {
		do {
			char path[1024+1];
			newfilename();
			strcpy(path, fname);
			strcat(path, ".tif");
			out = TIFFOpen(path, "w");
			if (out == NULL)
				return (-2);
			if (!tiffcp(in, out))
				return (-1);
			TIFFClose(out);
		} while (TIFFReadDirectory(in));
		(void) TIFFClose(in);
	}
	return (0);
}

static void
newfilename(void)
{
	static int first = 1;
	static long lastTurn;
	static long fnum;
	static short defname;
	static char *fpnt;

	if (first) {
		if (fname[0]) {
			fpnt = fname + strlen(fname);
			defname = 0;
		} else {
			fname[0] = 'x';
			fpnt = fname + 1;
			defname = 1;
		}
		first = 0;
	}
#define	MAXFILES	17576
	if (fnum == MAXFILES) {
		if (!defname || fname[0] == 'z') {
			fprintf(stderr, "tiffsplit: too many files.\n");
			exit(1);
		}
		fname[0]++;
		fnum = 0;
	}
	if (fnum % 676 == 0) {
		if (fnum != 0) {
			//advance to next letter every 676 pages
			//condition for 'z'++ will be covered above
			fpnt[0]++;
		} else {
			//set to 'a' if we are on the very first file
			fpnt[0] = 'a';
		}
		//set the value of the last turning point
		lastTurn = fnum;
	}
	//start from 0 every 676 times (provided by lastTurn)
	//this keeps us within a-z boundaries
	fpnt[1] = (fnum - lastTurn) / 26 + 'a';
	//cycle last letter every file, from a-z, then repeat
	fpnt[2] = fnum % 26 + 'a';
	fnum++;
}

static int
tiffcp(TIFF* in, TIFF* out)
{
	short bitspersample, samplesperpixel, shortv, *shortav;
	uint32 w, l;
	float floatv;
	char *stringv;
	uint32 longv;

	CopyField(TIFFTAG_SUBFILETYPE, longv);
	CopyField(TIFFTAG_TILEWIDTH, w);
	CopyField(TIFFTAG_TILELENGTH, l);
	CopyField(TIFFTAG_IMAGEWIDTH, w);
	CopyField(TIFFTAG_IMAGELENGTH, l);
	CopyField(TIFFTAG_BITSPERSAMPLE, bitspersample);
	CopyField(TIFFTAG_COMPRESSION, shortv);
	CopyField(TIFFTAG_PREDICTOR, shortv);
	CopyField(TIFFTAG_PHOTOMETRIC, shortv);
	CopyField(TIFFTAG_THRESHHOLDING, shortv);
	CopyField(TIFFTAG_FILLORDER, shortv);
	CopyField(TIFFTAG_ORIENTATION, shortv);
	CopyField(TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
	CopyField(TIFFTAG_MINSAMPLEVALUE, shortv);
	CopyField(TIFFTAG_MAXSAMPLEVALUE, shortv);
	CopyField(TIFFTAG_XRESOLUTION, floatv);
	CopyField(TIFFTAG_YRESOLUTION, floatv);
	CopyField(TIFFTAG_GROUP3OPTIONS, longv);
	CopyField(TIFFTAG_GROUP4OPTIONS, longv);
	CopyField(TIFFTAG_RESOLUTIONUNIT, shortv);
	CopyField(TIFFTAG_PLANARCONFIG, shortv);
	CopyField(TIFFTAG_ROWSPERSTRIP, longv);
	CopyField(TIFFTAG_XPOSITION, floatv);
	CopyField(TIFFTAG_YPOSITION, floatv);
	CopyField(TIFFTAG_IMAGEDEPTH, longv);
	CopyField(TIFFTAG_TILEDEPTH, longv);
	CopyField(TIFFTAG_SAMPLEFORMAT, longv);
	CopyField2(TIFFTAG_EXTRASAMPLES, shortv, shortav);
	{ uint16 *red, *green, *blue;
	  CopyField3(TIFFTAG_COLORMAP, red, green, blue);
	}
	{ uint16 shortv2;
	  CopyField2(TIFFTAG_PAGENUMBER, shortv, shortv2);
	}
	CopyField(TIFFTAG_ARTIST, stringv);
	CopyField(TIFFTAG_IMAGEDESCRIPTION, stringv);
	CopyField(TIFFTAG_MAKE, stringv);
	CopyField(TIFFTAG_MODEL, stringv);
	CopyField(TIFFTAG_SOFTWARE, stringv);
	CopyField(TIFFTAG_DATETIME, stringv);
	CopyField(TIFFTAG_HOSTCOMPUTER, stringv);
	CopyField(TIFFTAG_PAGENAME, stringv);
	CopyField(TIFFTAG_DOCUMENTNAME, stringv);
	if (TIFFIsTiled(in))
		return (cpTiles(in, out));
	else
		return (cpStrips(in, out));
}

static int
cpStrips(TIFF* in, TIFF* out)
{
	tsize_t bufsize  = TIFFStripSize(in);
	unsigned char *buf = (unsigned char *)_TIFFmalloc(bufsize);

	if (buf) {
		tstrip_t s, ns = TIFFNumberOfStrips(in);
		uint32 *bytecounts;

		TIFFGetField(in, TIFFTAG_STRIPBYTECOUNTS, &bytecounts);
		for (s = 0; s < ns; s++) {
			if (bytecounts[s] > bufsize) {
				buf = (unsigned char *)_TIFFrealloc(buf, bytecounts[s]);
				if (!buf)
					return (0);
				bufsize = bytecounts[s];
			}
			if (TIFFReadRawStrip(in, s, buf, bytecounts[s]) < 0 ||
			    TIFFWriteRawStrip(out, s, buf, bytecounts[s]) < 0) {
				_TIFFfree(buf);
				return (0);
			}
		}
		_TIFFfree(buf);
		return (1);
	}
	return (0);
}

static int
cpTiles(TIFF* in, TIFF* out)
{
	tsize_t bufsize = TIFFTileSize(in);
	unsigned char *buf = (unsigned char *)_TIFFmalloc(bufsize);

	if (buf) {
		ttile_t t, nt = TIFFNumberOfTiles(in);
		uint32 *bytecounts;

		TIFFGetField(in, TIFFTAG_TILEBYTECOUNTS, &bytecounts);
		for (t = 0; t < nt; t++) {
			if (bytecounts[t] > bufsize) {
				buf = (unsigned char *)_TIFFrealloc(buf, bytecounts[t]);
				if (!buf)
					return (0);
				bufsize = bytecounts[t];
			}
			if (TIFFReadRawTile(in, t, buf, bytecounts[t]) < 0 ||
			    TIFFWriteRawTile(out, t, buf, bytecounts[t]) < 0) {
				_TIFFfree(buf);
				return (0);
			}
		}
		_TIFFfree(buf);
		return (1);
	}
	return (0);
}
