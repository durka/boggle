#ifndef _BOGGLE_BOARD_H_
#define _BOGGLE_BOARD_H_

struct Board
{
    unsigned short size;
    char **tiles; /* dimensions will be 'size' by 'size'. chars are lowercase. don't forget that 'q' stands for 'qu'. */
    int *highlighted; /* array of length size^2, 0 for most but 1 for highlighted squares */
    int pbc; /* 0 for a regular board, 1 for allowing wraparound */
};

#define BOARD_INDEX(b, n) (b)->tiles[(n) % (b)->size][(n) / (b)->size]
#define DICE(n) ((n) == 4 ? dice4 : dice5)

/* actual tiles. initialized in board.c */
extern const char dice4[16][7];
extern const char dice5[25][7];
extern const char letters[8][417];

void make_board(struct Board *brd, unsigned short size, int pbc); /* allocate a board of the given size. allocates stuff inside the structure, not the struct itself */

void free_board(struct Board *brd); /* free stuff inside the board struct, not the struct itself */

void shuffle(struct Board *brd); /* shake up the tiles */

#ifndef BUILD
#   define WINDOW void
#endif
void print_board(WINDOW *win, struct Board *brd, int mode, char *prefix); /* print a formatted board to the screen */

int find_word(struct Board *brd, char *word, int *x, int *y); /* find a word on the board */
int subfind(struct Board *brd, char *word, int *x, int *y, int i); /* helper function for find_word */
int try_place(struct Board *brd, char *word, int *x, int *y, int i, int xx, int yy); /* helper function for subfind */

#endif // _BOGGLE_BOARD_H_

