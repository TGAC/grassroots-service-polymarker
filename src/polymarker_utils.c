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
#include <string.h>

#include "polymarker_utils.h"
#include "parameter_set.h"
#include "streams.h"
#include "regular_expressions.h"
#include "string_utils.h"
#include "polymarker_service_job.h"
#include "polymarker_tool.hpp"

#include "string_parameter.h"


/*
 * STATIC DECLARATIONS
 */

static bool WriteParameterValues (const ParameterSet *params_p, uint32 index, FILE *marker_f, bool *has_chromosome_param_p);

static bool WriteParameterValuesFromGroup (ParameterGroup *group_p, FILE *marker_f);


static char *ParseSequence (const char * const value_s);


static bool GetPosition (const json_t *polymorphism_p, const char * const key_s, uint32 *index_p);

static bool GetSequenceDifferences (const json_t *polymorphism_p, const char **query_ss, const char **hit_ss);

/*
 * API DEFINTIIONS
 */


const char *GetSequenceParametersGroupName (void)
{
	return "Sequence parameters";
}


bool CreateAndAddMarkerListFile (const char *marker_file_s, const ParameterSet *param_set_p, ByteBuffer *buffer_p)
{
	bool success_flag = false;
	FILE *marker_f = fopen (marker_file_s, "w");

	if (marker_f)
		{
			uint32 i = 0;
			bool loop_flag = true;
			bool has_chromosome_flag = false;

			if (WriteParameterValues (param_set_p, i, marker_f, &has_chromosome_flag))
				{
					if (!has_chromosome_flag)
						{
							if (AppendStringsToByteBuffer (buffer_p, " --arm_selection arm_selection_first_two", " --marker_list ", marker_file_s, NULL))
								{
									success_flag = true;
								}
						}
					else
						{
							if (AppendStringsToByteBuffer (buffer_p, " --marker_list ", marker_file_s, NULL))
								{
									success_flag = true;
								}
						}
				}

			fclose (marker_f);
		}

	return success_flag;
}

bool CreateMarkerListFile (const char *marker_file_s, const ParameterSet *param_set_p, bool has_chromosome_param_flag)
{
	bool success_flag = false;
	FILE *marker_f = fopen (marker_file_s, "w");

	if (marker_f)
		{
			uint32 i = 0;
			bool loop_flag = true;
			bool has_chromosome_flag = false;

			if (WriteParameterValues (param_set_p, i, marker_f, &has_chromosome_flag))
				{
					success_flag = true;
				}

			fclose (marker_f);
		}

	return success_flag;
}




bool CreateMarkerListFileByGroups (const char *marker_file_s, const ParameterSet *param_set_p)
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
					if (WriteParameterValuesFromGroup (group_p, marker_f))
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
																			if (! (WriteParameterValuesFromGroup (group_p, marker_f)))
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

static bool WriteParameterValues (const ParameterSet *params_p, uint32 index, FILE *marker_f, bool *has_chromosome_param_p)
{
	bool success_flag = false;
	const char *gene_s = NULL;


	if (GetCurrentStringParameterValueFromParameterSet (params_p, PS_GENE_ID.npt_name_s, &gene_s))
		{
			if (gene_s)
				{
					/*
					 * The gene and sequence values are required, the chromosome is optional
					 */
					const char *chromosome_s = NULL;
					const char *sequence_value_s = NULL;

					GetCurrentStringParameterValueFromParameterSet (params_p, PS_TARGET_CHROMOSOME.npt_name_s, &chromosome_s);

					if (GetCurrentStringParameterValueFromParameterSet (params_p, PS_SEQUENCE.npt_name_s, &sequence_value_s))
						{
							if (sequence_value_s)
								{
									/*
									 * This sequence could be of the form ATCG[C/A]TGCA... or
									 * in the grassroots markup from the blast service.
									 */
									char *sequence_s = ParseSequence (sequence_value_s);
									const char *sequence_to_use_s = sequence_s ? sequence_s : sequence_value_s;

									if (chromosome_s)
										{
											if (has_chromosome_param_p)
												{
													*has_chromosome_param_p = true;
												}

											if (fprintf (marker_f, "%s,%s,%s", gene_s, chromosome_s, sequence_to_use_s) > 0)
												{
													success_flag = true;
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "%s,%s,%s", gene_s, chromosome_s, sequence_to_use_s);
												}
										}
									else
										{
											if (has_chromosome_param_p)
												{
													*has_chromosome_param_p = false;
												}

											if (fprintf (marker_f, "%s,%s,%s", gene_s, gene_s, sequence_to_use_s) > 0)
												{
													success_flag = true;
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "%s,%s", gene_s, sequence_to_use_s);
												}
										}

									if (sequence_s)
										{
											FreeCopiedString (sequence_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "parameter \"%s\" is NULL", PS_SEQUENCE.npt_name_s);
								}

						}		/* if (GetParameterValueFromParameterSet (params_p, PS_TARGET_CHROMOSOME.npt_name_s, &chromosome_value, true)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get parameter \"%s\"", PS_SEQUENCE.npt_name_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "parameter \"%s\" is NULL", PS_GENE_ID.npt_name_s);
				}

		}		/* if (GetParameterValueFromParameterSet (param_set_p, PS_GENE_ID.npt_name_s, &value, true)) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get parameter \"%s\"", PS_GENE_ID.npt_name_s);
		}

	return success_flag;
}


static bool WriteParameterValuesFromGroup (ParameterGroup *group_p, FILE *marker_f)
{
	bool success_flag = false;
	const char *gene_s = NULL;


	if (GetCurrentStringParameterValueFromParameterGroup (group_p, PS_GENE_ID.npt_name_s, &gene_s))
		{
			if (gene_s)
				{
					if (fprintf (marker_f, "%s,", gene_s) > 0)
						{
							const char *sequence_value_s = NULL;
							const char *chromosome_s = NULL;

							/* The chromosome is optional */
							if (GetCurrentStringParameterValueFromParameterGroup (group_p, PS_TARGET_CHROMOSOME.npt_name_s, &chromosome_s))
								{
									if (chromosome_s)
										{
											if (fprintf (marker_f, "%s,", chromosome_s) < 0)
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to write parameter \"%s\" value \"%s\"", PS_TARGET_CHROMOSOME.npt_name_s, chromosome_s);
												}		/* if (fprintf (marker_f, "%s,", value.st_string_value_s) > 0) */
										}
								}		/* if (GetParameterValueFromParameterSet (param_set_p, PS_GENE_ID.npt_name_s, &value, true)) */
							else
								{
									PrintErrors (STM_LEVEL_INFO, __FILE__, __LINE__, "Failed to get parameter \"%s\"", PS_TARGET_CHROMOSOME.npt_name_s);
								}


							if (GetCurrentStringParameterValueFromParameterGroup (group_p, PS_SEQUENCE.npt_name_s, &sequence_value_s))
								{
									if (sequence_value_s)
										{
											/*
											 * This sequence could be of the form ATCG[C/A]TGCA... or
											 * in the grassroots markup from the blast service.
											 */
											char *sequence_s = ParseSequence (sequence_value_s);

											if (sequence_s)
												{
													success_flag = (fprintf (marker_f, "%s", sequence_s) > 0);
													FreeCopiedString (sequence_s);
												}
											else
												{
													success_flag = (fprintf (marker_f, "%s", sequence_value_s) > 0);
												}

											if (!success_flag)
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to write parameter \"%s\" value \"%s\"", PS_SEQUENCE.npt_name_s, sequence_value_s);
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
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to write parameter \"%s\" value \"%s\"", PS_GENE_ID.npt_name_s, gene_s);
						}

				}		/* if (gene_s) */


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
											if (AddServiceJobToService (service_p, (ServiceJob *) job_p, false))
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

												}		/* if (AddServiceJobToService (service_p, (ServiceJob *) job_p)) */
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
	char *parsed_sequence_s = NULL;
	json_error_t err;
	json_t *seq_json_p = json_loads (value_s, 0, &err);

	if (seq_json_p)
		{
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{
					const char * const key_s = "query_sequence";
					const char *sequence_s = GetJSONString (seq_json_p, key_s);

					if (sequence_s)
						{
							json_t *polymorphisms_p =  json_object_get (seq_json_p, "polymorphisms");

							if (polymorphisms_p)
								{
									if (json_is_array (polymorphisms_p))
										{
											bool success_flag = true;
											size_t i = 0;
											uint32 previous_end_index = 0;
											const size_t num_polymorphisms = json_array_size (polymorphisms_p);

											while ((i < num_polymorphisms) && success_flag)
												{
													json_t *polymorphism_p = json_array_get (polymorphisms_p, i);
													uint32 begin_index;

													if (GetPosition (polymorphism_p, "faldo:begin", &begin_index))
														{
															uint32 end_index;

															if (GetPosition (polymorphism_p, "faldo:end", &end_index))
																{
																	const char *query_s;
																	const char *hit_s;

																	if (GetSequenceDifferences (polymorphism_p, &query_s, &hit_s))
																		{
																			uint32 chunk_length = begin_index - previous_end_index - 1;
																			const size_t mnp_length = strlen (query_s);

																			if (AppendToByteBuffer (buffer_p, sequence_s, chunk_length))
																				{
																					size_t j;
																					char query_buffer_s [2];
																					char hit_buffer_s [2];

																					* (query_buffer_s + 1) = '\0';
																					* (hit_buffer_s + 1) = '\0';

																					chunk_length += mnp_length;

																					sequence_s += chunk_length;
																					previous_end_index += chunk_length;

																					for (j = 0; j < mnp_length; ++ j, ++ hit_s, ++ query_s)
																						{
																							*query_buffer_s = *query_s;
																							*hit_buffer_s = *hit_s;

																							if (!AppendStringsToByteBuffer (buffer_p, "[", query_buffer_s, "/", hit_buffer_s, "]", NULL))
																								{
																									j = mnp_length;
																									success_flag = false;
																								}
																						}

																					if (success_flag)
																						{
																							++ i;
																						}
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, polymorphism_p, "Failed to add sequence differences [%s/%s] to buffer", query_s, hit_s);
																						}

																				}		/* if (AppendToByteBuffer (buffer_p, sequence_s, chunk_length)) */
																			else
																				{
																					success_flag = false;		/* force exit from loop */
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, polymorphism_p, "Failed to add sequence chunk to buffer");
																				}

																		}		/* if (GetSequenceDifferences (polymorphism_p, &query_s, &hit_s)) */
																	else
																		{
																			success_flag = false;		/* force exit from loop */
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, polymorphism_p, "Failed to get sequence differences from polymorphism");
																		}

																}		/* if (GetPosition (polymorphism_p, "faldo:end", &end_index)) */
															else
																{
																	success_flag = false;		/* force exit from loop */
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, polymorphism_p, "Failed to get \"faldo:end\" from polymorphism");
																}

														}		/* if (GetPosition (polymorphism_p, "faldo:begin", &begin_index)) */
													else
														{
															success_flag = false;		/* force exit from loop */
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, polymorphism_p, "Failed to get \"faldo:begin\" from polymorphism");
														}

												}		/* for (i = 0; i < num_polymorphisms; ++ i) */

											if (success_flag)
												{
													/* Add the remaining sequence */
													if (AppendStringToByteBuffer (buffer_p, sequence_s))
														{
															parsed_sequence_s = DetachByteBufferData (buffer_p);
															buffer_p = NULL;

														}
													else
														{
															success_flag = false;
														}
												}

										}		/* if (json_is_array (polymorphisms_p)) */

								}		/* if (polymorphisms_p) */
							else
								{

								}

						}		/* if (sequence_s) */
					else
						{

						}

					if (buffer_p)
						{
							FreeByteBuffer (buffer_p);
						}
				}
			else
				{

				}

			json_decref (seq_json_p);
		}		/* if (seq_json_p) */


	return parsed_sequence_s;
}


static bool GetPosition (const json_t *polymorphism_p, const char * const key_s, uint32 *index_p)
{
	bool success_flag = false;
	const json_t *locus_p = json_object_get (polymorphism_p, "locus");

	if (locus_p)
		{
			const json_t *position_p = json_object_get (locus_p, key_s);

			if (position_p)
				{
					success_flag = GetJSONInteger (position_p, "faldo:position", (int *) index_p);
				}		/* if (position_p) */

		}		/* if (locus_p) */

	return success_flag;
}


static bool GetSequenceDifferences (const json_t *polymorphism_p, const char **query_ss, const char **hit_ss)
{
	bool success_flag = false;
	const json_t *sequence_difference_p = json_object_get (polymorphism_p, "sequence_difference");

	if (sequence_difference_p)
		{
			const char *query_s = GetJSONString (sequence_difference_p, "query");

			if (query_s)
				{
					const char *hit_s = GetJSONString (sequence_difference_p, "hit");

					if (hit_s)
						{
							*query_ss = query_s;
							*hit_ss = hit_s;

							success_flag = true;
						}		/* if (hit_s) */

				}		/* if (query_s) */

		}		/* if (sequence_difference_p) */

	return success_flag;
}

