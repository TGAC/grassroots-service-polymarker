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


class POLYMARKER_SERVICE_LOCAL AsyncSystemPolymarkerTool : public PolymarkerTool
{
public:
	AsyncSystemPolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p, const PolymarkerServiceData *data_p);

	AsyncSystemPolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p,  const PolymarkerServiceData *data_p, const json_t *root_p);

	virtual ~AsyncSystemPolymarkerTool ();

	virtual bool PreRun ();

	virtual bool PostRun ();

	virtual char *GetLog ();

	virtual char *GetResults (PolymarkerFormatter *formatter_p);


	virtual OperationStatus Run ();

	virtual OperationStatus GetStatus (bool update_flag);

	virtual bool ParseParameters (const ParameterSet * const param_set_p);


	virtual bool AddToJSON (json_t *root_p);

	virtual PolymarkerToolType GetToolType () const;


protected:
	const char *aspt_executable_s;
	char *aspt_command_line_args_s;

	bool CreateArgs (const char *input_s, char *output_s, char *contigs_s);
	TempFile *GetInputFile (const char *gene_id_s, const char *target_chromosome_s, const char *sequence_s);
	char *GetOutputFolder ();
	bool GetStringParameter (const ParameterSet * const params_p, const char *param_name_s, char **param_value_ss);
	void FreeCommandLineArgs ();

	bool SetExecuteable (const PolymarkerServiceData *data_p);


private:
	static uint32 SPT_NUM_ARGS;

	static const char * const ASPT_ASYNC_S;
	static const char * const ASPT_LOGFILE_S;
	static const char * const ASPT_EXECUTABLE_S;

	char *aspt_async_logfile_s;
	SystemAsyncTask *aspt_task_p;
};



#endif /* SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_SYSTEM_POLYMARKER_TOOL_HPP_ */
