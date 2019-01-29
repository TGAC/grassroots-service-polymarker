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
 * async_system_Polymarker_tool.cpp
 *
 *  Created on: 9 Mar 2017
 *      Author: billy
 *
 * @file
 * @brief
 */


#include "async_system_polymarker_tool.hpp"
#include "polymarker_service_job.h"
#include "polymarker_utils.h"

#include "string_utils.h"
#include "jobs_manager.h"
#include "json_util.h"
#include "system_async_task.h"
#include "streams.h"


#ifdef _DEBUG
	#define ASYNC_SYSTEM_POLYMARKER_TOOL_DEBUG (STM_LEVEL_FINE)
#else
	#define ASYNC_SYSTEM_POLYMARKER_TOOL_DEBUG (STM_LEVEL_NONE)
#endif

const char * const AsyncSystemPolymarkerTool :: ASPT_ASYNC_S = "async";

const char * const AsyncSystemPolymarkerTool :: ASPT_LOGFILE_S = "logfile";


const char * const AsyncSystemPolymarkerTool :: ASPT_EXECUTABLE_S = "executable";


static bool UpdateAsyncPolymarkerServiceJob (struct ServiceJob *job_p);



AsyncSystemPolymarkerTool :: AsyncSystemPolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p, const PolymarkerServiceData *data_p)
: PolymarkerTool (job_p, seq_p, data_p),
	aspt_executable_s (0),
	aspt_command_line_args_s (0),
	aspt_async_logfile_s (0)
{
	bool alloc_flag = false;
	const char *program_name_s = 0;
	const char *name_s = 0;

	SetServiceJobUpdateFunction (& (job_p -> psj_base_job), UpdateAsyncPolymarkerServiceJob);

	if (SetExecutable (data_p))
		{
			aspt_task_p = AllocateSystemAsyncTask (& (job_p -> psj_base_job), name_s, data_p -> psd_task_manager_p, true, program_name_s, PolymarkerServiceJobCompleted);

			if (aspt_task_p)
				{
					alloc_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate SystemAsyncTask for AsyncSystemPolymarkerTool");
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set executable for AsyncSystemPolymarkerTool");
		}

	if (!alloc_flag)
		{
			throw std :: bad_alloc ();
		}
}


AsyncSystemPolymarkerTool :: ~AsyncSystemPolymarkerTool ()
{
	if (aspt_command_line_args_s)
		{
			FreeCopiedString (aspt_command_line_args_s);
		}

	FreeSystemAsyncTask (aspt_task_p);
}


AsyncSystemPolymarkerTool :: AsyncSystemPolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p,  const PolymarkerServiceData *data_p, const json_t *root_p)
	: PolymarkerTool (job_p, seq_p, data_p, root_p),
		aspt_executable_s (0),
		aspt_command_line_args_s (0),
		aspt_task_p (0)
{
	bool alloc_flag = false;

	if (SetExecutable (data_p))
		{
			char *name_s = NULL;
			bool continue_flag = true;
			const char *value_s = GetJSONString (root_p, AsyncSystemPolymarkerTool :: ASPT_LOGFILE_S);

			if (value_s)
				{
					aspt_async_logfile_s = CopyToNewString (value_s, 0, false);

					if (!aspt_async_logfile_s)
						{
							continue_flag = false;
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set AsyncSystemPolymarkerTool to \"%s\"", value_s);
						}

				}		/* if (value_s) */
			else
				{
					aspt_async_logfile_s = NULL;
				}

			if (continue_flag)
				{
					aspt_task_p = AllocateSystemAsyncTask (& (job_p -> psj_base_job), name_s, data_p -> psd_task_manager_p, true, NULL, PolymarkerServiceJobCompleted);

					if (aspt_task_p)
						{
							alloc_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate SystemAsyncTask for AsyncSystemPolymarkerTool");
						}

				}

			if (!alloc_flag)
				{
					FreeCopiedString (aspt_async_logfile_s);
				}

		}		/* if (SetExecutable (data_p)) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set executable for AsyncSystemPolymarkerTool");
		}


	if (!alloc_flag)
		{
			throw std :: bad_alloc ();
		}
}


bool AsyncSystemPolymarkerTool :: SetExecutable (const PolymarkerServiceData *data_p)
{
	bool success_flag = false;
	const json_t *polymarker_config_p = data_p -> psd_base_data.sd_config_p;

	if (polymarker_config_p)
		{
			const char *exe_s = GetJSONString (polymarker_config_p, AsyncSystemPolymarkerTool :: ASPT_EXECUTABLE_S);

			if (exe_s)
				{
					aspt_executable_s = exe_s;
					success_flag = true;
				}
		}

	return success_flag;
}


PolymarkerToolType AsyncSystemPolymarkerTool :: GetToolType () const
{
	return PTT_SYSTEM;
}


bool AsyncSystemPolymarkerTool :: ParseParameters (const ParameterSet * const param_set_p)
{
	bool success_flag = false;
	char uuid_s [UUID_STRING_BUFFER_SIZE];

	/*
	 *  bin/polymarker.rb --contigs ~/Applications/grassroots-0/grassroots/extras/blast/databases/IWGSC_CSS_all_scaff_v1.fa --marker_list test/data/billy_primer_design_test.csv --output polymarker_out/
	 *
	 */

	ConvertUUIDToString (pt_service_job_p -> psj_base_job.sj_id, uuid_s);

	pt_job_dir_s = MakeFilename (pt_service_data_p -> psd_working_dir_s, uuid_s);

	if (pt_job_dir_s)
		{
			if (EnsureDirectoryExists (pt_job_dir_s))
				{
					ByteBuffer *buffer_p = AllocateByteBuffer (1024);

					if (buffer_p)
						{
							if (AppendStringsToByteBuffer (buffer_p, aspt_executable_s, " --contigs ", pt_seq_p -> ps_fasta_filename_s, " --output ", pt_job_dir_s, " --aligner ", pt_service_data_p -> psd_aligner_s, NULL))
								{
									char *markers_filename_s = MakeFilename (pt_job_dir_s, "markers_list");

									if (markers_filename_s)
										{
											if (CreateAndAddMarkerListFile (markers_filename_s, param_set_p, buffer_p))
												{
													char *prefs_file_s = WritePrimer3Config (param_set_p, pt_job_dir_s, pt_service_data_p);

													if (prefs_file_s)
														{
															if (AppendStringsToByteBuffer (buffer_p, " --primer_3_preferences ", prefs_file_s, NULL))
																{
																	aspt_command_line_args_s = DetachByteBufferData (buffer_p);
																	success_flag = true;
																}

															FreeCopiedString (prefs_file_s);
														}
												}		/* if (CreateMarkerListFile (markers_filename_s, param_set_p)) */

											FreeCopiedString (markers_filename_s);
										}		/* if (markers_filename_s) */

								}

							if (success_flag)
								{
									aspt_command_line_args_s = DetachByteBufferData (buffer_p);
								}
							else
								{
									FreeByteBuffer (buffer_p);
								}

						}		/* if (buffer_p) */


				}		/* if (EnsureDirectoryExists (pt_job_dir_s)) */

		}		/* if (pt_job_dir_s) */

	return success_flag;
}


bool AsyncSystemPolymarkerTool :: PreRun ()
{
	bool success_flag = PolymarkerTool :: PreRun ();
	OperationStatus status;

	if (success_flag)
		{
			status = OS_STARTED;
		}
	else
		{
			status = OS_FAILED_TO_START;
		}

	SetServiceJobStatus (& (pt_service_job_p -> psj_base_job), status);

	return success_flag;
}


bool AsyncSystemPolymarkerTool :: PostRun ()
{
	return true;
}


OperationStatus AsyncSystemPolymarkerTool :: Run ()
{
	OperationStatus status = OS_IDLE;
	char uuid_s [UUID_STRING_BUFFER_SIZE];
	ServiceJob *base_job_p = & (pt_service_job_p -> psj_base_job);

	ConvertUUIDToString (base_job_p -> sj_id, uuid_s);

	SetServiceJobStatus (base_job_p, status);

	if (aspt_command_line_args_s)
		{
			if (SetSystemAsyncTaskCommand	(aspt_task_p, aspt_command_line_args_s))
				{
					JobsManager *manager_p = GetJobsManager ();

					#if ASYNC_SYSTEM_POLYMARKER_TOOL_DEBUG >= STM_LEVEL_FINE
					PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "About to run SystemPolymarkerTool with \"%s\"", aspt_command_line_args_s);
					#endif

					if (AddServiceJobToJobsManager (manager_p, base_job_p -> sj_id, base_job_p))
						{
							status = OS_PENDING;
							SetServiceJobStatus (base_job_p, status);

							if (RunSystemAsyncTask (aspt_task_p))
								{
									/*
									 * The ServiceJob should now only be writeable by the SystemAsyncTask that it is running under.
									 */
									status = OS_STARTED;
								}
							else
								{
									status = OS_FAILED_TO_START;
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to run async task for uuid %s", uuid_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add Polymarker Service Job \"%s\" to jobs manager", uuid_s);
						}

					#if ASYNC_SYSTEM_POLYMARKER_TOOL_DEBUG >= STM_LEVEL_FINE
					PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Created async task for uuid %s", uuid_s);
					#endif
				}
			else
				{
					SetServiceJobStatus (base_job_p, status);
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set command \"%s\" for uuid \"%s\"", aspt_command_line_args_s, uuid_s);
				}


			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "\"%s\" returned %d", aspt_command_line_args_s, status);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get command to run for uuid \"%s\"", uuid_s);
		}

	if ((status == OS_ERROR) || (status == OS_FAILED_TO_START) || (status == OS_FAILED))
		{
			char *log_s = GetLog ();

			SetServiceJobStatus (base_job_p, status);

			if (log_s)
				{
					if (!AddErrorToServiceJob (base_job_p, ERROR_S, log_s))
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add error \"%s\" to service job");
						}

					FreeCopiedString (log_s);
				}		/* if (log_s) */
		}

	return status;
}



bool AsyncSystemPolymarkerTool :: AddToJSON (json_t *root_p)
{
	bool success_flag = PolymarkerTool :: AddToJSON (root_p);

	if (success_flag)
		{
			if (json_object_set_new (root_p, AsyncSystemPolymarkerTool :: ASPT_ASYNC_S, json_true ()) == 0)
				{
					if (aspt_async_logfile_s)
						{
							if (json_object_set_new (root_p, AsyncSystemPolymarkerTool :: ASPT_LOGFILE_S, json_string (aspt_async_logfile_s)) != 0)
								{
									success_flag = false;
								}
						}
				}
			else
				{
					success_flag = false;
				}


		}		/* if (success_flag) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AsyncSystemPolymarkerTool :: AddToJSON failed");
		}

	return success_flag;
}



OperationStatus AsyncSystemPolymarkerTool :: GetStatus (bool update_flag)
{
	OperationStatus status = GetCachedServiceJobStatus (& (pt_service_job_p -> psj_base_job));

/*
	if (update_flag)
		{
			JobsManager *manager_p = GetJobsManager ();
			ServiceJob *job_p = GetServiceJobFromJobsManager (manager_p, bt_job_p -> psj_base_job.sj_id);

			if (job_p)
				{
					status = job_p -> sj_status;
					SetServiceJobStatus (& (bt_job_p -> psj_base_job), status);
				}

			#if ASYNC_SYSTEM_POLYMARKER_TOOL_DEBUG >= STM_LEVEL_FINE
				{
					char uuid_s [UUID_STRING_BUFFER_SIZE];

					ConvertUUIDToString (bt_job_p -> psj_base_job.sj_id, uuid_s);
					PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Status " INT32_FMT " for uuid %s", status, uuid_s);
				}
			#endif


			if ((status == OS_SUCCEEDED) || (status == OS_PARTIALLY_SUCCEEDED) || (status == OS_FINISHED))
				{
					if (bt_job_p -> psj_base_job.sj_result_p == NULL)
						{
							if (!DeterminePolymarkerResult (bt_job_p))
								{

								}
						}
				}
		}
	else
		{
			status = GetCachedServiceJobStatus (& (bt_job_p -> psj_base_job));
		}
*/

	return status;
}




char *AsyncSystemPolymarkerTool :: GetResults (PolymarkerFormatter *formatter_p)
{
	char *results_s = 0;
	char *primers_filename_s = MakeFilename (pt_job_dir_s, "primers.csv");

	if (primers_filename_s)
		{
			char *primers_results_s = GetFileContentsAsStringByFilename (primers_filename_s);

			if (primers_results_s)
				{

				}

			FreeCopiedString (primers_filename_s);
		}		/* if (primers_filename_s) */


	return results_s;
}


char *AsyncSystemPolymarkerTool :: GetLog ()
{
	return 0;
}


static bool UpdateAsyncPolymarkerServiceJob (struct ServiceJob *job_p)
{
	PolymarkerServiceJob *polymarker_job_p = reinterpret_cast <PolymarkerServiceJob *> (job_p);
	AsyncSystemPolymarkerTool *tool_p = static_cast <AsyncSystemPolymarkerTool *> (polymarker_job_p -> psj_tool_p);

	tool_p -> GetStatus (true);

	return true;
}
