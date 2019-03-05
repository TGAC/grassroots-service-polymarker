/*
 * polymarker_service_job.c
 *
 *  Created on: 7 Feb 2017
 *      Author: billy
 */


#include <string.h>


#define ALLOCATE_POLYMARKER_SERVICE_JOB_TAGS (1)
#include "polymarker_service_job.h"
#include "polymarker_tool.hpp"

#include "string_utils.h"

/*
 * STATIC DECLARATIONS
 */

static const char * const PSJ_JOB_S = "job";
static const char * const PSJ_PROCESS_ID_S = "process_id";


static bool CalculatePolymarkerServiceJobResults (ServiceJob *job_p);



PolymarkerServiceJob *AllocatePolymarkerServiceJob (Service *service_p, const PolymarkerSequence *db_p, PolymarkerServiceData *data_p)
{
	PolymarkerServiceJob *poly_job_p = (PolymarkerServiceJob *) AllocMemory (sizeof (PolymarkerServiceJob));

	if (poly_job_p)
		{
			PolymarkerTool *tool_p = NULL;
			ServiceJob * const base_service_job_p = & (poly_job_p -> psj_base_job);
			const char *name_s = NULL;
			const char *description_s = NULL;

			if (db_p)
				{
					name_s = db_p -> ps_name_s;
					description_s = db_p -> ps_description_s;
				}

			InitServiceJob (base_service_job_p, service_p, name_s, description_s, NULL, NULL, FreePolymarkerServiceJob, NULL, PSJ_TYPE_S);

			tool_p = CreatePolymarkerTool (poly_job_p, db_p, data_p);

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

	FreeBaseServiceJob (job_p);
}


void CustomisePolymarkerServiceJob (Service * UNUSED_PARAM (service_p), ServiceJob *job_p)
{
	job_p -> sj_calculate_result_fn = CalculatePolymarkerServiceJobResults;
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
											PolymarkerSequence *seq_p = NULL;

											data_p -> psd_tool_type = tool_type;

											polymarker_job_p -> psj_tool_p  = CreatePolymarkerToolFromJSON (polymarker_job_p, seq_p, data_p, service_job_json_p);

											if (polymarker_job_p -> psj_tool_p )
												{
													CustomisePolymarkerServiceJob (service_p, & (polymarker_job_p -> psj_base_job));

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



json_t *ConvertPolymarkerServiceJobToJSON (Service * UNUSED_PARAM (service_p), ServiceJob *service_job_p, bool omit_results_flag)
{
	json_t *polymarker_job_json_p = json_object ();
	char uuid_s [UUID_STRING_BUFFER_SIZE];

	ConvertUUIDToString (service_job_p -> sj_id, uuid_s);

	if (polymarker_job_json_p)
		{
			const char *service_name_s = GetServiceName (service_job_p -> sj_service_p);

			if (json_object_set_new (polymarker_job_json_p, JOB_SERVICE_S, json_string (service_name_s)) == 0)
				{
					json_t *base_job_json_p = GetServiceJobAsJSON (service_job_p, omit_results_flag);

					if (base_job_json_p)
						{
							PolymarkerServiceJob *polymarker_job_p = (PolymarkerServiceJob *) service_job_p;
							const char *tool_type_s = NULL;

							switch (polymarker_job_p -> psj_tool_p -> GetToolType ())
								{
									case PTT_SYSTEM:
										tool_type_s = PS_TOOL_SYSTEM_S;
										break;

									case PTT_WEB:
										tool_type_s = PS_TOOL_WEB_S;
										break;

									default:
										break;
								}

							if (tool_type_s)
								{
									if (json_object_set_new (base_job_json_p, PS_TOOL_S, json_string (tool_type_s)) == 0)
										{
											if (json_object_set_new (polymarker_job_json_p, PSJ_JOB_S, base_job_json_p) == 0)
												{
													return polymarker_job_json_p;
												}		/* if (json_object_set_new (blast_job_json_p, BSJ_JOB_S, base_job_json_p) == 0) */
											else
												{
													json_decref (base_job_json_p);
												}

										}

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



void PolymarkerServiceJobCompleted (ServiceJob *job_p)
{
	if (job_p -> sj_result_p == NULL)
		{
			PolymarkerServiceJob *polymarker_job_p = (PolymarkerServiceJob *) job_p;

			if (!DeterminePolymarkerResult (polymarker_job_p))
				{
					char uuid_s [UUID_STRING_BUFFER_SIZE];

					ConvertUUIDToString (job_p -> sj_id, uuid_s);

					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__,  "Failed to get result for \"%s\"", uuid_s);
				}
		}
}


bool AddPolymarkerResult (PolymarkerServiceJob *polymarker_job_p, const char *uuid_s)
{
	bool success_flag = false;
	json_t *result_json_p = json_object ();

	if (result_json_p)
		{
			PolymarkerTool *tool_p = polymarker_job_p -> psj_tool_p;

			//bool PolymarkerTool :: AddSectionToResult (json_t *result_p, const char * const filename_s, const char * const key_s, PolymarkerFormatter *formatter_p)

			if (tool_p -> AddSectionToResult (result_json_p, "primers.csv", "primers", 0))
				{
					if (tool_p -> AddSectionToResult (result_json_p, "exons_genes_and_contigs.fa", "exons_genes_and_contigs", 0))
						{
							json_t *polymarker_result_json_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, uuid_s, result_json_p);

							if (polymarker_result_json_p)
								{
									if (AddResultToServiceJob (& (polymarker_job_p -> psj_base_job), polymarker_result_json_p))
										{
											success_flag = true;
										}
									else
										{
											json_decref (polymarker_result_json_p);
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to append polymarker result for \"%s\"", uuid_s);
										}

								}		/* if (polymarker_result_json_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create Polymarker result for \"%s\"", uuid_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add exons_genes_and_contigs results for \"%s\"", uuid_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add primers results for \"%s\"", uuid_s);
				}

			json_decref (result_json_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create JSON result object for \"%s\"", uuid_s);
		}

	return success_flag;
}


bool DeterminePolymarkerResult (PolymarkerServiceJob *polymarker_job_p)
{
	bool success_flag = false;
	OperationStatus status = GetServiceJobStatus (& (polymarker_job_p -> psj_base_job));
	char uuid_s [UUID_STRING_BUFFER_SIZE];
	ConvertUUIDToString (polymarker_job_p -> psj_base_job.sj_id, uuid_s);

	if (status == OS_SUCCEEDED)
		{
			if (AddPolymarkerResult (polymarker_job_p, uuid_s))
				{
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get PolymarkerServiceJob result for \"%s\"", uuid_s);
				}

		}		/* if (status == OS_SUCCEEDED) */
	else
		{
			success_flag = true;
		}

	return success_flag;
}



static bool CalculatePolymarkerServiceJobResults (ServiceJob *job_p)
{
	return DeterminePolymarkerResult ((PolymarkerServiceJob *) job_p);
}
