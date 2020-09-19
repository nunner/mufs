#include <stdbool.h>
#include <stdio.h>

#include "parser.h"

void
parse_format (char *conf)
{
    while(*conf != '\0') {
        if(*conf == '%') {
            conf++;
            switch(*conf) {
                case 'a':
                    break;
                default:
                    conf--;
            }
        }

        ++conf;
    }
}
