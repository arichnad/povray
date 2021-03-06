/*******************************************************************************
 * boundingtask.cpp
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
 * $File: //depot/povray/smp/source/backend/bounding/boundingtask.cpp $
 * $Revision: #52 $
 * $Change: 5088 $
 * $DateTime: 2010/08/05 17:08:44 $
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

#include <set>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/math/vector.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/support/bsptree.h"
#include "backend/bounding/boundingtask.h"
#include "backend/shape/cones.h"
#include "backend/texture/texture.h"
#include "backend/texture/pigment.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

class SceneObjects : public BSPTree::Objects
{
	public:
		vector<ObjectPtr> infinite;
		vector<ObjectPtr> finite;
		unsigned int numLights;

		SceneObjects(vector<ObjectPtr>& objects)
		{
			numLights = 0;
			for(vector<ObjectPtr>::iterator i(objects.begin()); i != objects.end(); i++)
			{
				if(Test_Flag((*i), INFINITE_FLAG))
				{
					infinite.push_back(*i);
					if (((*i)->Type & LIGHT_SOURCE_OBJECT) != 0)
						numLights++;
				}
				else
					finite.push_back(*i);
			}
		}

		virtual ~SceneObjects()
		{
			// nothing to do
		}

		virtual unsigned int size() const
		{
			return finite.size();
		}

		virtual float GetMin(unsigned int axis, unsigned int i) const
		{
			return finite[i]->BBox.lowerleft[axis];
		}

		virtual float GetMax(unsigned int axis, unsigned int i) const
		{
			return (finite[i]->BBox.lowerleft[axis] + finite[i]->BBox.length[axis]);
		}
};

class BSPProgress : public BSPTree::Progress
{
	public:
		BSPProgress(RenderBackend::SceneId sid, POVMSAddress addr) :
			sceneId(sid),
			frontendAddress(addr),
			lastProgressTime(0)
		{
		}

		virtual void operator()(unsigned int nodes) const
		{
			if((timer.ElapsedRealTime() - lastProgressTime) > 1000) // update progress at most every second
			{
				POVMS_Object obj(kPOVObjectClass_BoundingProgress);
				obj.SetLong(kPOVAttrib_RealTime, timer.ElapsedRealTime());
				obj.SetLong(kPOVAttrib_CurrentNodeCount, nodes);
				RenderBackend::SendSceneOutput(sceneId, frontendAddress, kPOVMsgIdent_Progress, obj);

				Task::CurrentTaskCooperate();

				lastProgressTime = timer.ElapsedRealTime();
			}
		}
	private:
		RenderBackend::SceneId sceneId;
		POVMSAddress frontendAddress;
		Timer timer;
		mutable POV_LONG lastProgressTime;

		BSPProgress();
};

BoundingTask::BoundingTask(shared_ptr<SceneData> sd, unsigned int bt) :
	Task(new SceneThreadData(sd), boost::bind(&BoundingTask::SendFatalError, this, _1)),
	sceneData(sd),
	boundingThreshold(bt)
{
}

BoundingTask::~BoundingTask()
{
}

void BoundingTask::AppendObject(ObjectPtr p)
{
	sceneData->objects.push_back(p);
}

void BoundingTask::Run()
{
	if((sceneData->objects.size() < boundingThreshold) || (sceneData->boundingMethod == 0))
	{
		SceneObjects objects(sceneData->objects);
		sceneData->boundingMethod = 0;
		sceneData->numberOfFiniteObjects = objects.finite.size();
		sceneData->numberOfInfiniteObjects = objects.infinite.size() - objects.numLights;
		return;
	}

	switch(sceneData->boundingMethod)
	{
		case 2:
		{
			// new BSP tree code
			SceneObjects objects(sceneData->objects);
			BSPProgress progress(sceneData->sceneId, sceneData->frontendAddress);

			sceneData->objects.clear();
			sceneData->objects.insert(sceneData->objects.end(), objects.finite.begin(), objects.finite.end());
			sceneData->objects.insert(sceneData->objects.end(), objects.infinite.begin(), objects.infinite.end());
			sceneData->numberOfFiniteObjects = objects.finite.size();
			sceneData->numberOfInfiniteObjects = objects.infinite.size() - objects.numLights;
			sceneData->tree = new BSPTree(sceneData->bspMaxDepth, sceneData->bspObjectIsectCost, sceneData->bspBaseAccessCost, sceneData->bspChildAccessCost, sceneData->bspMissChance);
			sceneData->tree->build(progress, objects,
			                       sceneData->nodes, sceneData->splitNodes, sceneData->objectNodes, sceneData->emptyNodes,
			                       sceneData->maxObjects, sceneData->averageObjects, sceneData->maxDepth, sceneData->averageDepth,
			                       sceneData->aborts, sceneData->averageAborts, sceneData->averageAbortObjects, sceneData->inputFile);
			break;
		}
		case 1:
		{
			// old bounding box code
			unsigned int numberOfLightSources;

			Build_Bounding_Slabs(&(sceneData->boundingSlabs), sceneData->objects, sceneData->numberOfFiniteObjects,
			                     sceneData->numberOfInfiniteObjects, numberOfLightSources);
			break;
		}
	}
}

void BoundingTask::Stopped()
{
}

void BoundingTask::Finish()
{
	GetSceneDataPtr()->timeType = SceneThreadData::kBoundingTime;
	GetSceneDataPtr()->realTime = ConsumedRealTime();
	GetSceneDataPtr()->cpuTime = ConsumedCPUTime();
}

void BoundingTask::SendFatalError(Exception& e)
{
	// if the front-end has been told about this exception already, we don't tell it again
	if (e.frontendnotified(true))
		return;

	POVMS_Message msg(kPOVObjectClass_ControlData, kPOVMsgClass_SceneOutput, kPOVMsgIdent_Error);

	msg.SetString(kPOVAttrib_EnglishText, e.what());
	msg.SetInt(kPOVAttrib_Error, 0);
	msg.SetInt(kPOVAttrib_SceneId, sceneData->sceneId);
	msg.SetSourceAddress(sceneData->backendAddress);
	msg.SetDestinationAddress(sceneData->frontendAddress);

	POVMS_SendMessage(msg);
}

}
