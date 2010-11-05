//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * dxopqserial.h
 *
 *  Created on: Nov 5, 2010
 *      Author: levk
 */

#ifndef DXOPQSERIAL_H
#define DXOPQSERIAL_H

#define POINTER_SERIALIZED_XMLRPC_VALUE_TYPE "s"

#define JOB_TEMPLATE_SERIALIZED_XMLRPC_VALUE_TYPE POINTER_SERIALIZED_XMLRPC_VALUE_TYPE
typedef void *serialized_job_template_t;
serialized_job_template_t serialize_job_template (drmaa_job_template_t *jt);
void release_serialized_job_template (serialized_job_template_t sjt);
drmaa_job_template_t *deserialize_job_template (serialized_job_template_t sjt);

#define ATTR_NAMES_SERIALIZED_XMLRPC_VALUE_TYPE POINTER_SERIALIZED_XMLRPC_VALUE_TYPE
typedef void *serialized_attr_names_t;
serialized_attr_names_t serialize_attr_names (drmaa_attr_names_t *an);
void release_serialized_attr_names (serialized_attr_names_t san);
drmaa_attr_names_t *deserialize_attr_names (serialized_attr_names_t san);

#define ATTR_VALUES_SERIALIZED_XMLRPC_VALUE_TYPE POINTER_SERIALIZED_XMLRPC_VALUE_TYPE
typedef void *serialized_attr_values_t;
serialized_attr_values_t serialize_attr_values (drmaa_attr_values_t *av);
void release_serialized_attr_values (serialized_attr_values_t sav);
drmaa_attr_values_t *deserialize_attr_values (serialized_attr_values_t sav);

#define JOB_IDS_SERIALIZED_XMLRPC_VALUE_TYPE POINTER_SERIALIZED_XMLRPC_VALUE_TYPE
typedef void *serialized_job_ids_t;
serialized_job_ids_t serialize_job_ids (drmaa_job_ids_t *ji);
void release_serialized_job_ids (serialized_job_ids_t sji);
drmaa_job_ids_t *deserialize_job_ids (serialized_job_ids_t sji);

#endif /* DXOPQSERIAL_H */
