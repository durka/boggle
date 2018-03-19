#ifndef _BOGGLE_GAME_H_
#define _BOGGLE_GAME_H_

enum State { BEGIN, START, PLAY, RECAP, WAIT };

void store_word(FILE *file, char *word); /* just use dict functions */

#endif // _BOGGLE_GAME_H_

