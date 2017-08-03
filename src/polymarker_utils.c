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
 * polymarker_utils.c
 *
 *  Created on: 13 Jul 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#include <stdio.h>

#include "polymarker_utils.h"
#include "parameter_set.h"
#include "streams.h"
#include "regular_expressions.h"
#include "string_utils.h"
#include "polymarker_service_job.h"
#include "polymarker_tool.hpp"


/*
 * STATIC DECLARATIONS
 */

static bool WriteParameterValues (ParameterGroup *group_p, FILE *marker_f);


/*
 * API DEFINTIIONS
 */


const char *GetSequenceParametersGroupName (void)
{
	return "Sequence parameters";
}


bool CreateMarkerListFile (const char *marker_file_s, const ParameterSet *param_set_p)
{
	bool success_flag = false;
	FILE *marker_f = fopen (marker_file_s, "w");

	if (marker_f)
		{
			ParameterGroup *group_p;
			const char *sequence_group_name_s = GetSequenceParametersGroupName ();

			/* First get the default group params */
			group_p = GetParameterGroupFromParameterSetByGroupName (param_set_p, sequence_group_name_s);

			if (group_p)
				{
					if (WriteParameterValues (group_p, marker_f))
						{
							RegExp *reg_ex_p = AllocateRegExp (32);

							if (reg_ex_p)
								{
									/* Now get any repeatable groups */
									char *reg_exp_s = GetRepeatableParameterGroupRegularExpression (group_p);

									if (reg_exp_s)
										{
											if (SetPattern (reg_ex_p, reg_exp_s, 0))
												{
													ParameterGroupNode *group_node_p = (ParameterGroupNode *) (param_set_p -> ps_grouped_params_p -> ll_head_p);

													success_flag = true;

													while (group_node_p && success_flag)
														{
															group_p = group_node_p -> pgn_param_group_p;

															if (MatchPattern (reg_ex_p, group_p -> pg_name_s))
																{
																	if (fprintf (marker_f, "\n") >= 0)
																		{
																			if (! (WriteParameterValues (group_p, marker_f)))
																				{
																					success_flag = false;
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set write parameter values to marker list file for \"%s\"", group_p -> pg_name_s);
																				}
																		}
																	else
																		{
																			success_flag = false;
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add newline in marker list file before \"%s\"", group_p -> pg_name_s);
																		}

																}		/* if (MatchPattern (reg_ex_p, group_p -> pg_name_s)) */

															group_node_p = (ParameterGroupNode *) (group_node_p -> pgn_node.ln_next_p);
														}		/* while (group_node_p) */

												}		/* if (SetPattern (reg_ex_p, reg_exp_s, 0)) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set regular expression pattern to \"%s\"", reg_exp_s);
												}

											FreeCopiedString (reg_exp_s);
										}		/* if (reg_exp_s) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get regular expression value for matching repeatable parameter groups of \"%s\"", sequence_group_name_s);
										}

									FreeRegExp (reg_ex_p);
								}		/* if (reg_ex_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate regular expression for matching repeatable parameter groups of \"%s\"", sequence_group_name_s);
								}

						}		/* if (WriteParameterValues (group_p, marker_f)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set write parameter values to marker list file for \"%s\"", group_p -> pg_name_s);
						}
				}		/* if (group_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get parameter group \"%s\"", sequence_group_name_s);
				}

			if (fclose (marker_f) != 0)
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to close marker file \"%s\"", marker_file_s);
				}

		}		/* if (marker_f) */
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to open marker file \"%s\"", marker_file_s);
		}

	return success_flag;
}



/*
 * STATIC DEFINITIONS
 */


static bool WriteParameterValues (ParameterGroup *group_p, FILE *marker_f)
{
	bool success_flag = false;
	SharedType value;

	InitSharedType (&value);

	if (GetParameterValueFromParameterGroup (group_p, PS_GENE_ID.npt_name_s, &value, true))
		{
			if (fprintf (marker_f, "%s,", value.st_string_value_s) > 0)
				{
					/* The chromosome is optional */
					if (GetParameterValueFromParameterGroup (group_p, PS_TARGET_CHROMOSOME.npt_name_s, &value, true))
						{
							if (fprintf (marker_f, "%s,", value.st_string_value_s) < 0)
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to write parameter \"%s\" value \"%s\"", PS_TARGET_CHROMOSOME.npt_name_s, value.st_string_value_s);
								}		/* if (fprintf (marker_f, "%s,", value.st_string_value_s) > 0) */

						}		/* if (GetParameterValueFromParameterSet (param_set_p, PS_GENE_ID.npt_name_s, &value, true)) */
					else
						{
							PrintErrors (STM_LEVEL_INFO, __FILE__, __LINE__, "Failed to get parameter \"%s\"", PS_TARGET_CHROMOSOME.npt_name_s);
						}


					if (GetParameterValueFromParameterGroup (group_p, PS_SEQUENCE.npt_name_s, &value, true))
						{
							if (value.st_string_value_s)
								{
									/*
									 * This sequence could be of the form ATCG[C/A]TGCA... or
									 * in the grassroots markup from the blast service.
									 */
									char *sequence_s = ParseSequence (value.st_string_value_s);

									if (sequence_s)
										{
											success_flag = (fprintf (marker_f, "%s", sequence_s) > 0);
											FreeCopiedString (sequence_s);
										}
									else
										{
											success_flag = (fprintf (marker_f, "%s", value.st_string_value_s) > 0);
										}

									if (!success_flag)
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to write parameter \"%s\" value \"%s\"", PS_SEQUENCE.npt_name_s, value.st_string_value_s);
										}
								}

						}		/* if (GetParameterValueFromParameterSet (param_set_p, PS_GENE_ID.npt_name_s, &value, true)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get parameter \"%s\"", PS_SEQUENCE.npt_name_s);
						}

				}		/* if (fprintf (marker_f, "%s,", value.st_string_value_s) > 0) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to write parameter \"%s\" value \"%s\"", PS_TARGET_CHROMOSOME.npt_name_s, value.st_string_value_s);
				}

		}		/* if (GetParameterValueFromParameterSet (param_set_p, PS_GENE_ID.npt_name_s, &value, true)) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get parameter \"%s\"", PS_GENE_ID.npt_name_s);
		}

	return success_flag;
}


ServiceJobSet *GetPreviousJobResults (LinkedList *ids_p, PolymarkerServiceData *polymarker_data_p)
{
	Service *service_p = polymarker_data_p -> psd_base_data.sd_service_p;
	ServiceJobSet *jobs_p = AllocateServiceJobSet (service_p);

	if (jobs_p)
		{
			StringListNode *node_p = (StringListNode *) (ids_p -> ll_head_p);
			uint32 num_successful_jobs = 0;

			while (node_p)
				{
					uuid_t job_id;
					const char * const job_id_s = node_p -> sln_string_s;

					if (uuid_parse (job_id_s, job_id) == 0)
						{
							PolymarkerServiceJob *polymarker_job_p = AllocatePolymarkerServiceJob (jobs_p -> sjs_service_p, NULL, polymarker_data_p);

							if (polymarker_job_p)
								{
									ServiceJob *job_p = (ServiceJob *) polymarker_job_p;

									if (polymarker_job_p -> psj_tool_p -> SetJobUUID (job_id))
										{
											if (AddServiceJobToServiceJobSet (jobs_p, (ServiceJob *) job_p))
												{
													if (polymarker_job_p -> psj_tool_p -> SetJobMetadata ())
														{
															if (AddPolymarkerResult (polymarker_job_p, job_id_s))
																{
																	SetServiceJobStatus (job_p, OS_SUCCEEDED);
																	++ num_successful_jobs;
																}		/* if (DeterminePolymarkerResult (polymarker_job_p)) */
															else
																{
																	char *error_s = ConcatenateVarargsStrings ("Failed to determine ", GetServiceName (service_p), " result for id \"", job_id_s, "\"", NULL);

																	SetServiceJobStatus (job_p, OS_FAILED);

																	if (error_s)
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, error_s);

																			if (!AddErrorToServiceJob (job_p, job_id_s, error_s))
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add error text for \"%s\"", job_id_s);
																				}

																			FreeCopiedString (error_s);
																		}		/* if (error_s) */
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get result for \"%s\"", job_id_s);
																		}
																}

														}		/* if (polymarker_job_p -> psj_tool_p -> SetJobMetadata ()) */

												}		/* if (AddServiceJobToServiceJobSet (jobs_p, (ServiceJob *) job_p)) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate add ServiceJob with id \"%s\" to ServiceJobSet", job_id_s);
													FreeServiceJob (job_p);
												}

										}


								}		/* if (polymarker_job_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate ServiceJob");
								}

						}		/* if (uuid_parse (job_id_s, job_id) == 0) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to parse job id \"%s\"", job_id_s);
						}

					node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
				}		/* while (node_p) */

		}		/* if (jobs_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate ServiceJobSet");
		}

	return jobs_p;
}



static char *ParseSequence (const char * const value_s)
{
	char *sequence_s = NULL;
	json_error_t err;
	json_t *seq_json_p = json_loads (value_s, 0, &err);

	if (seq_json_p)
		{
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{

					FreeByteBuffer (buffer_p);
				}
			else
				{

				}

			json_decref (seq_json_p);
		}		/* if (seq_json_p) */


	return sequence_s;
}


