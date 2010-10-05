//                                                                       
// This program is distributed in the hope that it will be useful,       
// but WITHOUT ANY WARRANTY; without even the implied warranty of        
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
// GNU General Public License for more details.                          
//                                                                       
// You should have received a copy of the GNU General Public License     
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

void load (int err_sz, char *error,
	   FILE *conf_file,
	   void (*consume) (char *name, char *value, void *arg), void *consume_arg);

#endif
