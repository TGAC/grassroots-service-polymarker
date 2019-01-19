/*
** Copyright 2014-2018 The Earlham Institute
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
/*
 * primer3_prefs.c
 *
 *  Created on: 19 Jan 2019
 *      Author: billy
 */

#include <stdio.h>
#include <string.h>

#include "primer3_prefs.h"
#include "memory_allocations.h"
#include "streams.h"
#include "io_utils.h"
#include "string_utils.h"


static bool WriteKeyValuePairForString (const char *key_s, const char *value_s, FILE *out_f);

static bool WriteKeyValuePairForUnsignedInt (const char *key_s, const uint32 value, FILE *out_f);

static char *GetProductSizeRangeAsString (const uint32 min_value, const uint32 max_value);



static NamedParameterType S_PROD_SIZE_MIN = { "Product size range min", PT_UNSIGNED_INT };
static NamedParameterType S_PROD_SIZE_MAX = { "Product size range max", PT_UNSIGNED_INT };
static NamedParameterType S_MAX_SIZE = { "Primer maximum size", PT_UNSIGNED_INT };
static NamedParameterType S_LIB_AMBIGUITY_CODES_CONSENSUS = { "Product size range min", PT_BOOLEAN };
static NamedParameterType S_LIBERAL_BASE = { "Product size range max", PT_BOOLEAN };
static NamedParameterType S_PP_NUM_RETURN = { "Primer maximum size", PT_UNSIGNED_INT };
static NamedParameterType S_EXPLAIN_FLAG = { "Product size range min", PT_BOOLEAN };


Primer3Prefs *AllocatePrimer3Prefs (const PolymarkerServiceData *data_p)
{
	Primer3Prefs *prefs_p = NULL;

	return NULL;
}


void FreePrimer3Prefs (Primer3Prefs *prefs_p)
{
	if (prefs_p -> pp_thermodynamic_parameters_path_s)
		{
			FreeCopiedString (prefs_p -> pp_thermodynamic_parameters_path_s);
		}

	FreeMemory (prefs_p);
}


bool SavePrimer3Prefs (Primer3Prefs *prefs_p, const char *working_dir_s, const char *job_id_s, const char *primer_config_path_s)
{
	bool success_flag = false;
	char *filename_s = MakeFilename (working_dir_s, job_id_s);

	if (filename_s)
		{
/*
      :primer_product_size_range => "50-150" ,
      :primer_max_size => 25 ,
      :primer_lib_ambiguity_codes_consensus => 1,
      :primer_liberal_base => 1,
      :primer_num_return=>5,
      :primer_explain_flag => 1,
      :primer_thermodynamic_parameters_path
 */
			FILE *out_f = fopen (filename_s, "w");

			if (out_f)
				{
					char *range_s = GetProductSizeRangeAsString (prefs_p -> pp_product_size_range_min, prefs_p -> pp_product_size_range_max);

					if (range_s)
						{
							if (WriteKeyValuePairForString ("primer_product_size_range", range_s, out_f))
								{
									if (WriteKeyValuePairForUnsignedInt ("primer_max_size", prefs_p -> pp_max_size, out_f))
										{
											if (WriteKeyValuePairForUnsignedInt ("primer_lib_ambiguity_codes_consensus", prefs_p -> pp_lib_ambiguity_codes_consensus ? 1 : 0, out_f))
												{
													if (WriteKeyValuePairForUnsignedInt ("primer_liberal_base", prefs_p -> pp_liberal_base ? 1 : 0, out_f))
														{
															if (WriteKeyValuePairForUnsignedInt ("primer_num_return", prefs_p -> pp_num_return, out_f))
																{
																	if (WriteKeyValuePairForUnsignedInt ("primer_explain_flag", prefs_p -> pp_explain_flag ? 1 : 0, out_f))
																		{
																			if (WriteKeyValuePairForString ("primer_max_size", primer_config_path_s, out_f))
																				{
																					success_flag = true;
																				}		/* if (WriteKeyValuePairForString ("primer_max_size", primer_config_path_s, out_f)) */

																		}		/* if (WriteKeyValuePairForUnsignedInt ("primer_max_size", prefs_p -> pp_explain_flag ? 1 : 0, out_f)) */

																}		/* if (WriteKeyValuePairForUnsignedInt ("primer_max_size", prefs_p -> pp_max_size, out_f)) */

														}		/* if (WriteKeyValuePairForUnsignedInt ("primer_max_size", prefs_p -> pp_max_size, out_f)) */

												}		/* if (WriteKeyValuePairForUnsignedInt ("primer_max_size", prefs_p -> pp_max_size, out_f)) */

										}		/* if (WriteKeyValuePairForUnsignedInt ("primer_max_size", prefs_p -> pp_max_size, out_f)) */

								}		/* if (WriteKeyValuePairForString ("primer_product_size_range", range_s, out_f)) */

							FreeCopiedString (range_s);
						}		/* if (range_s) */


					if (fclose (out_f) != 0)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to close primer config file \"%s\"", filename_s);
						}

				}		/* if (out_f) */

			FreeCopiedString (filename_s);
		}		/* if (filename_s) */

	return success_flag;
}


bool AddPrimer3PrefsParameters (ParameterSet *params_p, Primer3Prefs *prefs_p)
{
	bool success_flag = false;

	return success_flag;
}


bool GetPrimer3PrefsParameterTypesForNamedParameters (struct Service *service_p, const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_PROD_SIZE_MIN.npt_name_s) == 0)
		{
			*pt_p = S_PROD_SIZE_MIN.npt_type;
		}
	else if (strcmp (param_name_s, S_PROD_SIZE_MAX.npt_name_s) == 0)
		{
			*pt_p = S_PROD_SIZE_MAX.npt_type;
		}
	else if (strcmp (param_name_s, S_MAX_SIZE.npt_name_s) == 0)
		{
			*pt_p = S_MAX_SIZE.npt_type;
		}
	else if (strcmp (param_name_s, S_LIB_AMBIGUITY_CODES_CONSENSUS.npt_name_s) == 0)
		{
			*pt_p = S_LIB_AMBIGUITY_CODES_CONSENSUS.npt_type;
		}
	else if (strcmp (param_name_s, S_LIBERAL_BASE.npt_name_s) == 0)
		{
			*pt_p = S_LIBERAL_BASE.npt_type;
		}
	else if (strcmp (param_name_s, S_PP_NUM_RETURN.npt_name_s) == 0)
		{
			*pt_p = S_PP_NUM_RETURN.npt_type;
		}
	else if (strcmp (param_name_s, S_EXPLAIN_FLAG.npt_name_s) == 0)
		{
			*pt_p = S_EXPLAIN_FLAG.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}


bool ParsePrimer3PrefsParameters (ParameterSet *params_p, Primer3Prefs *prefs_p)
{
	bool success_flag = false;

	return success_flag;
}


static char *GetProductSizeRangeAsString (const uint32 min_value, const uint32 max_value)
{
	char *range_s = NULL;
	char *min_s = GetUnsignedIntAsString (min_value);

	if (min_s)
		{
			char *max_s = GetUnsignedIntAsString (max_value);

			if (max_s)
				{
					range_s = ConcatenateVarargsStrings (min_s, "-", max_s, NULL);

					if (!range_s)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to concatenate \"%s\", \"-\" and \"%s\"", min_s, max_s);
						}

					FreeCopiedString (max_s);
				}		/* if (max_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get " UINT32_FMT " as a string", max_value);
				}

			FreeCopiedString (min_s);
		}		/* if (min_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get " UINT32_FMT " as a string", min_value);
		}

	return range_s;
}


static bool WriteKeyValuePairForString (const char *key_s, const char *value_s, FILE *out_f)
{
	bool success_flag = false;

	if (fprintf (out_f, "%s=%s\n", key_s, value_s) >= 0)
		{
			success_flag = true;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to write \"%s\"=\"%s\"", key_s, value_s);
		}

	return success_flag;
}


static bool WriteKeyValuePairForUnsignedInt (const char *key_s, const uint32 value, FILE *out_f)
{
	bool success_flag = false;

	if (fprintf (out_f, "%s=" UINT32_FMT "\n", key_s, value) >= 0)
		{
			success_flag = true;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to write \"%s\"=" UINT32_FMT, key_s, value);
		}

	return success_flag;
}
