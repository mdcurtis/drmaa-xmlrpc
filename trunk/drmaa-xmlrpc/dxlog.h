//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * dxlog.h
 *
 *  Created on: Nov 5, 2010
 *      Author: levk
 */

#ifndef DXLOG_H
#define DXLOG_H

#include <time.h>

#define SEVERE (1L << 63)
#define WARNING (1L << 62)
#define INFO (1L << 61)

#define CONFIG (1L)
#define TEMPLATE (1L << 1)
#define JOB (1L << 2)
#define DRMAA (1L << 3)
#define SYSTEM (1L << 4)
#define STAT (1L << 5)
#define LIST (1L << 6)

char *_log_mask_to_level (unsigned long long mask);

#define LOG(MASK, ... ) \
  if ((configuration.log_mask & (MASK)) && configuration.log_file) { \
    time_t rt; struct tm * ti; time ( &rt ); ti = localtime ( &rt ); \
    fprintf (configuration.log_file, "[%02d:%02d:%02d] %s ", \
             ti->tm_hour, ti->tm_min, ti->tm_sec, _log_mask_to_level (MASK)); \
    fprintf (configuration.log_file, __VA_ARGS__ ); fflush (configuration.log_file); } else

#endif /* DXLOG_H */
