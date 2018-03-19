#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>

#include "parse.h"
#include "board.h"
#include "dict.h"
#include "rot13.h"
#include "game.h"

void setup_windows(WINDOW **titlebar, WINDOW **board, WINDOW **words)
{
    *titlebar = newwin(1, COLS, 0, 0); /* titlebar spans top line */
    
    *board = newwin(LINES - 3, COLS / 2, 2, 0); /* board goes in left half of the screen */

    *words = newwin(LINES - 3, COLS / 2, 2, COLS / 2); /* word list goes in right half */
}

void store_word(FILE *file, char *word)
{
    fprintf(file, "%s\n", word);
    fflush(file);
}

void print_instructions(struct Options *config, WINDOW *titlewin, WINDOW *boardwin, WINDOW *wordwin)
{
    mvwaddstr(titlewin, 0, 0, "INSTRUCTIONS");
    wrefresh(titlewin);
    mvwprintw(boardwin, 5, 0, "The Boggle board will appear below. Form words using adjacent tiles (diagonal is fine) -- just don't use any tile more than once. The words must be at least %d characters long (note that Qu is one tile but counts as two letters). Score is based on length and quantity of words, so find as many as you can before the time is up!\n\nIf you need to stop early, press S to end the game and report score, or Q to quit entirely.", config->minlength);
    wrefresh(boardwin);
    mvwprintw(wordwin, 15, 0, "The words you type will appear on this side of the screen in bold as you type them. Press enter when you've finished a word, and if it is indeed a word, is on the board, and is at least %d letters long, it will stay (otherwise it will disappear and you'll get an admonishment up at the top of the screen. Be careful -- every time you type a duplicate or invalid word, you lose 1/5 of a point! You have %d minutes and %d seconds to find words, so hurry up! The time starts when you press any key.", config->minlength, config->time / 60, config->time % 60);
    wrefresh(wordwin);
}

void shuffle_anim(WINDOW *boardwin, struct Board *board, int *brdx, int *brdy, char *spaces)
{
    int i, x, y;

    for (i = 0; i < 7; ++i)
    {
        usleep(75000);
        wclear(boardwin);
        x = randint(0, 9);
        y = randint(0, 9);
        wmove(boardwin, y, 0);
        spaces[x] = '\0';
        for (--x; x >= 0; --x) spaces[x] = ' ';
        shuffle(board);
        getyx(boardwin, *brdy, *brdx);
        print_board(boardwin, board, 1, spaces);
        wrefresh(boardwin);
    }
}

int judge_word(struct Options *config, struct Board *board, char *word, int *x, int *y, int *score, WINDOW *scold, FILE *log)
{
    int len, ret = 0;

    if ((len = strlen(word)) >= config->minlength)
    {
        if (check_word(word, config->dict))
        {
            if (!check_word_fd(word, log))
            {
                if (ret = find_word(board, word, x, y))
                {
                    switch (len - config->minlength)
                    {
                        case 0:
                        case 1:
                            *score += 10;
                            break;
                        case 2:
                            *score += 20;
                            break;
                        case 3:
                            *score += 30;
                            break;
                        case 4:
                            *score += 50;
                            break;
                        default:
                            *score += 110;
                            break;
                    }
                }
                else
                {
                    wclear(scold);
                    if (config->sfw)
                        mvwprintw(scold, 0, 0, "That's not on the board!");
                    else
                        mvwprintw(scold, 0, 0, "That's not on the board! (%s)", g_notthere);
                    wrefresh(scold);
                    *score -= 2;
                }
            }
            else
            {
                wclear(scold);
                if (config->sfw)
                    mvwprintw(scold, 0, 0, "You did that one already!");
                else
                    mvwprintw(scold, 0, 0, "You did that one already! (%s)", g_dupe);
                wrefresh(scold);
                *score -= 2;
            }
        }
        else
        {
            wclear(scold);
            if (config->sfw)
                mvwprintw(scold, 0, 0, "That's not a word!");
            else
                mvwprintw(scold, 0, 0, "That's not a word! (%s)", g_notword);
            wrefresh(scold);
            *score -= 2;
        }
    }
    else
    {
        wclear(scold);
        if (config->sfw)
            mvwprintw(scold, 0, 0, "Too short!");
        else
            mvwprintw(scold, 0, 0, "Too short! (%s)", g_short);
        wrefresh(scold);
        *score -= 2;
    }

    return ret;
}

int main()
{
    struct Options config;
    struct Board board;
    WINDOW *titlewin, *boardwin, *wordwin;
    FILE *log = NULL;
    int ret, c, i, j, k, quit = 0, x[80], y[80], brdx, brdy, timeleft, words, score; /* actually 10 times the score (fixed point for tenths) */
    char word[80], spaces[10];
    enum State state = BEGIN; /* game state */

    srand(time(NULL));
    log = tmpfile();

    if ((ret = parse(".bogglerc", &config)) != 0)
    {
        fprintf(stderr, "Error reading configuration file .bogglerc! Errno = %d\n", ret);
        return 1;
    }

    if (!config.sfw)
    {
        rot13(g_short);
        rot13(g_notword);
        rot13(g_notthere);
        rot13(g_dupe);
    }

    make_board(&board, config.size, config.pbc);
    
    initscr(); /* start ncurses mode */
    halfdelay(5); /* disable line buffering, but getch returns ERR after 1 second */
    noecho(); /* characters typed on the keyboard will not show up on the screen */
    keypad(stdscr, TRUE); /* enable arrow keys */

    /* game loop */
    while (!quit)
    {
        switch (state)
        {
            case BEGIN:
                refresh();
                setup_windows(&titlewin, &boardwin, &wordwin);
                print_instructions(&config, titlewin, boardwin, wordwin);
                if (wgetch(wordwin) != ERR)
                    state = START;
                break;
            case START:
                if (log) fclose(log); /* temporary file is deleted on fclose */
                log = tmpfile();
                wrefresh(titlewin);
                wclear(boardwin);
                wrefresh(boardwin);
                wclear(wordwin);
                wrefresh(wordwin);
                shuffle_anim(boardwin, &board, &brdx, &brdy, spaces);
                state = PLAY;
                i = 0;
                timeleft = config.time * 2;
                score = words = 0;
                break;
            case PLAY:
                wmove(boardwin, brdy, brdx);
                print_board(boardwin, &board, 1, spaces);
                wrefresh(boardwin);
                c = wgetch(wordwin);
                switch (c)
                {
                    case ERR: /* meaning we timed out after half a second waiting for word input */
                        for (k = 0; k < board.size * board.size; ++k) board.highlighted[k] = 0; /* clear out the highlighted squares */
                        wclear(titlewin); /* clear out the title bar */
                        --timeleft;
                        wattron(titlewin, A_BOLD);
                        mvwprintw(titlewin, 0, COLS - 55, "%2d words    Score: %3.1f    Time remaining: %2d:%02d", words, score / 10., timeleft / 120, (timeleft/2) % 60); /* print the time remaining */
                        wattroff(titlewin, A_BOLD);
                        wrefresh(titlewin);
                        if (!timeleft) state = RECAP; /* if the time is up, stop the game */
                        break;
                    case 'Q': /* quit */
                        quit = 1;
                        break;
                    case 'S': /* new game */
                        state = RECAP;
                        break;
                    case 'P': /* pause */
                        wclear(boardwin);
                        mvwprintw(boardwin, 0, 0, "PAUSED - Press any key to continue.");
                        wrefresh(boardwin);
                        while (wgetch(boardwin) == ERR); /* wait for anything except a timeout */
                        wclear(boardwin);
                        break;
                    case '\n':
                        word[i] = '\0';
                        for (j = 0; j < i; ++j) waddstr(wordwin, "\b \b");
                        ret = judge_word(&config, &board, word, x, y, &score, titlewin, log);
                        if (ret > 0)
                        {
                            ++words;
                            store_word(log, word);
                            wprintw(wordwin, "%s\n", word);
                            for (k = 0; k < ret; ++k) board.highlighted[x[k]*board.size + y[k]] = 1;
                            wrefresh(wordwin);
                        }
                        i = 0;
                        break;
                    case KEY_BACKSPACE:
                    case 127: // wtf apple
                        if (i != 0) --i;
                        waddstr(wordwin, "\b \b");
                        break;
                    default:
                        if (c >= 'a' && c <= 'z')
                        {
                            word[i++] = c;
                            wechochar(wordwin, c | A_BOLD);
                        }
                        break;
                }
                break;
            case RECAP:
                mvwprintw(titlewin, 0, 0, "GAME OVER");
                wrefresh(titlewin);
                wprintw(wordwin, "\n\nYou got %.1f points with %d words!\n\nTo quit, press Q. To start a new game, press any key.\n", score / 10., words);
                wrefresh(wordwin);
                state = WAIT;
                break;
            case WAIT:
                switch (wgetch(wordwin))
                {
                    case ERR:
                        break;
                    case 'Q':
                        quit = 1;
                        break;
                    default:
                        state = START;
                        break;
                }
                break;
        }
    }
    
    endwin(); /* end ncurses mode */
    fclose(log);

    return 0;
}

