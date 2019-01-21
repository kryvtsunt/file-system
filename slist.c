#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#include "slist.h"

// cons a piece of text to front of slist
slist*
s_cons(const char* text, slist* rest)
{
    slist* xs = malloc(sizeof(slist));
    xs->data = strdup(text);
    xs->refs = 1;
    xs->next = rest;
    if (xs->next == NULL) {
      xs->idx = 1;
    }
    else {
      xs->idx = xs->next->idx + 1;
    }
    return xs;
}

// clear up the data of the slist, not needed anymore
void
s_free(slist* xs)
{
    if (xs == 0) {
        return;
    }

    xs->refs -= 1;

    if (xs->refs == 0) {
        s_free(xs->next);
        free(xs->data);
        free(xs);
    }
}

// splits the slist into two
slist*
s_split(const char* text, char delim)
{
    if (*text == 0) {
        return 0;
    }

    int plen = 0;
    while (text[plen] != 0 && text[plen] != delim) {
        plen += 1;
    }

    int skip = 0;
    if (text[plen] == delim) {
        skip = 1;
    }

    slist* rest = s_split(text + plen + skip, delim);
    char*  part = alloca(plen + 2);
    memcpy(part, text, plen);
    part[plen] = 0;

    return s_cons(part, rest);
}

// simply drills through the slist and gets the last thing
slist*
slist_last(slist* list) {
  while(list->next != NULL) {
    list = list->next;
  }
  return list;
}
