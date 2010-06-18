// export LD_LIBRARY_PATH=/home/lgrcws/drmaaxml2/lib
// gcc -I/home/lgrcws/src/xmlrpc-c-1.16.29-I/usr/include -I/home/lgrcws/drmaaxml2/include -L/usr/lib64 -L/home/lgrcws/drmaaxml2/lib -ldrmaa -lxmlrpc -lxmlrpc_server -lxmlrpc_server_abyss -lxmlrpc_abyss -lxmlrpc_xmlparse -lxmlrpc_xmltok -lxmlrpc_util -o drmaa-xmlrpc drmaa-xmlrpc.c
// ./drmaa-xmlrpc

#undef DRMAA_95

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <drmaa.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>
#include <sys/signal.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define DEFAULT_TCP_PORT 56123
#define ERROR_DIAGNOSIS_MAX_SIZE 2048
#define ATTRIBUTE_NAME_MAX_SIZE 2048
#define ATTRIBUTE_VALUE_MAX_SIZE 2048
#define JOB_ID_MAX_SIZE 2048
#define MAX_VECTOR_ATTRIBUTES 2048
#define DEFAULT_LOG_FILE_NAME "/tmp/xmlrpc_log"
#define DEFAULT_PID_FILE_NAME "/tmp/drmaa-xmlrpc.pid"

static xmlrpc_value *array_append(xmlrpc_env * const env, xmlrpc_value * const array, xmlrpc_value * const value){
  xmlrpc_array_append_item(env, array, value);
  return array;
}

static xmlrpc_value *new_pointer(xmlrpc_env * const env, void *pointer){
  size_t sz = sizeof(pointer);
  char send[] = "0000000000000000";
  char *fmt;
  if(sz == 4) fmt = "%x";
  else if(sz == 8) fmt = "%llx";
  else{
    fprintf(stderr,
            "This machine is neither 64 nor 32 bit. It must "
            "be a toaster, and this server is meant to be ru"
            "nning on a head node of a cluster.\n");
    exit(-1);
  }
  sprintf(send, fmt, pointer);
  return xmlrpc_build_value(env, "s", send);
}

static void *read_pointer(xmlrpc_env * const env, xmlrpc_value *value){
  void *result = NULL;
  size_t sz = sizeof(result);
  char *recv;
  char *fmt;
  xmlrpc_decompose_value(env, value, "s", &recv);
  if(!env->fault_occurred){
    if(sz == 4) fmt = "%x";
    else if(sz == 8) fmt = "%llx";
    else{
      fprintf(stderr,
	      "This machine is neither 64 nor 32 bit. It must "
	      "be a toaster, and this server is meant to be ru"
	      "nning on a head node of a cluster.\n");
      exit(-1);
    }
    sscanf(recv, fmt, &result);
  }
  free(recv);
  return result;
}

#if 0
int drmaa_get_next_attr_name(drmaa_attr_names_t* values, char *value,
                             size_t value_len);
int drmaa_get_next_attr_value(drmaa_attr_values_t* values, char *value,
                              size_t value_len);
int drmaa_get_next_job_id(drmaa_job_ids_t* values, char *value,
                          size_t value_len);

int drmaa_get_num_attr_names(drmaa_attr_names_t* values, int *size);
int drmaa_get_num_attr_values(drmaa_attr_values_t* values, int *size);
int drmaa_get_num_job_ids(drmaa_job_ids_t* values, int *size);

void drmaa_release_attr_names(drmaa_attr_names_t* values);
void drmaa_release_attr_values(drmaa_attr_values_t* values);
void drmaa_release_job_ids(drmaa_job_ids_t* values);
#endif

/**
 * @return rc,jt,error
 */
static xmlrpc_value *
xmlrpc_drmaa_allocate_job_template(xmlrpc_env * const env,
				   xmlrpc_value * const param_array,
				   void * const xmlrpc_data){
  drmaa_job_template_t *jt;
  char error[ERROR_DIAGNOSIS_MAX_SIZE]; error[0] = 0;

  int rc;
  while((rc = drmaa_allocate_job_template(&jt, error, sizeof(error))) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

  xmlrpc_value *result = xmlrpc_array_new(env);
  array_append(env, result,
	       array_append(env,
			    array_append(env, xmlrpc_array_new(env), xmlrpc_build_value(env, "s", "rc")),
			    xmlrpc_build_value(env, "i", rc)));
  array_append(env, result,
	       array_append(env,
			    array_append(env, xmlrpc_array_new(env), xmlrpc_build_value(env, "s", "jt")),
			    new_pointer(env, jt)));
  array_append(env, result,
	       array_append(env,
			    array_append(env, xmlrpc_array_new(env), xmlrpc_build_value(env, "s", "error")),
			    xmlrpc_build_value(env, "s", error)));

  return result;
}

/**
 * @param jt
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_delete_job_template(xmlrpc_env * const env,
				 xmlrpc_value * const param_array,
				 void * const xmlrpc_data){
  xmlrpc_value *jt_b64 = NULL;
  size_t sz;
  drmaa_job_template_t *jt;
  char error[ERROR_DIAGNOSIS_MAX_SIZE];
  xmlrpc_value *result = NULL;

  xmlrpc_array_read_item(env, param_array, 0, &jt_b64);
  jt = (drmaa_job_template_t *)read_pointer(env, jt_b64);
  if(!env->fault_occurred){
    int rc;
    while((rc = drmaa_delete_job_template(jt, error, sizeof(error))) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    result = xmlrpc_build_value(env,
				"((si)(ss))",
				"rc", rc, "error", error);
  }
  if(jt_b64 != NULL) xmlrpc_DECREF(jt_b64);
  return result;
}

/**
 * @param jt,name,value
 * @return rc,error
 */
static xmlrpc_value *
xmlrpc_drmaa_set_attribute(xmlrpc_env * const env,
			   xmlrpc_value * const param_array,
			   void * const xmlrpc_data){
  xmlrpc_value *jt_b64 = NULL;
  size_t sz;
  drmaa_job_template_t *jt;
  xmlrpc_value *name_itm = NULL;
  char *name = NULL;
  xmlrpc_value *value_itm = NULL;
  char *value = NULL;
  char error[ERROR_DIAGNOSIS_MAX_SIZE];
  xmlrpc_value *result = NULL;

  xmlrpc_array_read_item(env, param_array, 0, &jt_b64);
  jt = (drmaa_job_template_t *)read_pointer(env, jt_b64);
  xmlrpc_array_read_item(env, param_array, 1, &name_itm);
  xmlrpc_read_string(env, name_itm, (const char**)&name);
  xmlrpc_array_read_item(env, param_array, 2, &value_itm);
  xmlrpc_read_string(env, value_itm, (const char**)&value);
  if(!env->fault_occurred){
    int rc;
    while((rc = drmaa_set_attribute(jt, name, value, error, sizeof(error))) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    result = xmlrpc_build_value(env,
                                "((si)(ss))",
                                "rc", rc, "error", error);
  }
  if(jt_b64) xmlrpc_DECREF(jt_b64);
  if(name_itm) xmlrpc_DECREF(name_itm);
  if(value_itm) xmlrpc_DECREF(value_itm);
  free(name);
  free(value);
  return result;
}

/**
 * @param jt,name
 * @return rc,value,error
 */
static xmlrpc_value *
xmlrpc_drmaa_get_attribute(xmlrpc_env * const env,
			   xmlrpc_value * const param_array,
			   void * const xmlrpc_data){
  xmlrpc_value *jt_b64 = NULL;
  size_t sz;
  drmaa_job_template_t *jt;
  xmlrpc_value *name_itm = NULL;
  char *name = NULL;
  char value[ATTRIBUTE_VALUE_MAX_SIZE];
  char error[ERROR_DIAGNOSIS_MAX_SIZE];
  xmlrpc_value *result = NULL;

  xmlrpc_array_read_item(env, param_array, 0, &jt_b64);
  jt = (drmaa_job_template_t *)read_pointer(env, jt_b64);
  xmlrpc_array_read_item(env, param_array, 1, &name_itm);
  xmlrpc_read_string(env, name_itm, (const char**)&name);
  if(!env->fault_occurred){
    int rc;
    while((rc = drmaa_get_attribute(jt, name, value, sizeof(value), error, sizeof(error))) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    result = xmlrpc_build_value(env,
                                "((si)(ss)(ss))",
                                "rc", rc, "value", value, "error", error);
  }
  if(jt_b64) xmlrpc_DECREF(jt_b64);
  if(name_itm) xmlrpc_DECREF(name_itm);
  free(name);
  return result;
}

  /**
   * @param jt,name,values
   * @return rc,error
   */
static xmlrpc_value *
xmlrpc_drmaa_set_vector_attribute(xmlrpc_env * const env,
				  xmlrpc_value * const param_array,
				  void * const xmlrpc_data){
  xmlrpc_value *jt_b64 = NULL;
  size_t sz;
  drmaa_job_template_t *jt;
  xmlrpc_value *name_itm = NULL;
  char *name = NULL;
  xmlrpc_value *value_itm = NULL;
  xmlrpc_value *array_itm = NULL;
  char *value[MAX_VECTOR_ATTRIBUTES];
  char error[ERROR_DIAGNOSIS_MAX_SIZE];
  xmlrpc_value *result = NULL;
  int i;
  for(i = MAX_VECTOR_ATTRIBUTES; --i >= 0; value[i] = NULL);

  xmlrpc_array_read_item(env, param_array, 0, &jt_b64);
  jt = (drmaa_job_template_t *)read_pointer(env, jt_b64);
  xmlrpc_array_read_item(env, param_array, 1, &name_itm);
  xmlrpc_read_string(env, name_itm, (const char**)&name);
  xmlrpc_array_read_item(env, param_array, 2, &value_itm);
  size_t array_sz = xmlrpc_array_size(env, param_array);
  for(i = 0; i < MAX_VECTOR_ATTRIBUTES - 1 && i < array_sz; i++){
    xmlrpc_array_read_item(env, value_itm, i, &array_itm);
    xmlrpc_read_string(env, array_itm, (const char**)&(value[i]));
    if(array_itm) xmlrpc_DECREF(array_itm);
  }
  if(!env->fault_occurred){
    int rc;
    while((rc = drmaa_set_vector_attribute(jt, name, (const char **)value, error, sizeof(error))) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    result = xmlrpc_build_value(env,
                                "((si)(ss))",
                                "rc", rc, "error", error);
  }
  if(jt_b64) xmlrpc_DECREF(jt_b64);
  if(name_itm) xmlrpc_DECREF(name_itm);
  if(value_itm) xmlrpc_DECREF(value_itm);
  free(name);
  while(--i >= 0) if(value[i]) free(value[i]);
  return result;
}

#if 0
int drmaa_get_vector_attribute(drmaa_job_template_t *jt, const char *name,
                               drmaa_attr_values_t **values,
                               char *error_diagnosis, size_t error_diag_len);
int drmaa_get_attribute_names(drmaa_attr_names_t **values,
                              char *error_diagnosis, size_t error_diag_len);
int drmaa_get_vector_attribute_names(drmaa_attr_names_t **values,
                                     char *error_diagnosis,
                                     size_t error_diag_len);
#endif

/**
 * @param jt
 * @return rc,jobid,error
 */
static xmlrpc_value *
xmlrpc_drmaa_run_job(xmlrpc_env * const env,
		     xmlrpc_value * const param_array,
		     void * const xmlrpc_data){
  xmlrpc_value *jt_b64 = NULL;
  size_t sz;
  drmaa_job_template_t *jt;
  char error[ERROR_DIAGNOSIS_MAX_SIZE];
  char jobid[JOB_ID_MAX_SIZE];
  xmlrpc_value *result = NULL;

  xmlrpc_array_read_item(env, param_array, 0, &jt_b64);
  jt = (drmaa_job_template_t *)read_pointer(env, jt_b64);
  if(!env->fault_occurred){
    int rc;
    while((rc = drmaa_run_job(jobid, sizeof(jobid), jt, error, sizeof(error))) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    result = xmlrpc_build_value(env,
                                "((si)(ss)(ss))",
                                "rc", rc, "jobid", jobid, "error", error);
  }
  if(jt_b64 != NULL) xmlrpc_DECREF(jt_b64);
  return result;
}

#if 0
int drmaa_run_bulk_jobs(drmaa_job_ids_t **jobids,
                        const drmaa_job_template_t *jt, int start, int end,
                        int incr, char *error_diagnosis, size_t error_diag_len);
int drmaa_control(const char *jobid, int action, char *error_diagnosis,
                  size_t error_diag_len);
int drmaa_synchronize(const char *job_ids[], signed long timeout, int dispose,
                      char *error_diagnosis, size_t error_diag_len);
int drmaa_wait(const char *job_id, char *job_id_out, size_t job_id_out_len,
               int *stat, signed long timeout, drmaa_attr_values_t **rusage, 
               char *error_diagnosis, size_t error_diag_len);
int drmaa_wifexited(int *exited, int stat, char *error_diagnosis,
                    size_t error_diag_len);
int drmaa_wexitstatus(int *exit_status, int stat, char *error_diagnosis,
                      size_t error_diag_len);
int drmaa_wifsignaled(int *signaled, int stat, char *error_diagnosis,
                      size_t error_diag_len);
int drmaa_wtermsig(char *signal, size_t signal_len, int stat,
                   char *error_diagnosis, size_t error_diag_len);
int drmaa_wcoredump(int *core_dumped, int stat, char *error_diagnosis,
                    size_t error_diag_len);
int drmaa_wifaborted(int *aborted, int stat, char *error_diagnosis,
                     size_t error_diag_len);
#endif

/**
 * @param jobid
 * @return rc,status,error
 */
static xmlrpc_value *
xmlrpc_drmaa_job_ps(xmlrpc_env * const env,
		    xmlrpc_value * const param_array,
		    void * const xmlrpc_data){
  char *jobid = NULL;
  xmlrpc_decompose_value(env, param_array, "(s)", &jobid);
  int status;
  char error[ERROR_DIAGNOSIS_MAX_SIZE];
  xmlrpc_value *result = NULL;
  if(!env->fault_occurred){
    int rc;
    while((rc = drmaa_job_ps(jobid, &status, error, sizeof(error))) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);
    result = xmlrpc_build_value(env,
                                "((si)(si)(ss))",
                                "rc", rc, "status", status, "error", error);
  }
  free(jobid);
  return result;
}

#if 0
const char *drmaa_strerror(int drmaa_errno);
int drmaa_get_contact(char *contact, size_t contact_len, 
         char *error_diagnosis, size_t error_diag_len);
int drmaa_version(unsigned int *major, unsigned int *minor, 
         char *error_diagnosis, size_t error_diag_len);
int drmaa_get_DRM_system(char *drm_system, size_t drm_system_len, 
         char *error_diagnosis, size_t error_diag_len);
int drmaa_get_DRMAA_implementation(char *drmaa_impl, size_t drmaa_impl_len, 
         char *error_diagnosis, size_t error_diag_len);
#endif

int main(int argc, char **argv){

  // xml-rpc initialization  
  xmlrpc_server_abyss_parms serverparm;
  xmlrpc_registry * registryP;
  xmlrpc_env env;

  xmlrpc_env_init(&env);
  registryP = xmlrpc_registry_new(&env);

  serverparm.port_number = argc - 1 < 1 ? DEFAULT_TCP_PORT : atoi(argv[1]);
  serverparm.registryP = registryP;
  serverparm.config_file_name = NULL;
  serverparm.log_file_name = argc - 1 < 2 ? DEFAULT_LOG_FILE_NAME : argv[2];
  char pid_file_command[2048];
  char *pid_file_name;
  sprintf(pid_file_command, "echo %d > %s\n", getpid(), pid_file_name = (argc - 1 < 3 ? DEFAULT_PID_FILE_NAME : argv[3]));
  system(pid_file_command);
  if (argc - 1 < 1) fprintf(stderr, "Usage: drmaa-xmlrpc [port [logfile [pidfile]]]. ");

  // using xml-rpc 1.06.39, so no add_method3()
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_next_attr_name", &xmlrpc_drmaa_get_next_attr_name, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_next_attr_value", &xmlrpc_drmaa_get_next_attr_value, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_next_job_id", &xmlrpc_drmaa_get_next_job_id, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_num_attr_names", &xmlrpc_drmaa_get_num_attr_names, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_num_attr_values", &xmlrpc_drmaa_get_num_attr_values, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_num_job_ids", &xmlrpc_drmaa_get_num_job_ids, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_release_attr_names", &xmlrpc_drmaa_release_attr_names, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_release_attr_values", &xmlrpc_drmaa_release_attr_values, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_release_job_ids", &xmlrpc_drmaa_release_job_ids, NULL);
  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_allocate_job_template", &xmlrpc_drmaa_allocate_job_template, NULL);
  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_delete_job_template", &xmlrpc_drmaa_delete_job_template, NULL);
  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_set_attribute", &xmlrpc_drmaa_set_attribute, NULL);
  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_attribute", &xmlrpc_drmaa_get_attribute, NULL);
  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_set_vector_attribute", &xmlrpc_drmaa_set_vector_attribute, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_vector_attribute", &xmlrpc_drmaa_get_vector_attribute, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_attribute_names", &xmlrpc_drmaa_get_attribute_names, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_vector_attribute_names", &xmlrpc_drmaa_get_vector_attribute_names, NULL);
  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_run_job", &xmlrpc_drmaa_run_job, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_run_bulk_jobs", &xmlrpc_drmaa_run_bulk_jobs, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_control", &xmlrpc_drmaa_control, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_synchronize", &xmlrpc_drmaa_synchronize, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_wait", &xmlrpc_drmaa_wait, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_wifexited", &xmlrpc_drmaa_wifexited, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_wexitstatus", &xmlrpc_drmaa_wexitstatus, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_wifsignaled", &xmlrpc_drmaa_wifsignaled, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_wtermsig", &xmlrpc_drmaa_wtermsig, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_wcoredump", &xmlrpc_drmaa_wcoredump, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_wifaborted", &xmlrpc_drmaa_wifaborted, NULL);
  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_job_ps", &xmlrpc_drmaa_job_ps, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_strerror", &xmlrpc_drmaa_strerror, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_contact", &xmlrpc_drmaa_get_contact, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_version", &xmlrpc_drmaa_version, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_DRM_system", &xmlrpc_drmaa_get_DRM_system, NULL);
//  xmlrpc_registry_add_method(&env, registryP, NULL, "drmaa_get_DRMAA_implementation", &xmlrpc_drmaa_get_DRMAA_implementation, NULL);
  
  // drmaa initialization
  char error[ERROR_DIAGNOSIS_MAX_SIZE];
  int rc = drmaa_init(NULL, error, sizeof(error));
  if(rc){ fprintf(stderr, "Failed to initialize DRMAA because: %s.\n", error); exit(1); }

  // It's a good idea to disable SIGPIPE signals; if client closes his end
  // of the pipe/socket, we'd rather see a failure to send a response than
  // get killed by the OS.
  signal(SIGPIPE, SIG_IGN);

  fprintf(stderr, "Starting XML-RPC to DRMAA server on port %d. Logging to %s. Pid file at %s.\n", 
	  serverparm.port_number, serverparm.log_file_name, pid_file_name);
  xmlrpc_server_abyss(&env, &serverparm, XMLRPC_APSIZE(log_file_name));

  return 0; // shouldn't get here
}

#ifdef  __cplusplus
}
#endif
