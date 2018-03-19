#ifndef _BOGGLE_PARSE_H_
#define _BOGGLE_PARSE_H_

struct Options
{
    char *dict; /* dictionary file */
    unsigned short size; /* board size (4 or 5) */
    unsigned short minlength; /* min length of a valid word */
    unsigned long time; /* game length in seconds */
    int sfw; /* inappropriate insults allowed? (boolean) */
    int pbc; /* periodic boundary conditions? (boolean) */
};

int parse(char *path, struct Options *cfg);

void free_options(struct Options *cfg);

#endif // _BOGGLE_PARSE_H_

