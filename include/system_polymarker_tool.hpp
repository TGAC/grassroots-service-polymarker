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
 * system_polymarker_tool.hpp
 *
 *  Created on: 6 Feb 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#ifndef SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_SYSTEM_POLYMARKER_TOOL_HPP_
#define SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_SYSTEM_POLYMARKER_TOOL_HPP_

#include "polymarker_tool.hpp"
#include "temp_file.hpp"


class POLYMARKER_SERVICE_LOCAL SystemPolymarkerTool : public PolymarkerTool
{
public:
	SystemPolymarkerTool (const PolymarkerServiceData *data_p, PolymarkerServiceJob *job_p);
	virtual ~SystemPolymarkerTool ();

	virtual OperationStatus Run ();

	virtual OperationStatus GetStatus (bool update_flag);

	virtual bool ParseParameters (const ParameterSet * const param_set_p);

protected:
	char *spt_executable_s;
	char *spt_command_line_args_s;
	bool spt_asynchronous_flag;

	bool CreateArgs (const char *input_s, char *output_s, char *contigs_s);
	TempFile *GetInputFile (const char *gene_id_s, const char *target_chromosome_s, const char *sequence_s);
	char *GetOutputFolder ();
	bool GetStringParameter (const ParameterSet * const params_p, const char *param_name_s, char **param_value_ss);
	void FreeCommandLineArgs ();

private:
	static uint32 SPT_NUM_ARGS;
};



#endif /* SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_SYSTEM_POLYMARKER_TOOL_HPP_ */
