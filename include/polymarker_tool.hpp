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
 * polymarker_tool.hpp
 *
 *  Created on: 6 Feb 2017
 *      Author: billy
 *
 * @file
 * @brief
 */

#ifndef SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_POLYMARKER_TOOL_HPP_
#define SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_POLYMARKER_TOOL_HPP_


#include "polymarker_service.h"
#include "parameter_set.h"
#include "service_job.h"
#include "polymarker_service_job.h"
#include "primer3_prefs.h"


class PolymarkerFormatter;

/**
 * The base class for the object that will actually run the Polymarker application
 */
class POLYMARKER_SERVICE_LOCAL PolymarkerTool
{
public:
	PolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p, const PolymarkerServiceData *data_p);

	PolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *seq_p, const PolymarkerServiceData *data_p, const json_t *root_p);

	/**
	 * The PolymarkerTool destructor
	 */
	virtual ~PolymarkerTool ();

	/**
	 * Get the name of this PolymarkerTool.
	 *
	 * @return the name
	 */
	const char *GetName ();

	/**
	 * Parse a ParameterSet to set the variables that the PolymarkerTool's ServiceJob
	 * will run with.
	 *
	 * @param param_set_p The ParameterSet that the variables will be set from.
	 * @return <code>true</code> if the required variables were collected successfully, <code>
	 * false</code> otherwise
	 */
	virtual bool ParseParameters (const ParameterSet * const param_set_p) = 0;

	/**
	 * The function that will be called before trying to run this PolymarkerTool.
	 *
	 * @return <code>true</code> if the call was successful, <code>
	 * false</code> otherwise
	 */
	virtual bool PreRun ();

	virtual OperationStatus Run () = 0;

	/**
	 * The function that will be called after running this PolymarkerTool.
	 *
	 * @return <code>true</code> if the call was successful, <code>
	 * false</code> otherwise
	 */
	virtual bool PostRun () = 0;

	/**
	 * Get the OperationStatus for the ServiceJob that this PolymarkerTool is running.
	 *
	 * @param update_flag If this is <code>true</code>, then the ServiceJob will be checked
	 * for its latest status. If this is <code>false</code>, then the last cached value will
	 * be used.
	 * @return The OperationStatus for this PolymarkerTool's ServiceJob.
	 */
	virtual OperationStatus GetStatus (bool update_flag) = 0;


	/**
	 * Get the log after the PolymarkerTool has finished
	 * running.
	 *
	 * @return The logging messages as a c-style string or 0 upon error.
	 */
	virtual char *GetLog () = 0;


	/**
	 * Get the results from the run of this PolymarkerTool.
	 *
	 * @param formatter_p The PolymarkerFormatter used to
	 * @return The results as a c-style string or 0 upon error.
	 */
	virtual char *GetResults (PolymarkerFormatter *formatter_p) = 0;

	/**
	 * Add the required information for this PolymarkerTool to be serialised
	 * to JSON and deserialised again.
	 *
	 * This is called by each child class of PolymarkerTool all the way down
	 * to the actual PolymarkerTool child object that is being used.
	 *
	 * @param root_p The JSON fragment that the required details will be added to
	 * @return <code>true</code> if the information was added successfully, <code>
	 * false</code> otherwise
	 */
	virtual bool AddToJSON (json_t *root_p);

	/**
	 * Get the PolymarkerToolType for this PolymarkerTool.
	 *
	 * @return The PolymarkerToolType.
	 */
	virtual PolymarkerToolType GetToolType () const = 0;

	bool AddSectionToResult (json_t *result_p, const char * const filename_s, const char * const key_s, PolymarkerFormatter *formatter_p);

	/**
	 * Set the PolymarkerSequence that this PolymarkerTool will run against.
	 *
	 * @param seq_p The PolymarkerSequence to use.
	 */
	void SetPolymarkerSequence (const PolymarkerSequence *seq_p);


	bool SaveJobMetadata () const;

	bool SetJobMetadata ();


	bool SetJobUUID (const uuid_t id);

protected:
	/**
	 * The PolymarkerServiceJob that this PolymarkerTool will run.
	 */
	PolymarkerServiceJob *pt_service_job_p;

	/**
	 * The PolymarkerSequence that this PolymarkerTool's ServiceJob
	 * will be running against.
	 */
	const PolymarkerSequence *pt_seq_p;

	/**
	 * The PolymarkerServiceData for the PolymarkerService that will
	 * be running this PolymarkerTool.
	 */
	const PolymarkerServiceData *pt_service_data_p;

	/**
	 * The local directory where the results and logging data will be stored.
	 */
	char *pt_job_dir_s;


	Primer3Prefs *pt_primer_prefs_p;

	/**
	 * The key used for specifying the PolymarkerTool's job directory within
	 * and JSON-based serialisations of a PolymarkerTool.
	 */
	static const char * const PT_JOB_DIR_S;

private:
	static const char * const PT_METADATA_FILENAME_S;
};


#ifdef __cplusplus
extern "C"
{
#endif


POLYMARKER_SERVICE_LOCAL PolymarkerTool *CreatePolymarkerTool (PolymarkerServiceJob *job_p, const PolymarkerSequence *sequence_p, PolymarkerServiceData *data_p);

POLYMARKER_SERVICE_LOCAL PolymarkerTool *CreatePolymarkerToolFromJSON (PolymarkerServiceJob *job_p, const PolymarkerSequence *sequence_p, PolymarkerServiceData *data_p, const json_t *service_job_json_p);

/**
 * Free a given PolymarkerTool.
 *
 * This is simply a C-wrapper function around the PolymarkerTool destructor.
 *
 * @param tool_p The PolymarkerTool to free
 * @memberof PolymarkerTool
 */
POLYMARKER_SERVICE_LOCAL void FreePolymarkerTool (PolymarkerTool *tool_p);


/**
 * Run a given PolymarkerTool.
 *
 * This is simply a C-wrapper function around the PolymarkerTool::run() function.
 *
 * @param tool_p The PolymarkerTool to run.
 * @return The OperationStatus of the PolymarkerTool.
 * @memberof PolymarkerTool
 */
POLYMARKER_SERVICE_LOCAL OperationStatus RunPolymarkerTool (PolymarkerTool *tool_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_POLYMARKER_TOOL_HPP_ */
