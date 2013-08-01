/*******************************************************************************
 * processoptions.h
 *
 * This module contains all defines, typedefs, and prototypes for the
 * C++ interface of processoptions.cpp.
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
 * $File: //depot/povray/smp/source/base/processoptions.h $
 * $Revision: #18 $
 * $Change: 5770 $
 * $DateTime: 2013/01/30 13:07:27 $
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

#ifndef PROCESSOPTIONS_H
#define PROCESSOPTIONS_H

#include "configbase.h"

#include "povms.h"
#include "textstream.h"

namespace pov_base
{

class ProcessOptions
{
	public:
		struct INI_Parser_Table
		{
			const char *keyword;
			POVMSType key;
			POVMSType type;
		};

		struct Cmd_Parser_Table
		{
			const char *command;
			POVMSType key;
			POVMSType type;
			POVMSType is_switch;
		};

		struct Output_FileType_Table
		{
			char        code;               // code used in INI and command line options
			POVMSType   attribute;          // attribute for which the entry is valid (0 = all)
			int         internalId;         // kPOVList_FileType_*
			bool        has16BitGrayscale;  // grayscale output at >> 8bpp is supported for this file type
			bool        hasAlpha;           // alpha channel output is officially(!) supported for this file type
		};

		struct Parameter_Code_Table
		{
			const char* code;               // code used in INI and command line options
			int         internalId;         // e.g. kPOVList_GammaType_*
		};

		ProcessOptions(INI_Parser_Table *, Cmd_Parser_Table *);
		~ProcessOptions();

		int ParseFile(const char *, POVMSObjectPtr);
		int ParseString(const char *, POVMSObjectPtr, bool singleswitch = false);

		int WriteFile(const char *, POVMSObjectPtr);
		int WriteFile(OTextStream *, POVMSObjectPtr);

		static bool Matches(const char *, const char *);
		static bool IsTrue(const char *);
		static bool IsFalse(const char *);

		static int POVMSAttr_GetUTF8String(POVMSAttributePtr, POVMSType, char *, int *);
		static int POVMSAttr_SetUTF8String(POVMSAttributePtr, POVMSType, const char *);
		static int POVMSUtil_SetUTF8String(POVMSObjectPtr, POVMSType, const char *);
		static size_t ConvertUTF8ToUCS2(const char *, UCS2 *);
		static size_t ConvertUCS2ToUTF8(const UCS2 *, char *);
	protected:
		virtual int ReadSpecialOptionHandler(INI_Parser_Table *, char *, POVMSObjectPtr);
		virtual int ReadSpecialSwitchHandler(Cmd_Parser_Table *, char *, POVMSObjectPtr, bool);
		virtual int WriteSpecialOptionHandler(INI_Parser_Table *, POVMSObjectPtr, OTextStream *);
		virtual bool WriteOptionFilter(INI_Parser_Table *);
		virtual bool ProcessUnknownSwitch(char *, char *, POVMSObjectPtr);
		virtual int ProcessUnknownString(char *, POVMSObjectPtr);

		virtual ITextStream *OpenFileForRead(const char *, POVMSObjectPtr) = 0;
		virtual OTextStream *OpenFileForWrite(const char *, POVMSObjectPtr) = 0;

		virtual void ParseError(const char *, ...);
		virtual void ParseErrorAt(ITextStream *, const char *, ...);
		virtual void WriteError(const char *, ...);
	private:
		INI_Parser_Table *parse_ini_table;
		Cmd_Parser_Table *parse_cmd_table;

		int Output_INI_Option(INI_Parser_Table *, POVMSObjectPtr, OTextStream *);

		int Parse_INI_Specification(const char *, char *&, char *&);
		int Parse_INI_Skip_Space(ITextStream *, bool);
		int Parse_INI_Skip_Line(ITextStream *);
		int Parse_INI_Option(ITextStream *, POVMSObjectPtr);
		int Parse_INI_Switch(ITextStream *, int, POVMSObjectPtr);
		char *Parse_INI_String(ITextStream *, int endchr = -1, bool smartmode = false);
		bool Parse_INI_String_Smartmode(ITextStream *);
		char *Parse_Raw_INI_String(ITextStream *file);

		int Parse_CL(char *, POVMSObjectPtr, bool);
		void Parse_CL_Skip_Space(const char *&);
		int Parse_CL_Switch(const char *&, int , POVMSObjectPtr, bool);
		int Parse_CL_Option(const char *&, POVMSObjectPtr, bool);
		char *Parse_CL_String(const char *&, int endchr = -1);

		int Process_INI_Option(INI_Parser_Table *, char *, POVMSObjectPtr);
		int Process_Switch(Cmd_Parser_Table *, char *, POVMSObjectPtr, bool);
};

}

#endif
