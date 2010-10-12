//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * stub-drmaa.c
 *
 *  Created on: Oct 12, 2010
 *      Author: levk
 */

#include <stdio.h>
#include <stdlib.h>
#include <drmaa.h>

int drmaa_init(const char *contact, char *error_diagnosis,
               size_t error_diag_len) {
  return DRMAA_ERRNO_SUCCESS;
}

int drmaa_allocate_job_template(drmaa_job_template_t **jt,
                                char *error_diagnosis, size_t error_diag_len) {
  *jt = (drmaa_job_template_t*) 0x55;
  return DRMAA_ERRNO_SUCCESS;
}

int drmaa_delete_job_template(drmaa_job_template_t *jt, char *error_diagnosis,
                              size_t error_diag_len) {
  return DRMAA_ERRNO_SUCCESS;
}

int drmaa_set_attribute(drmaa_job_template_t *jt, const char *name,
                        const char *value, char *error_diagnosis,
                        size_t error_diag_len) {
  return DRMAA_ERRNO_SUCCESS;
}

int drmaa_set_vector_attribute(drmaa_job_template_t *jt, const char *name,
                               const char *value[], char *error_diagnosis,
                               size_t error_diag_len) {
  return DRMAA_ERRNO_SUCCESS;
}

int drmaa_run_job(char *job_id, size_t job_id_len,
                  const drmaa_job_template_t *jt, char *error_diagnosis,
                  size_t error_diag_len) {
  return DRMAA_ERRNO_SUCCESS;
}

int drmaa_control(const char *jobid, int action, char *error_diagnosis,
                  size_t error_diag_len) {
  return DRMAA_ERRNO_SUCCESS;
}

int drmaa_job_ps(const char *job_id, int *remote_ps, char *error_diagnosis,
                 size_t error_diag_len) {
  *remote_ps = DRMAA_PS_DONE;
  return DRMAA_ERRNO_SUCCESS;
}
