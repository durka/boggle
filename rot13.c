#ifndef BUILD
#   include <stdio.h>
#endif

#include <ctype.h>

#include "rot13.h"

char g_short[] = "Gung'f jung FUR fnvq!"; /* too short */
char g_notword[] = "Unir lbh orra qevaxvat?"; /* not a word */
char g_notthere[] = "Lbhe fcngvny pbbeqvangvba vf JNL bss."; /* not on the board */
char g_dupe[] = "Univat n ovg bs n fravbe zbzrag, ru?"; /* duplicate */

void rot13(char *text)
{
    for (; *text; ++text)
    {
        if (isalpha(*text))
        {
            if (*text > (*text >= 'a' ? 'z' : 'Z') - 13)
            {
                *text -= 13;
            }
            else
            {
                *text += 13;
            }
        }
    }
}

#ifndef BUILD
int main()
{
    rot13(gshort);
    printf("%s\n", gshort);

    return 0;
}
#endif

