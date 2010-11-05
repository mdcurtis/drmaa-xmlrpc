//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * drmaa-xmlrpc.c
 *
 *  Created on: Oct 5, 2010
 *      Author: levk
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DRMAA_XMLRPC_CGI
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>
#else // DRMAA_XMLRPC_CGI
#include <xmlrpc.h>
#include <xmlrpc_cgi.h>
#endif // DRMAA_XMLRPC_CGI

#include <drmaa.h>
#include <sys/signal.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include "dxconfig.h"
#include "dxlog.h"

#define ERROR_DIAGNOSIS_MAX DRMAA_ERROR_STRING_BUFFER
#define INIT_ERROR_BUFFER(X) char X[ERROR_DIAGNOSIS_MAX]; memset (X, 0, ERROR_DIAGNOSIS_MAX)
#define INIT_JOBID(X) char X[DRMAA_JOBNAME_BUFFER]; memset (X, 0, DRMAA_JOBNAME_BUFFER)

#include "dxopqserial.h"

/**
 * @return rc,jt,error
 */
static xmlrpc_value *
xmlrpc_drmaa_allocate_job_template (xmlrpc_env * const env,
                                    xmlrpc_value * const param_array,
                                    void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  drmaa_job_template_t *jt = NULL;
  INIT_ERROR_BUFFER(error);
  serialized_job_template_t sjt;

  int rc;
  while ((rc = drmaa_allocate_job_template (&jt, error, sizeof (error)))
         == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

  LOG (TEMPLATE | (rc ? WARNING : 0),
       "allocate job template @%p%s%s\n",
       jt, rc ? "; non-successful return code, with diagnostic: " : "", error);
  xmlrpc_value *result = xmlrpc_build_value (env,
                                             "((si)(s" JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE ")(ss))",
                                             "rc", rc,
                                             "job-template", sjt = serialize_job_template (jt),
                                             "error", error);
  release_serialized_job_template (sjt);
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_allocate_job_template() is not supported by this compilation
  LOG (TEMPLATE | INFO, "allocate job is not supported with this compilation");
  return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                             "xmlrpc_drmaa_allocate_job_template is not supported on this compilation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param jt
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_delete_job_template (xmlrpc_env * const env,
                                  xmlrpc_value * const param_array,
                                  void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  serialized_job_template_t sjt;
  xmlrpc_decompose_value (env, param_array,
                          "(" JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE ")",
                          &sjt);
  if (!env->fault_occurred) {
	drmaa_job_template_t *jt = deserialize_job_template (sjt);
    release_serialized_job_template (sjt);
    if (jt != NULL) {
      int rc;
      while ((rc = drmaa_delete_job_template (jt, error, sizeof (error)))
             == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
      LOG (TEMPLATE | (rc ? WARNING : 0),
           "delete a job template @%p%s%s\n",
           jt, rc ? "; non-successful return code, with diagnostic: " : "", error);
      return xmlrpc_build_value (env,
                                 "((si)(ss))",
                                 "rc", rc, "error", error);
    } else {
      LOG (TEMPLATE | WARNING, "fault occurred while deserializing job template in delete template\n");
      return xmlrpc_build_value (env, "((si)(ss))", "rc", -2, "error",
                                 "fault occurred while deserializing job template");
    }
  } else {
    LOG (TEMPLATE | WARNING, "fault occurred while decomposing parameter in delete template\n");
    return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                                    "fault occurred while decomposing parameter");
  }
#else // DRMAA_XMLRPC_CGI
#warning drmaa_delete_job_template() is not supported by this compilation
  LOG (TEMPLATE | INFO, "delete job is not supported with this compilation");
  return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                             "xmlrpc_drmaa_delete_job_template is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param jt,name,value
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_set_attribute (xmlrpc_env * const env,
                            xmlrpc_value * const param_array,
                            void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  serialized_job_template_t sjt;
  char *name = NULL;
  char *value = NULL;
  xmlrpc_value *result = NULL;
  xmlrpc_decompose_value (env, param_array,
                          "(" JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE "ss)",
                          &sjt, &name, &value);
  if (!env->fault_occurred) {
	drmaa_job_template_t *jt = deserialize_job_template (sjt);
    release_serialized_job_template (sjt);
    if (jt != NULL) {
      int rc;
      while ((rc = drmaa_set_attribute(jt, name, value, error, sizeof (error)))
             == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
      LOG (TEMPLATE | (rc ? WARNING : 0),
           "set attribute %s=%s for job template @%p%s%s\n",
           name, value,
           jt, rc ? "; non-successful return code, with diagnostic: " : "", error);
      result = xmlrpc_build_value (env,
                                   "((si)(ss))",
                                   "rc", rc, "error", error);
    } else {
      LOG (TEMPLATE | WARNING, "fault occurred while deserializing job template in set attribute\n");
      result = xmlrpc_build_value (env, "((si)(ss))", "rc", -2, "error",
                                   "fault occurred while deserializing job template");
    }
  } else {
    LOG (TEMPLATE | WARNING, "fault occurred while decomposing parameters in set attribute\n");
    result = xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                                 "fault occurred while decomposing parameters");
  }
  free (name);
  free (value);
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_set_attribute() is not supported by this compilation
  return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                             "xmlrpc_drmaa_set_attribute is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param jt,name,[values...]
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_set_vector_attribute (xmlrpc_env * const env,
                            xmlrpc_value * const param_array,
                            void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  xmlrpc_value *x_sjt;
  xmlrpc_value *x_name;
  xmlrpc_value *x_values;
  serialized_job_template_t sjt;
  char *name = NULL;
  char **values = NULL;
  xmlrpc_array_read_item (env, param_array, 0, &x_sjt);
  xmlrpc_array_read_item (env, param_array, 1, &x_name);
  xmlrpc_array_read_item (env, param_array, 2, &x_values);
  xmlrpc_decompose_value (env, x_sjt, JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE, &sjt); xmlrpc_DECREF (x_sjt);
  xmlrpc_decompose_value (env, x_name, "s", &name); xmlrpc_DECREF (x_name);
  int values_count = 0;
  xmlrpc_value *result = NULL;
  if ((values = (char**) malloc (sizeof (char*) * (1 + (values_count = xmlrpc_array_size (env, x_values)))))) {
    int index;
    for (index = 0; index < values_count + 1; index++) values [index] = NULL;
    for (index = 0; index < values_count; index++) {
      xmlrpc_value *x_value;
      xmlrpc_array_read_item (env, x_values, index, &x_value);
      xmlrpc_decompose_value (env, x_value, "s", &(values [index]));
      xmlrpc_DECREF (x_value);
    }
    xmlrpc_DECREF (x_values);

    if (!env->fault_occurred) {
	  drmaa_job_template_t *jt = deserialize_job_template (sjt);
      release_serialized_job_template (sjt);
      if (jt != NULL) {
        int rc;
        while ((rc = drmaa_set_vector_attribute(jt, name, (const char**) values, error, sizeof (error)))
               == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
        result = xmlrpc_build_value (env,
                                     "((si)(ss))",
                                     "rc", rc, "error", error);
        LOG (TEMPLATE | (rc ? WARNING : 0),
             "set vector attribute %s=[%s%s%s%s%s%s%s%s%s] for job template @%p%s%s\n",
             name,
             values_count > 0 ? values[0] : "",
             values_count > 1 ? ", " : "", values_count > 1 ? values[1] : "",
             values_count > 2 ? ", " : "", values_count > 2 ? values[2] : "",
             values_count > 3 ? ", " : "", values_count > 3 ? values[3] : "",
             values_count > 4 ? ", " : "", values_count > 4 ? (values_count > 5 ? "..." : values[4]) : "",
             jt, rc ? "; non-successful return code, with diagnostic: " : "", error);
      } else {
        LOG (TEMPLATE | WARNING, "fault occurred while deserializing job template in set vector attribute\n");
        result = xmlrpc_build_value (env, "((si)(ss))", "rc", -2, "error",
                                     "fault occurred while deserializing job template");
      }
    } else {
      LOG (TEMPLATE | WARNING, "fault occurred while decomposing parameters in set vector attribute\n");
      result = xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                                   "fault occurred while decomposing parameters");
    }
  } else {
    LOG (TEMPLATE | WARNING, "failed to allocate memory for values array in set vector attribute\n");
    result = xmlrpc_build_value (env, "(si)(ss)", "rc", -1, "error",
                                 "failed to allocate memory for values array");
  }

  if (values) {
	  int i;
	  for (i = values_count; --i >= 0; free (values [i]));
  }
  free (name);
  free (values);
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_set_vector_attribute() is not supported by this compilation
  return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                             "xmlrpc_drmaa_set_vector_attribute is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param jt
 * @return rc,jobid,error
 */
static xmlrpc_value *
xmlrpc_drmaa_run_job (xmlrpc_env * const env,
                      xmlrpc_value * const param_array,
                      void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  serialized_job_template_t sjt;
  xmlrpc_decompose_value (env, param_array,
                          "(" JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE ")",
                          &sjt);
  if (!env->fault_occurred) {
	drmaa_job_template_t *jt = deserialize_job_template (sjt);
    release_serialized_job_template (sjt);
    if (jt != NULL) {
      int rc;
      INIT_JOBID(jobid);
      while ((rc = drmaa_run_job(jobid, sizeof(jobid), jt, error, sizeof(error)))
             == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
      LOG (TEMPLATE | JOB | (rc ? WARNING : 0),
           "start job template @%p with jobid %s%s%s\n",
           jt, jobid, rc ? "; non-successful return code, with diagnostic: " : "", error);
      return xmlrpc_build_value (env,
                                 "((si)(ss)(ss))",
                                 "rc", rc, "jobid", jobid, "error", error);
    } else {
      LOG (TEMPLATE | JOB | WARNING, "fault occurred while deserializing job template in run job\n");
      return xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -2, "jobid", "not available", "error"
                                 "fault occurred while deserializing job template");
    }
  } else {
    LOG (TEMPLATE | JOB | WARNING, "fault occurred while decomposing parameterin run job\n");
    return xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "jobid", "not available", "error",
                               "fault occurred while decomposing parameter");
  }
#else // DRMAA_XMLRPC_CGI
#warning drmaa_run_job() is not supported by this compilation
  return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                             "xmlrpc_drmaa_run_job is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param jt,start,end,incr
 * @return rc,job-ids,error
 */
static xmlrpc_value *
xmlrpc_drmaa_run_bulk_jobs (xmlrpc_env * const env,
                            xmlrpc_value * const param_array,
                            void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  serialized_job_template_t sjt;
  int start, end, incr;
  xmlrpc_decompose_value (env, param_array,
                          "(" JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE "iii)",
                          &sjt, &start, &end, &incr);
  if (!env->fault_occurred) {
	drmaa_job_template_t *jt = deserialize_job_template (sjt);
    release_serialized_job_template (sjt);
    if (jt != NULL) {
      int rc;
      drmaa_job_ids_t *ji;
      while ((rc = drmaa_run_bulk_jobs(&ji, jt, start, end, incr, error, sizeof(error)))
             == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
      LOG (TEMPLATE | JOB | (rc ? WARNING : 0),
           "start bunk jobs template @%p with jobids @%p%s%s\n",
           jt, ji, rc ? "; non-successful return code, with diagnostic: " : "", error);
      serialized_job_ids_t sji;
      xmlrpc_value *result = xmlrpc_build_value (env,
                                                 "((si)(s" JOB_IDS_SERIALIZED_XMLRPC_VALUE_TYPE ")(ss))",
                                                 "rc", rc, "job-ids", sji = serialize_job_ids (ji), "error", error);
      release_serialized_job_ids (sji);
      return result;
    } else {
      LOG (TEMPLATE | JOB | WARNING, "fault occurred while deserializing job template in run bulk jobs\n");
      return xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -2, "job-ids", "", "error"
                                 "fault occurred while deserializing job template");
    }
  } else {
    LOG (TEMPLATE | JOB | WARNING, "fault occurred while decomposing parameter in run bulk jobs\n");
    return xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "job-ids", "", "error",
                               "fault occurred while decomposing parameter");
  }
#else // DRMAA_XMLRPC_CGI
#warning drmaa_run_bulk_jobs() is not supported by this compilation
  return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                             "xmlrpc_drmaa_run_bulk_jobs is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param action
 * @return rc,status,error
 */
static xmlrpc_value *
xmlrpc_drmaa_job_ps (xmlrpc_env * const env,
                     xmlrpc_value * const param_array,
                     void * const xmlrpc_data) {
  INIT_ERROR_BUFFER(error);

  char *jobid = NULL;
  xmlrpc_decompose_value (env, param_array, "(s)", &jobid);
  xmlrpc_value *result = NULL;
  if (!env->fault_occurred) {
    int rc;
    int status = -1;
    while ((rc = drmaa_job_ps(jobid, &status, error, sizeof(error)))
           == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    LOG (JOB | (rc ? WARNING : 0),
         "job ps jobid %s=0x%X%s%s\n",
         jobid, status, rc ? "; non-successful return code, with diagnostic: " : "", error);
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", rc, "status", status, "error", error);
  } else {
    LOG (JOB | WARNING, "fault occurred while decomposing parameter in job ps\n");
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", -1, "status", -1, "error"
                                 "fault occurred while decomposing parameter");
  }
  free (jobid);
  return result;
}

/**
 * @param jobid,action
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_control (xmlrpc_env * const env,
                      xmlrpc_value * const param_array,
                      void * const xmlrpc_data) {
  INIT_ERROR_BUFFER(error);

  char *jobid = NULL;
  int action = -1;
  xmlrpc_decompose_value (env, param_array, "(si)", &jobid, &action);
  xmlrpc_value *result = NULL;
  if (!env->fault_occurred) {
    int rc;
    while ((rc = drmaa_control (jobid, action, error, sizeof(error)))
           == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    LOG (JOB | (rc ? WARNING : 0),
         "control jobid=%s action=0x%X%s%s\n",
         jobid, action, rc ? "; non-successful return code, with diagnostic: " : "", error);
    result = xmlrpc_build_value (env, "((si)(ss))", "rc", rc, "error", error);
  } else {
    LOG (JOB | WARNING, "fault occurred while decomposing parameters in control\n");
    result = xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error"
                                 "fault occurred while decomposing parameters");
  }
  free (jobid);
  return result;
}

/**
 * @param stat
 * @return rc,exited,error
 */
static xmlrpc_value *
xmlrpc_drmaa_wifexited (xmlrpc_env * const env,
                        xmlrpc_value * const param_array,
                        void * const xmlrpc_data) {
  INIT_ERROR_BUFFER(error);

  int stat = -1;
  int exited = 0;
  xmlrpc_decompose_value (env, param_array, "(i)", &stat);
  xmlrpc_value *result = NULL;
  if (!env->fault_occurred) {
    int rc;
    while ((rc = drmaa_wifexited (&exited, stat, error, sizeof(error)))
           == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    LOG (STAT | (rc ? WARNING : 0),
         "wifexited stat=%d result=%d%s%s\n",
         stat, exited, rc ? "; non-successful return code, with diagnostic: " : "", error);
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", rc, "exited", exited, "error", error);
  } else {
    LOG (STAT | WARNING, "fault occurred while decomposing parameters in if exited\n");
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", -1, "exited", -1, "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
}

/**
 * @param stat
 * @return rc,exit-status,error
 */
static xmlrpc_value *
xmlrpc_drmaa_wexitstatus (xmlrpc_env * const env,
                          xmlrpc_value * const param_array,
                          void * const xmlrpc_data) {
  INIT_ERROR_BUFFER(error);

  int stat = -1;
  int exit_status = 0;
  xmlrpc_decompose_value (env, param_array, "(i)", &stat);
  xmlrpc_value *result = NULL;
  if (!env->fault_occurred) {
    int rc;
    while ((rc = drmaa_wifexited (&exit_status, stat, error, sizeof(error)))
           == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    LOG (STAT | (rc ? WARNING : 0),
         "exit status stat=%d result=%d%s%s\n",
         stat, exit_status, rc ? "; non-successful return code, with diagnostic: " : "", error);
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", rc, "exit-status", exit_status, "error", error);
  } else {
    LOG (STAT | WARNING, "fault occurred while decomposing parameters in exit status\n");
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", -1, "exit-status", -1, "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
}

/**
 * @param stat
 * @return rc,exit-signaled,error
 */
static xmlrpc_value *
xmlrpc_drmaa_wifsignaled (xmlrpc_env * const env,
                          xmlrpc_value * const param_array,
                          void * const xmlrpc_data) {
  INIT_ERROR_BUFFER(error);

  int stat = -1;
  int signaled = 0;
  xmlrpc_decompose_value (env, param_array, "(i)", &stat);
  xmlrpc_value *result = NULL;
  if (!env->fault_occurred) {
    int rc;
    while ((rc = drmaa_wifsignaled (&signaled, stat, error, sizeof(error)))
           == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    LOG (STAT | (rc ? WARNING : 0),
         "signaled stat=%d result=%d%s%s\n",
         stat, signaled, rc ? "; non-successful return code, with diagnostic: " : "", error);
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", rc, "signaled", signaled, "error", error);
  } else {
    LOG (STAT | WARNING, "fault occurred while decomposing parameters in if signaled\n");
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", -1, "signaled", -1, "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
}

/**
 * @param stat
 * @return rc,signal,error
 */
static xmlrpc_value *
xmlrpc_drmaa_wtermsig (xmlrpc_env * const env,
                       xmlrpc_value * const param_array,
                       void * const xmlrpc_data) {
  INIT_ERROR_BUFFER(error);

#define SIGNAL_LENGTH 256
  int stat = -1;
  char signal[SIGNAL_LENGTH];
  xmlrpc_decompose_value (env, param_array, "(i)", &stat);
  xmlrpc_value *result = NULL;
  if (!env->fault_occurred) {
    int rc;
    while ((rc = drmaa_wtermsig (signal, sizeof (signal), stat, error, sizeof(error)))
           == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    LOG (STAT | (rc ? WARNING : 0),
         "signal stat=%d result=%s%s%s\n",
         stat, signal, rc ? "; non-successful return code, with diagnostic: " : "", error);
    result = xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", rc, "signal", signal, "error", error);
  } else {
    LOG (STAT | WARNING, "fault occurred while decomposing parameters in signal\n");
    result = xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "signal", "NOTAVAILABLE", "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
}

/**
 * @param stat
 * @return rc,core-dumped,error
 */
static xmlrpc_value *
xmlrpc_drmaa_wcoredump (xmlrpc_env * const env,
                        xmlrpc_value * const param_array,
                        void * const xmlrpc_data) {
  INIT_ERROR_BUFFER(error);

  int stat = -1;
  int coredump = 0;
  xmlrpc_decompose_value (env, param_array, "(i)", &stat);
  xmlrpc_value *result = NULL;
  if (!env->fault_occurred) {
    int rc;
    while ((rc = drmaa_wcoredump (&coredump, stat, error, sizeof(error)))
           == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    LOG (STAT | (rc ? WARNING : 0),
         "core dumped stat=%d result=%d%s%s\n",
         stat, coredump, rc ? "; non-successful return code, with diagnostic: " : "", error);
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", rc, "core-dump", coredump, "error", error);
  } else {
    LOG (STAT | WARNING, "fault occurred while decomposing parameters in if core dumped\n");
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", -1, "core-dump", -1, "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
}

/**
 * @param stat
 * @return rc,aborted,error
 */
static xmlrpc_value *
xmlrpc_drmaa_wifaborted (xmlrpc_env * const env,
                         xmlrpc_value * const param_array,
                         void * const xmlrpc_data) {
  INIT_ERROR_BUFFER(error);

  int stat = -1;
  int aborted = 0;
  xmlrpc_decompose_value (env, param_array, "(i)", &stat);
  xmlrpc_value *result = NULL;
  if (!env->fault_occurred) {
    int rc;
    while ((rc = drmaa_wifaborted (&aborted, stat, error, sizeof(error)))
           == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    LOG (STAT | (rc ? WARNING : 0),
         "core dumped stat=%d result=%d%s%s\n",
         stat, aborted, rc ? "; non-successful return code, with diagnostic: " : "", error);
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", rc, "aborted", aborted, "error", error);
  } else {
    LOG (STAT | WARNING, "fault occurred while decomposing parameters in if core dumped\n");
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", -1, "aborted", -1, "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
}

/**
 * @return rc,attribute-names,error
 */
static xmlrpc_value *
xmlrpc_drmaa_get_attribute_names (xmlrpc_env * const env,
                                  xmlrpc_value * const param_array,
                                  void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  drmaa_attr_names_t *an = NULL;
  INIT_ERROR_BUFFER(error);
  serialized_attr_names_t san;

  int rc;
  while ((rc = drmaa_get_attribute_names (&an, error, sizeof (error)))
         == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

  LOG (LIST | (rc ? WARNING : 0),
       "get attribute names @%p%s%s\n",
       an, rc ? "; non-successful return code, with diagnostic: " : "", error);
  xmlrpc_value *result = xmlrpc_build_value (env,
                                             "((si)(s" ATTR_NAMES_SERIALIZED_XMLRPC_VALUE_TYPE ")(ss))",
                                             "rc", rc,
                                             "attribute-names", san = serialize_attr_names (an),
                                             "error", error);
  release_serialized_attr_names (san);
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_get_attribute_names() is not supported by this compilation
  LOG (LIST | INFO, "get attribute names is not supported with this compilation");
  return xmlrpc_build_value (env,
                             "((si)(s" ATTR_NAMES_SERIALIZED_XMLRPC_VALUE_TYPE ")(ss))",
                             "rc", -1, "attribute-names", "",  "error",
                             "xmlrpc_drmaa_get_attribute_names is not supported on this compilation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @return rc,vector-attribute-names,error
 */
static xmlrpc_value *
xmlrpc_drmaa_get_vector_attribute_names (xmlrpc_env * const env,
                                         xmlrpc_value * const param_array,
                                         void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  drmaa_attr_names_t *an = NULL;
  INIT_ERROR_BUFFER(error);
  serialized_attr_names_t san;

  int rc;
  while ((rc = drmaa_get_vector_attribute_names (&an, error, sizeof (error)))
         == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

  LOG (LIST | (rc ? WARNING : 0),
       "get vector attribute names @%p%s%s\n",
       an, rc ? "; non-successful return code, with diagnostic: " : "", error);
  xmlrpc_value *result = xmlrpc_build_value (env,
                                             "((si)(s" ATTR_NAMES_SERIALIZED_XMLRPC_VALUE_TYPE ")(ss))",
                                             "rc", rc,
                                             "vector-attribute-names", san = serialize_attr_names (an),
                                             "error", error);
  release_serialized_attr_names (san);
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_get_vector_attribute_names() is not supported by this compilation
  LOG (LIST | INFO, "get vector attribute names is not supported with this compilation");
  return xmlrpc_build_value (env,
                             "((si)(s" ATTR_NAMES_SERIALIZED_XMLRPC_VALUE_TYPE ")(ss))",
                             "rc", -1, "attribute-names", "",  "error",
                             "xmlrpc_drmaa_get_attribute_names is not supported on this compilation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param attribute-names
 * @return rc,attribute-name,error
 */
static xmlrpc_value *
xmlrpc_drmaa_get_next_attr_name (xmlrpc_env * const env,
                                 xmlrpc_value * const param_array,
                                 void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  serialized_attr_names_t san;
  char name [DRMAA_ATTR_BUFFER];
  xmlrpc_value *result = NULL;
  xmlrpc_decompose_value (env, param_array,
                          "(" ATTR_NAMES_SERIALIZED_XMLRPC_VALUE_TYPE ")",
                          &san);
  if (!env->fault_occurred) {
	drmaa_attr_names_t *an = deserialize_attr_names (san);
    release_serialized_attr_names (san);
    if (an != NULL) {
      int rc;
      while ((rc = drmaa_get_next_attr_name (an, name, sizeof (name)))
             == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
      LOG (LIST | (rc ? WARNING : 0),
           "get next attribute name %s for attribute names @%p%s%s\n",
           name,
           an, rc ? "; non-successful return code, with diagnostic: " : "", error);
      result = xmlrpc_build_value (env,
                                   "((si)(ss)(ss))",
                                   "rc", rc, "attribute-name", name, "error", error);
    } else {
      LOG (LIST | WARNING, "fault occurred while deserializing attribute names in get next attribute name\n");
      result = xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -2, "attribute-name", "not-available", "error",
                                   "fault occurred while deserializing attribute names");
    }
  } else {
    LOG (LIST | WARNING, "fault occurred while decomposing parameters in get next attribute name\n");
    result = xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "attribute-name", "not-available", "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_get_next_attr_name() is not supported by this compilation
  return xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "attribute-name", "not-available", "error",
                             "drmaa_get_next_attr_name is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param attribute-names
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_release_attr_names (xmlrpc_env * const env,
                                 xmlrpc_value * const param_array,
                                 void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  serialized_attr_names_t san;
  xmlrpc_value *result = NULL;
  xmlrpc_decompose_value (env, param_array,
                          "(" ATTR_NAMES_SERIALIZED_XMLRPC_VALUE_TYPE ")",
                          &san);
  if (!env->fault_occurred) {
	drmaa_attr_names_t *an = deserialize_attr_names (san);
    release_serialized_attr_names (san);
    if (an != NULL) {
      drmaa_release_attr_names (an);
      LOG (LIST, "release attribute names %p\n", an);
      result = xmlrpc_build_value (env,
                                   "((si)(ss))",
                                   "rc", 0, "error", error);
    } else {
      LOG (LIST | WARNING, "fault occurred while deserializing attribute names in release attribute names\n");
      result = xmlrpc_build_value (env, "((si)(ss))", "rc", -2, "error",
                                   "fault occurred while deserializing attribute names");
    }
  } else {
    LOG (LIST | WARNING, "fault occurred while decomposing parameters in release attribute names\n");
    result = xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_release_attr_names() is not supported by this compilation
  return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                             "drmaa_release_attr_names is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @return rc,attribute-values,error
 */
static xmlrpc_value *
xmlrpc_drmaa_get_vector_attribute (xmlrpc_env * const env,
                                   xmlrpc_value * const param_array,
                                   void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  drmaa_attr_values_t *av = NULL;
  INIT_ERROR_BUFFER(error);
  serialized_attr_values_t sav = NULL;

  serialized_job_template_t sjt;
  char *name = NULL;
  xmlrpc_value *result = NULL;
  xmlrpc_decompose_value (env, param_array,
                          "(" JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE "s)",
                          &sjt, &name);
  if (!env->fault_occurred) {
    drmaa_job_template_t *jt = deserialize_job_template (sjt);
    release_serialized_job_template (sjt);

    int rc;
    while ((rc = drmaa_get_vector_attribute (jt, name, &av, error, sizeof (error)))
           == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

    LOG (TEMPLATE | (rc ? WARNING : 0),
         "get attribute values @%p%s%s\n",
         av, rc ? "; non-successful return code, with diagnostic: " : "", error);
    result = xmlrpc_build_value (env,
                                 "((si)(s" ATTR_VALUES_SERIALIZED_XMLRPC_VALUE_TYPE ")(ss))",
                                 "rc", rc,
                                 "attribute-values", sav = serialize_attr_values (av),
                                 "error", error);
    release_serialized_attr_values (sav);
  } else {
    LOG (TEMPLATE | WARNING, "fault occurred while decomposing parameters in get vector attribute\n");
    result = xmlrpc_build_value (env, "((si)(s" ATTR_VALUES_SERIALIZED_XMLRPC_VALUE_TYPE ")(ss))",
                                 "rc", -1, "attribute-values", "", "error"
                                 "fault occurred while decomposing parameters");
  }
  free (name);
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_get_attribute_values() is not supported by this compilation
  LOG (TEMPLATE | INFO, "get attribute values is not supported with this compilation");
  return xmlrpc_build_value (env,
                             "((si)(s" ATTR_VALUES_SERIALIZED_XMLRPC_VALUE_TYPE ")(ss))",
                             "rc", -1, "attribute-values", "",  "error",
                             "xmlrpc_drmaa_get_attribute_values is not supported on this compilation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param attribute-values
 * @return rc,attribute-value,error
 */
static xmlrpc_value *
xmlrpc_drmaa_get_next_attr_value (xmlrpc_env * const env,
                                  xmlrpc_value * const param_array,
                                  void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  serialized_attr_values_t sav;
  char value [DRMAA_ATTR_BUFFER];
  xmlrpc_value *result = NULL;
  xmlrpc_decompose_value (env, param_array,
                          "(" ATTR_VALUES_SERIALIZED_XMLRPC_VALUE_TYPE ")",
                          &sav);
  if (!env->fault_occurred) {
	drmaa_attr_values_t *av = deserialize_attr_values (sav);
    release_serialized_attr_values (sav);
    if (av != NULL) {
      int rc;
      while ((rc = drmaa_get_next_attr_value (av, value, sizeof (value)))
             == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
      LOG (TEMPLATE | (rc ? WARNING : 0),
           "get next attribute value %s for attribute values @%p%s%s\n",
           value,
           av, rc ? "; non-successful return code, with diagnostic: " : "", error);
      result = xmlrpc_build_value (env,
                                   "((si)(ss)(ss))",
                                   "rc", rc, "attribute-value", value, "error", error);
    } else {
      LOG (TEMPLATE | WARNING, "fault occurred while deserializing attribute names in get next attribute value\n");
      result = xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -2, "attribute-value", "not-available", "error",
                                   "fault occurred while deserializing attribute names");
    }
  } else {
    LOG (TEMPLATE | WARNING, "fault occurred while decomposing parameters in get next attribute name\n");
    result = xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "attribute-value", "not-available", "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_get_next_attr_name() is not supported by this compilation
  return xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "attribute-value", "not-available", "error",
                             "drmaa_get_next_attr_name is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param attribute-values
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_release_attr_values (xmlrpc_env * const env,
                                  xmlrpc_value * const param_array,
                                  void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  serialized_attr_values_t sav;
  xmlrpc_value *result = NULL;
  xmlrpc_decompose_value (env, param_array,
                          "(" ATTR_VALUES_SERIALIZED_XMLRPC_VALUE_TYPE ")",
                          &sav);
  if (!env->fault_occurred) {
	drmaa_attr_names_t *av = deserialize_attr_names (sav);
    release_serialized_attr_names (sav);
    if (av != NULL) {
      drmaa_release_attr_values (sav);
      LOG (TEMPLATE, "release attribute names %p\n", av);
      result = xmlrpc_build_value (env,
                                   "((si)(ss))",
                                   "rc", 0, "error", error);
    } else {
      LOG (TEMPLATE | WARNING, "fault occurred while deserializing attribute names in release attribute values\n");
      result = xmlrpc_build_value (env, "((si)(ss))", "rc", -2, "error",
                                   "fault occurred while deserializing attribute values");
    }
  } else {
    LOG (TEMPLATE | WARNING, "fault occurred while decomposing parameters in release attribute values\n");
    result = xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_release_attr_values() is not supported by this compilation
  return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                             "drmaa_release_attr_values is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param job-ids
 * @return rc,job-id,error
 */
static xmlrpc_value *
xmlrpc_drmaa_get_next_job_id (xmlrpc_env * const env,
                              xmlrpc_value * const param_array,
                              void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  serialized_job_ids_t sji;
  INIT_JOBID(jobid);
  xmlrpc_value *result = NULL;
  xmlrpc_decompose_value (env, param_array,
                          "(" JOB_IDS_SERIALIZED_XMLRPC_VALUE_TYPE ")",
                          &sji);
  if (!env->fault_occurred) {
	drmaa_job_ids_t *ji = deserialize_job_ids (sji);
    release_serialized_attr_names (sji);
    if (ji != NULL) {
      int rc;
      while ((rc = drmaa_get_next_job_id (ji, jobid, sizeof (jobid)))
             == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
      LOG (LIST | (rc ? WARNING : 0),
           "get next job id %s for job ids @%p%s%s\n",
           jobid,
           ji, rc ? "; non-successful return code, with diagnostic: " : "", error);
      result = xmlrpc_build_value (env,
                                   "((si)(ss)(ss))",
                                   "rc", rc, "job-id", jobid, "error", error);
    } else {
      LOG (LIST | WARNING, "fault occurred while deserializing job ids in get next job id\n");
      result = xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -2, "job-id", "not-available", "error",
                                   "fault occurred while deserializing job ids");
    }
  } else {
    LOG (LIST | WARNING, "fault occurred while decomposing parameters in get next job id\n");
    result = xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "job-id", "not-available", "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_get_next_job_id() is not supported by this compilation
  return xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "job-id", "not-available", "error",
                             "drmaa_get_next_job_id is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

/**
 * @param job-ids
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_release_job_ids (xmlrpc_env * const env,
                              xmlrpc_value * const param_array,
                              void * const xmlrpc_data) {
#ifndef DRMAA_XMLRPC_CGI
  INIT_ERROR_BUFFER(error);

  serialized_job_ids_t sji;
  xmlrpc_value *result = NULL;
  xmlrpc_decompose_value (env, param_array,
                          "(" JOB_IDS_SERIALIZED_XMLRPC_VALUE_TYPE ")",
                          &sji);
  if (!env->fault_occurred) {
	drmaa_job_ids_t *ji = deserialize_job_ids (sji);
    release_serialized_job_ids (sji);
    if (ji != NULL) {
      drmaa_release_job_ids (ji);
      LOG (LIST, "release job ids %p\n", ji);
      result = xmlrpc_build_value (env, "((si)(ss))", "rc", 0, "error", error);
    } else {
      LOG (LIST | WARNING, "fault occurred while deserializing job ids in release job ids\n");
      result = xmlrpc_build_value (env, "((si)(ss))", "rc", -2, "error",
                                   "fault occurred while deserializing job ids");
    }
  } else {
    LOG (LIST | WARNING, "fault occurred while decomposing parameters in release job ids\n");
    result = xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                                 "fault occurred while decomposing parameters");
  }
  return result;
#else // DRMAA_XMLRPC_CGI
#warning drmaa_release_job_ids() is not supported by this compilation
  return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                             "drmaa_release_job_ids is not supported on this implementation");
#endif // DRMAA_XMLRPC_CGI
}

// end of DRMAA stuff

#include "config.h"

void hangup (int param) {
  if (configuration.log_file_name) {
    configuration.log_file = freopen (configuration.log_file_name, "a", configuration.log_file);
    FILE *conf_file = fopen (configuration.conf_file, "r");
    if (conf_file) {
      INIT_ERROR_BUFFER(error);
      load (sizeof (error), error, conf_file, _config_consume_log_mask, NULL);
      fclose (conf_file);
      LOG (INFO | SYSTEM, "on SIGHUP reopened log file and refreshed log_mask\n");
    } else LOG (WARNING | SYSTEM, "on SIGHUP unable to open configuration file\n");
  } else LOG (WARNING | SYSTEM, "on SIGHUP missing configuration file\n");
}

int main (int argc, char **argv) {
  INIT_ERROR_BUFFER(error);
  _init_config ();

  FILE *conf_file;
#ifndef DRMAA_XMLRPC_CGI
  if (argc > 1)
    if (argc >= 3)
      if (strncmp (argv[1], "--conf-file", 11) == 0) conf_file = fopen (configuration.conf_file = argv[2], "r");
      else return -1;
    else return -2;
  else conf_file = fopen (configuration.conf_file = "drmaa-xmlrpc.conf", "r");
#else // DRMAA_XMLRPC_CGI
  conf_file = fopen (getenv (CONF_FILE_PATH_ENV_NAME_FOR_CGI), "r");
#endif // DRMAA_XMLRPC_CGI
  if (conf_file) {
    load (sizeof (error), error, conf_file, _config_consume, NULL);
    fclose (conf_file);
  }
  _log_config ();

#ifndef DRMAA_XMLRPC_CGI
  if (configuration.daemon) {
    if (daemon (1, 0)) {
      LOG (SEVERE | SYSTEM, "failed to daemonize\n");
      return -3;
    }
    if (configuration.pid_file) {
      if (fprintf (configuration.pid_file, "%d", getpid()) < 0) {
        LOG (WARNING | SYSTEM, "failed to write to pid file\n");
      }
      fclose (configuration.pid_file);
    }
  }

  if (signal (SIGPIPE, SIG_IGN) == SIG_ERR) { // so as to not get killed by the OS on SIGPIPE
    LOG (SEVERE | SYSTEM, "failed to set SIGPIPE handler with errno=%d(0x%X)\n", errno, errno);
    return -2;
  }

  if (signal (SIGHUP, hangup) == SIG_ERR) { LOG (WARNING | SYSTEM, "failed to register SIGHUP\n"); }

  xmlrpc_server_abyss_parms serverparm;
  xmlrpc_registry *registryP;
  xmlrpc_env env;

  xmlrpc_env_init (&env);
  registryP = xmlrpc_registry_new (&env);

  serverparm.port_number = configuration.port;
  serverparm.registryP = registryP;
  serverparm.config_file_name = NULL;
  serverparm.log_file_name = configuration.abyss_log_file_name;
#else // DRMAA_XMLRPC_CGI
  xmlrpc_cgi_init(XMLRPC_CGI_NO_FLAGS);
#endif // DRMAA_XMLRPC_CGI

  int rc = drmaa_init(configuration.drmaa_init_contact, error, sizeof(error));
  if (rc) {
    LOG (SEVERE | DRMAA,
         "failed to initialize DRMAA environment, drmaa_init returned 0x%X with diagnostic: %s\n", rc, error);
    exit (-2);
  } else LOG (DRMAA, "initialized DRMAA environment\n");

#ifndef DRMAA_XMLRPC_CGI
#define XMLRPC_ADD_METHOD(METHOD, FUNCTION) xmlrpc_registry_add_method (&env, registryP, NULL, METHOD, FUNCTION, NULL)
#else // DRMAA_XMLRPC_CGI
#define XMLRPC_ADD_METHOD(METHOD, FUNCTION) xmlrpc_cgi_add_method (METHOD, FUNCTION, NULL)
#endif // DRMAA_XMLRPC_CGI

  XMLRPC_ADD_METHOD ("drmaa_allocate_job_template", &xmlrpc_drmaa_allocate_job_template);
  XMLRPC_ADD_METHOD ("drmaa_delete_job_template", &xmlrpc_drmaa_delete_job_template);
  XMLRPC_ADD_METHOD ("drmaa_set_attribute", &xmlrpc_drmaa_set_attribute);
  XMLRPC_ADD_METHOD ("drmaa_set_vector_attribute", &xmlrpc_drmaa_set_vector_attribute);
  XMLRPC_ADD_METHOD ("drmaa_run_job", &xmlrpc_drmaa_run_job);
  XMLRPC_ADD_METHOD ("drmaa_job_ps", &xmlrpc_drmaa_job_ps);
  XMLRPC_ADD_METHOD ("drmaa_control", &xmlrpc_drmaa_control);
  XMLRPC_ADD_METHOD ("drmaa_wifexited", &xmlrpc_drmaa_wifexited);
  XMLRPC_ADD_METHOD ("drmaa_wexitstatus", &xmlrpc_drmaa_wexitstatus);
  XMLRPC_ADD_METHOD ("drmaa_wifsignaled", &xmlrpc_drmaa_wifsignaled);
  XMLRPC_ADD_METHOD ("drmaa_wtermsig", &xmlrpc_drmaa_wtermsig);
  XMLRPC_ADD_METHOD ("drmaa_wcoredump", &xmlrpc_drmaa_wcoredump);
  XMLRPC_ADD_METHOD ("drmaa_wifaborted", &xmlrpc_drmaa_wifaborted);
  XMLRPC_ADD_METHOD ("drmaa_get_attribute_names", &xmlrpc_drmaa_get_attribute_names);
  XMLRPC_ADD_METHOD ("drmaa_get_vector_attribute_names", &xmlrpc_drmaa_get_vector_attribute_names);
  XMLRPC_ADD_METHOD ("drmaa_get_next_attr_name", &xmlrpc_drmaa_get_next_attr_name);
  XMLRPC_ADD_METHOD ("drmaa_release_attr_names", &xmlrpc_drmaa_release_attr_names);
  XMLRPC_ADD_METHOD ("drmaa_get_vector_attribute", &xmlrpc_drmaa_get_vector_attribute);
  XMLRPC_ADD_METHOD ("drmaa_get_next_attr_value", &xmlrpc_drmaa_get_next_attr_value);
  XMLRPC_ADD_METHOD ("drmaa_release_attr_values", &xmlrpc_drmaa_release_attr_values);
  XMLRPC_ADD_METHOD ("drmaa_get_next_job_id", &xmlrpc_drmaa_get_next_job_id);
  XMLRPC_ADD_METHOD ("drmaa_release_job_ids", &xmlrpc_drmaa_release_job_ids);
  XMLRPC_ADD_METHOD ("drmaa_run_bulk_jobs", &xmlrpc_drmaa_run_bulk_jobs);

#ifndef DRMAA_XMLRPC_CGI
  xmlrpc_server_abyss(&env, &serverparm, XMLRPC_APSIZE(log_file_name));

  if (_DEFAULT_ABYSS_LOG_FILE_NAME != configuration.abyss_log_file_name)
    free (configuration.abyss_log_file_name);
#else // DRMAA_XMLRPC_CGI
  xmlrpc_cgi_process_call();
  xmlrpc_cgi_cleanup();
#endif // DRMAA_XMLRPC_CGI
  rc = drmaa_exit(error, sizeof(error));
  LOG (DRMAA | (rc ? WARNING : 0), "drmaa exit status 0x%X%s%s\n", rc, rc ? ": " : "", rc ? error : "");
  if (configuration.log_file) fclose (configuration.log_file);
  if (configuration.drmaa_init_contact) free (configuration.drmaa_init_contact);
  if (configuration.log_file_name) free (configuration.log_file_name);

  return 0;
}
