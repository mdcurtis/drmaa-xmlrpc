//                                                                       
// This program is distributed in the hope that it will be useful,       
// but WITHOUT ANY WARRANTY; without even the implied warranty of        
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
// GNU General Public License for more details.                          
//                                                                       
// You should have received a copy of the GNU General Public License     
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FOREACH_H
#define FOREACH_H

void foreach_array ( // exec a function for every element of an array
		    int sz, void **arr, // array and its size
		    void (*fun)(void*, void*), void *fun_arg); // function and its last arg

void foreach_stream ( // exec a function for every element in a stream,
		      // NULL marks end of the stream
		     void *first, // first element of the stream
		     void *(*next)(void*, void*), void *next_arg, // function to get next
		     void (*fun)(void*, void*), void *fun_arg); // exec function

#endif
