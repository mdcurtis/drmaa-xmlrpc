//                                                                       
// This program is distributed in the hope that it will be useful,       
// but WITHOUT ANY WARRANTY; without even the implied warranty of        
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
// GNU General Public License for more details.                          
//                                                                       
// You should have received a copy of the GNU General Public License     
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "foreach.h"

void foreach_array (int sz, void **arr, void (*fun)(void*, void*), void *fun_arg) {
  int i;
  for (i = 0; sz > i ; i++) fun(arr[i], fun_arg);
}

void foreach_stream (void *first, void *(*next)(void*, void*), void *next_arg,
		     void (*fun)(void*,void*),void *fun_arg) {
  for (; first; first = next(first, next_arg)) fun(first, fun_arg);
}

#ifdef FOREACH_TEST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int a = 1;
int b = 2;
int c = 3;

void add_fun (void *add, void *sum) {
  *((int*)sum) += *((int*)add);
}

void *next_int (void *prev, void *garbage) {
  if (prev == (void*)&a) return (void*)&b;
  else if (prev == (void*)&b) return (void*)&c;
  else return NULL;
}

int main (void) {
  int *arr[] = { &a, &b, &c };
  int sum1, sum2;
  sum1 = sum2 = 0;

  foreach_array (3, (void**)arr, add_fun, (void*)&sum1);
  foreach_stream ((void*)&a, next_int, NULL, add_fun, (void*)&sum2);

  printf ("6 ?= %d(foreach_array) ?= %d(foreach_stream)\n", sum1, sum2);

  return !(sum1 == 6 && sum2 == 6);
}

#endif
