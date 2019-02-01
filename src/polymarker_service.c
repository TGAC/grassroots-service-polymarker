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
#include <string.h>
#include <time.h>


#define ALLOCATE_POLYMARKER_TAGS (1)
#include "polymarker_service.h"

#include "memory_allocations.h"
#include "string_utils.h"
#include "jobs_manager.h"
#include "byte_buffer.h"
#include "polymarker_service_job.h"
#include "service_config.h"
#include "grassroots_config.h"
#include "provider.h"
#include "service_job_set_iterator.h"
#include "service_job.h"
#include "polymarker_utils.h"
#include "polymarker_tool.hpp"
#include "primer3_prefs.h"


#ifdef _DEBUG
	#define POLYMARKER_SERVICE_DEBUG	(STM_LEVEL_FINEST)
#else
	#define POLYMARKER_SERVICE_DEBUG	(STM_LEVEL_NONE)
#endif


/*
 * bin/polymarker.rb --contigs ~/Applications/grassroots-0/grassroots/extras/blast/databases/Triticum_aestivum_CS42_TGACv1_all.fa --marker_list test/data/short_primer_design_test.csv --output ~/Desktop/polymarker_out
 */


static const char * const PS_SEQUENCE_NAME_S = "sequence";
static const char * const PS_FASTA_FILENAME_S = "fasta";
static const char * const PS_DATABASE_GROUP_NAME_S = "Available contigs";

static const char * const S_DB_SEP_S = " -> ";

/*
 * STATIC PROTOTYPES
 */

static PolymarkerServiceData *AllocatePolymarkerServiceData (Service *service_p);

static void FreePolymarkerServiceData (PolymarkerServiceData *data_p);

static const char *GetPolymarkerServiceName (Service *service_p);

static const char *GetPolymarkerServiceDesciption (Service *service_p);

static const char *GetPolymarkerServiceWebpage (Service *service_p);

static ParameterSet *GetPolymarkerServiceParameters (Service *service_p, Resource *resource_p, UserDetails *user_p);

static void ReleasePolymarkerServiceParameters (Service *service_p, ParameterSet *params_p);

static ServiceJobSet *RunPolymarkerService (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static  ParameterSet *IsFileForPolymarkerService (Service *service_p, Resource *resource_p, Handler *handler_p);

static bool ClosePolymarkerService (Service *service_p);

static bool GetPolymarkerServiceConfig (PolymarkerServiceData *data_p);


static void CustomisePolymarkerServiceJob (Service * UNUSED_PARAM (service_p), ServiceJob *job_p);


static bool GetPolymarkerServiceParameterTypesForNamedParameters (struct Service *service_p, const char *param_name_s, ParameterType *pt_p);

static bool GetDatabaseParameterTypeForNamedParameter (PolymarkerServiceData *data_p, const char *param_name_s, ParameterType *pt_p);



static void PreparePolymarkerServiceJobs (const ParameterSet * const param_set_p, Service *service_p, PolymarkerServiceData *data_p);

static bool RunPolymarkerJob (PolymarkerServiceJob *job_p, ParameterSet *param_set_p, PolymarkerServiceData *data_p);


static char *CreateGroupName (const char *server_s);

static char *GetLocalDatabaseGroupName (void);


static char *GetFullyQualifiedDatabaseName (const char *group_s, const char *db_s);



static uint16 AddDatabaseParams (PolymarkerServiceData *data_p, ParameterSet *param_set_p);


static bool PreRunJobs (PolymarkerServiceData *data_p);

static void SetPolymarkerSequenceConfig (PolymarkerSequence *seq_p, const json_t *config_p);

static ServiceMetadata *GetPolymarkerServiceMetadata (Service *service_p);


static bool CleanupAsyncPolymarkerService (void *data_p);

/*
 * API FUNCTIONS
 */
ServicesArray *GetServices (UserDetails *user_p)
{
	Service *service_p = (Service *) AllocMemory (sizeof (Service));

	if (service_p)
		{
			ServicesArray *services_p = AllocateServicesArray (1);
			
			if (services_p)
				{		
					PolymarkerServiceData *data_p = AllocatePolymarkerServiceData (service_p);
					
					if (data_p)
						{

							if (InitialiseService (service_p,
								GetPolymarkerServiceName,
								GetPolymarkerServiceDesciption,
								GetPolymarkerServiceWebpage,
								RunPolymarkerService,
								IsFileForPolymarkerService,
								GetPolymarkerServiceParameters,
								GetPolymarkerServiceParameterTypesForNamedParameters,
								ReleasePolymarkerServiceParameters,
								ClosePolymarkerService,
								CustomisePolymarkerServiceJob,
								true,
								SY_SYNCHRONOUS,
								(ServiceData *) data_p,
								GetPolymarkerServiceMetadata))
								{
							
									if (GetPolymarkerServiceConfig (data_p))
										{
											* (services_p -> sa_services_pp) = service_p;

											service_p -> se_deserialise_job_json_fn = GetPolymarkerServiceJobFromJSON;
											service_p -> se_serialise_job_json_fn = ConvertPolymarkerServiceJobToJSON;

											return services_p;
										}
								}
						}

					FreeServicesArray (services_p);
				}

			FreeService (service_p);
		}

	return NULL;
}


void ReleaseServices (ServicesArray *services_p)
{
	FreeServicesArray (services_p);
}


/*
 * STATIC FUNCTIONS 
 */
 

static bool GetPolymarkerServiceConfig (PolymarkerServiceData *data_p)
{
	bool success_flag = true;
	const json_t *polymarker_config_p = data_p -> psd_base_data.sd_config_p;

	if (polymarker_config_p)
		{
			json_t *index_files_p;
			const char * const WORKING_DIRECTORY_KEY_S = "working_directory";
			const char * const ALIGNER_KEY_S = "aligner";
			const char *config_value_s = GetJSONString (polymarker_config_p, PS_TOOL_S);


			/*
			 * polymarker tool
			 */
			if (config_value_s)
				{
					if (strcmp (config_value_s, PS_TOOL_WEB_S) == 0)
						{
							data_p -> psd_tool_type = PTT_WEB;
						}
					else if (strcmp (config_value_s, PS_TOOL_SYSTEM_S) == 0)
						{
							data_p -> psd_tool_type = PTT_SYSTEM;
						}
				}

			if (data_p -> psd_tool_type == PTT_SYSTEM)
				{
					Service *service_p = data_p -> psd_base_data.sd_service_p;

					service_p -> se_synchronous = SY_ASYNCHRONOUS_ATTACHED;

					data_p -> psd_task_manager_p = AllocateAsyncTasksManager (GetServiceName (service_p), CleanupAsyncPolymarkerService, service_p);

					if (data_p -> psd_task_manager_p)
						{
							SetServiceReleaseFunction (data_p -> psd_base_data.sd_service_p, ReleasePolymarkerService);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocateAsyncTasksManager failed");
							success_flag = false;
						}
				}

			config_value_s = GetJSONString (polymarker_config_p, WORKING_DIRECTORY_KEY_S);
			if (config_value_s)
				{
					data_p -> psd_working_dir_s = config_value_s;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Polymarker Service requires a working directory specified by the %s", WORKING_DIRECTORY_KEY_S);
					success_flag = false;
				}

			/*
			 * Set the aligner to use
			 */
			data_p -> psd_aligner_s = PS_ALIGNER_EXONERATE_S;
			config_value_s = GetJSONString (polymarker_config_p, ALIGNER_KEY_S);
			if (config_value_s)
				{
					if (strcmp (config_value_s, PS_ALIGNER_BLAST_S) == 0)
						{
							data_p -> psd_aligner_s = PS_ALIGNER_BLAST_S;
						}
					else if (strcmp (config_value_s, PS_ALIGNER_EXONERATE_S) != 0)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Unknown aligner \"%s\", using exonerate", config_value_s);
						}
				}


			/*
			 * Primer3 config
			 */
			data_p -> psd_thermodynamic_parameters_path_s = GetJSONString (polymarker_config_p, "thermodynamic_parameters_path");


			/*
			 * index files
			 */
			index_files_p = json_object_get (polymarker_config_p, "index_files");

			if (index_files_p)
				{
					if (json_is_array (index_files_p))
						{
							size_t size = json_array_size (index_files_p);

							data_p -> psd_index_data_p = (PolymarkerSequence *) AllocMemoryArray (sizeof (PolymarkerSequence), size);

							if (data_p -> psd_index_data_p)
								{
									size_t i;
									json_t *index_file_p;

									json_array_foreach (index_files_p, i, index_file_p)
										{
											SetPolymarkerSequenceConfig ((data_p -> psd_index_data_p) + i, index_file_p);
										}

									data_p -> psd_index_data_size = size;

									success_flag = true;
								}
						}
					else
						{
							if (json_is_object (index_files_p))
								{
									data_p -> psd_index_data_p = (PolymarkerSequence *) AllocMemory (sizeof (PolymarkerSequence));

									if (data_p -> psd_index_data_p)
										{
											SetPolymarkerSequenceConfig (data_p -> psd_index_data_p, index_files_p);

											data_p -> psd_index_data_size = 1;

											success_flag = true;
										}
								}
						}

				}		/* if (index_files_p) */

		}		/* if (polymarker_config_p) */

	return success_flag;
}


static void SetPolymarkerSequenceConfig (PolymarkerSequence *seq_p, const json_t *config_p)
{
	seq_p -> ps_name_s = GetJSONString (config_p, PS_SEQUENCE_NAME_S);
	seq_p -> ps_fasta_filename_s = GetJSONString (config_p, PS_FASTA_FILENAME_S);

	GetJSONBoolean (config_p, "active", & (seq_p -> ps_active_flag));
}


static PolymarkerServiceData *AllocatePolymarkerServiceData (Service * UNUSED_PARAM (service_p))
{
	PolymarkerServiceData *data_p = (PolymarkerServiceData *) AllocMemory (sizeof (PolymarkerServiceData));

	data_p -> psd_tool_type = PTT_SYSTEM;

	data_p -> psd_executable_s = NULL;
	data_p -> psd_index_data_p = NULL;
	data_p -> psd_index_data_size = 0;
	data_p -> psd_working_dir_s = NULL;
	data_p -> psd_task_manager_p = NULL;
	data_p -> psd_tool_type = PTT_NUM_TYPES;

	return data_p;
}


static void FreePolymarkerServiceData (PolymarkerServiceData *data_p)
{
	if (data_p -> psd_index_data_p)
		{
			FreeMemory (data_p -> psd_index_data_p);
		}

	if (data_p -> psd_task_manager_p)
		{
			FreeAsyncTasksManager (data_p -> psd_task_manager_p);
		}

	FreeMemory (data_p);
}

 
static bool ClosePolymarkerService (Service *service_p)
{
	FreePolymarkerServiceData ((PolymarkerServiceData *) (service_p -> se_data_p));

	return true;
}
 
 
static const char *GetPolymarkerServiceName (Service * UNUSED_PARAM (service_p))
{
	return "Polymarker service";
}


static const char *GetPolymarkerServiceDesciption (Service * UNUSED_PARAM (service_p))
{
	return "PolyMarker is an automated bioinformatics pipeline for SNP assay development which increases "
		"the probability of generating homoeologue-specific assays for polyploid wheat. PolyMarker generates "
		"a multiple alignment between the target SNP sequence and the IWGSC chromosome survey sequences (IWGSC, 2014 ) for each of the three wheat genomes. "
		"It then generates a mask with informative positions which are highlighted with respect to the target genome.";
}


static const char *GetPolymarkerServiceWebpage (Service * UNUSED_PARAM (service_p))
{
	return "http://polymarker.tgac.ac.uk/";
}


static ParameterSet *GetPolymarkerServiceParameters (Service *service_p, Resource * UNUSED_PARAM (resource_p), UserDetails * UNUSED_PARAM (user_p))
{
	ParameterSet *param_set_p = AllocateParameterSet ("Polymarker service parameters", "The parameters used for the Polymarker service");
	
	if (param_set_p)
		{
			PolymarkerServiceData *data_p = (PolymarkerServiceData *) (service_p -> se_data_p);
			Parameter *param_p = NULL;
			SharedType def;
			ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet (GetSequenceParametersGroupName (), true, & (data_p -> psd_base_data), param_set_p);


			if (!group_p)
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to create Polymarker service Sequence parameters group");
				}

			def.st_string_value_s = NULL;


			if ((param_p = EasyCreateAndAddParameterToParameterSet (service_p -> se_data_p, param_set_p, NULL, PS_JOB_IDS.npt_type, PS_JOB_IDS.npt_name_s, "Previous job ids", "The ids for previous sets of results", def, PL_ALL)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (service_p -> se_data_p, param_set_p, group_p, PS_GENE_ID.npt_type, PS_GENE_ID.npt_name_s, "Gene ID", "An unique identifier for the assay", def, PL_ALL)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (service_p -> se_data_p, param_set_p, group_p, PS_TARGET_CHROMOSOME.npt_type, PS_TARGET_CHROMOSOME.npt_name_s, "Target Chromosome", "The chromosome to use", def, PL_ALL)) != NULL)
								{
									if ((param_p = EasyCreateAndAddParameterToParameterSet (service_p -> se_data_p, param_set_p, group_p, PS_SEQUENCE.npt_type, PS_SEQUENCE.npt_name_s, "Sequence surrounding the polymorphisms", "The SNP must be marked in the format [A/T] for a varietal SNP with alternative bases, A or T",  def, PL_ALL)) != NULL)
										{
											uint16 num_dbs = AddDatabaseParams (data_p, param_set_p);

											if (num_dbs > 0)
												{
													if (AddPrimer3PrefsParameters (param_set_p, data_p))
														{
															return param_set_p;
														}
												}
										}
								}
						}
				}

			FreeParameterSet (param_set_p);
		}		/* if (param_set_p) */
		
	return NULL;
}


static void ReleasePolymarkerServiceParameters (Service * UNUSED_PARAM (service_p), ParameterSet *params_p)
{
	FreeParameterSet (params_p);
}

static bool GetPolymarkerServiceParameterTypesForNamedParameters (struct Service *service_p, const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (!GetPrimer3PrefsParameterTypesForNamedParameters (service_p, param_name_s, pt_p))
		{
			if (strcmp (param_name_s, PS_JOB_IDS.npt_name_s) == 0)
				{
					*pt_p = PS_JOB_IDS.npt_type;
				}
			else if (strcmp (param_name_s, PS_GENE_ID.npt_name_s) == 0)
				{
					*pt_p = PS_GENE_ID.npt_type;
				}
			else if (strcmp (param_name_s, PS_TARGET_CHROMOSOME.npt_name_s) == 0)
				{
					*pt_p = PS_TARGET_CHROMOSOME.npt_type;
				}
			else if (strcmp (param_name_s, PS_SEQUENCE.npt_name_s) == 0)
				{
					*pt_p = PS_SEQUENCE.npt_type;
				}
			else if (!GetDatabaseParameterTypeForNamedParameter ((PolymarkerServiceData *) (service_p -> se_data_p), param_name_s, pt_p))
				{
					success_flag = false;
				}
		}

	return success_flag;
}


static ServiceJobSet *RunPolymarkerService (Service *service_p, ParameterSet *param_set_p, UserDetails * UNUSED_PARAM (user_p), ProvidersStateTable * UNUSED_PARAM (providers_p))
{
	PolymarkerServiceData *data_p = (PolymarkerServiceData *) (service_p -> se_data_p);
	SharedType value;

	InitSharedType (&value);

	if ((GetParameterValueFromParameterSet (param_set_p, PS_JOB_IDS.npt_name_s, &value, true)) && (!IsStringEmpty (value.st_string_value_s)))
		{
			LinkedList *uuids_p = GetUUIDSList (value.st_string_value_s);

			if (uuids_p)
				{
					service_p -> se_jobs_p = GetPreviousJobResults (uuids_p, data_p);

					if (! (service_p -> se_jobs_p))
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get previous jobs for \"%s\"", value.st_string_value_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to parse \"%s\" for ids", value.st_string_value_s);
				}

		}		/* if (previous_job_ids_s) */
	else
		{
			service_p -> se_jobs_p = AllocateServiceJobSet (service_p);

			if (service_p -> se_jobs_p)
				{
					PreparePolymarkerServiceJobs (param_set_p, service_p, data_p);

					if (GetServiceJobSetSize (service_p -> se_jobs_p) > 0)
						{
							if (PreRunJobs (data_p))
								{
									ServiceJobSetIterator iterator;
									PolymarkerServiceJob *job_p = NULL;

									InitServiceJobSetIterator (&iterator, service_p -> se_jobs_p);

									job_p = (PolymarkerServiceJob *) GetNextServiceJobFromServiceJobSetIterator (&iterator);

									while (job_p)
										{
											if (job_p -> psj_tool_p -> ParseParameters (param_set_p))
												{
													if (!RunPolymarkerJob (job_p, param_set_p, data_p))
														{

														}

												}
											else
												{
													SetServiceJobStatus (& (job_p -> psj_base_job), OS_FAILED_TO_START);
												}

											job_p = (PolymarkerServiceJob *) GetNextServiceJobFromServiceJobSetIterator (&iterator);
										}

								}

						}		/* if (GetServiceJobSetSize (service_p -> se_jobs_p) > 0) */
					else
						{
							PrintErrors (STM_LEVEL_INFO, __FILE__, __LINE__, "No jobs specified");
						}

				}		/* if (service_p -> se_jobs_p) */

		}

	return service_p -> se_jobs_p;
}



static ServiceMetadata *GetPolymarkerServiceMetadata (Service *service_p)
{
	const char *term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "operation_2419";
	SchemaTerm *category_p = AllocateSchemaTerm (term_url_s, "Primer and probe design", "Predict oligonucleotide primers or probes.");

	if (category_p)
		{
			SchemaTerm *subcategory_p = NULL;

			term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "topic_0632";

			subcategory_p = AllocateSchemaTerm (term_url_s, "Probes and primers", "This includes the design of primers for PCR and DNA "
				"amplification or the design of molecular probes. Molecular probes (e.g. a peptide probe or DNA microarray probe) or PCR "
				"primers and hybridisation oligos in a nucleic acid sequence.");


			if (subcategory_p)
				{
					ServiceMetadata *metadata_p = AllocateServiceMetadata (category_p, subcategory_p);

					if (metadata_p)
						{
							SchemaTerm *input_p;

							/* Gene ID */
							term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "data_2295";
							input_p = AllocateSchemaTerm (term_url_s, "Gene ID", "A unique (and typically persistent) identifier of a gene in a database, "
								"that is (typically) different to the gene name/symbol.");

							if (input_p)
								{
									if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p))
										{
											/* Target chromosome */
											term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "data_0987";
											input_p = AllocateSchemaTerm (term_url_s, "Chromosome name", "Name of a chromosome.");

											if (input_p)
												{
													if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p))
														{

															/* Sequence */
															term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "data_2044";
															input_p = AllocateSchemaTerm (term_url_s, "Sequence", "This concept is a placeholder of concepts for primary sequence "
																"data including raw sequences and sequence records. It should not normally be used for derivatives such as sequence "
																"alignments, motifs or profiles. One or more molecular sequences, possibly with associated annotation.");

															if (input_p)
																{
																	if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p))
																		{
																			SchemaTerm *output_p;

																				term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "data_0863";
																				output_p = AllocateSchemaTerm (term_url_s, "Sequence alignment", "Alignment of multiple molecular sequences.");

																				if (output_p)
																					{
																						if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p))
																							{
																								return metadata_p;
																							}		/* if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p)) */
																						else
																							{
																								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add output term %s to service metadata", term_url_s);
																								FreeSchemaTerm (output_p);
																							}

																					}		/* if (output_p) */
																				else
																					{
																						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate output term %s for service metadata", term_url_s);
																					}

																		}		/* if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p)) */
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add input term %s to service metadata", term_url_s);
																			FreeSchemaTerm (input_p);
																		}

																}		/* if (input_p) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate input term %s for service metadata", term_url_s);
																}

														}		/* if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p)) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add input term %s to service metadata", term_url_s);
															FreeSchemaTerm (input_p);
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate input term %s for service metadata", term_url_s);
												}

										}		/* if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p)) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add input term %s to service metadata", term_url_s);
											FreeSchemaTerm (input_p);
										}

								}		/* if (input_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate input term %s for service metadata", term_url_s);
								}

						}		/* if (metadata_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate service metadata");
						}

				}		/* if (subcategory_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate subcategory term %s for service metadata", term_url_s);
				}

		}		/* if (category_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate category term %s for service metadata", term_url_s);
		}

	return NULL;
}


static bool RunPolymarkerJob (PolymarkerServiceJob *job_p, ParameterSet *param_set_p, PolymarkerServiceData *data_p)
{
	bool success_flag = false;

	/*
	 *  bin/polymarker.rb --contigs ~/Applications/grassroots-0/grassroots/extras/blast/databases/IWGSC_CSS_all_scaff_v1.fa --marker_list test/data/billy_primer_design_test.csv --output polymarker_out/
	 *
	 */
	OperationStatus status = RunPolymarkerTool (job_p -> psj_tool_p);

	switch (status)
		{
			case OS_STARTED:
			case OS_PENDING:
			case OS_FINISHED:
			case OS_PARTIALLY_SUCCEEDED:
			case OS_SUCCEEDED:
				success_flag = true;
				break;

			default:
				break;
		}

	return success_flag;
}


static  ParameterSet *IsFileForPolymarkerService (Service * UNUSED_PARAM (service_p), Resource * UNUSED_PARAM (resource_p), Handler * UNUSED_PARAM (handler_p))
{
	return NULL;
}


static void CustomisePolymarkerServiceJob (Service * UNUSED_PARAM (service_p), ServiceJob *job_p)
{
	job_p -> sj_update_fn = UpdatePolymarkerServiceJob;
	job_p -> sj_free_fn = FreePolymarkerServiceJob;
}


static void PreparePolymarkerServiceJobs (const ParameterSet * const param_set_p, Service *service_p, PolymarkerServiceData *data_p)
{
	PolymarkerSequence *db_p = data_p -> psd_index_data_p;
	size_t i = 0;
	char *group_s = GetLocalDatabaseGroupName ();

	for (i = data_p -> psd_index_data_size; i > 0; -- i, ++ db_p)
		{
			char *db_s = GetFullyQualifiedDatabaseName (group_s ? group_s : PS_DATABASE_GROUP_NAME_S, db_p -> ps_name_s);

			if (db_s)
				{
					Parameter *param_p = GetParameterFromParameterSetByName (param_set_p, db_s);

					/* Do we have a matching parameter? */
					if (param_p)
						{
							/* Is the database selected to search against? */
							if (param_p -> pa_current_value.st_boolean_value)
								{
									PolymarkerServiceJob *job_p = AllocatePolymarkerServiceJob (service_p, db_p, data_p);

									if (job_p)
										{
											if (!AddServiceJobToService (service_p, (ServiceJob *) job_p, false))
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add ServiceJob to the ServiceJobSet for \"%s\"", db_s);
													FreePolymarkerServiceJob (& (job_p -> psj_base_job));
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create ServiceJob for \"%s\"", db_s);
										}

								}		/* if (param_p -> pa_current_value.st_boolean_value) */

						}		/* if (param_p) */

					FreeCopiedString (db_s);
				}		/* if (db_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetFullyQualifiedDatabaseName failed for \"%s\" and \"%s\"", group_s ? group_s : PS_DATABASE_GROUP_NAME_S, db_p -> ps_name_s);

				}

		}

	if (group_s)
		{
			FreeCopiedString (group_s);
		}
}




/*
 * The list of databases that can be searched
 */
static uint16 AddDatabaseParams (PolymarkerServiceData *data_p, ParameterSet *param_set_p)
{
	uint16 num_added_databases = 0;

	if (data_p -> psd_index_data_size > 0)
		{
			ParameterGroup *group_p = NULL;
			PolymarkerSequence *db_p = data_p -> psd_index_data_p;
			SharedType def;
			size_t i = 0;
			char *group_s = GetLocalDatabaseGroupName ();

			group_p = CreateAndAddParameterGroupToParameterSet (group_s, false, & (data_p -> psd_base_data), param_set_p);

			InitSharedType (&def);

			for (i = data_p -> psd_index_data_size; i > 0; -- i, ++ db_p)
				{
					char *db_s = GetFullyQualifiedDatabaseName (group_s ? group_s : PS_DATABASE_GROUP_NAME_S, db_p -> ps_name_s);

					if (db_s)
						{
							def.st_boolean_value = db_p -> ps_active_flag;

							if (EasyCreateAndAddParameterToParameterSet (& (data_p -> psd_base_data), param_set_p, group_p, PT_BOOLEAN, db_s, db_p -> ps_name_s, db_p -> ps_description_s, def, PL_ALL))
								{
									++ num_added_databases;
								}
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add database \"%s\"", db_p -> ps_name_s);
								}

							FreeCopiedString (db_s);
						}

				}


			if (group_s)
				{
					FreeCopiedString (group_s);
				}

		}		/* if (num_group_params) */

	return num_added_databases;
}



static bool GetDatabaseParameterTypeForNamedParameter (PolymarkerServiceData *data_p, const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = false;

	if (data_p -> psd_index_data_size > 0)
		{
			char *group_s = GetLocalDatabaseGroupName ();
			PolymarkerSequence *db_p = data_p -> psd_index_data_p;
			size_t i = data_p -> psd_index_data_size;

			while ((i > 0) && (!success_flag))
				{
					char *db_s = GetFullyQualifiedDatabaseName (group_s ? group_s : PS_DATABASE_GROUP_NAME_S, db_p -> ps_name_s);

					if (db_s)
						{
							if (strcmp (param_name_s, db_s) == 0)
								{
									*pt_p = PT_BOOLEAN;
									success_flag = true;
								}

							FreeCopiedString (db_s);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetFullyQualifiedDatabaseName failed for \"%s\" and \"%s\"", group_s ? group_s : PS_DATABASE_GROUP_NAME_S, db_p -> ps_name_s);
						}

					++ db_p;
					-- i;
				}

			if (group_s)
				{
					FreeCopiedString (group_s);
				}

		}		/* if (num_group_params) */

	return success_flag;
}


static char *CreateGroupName (const char *server_s)
{
	char *group_name_s = ConcatenateVarargsStrings (PS_DATABASE_GROUP_NAME_S, " provided by ", server_s, NULL);

	if (group_name_s)
		{
			#if PAIRED_BLAST_SERVICE_DEBUG >= STM_LEVEL_FINER
			PrintLog (STM_LEVEL_FINER, __FILE__, __LINE__, "Created group name \"%s\" for \"%s\" and \"%s\"", group_name_s, server_s);
			#endif
		}
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to create group name for \"%s\"", group_name_s);
		}

	return group_name_s;
}


static char *GetLocalDatabaseGroupName (void)
{
	char *group_s = NULL;
	const json_t *provider_p = GetGlobalConfigValue (SERVER_PROVIDER_S);

	if (provider_p)
		{
			const char *provider_s = GetProviderName (provider_p);

			if (provider_s)
				{
					group_s = CreateGroupName (provider_s);
				}
		}		/* if (provider_p) */

	return group_s;
}


static char *GetFullyQualifiedDatabaseName (const char *group_s, const char *db_s)
{
	char *fq_db_s = ConcatenateVarargsStrings (group_s ? group_s : PS_DATABASE_GROUP_NAME_S, S_DB_SEP_S, db_s, NULL);

	return fq_db_s;
}



static bool PreRunJobs (PolymarkerServiceData *data_p)
{
	if (data_p -> psd_task_manager_p)
		{
			/*
			 * Set the initial counter value to 1 which will take the system's call
			 * to ReleaseService () into account once it has finished with this
			 * Service. This is to make sure that the Service doesn't delete itself
			 * after it has finished running all of its jobs asynchronously before
			 * the system has finished with it and then accessing freed memory.
			 */
			PrepareAsyncTasksManager (data_p -> psd_task_manager_p, 1);

			/* If we have asynchronous jobs running then set the "is running" flag for this service */
			SetServiceRunning (data_p ->  psd_base_data.sd_service_p, true);
		}

	return true;
}



static bool CleanupAsyncPolymarkerService (void *data_p)
{
	Service *polymarker_service_p = (Service *) data_p;

	FreeService (polymarker_service_p);

	return true;
}


void ReleasePolymarkerService (Service *service_p)
{
	PolymarkerServiceData *data_p = (PolymarkerServiceData *) service_p -> se_data_p;

	if (data_p -> psd_task_manager_p)
		{
			IncrementAsyncTaskManagerCount (data_p -> psd_task_manager_p);
		}		/* if (data_p -> psd_task_manager_p) */
}

