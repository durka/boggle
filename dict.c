#include <stdio.h>
#include <string.h>

#include "dict.h"

int check_word(char *word, char *dictpath)
{
    FILE *dict;

    dict = fopen(dictpath, "r");

    return check_word_fd(word, dict);
}

int check_word_fd(char *word, FILE *dict)
{
    char line[80];

    rewind(dict); /* file pointer to beginning */
    
    while (fgets(line, sizeof line, dict) != NULL)
    {
        line[strlen(line)-1] = '\0';
        if (strcmp(word, line) == 0) return 1; /* found the word! abort/success! */
    }

    return 0; /* still here? word not found */
}

#ifndef BUILD
int main()
{
    char word[32];

    while (1)
    {
        printf("Word: ");
        scanf("%s", word);
        printf("That is %sa word!\n", check_word(word, "words") ? "" : "not ");
    }

    return 0;
}
#endif // BUILD

