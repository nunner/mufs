#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "mufs.h"

#define FMTSIZE 50

static char *names[] = {
	"Artist",
	"Album",
	"Title",
};

void
parse_format (struct mufs_opts *opts)
{
	opts->format = calloc(1, sizeof(struct mufs_format));
	char *conf = opts->format_str;
	int *specifiers = &opts->format[0].specifiers;
	int offset = 0, levels = 0;
	opts->format[levels].format = malloc(FMTSIZE);

	if(*opts->format_str == '/') opts->format_str = &opts->format_str[1];

	while(*conf != '\0') {
		opts->format[levels].format[offset] = *conf;

		if(*conf == '%') {
			++conf, ++offset;

			specifiers = &opts->format[levels].specifiers;
			opts->format[levels].names = realloc(opts->format[levels].names, 
					(*specifiers + 1) * sizeof(char *));

			switch(*conf) {
				case 'a':
					opts->format[levels].names[*specifiers] = names[0];
					break;
				case 'f':
					opts->format[levels].names[*specifiers] = names[1];
					break;
				case 't':
					opts->format[levels].names[*specifiers] = names[2];
					break;
			}

			opts->format[levels].format[offset] = 's';

			++*specifiers;
		} else if(*conf == '/') {
			opts->format[levels].format[offset] = '\0';
			offset = -1;
			++levels;
			opts->format = realloc(opts->format, (levels + 1) * sizeof(struct mufs_format));
			opts->format[levels].names 		= 0;
			opts->format[levels].specifiers = 0;
			opts->format[levels].format 	= malloc(FMTSIZE);
		}
		
		++conf, ++offset;
	}

	opts->levels = levels;
}
