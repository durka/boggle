#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef BUILD
    #include <ncurses.h>
#endif

#include "board.h"

#ifndef BUILD
/* no ncurses, so use printf instead of printw */
#   define WINDOW void
#   define wprintw(w,f...) printf(f)
#   define attron(s...)
#   define attroff(s...)
#endif

/* actual tiles. http://www2.research.att.com/~njas/sequences/a063000.txt */
const char dice4[16][7] = { "aaeegn",
                            "abbjoo",
                            "achops",
                            "affkps",
                            "aoottw",
                            "cimotu",
                            "deilrx",
                            "delrvy",
                            "distty",
                            "eeghnw",
                            "eeinsu",
                            "ehrtvw",
                            "eiosst",
                            "elrtty",
                            "himnqu",
                            "hlnnrz" };
const char dice5[25][7] = { "afirsy",
                            "adennn",
                            "aeeeem",
                            "aaafrs",
                            "aegmnn",
                            "aaeeee",
                            "aeegmu",
                            "aafirs",
                            "bjkqxz",
                            "ccenst",
                            "ceilpt",
                            "ceiilt",
                            "ceipst",
                            "dhlnor",
                            "dhlnor",
                            "ddhnot",
                            "dhhlor",
                            "ensssu",
                            "emottt",
                            "eiiitt",
                            "fiprsy",
                            "gorrvw",
                            "iprrry",
                            "nootuw",
                            "ooottu" };

const char letters[8][417] = {
    ".--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------..--------------.",
    "|      __      ||   ______     ||     ______   ||  ________    ||  _________   ||  _________   ||    ______    ||  ____  ____  ||     _____    ||     _____    ||  ___  ____   ||   _____      || ____    ____ || ____  _____  ||     ____     ||   ______     ||              ||  _______     ||    _______   ||  _________   || _____  _____ || ____   ____  || _____  _____ ||  ____  ____  ||  ____  ____  ||   ________   |",
    "|     /  \\     ||  |_   _ \\    ||   .' ___  |  || |_   ___ `.  || |_   ___  |  || |_   ___  |  ||  .' ___  |   || |_   ||   _| ||    |_   _|   ||    |_   _|   || |_  ||_  _|  ||  |_   _|     |||_   \\  /   _||||_   \\|_   _| ||   .'    `.   ||  |_   __ \\   || .--.         || |_   __ \\    ||   /  ___  |  || |  _   _  |  |||_   _||_   _||||_  _| |_  _| |||_   _||_   _||| |_  _||_  _| || |_  _||_  _| ||  |  __   _|  |",
    "|    / /\\ \\    ||    | |_) |   ||  / .'   \\_|  ||   | |   `. \\ ||   | |_  \\_|  ||   | |_  \\_|  || / .'   \\_|   ||   | |__| |   ||      | |     ||      | |     ||   | |_/ /    ||    | |       ||  |   \\/   |  ||  |   \\ | |   ||  /  .--.  \\  ||    | |__) |  ||/ .. \\        ||   | |__) |   ||  |  (__ \\_|  || |_/ | | \\_|  ||  | |    | |  ||  \\ \\   / /   ||  | | /\\ | |  ||   \\ \\  / /   ||   \\ \\  / /   ||  |_/  / /    |",
    "|   / ____ \\   ||    |  __'.   ||  | |         ||   | |    | | ||   |  _|  _   ||   |  _|      || | |  ______  ||   |  __  |   ||      | |     ||   _  | |     ||   |  __'.    ||    | |   _   ||  | |\\  /| |  ||  | |\\ \\| |   ||  | |    | |  ||    |  ___/   ||||  ||  || || ||   |  __ /    ||   '.___`-.   ||     | |      ||  | '    ' |  ||   \\ \\ / /    ||  | |/  \\| |  ||    > `' <    ||    \\ \\/ /    ||     .'.' _   |",
    "| _/ /    \\ \\_ ||   _| |__) |  ||  \\ `.___.'\\  ||  _| |___.' / ||  _| |___/ |  ||  _| |_       || \\ `.__]   _| ||  _| |  | |_  ||     _| |_    ||  | |_' |     ||  _| |  \\ \\_  ||   _| |__/ |  || _| |_\\/_| |_ || _| |_\\   |_  ||  \\  `--'  /  ||   _| |_      ||\\ `'  \\ ||_|| ||  _| |  \\ \\_  ||  |`\\____) |  ||    _| |_     ||   \\ `--' /   ||    \\ ' /     ||  |   /\\   |  ||  _/ /'`\\ \\_  ||    _|  |_    ||   _/ /__/ |  |",
    "||____|  |____|||  |_______/   ||   `._____.'  || |________.'  || |_________|  || |_____|      ||  `._____.'   || |____||____| ||    |_____|   ||  `.___.'     || |____||____| ||  |________|  |||_____||_____||||_____|\\____| ||   `.____.'   ||  |_____|     || `._.\\| \\\\_// || |____| |___| ||  |_______.'  ||   |_____|    ||    `.__.'    ||     \\_/      ||  |__/  \\__|  || |____||____| ||   |______|   ||  |________|  |",
    "'--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------''--------------'"
}; /* http://patorjk.com/software/taag/ Blocks font */

/* Allocate a board with the given size.
 * Allocates the internal 2-D char array, not the struct itself.
 * Does not initialize the tiles at all. */
void make_board(struct Board *brd, unsigned short size, int pbc)
{
    int i, j;

    brd->size = size;
    brd->pbc = pbc;

    brd->highlighted = malloc(size * size * sizeof(char*));

    brd->tiles = malloc(size * sizeof(char*));
    for (i = 0; i < size; ++i)
    {
        brd->tiles[i] = malloc(size * sizeof(char));
        for (j = 0; j < size; ++j) brd->highlighted[i*j] = 0;
    }
}

/* Free a board (the stuff inside, not the struct itself). */
void free_board(struct Board *brd)
{
    int i;

    for (i = 0; i < brd->size; ++i)
    {
        free(brd->tiles[i]);
    }
    
    free(brd->tiles);
}

/* Helper function: random integer between a and b.
 * Assumes RNG has already been seeded. */
int randint(int a, int b)
{
    return rand() % (b - a + 1) + a;
}

/* Helper function: swap two numbers. */
int swap(char *x, char *y)
{
    *x ^= *y;
    *y ^= *x;
    *x ^= *y;
}

/* Shuffle a board. */
void shuffle(struct Board *brd)
{
    int i, j;

    /* first we need to shuffle the tiles themselves.
     * fill up the array in order. Then, go through it,
     * swapping every element with itself or something
     * after it. http://adrianquark.blogspot.com/2008/09/how-to-shuffle-array-correctly.html */
    for (i = 0; i < brd->size; ++i)
    {
        for (j = 0; j < brd->size; ++j)
        {
            brd->tiles[i][j] = (i * brd->size) + j;
        }
    }
    /* tiles are numbered in order now (eventually they will have chars instead of numbers) */
    for (i = 0; i < brd->size * brd->size; ++i)
    {
        j = randint(j, brd->size * brd->size - 1);
        swap(&BOARD_INDEX(brd, i), &BOARD_INDEX(brd, j));
    }
    
    /* now we have the tiles numbered, and we use that as an index into the
     * tile formats, selecting a random char out of the six for each one */
    for (i = 0; i < brd->size * brd->size; ++i)
    {
        j = randint(0, 5);
        BOARD_INDEX(brd, i) = DICE(brd->size)[BOARD_INDEX(brd, i)][j];
    }
}

/* Print a board, nicely formatted, to the screen.
 * If mode is 0, uses regular letters. If mode is 1, uses large-format letters. */
void print_board(WINDOW *win, struct Board *brd, int mode, char *prefix)
{
    int i, j, k;
    char c;

    switch (mode)
    {
        case 0:
            /* top border */
            wprintw(win, "%s+---", prefix);
            for (i = 1; i < brd->size; ++i)
            {
                wprintw(win, "+---");
            }
            wprintw(win, "+\n");

            /* tile rows */
            for (i = 0; i < brd->size; ++i)
            {
                wprintw(win, "%s", prefix);
                for (j = 0; j < brd->size; ++j)
                {
                    c = brd->tiles[i][j];
                    if (brd->highlighted[i*brd->size + j]) wattron(win, A_REVERSE);
                    if (c == 'q') /* need to special-case Qu */
                    {
                        wprintw(win, "| Qu");
                    }
                    else
                    {
                        wprintw(win, "| %c ", c + 'A' - 'a');
                    }
                    if (brd->highlighted[i*brd->size + j]) wattroff(win, A_REVERSE);
                }
                wprintw(win, "|\n%s", prefix);
                for (j = 0; j < brd->size; ++j)
                {
                    wprintw(win, "%c---", i == brd->size-1 ? '+' : '|'); /* the border changes at the bottom */
                }
                wprintw(win, "%c\n", i == brd->size-1 ? '+' : '|');
            }
            break;
        case 1:
            /* tile rows */
            for (i = 0; i < brd->size; ++i) /* loop for rows */
            {
                for (k = 0; k < 8; ++k) /* loop for parts of rows */
                {
                    wprintw(win, "%s", prefix);
                    for (j = 0; j < brd->size; ++j) /* loop for columns */
                    {
                        if (brd->highlighted[i*brd->size + j]) wattron(win, A_REVERSE);
                        waddnstr(win, &letters[k][(brd->tiles[i][j] - 'a') * 16], 16);
                        if (brd->highlighted[i*brd->size + j]) wattroff(win, A_REVERSE);
                    }
                    wprintw(win, "\n");
                }
            }
            break;
    }
}

/* helper function: searches two arrays side-by-side for an ordered pair */
int in_array(int left[], int right[], int le, int ri, int len)
{
    int i;

    for (i = 0; i < len; ++i)
    {
        if (left[i] == le && right[i] == ri) return i;
    }

    return -1;
}

int try_place(struct Board *brd, char *word, int *x, int *y, int i, int xx, int yy)
{
    int newx = x[i-1] + xx, newy = y[i-1] + yy;

    if (brd->pbc)
    {
        if (newx < 0)
            newx = brd->size + newx;
        if (newx >= brd->size)
            newx -= brd->size;

        if (newy < 0)
            newy = brd->size + newy;
        if (newy >= brd->size)
            newy -= brd->size;
    }
    else
    {
        if (newx < 0 || newx >= brd->size
                || newy < 0 || newy >= brd->size)
            return 0;
    }

    if (brd->tiles[newx][newy] != word[i] || in_array(x, y, newx, newy, i-1) != -1) return 0;

    x[i] = newx;
    y[i] = newy;

    return subfind(brd, word, x, y, i + 1);
}

/* helper for find_word. starts a given a position and tries to continue a word. */
int subfind(struct Board *brd, char *word, int *x, int *y, int i)
{
    int ret;

    /* need a base case for the recursion */
    /* if we're still recursing at the end of the word, that means success */
    if (!word[i]) return i;

    /* U after Q should be disregarded because it is on the same tile, so recurse immediately */
    if (word[i] == 'u' && word[i-1] == 'q') return subfind(brd, word+1, x, y, i); /* by incrementing the word pointer, word[i] is shifted past the U */
    
    /* try the eight tiles around the current one, and recurse if the word matches */
    #define do_try_place(xx, yy) ret = try_place(brd, word, x, y, i, xx, yy); \
                                 if (ret != 0) return ret;

    do_try_place(-1, 0);
    do_try_place(0, -1);
    do_try_place(1, 0);
    do_try_place(0, 1);
    do_try_place(-1, -1);
    do_try_place(-1, 1);
    do_try_place(1, -1);
    do_try_place(1, 1);

    /* still here? none of the places worked */
    return 0;
}

/* Given a board and a word, determines whether the word is on the board,
 * and if so generates an ordered list of ordered pairs (origin is top left)
 * describing where it is on the board. The word is assumed to be in all lower case,
 * and the x/y arrays are assumed to have enough space. */
int find_word(struct Board *brd, char *word, int *x, int *y)
{
    int i, ret;

    /* find the first character */
    for (i = 0; i < brd->size * brd->size; ++i)
    {
        if (BOARD_INDEX(brd, i) == word[0])
        {
            x[0] = i % brd->size;
            y[0] = i / brd->size;
            ret = subfind(brd, word, x, y, 1);
            if (ret != 0) return ret;
        }
    }
    if (i == brd->size * brd->size) return 0; // if we got to the end, not found...
}

#ifndef BUILD
/* just for testing */
int main()
{
    struct Board brd;
    int i, j, x[16], y[16];
    char word[16];
    char spaces[10];

    srand(time(NULL));

    make_board(&brd, 5);
    
    for (i = 0; i < 10; ++i)
    {
        usleep(75000);
        shuffle(&brd);
        j = randint(0, 9);
        spaces[j] = '\0';
        for (--j; j >= 0; --j) spaces[j] = ' ';
        print_board(NULL, &brd, 0, spaces);
    }

    while (1)
    {
        printf("Word: ");
        scanf("%s", word);
        i = find_word(&brd, word, x, y);
        if (i == 0)
        {
            printf("Word not found!\n");
        }
        else
        {
            printf("%s is at ", word);
            for (j = 0; j < i; ++j)
            {
                printf("(%d, %d) ", x[j], y[j]);
            }
            printf("\n");
        }
    }

    free_board(&brd);

    /*for (i = 0; i < 26; ++i)
    {
        for (j = 0; j < 4; ++j)
        {
            printf("%s\n", letters[i][j]);
        }
        printf("\n");
    }*/

    return 0;
}
#endif // BUILD

