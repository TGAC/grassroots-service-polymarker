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
#include "math_utils.h"


static bool WriteKeyValuePairForString (const char *key_s, const char *value_s, FILE *out_f);

static bool WriteKeyValuePairForUnsignedInt (const char *key_s, const uint32 value, FILE *out_f);

static char *GetProductSizeRangeAsString (const uint32 min_value, const uint32 max_value);


static const uint32 S_DEFAULT_PROD_SIZE_MIN = 100;
static const uint32 S_DEFAULT_PROD_SIZE_MAX = 300;
static const uint32 S_DEFAULT_MAX_SIZE = 27;
static const bool S_DEFAULT_LIB_AMBIGUITY_CODES_CONSENSUS = false;
static const bool S_DEFAULT_LIBERAL_BASE = false;
static const uint32 S_DEFAULT_NUM_RETURN = 5;
static const bool S_DEFAULT_EXPLAIN = false;


static NamedParameterType S_PROD_SIZE_MIN = { "Product size range min", PT_UNSIGNED_INT };
static NamedParameterType S_PROD_SIZE_MAX = { "Product size range max", PT_UNSIGNED_INT };
static NamedParameterType S_MAX_SIZE = { "Primer maximum size", PT_UNSIGNED_INT };
static NamedParameterType S_LIB_AMBIGUITY_CODES_CONSENSUS = { "Lib ambiguity code consensus", PT_BOOLEAN };
static NamedParameterType S_LIBERAL_BASE = { "Liberal base", PT_BOOLEAN };
static NamedParameterType S_NUM_RETURN = { "Max number of hits", PT_UNSIGNED_INT };
static NamedParameterType S_EXPLAIN_FLAG = { "Explain results", PT_BOOLEAN };


Primer3Prefs *AllocatePrimer3Prefs (const PolymarkerServiceData *data_p)
{
	Primer3Prefs *prefs_p = (Primer3Prefs *) AllocMemory (sizeof (Primer3Prefs));

	if (prefs_p)
		{
			prefs_p -> pp_product_size_range_min = S_DEFAULT_PROD_SIZE_MIN;
			prefs_p -> pp_product_size_range_max = S_DEFAULT_PROD_SIZE_MAX;
			prefs_p -> pp_max_size = S_DEFAULT_MAX_SIZE;
			prefs_p -> pp_lib_ambiguity_codes_consensus = S_DEFAULT_LIB_AMBIGUITY_CODES_CONSENSUS;
			prefs_p -> pp_liberal_base = S_DEFAULT_LIBERAL_BASE;
			prefs_p -> pp_num_return = S_DEFAULT_NUM_RETURN;
			prefs_p -> pp_explain_flag = S_DEFAULT_EXPLAIN;
			prefs_p -> pp_thermodynamic_parameters_path_s = data_p -> psd_thermodynamic_parameters_path_s;
		}

	return prefs_p;
}


void FreePrimer3Prefs (Primer3Prefs *prefs_p)
{
	FreeMemory (prefs_p);
}


char *SavePrimer3Prefs (Primer3Prefs *prefs_p, const char *path_s)
{
	char *filename_s = MakeFilename (path_s, "primer3.prefs");
	bool success_flag = false;

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
																			if (WriteKeyValuePairForString ("primer_thermodynamic_parameters_path", prefs_p -> pp_thermodynamic_parameters_path_s, out_f))
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

			if (!success_flag)
				{
					FreeCopiedString (filename_s);
					filename_s = NULL;
				}

		}		/* if (filename_s) */

	return filename_s;
}


bool AddPrimer3PrefsParameters (ParameterSet *params_p, PolymarkerServiceData *data_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Primer3 Parameters", false, & (data_p -> psd_base_data), params_p);

	def.st_ulong_value_p = (uint32 *) &S_DEFAULT_PROD_SIZE_MIN;

	if ((param_p = EasyCreateAndAddParameterToParameterSet (& (data_p -> psd_base_data), params_p, group_p, S_PROD_SIZE_MIN.npt_type, S_PROD_SIZE_MIN.npt_name_s, "Product size minimum", "The minimum value of the PCR range", def, PL_ADVANCED)) != NULL)
		{
			def.st_ulong_value_p = (uint32 *) &S_DEFAULT_PROD_SIZE_MAX;

			if ((param_p = EasyCreateAndAddParameterToParameterSet (& (data_p -> psd_base_data), params_p, group_p, S_PROD_SIZE_MAX.npt_type, S_PROD_SIZE_MAX.npt_name_s, "Product size maximum", "The maximum value of the PCR range", def, PL_ADVANCED)) != NULL)
				{
					def.st_ulong_value_p = (uint32 *) &S_DEFAULT_MAX_SIZE;

					if ((param_p = EasyCreateAndAddParameterToParameterSet (& (data_p -> psd_base_data), params_p, group_p, S_MAX_SIZE.npt_type, S_MAX_SIZE.npt_name_s, "Maximum primer size", "Maximum acceptable length (in bases) of a primer. Currently this parameter cannot be larger than 35.", def, PL_ADVANCED)) != NULL)
						{
							def.st_boolean_value_p = (bool *) &S_DEFAULT_LIB_AMBIGUITY_CODES_CONSENSUS;

							if ((param_p = EasyCreateAndAddParameterToParameterSet (& (data_p -> psd_base_data), params_p, group_p, S_LIB_AMBIGUITY_CODES_CONSENSUS.npt_type, S_LIB_AMBIGUITY_CODES_CONSENSUS.npt_name_s, "Ambiguity codes consensus", "If true, treat ambiguity codes as if they were consensus codes when matching oligos to mispriming or mishyb libraries", def, PL_ADVANCED)) != NULL)
								{
									def.st_boolean_value_p = (bool *) &S_DEFAULT_LIBERAL_BASE;

									if ((param_p = EasyCreateAndAddParameterToParameterSet (& (data_p -> psd_base_data), params_p, group_p, S_LIBERAL_BASE.npt_type, S_LIBERAL_BASE.npt_name_s, "Liberal base", "This parameter provides a quick-and-dirty way to get primer3 to accept IUB / IUPAC codes for ambiguous bases (i.e. by changing all unrecognized bases to N). If you wish to include an ambiguous base in an oligo, you must set this to true", def, PL_ADVANCED)) != NULL)
										{
											def.st_ulong_value_p = (uint32 *) &S_DEFAULT_NUM_RETURN;

											if ((param_p = EasyCreateAndAddParameterToParameterSet (& (data_p -> psd_base_data), params_p, group_p, S_NUM_RETURN.npt_type, S_NUM_RETURN.npt_name_s, "Number of hits", "The maximum number of primer (pairs) to return.", def, PL_ADVANCED)) != NULL)
												{
													def.st_boolean_value_p = (bool *) &S_DEFAULT_EXPLAIN;

													if ((param_p = EasyCreateAndAddParameterToParameterSet (& (data_p -> psd_base_data), params_p, group_p, S_EXPLAIN_FLAG.npt_type, S_EXPLAIN_FLAG.npt_name_s, "Explain", "These output tags are intended to provide information on the number of oligos and primer pairs that primer3 examined and counts of the number discarded for various reasons", def, PL_ADVANCED)) != NULL)
														{
															success_flag = true;
														}
												}
										}
								}
						}
				}
		}

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
	else if (strcmp (param_name_s, S_NUM_RETURN.npt_name_s) == 0)
		{
			*pt_p = S_NUM_RETURN.npt_type;
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


char *WritePrimer3Config (const ParameterSet *params_p, const char *prefs_path_s, const PolymarkerServiceData *data_p)
{
	char *filename_s = NULL;
	Primer3Prefs *prefs_p = AllocatePrimer3Prefs (data_p);

	if (prefs_p)
		{
			ParsePrimer3PrefsParameters (params_p, prefs_p);

			if ((filename_s = SavePrimer3Prefs (prefs_p, prefs_path_s)) == NULL)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SavePrimer3Prefs to \"%s\" failed", prefs_path_s);
				}

			FreePrimer3Prefs (prefs_p);
		}		/* if (prefs_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate Primer3Prefs");
		}

	return filename_s;
}


void ParsePrimer3PrefsParameters (const ParameterSet *params_p, Primer3Prefs *prefs_p)
{
	SharedType value;

	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (params_p, S_PROD_SIZE_MIN.npt_name_s, &value, true))
		{
			if (value.st_ulong_value_p)
				{
					prefs_p -> pp_product_size_range_min = *value.st_ulong_value_p;
				}
		}

	if (GetParameterValueFromParameterSet (params_p, S_PROD_SIZE_MAX.npt_name_s, &value, true))
		{
			if (value.st_ulong_value_p)
				{
					prefs_p -> pp_product_size_range_max = *value.st_ulong_value_p;
				}
		}

	if (GetParameterValueFromParameterSet (params_p, S_MAX_SIZE.npt_name_s, &value, true))
		{
			if (value.st_ulong_value_p)
				{
					prefs_p -> pp_max_size = *value.st_ulong_value_p;
				}
		}

	if (GetParameterValueFromParameterSet (params_p, S_LIB_AMBIGUITY_CODES_CONSENSUS.npt_name_s, &value, true))
		{
			if (value.st_boolean_value_p)
				{
					prefs_p -> pp_lib_ambiguity_codes_consensus = *value.st_boolean_value_p;
				}
		}

	if (GetParameterValueFromParameterSet (params_p, S_LIBERAL_BASE.npt_name_s, &value, true))
		{
			if (value.st_boolean_value_p)
				{
					prefs_p -> pp_liberal_base = *value.st_boolean_value_p;
				}
		}

	if (GetParameterValueFromParameterSet (params_p, S_NUM_RETURN.npt_name_s, &value, true))
		{
			if (value.st_boolean_value_p)
				{
					prefs_p -> pp_num_return = *value.st_boolean_value_p;
				}
		}

	if (GetParameterValueFromParameterSet (params_p, S_EXPLAIN_FLAG.npt_name_s, &value, true))
		{
			if (value.st_boolean_value_p)
				{
					prefs_p -> pp_explain_flag = *value.st_boolean_value_p;
				}
		}
}


static char *GetProductSizeRangeAsString (const uint32 min_value, const uint32 max_value)
{
	char *range_s = NULL;
	char *min_s = ConvertUnsignedIntegerToString (min_value);

	if (min_s)
		{
			char *max_s = ConvertUnsignedIntegerToString (max_value);

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
