/*******************************************************************************
 * bcyl.cpp
 *
 * This file contains all functions for bounding
 * cylinders used by lathe and sor objects.
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
 * $File: //depot/povray/smp/source/backend/bounding/bcyl.cpp $
 * $Revision: #15 $
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

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/bounding/bcyl.h"
#include "backend/math/vector.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/



/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Static functions
******************************************************************************/

static int  intersect_thick_cylinder (const BCYL *BCyl, const BCYL_INT *rint, const BCYL_INT *hint, const BCYL_ENTRY *Entry, DBL *dist);
static void insert_hit (const BCYL_INT *Element, BCYL_INT *intervals, int *cnt);
static void intersect_bound_elements (const BCYL *BCyl, BCYL_INT *rint, BCYL_INT *hint, const VECTOR P, const VECTOR D);


/*****************************************************************************
* Local variables
******************************************************************************/



/*****************************************************************************
*
* FUNCTION
*
*   intersect_thick_cylinder
*
* INPUT
*
*   BCyl - Pointer to lathe structure
*   Entry - Segment whos bounding cylinder to intersect
*   dist  - List of sorted intersection depths
*
* OUTPUT
*
* RETURNS
*
*   int - number of intersections found
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Find all intersections of the current ray with the bounding
*   cylinder of the given segment. The intersection tests are
*   done in intersect_bound_elements() and the results stored
*   in the lathe structure are evaluated here.
*
* CHANGES
*
*   Oct 1996 : Creation.
*
******************************************************************************/

static int intersect_thick_cylinder(const BCYL *BCyl, const BCYL_INT *rint, const BCYL_INT *hint, const BCYL_ENTRY *Entry, DBL *dist)
{
	int i, j, n;
	DBL k, r, h;

	n = 0;

	/* Intersect ray with the cap-plane. */

	if (hint[Entry->h2].n)
	{
		r = hint[Entry->h2].w[0];

		if ((r >= BCyl->radius[Entry->r1]) &&
		    (r <= BCyl->radius[Entry->r2]))
		{
			dist[n++] = hint[Entry->h2].d[0];
		}
	}

	/* Intersect ray with the base-plane. */

	if (hint[Entry->h1].n)
	{
		r = hint[Entry->h1].w[0];

		if ((r >= BCyl->radius[Entry->r1]) &&
		    (r <= BCyl->radius[Entry->r2]))
		{
			dist[n++] = hint[Entry->h1].d[0];
		}
	}

	/* Intersect with inner cylinder. */

	if (rint[Entry->r1].n)
	{
		h = rint[Entry->r1].w[0];

		if ((h >= BCyl->height[Entry->h1]) &&
		    (h <= BCyl->height[Entry->h2]))
		{
			dist[n++] = rint[Entry->r1].d[0];
		}

		h = rint[Entry->r1].w[1];

		if ((h >= BCyl->height[Entry->h1]) &&
		    (h <= BCyl->height[Entry->h2]))
		{
			dist[n++] = rint[Entry->r1].d[1];
		}
	}

	/* Intersect with outer cylinder. */

	if (rint[Entry->r2].n)
	{
		h = rint[Entry->r2].w[0];

		if ((h >= BCyl->height[Entry->h1]) &&
		    (h <= BCyl->height[Entry->h2]))
		{
			dist[n++] = rint[Entry->r2].d[0];
		}

		h = rint[Entry->r2].w[1];

		if ((h >= BCyl->height[Entry->h1]) &&
		    (h <= BCyl->height[Entry->h2]))
		{
			dist[n++] = rint[Entry->r2].d[1];
		}
	}

	/* Sort intersections. */

	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n - i - 1; j++)
		{
			if (dist[j] > dist[j+1])
			{
				k         = dist[j];
				dist[j]   = dist[j+1];
				dist[j+1] = k;
			}
		}
	}

	return(n);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_bound_elements
*
* INPUT
*
*   BCyl - Pointer to lathe structure
*   P, D  - Current ray
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
*   Intersect all bounding discs and cylinders and store
*   the intersections found in the lathe structure.
*
*   By intersecting all different discs and cylinders once
*   we avoid testing the same cylinders and discs more than
*   once. This happened when we tested one bounding cylinder
*   after the other.
*
* CHANGES
*
*   Oct 1996 : Creation.
*
******************************************************************************/

static void intersect_bound_elements(const BCYL *BCyl, BCYL_INT *rint, BCYL_INT *hint, const VECTOR P, const VECTOR  D)
{
	int i;
	DBL a, b, bb, b2, c, d, k;

	/* Init constants. */

	a = D[X] * D[X] + D[Z] * D[Z];

	b = P[X] * D[X] + P[Z] * D[Z];

	bb = b * b;

	b2 = 2.0 * b;

	c = P[X] * P[X] + P[Z] * P[Z];

	/* Intersect all rings. */

	if ((D[Y] < -EPSILON) || (D[Y] > EPSILON))
	{
		for (i = 0; i < BCyl->nheight; i++)
		{
			k = (BCyl->height[i] - P[Y]) / D[Y];

			hint[i].n = 1;

			hint[i].d[0] = k;

			hint[i].w[0] = k * (a * k + b2) + c;
		}
	}
	else
	{
		for (i = 0; i < BCyl->nheight; i++)
		{
			hint[i].n = 0;
		}
	}

	/* Intersect all cylinders. */

	for (i = 0; i < BCyl->nradius; i++)
	{
		rint[i].n = 0;

		if (BCyl->radius[i] > EPSILON)
		{
			d = bb - a * (c - BCyl->radius[i]);

			if (d > 0.0)
			{
				d = sqrt(d);

				k = (-b + d) / a;

				rint[i].n = 2;

				rint[i].d[0] = k;

				rint[i].w[0] = P[Y] + k * D[Y];

				k = (-b - d) / a;

				rint[i].d[1] = k;

				rint[i].w[1] = P[Y] + k * D[Y];
			}
		}
	}
}

/*****************************************************************************
*
* FUNCTION
*
*   insert_hit
*
* INPUT
*
*   element   - Intersection to insert
*   intervals - List of intervals
*   cnt       - Number of elements in the list
*
* OUTPUT
*
*   intervals, cnt
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Insert an intersection into the depth sorted intersection list.
*
* CHANGES
*
*   Oct 1996 : Creation.
*
******************************************************************************/

static void insert_hit(const BCYL_INT *element, BCYL_INT *intervals, int *cnt)
{
	int k;

	intervals[*cnt] = *element;

	for (k = 0; element->d[0] > intervals[k].d[0]; k++);

	if (k < *cnt)
	{
		// Note: this data structure may be worthwhile optimizing
		POV_MEMMOVE(&intervals[k+1], &intervals[k], (*cnt-k)*sizeof(BCYL_INT));

		intervals[k] = *element;
	}

	(*cnt)++;
}



/*****************************************************************************
*
* FUNCTION
*
*   Intersect_All_Bounds
*
* INPUT
*
*   BCyl     - Pointer to lathe structure
*   P, D      - Current ray
*   intervals - List of intervals
*   cnt       - Number of elements in the list
*
* OUTPUT
*
*   intervals, cnt
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Intersect given ray with all bounding cylinders of the given lathe
*   and return a sorted list of intersection depths and segments hit.
*
* CHANGES
*
*   Oct 1996 : Creation.
*
******************************************************************************/

int Intersect_BCyl(const BCYL *BCyl, BCYL_INT *intervals, BCYL_INT *rint, BCYL_INT *hint, const VECTOR P, const VECTOR  D)
{
	int i, cnt;
	DBL dist[8];
	BCYL_INT Inter;
	BCYL_ENTRY *Entry;

	cnt = 0;

	Inter.d[1] = 0.0;

	/* Intersect all cylinder and plane elements. */

	intersect_bound_elements(BCyl, rint, hint, P, D);

	/* Intersect all spline segments. */
	for (i = 0; i < BCyl->number; i++)
	{
		Entry = &BCyl->entry[i];

		switch (intersect_thick_cylinder(BCyl, rint, hint, Entry, dist))
		{
			case 0:
				break;

			case 2:

				if (dist[0] > EPSILON)
				{
					Inter.d[0] = dist[0];
					Inter.n    = i;

					insert_hit(&Inter, intervals, &cnt);
				}
				else
				{
					if (dist[1] > EPSILON)
					{
						Inter.d[0] = 0.0;
						Inter.n    = i;

						insert_hit(&Inter, intervals, &cnt);
					}
				}

				break;

			case 4:

				if (dist[0] > EPSILON)
				{
					Inter.d[0] = dist[0];
					Inter.n    = i;

					insert_hit(&Inter, intervals, &cnt);
				}
				else
				{
					if (dist[1] > EPSILON)
					{
						Inter.d[0] = 0.0;
						Inter.n    = i;

						insert_hit(&Inter, intervals, &cnt);
					}
					else
					{
						if (dist[2] > EPSILON)
						{
							Inter.d[0] = dist[2];
							Inter.n    = i;

							insert_hit(&Inter, intervals, &cnt);
						}
						else
						{
							if (dist[3] > EPSILON)
							{
								Inter.d[0] = 0.0;
								Inter.n    = i;

								insert_hit(&Inter, intervals, &cnt);
							}
						}
					}
				}

				break;

			default:

				/*
				 * We weren't able to find an even number of intersections. Thus
				 * we can't tell where the ray enters and leaves the bounding
				 * cylinder. To avoid problems we assume that the ray is always
				 * inside the cylinder in that case.
				 */

				Inter.d[0] = dist[0];
				Inter.n    = i;

				insert_hit(&Inter, intervals, &cnt);

				break;
		}
	}

	return(cnt);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_BCyl
*
* INPUT
*
*   number - number of cylindrical segments
*   r1, r2 - list of segment radii
*   h1, h2 - list of segment heights
*
* OUTPUT
*
* RETURNS
*
*   BCYL * - created bounding cylinder.
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Create a bounding cylinder data structure from the given
*   radii and heights.
*
* CHANGES
*
*   Oct 1996 : Creation.
*
******************************************************************************/

BCYL *Create_BCyl(int number, const DBL *tmp_r1, const DBL *tmp_r2, const DBL *tmp_h1, const DBL *tmp_h2)
{
	int i, j, nr, nh;
	int *tmp_r1_index;
	int *tmp_r2_index;
	int *tmp_h1_index;
	int *tmp_h2_index;
	DBL *tmp_radius;
	DBL *tmp_height;
	BCYL *bcyl;

	/* Allocate bounding cylinder. */

	bcyl = (BCYL *)POV_MALLOC(sizeof(BCYL), "bounding cylinder");

	/* Allocate entries. */

	bcyl->number = number;

	bcyl->entry = (BCYL_ENTRY *)POV_MALLOC(bcyl->number*sizeof(BCYL_ENTRY), "bounding cylinder data");

	/* Allocate temporary lists. */

	tmp_r1_index = (int *)POV_MALLOC(bcyl->number * sizeof(int), "temp lathe data");
	tmp_r2_index = (int *)POV_MALLOC(bcyl->number * sizeof(int), "temp lathe data");
	tmp_h1_index = (int *)POV_MALLOC(bcyl->number * sizeof(int), "temp lathe data");
	tmp_h2_index = (int *)POV_MALLOC(bcyl->number * sizeof(int), "temp lathe data");

	tmp_radius = (DBL *)POV_MALLOC(2 * bcyl->number * sizeof(DBL), "temp lathe data");
	tmp_height = (DBL *)POV_MALLOC(2 * bcyl->number * sizeof(DBL), "temp lathe data");

	/* Get different bounding radii and heights. */

	nr = 0;
	nh = 0;

	for (i = 0; i < bcyl->number; i++)
	{
		tmp_r1_index[i] = -1;
		tmp_r2_index[i] = -1;
		tmp_h1_index[i] = -1;
		tmp_h2_index[i] = -1;

		for (j = 0; j < nr; j++)
		{
			if (tmp_r1[i] == tmp_radius[j])
			{
				break;
			}
		}

		if (j == nr)
		{
			tmp_radius[nr++] = tmp_r1[i];
		}

		tmp_r1_index[i] = j;

		for (j = 0; j < nr; j++)
		{
			if (tmp_r2[i] == tmp_radius[j])
			{
				break;
			}
		}

		if (j == nr)
		{
			tmp_radius[nr++] = tmp_r2[i];
		}

		tmp_r2_index[i] = j;

		for (j = 0; j < nh; j++)
		{
			if (tmp_h1[i] == tmp_height[j])
			{
				break;
			}
		}

		if (j == nh)
		{
			tmp_height[nh++] = tmp_h1[i];
		}

		tmp_h1_index[i] = j;

		for (j = 0; j < nh; j++)
		{
			if (tmp_h2[i] == tmp_height[j])
			{
				break;
			}
		}

		if (j == nh)
		{
			tmp_height[nh++] = tmp_h2[i];
		}

		tmp_h2_index[i] = j;
	}

	/* Copy lists into the lathe. */

	bcyl->radius = (DBL *)POV_MALLOC(nr * sizeof(DBL), "bounding cylinder data");
	bcyl->height = (DBL *)POV_MALLOC(nh * sizeof(DBL), "bounding cylinder data");

	for (i = 0; i < nr; i++)
	{
		bcyl->radius[i] = Sqr(tmp_radius[i]);
	}

	for (i = 0; i < nh; i++)
	{
		bcyl->height[i] = tmp_height[i];
	}

	/* Assign height and radius indices. */

	bcyl->nradius = nr;
	bcyl->nheight = nh;

	for (i = 0; i < bcyl->number; i++)
	{
		bcyl->entry[i].r1 = tmp_r1_index[i];
		bcyl->entry[i].r2 = tmp_r2_index[i];
		bcyl->entry[i].h1 = tmp_h1_index[i];
		bcyl->entry[i].h2 = tmp_h2_index[i];
	}

/*
	fprintf(stderr, "number of different radii   = %d\n", nr);
	fprintf(stderr, "number of different heights = %d\n", nh);
*/

	/* Get rid of temp. memory. */

	POV_FREE(tmp_height);
	POV_FREE(tmp_radius);
	POV_FREE(tmp_h2_index);
	POV_FREE(tmp_h1_index);
	POV_FREE(tmp_r2_index);
	POV_FREE(tmp_r1_index);

	return(bcyl);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_BCyl
*
* INPUT
*
*   BCyl - bounding cylinder
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
*   Destroy a bounding cylinder.
*
* CHANGES
*
*   Oct 1996 : Creation.
*
******************************************************************************/

void Destroy_BCyl(BCYL *BCyl)
{
	POV_FREE(BCyl->entry);

	POV_FREE(BCyl->radius);

	POV_FREE(BCyl->height);

	POV_FREE(BCyl);
}

}
