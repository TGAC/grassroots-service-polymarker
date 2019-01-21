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
 * primer3_prefs.h
 *
 *  Created on: 19 Jan 2019
 *      Author: billy
 */

#ifndef SERVICES_POLYMARKER_SERVICE_INCLUDE_PRIMER3_PREFS_H_
#define SERVICES_POLYMARKER_SERVICE_INCLUDE_PRIMER3_PREFS_H_

#include "polymarker_service.h"


/*
:primer_product_size_range => "50-150" ,
:primer_max_size => 25 ,
:primer_lib_ambiguity_codes_consensus => 1,
:primer_liberal_base => 1,
:primer_num_return=>5,
:primer_explain_flag => 1,
:primer_thermodynamic_parameters_path=>File.expand_path(File.dirname(__FILE__) + '../../conf/primer3_config/') + '/'
*/
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

	const char *pp_thermodynamic_parameters_path_s;
} Primer3Prefs;


#ifdef __cplusplus
extern "C"
{
#endif


POLYMARKER_SERVICE_LOCAL Primer3Prefs *AllocatePrimer3Prefs (const PolymarkerServiceData *data_p);


POLYMARKER_SERVICE_LOCAL void FreePrimer3Prefs (Primer3Prefs *prefs_p);


POLYMARKER_SERVICE_LOCAL bool SavePrimer3Prefs (Primer3Prefs *prefs_p, const char *working_dir_s, const char *job_id_s);


POLYMARKER_SERVICE_LOCAL bool AddPrimer3PrefsParameters (ParameterSet *params_p, PolymarkerServiceData *data_p);


POLYMARKER_SERVICE_LOCAL bool GetPrimer3PrefsParameterTypesForNamedParameters (struct Service *service_p, const char *param_name_s, ParameterType *pt_p);


POLYMARKER_SERVICE_LOCAL bool ParsePrimer3PrefsParameters (const ParameterSet *params_p, Primer3Prefs *prefs_p);


POLYMARKER_SERVICE_LOCAL bool WritePrimer3Config (const ParameterSet *params_p, const char *prefs_path_s, const PolymarkerServiceData *data_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_POLYMARKER_SERVICE_INCLUDE_PRIMER3_PREFS_H_ */
