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

#include "primer3_prefs.h"
#include "memory_allocations.h"



typedef struct Primer3Prefs
{
	/*
	 * PRIMER_PRODUCT_SIZE_RANGE (size range list; default 100-300)
	 *
	 * The associated values specify the lengths of the product that the user wants the
	 * primers to create, and is a space separated list of elements of the form
	 *
	 * <x>-<y>
	 *
	 */
	uint32 pp_product_size_range_min;

	uint32 pp_product_size_range_max;

	/*
	 * PRIMER_MAX_SIZE (int; default 27)
	 *
	 * Maximum acceptable length (in bases) of a primer. Currently this parameter cannot
	 * be larger than 35. This limit is governed by maximum oligo size for which primer3's
	 * melting-temperature is valid.
	 */
	uint32 pp_max_size;

	bool pp_lib_ambiguity_codes_consensus;

	bool pp_liberal_base;

	uint32 pp_num_return;

	bool pp_explain_flag;

	char *pp_thermodynamic_parameters_path_s;
} Primer3Prefs;



static NamedParameterType S_PROD_SIZE_MIN = { "Product size range min", PT_UNSIGNED_INT };
static NamedParameterType S_PROD_SIZE_MAX = { "Product size range max", PT_UNSIGNED_INT };
static NamedParameterType S_MAX_SIZE = { "Primer maximum size", PT_UNSIGNED_INT };
static NamedParameterType S_LIB_AMBIGUITY_CODES_CONSENSUS = { "Product size range min", PT_BOOLEAN };
static NamedParameterType S_LIBERAL_BASE = { "Product size range max", PT_BOOLEAN };
static NamedParameterType S_PP_NUM_RETURN = { "Primer maximum size", PT_UNSIGNED_INT };
static NamedParameterType S_EXPLAIN_FLAG = { "Product size range min", PT_BOOLEAN };


Primer3Prefs *AllocatePrimer3Prefs (const PolymarkerServiceData *data_p)
{

}


void FreePrimer3Prefs (Primer3Prefs *prefs_p)
{
	if (prefs_p -> pp_thermodynamic_parameters_path_s)
		{
			FreeCopiedString (prefs_p -> pp_thermodynamic_parameters_path_s);
		}

	FreeMemory (prefs_p);
}


bool SavePrimer3Prefs (Primer3Prefs *prefs_p, const char *working_dir_s, const char *job_id_s)
{
	bool success_flag = false;
	char *filename_s = MakeFilename (working_dir_s, job_id_s);

	if (filename_s)
		{

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
	else if (strcmp (param_name_s, S_LIBERAL_BASE.npt_name_s) == 0)
		{
			*pt_p = S_LIBERAL_BASE.npt_type;
		}
	else if (strcmp (param_name_s, S_LIB_AMBIGUITY_CODES_CONSENSUS.npt_name_s) == 0)
		{
			*pt_p = S_LIB_AMBIGUITY_CODES_CONSENSUS.npt_type;
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
