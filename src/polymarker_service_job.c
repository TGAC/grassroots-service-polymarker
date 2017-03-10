/*
 * polymarker_service_job.c
 *
 *  Created on: 7 Feb 2017
 *      Author: billy
 */


#include <string.h>

#include "polymarker_service_job.h"
#include "polymarker_tool.hpp"

#include "string_utils.h"

/*
 * STATIC DECLARATIONS
 */

static const char * const PSJ_JOB_S = "job";
static const char * const PSJ_PROCESS_ID_S = "process_id";




PolymarkerServiceJob *AllocatePolymarkerServiceJob (Service *service_p, const char *job_name_s, const char *job_description_s, const char *tool_name_s, PolymarkerServiceData *data_p)
{
	PolymarkerServiceJob *poly_job_p = (PolymarkerServiceJob *) AllocMemory (sizeof (PolymarkerServiceJob));

	if (poly_job_p)
		{
			PolymarkerTool *tool_p = NULL;
			ServiceJob * const base_service_job_p = & (poly_job_p -> psj_base_job);

			InitServiceJob (base_service_job_p, service_p, job_name_s, job_description_s, NULL, FreePolymarkerServiceJob, NULL);

			tool_p = CreatePolymarkerTool (data_p, poly_job_p, data_p -> psd_tool_type);

			if (tool_p)
				{
					return poly_job_p;
				}		/* if (tool_p) */

			ClearServiceJob (base_service_job_p);
			FreeMemory (poly_job_p);
		}		/* if (blast_job_p) */

	return NULL;
}


void FreePolymarkerServiceJob (ServiceJob *job_p)
{
	PolymarkerServiceJob *poly_job_p = (PolymarkerServiceJob *) job_p;

	if (poly_job_p -> psj_tool_p)
		{
			FreePolymarkerTool (poly_job_p -> psj_tool_p);
		}

	ClearServiceJob (job_p);

	FreeMemory (poly_job_p);
}


void CustomisePolymarkerServiceJob (Service * UNUSED_PARAM (service_p), ServiceJob *job_p)
{
	job_p -> sj_update_fn = UpdatePolymarkerServiceJob;
	job_p -> sj_free_fn = FreePolymarkerServiceJob;
}


bool UpdatePolymarkerServiceJob (ServiceJob *job_p)
{
	PolymarkerServiceJob *poly_job_p = (PolymarkerServiceJob *) job_p;

	poly_job_p -> psj_tool_p -> GetStatus (true);

	return true;
}


ServiceJob *GetPolymarkerServiceJobFromJSON (struct Service *service_p, const json_t *service_job_json_p)
{
	json_t *job_json_p = json_object_get (service_job_json_p, PSJ_JOB_S);

	if (job_json_p)
		{
			PolymarkerServiceJob *polymarker_job_p = (PolymarkerServiceJob *) AllocMemory (sizeof (PolymarkerServiceJob));

			if (polymarker_job_p)
				{
					polymarker_job_p -> psj_tool_p = NULL;
					polymarker_job_p -> psj_process_id = -1;

					if (InitServiceJobFromJSON (& (polymarker_job_p -> psj_base_job), job_json_p))
						{
							const char *tool_type_s = GetJSONString (job_json_p, PS_TOOL_S);

							if (tool_type_s)
								{
									PolymarkerToolType tool_type = PTT_NUM_TYPES;
									PolymarkerServiceData *data_p = (PolymarkerServiceData *) (service_p -> se_data_p);

									if (strcmp (tool_type_s, PS_TOOL_SYSTEM_S) == 0)
										{
											tool_type = PTT_SYSTEM;
										}
									else if (strcmp (tool_type_s, PS_TOOL_WEB_S) == 0)
										{
											tool_type = PTT_WEB;
										}

									if (tool_type != PTT_NUM_TYPES)
										{
											PolymarkerTool *tool_p = CreatePolymarkerTool (data_p, polymarker_job_p, tool_type);

											if (tool_p)
												{
													return (& (polymarker_job_p -> psj_base_job));
												}

										}		/* if (tool_type != PTT_NUM_TYPES) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get tool type of \"%s\"", tool_type_s);
										}

								}		/* if (tool_type_s) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to get tool from json");
								}		/* if (tool_type_s) */

						}		/* if (InitServiceJobFromJSON (& (polymarker_job_p -> psj_base_job), job_json_p)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "InitServiceJobFromJSON failed");
						}

					FreePolymarkerServiceJob ((ServiceJob *) polymarker_job_p);
				}		/* if (polymarker_job_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for BlastServiceJob");
				}

		}		/* if (job_json_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_json_p, "Failed to get job from json");
		}

	return NULL;

}



json_t *ConvertPolymarkerServiceJobToJSON (Service * UNUSED_PARAM (service_p), ServiceJob *service_job_p)
{
	json_t *polymarker_job_json_p = json_object ();
	char uuid_s [UUID_STRING_BUFFER_SIZE];

	ConvertUUIDToString (service_job_p -> sj_id, uuid_s);

	if (polymarker_job_json_p)
		{
			const char *service_name_s = GetServiceName (service_job_p -> sj_service_p);

			if (json_object_set_new (polymarker_job_json_p, JOB_SERVICE_S, json_string (service_name_s)) == 0)
				{
					json_t *base_job_json_p = GetServiceJobAsJSON (service_job_p);

					if (base_job_json_p)
						{
							if (json_object_set_new (polymarker_job_json_p, PSJ_JOB_S, base_job_json_p) == 0)
								{
									PolymarkerServiceJob *poly_job_p = (PolymarkerServiceJob *) service_job_p;

									if (json_object_set_new (polymarker_job_json_p, PSJ_PROCESS_ID_S, json_integer (poly_job_p -> psj_process_id)) == 0)
										{
											return polymarker_job_json_p;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, polymarker_job_json_p, "Failed to add %s =" UINT32_FMT " for job %s", PSJ_PROCESS_ID_S, poly_job_p -> psj_process_id, uuid_s);
										}

								}		/* if (json_object_set_new (blast_job_json_p, BSJ_JOB_S, base_job_json_p) == 0) */
							else
								{
									json_decref (base_job_json_p);
								}

						}		/* if (base_job_json_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetServiceJobAsJSON failed for %s", uuid_s);
						}

				}		/* if (json_object_set_new (polymarker_job_json_p, JOB_SERVICE_S, json_string (service_name_s)) == 0) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, polymarker_job_json_p, "Could not set %s:%s", JOB_SERVICE_S, service_name_s);
				}

			json_decref (polymarker_job_json_p);
		}		/* if (polymarker_job_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "json_object failed for %s", uuid_s);
		}

	return NULL;
}



