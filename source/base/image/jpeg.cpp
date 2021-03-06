/*******************************************************************************
 * jpeg.cpp
 *
 * from Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
 * Copyright 1991-2003 Persistence of Vision Team
 * Copyright 2003-2009 Persistence of Vision Raytracer Pty. Ltd.
 * ---------------------------------------------------------------------------
 * NOTICE: This source code file is provided so that users may experiment
 * with enhancements to POV-Ray and to port the software to platforms other
 * than those supported by the POV-Ray developers. There are strict rules
 * regarding how you are permitted to use this file. These rules are contained
 * in the distribution and derivative versions licenses which should have been
 * provided with this file.
 *
 * These licences may be found online, linked from the end-user license
 * agreement that is located at http://www.povray.org/povlegal.html
 * ---------------------------------------------------------------------------
 * POV-Ray is based on the popular DKB raytracer version 2.12.
 * DKBTrace was originally written by David K. Buck.
 * DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
 * ---------------------------------------------------------------------------
 * $File: //depot/povray/smp/source/base/image/jpeg.cpp $
 * $Revision: #29 $
 * $Change: 5625 $
 * $DateTime: 2012/03/10 21:41:16 $
 * $Author: clipka $
 *******************************************************************************/

/*********************************************************************************
 * NOTICE
 *
 * This file is part of a BETA-TEST version of POV-Ray version 3.7. It is not
 * final code. Use of this source file is governed by both the standard POV-Ray
 * licences referred to in the copyright header block above this notice, and the
 * following additional restrictions numbered 1 through 4 below:
 *
 *   1. This source file may not be re-distributed without the written permission
 *      of Persistence of Vision Raytracer Pty. Ltd.
 *
 *   2. This notice may not be altered or removed.
 *   
 *   3. Binaries generated from this source file by individuals for their own
 *      personal use may not be re-distributed without the written permission
 *      of Persistence of Vision Raytracer Pty. Ltd. Such personal-use binaries
 *      are not required to have a timeout, and thus permission is granted in
 *      these circumstances only to disable the timeout code contained within
 *      the beta software.
 *   
 *   4. Binaries generated from this source file for use within an organizational
 *      unit (such as, but not limited to, a company or university) may not be
 *      distributed beyond the local organizational unit in which they were made,
 *      unless written permission is obtained from Persistence of Vision Raytracer
 *      Pty. Ltd. Additionally, the timeout code implemented within the beta may
 *      not be disabled or otherwise bypassed in any manner.
 *
 * The following text is not part of the above conditions and is provided for
 * informational purposes only.
 *
 * The purpose of the no-redistribution clause is to attempt to keep the
 * circulating copies of the beta source fresh. The only authorized distribution
 * point for the source code is the POV-Ray website and Perforce server, where
 * the code will be kept up to date with recent fixes. Additionally the beta
 * timeout code mentioned above has been a standard part of POV-Ray betas since
 * version 1.0, and is intended to reduce bug reports from old betas as well as
 * keep any circulating beta binaries relatively fresh.
 *
 * All said, however, the POV-Ray developers are open to any reasonable request
 * for variations to the above conditions and will consider them on a case-by-case
 * basis.
 *
 * Additionally, the developers request your co-operation in fixing bugs and
 * generally improving the program. If submitting a bug-fix, please ensure that
 * you quote the revision number of the file shown above in the copyright header
 * (see the '$Revision:' field). This ensures that it is possible to determine
 * what specific copy of the file you are working with. The developers also would
 * like to make it known that until POV-Ray 3.7 is out of beta, they would prefer
 * to emphasize the provision of bug fixes over the addition of new features.
 *
 * Persons wishing to enhance this source are requested to take the above into
 * account. It is also strongly suggested that such enhancements are started with
 * a recent copy of the source.
 *
 * The source code page (see http://www.povray.org/beta/source/) sets out the
 * conditions under which the developers are willing to accept contributions back
 * into the primary source tree. Please refer to those conditions prior to making
 * any changes to this source, if you wish to submit those changes for inclusion
 * with POV-Ray.
 *
 *********************************************************************************/

/*****************************************************************************
*
*  This code requires the use of jpeg-6b, The Independent JPEG Group's JPEG
*  software. The complete legal section below is taken from README in the
*  jpeg-6b directory, note that not all issues listed there may apply to this
*  distribution. For complete details read the README in the jpeg-6b directory.
*
******************************************************************************

LEGAL ISSUES
============

In plain English:

1. We don't promise that this software works.  (But if you find any bugs,
   please let us know!)
2. You can use this software for whatever you want.  You don't have to pay us.
3. You may not pretend that you wrote this software.  If you use it in a
   program, you must acknowledge somewhere in your documentation that
   you've used the IJG code.

In legalese:

The authors make NO WARRANTY or representation, either express or implied,
with respect to this software, its quality, accuracy, merchantability, or
fitness for a particular purpose.  This software is provided "AS IS", and you,
its user, assume the entire risk as to its quality and accuracy.

This software is copyright (C) 1991-1998, Thomas G. Lane.
All Rights Reserved except as specified below.

Permission is hereby granted to use, copy, modify, and distribute this
software (or portions thereof) for any purpose, without fee, subject to these
conditions:
(1) If any part of the source code for this software is distributed, then this
README file must be included, with this copyright and no-warranty notice
unaltered; and any additions, deletions, or changes to the original files
must be clearly indicated in accompanying documentation.
(2) If only executable code is distributed, then the accompanying
documentation must state that "this software is based in part on the work of
the Independent JPEG Group".
(3) Permission for use of this software is granted only if the user accepts
full responsibility for any undesirable consequences; the authors accept
NO LIABILITY for damages of any kind.

These conditions apply to any software derived from or based on the IJG code,
not just to the unmodified library.  If you use our work, you ought to
acknowledge us.

Permission is NOT granted for the use of any IJG author's name or company name
in advertising or publicity relating to this software or products derived from
it.  This software may be referred to only as "the Independent JPEG Group's
software".

We specifically permit and encourage the use of this software as the basis of
commercial products, provided that all warranty or liability claims are
assumed by the product vendor.


ansi2knr.c is included in this distribution by permission of L. Peter Deutsch,
sole proprietor of its copyright holder, Aladdin Enterprises of Menlo Park, CA.
ansi2knr.c is NOT covered by the above copyright and conditions, but instead
by the usual distribution terms of the Free Software Foundation; principally,
that you must include source code if you redistribute it.  (See the file
ansi2knr.c for full details.)  However, since ansi2knr.c is not needed as part
of any program generated from the IJG code, this does not limit you more than
the foregoing paragraphs do.

The Unix configuration script "configure" was produced with GNU Autoconf.
It is copyright by the Free Software Foundation but is freely distributable.
The same holds for its supporting scripts (config.guess, config.sub,
ltconfig, ltmain.sh).  Another support script, install-sh, is copyright
by M.I.T. but is also freely distributable.

It appears that the arithmetic coding option of the JPEG spec is covered by
patents owned by IBM, AT&T, and Mitsubishi.  Hence arithmetic coding cannot
legally be used without obtaining one or more licenses.  For this reason,
support for arithmetic coding has been removed from the free JPEG software.
(Since arithmetic coding provides only a marginal gain over the unpatented
Huffman mode, it is unlikely that very many implementations will support it.)
So far as we are aware, there are no patent restrictions on the remaining
code.

The IJG distribution formerly included code to read and write GIF files.
To avoid entanglement with the Unisys LZW patent, GIF reading support has
been removed altogether, and the GIF writer has been simplified to produce
"uncompressed GIFs".  This technique does not use the LZW algorithm; the
resulting GIF files are larger than usual, but are readable by all standard
GIF decoders.

We are required to state that
    "The Graphics Interchange Format(c) is the Copyright property of
    CompuServe Incorporated.  GIF(sm) is a Service Mark property of
    CompuServe Incorporated."

*****************************************************************************/

// NOTE: this file throws exceptions from 'extern C' functions. be sure to enable
// this in your compiler options (preferably for this file only).

#include <setjmp.h>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/image/image.h"
#include "base/image/jpeg_pov.h"

#include <boost/scoped_array.hpp>
#include <string>

#ifndef LIBJPEG_MISSING

extern "C"
{
	#include "jpeglib.h"
}

#include "metadata.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

namespace Jpeg
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

const int POV_JPEG_BUFFER_SIZE = 1024;

/*****************************************************************************
* Local typedefs
******************************************************************************/

// write buffer for JPEG images
class POV_JPEG_Write_Buffer
{
public:
	POV_JPEG_Write_Buffer();

	struct jpeg_error_mgr jerr;
	jpeg_source_mgr jsrc;
	jpeg_destination_mgr jdest;
	jmp_buf setjmp_buffer;  // for return to caller
	char buffer[POV_JPEG_BUFFER_SIZE];
	JSAMPROW row_pointer[1];
	int row_stride;
	struct jpeg_compress_struct cinfo;
	string msg;
	OStream *file;
} ;

POV_JPEG_Write_Buffer::POV_JPEG_Write_Buffer()
{
	memset(&jerr, 0, sizeof(jerr));
	memset(&jsrc, 0, sizeof(jsrc));
	memset(&jdest, 0, sizeof(jdest));
	memset(&cinfo, 0, sizeof(cinfo));
	row_pointer[0] = NULL;
	file = NULL;
	row_stride = 0;
}

class POV_JPEG_Read_Buffer
{
public:
	POV_JPEG_Read_Buffer();

	struct jpeg_error_mgr jerr;
	jpeg_source_mgr jsrc;
	jpeg_destination_mgr jdest;
	jmp_buf setjmp_buffer;  // for return to caller
	char buffer[POV_JPEG_BUFFER_SIZE];
	JSAMPROW row_pointer[1];
	int row_stride;
	struct jpeg_decompress_struct cinfo;
	string msg;
	IStream *file;

};

POV_JPEG_Read_Buffer::POV_JPEG_Read_Buffer()
{
	memset(&jerr, 0, sizeof(jerr));
	memset(&jsrc, 0, sizeof(jsrc));
	memset(&jdest, 0, sizeof(jdest));
	memset(&cinfo, 0, sizeof(cinfo));
	row_pointer[0] = NULL;
	file = NULL;
	row_stride = 0;
}

extern "C"
{
	METHODDEF(void) read_error_exit(j_common_ptr cinfo);
	METHODDEF(void) read_output_message(j_common_ptr cinfo);
	METHODDEF(void) write_error_exit(j_common_ptr cinfo);
	METHODDEF(void) write_output_message(j_common_ptr cinfo);
	METHODDEF(void) write_init_dest(j_compress_ptr cinfo);
	METHODDEF(void) read_init_source(j_decompress_ptr cinfo);
	METHODDEF(boolean) read_fill_input_buffer(j_decompress_ptr cinfo);
	METHODDEF(void) read_skip_input_data(j_decompress_ptr cinfo, long num_bytes);
	METHODDEF(void) read_term_source(j_decompress_ptr cinfo);
	METHODDEF(boolean) write_empty_output_buffer(j_compress_ptr cinfo);
	METHODDEF(void) write_term_destination(j_compress_ptr cinfo);

	METHODDEF(void) read_error_exit (j_common_ptr cinfo)
	{
		POV_JPEG_Read_Buffer * myerr = reinterpret_cast<POV_JPEG_Read_Buffer *>(cinfo->client_data);

		(*cinfo->err->output_message)(cinfo);

		// Return control to the setjmp point
		longjmp(myerr->setjmp_buffer, 1);
	}

	METHODDEF(void) write_error_exit (j_common_ptr cinfo)
	{
		POV_JPEG_Write_Buffer * myerr = reinterpret_cast<POV_JPEG_Write_Buffer *>(cinfo->client_data);

		(*cinfo->err->output_message)(cinfo);

		// Return control to the setjmp point
		longjmp(myerr->setjmp_buffer, 1);
	}

	METHODDEF(void) read_output_message(j_common_ptr cinfo)
	{
		char buffer[JMSG_LENGTH_MAX];
		POV_JPEG_Read_Buffer *bufptr = reinterpret_cast<POV_JPEG_Read_Buffer *>(cinfo->client_data);

		// Create the message
		(*cinfo->err->format_message) (cinfo, buffer);

		// store it for later display
		bufptr->msg = buffer;
	}

	METHODDEF(void) write_output_message(j_common_ptr cinfo)
	{
		char buffer[JMSG_LENGTH_MAX];
		POV_JPEG_Write_Buffer *bufptr = reinterpret_cast<POV_JPEG_Write_Buffer *>(cinfo->client_data);

		// Create the message
		(*cinfo->err->format_message) (cinfo, buffer);

		// store it for later display
		bufptr->msg = buffer;
	}

	METHODDEF(void) write_init_dest(j_compress_ptr cinfo)
	{
		POV_JPEG_Write_Buffer * bufptr = reinterpret_cast<POV_JPEG_Write_Buffer *>(cinfo->client_data);

		bufptr->jdest.next_output_byte = reinterpret_cast<unsigned char *>(&(bufptr->buffer[0]));
		bufptr->jdest.free_in_buffer = POV_JPEG_BUFFER_SIZE;
	}

	METHODDEF(void) read_init_source(j_decompress_ptr cinfo)
	{
		POV_JPEG_Read_Buffer * bufptr = reinterpret_cast<POV_JPEG_Read_Buffer *>(cinfo->client_data);

		bufptr->jsrc.next_input_byte = reinterpret_cast<unsigned char *>(&(bufptr->buffer[0]));
		bufptr->jsrc.bytes_in_buffer = 0;
	}

	METHODDEF(boolean) read_fill_input_buffer(j_decompress_ptr cinfo)
	{
		POV_JPEG_Read_Buffer * bufptr = reinterpret_cast<POV_JPEG_Read_Buffer *>(cinfo->client_data);
		int i;

		for(i = 0; i < POV_JPEG_BUFFER_SIZE; i++)
		{
			if (!bufptr->file->read(&(bufptr->buffer[i]), 1))
				break;
		}
		bufptr->jsrc.bytes_in_buffer = i;
		bufptr->jsrc.next_input_byte = reinterpret_cast<unsigned char *>(&(bufptr->buffer[0]));

		return true;
	}

	METHODDEF(void) read_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
	{
		POV_JPEG_Read_Buffer *bufptr = reinterpret_cast<POV_JPEG_Read_Buffer *>(cinfo->client_data);

		if ( bufptr->jsrc.bytes_in_buffer < num_bytes )
		{
			num_bytes -= (long) bufptr->jsrc.bytes_in_buffer;
			bufptr->jsrc.bytes_in_buffer = 0;
			bufptr->file->seekg(num_bytes, POV_SEEK_CUR);
		}
		else
		{
			bufptr->jsrc.bytes_in_buffer -= num_bytes;
			bufptr->jsrc.next_input_byte += num_bytes;
		}

	}

	METHODDEF(void) read_term_source(j_decompress_ptr /*cinfo*/)
	{
	}

	METHODDEF(boolean) write_empty_output_buffer(j_compress_ptr cinfo)
	{
		POV_JPEG_Write_Buffer * bufptr = reinterpret_cast<POV_JPEG_Write_Buffer *>(cinfo->client_data);

		// throw an exception on failure rather than return false; this gives more
		// helpful error reporting
		if (!bufptr->file->write(bufptr->buffer, POV_JPEG_BUFFER_SIZE))
			throw POV_EXCEPTION(kFileDataErr, "Failed to write JPEG data to disk");
		bufptr->jdest.next_output_byte = reinterpret_cast<unsigned char *>(&(bufptr->buffer[0]));
		bufptr->jdest.free_in_buffer = POV_JPEG_BUFFER_SIZE;
		return true;
	}

	METHODDEF(void) write_term_destination(j_compress_ptr cinfo)
	{
		POV_JPEG_Write_Buffer * bufptr = reinterpret_cast<POV_JPEG_Write_Buffer *>(cinfo->client_data);

		if(POV_JPEG_BUFFER_SIZE - bufptr->jdest.free_in_buffer > 0)
			if (!bufptr->file->write(bufptr->buffer, POV_JPEG_BUFFER_SIZE - bufptr->jdest.free_in_buffer))
				throw POV_EXCEPTION(kFileDataErr, "Failed to write final JPEG data block to disk");
	}
}

// TODO: handle possible memory leakage if an exception is thrown during a read
Image *Read (IStream *file, const Image::ReadOptions& options)
{
	int                             width;
	int                             height;
	int                             row;
	int                             col;
	Image                           *image;
	POV_JPEG_Read_Buffer            readbuf;

	// We set up the normal JPEG error routines, then override error_exit and output_message.
	readbuf.cinfo.err = jpeg_std_error(&readbuf.jerr);
	readbuf.jerr.error_exit = read_error_exit;
	readbuf.jerr.output_message = read_output_message;
	readbuf.cinfo.client_data = reinterpret_cast<void *>(&readbuf);

	if (setjmp(readbuf.setjmp_buffer))
	{
		jpeg_destroy_decompress(&readbuf.cinfo);
		if (readbuf.msg.length() > 0)
			throw POV_EXCEPTION(kFileDataErr, readbuf.msg.c_str());
		throw POV_EXCEPTION(kFileDataErr, "Cannot read JPEG image");
	}

	// Now we can initialize the JPEG decompression object.
	jpeg_create_decompress(&readbuf.cinfo);

	readbuf.file = file;
	readbuf.jsrc.init_source = read_init_source;
	readbuf.jsrc.fill_input_buffer = read_fill_input_buffer;
	readbuf.jsrc.skip_input_data = read_skip_input_data;
	readbuf.jsrc.resync_to_restart = jpeg_resync_to_restart;
	readbuf.jsrc.term_source = read_term_source;
	readbuf.cinfo.src = &readbuf.jsrc;

	/* We can ignore the return value from jpeg_read_header since
	 *   (a) suspension is not possible with the stdio data source, and
	 *   (b) we passed true to reject a tables-only JPEG file as an error.
	 * See libjpeg.doc for more info.
	 */
	(void)jpeg_read_header(&readbuf.cinfo, true);

	// check for unsupported formats
	if((readbuf.cinfo.output_components != 1) && (readbuf.cinfo.output_components != 3) &&
	   (readbuf.cinfo.out_color_space != JCS_GRAYSCALE) && (readbuf.cinfo.out_color_space != JCS_RGB))
	{
		jpeg_destroy_decompress(&readbuf.cinfo);
		throw POV_EXCEPTION(kFileDataErr, "Unsupported color format in JPEG image");
	}

	// TODO FIXME - in order to preserve image quality, there should be a way to perform the conversion from YCbCr to RGB
	// including gamma correction in the floating-point domain, or at least in a higher-resolution integer domain.

	// JPEG files used to have no clearly defined gamma by default, but a W3C recommendation exists for them to use sRGB
	// unless an ICC profile is present (which we presently don't support).
	GammaCurvePtr gamma;
	if (options.gammacorrect)
	{
		if (options.defaultGamma)
			gamma = TranscodingGammaCurve::Get(options.workingGamma, options.defaultGamma);
		else
			gamma = TranscodingGammaCurve::Get(options.workingGamma, SRGBGammaCurve::Get());
	}

	// begin decompression
	(void)jpeg_start_decompress(&readbuf.cinfo);

	height = readbuf.cinfo.output_height;
	width = readbuf.cinfo.output_width;

	Image::ImageDataType imagetype = options.itype;
	if (imagetype == Image::Undefined)
	{
		if (GammaCurve::IsNeutral(gamma))
			// No gamma correction required, raw values can be stored "as is".
			imagetype = readbuf.cinfo.output_components == 1 ? Image::Gray_Int8 : Image::RGB_Int8;
		else
			// Gamma correction required; use an image container that will take care of that.
			imagetype = readbuf.cinfo.output_components == 1 ? Image::Gray_Gamma8 : Image::RGB_Gamma8;
	}
	image = Image::Create (width, height, imagetype) ;
	// NB: JPEG files don't use alpha, so premultiplied vs. non-premultiplied is not an issue
	image->TryDeferDecoding(gamma, 255); // try to have gamma adjustment being deferred until image evaluation.

	// JSAMPLEs per row in output buffer
	readbuf.row_stride = readbuf.cinfo.output_width * readbuf.cinfo.output_components;

	// Make a one-row-high sample array
	boost::scoped_array<JSAMPLE> scopedarray (new JSAMPLE [readbuf.row_stride]);
	readbuf.row_pointer[0] = (JSAMPROW) &scopedarray[0] ;

	// read image row by row
	for (row = 0; row < height; row++)
	{
		// read scanline
		(void)jpeg_read_scanlines(&readbuf.cinfo, &readbuf.row_pointer[0], 1);

		if(readbuf.cinfo.output_components == 3) // 24-bit rgb image
		{
			for (col = 0; col < width; col++)
			{
				unsigned int r = readbuf.row_pointer[0][col * 3];
				unsigned int g = readbuf.row_pointer[0][(col * 3) + 1];
				unsigned int b = readbuf.row_pointer[0][(col * 3) + 2];
				SetEncodedRGBValue (image, col, row, gamma, 255, r, g, b) ;
			}
		}
		else if (readbuf.cinfo.output_components == 1) // 8-bit grayscale image
		{
			for (col = 0; col < width; col++)
			{
				SetEncodedGrayValue (image, col, row, gamma, 255, (unsigned int) readbuf.row_pointer[0][col]) ;
			}
		}
	}

	// finish decompression
	(void)jpeg_finish_decompress(&readbuf.cinfo);

	// release JPEG decompression object
	jpeg_destroy_decompress(&readbuf.cinfo);

	return (image);
}

// TODO: handle possible memory leakage if an exception is thrown during a write
void Write (OStream *file, const Image *image, const Image::WriteOptions& options)
{
	int                         width = image->GetWidth() ;
	int                         height = image->GetHeight() ;
	int                         quality = options.compress;
	int                         grayscale = image->IsGrayscale();
	POV_JPEG_Write_Buffer       writebuf;
	GammaCurvePtr               gamma;
	DitherHandlerPtr            dither = GetNoOpDitherHandler(); // dithering doesn't make much sense with JPEG

	if (options.encodingGamma)
		gamma = TranscodingGammaCurve::Get(options.workingGamma, options.encodingGamma);
	else
		// JPEG files used to have no clearly defined gamma by default, but a W3C recommendation exists for them to use sRGB
		// unless an ICC profile is present (which we presently don't support).
		gamma = TranscodingGammaCurve::Get(options.workingGamma, SRGBGammaCurve::Get());

	writebuf.file = file;

	// We set up the normal JPEG error routines, then override error_exit and output_message.
	writebuf.cinfo.err = jpeg_std_error(&writebuf.jerr);
	writebuf.jerr.error_exit = write_error_exit;
	writebuf.jerr.output_message = write_output_message;
	writebuf.cinfo.client_data = reinterpret_cast<void *>(&writebuf);

	if (setjmp(writebuf.setjmp_buffer))
	{
		jpeg_destroy_compress(&writebuf.cinfo);
		if (writebuf.msg.length() > 0)
			throw POV_EXCEPTION(kFileDataErr, writebuf.msg.c_str());
		throw POV_EXCEPTION(kFileDataErr, "Cannot write JPEG image");
	}

	jpeg_create_compress(&writebuf.cinfo);
	writebuf.jdest.init_destination = write_init_dest;
	writebuf.jdest.empty_output_buffer = write_empty_output_buffer;
	writebuf.jdest.term_destination = write_term_destination;
	writebuf.cinfo.dest = &writebuf.jdest;
	writebuf.cinfo.image_width = width;
	writebuf.cinfo.image_height = height;

	if (grayscale)
	{
		writebuf.cinfo.in_color_space = JCS_GRAYSCALE;
		writebuf.cinfo.input_components = 1;
		jpeg_set_defaults(&writebuf.cinfo);
		jpeg_set_colorspace(&writebuf.cinfo, JCS_GRAYSCALE);
	}
	else
	{
		writebuf.cinfo.in_color_space = JCS_RGB; // colour model of data we're feeding to the jpeglib
		writebuf.cinfo.input_components = 3;
		jpeg_set_defaults(&writebuf.cinfo);
		jpeg_set_colorspace(&writebuf.cinfo, JCS_YCbCr); // colour model to use in the output file; must be YCbCr for compliance with JFIF standard
	}

	writebuf.row_stride = writebuf.cinfo.image_width * writebuf.cinfo.input_components;

	// Make a one-row-high sample array
	boost::scoped_array<JSAMPLE> scopedarray (new JSAMPLE [writebuf.row_stride]);
	writebuf.row_pointer[0] = (JSAMPROW) &scopedarray[0] ;

	// if quality is not specified, we wind the output quality waaaay up (usually needed for raytracing)
	// (This used to be 95% when we still did chroma sub-sampling; without chroma sub-sampling,
	// about the same visual quality and file size is achieved at 85%. - [CLi])
	if (quality == 0)
		quality = 85;
	else if (quality == 1) // this is to catch cases where users use '1' to turn on compression
		quality = 85;
	jpeg_set_quality(&writebuf.cinfo, quality, true); // quality (range 2 to 100)

	if (!grayscale)
	{
		// inhibit chroma sub-sampling to better preserve small colour details
		writebuf.cinfo.comp_info[0].h_samp_factor = 1;
		writebuf.cinfo.comp_info[0].v_samp_factor = 1;
	}

	// begin compression
	jpeg_start_compress(&writebuf.cinfo, true);

	// prepare metadata
	Metadata meta;
	/* Line feed is "local" */
	string comment=string("Render Date: ") + meta.getDateTime() + "\n";
	comment += "Software: " + meta.getSoftware() + "\n";
	if (!meta.getComment1().empty())
		comment += meta.getComment1() + "\n";
	if (!meta.getComment2().empty())
		comment += meta.getComment2() + "\n";
	if (!meta.getComment3().empty())
		comment += meta.getComment3() + "\n";
	if (!meta.getComment4().empty())
		comment += meta.getComment4() + "\n";

	const JOCTET *pcom(reinterpret_cast<const JOCTET*>(&comment[0])); /* lack of converter, cast, old-style (bad, go mixing C & C++!) */
	
	// The comment marker must be here, before the image data 
	jpeg_write_marker(&writebuf.cinfo, JPEG_COM, pcom,comment.length());

	// write image row by row
	for (int row = 0; row < height; row++)
	{
		JSAMPLE *sample = writebuf.row_pointer[0];
		if (writebuf.cinfo.input_components == 3) // 24-bit rgb image
		{
			for (int col = 0; col < width; col++)
			{
				unsigned int r, g, b;
				GetEncodedRGBValue(image, col, row, gamma, 255, r, g, b, *dither);
				*sample++ = (JSAMPLE) r;
				*sample++ = (JSAMPLE) g;
				*sample++ = (JSAMPLE) b;
			}
		}
		else if (writebuf.cinfo.input_components == 1) // 8-bit grayscale image
		{
			for (int col = 0; col < width; col++)
				*sample++ = (JSAMPLE) GetEncodedGrayValue(image, col, row, gamma, 255, *dither);
		}
		jpeg_write_scanlines(&writebuf.cinfo, writebuf.row_pointer, 1);
	}

	jpeg_finish_compress(&writebuf.cinfo);
	jpeg_destroy_compress(&writebuf.cinfo);
}

} // end of namespace Jpeg

}

#endif  // LIBJPEG_MISSING
