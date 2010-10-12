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
#include <time.h>
#include <errno.h>
#include <signal.h>

#define CONF_FILE_PATH_ENV_NAME_FOR_CGI "DRMAA_XMLRPC_CGI_CONF_PATH"

struct {
  int daemon;
  int port;

  FILE *pid_file;
  FILE *log_file;
  unsigned long long log_mask;
  char *abyss_log_file_name;

  char *drmaa_init_contact;
} configuration;

#define SEVERE (1L << 63)
#define WARNING (1L << 62)
#define INFO (1L << 61)

#define CONFIG (1L)
#define TEMPLATE (1L << 1)
#define JOB (1L << 2)
#define DRMAA (1L << 3)
#define SYSTEM (1L << 4)

static char *_log_mask_to_level (unsigned long long mask) {
  if (mask & SEVERE) return "SEVERE";
  else if (mask & WARNING) return "WARNING";
  else if (mask & INFO) return "INFO";
  else return "TRACE";
}

#define LOG(MASK, ... ) \
  if ((configuration.log_mask & (MASK)) && configuration.log_file) { \
    time_t rt; struct tm * ti; time ( &rt ); ti = localtime ( &rt ); \
    fprintf (configuration.log_file, "[%02d:%02d:%02d] %s ", \
             ti->tm_hour, ti->tm_min, ti->tm_sec, _log_mask_to_level (MASK)); \
    fprintf (configuration.log_file, __VA_ARGS__ ); } else

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
                                             "jt", sjt = serialize_job_template (jt),
                                             "error", error);
  release_serialized_job_template_type (sjt);
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
    release_serialized_job_template_type (sjt);
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
    release_serialized_job_template_type (sjt);
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
      release_serialized_job_template_type (sjt);
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
    release_serialized_job_template_type (sjt);
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

#include "config.h"

static char *_DEFAULT_ABYSS_LOG_FILE_NAME = "/dev/null";

static void _init_config (void) {
  configuration.daemon = 1;
  configuration.port = 41334;

  configuration.pid_file = NULL;
  configuration.log_file = NULL;
  configuration.log_mask = 0;
  configuration.abyss_log_file_name = _DEFAULT_ABYSS_LOG_FILE_NAME;
  configuration.drmaa_init_contact = NULL;
}

static void _log_config () {
  LOG (INFO | CONFIG, "drmaa-xmlrpc configuration: { daemon=%d, port=%d, log_mask=0x%llX }\n",
       configuration.daemon, configuration.port, configuration.log_mask);
}

static void _config_consume (char *n, char *v, void *garbage) {
  if (strncmp (n, "daemon", 50) == 0) configuration.daemon = atoi (v);
  else if (strncmp (n, "port", 50) == 0) configuration.port = atoi (v);
#ifndef DRMAA_XMLRPC_CGI
  else if (strncmp (n, "pid_file", 50) == 0) configuration.pid_file = fopen (v, "w");
#endif
  else if (strncmp (n, "log_file", 50) == 0)
#ifndef DRMAA_XMLRPC_CGI
  {
    if (strncmp (v, "stdout", 50) == 0) configuration.log_file = stdout;
    else if (strncmp (v, "stderr", 50) == 0) configuration.log_file = stderr;
    else configuration.log_file = fopen (v, "w");
  }
#else
    configuration.log_file = fopen (v, "a");
#endif
  else if (strncmp (n, "log_mask", 50) == 0) configuration.log_mask = atoll (v);
  else if (strncmp (n, "abyss_log_file_name", 50) == 0) {
    char *tmp = (char*) malloc (strlen (v) + 1);
    if (tmp) strcpy (configuration.abyss_log_file_name = tmp, v);
  } else if (strncmp (n, "drmaa_init_contact", 50) == 0) {
    char *tmp = (char*) malloc (strlen (v) + 1);
    if (tmp) strcpy (configuration.drmaa_init_contact = tmp, v);
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

#ifndef DRMAA_XMLRPC_CGI
  xmlrpc_server_abyss(&env, &serverparm, XMLRPC_APSIZE(log_file_name));

  if (_DEFAULT_ABYSS_LOG_FILE_NAME != configuration.abyss_log_file_name)
    free (configuration.abyss_log_file_name);
  if (configuration.log_file == stdout || configuration.log_file == stderr) goto skip_log_file_close;
#else // DRMAA_XMLRPC_CGI
  xmlrpc_cgi_process_call();
  xmlrpc_cgi_cleanup();
#endif // DRMAA_XMLRPC_CGI
  if (configuration.log_file) fclose (configuration.log_file);
skip_log_file_close:
  if (NULL != configuration.drmaa_init_contact) free (configuration.drmaa_init_contact);

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
