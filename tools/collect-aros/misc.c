#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/param.h>
#include <string.h>

#include "misc.h"

char *program_name;
void nonfatal(const char *msg, const char *errorstr)
{
    if (msg != NULL)
        fprintf(stderr, "%s: %s: %s\n" , program_name, msg, errorstr);
    else
        fprintf(stderr, "%s: %s\n" , program_name, errorstr);
}

void fatal(const char *msg, const char *errorstr)
{
    nonfatal(msg, errorstr);
    exit(EXIT_FAILURE);
}

void set_compiler_path(void)
{
    static int path_set = 0;

    if (!path_set)
    {
        char *compiler_path = getenv("COMPILER_PATH");
        char *path          = getenv("PATH");

        if (compiler_path && path)
	{
            char *new_path;
	    size_t compiler_path_len = strlen(compiler_path);
	    size_t path_len          = strlen(path);

            new_path = malloc(5 + compiler_path_len + 1 + path_len + 1);
            if (new_path)
            {
                memcpy(new_path, "PATH=", 5);
                memcpy(new_path + 5, compiler_path, compiler_path_len);
                memcpy(new_path + 5 + compiler_path_len, ":", 1);
                memcpy(new_path + 5 + compiler_path_len + 1, path, path_len + 1);

	        if (putenv(new_path) == 0)
		    path_set = 1;
            }
	}
    }
}

#ifndef _HAVE_LIBIBERTY_

void *xmalloc(size_t size)
{
    void *ret = malloc(size);
    if (ret == NULL)
    {
        fatal("xmalloc", strerror(errno));
    }

    return ret;
}

char *make_temp_file(char *suffix __attribute__((unused)))
{
    int fd;
    /* If you're unlucky enough to not have libiberty available, you'll have
       to live with temporary files in /tmp and no suffix; it's ok for our own
       purposes,  */
    char template[] = "/tmp/catmpXXXXXX";

    fd = mkstemp(template);
    if (fd == -1)
        return NULL;

    if (close(fd) != 0)
        fatal("make_temp_file()/close()", strerror(errno));

    return strdup(template);
}

#endif
