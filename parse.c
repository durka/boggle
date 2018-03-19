#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "parse.h"


/* Parses a configuration file into a structure.
 * Assumes cfg already points to a valid struct.
 * Allocates stuff inside the struct -- call free_options when done!
 */
int parse(char *path, struct Options *cfg)
{
    FILE *file;
    char aline[128], *line = aline, *phash, *pequals;
    int equals, len, i;

    file = fopen(path, "r");
    if (!file)
    {
        perror(path);
        return errno;
    }

    while (fgets(aline, sizeof aline, file) != NULL)
    {
        line = aline; /* HACK: I want to increment the beginning of the string to trim spaces. 'aline' is an array (so I can't move it), so I use the pointer 'line' which starts off pointing at the same place */

        phash = strchr(line, '#');

        if (phash == line) continue; /* get rid of comments */
        if (phash != NULL) *phash = '\0'; /* if there was a comment, get rid of it */

        /* trim the line */
        while (*line != '\0' && isspace(*line)) ++line;
        len = strlen(line);
        for (i = len - 1; i >= 0 && isspace(line[i]); --i);
        if (i == 0) continue;
        line[i+1] = '\0';

        /* now we have the naked "name=value" */
        
        pequals = strchr(line, '=');
        if (pequals == NULL) continue; /* there wasn't an equals sign? whatever, skip it */
        *pequals = '\0'; /* the name ends at = */
        ++pequals; /* the value starts after = */
        
        /* now 'line' is "name" and 'pequals' is "value" */

        if (strcmp(line, "dict") == 0)
        {
            cfg->dict = malloc(strlen(pequals) + 1);
            strcpy(cfg->dict, pequals);
            continue;
        }
        if (strcmp(line, "size") == 0)
        {
            cfg->size = pequals[0] - '0'; /* this assumes the value is 4x4 or 5x5 */
            continue;
        }
        if (strcmp(line, "minlength") == 0)
        {
            cfg->minlength = atoi(pequals);
            continue;
        }
        if (strcmp(line, "time") == 0)
        {
            cfg->time = atoi(pequals);
            continue;
        }
        if (strcmp(line, "pbc") == 0)
        {
            cfg->pbc = (strcmp(pequals, "true") == 0) ? 1 : 0;
        }
        if (strcmp(line, "sfw") == 0)
        {
            cfg->sfw = (strcmp(pequals, "true") == 0) ? 1 : 0;
        }

        /* still here? illegal configuration name! whatever, go on to the next line */
    }

    fclose(file);
    return 0;
}

/* Frees any memory which is allocated within the cfg struct. Does not free the 'cfg' pointer itself. */
void free_options(struct Options *cfg)
{
    free(cfg->dict);
}

#ifndef BUILD
int main() /* just for testing */
{
    struct Options config;

    if (parse(".bogglerc", &config) != 0)
    {
        printf("Failed to parse configuration file.\n");
        return 1;
    }

    printf("dict is '%s'\nsize is '%s'\nminlength is %d\ntime is %g minutes\n", config.dict, config.size == 4 ? "4x4" : "5x5", config.minlength, config.time / 60.);

    free_options(&config);

    return 0;
}
#endif // BUILD

