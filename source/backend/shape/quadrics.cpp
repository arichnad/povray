/*******************************************************************************
 * quadrics.cpp
 *
 * This module implements the code for the quadric shape primitive.
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
 * $File: //depot/povray/smp/source/backend/shape/quadrics.cpp $
 * $Revision: #32 $
 * $Change: 5103 $
 * $DateTime: 2010/08/22 06:58:49 $
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

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/math/vector.h"
#include "backend/bounding/bbox.h"
#include "backend/scene/objects.h"
#include "backend/math/matrices.h"
#include "backend/shape/planes.h"
#include "backend/shape/quadrics.h"
#include "backend/scene/threaddata.h"

#include <algorithm>

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

/* The following defines make typing much easier! [DB 7/94] */

#define Xd ray.Direction[X]
#define Yd ray.Direction[Y]
#define Zd ray.Direction[Z]

#define Xo ray.Origin[X]
#define Yo ray.Origin[Y]
#define Zo ray.Origin[Z]

#define QA Square_Terms[X]
#define QE Square_Terms[Y]
#define QH Square_Terms[Z]

#define QB Mixed_Terms[X]
#define QC Mixed_Terms[Y]
#define QF Mixed_Terms[Z]

#define QD Terms[X]
#define QG Terms[Y]
#define QI Terms[Z]

#define QJ Constant

const DBL DEPTH_TOLERANCE = 1.0e-6;


/*****************************************************************************
*
* FUNCTION
*
*   All_Quadric_Intersections
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

bool Quadric::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	DBL Depth1, Depth2;
	VECTOR IPoint;
	register int Intersection_Found;

	Intersection_Found = false;

	Thread->Stats()[Ray_Quadric_Tests]++;
	if (Intersect(ray, &Depth1, &Depth2))
	{
		Thread->Stats()[Ray_Quadric_Tests_Succeeded]++;
		if ((Depth1 > DEPTH_TOLERANCE) && (Depth1 < MAX_DISTANCE))
		{
			VEvaluateRay(IPoint, ray.Origin, Depth1, ray.Direction);
			if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
			{
				Depth_Stack->push(Intersection(Depth1, IPoint, this));

				Intersection_Found = true;
			}
		}

		if ((Depth2 > DEPTH_TOLERANCE) && (Depth2 < MAX_DISTANCE))
		{
			VEvaluateRay(IPoint, ray.Origin, Depth2, ray.Direction);

			if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
			{
				Depth_Stack->push(Intersection(Depth2, IPoint, this));

				Intersection_Found = true;
			}
		}
	}

	return(Intersection_Found);
}



/*****************************************************************************
*
* FUNCTION
*
*   Intersect_Quadric
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

bool Quadric::Intersect(const Ray& ray, DBL *Depth1, DBL *Depth2) const
{
	register DBL a, b, c, d;

	a = Xd * (QA * Xd + QB * Yd + QC * Zd) +
	    Yd * (QE * Yd + QF * Zd) +
	    Zd *  QH * Zd;

	b = Xd * (QA * Xo + 0.5 * (QB * Yo + QC * Zo + QD)) +
	    Yd * (QE * Yo + 0.5 * (QB * Xo + QF * Zo + QG)) +
	    Zd * (QH * Zo + 0.5 * (QC * Xo + QF * Yo + QI));

	c = Xo * (QA * Xo + QB * Yo + QC * Zo + QD) +
	    Yo * (QE * Yo + QF * Zo + QG) +
	    Zo * (QH * Zo + QI) +
	    QJ;

	if (a != 0.0)
	{
		/* The equation is quadratic - find its roots */

		d = Sqr(b) - a * c;

		if (d <= 0.0)
		{
			return(false);
		}

		d = sqrt (d);

		*Depth1 = (-b + d) / (a);
		*Depth2 = (-b - d) / (a);
	}
	else
	{
		/* There are no quadratic terms. Solve the linear equation instead. */

		if (b == 0.0)
		{
			return(false);
		}

		*Depth1 = - 0.5 * c / b;
		*Depth2 = MAX_DISTANCE;
	}

	return(true);
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Quadric
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

bool Quadric::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	/* This is faster and shorter. [DB 7/94] */

	return((IPoint[X] * (QA * IPoint[X] + QB * IPoint[Y] + QD) +
	        IPoint[Y] * (QE * IPoint[Y] + QF * IPoint[Z] + QG) +
	        IPoint[Z] * (QH * IPoint[Z] + QC * IPoint[X] + QI) + QJ) <= 0.0);
}



/*****************************************************************************
*
* FUNCTION
*
*   Quadric_Normal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void Quadric::Normal(VECTOR Result, Intersection *Inter, TraceThreadData *Thread) const
{
	DBL Len;

	/* This is faster and shorter. [DB 7/94] */

	Result[X] = 2.0 * QA * Inter->IPoint[X] +
	                  QB * Inter->IPoint[Y] +
	                  QC * Inter->IPoint[Z] +
	                  QD;

	Result[Y] =       QB * Inter->IPoint[X] +
	            2.0 * QE * Inter->IPoint[Y] +
	                  QF * Inter->IPoint[Z] +
	                  QG;

	Result[Z] =       QC * Inter->IPoint[X] +
	                  QF * Inter->IPoint[Y] +
	            2.0 * QH * Inter->IPoint[Z] +
	                  QI;

	VLength(Len, Result);

	if (Len == 0.0)
	{
		/* The normal is not defined at this point of the surface. */
		/* Set it to any arbitrary direction. */

		Make_Vector(Result, 1.0, 0.0, 0.0);
	}
	else
	{
		VInverseScaleEq(Result, Len);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Quadric
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void Quadric::Transform(const TRANSFORM *tr)
{
	MATRIX Quadric_Matrix, Transform_Transposed;

	Quadric_To_Matrix (Quadric_Matrix);

	MTimesB (tr->inverse, Quadric_Matrix);
	MTranspose (Transform_Transposed, tr->inverse);
	MTimesA (Quadric_Matrix, Transform_Transposed);

	Matrix_To_Quadric (Quadric_Matrix);

	Recompute_BBox(&BBox, tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Quadric_To_Matrix
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void Quadric::Quadric_To_Matrix(MATRIX Matrix) const
{
	MZero (Matrix);

	Matrix[0][0] = Square_Terms[X];
	Matrix[1][1] = Square_Terms[Y];
	Matrix[2][2] = Square_Terms[Z];
	Matrix[0][1] = Mixed_Terms[X];
	Matrix[0][2] = Mixed_Terms[Y];
	Matrix[0][3] = Terms[X];
	Matrix[1][2] = Mixed_Terms[Z];
	Matrix[1][3] = Terms[Y];
	Matrix[2][3] = Terms[Z];
	Matrix[3][3] = Constant;
}



/*****************************************************************************
*
* FUNCTION
*
*   Matrix_To_Quadric
*
* INPUT
*   
* OUTPUT
*
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void Quadric::Matrix_To_Quadric(const MATRIX Matrix)
{
	Square_Terms[X] = Matrix[0][0];
	Square_Terms[Y] = Matrix[1][1];
	Square_Terms[Z] = Matrix[2][2];

	Mixed_Terms[X] = Matrix[0][1] + Matrix[1][0];
	Mixed_Terms[Y] = Matrix[0][2] + Matrix[2][0];
	Mixed_Terms[Z] = Matrix[1][2] + Matrix[2][1];

	Terms[X] = Matrix[0][3] + Matrix[3][0];
	Terms[Y] = Matrix[1][3] + Matrix[3][1];
	Terms[Z] = Matrix[2][3] + Matrix[3][2];

	Constant = Matrix[3][3];
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Quadric
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void Quadric::Translate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Quadric
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void Quadric::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Quadric
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void Quadric::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Quadric
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void Quadric::Invert()
{
	VScaleEq(Square_Terms, -1.0);
	VScaleEq(Mixed_Terms, -1.0);
	VScaleEq(Terms, -1.0);

	Constant *= -1.0;

	Invert_Flag(this, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Quadric
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

Quadric::Quadric() : ObjectBase(QUADRIC_OBJECT)
{
	Make_Vector(Square_Terms, 1.0, 1.0, 1.0);
	Make_Vector(Mixed_Terms, 0.0, 0.0, 0.0);
	Make_Vector(Terms, 0.0, 0.0, 0.0);
	Constant = 1.0;
	Automatic_Bounds = false;
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Quadric
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

ObjectPtr Quadric::Copy()
{
	Quadric *New = new Quadric();
	Destroy_Transform(New->Trans);
	*New = *this;

	return(New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Quadric
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

Quadric::~Quadric()
{
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Quadric_BBox
*
* INPUT
*
*   Quadric - Qaudric object
*   
* OUTPUT
*
*   Quadric
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Compute a bounding box around a quadric.
*
*   This function calculates the bounding box of a quadric given in
*   its normal form, i.e. f(x,y,z) = A*x^2 + E*y^2 + H*z^2 + J = 0.
*
*   NOTE: Translated quadrics can also be bounded by this function.
*
*         Supported: cones, cylinders, ellipsoids, hyperboloids.
*
* CHANGES
*
*   May 1994 : Creation.
*
*   Sep 1994 : Added support of hyperpoloids. Improved bounding of
*              quadrics used in CSG intersections. [DB]
*
******************************************************************************/

void Quadric::Compute_BBox()
{
	VECTOR pOne;
	VECTOR mOne;

	Make_Vector(pOne, 1.0, 1.0, 1.0);
	Make_Vector(mOne, -1.0, -1.0, -1.0);

	Compute_BBox(mOne, pOne);
}

void Quadric::Compute_BBox(VECTOR ClipMin, VECTOR ClipMax)
{
	DBL A, B, C, D, E, F, G, H, I, J;
	DBL a, b, c, d;
	DBL rx, ry, rz, rx1, rx2, ry1, ry2, rz1, rz2, x, y, z;
	DBL New_Volume, Old_Volume;
	VECTOR TmpMin, TmpMax, NewMin, NewMax, T1;
	BBOX Old = BBox;

	if(!Clip.empty())
	{
		/* Intersect the members bounding boxes. */
		for(vector<ObjectPtr>::iterator it = Clip.begin(); it != Clip.end(); it++)
		{
			ObjectPtr p = *it;
			if (Test_Flag(p, INVERTED_FLAG) == false)
			{
				if (dynamic_cast<Plane *> (p) != NULL)
					Compute_Plane_Min_Max((Plane *) p, TmpMin, TmpMax);
				else
					Make_min_max_from_BBox(TmpMin, TmpMax, p->BBox);

				ClipMin[X] = max(ClipMin[X], TmpMin[X]);
				ClipMin[Y] = max(ClipMin[Y], TmpMin[Y]);
				ClipMin[Z] = max(ClipMin[Z], TmpMin[Z]);
				ClipMax[X] = min(ClipMax[X], TmpMax[X]);
				ClipMax[Y] = min(ClipMax[Y], TmpMax[Y]);
				ClipMax[Z] = min(ClipMax[Z], TmpMax[Z]);
			}
		}
	}

	/*
	 * Check for 'normal' form. If the quadric isn't in it's normal form
	 * we can't do anything (we could, but that would be to tedious!
	 * Diagonalising the quadric's 4x4 matrix, i.e. finding its eigenvalues
	 * and eigenvectors -> solving a 4th order polynom).
	 */

	/* Get quadrics coefficients. */

	A = Square_Terms[X];
	E = Square_Terms[Y];
	H = Square_Terms[Z];
	B = Mixed_Terms[X] / 2.0;
	C = Mixed_Terms[Y] / 2.0;
	F = Mixed_Terms[Z] / 2.0;
	D = Terms[X] / 2.0;
	G = Terms[Y] / 2.0;
	I = Terms[Z] / 2.0;
	J = Constant;

	/* Set small values to 0. */

	if (fabs(A) < EPSILON) A = 0.0;
	if (fabs(B) < EPSILON) B = 0.0;
	if (fabs(C) < EPSILON) C = 0.0;
	if (fabs(D) < EPSILON) D = 0.0;
	if (fabs(E) < EPSILON) E = 0.0;
	if (fabs(F) < EPSILON) F = 0.0;
	if (fabs(G) < EPSILON) G = 0.0;
	if (fabs(H) < EPSILON) H = 0.0;
	if (fabs(I) < EPSILON) I = 0.0;
	if (fabs(J) < EPSILON) J = 0.0;

	/* Non-zero mixed terms --> return */

	if ((B != 0.0) || (C != 0.0) || (F != 0.0))
	{
		New_Volume = (ClipMax[X] - ClipMin[X]) * (ClipMax[Y] - ClipMin[Y]) * (ClipMax[Z] - ClipMin[Z]);
		BOUNDS_VOLUME(Old_Volume, Old);
		if (New_Volume < Old_Volume)
			Make_BBox_from_min_max(BBox, ClipMin, ClipMax);
		return;
	}

	/* Non-zero linear terms --> get translation vector */

	if ((D != 0.0) || (G != 0.0) || (I != 0.0))
	{
		if (A != 0.0)
		{
			T1[X] = -D / A;
		}
		else
		{
			if (D != 0.0)
			{
			 T1[X] = J / (2.0 * D);
			}
			else
			{
				T1[X] = 0.0;
			}
		}

		if (E != 0.0)
		{
			T1[Y] = -G / E;
		}
		else
		{
			if (G != 0.0)
			{
				T1[Y] = J / (2.0 * G);
			}
			else
			{
				T1[Y] = 0.0;
			}
		}

		if (H != 0.0)
		{
			T1[Z] = -I / H;
		}
		else
		{
			if (I != 0.0)
			{
				T1[Z] = J / (2.0 * I);
			}
			else
			{
				T1[Z] = 0.0;
			}
		}

		/* Recalculate coefficients. */

		D += A * T1[X];
		G += E * T1[Y];
		I += H * T1[Z];
		J -= T1[X]*(A*T1[X] + 2.0*D) + T1[Y]*(E*T1[Y] + 2.0*G) + T1[Z]*(H*T1[Z] + 2.0*I);
	}
	else
	{
		Make_Vector(T1, 0.0, 0.0, 0.0);
	}

	/* Init new bounding box. */

	NewMin[X] = NewMin[Y] = NewMin[Z] = -BOUND_HUGE/2;
	NewMax[X] = NewMax[Y] = NewMax[Z] =  BOUND_HUGE/2;

	/* Translate clipping box. */

	VSubEq(ClipMin, T1);
	VSubEq(ClipMax, T1);

	/* We want A to be non-negative. */

	if (A < 0.0)
	{
		A = -A;
		D = -D;
		E = -E;
		G = -G;
		H = -H;
		I = -I;
		J = -J;
	}

	/*
	 *
	 * Check for ellipsoid.
	 *
	 *    x*x     y*y     z*z
	 *   ----- + ----- + ----- - 1 = 0
	 *    a*a     b*b     c*c
	 *
	 */

	if ((A > 0.0) && (E > 0.0) && (H > 0.0) && (J < 0.0))
	{
		a = sqrt(-J/A);
		b = sqrt(-J/E);
		c = sqrt(-J/H);

		NewMin[X] = -a;
		NewMin[Y] = -b;
		NewMin[Z] = -c;
		NewMax[X] = a;
		NewMax[Y] = b;
		NewMax[Z] = c;
	}

	/*
	 *
	 * Check for cylinder (x-axis).
	 *
	 *    y*y     z*z
	 *   ----- + ----- - 1 = 0
	 *    b*b     c*c
	 *
	 */

	if ((A == 0.0) && (E > 0.0) && (H > 0.0) && (J < 0.0))
	{
		b = sqrt(-J/E);
		c = sqrt(-J/H);

		NewMin[Y] = -b;
		NewMin[Z] = -c;
		NewMax[Y] = b;
		NewMax[Z] = c;
	}

	/*
	 *
	 * Check for cylinder (y-axis).
	 *
	 *    x*x     z*z
	 *   ----- + ----- - 1 = 0
	 *    a*a     c*c
	 *
	 */

	if ((A > 0.0) && (E == 0.0) && (H > 0.0) && (J < 0.0))
	{
		a = sqrt(-J/A);
		c = sqrt(-J/H);

		NewMin[X] = -a;
		NewMin[Z] = -c;
		NewMax[X] = a;
		NewMax[Z] = c;
	}

	/*
	 *
	 * Check for cylinder (z-axis).
	 * 
	 *    x*x     y*y
	 *   ----- + ----- - 1 = 0
	 *    a*a     b*b
	 *
	 */

	if ((A > 0.0) && (E > 0.0) && (H == 0.0) && (J < 0.0))
	{
		a = sqrt(-J/A);
		b = sqrt(-J/E);

		NewMin[X] = -a;
		NewMin[Y] = -b;
		NewMax[X] = a;
		NewMax[Y] = b;
	}

	/*
	 *
	 * Check for cone (x-axis).
	 *
	 *    x*x     y*y     z*z
	 *   ----- - ----- - ----- = 0
	 *    a*a     b*b     c*c
	 *
	 */

	if ((A > 0.0) && (E < 0.0) && (H < 0.0) && (J == 0.0))
	{
		a = sqrt(1.0/A);
		b = sqrt(-1.0/E);
		c = sqrt(-1.0/H);

		/* Get radii for lower x value. */

		x = ClipMin[X];

		ry1 = fabs(x * b / a);
		rz1 = fabs(x * c / a);

		/* Get radii for upper x value. */

		x = ClipMax[X];

		ry2 = fabs(x * b / a);
		rz2 = fabs(x * c / a);

		ry = max(ry1, ry2);
		rz = max(rz1, rz2);

		NewMin[Y] = -ry;
		NewMin[Z] = -rz;
		NewMax[Y] = ry;
		NewMax[Z] = rz;
	}

	/*
	 *
	 *  Check for cone (y-axis).
	 *
	 *    x*x     y*y     z*z
	 *   ----- - ----- + ----- = 0
	 *    a*a     b*b     c*c
	 *
	 */

	if ((A > 0.0) && (E < 0.0) && (H > 0.0) && (J == 0.0))
	{
		a = sqrt(1.0/A);
		b = sqrt(-1.0/E);
		c = sqrt(1.0/H);

		/* Get radii for lower y value. */

		y = ClipMin[Y];

		rx1 = fabs(y * a / b);
		rz1 = fabs(y * c / b);

		/* Get radii for upper y value. */

		y = ClipMax[Y];

		rx2 = fabs(y * a / b);
		rz2 = fabs(y * c / b);

		rx = max(rx1, rx2);
		rz = max(rz1, rz2);

		NewMin[X] = -rx;
		NewMin[Z] = -rz;
		NewMax[X] = rx;
		NewMax[Z] = rz;
	}

	/*
	 *
	 * Check for cone (z-axis).
	 * 
	 *    x*x     y*y     z*z
	 *   ----- + ----- - ----- = 0
	 *    a*a     b*b     c*c
	 *
	 */

	if ((A > 0.0) && (E > 0.0) && (H < 0.0) && (J == 0.0))
	{
		a = sqrt(1.0/A);
		b = sqrt(1.0/E);
		c = sqrt(-1.0/H);

		/* Get radii for lower z value. */

		z = ClipMin[Z];

		rx1 = fabs(z * a / c);
		ry1 = fabs(z * b / c);

		/* Get radii for upper z value. */

		z = ClipMax[Z];

		rx2 = fabs(z * a / c);
		ry2 = fabs(z * b / c);

		rx = max(rx1, rx2);
		ry = max(ry1, ry2);

		NewMin[X] = -rx;
		NewMin[Y] = -ry;
		NewMax[X] = rx;
		NewMax[Y] = ry;
	}

	/*
	 *
	 * Check for hyperboloid (x-axis).
	 *
	 *    x*x     y*y     z*z
	 *   ----- - ----- - ----- + 1 = 0
	 *    a*a     b*b     c*c
	 *
	 */

	if ((A > 0.0) && (E < 0.0) && (H < 0.0) && (J > 0.0))
	{
		/* Get radii for lower x value. */

		x = ClipMin[X];

		d = 1.0 + A * Sqr(x);

		ry1 = sqrt(-d / E);
		rz1 = sqrt(-d / H);

		/* Get radii for upper x value. */

		x = ClipMax[X];

		d = 1.0 + A * Sqr(x);

		ry2 = sqrt(-d / E);
		rz2 = sqrt(-d / H);

		ry = max(ry1, ry2);
		rz = max(rz1, rz2);

		NewMin[Y] = -ry;
		NewMin[Z] = -rz;
		NewMax[Y] = ry;
		NewMax[Z] = rz;
	}

	/*
	 *
	 * Check for hyperboloid (y-axis).
	 * 
	 *    x*x     y*y     z*z
	 *   ----- - ----- + ----- - 1 = 0
	 *    a*a     b*b     c*c
	 *
	 */

	if ((A > 0.0) && (E < 0.0) && (H > 0.0) && (J < 0.0))
	{
		/* Get radii for lower y value. */

		y = ClipMin[Y];

		d = 1.0 - E * Sqr(y);

		rx1 = sqrt(d / A);
		rz1 = sqrt(d / H);

		/* Get radii for upper y value. */

		y = ClipMax[Y];

		d = 1.0 - E * Sqr(y);

		rx2 = sqrt(d / A);
		rz2 = sqrt(d / H);

		rx = max(rx1, rx2);
		rz = max(rz1, rz2);

		NewMin[X] = -rx;
		NewMin[Z] = -rz;
		NewMax[X] = rx;
		NewMax[Z] = rz;
	}

	/*
	 *
	 * Check for hyperboloid (z-axis).
	 *
	 *    x*x     y*y     z*z
	 *   ----- + ----- - ----- - 1 = 0
	 *    a*a     b*b     c*c
	 *
	 */

	if ((A > 0.0) && (E > 0.0) && (H < 0.0) && (J < 0.0))
	{
		/* Get radii for lower z value. */

		z = ClipMin[Z];

		d = 1.0 - H * Sqr(z);

		rx1 = sqrt(d / A);
		ry1 = sqrt(d / E);

		/* Get radii for upper z value. */

		z = ClipMax[Z];

		d = 1.0 - H * Sqr(z);

		rx2 = sqrt(d / A);
		ry2 = sqrt(d / E);

		rx = max(rx1, rx2);
		ry = max(ry1, ry2);

		NewMin[X] = -rx;
		NewMin[Y] = -ry;
		NewMax[X] = rx;
		NewMax[Y] = ry;
	}

	/*
	 *
	 * Check for paraboloid (x-axis).
	 *
	 *        y*y     z*z
	 *   x - ----- - ----- = 0
	 *        b*b     c*c
	 *
	 */

	if ((A == 0.0) && (D != 0.0) && (E * H > 0.0) && (J == 0.0))
	{
		/* Get radii for lower x value. */

		x = D * E < 0 ? max(0., ClipMin[X]) : ClipMin[X];

		ry1 = sqrt(fabs(2.0 * D * x / E));
		rz1 = sqrt(fabs(2.0 * D * x / H));

		/* Get radii for upper x value. */

		x = D * E > 0 ? min(0., ClipMax[X]) : ClipMax[X];

		ry2 = sqrt(fabs(2.0 * D * x / E));
		rz2 = sqrt(fabs(2.0 * D * x / H));

		ry = max(ry1, ry2);
		rz = max(rz1, rz2);

		if (D*E < 0) NewMin[X] = max(0., ClipMin[X]);
		NewMin[Y] = -ry;
		NewMin[Z] = -rz;
		if (D*E > 0) NewMax[X] = min(0., ClipMax[X]);
		NewMax[Y] = ry;
		NewMax[Z] = rz;
	}

	/*
	 *
	 * Check for paraboloid (y-axis).
	 *
	 *        x*x     z*z
	 *   y - ----- - ----- = 0
	 *        a*a     c*c
	 *
	 */

	if ((E == 0.0) && (G != 0.0) && (A * H > 0.0) && (J == 0.0))
	{
		/* Get radii for lower y-value. */
		y = G > 0 ? ClipMin[Y] : max(0., ClipMin[Y]);

		rx1 = sqrt(fabs(2.0 * G * y / A));
		rz1 = sqrt(fabs(2.0 * G * y / H));

		/* Get radii for upper y value. */

		y = G < 0 ? ClipMax[Y] : min(0., ClipMax[Y]);

		rx2 = sqrt(fabs(2.0 * G * y / A));
		rz2 = sqrt(fabs(2.0 * G * y / H));

		rx = max(rx1, rx2);
		rz = max(rz1, rz2);

		NewMin[X] = -rx;
		if (G < 0) NewMin[Y] = max(0., ClipMin[Y]);
		NewMin[Z] = -rz;
		NewMax[X] = rx;
		if (G > 0) NewMax[Y] = min(0., ClipMax[Y]);
		NewMax[Z] = rz;
	}

	/*
	 *
	 * Check for paraboloid (z-axis).
	 *
	 *        x*x     y*y
	 *   z - ----- - ----- = 0
	 *        a*a     b*b
	 *
	 */

	if ((H == 0.0) && (I != 0.0) && (A * E > 0.0) && (J == 0.0))
	{
		/* Get radii for lower z-value. */

		z = I < 0 ? max(ClipMin[Z], 0.) : ClipMin[Z];

		rx1 = sqrt(fabs(2.0 * I * z / A));
		ry1 = sqrt(fabs(2.0 * I * z / E));

		/* Get radii for upper z value. */

		z = I > 0 ? min(0., ClipMax[Z]) : ClipMax[Z];

		rx2 = sqrt(fabs(2.0 * I * z / A));
		ry2 = sqrt(fabs(2.0 * I * z / E));

		rx = max(rx1, rx2);
		ry = max(ry1, ry2);

		NewMin[X] = -rx;
		NewMin[Y] = -ry;
		if (I < 0) NewMin[Z] = max(ClipMin[Z], 0.);
		NewMax[X] = rx;
		NewMax[Y] = ry;
		if (I > 0) NewMax[Z] = min(ClipMax[Z], 0.);
	}

	/* Intersect clipping object's and quadric's bounding boxes */

	NewMin[X] = max(NewMin[X], ClipMin[X]);
	NewMin[Y] = max(NewMin[Y], ClipMin[Y]);
	NewMin[Z] = max(NewMin[Z], ClipMin[Z]);

	NewMax[X] = min(NewMax[X], ClipMax[X]);
	NewMax[Y] = min(NewMax[Y], ClipMax[Y]);
	NewMax[Z] = min(NewMax[Z], ClipMax[Z]);

	/* Use old or new bounding box? */

	New_Volume = (NewMax[X] - NewMin[X]) * (NewMax[Y] - NewMin[Y]) * (NewMax[Z] - NewMin[Z]);

	BOUNDS_VOLUME(Old_Volume, Old);

	if (New_Volume < Old_Volume)
	{
		/* Add translation. */
		Automatic_Bounds = true;

		VAddEq(NewMin, T1);
		VAddEq(NewMax, T1);

		Make_BBox_from_min_max(BBox, NewMin, NewMax);

		/* Beware of bounding boxes to large. */

		if ((BBox.Lengths[X] > CRITICAL_LENGTH) ||
		    (BBox.Lengths[Y] > CRITICAL_LENGTH) ||
		    (BBox.Lengths[Z] > CRITICAL_LENGTH))
		{
			Make_BBox(BBox, -BOUND_HUGE/2, -BOUND_HUGE/2, -BOUND_HUGE/2,
			  BOUND_HUGE, BOUND_HUGE, BOUND_HUGE);
		}
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Plane_Min_Max
*
* INPUT
*
*   Plane    - Plane
*   Min, Max - Vectors containing plane's dimensions
*   
* OUTPUT
*
*   Min, Max
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Compute min/max vectors for planes perpendicular to an axis.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Quadric::Compute_Plane_Min_Max(Plane *plane, VECTOR Min, VECTOR  Max)
{
	DBL d;
	VECTOR P, N;

	if (plane->Trans == NULL)
	{
		Assign_Vector(N, plane->Normal_Vector);

		d = -plane->Distance;
	}
	else
	{
		MInvTransNormal(N, plane->Normal_Vector, plane->Trans);

		MInvTransPoint(P, N, plane->Trans);

		d = -plane->Distance - P[X] * N[X] - P[Y] * N[Y] - P[Z] * N[Z] + 1.0;
	}

	Min[X] = Min[Y] = Min[Z] = -BOUND_HUGE/2;
	Max[X] = Max[Y] = Max[Z] =  BOUND_HUGE/2;

	/* y-z-plane */

	if ((fabs(N[Y]) < EPSILON) && (fabs(N[Z]) < EPSILON)) // [CLi] can't test for fabs(1-N[X])<EPSILON because N isn't normalized
	{
		if (N[X] > 0.0)
		{
			Max[X] = d;
		}
		else
		{
			Min[X] = -d;
		}
	}

	/* x-z-plane */

	if ((fabs(N[X]) < EPSILON) && (fabs(N[Z]) < EPSILON)) // [CLi] can't test for fabs(1-N[Y])<EPSILON because N isn't normalized
	{
		if (N[Y] > 0.0)
		{
			Max[Y] = d;
		}
		else
		{
			Min[Y] = -d;
		}
	}

	/* x-y-plane */

	if ((fabs(N[X]) < EPSILON) && (fabs(N[Y]) < EPSILON)) // [CLi] can't test for fabs(1-N[Z])<EPSILON because N isn't normalized
	{
		if (N[Z] > 0.0)
		{
			Max[Z] = d;
		}
		else
		{
			Min[Z] = -d;
		}
	}
}

}