//                                                                       
// This program is distributed in the hope that it will be useful,       
// but WITHOUT ANY WARRANTY; without even the implied warranty of        
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
// GNU General Public License for more details.                          
//                                                                       
// You should have received a copy of the GNU General Public License     
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "config.h"
#include <foreach.h>

#define LINE_BUFFER_SIZE 1024

typedef struct {
  FILE *conf_file;
  char line_buffer[LINE_BUFFER_SIZE];
} next_arg;

static void *_next_line (void *garbage, void *_next_arg) {
  next_arg *n = (next_arg*)_next_arg;

  return fgets (n->line_buffer, LINE_BUFFER_SIZE, n->conf_file);
}

typedef struct {
  void (*consume) (char*, char*, void*);
  void *consume_arg;
} consume_wrapper_arg;

static void consume_wrapper (void *_line, void *_consume_wrapper_arg) {
  char *l = (char*)_line;
  consume_wrapper_arg *c = (consume_wrapper_arg*)_consume_wrapper_arg;

  char *name; char *val; char *eval;
  for (name = l; *name == ' '; name++);
  for (val = name; *val != ' ' && *val != '='; val++);
  *(val++) = 0;
  for (; *val == ' ' || *val == '='; val++);
  for (eval = val; *eval && *eval != ' ' && *eval != '\n' && *eval != '\r'; eval++);
  *eval = 0;

  c->consume (name, val, c->consume_arg);
}

void load (int err_sz, char *error,
	   FILE *conf_file,
	   void (*consume) (char *name, char *value, void *arg), void *consume_arg) {
  next_arg n; n.conf_file = conf_file;
  consume_wrapper_arg c; c.consume = consume; c.consume_arg = consume_arg;

  foreach_stream (_next_line (NULL, &n), _next_line, &n, consume_wrapper, &c);
}

#ifdef CONFIG_TEST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_consume (char *n, char *v, void *garbage) {
  printf ("name=%s; value=%s\n", n, v);
}

int main (void) {
  FILE *f = fopen ("test.conf", "r");

  if (f) load (0, NULL, f, test_consume, NULL);
  else printf ("no test.conf found\n");

  return 0;
}

#endif
