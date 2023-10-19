#include "fcaseopen.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#endif

#if !defined(_WIN32)
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include "../string_proxy.hpp"

// r must have strlen(path) + 2 bytes
static int casepath(char const *path, char *r)
{
  size_t l = strlen(path);
  char *p = (char*)alloca(l + 1);
  strcpy(p, path);
  size_t rl = 0;

  DIR *d;
  if(p[0] == '/')
  {
    d = opendir("/");
    p = p + 1;
  } else
  {
    d = opendir(".");
    r[0] = '.';
    r[1] = 0;
    rl = 1;
  }

  int last = 0;
  char *c = strsep(&p, "/");
  while(c)
  {
    if(!d)
    {
      return 0;
    }

    if(last)
    {
      closedir(d);
      return 0;
    }

    r[rl] = '/';
    rl += 1;
    r[rl] = 0;

    struct dirent *e = readdir(d);
    while(e)
    {
      if(strcasecmp(c, e->d_name) == 0)
      {
        strcpy(r + rl, e->d_name);
        rl += strlen(e->d_name);

        closedir(d);
        d = opendir(r);

        break;
      }

      e = readdir(d);
    }

    if(!e)
    {
      strcpy(r + rl, c);
      rl += strlen(c);
      last = 1;
    }

    c = strsep(&p, "/");
  }

  if(d) closedir(d);
  return 1;
}
#endif

FILE *fcaseopen(char const *path, char const *mode)
{
  FILE *f = fopen(path, mode);
#if !defined(_WIN32)
  if(!f)
  {
    char *r = (char*)alloca(strlen(path) + 2);
    if(casepath(path, r))
    {
      f = fopen(r, mode);
    }
  }
#endif
  return f;
}

void casechdir(char const *path)
{
#if !defined(_WIN32)
  char *r = (char*)alloca(strlen(path) + 2);
  if(casepath(path, r))
  {
    chdir(r);
  } else
  {
    errno = ENOENT;
  }
#else
  _chdir(path);
#endif
}

char* fixFileCase(char *path) {
#if defined(_WIN32)
  return path;
#else
  char *r = (char*)alloca(strlen(path) + 2);
  if(casepath(path, r)) {
    free(path);
    if(r[0] == '.' && r[1] == '/') r += 2;
    return strdup_s(r);
  } else {
    return path;
  }
#endif
}