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
 * @file
 */

#ifndef POLYMARKER_SERVICE_H
#define POLYMARKER_SERVICE_H

#include "service.h"
#include "library.h"

/*
** Now we use the generic helper definitions above to define LIB_API and LIB_LOCAL.
** LIB_API is used for the public API symbols. It either DLL imports or DLL exports
** (or does nothing for static build)
** LIB_LOCAL is used for non-api symbols.
*/




#ifdef SHARED_LIBRARY /* defined if LIB is compiled as a DLL */
  #ifdef POLYMARKER_LIBRARY_EXPORTS /* defined if we are building the LIB DLL (instead of using it) */
    #define POLYMARKER_SERVICE_API LIB_HELPER_SYMBOL_EXPORT
  #else
    #define POLYMARKER_SERVICE_API LIB_HELPER_SYMBOL_IMPORT
  #endif /* #ifdef POLYMARKER_LIBRARY_EXPORTS */
  #define POLYMARKER_SERVICE_LOCAL LIB_HELPER_SYMBOL_LOCAL
#else /* SHARED_LIBRARY is not defined: this means LIB is a static lib. */
  #define POLYMARKER_SERVICE_API
  #define POLYMARKER_SERVICE_LOCAL
#endif /* #ifdef SHARED_LIBRARY */


/*
 * The following preprocessor macros allow us to declare
 * and define the variables in the same place. By default,
 * they will expand to
 *
 * 		extern const char *SERVICE_NAME_S;
 *
 * however if ALLOCATE_JSON_TAGS is defined then it will
 * become
 *
 * 		const char *SERVICE_NAME_S = "path";
 *
 * ALLOCATE_JSON_TAGS must be defined only once prior to
 * including this header file. Currently this happens in
 * json_util.c.
 */
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_POLYMARKER_TAGS
	#define POLYMARKER_PREFIX POLYMARKER_SERVICE_API
	#define POLYMARKER_VAL(x)	= x
	#define POLYMARKER_STRUCT_VAL(x,y)	= { x, y}
#else
	#define POLYMARKER_PREFIX extern
	#define POLYMARKER_VAL(x)
	#define POLYMARKER_STRUCT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


typedef enum
{
	PTT_WEB,
	PTT_SYSTEM,
	PTT_NUM_TYPES
} PolymarkerToolType;




typedef struct IndexData
{
	const char *id_blast_db_name_s;
	const char *id_fasta_filename_s;
} IndexData;


typedef struct PolymarkerServiceData
{
	ServiceData psd_base_data;
	PolymarkerToolType psd_tool_type;

	/**
	 * The directory where the Polymarker input, output and log files
	 * will be stored.
	 */
	const char *psd_working_dir_s;


	const char *psd_executable_s;

	IndexData *psd_index_data_p;

	size_t psd_index_data_size;

} PolymarkerServiceData;



POLYMARKER_PREFIX NamedParameterType PS_CONTIG_FILENAME POLYMARKER_STRUCT_VAL ("Contig filename", PT_STRING);
POLYMARKER_PREFIX NamedParameterType PS_GENE_ID POLYMARKER_STRUCT_VAL ("Gene", PT_STRING);
POLYMARKER_PREFIX NamedParameterType PS_TARGET_CHROMOSOME POLYMARKER_STRUCT_VAL ("Chromosome", PT_STRING);
POLYMARKER_PREFIX NamedParameterType PS_SEQUENCE POLYMARKER_STRUCT_VAL ("Sequence", PT_LARGE_STRING);


POLYMARKER_PREFIX const char *PS_TOOL_S POLYMARKER_VAL ("tool");
POLYMARKER_PREFIX const char *PS_TOOL_SYSTEM_S POLYMARKER_VAL ("system");
POLYMARKER_PREFIX const char *PS_TOOL_WEB_S POLYMARKER_VAL ("web");



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Get the ServicesArray containing the Polymarker Services.
 *
 * @param config_p The service configuration data.
 * @return The ServicesArray containing all of the Polymarker Services or
 * <code>NULL</code> upon error.
 * @ingroup polymarker_service
 */
POLYMARKER_SERVICE_API ServicesArray *GetServices (UserDetails *user_p);


/**
 * Free the ServicesArray containing the Polymarker Services.
 *
 * @param services_p The ServicesArray to free.
 * @ingroup polymarker_service
 */
POLYMARKER_SERVICE_API void ReleaseServices (ServicesArray *services_p);

#ifdef __cplusplus
}
#endif



#endif		/* #ifndef POLYMARKER_SERVICE_H */
