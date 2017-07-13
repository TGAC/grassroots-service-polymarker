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
 * polymarker_utils.h
 *
 *  Created on: 13 Jul 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#ifndef SERVICES_POLYMARKER_SERVICE_INCLUDE_POLYMARKER_UTILS_H_
#define SERVICES_POLYMARKER_SERVICE_INCLUDE_POLYMARKER_UTILS_H_


#include "polymarker_service.h"


#ifdef __cplusplus
extern "C"
{
#endif


POLYMARKER_SERVICE_LOCAL bool CreateMarkerListFile (const char *marker_file_s, const ParameterSet *param_set_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_POLYMARKER_SERVICE_INCLUDE_POLYMARKER_UTILS_H_ */
