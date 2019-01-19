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

#include "async_tasks_manager.h"

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
 * ALLOCATE_POLYMARKER_TAGS must be defined only once prior to
 * including this header file. Currently this happens in
 * polymarker_service.c.
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

/**
 * An enum listing the different types of PolymarkerTool
 * that are available
 */
typedef enum
{
	/** The web-based tool */
	PTT_WEB,

	/**
	 * Run the Polymarker program directly on the server.
	 */
	PTT_SYSTEM,

	/** The number of different PolymarkerTools available */
	PTT_NUM_TYPES
} PolymarkerToolType;



/**
 * A datatype that stores the information of sequence data
 * that the PolymarkerService can run with.
 */
typedef struct PolymarkerSequence
{
	/** The name of the database to display to the user. */
	const char *ps_name_s;

	/** The filename of the fasta file for this sequence. */
	const char *ps_fasta_filename_s;

	/** The description of the database to display to the user. */
	const char *ps_description_s;

	/**
	 * Sets whether the PolymarkerSequence defaults to being searched against
	 * or not.
	 */
	bool ps_active_flag;

} PolymarkerSequence;


/**
 * The ServiceData used for the PolymarkerService.
 */
typedef struct PolymarkerServiceData
{
	/** The base ServiceData. */
	ServiceData psd_base_data;

	/** The type of tool that this PolymarkerService will use. */
	PolymarkerToolType psd_tool_type;

	/**
	 * The directory where the Polymarker input, output and log files
	 * will be stored.
	 */
	const char *psd_working_dir_s;


	/**
	 * If the system-based PolymarkerTool is to be used, this is the executable
	 * that the system-based PolymarkerTool will use.
	 */
	const char *psd_executable_s;


	/**
	 * Polymarker can use either blast or exonerate as its aligner
	 */
	const char *psd_aligner_s;


	/**
	 * File with preferences to be sent to primer3.
	 */
	const char *psd_primer_config_file_s;

	/**
	 * An array of available PolymarkerSequence objects that the PolymarkerService
	 * can run against.
	 */
	PolymarkerSequence *psd_index_data_p;

	/**
	 * The number of PolymarkerSequence objects stored in psd_index_data_p.
	 */
	size_t psd_index_data_size;


	/**
	 * The AsyncTasksManager used for coordinating the PolymarkerServiceJobs
	 * being run.
	 */
	AsyncTasksManager *psd_task_manager_p;

} PolymarkerServiceData;


/**
 * The NamedParameterType for the contig filename parameter.
 */
POLYMARKER_PREFIX NamedParameterType PS_CONTIG_FILENAME POLYMARKER_STRUCT_VAL ("Contig filename", PT_STRING);


/**
 * The NamedParameterType for the gene ID parameter.
 */
POLYMARKER_PREFIX NamedParameterType PS_GENE_ID POLYMARKER_STRUCT_VAL ("Gene", PT_STRING);

/**
 * The NamedParameterType for the target chromosome parameter.
 */
POLYMARKER_PREFIX NamedParameterType PS_TARGET_CHROMOSOME POLYMARKER_STRUCT_VAL ("Chromosome", PT_STRING);


/**
 * The NamedParameterType for the sequence parameter.
 */
POLYMARKER_PREFIX NamedParameterType PS_SEQUENCE POLYMARKER_STRUCT_VAL ("Sequence", PT_LARGE_STRING);


/**
 * The NamedParameterType for the parameter used for retrieving the results of
 * previously-run jobs.
 */
POLYMARKER_PREFIX NamedParameterType PS_JOB_IDS POLYMARKER_STRUCT_VAL ("Previous results", PT_LARGE_STRING);


/** The constant string for configuring the tool that Polymarker will use. */
POLYMARKER_PREFIX const char *PS_TOOL_S POLYMARKER_VAL ("tool");

/** The constant string for denoting that Polymarker will use the system-based tool. */
POLYMARKER_PREFIX const char *PS_TOOL_SYSTEM_S POLYMARKER_VAL ("system");

/** The constant string for denoting that Polymarker will use the web-based tool. */
POLYMARKER_PREFIX const char *PS_TOOL_WEB_S POLYMARKER_VAL ("web");

/** The constant string for denoting that Polymarker will use the blast aligner. */
POLYMARKER_PREFIX const char *PS_ALIGNER_BLAST_S POLYMARKER_VAL ("blast");

/** The constant string for denoting that Polymarker will use the exonerate aligner. */
POLYMARKER_PREFIX const char *PS_ALIGNER_EXONERATE_S POLYMARKER_VAL ("exonerate");


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Get the ServicesArray containing the Polymarker Services.
 *
 * @param user_p The UserDetails for the user trying to access the services.
 * This can be <code>NULL</code>.
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


POLYMARKER_SERVICE_LOCAL void ReleasePolymarkerService (Service *service_p);


#ifdef __cplusplus
}
#endif



#endif		/* #ifndef POLYMARKER_SERVICE_H */
