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
#warning PRESENT OPERATION PRINCIPLE OF PASSING THE MEMORY ADDRESS TO THE
#warning CLIENT AS THE SESSION TOKEN WILL NOT WORK FOR CGI BECAUSE THE
#warning PROCESS GOES AWAY BETWEEN CALLS. A DIFFERENT PRINCIPLE IS NEEDED
#warning FOR CGI OPERATION.
#include <xmlrpc.h>
#include <xmlrpc_cgi.h>
#endif // DRMAA_XMLRPC_CGI

#include <drmaa.h>
#include <sys/signal.h>
#include <unistd.h>
#include <limits.h>

#define CONF_FILE_PATH_ENV_NAME_FOR_CGI "DRMAA_XMLRPC_CGI_CONF_PATH"

#define ERROR_DIAGNOSIS_MAX DRMAA_ERROR_STRING_BUFFER
#define INIT_ERROR_BUFFER(X) char X[ERROR_DIAGNOSIS_MAX]; memset (X, 0, ERROR_DIAGNOSIS_MAX)
#define JOB_ID_MAX DRMAA_JOBNAME_BUFFER
#define INIT_JOBID(X) char X[JOB_ID_MAX]; memset (X, 0, JOB_ID_MAX)

#define JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE "s"
typedef void *serialized_job_template_t;

static serialized_job_template_t serialize_job_template (drmaa_job_template_t *jt);
static void release_serialized_job_template_type (serialized_job_template_t sjt);
static drmaa_job_template_t *deserialize_job_template (serialized_job_template_t sjt);

/**
 * @return rc,jt,error
 */
static xmlrpc_value *
xmlrpc_drmaa_allocate_job_template (xmlrpc_env * const env,
                                    xmlrpc_value * const param_array,
                                    void * const xmlrpc_data) {
  drmaa_job_template_t *jt = NULL;
  INIT_ERROR_BUFFER(error);
  serialized_job_template_t sjt;

  int rc;
  while ((rc = drmaa_allocate_job_template (&jt, error, sizeof (error)))
         == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

  xmlrpc_value *result = xmlrpc_build_value (env,
                                             "((si)(s" JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE ")(ss))",
                                             "rc", rc,
                                             "jt", sjt = serialize_job_template (jt),
                                             "error", error);
  release_serialized_job_template_type (sjt);
  return result;
}

/**
 * @param jt
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_delete_job_template (xmlrpc_env * const env,
                                  xmlrpc_value * const param_array,
                                  void * const xmlrpc_data) {
  INIT_ERROR_BUFFER(error);

  serialized_job_template_t sjt;
  xmlrpc_decompose_value (env, param_array,
                          "(" JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE ")",
                          &sjt);
  if (!env->fault_occurred) {
	drmaa_job_template_t *jt = deserialize_job_template (sjt);
    release_serialized_job_template_type (sjt);
    if (jt != NULL) {
      int rc;
      while ((rc = drmaa_delete_job_template (jt, error, sizeof (error)))
             == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
      return xmlrpc_build_value (env,
                                 "((si)(ss))",
                                 "rc", rc, "error", error);
    } else return xmlrpc_build_value (env, "((si)(ss))", "rc", -2, "error",
                                      "fault occurred while deserializing job template");
  } else return xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                                    "fault occurred while decomposing parameter");
}

/**
 * @param jt,name,value
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_set_attribute (xmlrpc_env * const env,
                            xmlrpc_value * const param_array,
                            void * const xmlrpc_data) {
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
    release_serialized_job_template_type (sjt);
    if (jt != NULL) {
      int rc;
      while ((rc = drmaa_set_attribute(jt, name, value, error, sizeof (error)))
             == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
      result = xmlrpc_build_value (env,
                                   "((si)(ss))",
                                   "rc", rc, "error", error);
    } else result = xmlrpc_build_value (env, "((si)(ss))", "rc", -2, "error",
                                        "fault occurred while deserializing job template");
  } else result = xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                                      "fault occurred while decomposing parameters");
  free (name);
  free (value);
  return result;
}

/**
 * @param jt,name,[values...]
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_set_vector_attribute (xmlrpc_env * const env,
                            xmlrpc_value * const param_array,
                            void * const xmlrpc_data) {
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
  if ((values = (char**) malloc (sizeof (char*) * (values_count = xmlrpc_array_size (env, x_values))))) {
    int index = 0;
    for (; index < values_count; index++) {
      xmlrpc_value *x_value;
      xmlrpc_array_read_item (env, x_values, index, &x_value);
      xmlrpc_decompose_value (env, x_value, "s", &(values [index]));
      xmlrpc_DECREF (x_value);
    }

    if (!env->fault_occurred) {
	  drmaa_job_template_t *jt = deserialize_job_template (sjt);
      release_serialized_job_template_type (sjt);
      if (jt != NULL) {
        int rc;
        while ((rc = drmaa_set_vector_attribute(jt, name, (const char**) values, error, sizeof (error)))
               == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
        result = xmlrpc_build_value (env,
                                     "((si)(ss))",
                                     "rc", rc, "error", error);
      } else result = xmlrpc_build_value (env, "((si)(ss))", "rc", -2, "error",
                                          "fault occurred while deserializing job template");
    } else result = xmlrpc_build_value (env, "((si)(ss))", "rc", -1, "error",
                                        "fault occurred while decomposing parameters");
  } else result = xmlrpc_build_value (env, "(si)(ss)", "rc", -1, "error",
                                      "failed to allocate memory for values array");
  if (values) {
	  int i;
	  for (i = 0; i < values_count; i++) free (values [i]);
  }
  free (name);
  free (values);
  return result;
}

/**
 * @param jt
 * @return rc,jobid,error
 */
static xmlrpc_value *
xmlrpc_drmaa_run_job (xmlrpc_env * const env,
                      xmlrpc_value * const param_array,
                      void * const xmlrpc_data) {
  INIT_ERROR_BUFFER(error);

  serialized_job_template_t sjt;
  xmlrpc_decompose_value (env, param_array,
                          "(" JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE ")",
                          &sjt);
  if (!env->fault_occurred) {
	drmaa_job_template_t *jt = deserialize_job_template (sjt);
    release_serialized_job_template_type (sjt);
    if (jt != NULL) {
      int rc;
      INIT_JOBID(jobid);
      while ((rc = drmaa_run_job(jobid, sizeof(jobid), jt, error, sizeof(error)))
             == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
      return xmlrpc_build_value (env,
                                 "((si)(ss)(ss))",
                                 "rc", rc, "jobid", jobid, "error", error);
    } else return xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -2, "jobid", "not available", "error"
                                      "fault occurred while deserializing job template");
  } else return xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "jobid", "not available", "error",
                                    "fault occurred while decomposing parameter");
}

/**
 * @param jobid
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
    result = xmlrpc_build_value (env, "((si)(si)(ss))", "rc", rc, "status", status, "error", error);
  } else result = xmlrpc_build_value (env, "((si)(ss)(ss))", "rc", -1, "status", -1, "error"
                                      "fault occurred while decomposing parameter");
  free (jobid);
  return result;
}

#include "config.h"

struct {
  int daemon;
  int port;

  FILE *pid_file;
  FILE *log_file;
  unsigned long long log_mask;
  char *abyss_log_file_name;
} configuration;

static char *_DEFAULT_ABYSS_LOG_FILE_NAME = "/dev/null";

static void _init_config (void) {
  configuration.daemon = 1;
  configuration.port = 41334;

  configuration.pid_file = NULL;
  configuration.log_file = NULL;
  configuration.log_mask = 0;
  configuration.abyss_log_file_name = _DEFAULT_ABYSS_LOG_FILE_NAME;
}

static void _config_consume (char *n, char *v, void *garbage) {
  if (strncmp (n, "daemon", 50) == 0) configuration.daemon = atoi (v);
  else if (strncmp (n, "port", 50) == 0) configuration.port = atoi (v);
  else if (strncmp (n, "pid_file", 50) == 0) configuration.pid_file = fopen (v, "w");
  else if (strncmp (n, "log_file", 50) == 0) configuration.log_file = fopen (v, "w");
  else if (strncmp (n, "log_mask", 50) == 0) configuration.log_mask = atoll (v);
  else if (strncmp (n, "abyss_log_file_name", 50) == 0) {
    char *tmp = (char*) malloc (strlen (v) + 1);
    if (tmp) strcpy (configuration.abyss_log_file_name = tmp, v);
  }
}

int main (int argc, char **argv) {
  INIT_ERROR_BUFFER(error);
  _init_config ();

  FILE *conf_file;
#ifndef DRMAA_XMLRPC_CGI
  if (argc > 1)
    if (argc >= 3)
      if (strncmp (argv[1], "--conf-file", 11) == 0) conf_file = fopen (argv[2], "r");
      else return -1;
    else return -2;
  else conf_file = fopen ("drmaa-xmlrpc.conf", "r");
#else // DRMAA_XMLRPC_CGI
  conf_file = fopen (getenv (CONF_FILE_PATH_ENV_NAME_FOR_CGI), "r");
#endif // DRMAA_XMLRPC_CGI
  if (conf_file) load (sizeof (error), error, conf_file, _config_consume, NULL);

#ifndef DRMAA_XMLRPC_CGI
  if (configuration.daemon) {
    if (daemon (1, 0)) return -3;
    if (configuration.pid_file) {
      fprintf (configuration.pid_file, "%d", getpid());
      fclose (configuration.pid_file);
    }
  }

  signal(SIGPIPE, SIG_IGN); // so as to not get killed by the OS on SIGPIPE

  xmlrpc_server_abyss_parms serverparm;
  xmlrpc_registry *registryP;
  xmlrpc_env env;

  xmlrpc_env_init(&env);
  registryP = xmlrpc_registry_new(&env);

  serverparm.port_number = configuration.port;
  serverparm.registryP = registryP;
  serverparm.config_file_name = NULL;
  serverparm.log_file_name = configuration.abyss_log_file_name;
#else // DRMAA_XMLRPC_CGI
  xmlrpc_cgi_init(XMLRPC_CGI_NO_FLAGS);
#endif // DRMAA_XMLRPC_CGI

  int rc = drmaa_init(NULL, error, sizeof(error));
  if (rc) { /* TODO: log this error */
    exit (-2);
  }

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

#ifndef DRMAA_XMLRPC_CGI
  xmlrpc_server_abyss(&env, &serverparm, XMLRPC_APSIZE(log_file_name));

  if (configuration.log_file) fclose (configuration.log_file);
  if (_DEFAULT_ABYSS_LOG_FILE_NAME != configuration.abyss_log_file_name)
    free (configuration.abyss_log_file_name);
#else // DRMAA_XMLRPC_CGI
  xmlrpc_cgi_process_call();
  xmlrpc_cgi_cleanup();
#endif // DRMAA_XMLRPC_CGI

  return 0;
}

// helpers

#define MAX_ADDRESS_ASCII_LENGTH 50

static serialized_job_template_t serialize_job_template (drmaa_job_template_t *jt) {
  char *result = (char*) malloc (MAX_ADDRESS_ASCII_LENGTH);
  sprintf (result, "%p", jt);
  return (serialized_job_template_t) result;
}

static void release_serialized_job_template_type (serialized_job_template_t sjt) {
  free (sjt);
}

static drmaa_job_template_t *deserialize_job_template (serialized_job_template_t sjt) {
  char *address = (char*) sjt;
  drmaa_job_template_t *result = NULL;
  sscanf (address, "%p", &result);
  return result;
}
