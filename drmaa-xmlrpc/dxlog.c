//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * dxlog.c
 *
 *  Created on: Nov 5, 2010
 *      Author: levk
 */

#include "dxlog.h"

char *_log_mask_to_level (unsigned long long mask) {
  if (mask & SEVERE) return "SEVERE";
  else if (mask & WARNING) return "WARNING";
  else if (mask & INFO) return "INFO";
  else return "TRACE";
}
