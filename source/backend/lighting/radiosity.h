/*******************************************************************************
 * radiosity.h
 *
 * This file contains radiosity computation classes and types.
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
 * $File: //depot/povray/smp/source/backend/lighting/radiosity.h $
 * $Revision: #38 $
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

#ifndef POVRAY_BACKEND_RADIOSITY_H
#define POVRAY_BACKEND_RADIOSITY_H

#include "backend/frame.h"
#include "backend/povray.h"
#include "backend/scene/view.h"
#include "backend/scene/scene.h"
#include "backend/render/tracepixel.h"
#include "backend/render/rendertask.h"
#include "backend/support/octree.h"
#include "backend/support/randomsequences.h"
#include "backend/lighting/photons.h"
#include "backend/interior/media.h"

#include "povrayold.h" // TODO FIXME

#include <vector>

namespace pov
{

class ViewData;

#define RADIOSITY_CACHE_EXTENSION ".rca"

static const unsigned int RADIOSITY_MAX_SAMPLE_DIRECTIONS    = 1600;
// to get some more pseudo-randomness and make use of the full range of all the precomputed sample directions,
// we start each sample direction sequence at a different index than the previous one; 663 has some nice properties for this:
// - it is fairly large stride, only giving "overlap" of consecutive samples at high sample counts
// - it has no divisors in common with 1600, so that any consecutive 1600 samples will start at a different index
// - it gives the highest possible number of "secondary strides", those being -274, 115, -44, -17, -7 and 3

class SceneRadiositySettings;

// settings as effective for a particular bounce depth during a particuar trace step
struct RadiosityRecursionSettings
{
	// true "tweakables"
	unsigned int    raysPerSample;          // number of sample rays to shoot per sample
	unsigned int    reuseCount;             // number of samples required for re-use
	double          errorBoundFactor;       // factor governing spacing of samples in general
	double          minReuseFactor;         // factor governing minimum spacing of samples in creases
	double          maxReuseFactor;         // factor governing maximum spacing of samples in open areas
	double          octreeOverfillFactor;   // factor governing octree lookup performance
	unsigned int    traceLevel;             // base trace level to use for secondary rays
	double          weight;                 // base weight to use for secondary rays

	// precomputed values
	double          maxErrorBound;          // maximum error bound to be expected for sample lookup
	double          octreeAddressFactor;    // effective radius factor for filing samples in the octree
};

// settings as specified in the scene file;
// naming conventions are as per the respective scene file parameter
class SceneRadiositySettings
{
	public:

		// primary settings from the scene file

		bool    radiosityEnabled;

		double  brightness;
		long    count;
		long    directionPoolSize;
		double  errorBound;
		double  grayThreshold;
		double  lowErrorFactor;
		double  minimumReuse;
		bool    minimumReuseSet;
		double  maximumReuse;
		bool    maximumReuseSet;
		long    nearestCount;
		long    nearestCountAPT;
		int     recursionLimit;
		double  maxSample;
		double  adcBailout;
		bool    normal;
		bool    media;
		double  pretraceStart;
		double  pretraceEnd;
		bool    alwaysSample;
		bool    vainPretrace;               // whether to use full quality during pretrace even where it doesn't matter, to give the user a nice show
		float   defaultImportance;
		bool    subsurface;                 // whether to use subsurface scattering for radiosity sampling rays

		SceneRadiositySettings() {
			radiosityEnabled    = false;
			brightness          = 1.0;
			count               = 35;
			directionPoolSize   = RADIOSITY_MAX_SAMPLE_DIRECTIONS;
			errorBound          = 1.8;
			grayThreshold       = 0.0;
			lowErrorFactor      = 0.5;
			minimumReuse        = 0.015;
			minimumReuseSet     = false;
			maximumReuse        = 0.2;
			maximumReuseSet     = false;
			nearestCount        = 5;        // TODO FIXME - let's get rid of this completely
			nearestCountAPT     = 0;        // second nearest_count parameter, governing adaptive pretrace
			recursionLimit      = 2;
			maxSample           = -1.0;     // default max brightness allows any
			adcBailout          = 0.01;
			normal              = false;
			media               = false;
			pretraceStart       = 0.08;
			pretraceEnd         = 0.04;
			alwaysSample        = false;
			vainPretrace        = false;
			defaultImportance   = 1.0;
			subsurface          = false;
		}

		RadiosityRecursionSettings* GetRecursionSettings (bool final) const;
};

class RadiosityCache
{
	private:

		static const unsigned int BLOCK_POOL_UNIT_SIZE = 32;

	public:

		class BlockPool
		{
			friend class RadiosityCache;
			public:
				BlockPool();
				~BlockPool();
			protected:
				OT_BLOCK* NewBlock();
				void Save(OStream *fd);
			private:
				struct PoolUnit
				{
					PoolUnit *next;
					OT_BLOCK blocks[BLOCK_POOL_UNIT_SIZE];
					PoolUnit(PoolUnit *n) : next(n) { }
				};
				PoolUnit *head;                 // newest pool unit
				PoolUnit *savedHead;            // newest block that has been saved completely
				unsigned int nextFreeBlock;     // next free block (in *head)
				unsigned int nextUnsavedBlock;  // next unsaved block (in *savedHead predecessor)
		};

		int firstRadiosityPass;

		long ra_reuse_count;
		long ra_gather_count;

		RGBColour Gather_Total;
		long Gather_Total_Count;

		#ifdef RADSTATS
			extern long ot_blockcount;
			long ot_seenodecount;
			long ot_seeblockcount;
			long ot_doblockcount;
			long ot_dotokcount;
			long ot_lastcount;
			long ot_lowerrorcount;
		#endif

		RadiosityCache(const SceneRadiositySettings& radset);
		~RadiosityCache();

		bool Load(const Path& inputFile);
		void InitAutosave(const Path& outputFile, bool append);

		DBL FindReusableBlock(RenderStatistics& stats, DBL errorbound, const Vector3d& ipoint, const Vector3d& snormal, RGBColour& illuminance, int recursionDepth, int pretraceStep, int tileId);
		BlockPool* AcquireBlockPool();
		void AddBlock(BlockPool* pool, RenderStatistics* stats, const Vector3d& Point, const Vector3d& S_Normal, const Vector3d& To_Nearest_Surface,
		              const RGBColour& dx, const RGBColour& dy, const RGBColour& dz, const RGBColour& Illuminance,
		              DBL Harmonic_Mean_Distance, DBL Nearest_Distance, DBL Quality, int Bounce_Depth, int pretraceStep, int tileId);
		void ReleaseBlockPool(BlockPool* pool);

	private:

		struct Octree
		{
			OT_NODE *root;
			boost::mutex treeMutex;   // lock this when adding nodes to the tree
			boost::mutex blockMutex;  // lock this when adding blocks to any node of the tree

			Octree() : root(NULL) {}
		};

		vector<BlockPool*> blockPools;  // block pools ready to be re-used
		boost::mutex blockPoolsMutex;   // lock this when accessing blockPools

		Octree octree;

		OStream *ot_fd;
		boost::mutex fileMutex;         // lock this when accessing ot_fd

		RadiosityRecursionSettings* recursionSettings; // dynamically allocated array; use recursion depth as index

		void InsertBlock(OT_NODE* node, OT_BLOCK *block);
		OT_NODE *GetNode(RenderStatistics* stats, const OT_ID& id);

		static bool AverageNearBlock(OT_BLOCK *block, void *void_info);
};

class RadiosityFunction : public Trace::RadiosityFunctor
{
	public:

		static const unsigned int TILE_MAX          = OT_TILE_MAX;
		static const unsigned int PRETRACE_INVALID  = OT_PASS_INVALID;
		static const unsigned int PRETRACE_FIRST    = OT_PASS_FIRST;
		static const unsigned int PRETRACE_MAX      = OT_PASS_MAX;
		static const unsigned int FINAL_TRACE       = OT_PASS_FINAL;
		static const unsigned int DEPTH_MAX         = (OT_DEPTH_MAX < 20 ? OT_DEPTH_MAX : 20);
		static const unsigned int MAX_NEAREST_COUNT = 20;

		// initializes radiosity module from:
		//      sd      - pointer to the scene data
		//      td      - pointer to the thread-specific data
		//      rs      - the radiosity settings as parsed from the scene file
		//      rc      - the radiosity cache to retrieve previously computed samples from, and store newly computed samples in
		//      cf      - the cooperate functor (whatever that is - some thing that handles inter-thread communication?)
		//      pts     - number of the current pretrace step (PRETRACE_FIRST to PRETRACE_MAX, or FINAL_TRACE for main render)
		//      camera  - position of the camera
		RadiosityFunction(shared_ptr<SceneData> sd, TraceThreadData *td,
		                  const SceneRadiositySettings& rs, RadiosityCache& rc, Trace::CooperateFunctor& cf, bool ft, const Vector3d& camera);
		virtual ~RadiosityFunction();

		// looks up the ambient value for a certain point
		//      ipoint          - point on the surface
		//      raw_normal      - the geometry raw norml at this pont
		//      layer_normal    - texture-pertubed normal
		//      ambient_colour  - (output) the ambient color at this point
		//      weight          - the base "weight" of the traced ray (used to compare againgst ADC bailout)
		virtual void ComputeAmbient(const Vector3d& ipoint, const Vector3d& raw_normal, const Vector3d& layer_normal, RGBColour& ambient_colour, DBL weight, Trace::TraceTicket& ticket);

		// checks whether the specified recursion depth is still within the configured limits
		virtual bool CheckRadiosityTraceLevel(const Trace::TraceTicket& ticket);

		// retrieves top level statistics information to drive pretrace re-iteration
		virtual void GetTopLevelStats(long& queryCount, float& reuse);
		virtual void ResetTopLevelStats();
		virtual void BeforeTile(int id, unsigned int pts = FINAL_TRACE);
		virtual void AfterTile();

	private:

		class SampleDirectionGenerator
		{
			public:
				/// constructor
				SampleDirectionGenerator();
				/// Called before each tile
				void Reset(unsigned int samplePoolCount);
				/// Called before each sample
				void InitSequence(unsigned int& sample_count, const Vector3d& raw_normal, const Vector3d& layer_normal, bool use_raw_normal);
				/// Called to get the next sampling ray direction
				bool GetDirection(Vector3d& direction);
			protected:
				/// number of remaining directions to try
				size_t remainingDirections;
				/// whether we're using the raw surface normal instead of the pertubed normal
				bool rawNormalMode;
				/// the raw surface normal // TODO FIXME - for smooth triangles etc. this *should* be *really* raw, but it isn't!
				Vector3d rawNormal;
				/// direction we'll map the precomputed sample directions' X axis to
				Vector3d frameX;
				/// direction we'll map the precomputed sample directions' Y axis to (the effective normal vector)
				Vector3d frameY;
				/// direction we'll map the precomputed sample directions' Z axis to
				Vector3d frameZ;
				/// Generator for sampling directions
				SequentialVectorGeneratorPtr sampleDirections;
		};

		// structure to store precomputed effective parameters for each recursion depth
		struct RecursionParameters
		{
			SampleDirectionGenerator directionGenerator;    // sample generator for this recursion depth
			IntStatsIndex statsId;                          // statistics id for per-pass per-recursion statistics
			IntStatsIndex queryCountStatsId;                // statistics id for per-recursion statistics
			FPStatsIndex weightStatsId;                     // statistics id for per-recursion statistics
		};

		// The modules that do the actual computing
		// (we use our own instances for the sake of thread-safety)

		Trace trace;                        // does the main raytracing
		MediaFunction media;                // computes media effects
		PhotonGatherer photonGatherer;      // computes photon-based illumination

		// Local data

		TraceThreadData *threadData;
		RadiosityCache& radiosityCache;     // this is where we retrieve previously computed samples from, and store newly computed samples in
		RadiosityCache::BlockPool* cacheBlockPool;
		DBL errorBound;                     // the error_bound setting
		bool isFinalTrace;
		unsigned int pretraceStep;
		Vector3d cameraPosition;
		const SceneRadiositySettings&       settings;
		const RadiosityRecursionSettings*   recursionSettings;      // dynamically allocated array; use recursion depth as index
		RecursionParameters*                recursionParameters;    // dynamically allocated array; use recursion depth as index
		long topLevelQueryCount;
		float topLevelReuse;
		int tileId;

		double GatherLight(const Vector3d& IPoint, const Vector3d& Raw_Normal, const Vector3d& LayNormal, RGBColour& Illuminance, Trace::TraceTicket& ticket);
};

} // end of namespace

#endif // POVRAY_BACKEND_RADIOSITY_H
