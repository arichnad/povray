/*******************************************************************************
 * types.h
 *
 * This file contains ... TODO ...
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
 * $File: //depot/povray/smp/source/base/safemath.h $
 * $Revision: #2 $
 * $Change: 5784 $
 * $DateTime: 2013/02/04 13:06:24 $
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

#ifndef POVRAY_BASE_SAFEMATH_H
#define POVRAY_BASE_SAFEMATH_H

#include <cassert>
#include <limits>

#include "base/configbase.h"
#include "base/pov_err.h"

namespace pov_base
{

/// Multiply four (unsigned integer) factors, throwing an exception in case of numerical overflow.
template<typename T, typename T1, typename T2, typename T3, typename T4>
static inline T SafeUnsignedProduct(T1 p1, T2 p2, T3 p3, T4 p4)
{
	// the function is intended for use with unisgned integer parameters only
	// (NB: Instead of testing for (pN >= 0) we could also test for (!numeric_limits<TN>::is_signed),
	//  but this would make passing constant factors more cumbersome)
	assert (numeric_limits<T>::is_integer);
	assert (numeric_limits<T1>::is_integer && (p1 >= 0));
	assert (numeric_limits<T2>::is_integer && (p2 >= 0));
	assert (numeric_limits<T3>::is_integer && (p3 >= 0));
	assert (numeric_limits<T4>::is_integer && (p4 >= 0));

	// avoid divide-by-zero issues
	if ((p1==0) || (p2==0) || (p3==0) || (p4==0))
		return 0;

	if ( (((numeric_limits<T>::max() / p4) / p3) / p2) < p1 )
		throw POV_EXCEPTION_CODE(kNumericalLimitErr);

	return T(p1) * T(p2) * T(p3) * T(p4);
}

/// Multiply three (unsigned integer) factors, throwing an exception in case of numerical overflow.
template<typename T, typename T1, typename T2, typename T3>
static inline T SafeUnsignedProduct(T1 p1, T2 p2, T3 p3)
{
	return SafeUnsignedProduct<T,T1,T2,T3,unsigned int>(p1, p2, p3, 1u);
}

/// Multiply two (unsigned integer) factors, throwing an exception in case of numerical overflow.
template<typename T, typename T1, typename T2>
static inline T SafeUnsignedProduct(T1 p1, T2 p2)
{
	return SafeUnsignedProduct<T,T1,T2,unsigned int,unsigned int>(p1, p2, 1u, 1u);
}

#if 0 // not currently used, but I hesitate to throw it away [CLi]

/// Multiply up to four (signed integer) values, throwing an exception in case of numerical overflow.
/// @note: The function will also throw an exception if negating the result would overflow.
template<typename T>
static inline T SafeSignedProduct(T p1, T p2, T p3 = 1, T p4 = 1)
{
	// the function is intended for use with signed integer types only
	assert (numeric_limits<T>::is_integer);
	assert (numeric_limits<T>::is_signed);

	// avoid divide-by-zero issues
	if ((p1==0) || (p2==0) || (p3==0) || (p4==0))
		return 0;

	if (numeric_limits<T>::min() + numeric_limits<T>::max() == 0)
	{
		// integer representation appears to be sign-and-magnitude or one's complement; at any rate,
		// abs(pN) is guaranteed to be a safe operation, and so is x/abs(pN) (as we've made sure that
		// pN are all nonzero), and |::min()|==|::max()| is also guaranteed, i.e. the limits in the positive
		// and negative domain are equally stringent.
		if ( (((numeric_limits<T>::max() / abs(p4)) / abs(p3)) / abs(p2)) < abs(p1) )
			throw POV_EXCEPTION_CODE(kNumericalLimitErr);
	}
	else if (numeric_limits<T>::min() + numeric_limits<T>::max() < 0)
	{
		// integer representation appears to be two's complement; at any rate, abs(pN) is a potentially
		// unsafe operation, while -x is a safe operation for positive x; |::max()| > |::min()| is guaranteed,
		// i.e. the limits in the positive domain are more stringent than those in the negative one.

		// specifically handle situations in which abs(pN) would overflow
		// NB we're deliberately not testing for pN == numeric_limits<T>::min(), in order to make the test robust
		// against exotic integer representations
		if ((p1 < -numeric_limits<T>::max()) ||
			(p2 < -numeric_limits<T>::max()) ||
			(p3 < -numeric_limits<T>::max()) ||
			(p4 < -numeric_limits<T>::max()))
			throw POV_EXCEPTION_CODE(kNumericalLimitErr);

		// we've made sure that abs(pN) is a safe operation, and hence also x/abs(pN) (as we've also made sure that
		// all pN are nonzero); we also know that whatever is safe in the positive domain is also safe in the
		// negative domain
		if ( (((numeric_limits<T>::max() / abs(p4)) / abs(p3)) / abs(p2)) < abs(p1) )
			throw POV_EXCEPTION_CODE(kNumericalLimitErr);
	}
	else
	{
		// integer representation is exotic; abs(pN) is guaranteed to be a safe operation, and |::min()| > |::max()|
		// is guaranteed, i.e. the limits in the negative domain are more stringent than those in the positive one.

		// with abs(pN) a safe operation and having made sure all pN are non-zero, x/abs(pN) is guaranteed to be a safe
		// operation as well; we also know that whatever is safe in the negative domain is also safe in the
		// positive domain
		if ( (((abs(numeric_limits<T>::min()) / abs(p4)) / abs(p3)) / abs(p2)) < abs(p1) )
			throw POV_EXCEPTION_CODE(kNumericalLimitErr);
	}

	// product is safe, go ahead
	return p1 * p2 * p3 * p4;
}

#endif

}

#endif // POVRAY_BASE_SAFEMATH_H
