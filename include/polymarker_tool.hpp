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
 * polymarker_tool.hpp
 *
 *  Created on: 6 Feb 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#ifndef SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_POLYMARKER_TOOL_HPP_
#define SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_POLYMARKER_TOOL_HPP_


#include "polymarker_service.h"
#include "parameter_set.h"
#include "service_job.h"
#include "polymarker_service_job.h"


class POLYMARKER_SERVICE_LOCAL PolymarkerTool
{
public:
	PolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerServiceData *data_p);

	PolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerServiceData *data_p, const json_t *root_p);

	virtual ~PolymarkerTool ();


	virtual bool ParseParameters (const ParameterSet * const param_set_p) = 0;

	virtual bool PreRun () = 0;

	virtual OperationStatus Run () = 0;

	virtual OperationStatus GetStatus (bool update_flag) = 0;


	/**
	 * Get the log after the PolymarkerTool has finished
	 * running.
	 *
	 * @return The results as a c-style string or 0 upon error.
	 */
	virtual char *GetLog () = 0;


protected:
	const PolymarkerServiceData *pt_service_data_p;
	PolymarkerServiceJob *pt_service_job_p;
	int32 pt_process_id;
};


#ifdef __cplusplus
extern "C"
{
#endif

PolymarkerTool *CreatePolymarkerTool (PolymarkerServiceJob *job_p, PolymarkerServiceData *data_p, PolymarkerToolType ptt);


void FreePolymarkerTool (PolymarkerTool *tool_p);

#ifdef __cplusplus
}
#endif


#endif /* SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_POLYMARKER_TOOL_HPP_ */
