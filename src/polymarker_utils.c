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


bool CreateMarkerListFile (const char *marker_file_s, const ParameterSet *param_set_p)
{
	bool success_flag = false;
	FILE *marker_f = fopen (marker_file_s, "w");

	if (marker_f)
		{
			SharedType value;

			InitSharedType (&value);

			if (GetParameterValueFromParameterSet (param_set_p, PS_GENE_ID.npt_name_s, &value, true))
				{
					if (fprintf (marker_f, "%s,", value.st_string_value_s) > 0)
						{
							/* The chromosome is optional */
							if (GetParameterValueFromParameterSet (param_set_p, PS_TARGET_CHROMOSOME.npt_name_s, &value, true))
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


							if (GetParameterValueFromParameterSet (param_set_p, PS_SEQUENCE.npt_name_s, &value, true))
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
