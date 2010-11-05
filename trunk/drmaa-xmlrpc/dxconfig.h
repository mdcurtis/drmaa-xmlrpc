//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * dxconfig.h
 *
 *  Created on: Nov 5, 2010
 *      Author: levk
 */

#ifndef DXCONFIG_H
#define DXCONFIG_H

#define CONF_FILE_PATH_ENV_NAME_FOR_CGI "DRMAA_XMLRPC_CGI_CONF_PATH"
extern char *_DEFAULT_ABYSS_LOG_FILE_NAME;

struct {
#ifndef DRMAA_XMLRPC_CGI
  char *conf_file;
#endif

  int daemon;
  int port;

  FILE *pid_file;
  FILE *log_file;
  char *log_file_name;
  unsigned long long log_mask;
  char *abyss_log_file_name;

  char *drmaa_init_contact;
} configuration;

void _init_config (void); // initializes defaults
void _log_config (void); // logs conf values
void _config_consume_log_mask (char *n, char *v, void *garbage); // pass this to <config.h>.load(), only updates log mask
void _config_consume (char *n, char *v, void *garbage); // loads everything

#endif /* DXCONFIG_H */
