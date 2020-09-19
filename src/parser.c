#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "mufs.h"

static char *names[] = {
	"Artist",
	"Album",
	"Title",
};

void
parse_format (struct mufs_opts *opts)
{
	char *conf = opts->format_str;
	int levels = 0;

	while(*conf != '\0') {
		if(*conf == '%') {
			++conf;
			opts->format = realloc(opts->format, (levels + 1) * sizeof(struct mufs_format));

			switch(*conf) {
				case 'a':
					opts->format[levels].name = names[0];
					break;
				case 'f':
					opts->format[levels].name = names[1];
					break;
				case 't':
					opts->format[levels].name = names[2];
					break;
			}

			++levels;
		}

		++conf;
	}

	printf("\n");
}
