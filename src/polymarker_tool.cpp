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
#include "string_utils.h"

const char * const PolymarkerTool :: PT_JOB_DIR_S = "job_dir";

const char * const PolymarkerTool :: PT_METADATA_FILENAME_S = "metadata";


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



PolymarkerTool *CreatePolymarkerToolFromJSON (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p, PolymarkerServiceData *data_p, const json_t *service_job_json_p)
{
	PolymarkerTool *tool_p = 0;

	switch (data_p -> psd_tool_type)
		{
			case PTT_SYSTEM:
				try
					{
						tool_p = new AsyncSystemPolymarkerTool (job_p, seq_p, data_p, service_job_json_p);
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
	pt_job_dir_s = nullptr;
}



PolymarkerTool :: PolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p, const PolymarkerServiceData *data_p, const json_t *root_p)
{
	const char *value_s = GetJSONString (root_p, PT_JOB_DIR_S);

	pt_service_data_p = data_p;
	pt_seq_p = seq_p;
	pt_service_job_p = job_p;

	pt_job_dir_s = nullptr;


	if (value_s)
		{
			pt_job_dir_s = CopyToNewString (value_s, 0, false);
		}

	if (!pt_job_dir_s)
		{
			char uuid_s [UUID_STRING_BUFFER_SIZE];

			ConvertUUIDToString (job_p -> psj_base_job.sj_id, uuid_s);

			pt_job_dir_s = MakeFilename (data_p -> psd_working_dir_s, uuid_s);

			if (!pt_job_dir_s)
				{
					throw std :: bad_alloc ();
				}
		}

}



PolymarkerTool :: ~PolymarkerTool ()
{
	if (pt_job_dir_s)
		{
			FreeCopiedString (pt_job_dir_s);
		}
}



const char *PolymarkerTool :: GetName ()
{
	return (pt_seq_p ? pt_seq_p -> ps_name_s : NULL);
}


bool PolymarkerTool :: AddToJSON (json_t *root_p)
{
	bool success_flag = true;

	if (pt_job_dir_s)
		{
			success_flag = (json_object_set_new (root_p, PolymarkerTool :: PT_JOB_DIR_S, json_string (pt_job_dir_s)) == 0);
		}

	return success_flag;
}


bool PolymarkerTool :: AddSectionToResult (json_t *result_p, const char * const filename_s, const char * const key_s, PolymarkerFormatter *formatter_p)
{
	bool success_flag = false;
	char *full_filename_s = MakeFilename (pt_job_dir_s, filename_s);

	if (full_filename_s)
		{
			char *results_s = GetFileContentsAsStringByFilename (full_filename_s);

			if (results_s)
				{
					if (json_object_set_new (result_p, key_s, json_string (results_s)) == 0)
						{
							success_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add \"%s\": \"%s\"", key_s, results_s);
						}

					FreeCopiedString (results_s);
				}		/* if (results_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get data from %s", full_filename_s);
				}

			FreeCopiedString (full_filename_s);
		}		/* if (full_filename_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get full filename from \"%s\" and \"%s\"", pt_job_dir_s, filename_s);
		}

	return success_flag;
}


void PolymarkerTool :: SetPolymarkerSequence (const PolymarkerSequence *seq_p)
{
	pt_seq_p = seq_p;
}


bool PolymarkerTool ::  PreRun ()
{
	return SaveJobMetadata ();
}


bool PolymarkerTool :: SaveJobMetadata () const
{
	bool success_flag = false;

	if (pt_job_dir_s)
		{
			if (EnsureDirectoryExists (pt_job_dir_s))
				{
					char *metadata_s = MakeFilename (pt_job_dir_s, PT_METADATA_FILENAME_S);

					if (metadata_s)
						{
							json_t *metadata_p = json_object ();

							if (metadata_p)
								{
									const ServiceJob *base_job_p = & (pt_service_job_p -> psj_base_job);

									if (json_object_set_new (metadata_p, JOB_NAME_S, json_string (base_job_p -> sj_name_s)) == 0)
										{
											if ((! (base_job_p -> sj_description_s)) || (json_object_set_new (metadata_p, JOB_DESCRIPTION_S, json_string (base_job_p -> sj_name_s)) == 0))
												{
													if (json_dump_file (metadata_p, metadata_s, JSON_INDENT (2)) == 0)
														{
															success_flag = true;
														}
												}
										}

									json_decref (metadata_p);
								}

							FreeCopiedString (metadata_s);
						}
				}
		}

	return success_flag;
}


bool PolymarkerTool :: SetJobMetadata ()
{
	bool success_flag = false;

	char *metadata_s = MakeFilename (pt_job_dir_s, PT_METADATA_FILENAME_S);

	if (metadata_s)
		{
			json_error_t err;
			json_t *metadata_p = json_load_file (metadata_s, 0, &err);

			if (metadata_p)
				{
					const char *value_s = GetJSONString (metadata_p, JOB_NAME_S);

					if (value_s)
						{
							if (SetServiceJobName (& (pt_service_job_p -> psj_base_job), value_s))
								{
									success_flag = true;
									value_s = GetJSONString (metadata_p, JOB_DESCRIPTION_S);

									if (value_s)
										{
											if (!SetServiceJobDescription (& (pt_service_job_p -> psj_base_job), value_s))
												{

												}
										}
								}
						}

					json_decref (metadata_p);
				}

			FreeCopiedString (metadata_s);
		}

	return success_flag;
}



bool PolymarkerTool :: SetJobUUID (const uuid_t id)
{
	bool success_flag = false;
	char uuid_s [UUID_STRING_BUFFER_SIZE];
	char *new_job_dir_s;

	ConvertUUIDToString (id, uuid_s);

	new_job_dir_s = MakeFilename (pt_service_data_p -> psd_working_dir_s, uuid_s);

	if (new_job_dir_s)
		{
			if (pt_job_dir_s)
				{
					FreeCopiedString (pt_job_dir_s);
				}

			pt_job_dir_s = new_job_dir_s;

			SetServiceJobUUID (& (pt_service_job_p -> psj_base_job), id);

			success_flag = true;
		}

	return success_flag;
}

