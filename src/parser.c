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
	"Genre",
	"Track",
};

void
parse_format (struct mufs_opts *opts)
{
	opts->format = calloc(1, sizeof(struct mufs_format));
	int *specifiers = &opts->format[0].specifiers;
	char *conf = opts->format_str;
	int offset = 0, levels = 0;
	opts->format[levels].select.format = malloc(FMTSIZE);

	if(*opts->format_str == '/') opts->format_str = &opts->format_str[1];

	while(*conf != '\0') {
		opts->format[levels].select.format[offset] = *conf;

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
				case 'g':
					opts->format[levels].names[*specifiers] = names[3];
					break;
				case 'n':
					opts->format[levels].names[*specifiers] = names[4];
					break;
			}

			opts->format[levels].select.format[offset] = 's';

			++*specifiers;
		} else if(*conf == '/') {
			opts->format[levels].select.format[offset] = '\0';
			offset = -1;
			++levels;
			opts->format = realloc(opts->format, (levels + 1) * sizeof(struct mufs_format));
			opts->format[levels].names 				= 0;
			opts->format[levels].specifiers 		= 0;
			opts->format[levels].select.format 		= malloc(FMTSIZE);
		}
		
		++conf, ++offset;
	}

	for(size_t i = 0; i <= levels; i++) {
		opts->format[i].select.specifiers = malloc(FMTSIZE);
		for(size_t j = 0, k = 0; j < opts->format[i].specifiers; j++) {
			k += snprintf(opts->format[i].select.specifiers + k, FMTSIZE - k, ", %s", opts->format[i].names[j]);	
		}
	}	

	opts->levels = levels;
}
