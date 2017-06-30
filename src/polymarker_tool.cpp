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


PolymarkerTool *CreatePolymarkerTool (PolymarkerServiceJob *job_p, PolymarkerServiceData *data_p, PolymarkerToolType ptt)
{
	PolymarkerTool *tool_p = 0;

	switch (ptt)
		{
			case PTT_SYSTEM:
				tool_p = new AsyncSystemPolymarkerTool (job_p, data_p);
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


PolymarkerTool :: PolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerServiceData *data_p)
	: pt_service_data_p  (data_p), pt_service_job_p (job_p)
{
	job_p -> psj_tool_p = this;
	pt_process_id = 0;
}



PolymarkerTool :: PolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerServiceData *data_p, const json_t *root_p)
{
	pt_service_data_p = data_p;
	pt_service_job_p = job_p;
}



PolymarkerTool :: ~PolymarkerTool ()
{

}


bool PolymarkerTool :: AddToJSON (json_t *root_p)
{
	return true;
}
