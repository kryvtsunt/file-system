#ifndef SLIST_H
#define SLIST_H

// struct for slist
typedef struct slist {
    char* data;
    int   refs;
    struct slist* next;
  int idx;
} slist;

slist* s_cons(const char* text, slist* rest);
void   s_free(slist* xs);
// splits the slist in two
slist* s_split(const char* text, char delim);
// acquired last element of linked list
slist* slist_last(slist* list);


#endif

