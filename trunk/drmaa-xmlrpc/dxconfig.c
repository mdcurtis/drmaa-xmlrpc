//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * dxconfig.c
 *
 *  Created on: Nov 5, 2010
 *      Author: levk
 */

#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "dxconfig.h"
#include "dxlog.h"

void _init_config (void) {
  configuration.daemon = 1;
  configuration.port = 41334;

  configuration.pid_file = NULL;
  configuration.log_file = NULL;
  configuration.log_mask = 0;
  configuration.abyss_log_file_name = _DEFAULT_ABYSS_LOG_FILE_NAME;
  configuration.drmaa_init_contact = NULL;
  configuration.log_file_name = NULL;
}

char *_DEFAULT_ABYSS_LOG_FILE_NAME = "/dev/null"; // it's not very useful for the most part

void _log_config () {
  LOG (INFO | CONFIG, "drmaa-xmlrpc configuration: { daemon=%d, port=%d, log_mask=0x%llX }\n",
       configuration.daemon, configuration.port, configuration.log_mask);
}


void _config_consume_log_mask (char *n, char *v, void *garbage) {
  if (strncmp (n, "log_mask", 50) == 0) configuration.log_mask = atoll (v);
}

void _config_consume (char *n, char *v, void *garbage) {
  if (strncmp (n, "daemon", 50) == 0) configuration.daemon = atoi (v);
  else if (strncmp (n, "port", 50) == 0) configuration.port = atoi (v);
#ifndef DRMAA_XMLRPC_CGI
  else if (strncmp (n, "pid_file", 50) == 0) configuration.pid_file = fopen (v, "w");
#endif
  else if (strncmp (n, "log_file", 50) == 0) {
#ifndef DRMAA_XMLRPC_CGI
    char *tmp = (char*) malloc (strlen (v) + 1);
    if (tmp) strcpy (configuration.log_file_name = tmp, v);
#endif
    configuration.log_file = fopen (v, "a");
  } else if (strncmp (n, "abyss_log_file_name", 50) == 0) {
    char *tmp = (char*) malloc (strlen (v) + 1);
    if (tmp) strcpy (configuration.abyss_log_file_name = tmp, v);
  } else if (strncmp (n, "drmaa_init_contact", 50) == 0) {
    char *tmp = (char*) malloc (strlen (v) + 1);
    if (tmp) strcpy (configuration.drmaa_init_contact = tmp, v);
  } else _config_consume_log_mask (n, v, garbage);
}
