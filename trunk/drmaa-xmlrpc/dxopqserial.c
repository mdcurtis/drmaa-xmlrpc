//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * dxopqserial.c
 *
 *  Created on: Nov 5, 2010
 *      Author: levk
 */

#include <stdlib.h>
#include <stdio.h>
#include <drmaa.h>

#include "dxopqserial.h"

#define MAX_ADDRESS_ASCII_LENGTH 50

static void *serialize_pointer (void *source) {
  char *result = (char*) malloc (MAX_ADDRESS_ASCII_LENGTH);
  sprintf (result, "%p", source);
  return result;
}

static void release_serialized_pointer (void *sptr) { free (sptr); }

static void *deserialize_pointer (void *sptr) {
	char *address = (char*) sptr;
	void *result = NULL;
	sscanf (address, "%p", &result);
	return result;
}

serialized_job_template_t serialize_job_template (drmaa_job_template_t *jt) { return (serialized_job_template_t) serialize_pointer (jt); }
void release_serialized_job_template (serialized_job_template_t sjt) { release_serialized_pointer (sjt); }
drmaa_job_template_t *deserialize_job_template (serialized_job_template_t sjt) { return (drmaa_job_template_t*) deserialize_pointer (sjt); }

serialized_attr_names_t serialize_attr_names (drmaa_attr_names_t *an) { return (serialized_attr_names_t) serialize_pointer (an); }
void release_serialized_attr_names (serialized_attr_names_t san) { release_serialized_pointer (san); }
drmaa_attr_names_t *deserialize_attr_names (serialized_attr_names_t san) { return (drmaa_attr_names_t*) deserialize_pointer (san); }

serialized_attr_values_t serialize_attr_values (drmaa_attr_values_t *av) { return (serialized_attr_values_t) serialize_pointer (av); }
void release_serialized_attr_values (serialized_attr_values_t sav) { release_serialized_pointer (sav); }
drmaa_attr_values_t *deserialize_attr_values (serialized_attr_values_t sav) { return (drmaa_attr_values_t*) deserialize_pointer (sav); }

serialized_job_ids_t serialize_job_ids (drmaa_job_ids_t *ji) { return (serialized_job_ids_t) serialize_pointer (ji); }
void release_serialized_job_ids (serialized_job_ids_t sji) { release_serialized_pointer (sji); }
drmaa_job_ids_t *deserialize_job_ids (serialized_job_ids_t sji) { return (drmaa_job_ids_t*) deserialize_pointer (sji); }
