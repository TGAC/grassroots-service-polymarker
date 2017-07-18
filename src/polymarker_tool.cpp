/*
** Copyright 2014-2016 The Earlham Institute
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/**
 * polymarker_tool.cpp
 *
 *  Created on: 6 Feb 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#include "polymarker_tool.hpp"
#include "async_system_polymarker_tool.hpp"
#include "streams.h"



PolymarkerTool *CreatePolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p, PolymarkerServiceData *data_p)
{
	PolymarkerTool *tool_p = 0;

	switch (data_p -> psd_tool_type)
		{
			case PTT_SYSTEM:
				try
					{
						tool_p = new AsyncSystemPolymarkerTool (job_p, seq_p, data_p);
					}
				catch (std :: bad_alloc &ex_r)
					{
						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate AsyncSystemPolymarkerTool, \"%s\"", ex_r.what ());
					}
				break;

			case PTT_WEB:
			default:
				break;
		}

	return tool_p;
}


void FreePolymarkerTool (PolymarkerTool *tool_p)
{
	delete tool_p;
}


OperationStatus RunPolymarkerTool (PolymarkerTool *tool_p)
{
	OperationStatus status = OS_IDLE;

	if (tool_p -> PreRun  ())
		{
			status = tool_p -> Run ();

			tool_p -> PostRun  ();
		}
	else
		{
			status = OS_FAILED_TO_START;
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to prepare run of PolymarkerTool %s", tool_p ->  GetName ());
		}

	return status;
}




PolymarkerTool :: PolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p, const PolymarkerServiceData *data_p)
	:  pt_service_job_p (job_p), pt_seq_p (seq_p), pt_service_data_p  (data_p)
{
	job_p -> psj_tool_p = this;
	pt_process_id = 0;
}



PolymarkerTool :: PolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p, const PolymarkerServiceData *data_p, const json_t *root_p)
{
	pt_service_data_p = data_p;
	pt_seq_p = seq_p;
	pt_service_job_p = job_p;
}



PolymarkerTool :: ~PolymarkerTool ()
{

}



const char *PolymarkerTool :: GetName ()
{
	return (pt_seq_p ? pt_seq_p -> ps_name_s : NULL);
}


bool PolymarkerTool :: AddToJSON (json_t *root_p)
{
	return true;
}
