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
					if (! (WriteParameterValues (group_p, marker_f)))
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
													bool loop_flag = true;

													while (group_node_p && loop_flag)
														{
															group_p = group_node_p -> pgn_param_group_p;

															if (MatchPattern (reg_ex_p, group_p -> pg_name_s))
																{
																	if (! (WriteParameterValues (group_p, marker_f)))
																		{
																			loop_flag = false;
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set write parameter values to marker list file for \"%s\"", group_p -> pg_name_s);
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

						}		/* if (! (WriteParameterValues (group_p, marker_f))) */
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
									if (fprintf (marker_f, "%s", value.st_string_value_s) > 0)
										{
											success_flag = true;
										}		/* if (fprintf (marker_f, "%s,", value.st_string_value_s) > 0) */
									else
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
