/*******************************************************************************
 * mesh.cpp
 *
 * This module implements primitives for mesh objects.
 *
 * This module was written by Dieter Bayer [DB].
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
 * $File: //depot/povray/smp/source/backend/shape/mesh.cpp $
 * $Revision: #42 $
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

/****************************************************************************
*
*  Explanation:
*
*    -
*
*  Syntax:
*
*    mesh
*    {
*      triangle { <CORNER1>, <CORNER2>, <CORNER3>, texture { NAME } }
*      smooth_triangle { <CORNER1>, <NORMAL1>, <CORNER2>, <NORMAL2>, <CORNER3>, <NORMAL3>, texture { NAME } }
*      ...
*      [ hierarchy FLAG ]
*    }
*
*  ---
*
*  Feb 1995 : Creation. [DB]
*
*****************************************************************************/

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/math/vector.h"
#include "backend/bounding/bbox.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/shape/mesh.h"
#include "backend/texture/texture.h"
#include "backend/shape/triangle.h"
#include "backend/scene/threaddata.h"
#include "base/pov_err.h"

#include <algorithm>

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

const DBL DEPTH_TOLERANCE = 1e-6;

#define max3_coordinate(x,y,z) ((x > y) ? ((x > z) ? X : Z) : ((y > z) ? Y : Z))

const int HASH_SIZE = 1000;

const int INITIAL_NUMBER_OF_ENTRIES = 256;



/*****************************************************************************
* Local variables
******************************************************************************/

HASH_TABLE **Mesh::Vertex_Hash_Table;
HASH_TABLE **Mesh::Normal_Hash_Table;
UV_HASH_TABLE **Mesh::UV_Hash_Table;

/*****************************************************************************
*
* FUNCTION
*
*   All_Mesh_Intersections
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

bool Mesh::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	Thread->Stats()[Ray_Mesh_Tests]++;

	if (Intersect(ray, Depth_Stack, Thread))
	{
		Thread->Stats()[Ray_Mesh_Tests_Succeeded]++;
		return(true);
	}

	return(false);
}



/*****************************************************************************
*
* FUNCTION
*
*   Intersect_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

bool Mesh::Intersect(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	int i;
	bool found;
	DBL len, t;
	Ray New_Ray;

	/* Transform the ray into mesh space. */

	if (Trans != NULL)
	{
		MInvTransPoint(New_Ray.Origin, ray.Origin, Trans);
		MInvTransDirection(New_Ray.Direction, ray.Direction, Trans);

		VLength(len, New_Ray.Direction);
		VInverseScaleEq(New_Ray.Direction, len);
	}
	else
	{
		New_Ray = ray;

		len = 1.0;
	}

	found = false;

	if (Data->Tree == NULL)
	{
		/* There's no bounding hierarchy so just step through all elements. */

		for (i = 0; i < Data->Number_Of_Triangles; i++)
		{
			if (intersect_mesh_triangle(New_Ray, &Data->Triangles[i], &t))
			{
				if (test_hit(&Data->Triangles[i], ray, t, len, Depth_Stack, Thread))
				{
					found = true;
				}
			}
		}
	}
	else
	{
		/* Use the mesh's bounding hierarchy. */

		return(intersect_bbox_tree(New_Ray, ray, len, Depth_Stack, Thread));
	}

	return(found);
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Nathan Kopp & Dieter Bayer (adapted from Intersect_Mesh by Dieter Bayer)
*   
* DESCRIPTION
*
*   Shoot a ray out from this point, if the ray hits an odd number of
*     triangles, it is inside the object.  If it hits an even number
*     than it is outside the object.
*   The triangle mesh should be closed, otherwise results are unpredictable.
*
* CHANGES
*
*   October, 1998 : Creation.
*
******************************************************************************/

bool Mesh::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	int inside, i;
	unsigned found;
	DBL len, t;
	Ray ray, New_Ray;

	if (has_inside_vector==false)
		return false;

	/* shoot a ray in the chosen direction from this point */
//	if( (fabs(Data->Inside_Vect[X]) < EPSILON) &&
//	    (fabs(Data->Inside_Vect[Y]) < EPSILON) &&
//	    (fabs(Data->Inside_Vect[Z]) < EPSILON))
//	{
//		/* if no inside_vect , use X... maybe we should quit */
//		/* return(false); */
//
//		Make_Vector(Ray.Direction, 0, 0, 1);
//	}
//	else

	Assign_Vector(ray.Direction, Data->Inside_Vect);

	Assign_Vector(ray.Origin, IPoint);

	/* Transform the ray into mesh space. */
	if (Trans != NULL)
	{
		MInvTransPoint(New_Ray.Origin, ray.Origin, Trans);
		MInvTransDirection(New_Ray.Direction, ray.Direction, Trans);

		VLength(len, New_Ray.Direction);
		VInverseScaleEq(New_Ray.Direction, len);
	}
	else
	{
		New_Ray = ray;
		len = 1.0;
	}

	found = 0;

	if (Data->Tree == NULL)
	{
		/* just step through all elements. */
		for (i = 0; i < Data->Number_Of_Triangles; i++)
		{
			if (intersect_mesh_triangle(New_Ray, &Data->Triangles[i], &t))
			{
				/* actually, this should push onto a local depth stack and
				   make sure that we don't have the same intersection point from
				   two (or three) different triangles!!!!! */
				found++;
			}
		}
		/* odd number = inside, even number = outside */
		inside = found & 1;
	}
	else
	{
		/* Use the mesh's bounding hierarchy. */
		inside = inside_bbox_tree(New_Ray, Thread);
	}

	if (Test_Flag(this, INVERTED_FLAG))
	{
		inside = !inside;
	}
	return (inside);
}




/*****************************************************************************
*
* FUNCTION
*
*   Mesh_Normal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Return the normalized normal in the given point.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::Normal(VECTOR Result, Intersection *Inter, TraceThreadData *Thread) const
{
	VECTOR IPoint;
	const MESH_TRIANGLE *Triangle;

	Triangle = reinterpret_cast<const MESH_TRIANGLE *>(Inter->Pointer);

	if (Triangle->Smooth)
	{
		if (Trans != NULL)
		{
			MInvTransPoint(IPoint, Inter->IPoint, Trans);
		}
		else
		{
			Assign_Vector(IPoint, Inter->IPoint);
		}

		Smooth_Mesh_Normal(Result, Triangle, IPoint);

		if (Trans != NULL)
		{
			MTransNormal(Result, Result, Trans);
		}

		VNormalize(Result, Result);
	}
	else
	{
		Assign_Vector(Result, Data->Normals[Triangle->Normal_Ind]);

		if (Trans != NULL)
		{
			MTransNormal(Result, Result, Trans);

			VNormalize(Result, Result);
		}
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Smooth_Mesh_Normal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Remove the un-normalized normal of a smoothed triangle.
*
* CHANGES
*
*   Feb 1995 : Creation. (Derived from TRIANGLE.C)
*
******************************************************************************/

void Mesh::Smooth_Mesh_Normal(VECTOR Result, const MESH_TRIANGLE *Triangle, const VECTOR IPoint) const
{
	int axis;
	DBL u, v;
	DBL k1, k2, k3;
	VECTOR PIMinusP1, N1, N2, N3;

	get_triangle_normals(Triangle, N1, N2, N3);

	VSub(PIMinusP1, IPoint, Data->Vertices[Triangle->P1]);

	VDot(u, PIMinusP1, Triangle->Perp);

	if (u < EPSILON)
	{
		Assign_Vector(Result, N1);
	}
	else
	{
		axis = Triangle->vAxis;

		k1 = Data->Vertices[Triangle->P1][axis];
		k2 = Data->Vertices[Triangle->P2][axis];
		k3 = Data->Vertices[Triangle->P3][axis];

		v = (PIMinusP1[axis] / u + k1 - k2) / (k3 - k2);

		Result[X] = N1[X] + u * (N2[X] - N1[X] + v * (N3[X] - N2[X]));
		Result[Y] = N1[Y] + u * (N2[Y] - N1[Y] + v * (N3[Y] - N2[Y]));
		Result[Z] = N1[Z] + u * (N2[Z] - N1[Z] + v * (N3[Z] - N2[Z]));
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::Translate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transfrom_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::Transform(const TRANSFORM *tr)
{
	int i;

	if (Trans == NULL)
	{
		Trans = Create_Transform();
	}

	Recompute_BBox(&BBox, tr);

	Compose_Transforms(Trans, tr);

	/* NK 1998 added if */
	if (!Test_Flag(this, UV_FLAG))
		for (i=0; i<Number_Of_Textures; i++)
			Transform_Textures(Textures[i], tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::Invert()
{
	Invert_Flag(this, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

Mesh::Mesh() : ObjectBase(MESH_OBJECT)
{
	Set_Flag(this, HIERARCHY_FLAG);

	Trans = NULL;

	Data = NULL;

	has_inside_vector=false;

	Number_Of_Textures=0; /* [LSK] these were uninitialized */
	Textures=NULL;
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Copy a mesh.
*
*   NOTE: The components are not copied, only the number of references is
*         counted, so that Destroy_Mesh() knows if they can be destroyed.
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

ObjectPtr Mesh::Copy()
{
	Mesh *New = new Mesh();
	int i;

	Destroy_Transform(New->Trans);
	*New = *this;
	New->Trans = Copy_Transform(Trans);

	New->Data = Data;
	New->Data->References++;

	/* NK 1999 copy textures */
	if(Textures != NULL)
	{
		New->Textures = reinterpret_cast<TEXTURE **>(POV_MALLOC(Number_Of_Textures*sizeof(TEXTURE *), "triangle mesh data"));
		for (i = 0; i < Number_Of_Textures; i++)
			New->Textures[i] = Copy_Textures(Textures[i]);
	}

	return(New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

Mesh::~Mesh()
{
	int i;

	Destroy_Transform(Trans);

	/* NK 1999 move texture outside of data block */
	if (Textures != NULL)
	{
		for (i = 0; i < Number_Of_Textures; i++)
		{
			Destroy_Textures(Textures[i]);
		}

		POV_FREE(Textures);
	}

	if (--(Data->References) == 0)
	{
		Destroy_BBox_Tree(Data->Tree);

		if (Data->Normals != NULL)
		{
			POV_FREE(Data->Normals);
		}

		/* NK 1998 */
		if (Data->UVCoords != NULL)
		{
			POV_FREE(Data->UVCoords);
		}
		/* NK ---- */

		if (Data->Vertices != NULL)
		{
			POV_FREE(Data->Vertices);
		}

		if (Data->Triangles != NULL)
		{
			POV_FREE(Data->Triangles);
		}

		POV_FREE(Data);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Mesh_BBox
*
* INPUT
*
*   Mesh - Mesh
*   
* OUTPUT
*
*   Mesh
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the bounding box of a triangle.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::Compute_BBox()
{
	int i;
	VECTOR P1, P2, P3;
	VECTOR mins, maxs;

	Make_Vector(mins, BOUND_HUGE, BOUND_HUGE, BOUND_HUGE);
	Make_Vector(maxs, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);

	for (i = 0; i < Data->Number_Of_Triangles; i++)
	{
		get_triangle_vertices(&Data->Triangles[i], P1, P2, P3);

		mins[X] = min(mins[X], min3(P1[X], P2[X], P3[X]));
		mins[Y] = min(mins[Y], min3(P1[Y], P2[Y], P3[Y]));
		mins[Z] = min(mins[Z], min3(P1[Z], P2[Z], P3[Z]));

		maxs[X] = max(maxs[X], max3(P1[X], P2[X], P3[X]));
		maxs[Y] = max(maxs[Y], max3(P1[Y], P2[Y], P3[Y]));
		maxs[Z] = max(maxs[Z], max3(P1[Z], P2[Z], P3[Z]));
	}

	Make_BBox_from_min_max(BBox, mins, maxs);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

bool Mesh::Compute_Mesh_Triangle(MESH_TRIANGLE *Triangle, int Smooth, VECTOR P1, VECTOR  P2, VECTOR  P3, VECTOR  S_Normal)
{
	int temp, swap;
	DBL x, y, z;
	VECTOR V1, V2, T1;
	DBL Length;

	VSub(V1, P2, P1);
	VSub(V2, P3, P1);

	VCross(S_Normal, V2, V1);

	VLength(Length, S_Normal);

	/* Set up a flag so we can ignore degenerate triangles */

	if (Length == 0.0)
	{
		return(false);
	}

	/* Normalize the normal vector. */

	VInverseScaleEq(S_Normal, Length);

	VDot(Triangle->Distance, S_Normal, P1);

	Triangle->Distance *= -1.0;

	/* Find triangle's dominant axis. */

	x = fabs(S_Normal[X]);
	y = fabs(S_Normal[Y]);
	z = fabs(S_Normal[Z]);

	Triangle->Dominant_Axis = max3_coordinate(x, y, z);

	swap = false;

	switch (Triangle->Dominant_Axis)
	{
		case X:

			if ((P2[Y] - P3[Y])*(P2[Z] - P1[Z]) < (P2[Z] - P3[Z])*(P2[Y] - P1[Y]))
			{
				swap = true;
			}

			break;

		case Y:

			if ((P2[X] - P3[X])*(P2[Z] - P1[Z]) < (P2[Z] - P3[Z])*(P2[X] - P1[X]))
			{
				swap = true;
			}

			break;

		case Z:

			if ((P2[X] - P3[X])*(P2[Y] - P1[Y]) < (P2[Y] - P3[Y])*(P2[X] - P1[X]))
			{
				swap = true;
			}

			break;
	}

	if (swap)
	{
		temp = Triangle->P2;
		Triangle->P2 = Triangle->P1;
		Triangle->P1 = temp;

		/* NK 1998 */
		temp = Triangle->UV2;
		Triangle->UV2 = Triangle->UV1;
		Triangle->UV1 = temp;

		if (Triangle->ThreeTex)
		{
			temp = Triangle->Texture2;
			Triangle->Texture2 = Triangle->Texture;
			Triangle->Texture = temp;
		}

		Assign_Vector(T1, P1);
		Assign_Vector(P1, P2);
		Assign_Vector(P2, T1);

		if (Smooth)
		{
			temp = Triangle->N2;
			Triangle->N2 = Triangle->N1;
			Triangle->N1 = temp;
		}
	}

	if (Smooth)
	{
	//	compute_smooth_triangle(Triangle, P1, P2, P3);
		Triangle->Smooth = true;
	}

	compute_smooth_triangle(Triangle, P1, P2, P3);

	return(true);
}



/*****************************************************************************
*
* FUNCTION
*
*   compute_smooth_triangle
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::compute_smooth_triangle(MESH_TRIANGLE *Triangle, const VECTOR P1, const VECTOR P2, const VECTOR P3)
{
	VECTOR P3MinusP2, VTemp1, VTemp2;
	DBL x, y, z, uDenominator, Proj;

	VSub(P3MinusP2, P3, P2);

	x = fabs(P3MinusP2[X]);
	y = fabs(P3MinusP2[Y]);
	z = fabs(P3MinusP2[Z]);

	Triangle->vAxis = max3_coordinate(x, y, z);

	VSub(VTemp1, P2, P3);

	VNormalize(VTemp1, VTemp1);

	VSub(VTemp2, P1, P3);

	VDot(Proj, VTemp2, VTemp1);

	VScaleEq(VTemp1, Proj);

	VSub(Triangle->Perp, VTemp1, VTemp2);

	VNormalize(Triangle->Perp, Triangle->Perp);

	VDot(uDenominator, VTemp2, Triangle->Perp);

	VInverseScaleEq(Triangle->Perp, -uDenominator);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_mesh_triangle
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

bool Mesh::intersect_mesh_triangle(const Ray &ray, const MESH_TRIANGLE *Triangle, DBL *Depth) const
{
	DBL NormalDotOrigin, NormalDotDirection;
	DBL s, t;
	VECTOR P1, P2, P3, S_Normal;

	Assign_Vector(S_Normal, Data->Normals[Triangle->Normal_Ind]);

	VDot(NormalDotDirection, S_Normal, ray.Direction);

	if (fabs(NormalDotDirection) < EPSILON)
	{
		return(false);
	}

	VDot(NormalDotOrigin, S_Normal, ray.Origin);

	*Depth = -(Triangle->Distance + NormalDotOrigin) / NormalDotDirection;

	if ((*Depth < DEPTH_TOLERANCE) || (*Depth > MAX_DISTANCE))
	{
		return(false);
	}

	get_triangle_vertices(Triangle, P1, P2, P3);

	switch (Triangle->Dominant_Axis)
	{
		case X:

			s = ray.Origin[Y] + *Depth * ray.Direction[Y];
			t = ray.Origin[Z] + *Depth * ray.Direction[Z];

			if ((P2[Y] - s) * (P2[Z] - P1[Z]) < (P2[Z] - t) * (P2[Y] - P1[Y]))
			{
				return(false);
			}

			if ((P3[Y] - s) * (P3[Z] - P2[Z]) < (P3[Z] - t) * (P3[Y] - P2[Y]))
			{
				return(false);
			}

			if ((P1[Y] - s) * (P1[Z] - P3[Z]) < (P1[Z] - t) * (P1[Y] - P3[Y]))
			{
				return(false);
			}

			return(true);

		case Y:

			s = ray.Origin[X] + *Depth * ray.Direction[X];
			t = ray.Origin[Z] + *Depth * ray.Direction[Z];

			if ((P2[X] - s) * (P2[Z] - P1[Z]) < (P2[Z] - t) * (P2[X] - P1[X]))
			{
				return(false);
			}

			if ((P3[X] - s) * (P3[Z] - P2[Z]) < (P3[Z] - t) * (P3[X] - P2[X]))
			{
				return(false);
			}

			if ((P1[X] - s) * (P1[Z] - P3[Z]) < (P1[Z] - t) * (P1[X] - P3[X]))
			{
				return(false);
			}

			return(true);

		case Z:

			s = ray.Origin[X] + *Depth * ray.Direction[X];
			t = ray.Origin[Y] + *Depth * ray.Direction[Y];

			if ((P2[X] - s) * (P2[Y] - P1[Y]) < (P2[Y] - t) * (P2[X] - P1[X]))
			{
				return(false);
			}

			if ((P3[X] - s) * (P3[Y] - P2[Y]) < (P3[Y] - t) * (P3[X] - P2[X]))
			{
				return(false);
			}

			if ((P1[X] - s) * (P1[Y] - P3[Y]) < (P1[Y] - t) * (P1[X] - P3[X]))
			{
				return(false);
			}

			return(true);
	}

	return(false);
}

/*
 *  MeshUV - By Xander Enzmann
 *
 *  Currently unused
 *
 */

const DBL BARY_VAL1 = -0.00001;
const DBL BARY_VAL2 =  1.00001;

void Mesh::MeshUV(const VECTOR P, const MESH_TRIANGLE *Triangle, UV_VECT Result) const
{
	DBL a, b, r;
	VECTOR Q, B[3], IB[3], P1, P2, P3;
	UV_VECT UV1, UV2, UV3;

	get_triangle_vertices(Triangle, P1, P2, P3);
	VSub(B[0], P2, P1);
		VSub(B[1], P3, P1);
		Assign_Vector(B[2], Data->Normals[Triangle->Normal_Ind]);

	if (!MInvers3(B, IB)) {
		// Failed to invert - that means this is a degenerate triangle
		Result[U] = P[X];
		Result[V] = P[Y];
		return;
	}

	VSub(Q, P, P1);
	VDot(a, Q, IB[0]);
	VDot(b, Q, IB[1]);

	if (a < BARY_VAL1 || b < BARY_VAL1 || a + b > BARY_VAL2)
	{
		// The use of BARY_VAL1 is an attempt to compensate for the
		//  lack of precision in the floating point numbers used in
		//  the matrices B and IB.  Since floats only have around
		//  7 digits of precision, we make sure that we allow any
		//  slop in a and b that is less than that. */
		Result[U] = P[X];
		Result[V] = P[Y];
		return;
	}

	r = 1.0f - a - b;

	get_triangle_uvcoords(Triangle, UV1, UV2, UV3);
	VScale(Q, UV1, r);
	VAddScaledEq(Q, a, UV2);
	VAddScaledEq(Q, b, UV3);

	Result[U] = Q[U];
	Result[V] = Q[V];
}


/*****************************************************************************
*
* FUNCTION
*
*   test_hit
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Test if a hit is valid and push if on the intersection depth.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

bool Mesh::test_hit(const MESH_TRIANGLE *Triangle, const Ray &OrigRay, DBL Depth, DBL len, IStack& Depth_Stack, TraceThreadData *Thread)
{
	VECTOR IPoint;
	DBL world_dist = Depth / len;

	VEvaluateRay(IPoint, OrigRay.Origin, world_dist, OrigRay.Direction);

	if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
	{
		/*
		don't bother calling MeshUV because we reclaculate it later (if needed) anyway
		UV_VECT uv;
		VECTOR P; // Object coordinates of intersection
		VEvaluateRay(P, ray.Origin, Depth, ray.Direction);

		MeshUV(P, Triangle, Mesh, uv);  

		push_entry_pointer_uv(world_dist, IPoint, uv, Object, Triangle, Depth_Stack);
		*/

		Depth_Stack->push(Intersection(world_dist, IPoint, this, Triangle));
		return(true);
	}

	return(false);
}



/*****************************************************************************
*
* FUNCTION
*
*   Init_Mesh_Triangle
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::Init_Mesh_Triangle(MESH_TRIANGLE *Triangle)
{
	Triangle->Smooth = false;
	Triangle->ThreeTex = false;
	Triangle->Dominant_Axis = 0;
	Triangle->vAxis         = 0;

	Triangle->P1 =
	Triangle->P2 =
	Triangle->P3 = -1;

	Triangle->Normal_Ind = -1;
	Triangle->Texture2 =
	Triangle->Texture3 = -1;

	Triangle->Texture = -1;

	Triangle->N1 =
	Triangle->N2 =
	Triangle->N3 = -1;

	Triangle->UV1 =
	Triangle->UV2 =
	Triangle->UV3 = -1;

	Make_Vector(Triangle->Perp, 0.0, 0.0, 0.0);

	Triangle->Distance = 0.0;
}



/*****************************************************************************
*
* FUNCTION
*
*   get_triangle_bbox
*
* INPUT
*
*   Triangle - Pointer to triangle
*   
* OUTPUT
*
*   BBox     - Bounding box
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the bounding box of a triangle.
*
* CHANGES
*
*   Sep 1994 : Creation.
*
******************************************************************************/

void Mesh::get_triangle_bbox(const MESH_TRIANGLE *Triangle, BBOX *BBox) const
{
	VECTOR P1, P2, P3;
	VECTOR Min, Max;

	get_triangle_vertices(Triangle, P1, P2, P3);

	Min[X] = min3(P1[X], P2[X], P3[X]);
	Min[Y] = min3(P1[Y], P2[Y], P3[Y]);
	Min[Z] = min3(P1[Z], P2[Z], P3[Z]);

	Max[X] = max3(P1[X], P2[X], P3[X]);
	Max[Y] = max3(P1[Y], P2[Y], P3[Y]);
	Max[Z] = max3(P1[Z], P2[Z], P3[Z]);

	Make_BBox_from_min_max(*BBox, Min, Max);
}



/*****************************************************************************
*
* FUNCTION
*
*   Build_Mesh_BBox_Tree
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Create the bounding box hierarchy.
*
* CHANGES
*
*   Feb 1995 : Creation. (Derived from the bounding slab creation code)
*
******************************************************************************/

void Mesh::Build_Mesh_BBox_Tree()
{
	int i, nElem, maxelements;
	BBOX_TREE **Triangles;

	if (!Test_Flag(this, HIERARCHY_FLAG))
	{
		return;
	}

	nElem = (int)Data->Number_Of_Triangles;

	maxelements = 2 * nElem;

	/* Now allocate an array to hold references to these elements. */

	Triangles = reinterpret_cast<BBOX_TREE **>(POV_MALLOC(maxelements*sizeof(BBOX_TREE *), "mesh bbox tree"));

	/* Init list with mesh elements. */

	for (i = 0; i < nElem; i++)
	{
		Triangles[i] = reinterpret_cast<BBOX_TREE *>(POV_MALLOC(sizeof(BBOX_TREE), "mesh bbox tree"));

		Triangles[i]->Infinite = false;
		Triangles[i]->Entries  = 0;
		Triangles[i]->Node     = reinterpret_cast<BBOX_TREE **>(&Data->Triangles[i]);

		get_triangle_bbox(&Data->Triangles[i], &Triangles[i]->BBox);
	}

	size_t maxfinitecount = 0;
	Build_BBox_Tree(&Data->Tree, nElem, Triangles, 0, NULL, maxfinitecount);

	/* Get rid of the Triangles array. */

	POV_FREE(Triangles);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_bbox_tree
*
* INPUT
*
*   Mesh     - Mesh object
*   Ray      - Current ray
*   Orig_Ray - Original, untransformed ray
*   len      - Length of the transformed ray direction
*   
* OUTPUT
*
*   Depth_Stack - Stack of intersections
*   
* RETURNS
*
*   int - true if an intersection was found
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Intersect a ray with the bounding box tree of a mesh.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

bool Mesh::intersect_bbox_tree(const Ray &ray, const Ray &Orig_Ray, DBL len, IStack& Depth_Stack, TraceThreadData *Thread)
{
	bool found;
	int i;
	DBL Best, Depth;
	const BBOX_TREE *Node, *Root;
	short OldStyle= has_inside_vector;

	/* Create the direction vectors for this ray. */
	Rayinfo rayinfo(ray);

	/* Start with an empty priority queue. */
	Thread->Mesh_Queue.QSize = 0;
	found = false;

	Best = BOUND_HUGE;

#ifdef BBOX_EXTRA_STATS
	Thread->Stats()[totalQueueResets]++;
#endif

	/* Check top node. */

	Root = Data->Tree;

	/* Set the root object infinite to avoid a test. */

	Check_And_Enqueue(Thread->Mesh_Queue, Root, &Root->BBox, &rayinfo, Thread);

	/* Check elements in the priority queue. */

	while (Thread->Mesh_Queue.QSize != 0)
	{
		Priority_Queue_Delete(Thread->Mesh_Queue, &Depth, &Node);

		/*
		 * If current intersection is larger than the best intersection found
		 * so far our task is finished, because all other bounding boxes in
		 * the priority queue are further away.
		 */

		/* NK 1999 - had to comment this out for use with CSG
		if (Depth > Best)
		{
			break;
		}
		*/
		if ( !OldStyle && Depth > Best)
			break;

		/* Check current node. */

		if (Node->Entries)
		{
			/* This is a node containing leaves to be checked. */

			for (i = 0; i < Node->Entries; i++)
				Check_And_Enqueue(Thread->Mesh_Queue, Node->Node[i], &Node->Node[i]->BBox, &rayinfo, Thread);
		}
		else
		{
			/* This is a leaf so test the contained triangle. */

			if (intersect_mesh_triangle(ray, reinterpret_cast<MESH_TRIANGLE *>(Node->Node), &Depth))
			{
				if (test_hit(reinterpret_cast<MESH_TRIANGLE *>(Node->Node), Orig_Ray, Depth, len, Depth_Stack, Thread))
				{
					found = true;

					Best = Depth;
				}
			}
		}
	}

	return(found);
}



/*****************************************************************************
*
* FUNCTION
*
*   mesh_hash
*
* INPUT
*
*   aPoint - Normal/Vertex to store
*   
* OUTPUT
*
*   Hash_Table - Normal/Vertex hash table
*   Number     - Number of normals/vertices
*   Max        - Max. number of normals/vertices
*   Elements   - List of normals/vertices
*   
* RETURNS
*
*   int - Index of normal/vertex into the normals/vertices list
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Try to locate a triangle normal/vertex in the normal/vertex list.
*   If the vertex is not found its stored in the normal/vertex list.
*
* CHANGES
*
*   Feb 1995 : Creation. (With help from Steve Anger's RAW2POV code)
*
******************************************************************************/

int Mesh::mesh_hash(HASH_TABLE **Hash_Table, int *Number, int  *Max, SNGL_VECT **Elements, const VECTOR aPoint)
{
	int hash;
	SNGL_VECT D, P;
	HASH_TABLE *p;

	Assign_Vector(P, aPoint);

	/* Get hash value. */

	hash = (unsigned)((int)(326.0*P[X])^(int)(694.7*P[Y])^(int)(1423.6*P[Z])) % HASH_SIZE;

	/* Try to find normal/vertex. */

	for (p = Hash_Table[hash]; p != NULL; p = p->Next)
	{
		VSub(D, p->P, P);

		if ((fabs(D[X]) < EPSILON) && (fabs(D[Y]) < EPSILON) && (fabs(D[Z]) < EPSILON))
		{
			break;
		}
	}

	if ((p != NULL) && (p->Index >= 0))
	{
		return(p->Index);
	}

	/* Add new normal/vertex to the list and hash table. */

	if ((*Number) >= (*Max))
	{
		if ((*Max) >= INT_MAX/2)
		{
			throw POV_EXCEPTION_STRING("Too many normals/vertices in mesh.");
		}

		(*Max) *= 2;

		(*Elements) = reinterpret_cast<SNGL_VECT *>(POV_REALLOC((*Elements), (*Max)*sizeof(SNGL_VECT), "mesh data"));
	}

	Assign_Vector((*Elements)[*Number], P);

	p = reinterpret_cast<HASH_TABLE *>(POV_MALLOC(sizeof(HASH_TABLE), "mesh data"));

	Assign_Vector(p->P, P);

	p->Index = *Number;

	p->Next = Hash_Table[hash];

	Hash_Table[hash] = p;

	return((*Number)++);
}



/*****************************************************************************
*
* FUNCTION
*
*   Mesh_Hash_Vertex
*
* INPUT
*
*   Vertex - Vertex to store
*   
* OUTPUT
*
*   Number_Of_Vertices - Number of vertices
*   Max_Vertices       - Max. number of vertices
*   Vertices           - List of vertices
*   
* RETURNS
*
*   int - Index of vertex into the vertices list
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Try to locate a triangle vertex in the vertex list.
*   If the vertex is not found its stored in the vertex list.
*
* CHANGES
*
*   Feb 1995 : Creation. (With help from Steve Anger's RAW2POV code)
*
******************************************************************************/

int Mesh::Mesh_Hash_Vertex(int *Number_Of_Vertices, int  *Max_Vertices, SNGL_VECT **Vertices, const VECTOR Vertex)
{
	return(mesh_hash(Vertex_Hash_Table, Number_Of_Vertices, Max_Vertices, Vertices, Vertex));
}



/*****************************************************************************
*
* FUNCTION
*
*   Mesh_Hash_Normal
*
* INPUT
*
*   Normal - Normal to store
*   
* OUTPUT
*
*   Number_Of_Normals - Number of normals
*   Max_Normals       - Max. number of normals
*   Normals           - List of normals
*   
* RETURNS
*
*   int - Index of normal into the normals list
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Try to locate a triangle normal in the normal list.
*   If the normal is not found its stored in the normal list.
*
* CHANGES
*
*   Feb 1995 : Creation. (With help from Steve Anger's RAW2POV code)
*
******************************************************************************/

int Mesh::Mesh_Hash_Normal(int *Number_Of_Normals, int  *Max_Normals, SNGL_VECT **Normals, const VECTOR S_Normal)
{
	return(mesh_hash(Normal_Hash_Table, Number_Of_Normals, Max_Normals, Normals, S_Normal));
}



/*****************************************************************************
*
* FUNCTION
*
*   Mesh_Hash_Texture
*
* INPUT
*
*   Texture - Texture to store
*   
* OUTPUT
*
*   Number_Of_Textures - Number of textures
*   Max_Textures       - Max. number of textures
*   Textures           - List of textures
*   
* RETURNS
*
*   int - Index of texture into the texture list
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Try to locate a texture in the texture list.
*   If the texture is not found its stored in the texture list.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

int Mesh::Mesh_Hash_Texture(int *Number_Of_Textures, int *Max_Textures, TEXTURE ***Textures, TEXTURE *Texture)
{
	int i;

	if (Texture == NULL)
	{
		return(-1);
	}

	/* Just do a linear search. */

	for (i = 0; i < *Number_Of_Textures; i++)
	{
		if ((*Textures)[i] == Texture)
		{
			break;
		}
	}

	if (i == *Number_Of_Textures)
	{
		if ((*Number_Of_Textures) >= (*Max_Textures))
		{
			if ((*Max_Textures) >= INT_MAX/2)
			{
				throw POV_EXCEPTION_STRING("Too many textures in mesh.");
			}

			(*Max_Textures) *= 2;

			(*Textures) = reinterpret_cast<TEXTURE **>(POV_REALLOC((*Textures), (*Max_Textures)*sizeof(TEXTURE *), "mesh data"));
		}

		(*Textures)[(*Number_Of_Textures)++] = Copy_Texture_Pointer(Texture);
	}

	return(i);
}

/*****************************************************************************
*
* FUNCTION
*
*   Mesh_Hash_UV
*
* INPUT
*
*   aPoint - UV vector to store
*
* OUTPUT
*
*   Hash_Table - UV vector hash table
*   Number     - Number of UV vectors
*   Max        - Max. number of UV vectors
*   Elements   - List of UV vectors
*
* RETURNS
*
*   int - Index of UV vector into the UV vector list
*
* AUTHOR
*
*   Dieter Bayer / Nathan Kopp
*
* DESCRIPTION
*
*   adapted from mesh_hash
*
* CHANGES
*
*
******************************************************************************/

int Mesh::Mesh_Hash_UV(int *Number, int *Max, UV_VECT **Elements, const UV_VECT aPoint)
{
	int hash;
	UV_VECT D, P;
	UV_HASH_TABLE *p;

	Assign_UV_Vect(P, aPoint);

	/* Get hash value. */

	hash = (unsigned)((int)(326.0*P[U])^(int)(694.7*P[V])) % HASH_SIZE;

	/* Try to find normal/vertex. */

	for (p = UV_Hash_Table[hash]; p != NULL; p = p->Next)
	{
		/* VSub(D, p->P, P); */
		D[U] = p->P[U] - P[U];
		D[V] = p->P[V] - P[V];

		if ((fabs(D[U]) < EPSILON) && (fabs(D[V]) < EPSILON))
		{
			break;
		}
	}

	if ((p != NULL) && (p->Index >= 0))
	{
		return(p->Index);
	}

	/* Add new normal/vertex to the list and hash table. */

	if ((*Number) >= (*Max))
	{
		if ((*Max) >= INT_MAX/2)
		{
			throw POV_EXCEPTION_STRING("Too many normals/vertices in mesh.");
		}

		(*Max) *= 2;

		(*Elements) = reinterpret_cast<UV_VECT *>(POV_REALLOC((*Elements), (*Max)*sizeof(UV_VECT), "mesh data"));
	}

	Assign_UV_Vect((*Elements)[*Number], P);

	p = reinterpret_cast<UV_HASH_TABLE *>(POV_MALLOC(sizeof(UV_HASH_TABLE), "mesh data"));

	Assign_UV_Vect(p->P, P);

	p->Index = *Number;

	p->Next = UV_Hash_Table[hash];

	UV_Hash_Table[hash] = p;

	return((*Number)++);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Mesh_Hash_Tables
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::Create_Mesh_Hash_Tables()
{
	int i;

	Vertex_Hash_Table = reinterpret_cast<HASH_TABLE **>(POV_MALLOC(HASH_SIZE*sizeof(HASH_TABLE *), "mesh hash table"));

	for (i = 0; i < HASH_SIZE; i++)
	{
		Vertex_Hash_Table[i] = NULL;
	}

	Normal_Hash_Table = reinterpret_cast<HASH_TABLE **>(POV_MALLOC(HASH_SIZE*sizeof(HASH_TABLE *), "mesh hash table"));

	for (i = 0; i < HASH_SIZE; i++)
	{
		Normal_Hash_Table[i] = NULL;
	}

	/* NK 1998 */
	UV_Hash_Table = reinterpret_cast<UV_HASH_TABLE **>(POV_MALLOC(HASH_SIZE*sizeof(UV_HASH_TABLE *), "mesh hash table"));

	for (i = 0; i < HASH_SIZE; i++)
	{
		UV_Hash_Table[i] = NULL;
	}
	/* NK ---- */
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Mesh_Hash_Tables
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::Destroy_Mesh_Hash_Tables()
{
	int i;
	HASH_TABLE *Temp;
	/* NK 1998 */
	UV_HASH_TABLE *UVTemp;
	/* NK ---- */

	for (i = 0; i < HASH_SIZE; i++)
	{
		while (Vertex_Hash_Table[i] != NULL)
		{
			Temp = Vertex_Hash_Table[i];

			Vertex_Hash_Table[i] = Temp->Next;

			POV_FREE(Temp);
		}
	}

	POV_FREE(Vertex_Hash_Table);

	for (i = 0; i < HASH_SIZE; i++)
	{
		while (Normal_Hash_Table[i] != NULL)
		{
			Temp = Normal_Hash_Table[i];

			Normal_Hash_Table[i] = Temp->Next;

			POV_FREE(Temp);
		}
	}

	POV_FREE(Normal_Hash_Table);

	/* NK 1998 */
	for (i = 0; i < HASH_SIZE; i++)
	{
		while (UV_Hash_Table[i] != NULL)
		{
			UVTemp = UV_Hash_Table[i];

			UV_Hash_Table[i] = UVTemp->Next;

			POV_FREE(UVTemp);
		}
	}

	POV_FREE(UV_Hash_Table);
	/* NK ---- */
}



/*****************************************************************************
*
* FUNCTION
*
*   get_triangle_vertices
*
* INPUT
*
*   Mesh     - Mesh object
*   Triangle - Triangle
*   
* OUTPUT
*   
* RETURNS
*
*   P1, P2, P3 - Vertices of the triangle
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::get_triangle_vertices(const MESH_TRIANGLE *Triangle, VECTOR P1, VECTOR P2, VECTOR P3) const
{
	Assign_Vector(P1, Data->Vertices[Triangle->P1]);
	Assign_Vector(P2, Data->Vertices[Triangle->P2]);
	Assign_Vector(P3, Data->Vertices[Triangle->P3]);
}



/*****************************************************************************
*
* FUNCTION
*
*   get_triangle_normals
*
* INPUT
*
*   Mesh     - Mesh object
*   Triangle - Triangle
*   
* OUTPUT
*   
* RETURNS
*
*   N1, N2, N3 - Normals of the triangle
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Mesh::get_triangle_normals(const MESH_TRIANGLE *Triangle, VECTOR N1, VECTOR N2, VECTOR N3) const
{
	Assign_Vector(N1, Data->Normals[Triangle->N1]);
	Assign_Vector(N2, Data->Normals[Triangle->N2]);
	Assign_Vector(N3, Data->Normals[Triangle->N3]);
}


/*****************************************************************************
*
* FUNCTION
*
*   get_triangle_uvcoords
*
* INPUT
*
*   Mesh     - Mesh object
*   Triangle - Triangle
*
* OUTPUT
*
* RETURNS
*
*   UV1, UV2, UV3 - UV coordinates of the triangle's corners
*
* AUTHOR
*
*   Nathan Kopp
*
* DESCRIPTION
*
*   adapted from get_triangle_normals
*
* CHANGES
*
******************************************************************************/

void Mesh::get_triangle_uvcoords(const MESH_TRIANGLE *Triangle, UV_VECT UV1, UV_VECT UV2, UV_VECT UV3) const
{
	Assign_UV_Vect(UV1, Data->UVCoords[Triangle->UV1]);
	Assign_UV_Vect(UV2, Data->UVCoords[Triangle->UV2]);
	Assign_UV_Vect(UV3, Data->UVCoords[Triangle->UV3]);
}


/*****************************************************************************
*
* FUNCTION
*
*   Mesh_Degenerate
*
* INPUT
*
*   P1, P2, P3 - Triangle's vertices
*   
* OUTPUT
*   
* RETURNS
*
*   int - true if degenerate
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Test if a triangle is degenerate.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

bool Mesh::Degenerate(const VECTOR P1, const VECTOR  P2, const VECTOR  P3)
{
	VECTOR V1, V2, Temp;
	DBL Length;

	VSub(V1, P1, P2);
	VSub(V2, P3, P2);

	VCross(Temp, V1, V2);

	VLength(Length, Temp);

	return(Length == 0.0);
}


/*****************************************************************************
*
* FUNCTION
*
*   Test_Mesh_Opacity
*
* INPUT
*
*   Mesh - Pointer to mesh structure
*
* OUTPUT
*
*   Mesh
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Set the opacity flag of the mesh according to the opacity
*   of the mesh's texture(s).
*
* CHANGES
*
*   Apr 1996 : Creation.
*
******************************************************************************/

void Mesh::Test_Mesh_Opacity()
{
	int i;

	/* Initialize opacity flag to the opacity of the object's texture. */

	if ((Texture == NULL) || (Test_Opacity(Texture)))
	{
		Set_Flag(this, OPAQUE_FLAG);
	}

	if (Test_Flag(this, MULTITEXTURE_FLAG))
	{
		for (i = 0; i < Number_Of_Textures; i++)
		{
			if (Textures[i] != NULL)
			{
				/* If component's texture isn't opaque the mesh is neither. */

				if (!Test_Opacity(Textures[i]))
				{
					Clear_Flag(this, OPAQUE_FLAG);
				}
			}
		}
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Mesh_UVCoord
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Nathan Kopp
*
* DESCRIPTION
*
*   computes the UV coordinates of the intersection for a mesh
*
******************************************************************************/

void Mesh::UVCoord(VECTOR Result, const Intersection *Inter, TraceThreadData *) const
{
	DBL w1, w2, w3, t1, t2;
	VECTOR vA, vB;
	VECTOR Side1, Side2;
	const MESH_TRIANGLE *Triangle;
	VECTOR P;

	if (Trans != NULL)
		MInvTransPoint(P, Inter->IPoint, Trans);
	else
		Assign_Vector(P, Inter->IPoint);

	Triangle = reinterpret_cast<const MESH_TRIANGLE *>(Inter->Pointer);

	/* ---------------- this is for P1 ---------------- */
	/* Side1 is opposite side, Side2 is an adjacent side (vector pointing away) */
	VSub(Side1, Data->Vertices[Triangle->P3], Data->Vertices[Triangle->P2]);
	VSub(Side2, Data->Vertices[Triangle->P3], Data->Vertices[Triangle->P1]);

	/* find A */
	/* A is a vector from this vertex to the intersection point */
	VSub(vA, P, Data->Vertices[Triangle->P1]);

	/* find B */
	/* B is a vector from this intersection to the opposite side (Side1) */
	/*    which is the exact length to get to the side                   */
	/* to do this we split Side2 into two components, but only keep the  */
	/*    one that's perp to Side2                                       */
	VDot(t1, Side2, Side1);
	VDot(t2, Side1, Side1);
	VScale(vB, Side1, t1/t2);
	VSubEq(vB, Side2);

	/* finding the weight is the scale part of a projection of A onto B */
	VDot(t1, vA, vB);
	VDot(t2, vB, vB);
	/* w1 = 1-fabs(t1/t2); */
	w1 = 1+t1/t2;

	/* ---------------- this is for P2 ---------------- */
	VSub(Side1, Data->Vertices[Triangle->P3], Data->Vertices[Triangle->P1]);
	VSub(Side2, Data->Vertices[Triangle->P3], Data->Vertices[Triangle->P2]);

	/* find A */
	VSub(vA, P, Data->Vertices[Triangle->P2]);

	/* find B */
	VDot(t1, Side2, Side1);
	VDot(t2, Side1, Side1);
	VScale(vB, Side1, t1/t2);
	VSubEq(vB, Side2);

	VDot(t1, vA, vB);
	VDot(t2, vB, vB);
	/* w2 = 1-fabs(t1/t2); */
	w2 = 1+t1/t2;

	/* ---------------- this is for P3 ---------------- */
	VSub(Side1, Data->Vertices[Triangle->P2], Data->Vertices[Triangle->P1]);
	VSub(Side2, Data->Vertices[Triangle->P2], Data->Vertices[Triangle->P3]);

	/* find A */
	VSub(vA, P, Data->Vertices[Triangle->P3]);

	/* find B */
	VDot(t1, Side2, Side1);
	VDot(t2, Side1, Side1);
	VScale(vB, Side1, t1/t2);
	VSubEq(vB, Side2);

	VDot(t1, vA, vB);
	VDot(t2, vB, vB);
	/* w3 = 1-fabs(t1/t2); */
	w3 = 1+t1/t2;

	Result[U] =  w1 * Data->UVCoords[Triangle->UV1][U] +
	             w2 * Data->UVCoords[Triangle->UV2][U] +
	             w3 * Data->UVCoords[Triangle->UV3][U];
	Result[V] =  w1 * Data->UVCoords[Triangle->UV1][V] +
	             w2 * Data->UVCoords[Triangle->UV2][V] +
	             w3 * Data->UVCoords[Triangle->UV3][V];
}


/*****************************************************************************
*
* FUNCTION
*
*   inside_bbox_tree
*
* INPUT
*
*   Mesh     - Mesh object
*   Ray      - Current ray
*   
* OUTPUT
*
* RETURNS
*
*   int - true if inside the object
*   
* AUTHOR
*
*   Nathan Kopp & Dieter Bayer
*   
* DESCRIPTION
*
*   Check if a point is within the bounding box tree of a mesh.
*
* CHANGES
*
*   Oct 1998 : Creation.
*
******************************************************************************/

bool Mesh::inside_bbox_tree(const Ray &ray, TraceThreadData *Thread) const
{
	int i, found;
	DBL Best, Depth;
	const BBOX_TREE *Node, *Root;

	/* Create the direction vectors for this ray. */
	Rayinfo rayinfo(ray);

	/* Start with an empty priority queue. */
	Thread->Mesh_Queue.QSize = 0;
	found = 0;

	Best = BOUND_HUGE;

#ifdef BBOX_EXTRA_STATS
	Thread->Stats()[totalQueueResets]++;
#endif

	/* Check top node. */
	Root = Data->Tree;

	/* Set the root object infinite to avoid a test. */
	Check_And_Enqueue(Thread->Mesh_Queue, Root, &Root->BBox, &rayinfo, Thread);

	/* Check elements in the priority queue. */
	while (Thread->Mesh_Queue.QSize != 0)
	{
		Priority_Queue_Delete(Thread->Mesh_Queue, &Depth, &Node);

		/* Check current node. */
		if (Node->Entries)
		{
			/* This is a node containing leaves to be checked. */
			for (i = 0; i < Node->Entries; i++)
				Check_And_Enqueue(Thread->Mesh_Queue, Node->Node[i], &Node->Node[i]->BBox, &rayinfo, Thread);
		}
		else
		{
			/* This is a leaf so test the contained triangle. */

			if (intersect_mesh_triangle(ray, reinterpret_cast<MESH_TRIANGLE *>(Node->Node), &Depth))
			{
				/* actually, this should push onto a local depth stack and
				   make sure that we don't have the same intersection point from
				   two (or three) different triangles!!!!! */
				found++;
			}
		}
	}

	/* odd number = inside, even number = outside */
	return ((found & 1) != 0);
}

void Mesh::Determine_Textures(Intersection *isect, bool hitinside, WeightedTextureVector& textures, TraceThreadData *Threaddata)
{
	const MESH_TRIANGLE *tri = reinterpret_cast<const MESH_TRIANGLE *>(isect->Pointer);

	if((Interior_Texture != NULL) && (hitinside == true)) // useful feature for checking mesh orientation and other effects [trf]
		textures.push_back(WeightedTexture(1.0, Interior_Texture));
	else if(tri->ThreeTex)
	{
		VECTOR p1, p2, p3;
		VECTOR epoint;
		COLC w1, w2, w3;
		COLC wsum;

		if(Trans != NULL)
			MInvTransPoint(epoint, isect->IPoint, Trans);
		else
			Assign_Vector(epoint, isect->IPoint);

		Assign_Vector(p1, Data->Vertices[tri->P1]);
		Assign_Vector(p2, Data->Vertices[tri->P2]);
		Assign_Vector(p3, Data->Vertices[tri->P3]);

		w1 = 1.0 - COLC(SmoothTriangle::Calculate_Smooth_T(epoint, p1, p2, p3));
		w2 = 1.0 - COLC(SmoothTriangle::Calculate_Smooth_T(epoint, p2, p3, p1));
		w3 = 1.0 - COLC(SmoothTriangle::Calculate_Smooth_T(epoint, p3, p1, p2));

		wsum = 1.0 / (w1 + w2 + w3);

		textures.push_back(WeightedTexture(w1 * wsum, Textures[tri->Texture]));
		textures.push_back(WeightedTexture(w2 * wsum, Textures[tri->Texture2]));
		textures.push_back(WeightedTexture(w3 * wsum, Textures[tri->Texture3]));
	}
	else if(tri->Texture >= 0) // TODO FIXME - make sure there always is some valid texture, also for code above! [trf]
		textures.push_back(WeightedTexture(1.0, Textures[tri->Texture]));
	else if(Texture != NULL)
		textures.push_back(WeightedTexture(1.0, Texture));
}

}
