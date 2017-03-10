/*
 * polymarker_service_job.h
 *
 *  Created on: 7 Feb 2017
 *      Author: billy
 */

#ifndef SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_POLYMARKER_SERVICE_JOB_H_
#define SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_POLYMARKER_SERVICE_JOB_H_


#include "polymarker_service.h"
#include "service_job.h"


class PolymarkerTool;


typedef struct PolymarkerServiceJob
{
	ServiceJob psj_base_job;

	int32 psj_process_id;

	PolymarkerTool *psj_tool_p;

} PolymarkerServiceJob;



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Create a PolymarkerServiceJob.
 *
 * @param service_p The Polymarker service.
 * @param job_name_s The name of the PolymarkerServiceJob.
 * @param job_description_s The description of the PolymarkerServiceJob. This can be <code>NULL</code>.
 * @param data_p The PolymarkerServiceData.
 * @return The newly-allocated PolymarkerServiceJob or <code>NULL</code> upon error.
 */
POLYMARKER_SERVICE_LOCAL PolymarkerServiceJob *AllocatePolymarkerServiceJob (Service *service_p, const char *job_name_s, const char *job_description_s, const char *tool_name_s, PolymarkerServiceData *data_p);


/**
 * Free the PolymarkerServiceJob.
 *
 * @param job_p The PolymarkerServiceJob to free.
 * @memberof PolymarkerServiceJob
 */
POLYMARKER_SERVICE_LOCAL void FreePolymarkerServiceJob (ServiceJob *job_p);


/**
 * Update the running status of PolymarkerServiceJob if needed.
 *
 * @param job_p The PolymarkerServiceJob to check.
 * @return <code>true</code> if the PolymarkerServiceJob was updated successfully,
 * <code>false</code> otherwise.
 * @memberof PolymarkerServiceJob
 */
POLYMARKER_SERVICE_LOCAL bool UpdatePolymarkerServiceJob (ServiceJob *job_p);



/**
 * Create a PolymarkerServiceJob from a JSON-based serialisation.
 *
 * @param service_p The type of Blast Service that previously created the BlastServiceJob.
 * @param service_job_json_p The JSON fragment representing the PolymarkerServiceJob.
 * @return The PolymarkerServiceJob or <code>NULL</code> upon error.
 * @memberof PolymarkerServiceJob
 */
POLYMARKER_SERVICE_LOCAL ServiceJob *GetPolymarkerServiceJobFromJSON (struct Service *service_p, const json_t *service_job_json_p);


/**
 * Get the JSON representation of a PolymarkerServiceJob.
 *
 * @param service_p The Service that ran the PolymarkerServiceJob.
 * @param service_job_p The PolymarkerServiceJob to serialise.
 * @return The JSON fragment representing the PolymarkerServiceJob or <code>NULL</code>
 * upon error.
 * @memberof PolymarkerServiceJob
 */
POLYMARKER_SERVICE_LOCAL json_t *ConvertPolymarkerServiceJobToJSON (Service *service_p, ServiceJob *service_job_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVER_SRC_SERVICES_POLYMARKER_INCLUDE_POLYMARKER_SERVICE_JOB_H_ */
