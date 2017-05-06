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
 * system_polymarker_tool.cpp
 *
 *  Created on: 6 Feb 2017
 *      Author: billy
 *
 * @file
 * @brief
 */


#include "system_polymarker_tool.hpp"

#include "system_async_task.h"
#include "string_utils.h"
#include "json_util.h"
#include "alloc_failure.hpp"
#include "jobs_manager.h"


uint32 SystemPolymarkerTool :: SPT_NUM_ARGS = 8;



SystemPolymarkerTool :: SystemPolymarkerTool (PolymarkerServiceData *data_p, PolymarkerServiceJob *job_p)
	: PolymarkerTool (data_p, job_p)
{
	spt_command_line_args_s = 0;
	spt_asynchronous_flag = false;
	spt_executable_s = 0;

	json_t *conf_p = json_object_get (data_p -> psd_base_data.sd_config_p, "system");

	if (conf_p)
		{
			GetJSONBoolean (conf_p, "asynchronous", &spt_asynchronous_flag);

			data_p -> psd_executable_s = GetJSONString (conf_p, "program");
		}

	if (! (data_p -> psd_executable_s))
		{
			throw AllocFailure ("Failed to get program name for SystemPolymarkerTool");
		}
}


SystemPolymarkerTool ::  ~SystemPolymarkerTool ()
{
	FreeCommandLineArgs ();
}


void SystemPolymarkerTool :: FreeCommandLineArgs ()
{
	if (spt_command_line_args_s)
		{
			FreeCopiedString (spt_command_line_args_s);
			spt_command_line_args_s = 0;
		}
}


bool SystemPolymarkerTool :: ParseParameters (const ParameterSet * const param_set_p)
{
	bool success_flag = false;
	char *contig_s;

	if (GetStringParameter (param_set_p, PS_CONTIG_FILENAME.npt_name_s, &contig_s))
		{
			char *gene_id_s;

			if (GetStringParameter (param_set_p, PS_GENE_ID.npt_name_s, &gene_id_s))
				{
					char *target_chromosome_s;

					if (GetStringParameter (param_set_p, PS_TARGET_CHROMOSOME.npt_name_s, &target_chromosome_s))
						{
							char *sequence_s;

							if (GetStringParameter (param_set_p, PS_SEQUENCE.npt_name_s, &sequence_s))
								{
									TempFile *input_file_p = GetInputFile (gene_id_s, target_chromosome_s, sequence_s);

									if (input_file_p)
										{
											const char *input_s = input_file_p -> GetFilename ();
											char *output_s = GetOutputFolder ();

											if (output_s)
												{
													if (CreateArgs (input_s, output_s, contig_s))
														{
															success_flag = true;
														}
													else
														{
															FreeCopiedString (output_s);
														}

													if (!success_flag)
														{
															FreeCopiedString (output_s);
														}
												}

											delete input_file_p;
										}

									FreeCopiedString (sequence_s);
								}

							FreeCopiedString (target_chromosome_s);
						}

					FreeCopiedString (gene_id_s);
				}

		}		/* if (GetStringParameter (param_set_p, PS_CONTIG_FILENAME.npt_name_s, &contig_s)) */


	return success_flag;
}


bool SystemPolymarkerTool :: CreateArgs (const char *input_s, char *output_s, char *contigs_s)
{
	bool success_flag = false;
	char *command_line_args_s = ConcatenateVarargsStrings (pt_service_data_p -> psd_executable_s, " --marker_list ", input_s, " --output ", output_s, " -- contigs ", contigs_s , NULL);

	if (command_line_args_s)
		{
			if (spt_command_line_args_s)
				{
					FreeCopiedString (spt_command_line_args_s);
				}

			spt_command_line_args_s = command_line_args_s;
			success_flag = true;
		}
	else
		{

		}


	return success_flag;
}



bool SystemPolymarkerTool :: Run ()
{
	bool success_flag = false;
	ServiceJob *base_job_p = & (pt_service_job_p -> psj_base_job);

	if (spt_asynchronous_flag)
		{
			/*
			int32 process_id = CreateProcess (spt_command_line_args_s, NULL, NULL);

			if (process_id >= 0)
				{
					pt_process_id = process_id;

					AddServiceJobToJobsManager (GetJobsManager (), base_job_p -> sj_id, base_job_p);
					success_flag = true;
				}
			*/
		}
	else
		{
			int res = system (spt_command_line_args_s);

			if (res == 0)
				{
					success_flag = true;
					SetServiceJobStatus (base_job_p, OS_SUCCEEDED);
				}
			else
				{
					SetServiceJobStatus (base_job_p, OS_FAILED);
				}
		}

	return success_flag;
}


OperationStatus SystemPolymarkerTool :: GetStatus (bool update_flag)
{
	OperationStatus status;

	if (spt_asynchronous_flag)
		{
			//status = GetProcessStatus (pt_process_id);
			SetServiceJobStatus (& (pt_service_job_p -> psj_base_job), status);
		}
	else
		{
			status = GetServiceJobStatus (& (pt_service_job_p -> psj_base_job));
		}

	return status;
}


TempFile *SystemPolymarkerTool :: GetInputFile (const char *gene_id_s, const char *target_chromosome_s, const char *sequence_s)
{
	bool success_flag = false;
	TempFile *input_file_p = TempFile :: GetTempFile (pt_service_data_p -> psd_working_dir_s, pt_service_job_p -> psj_base_job.sj_id, ".input");

	if (input_file_p)
		{
			if (input_file_p -> Open ("a"))
				{
					char *line_s = ConcatenateVarargsStrings (gene_id_s, ",", target_chromosome_s, ", ", sequence_s, "\n", NULL);

					if (line_s)
						{
							if (input_file_p -> Print (line_s))
								{
									success_flag = true;

								}

							FreeCopiedString (line_s);
						}
				}

			if (!success_flag)
				{
					delete input_file_p;
					input_file_p = 0;
				}

		}

	return input_file_p;
}


char *SystemPolymarkerTool :: GetOutputFolder ()
{
	char *output_s = 0;
	char uuid_s [UUID_STRING_BUFFER_SIZE];
	char *parent_s;

	ConvertUUIDToString (pt_service_job_p -> psj_base_job.sj_id, uuid_s);

	parent_s = MakeFilename (pt_service_data_p -> psd_working_dir_s, uuid_s);

	if (parent_s)
		{
			output_s = MakeFilename (parent_s, "_output");

			FreeCopiedString (parent_s);
		}

	return output_s;
}




bool SystemPolymarkerTool :: GetStringParameter (const ParameterSet * const params_p, const char *param_name_s, char **param_value_ss)
{
	bool success_flag = false;
	SharedType value;

	if (GetParameterValueFromParameterSet (params_p, param_name_s, &value, true))
		{
			*param_value_ss = CopyToNewString (value.st_string_value_s, 0, false);

			if (*param_value_ss)
				{
					success_flag = true;
				}
		}

	return success_flag;
}


