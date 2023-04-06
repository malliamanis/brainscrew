#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "brainscrew.h"

// TODO: add assembly support

static char *read_file(const char *name);
static void write_file(const char *name, const char *src);

static void print_help(void);

int main(int argc, char **argv)
{
	if (argc < 2) {
		print_help();
		return 1;
	}

	bool compile_c = false;
	bool compile_bf = false;
	bool compile_bsc = false;

	uint32_t i;
	for (i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		if (arg[0] == '-') {
			if (arg[1] == 'h') {
				print_help();
				return 1;
			}
			else if (arg[1] == 'c') {
				compile_c = true;
			}
			else if (arg[1] == 'b' && arg[2] == 'f') {
				compile_bf = true;
			}
			else if (arg[1] == 'b' && arg[2] == 's' && arg[3] == 'c') {
				compile_bsc = true;
			}
			else {
				fprintf(stderr, "error: invalid option \'-%s\'", arg + 1);
				exit(EXIT_FAILURE);
			}
		}
		else
			break;
	}

	if (compile_c + compile_bf + compile_bsc > 1) {
		fprintf(stderr, "error: can't have more than one compile options at once\n");
		return 1;
	}
	else if (i == argc) {
		fprintf(stderr, "error: no input files\n");
		return 1;
	}

	for (; i < argc; ++i) {
		char *src = read_file(argv[i]);

		if (compile_c || compile_bf || compile_bsc) {
			const char *file_name = argv[i];
			uint32_t file_name_len = strlen(file_name);

			int32_t extension_index;
			for (extension_index = file_name_len - 1; extension_index != -1 && file_name[extension_index] != '.'; --extension_index);

			int32_t path_index;
			for (path_index = file_name_len - 1; path_index != -1 && file_name[path_index] != '/'; --path_index);
			++path_index;

			char *compiled = NULL;
			uint32_t out_name_offset; // for the sprintf offset
			const char *extension_str = NULL;

			if (compile_c) {
				compiled = brainscrew_compile_c(src);
				extension_str = ".c";
			}
			else if (compile_bf) {
				compiled = brainscrew_compile_bf(src);
				extension_str = ".bf";
			}
			else if (compile_bsc) {
				compiled = brainscrew_compile_bsc(src);
				extension_str = ".bsc";
			}

			if (extension_index == -1)
				out_name_offset = file_name_len - 1;
			else
				out_name_offset = extension_index - path_index;

			char *out_name = malloc(file_name_len - path_index + strlen(extension_str) + 1);
			strcpy(out_name, file_name + path_index);
			sprintf(out_name + out_name_offset, "%s", extension_str);

			write_file(out_name, compiled);
			free(out_name);
			free(compiled);
		}
		else
			brainscrew_interpret(src);	
		
		free(src);
	}

	return 0;
}

static char *read_file(const char *name)
{
	FILE *file = fopen(name, "r");
	if (file == NULL) {
		fprintf(stderr, "error: %s: cannot open file\n", name);
		exit(EXIT_FAILURE);
	}

	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *buffer = malloc(file_size + 1);
	fread(buffer, sizeof(char), file_size, file);

	fclose(file);

	return buffer;
}

static void write_file(const char *name, const char *src)
{
	FILE *file = fopen(name, "w+");
	fwrite(src, sizeof(char), strlen(src), file);
	fclose(file);
}

static void print_help(void)
{
	printf("Usage: brainscrew [options] files...\n"
	       "Options:\n"
	       "  -h   help\n"
	       "  -c   compile to C\n"
		   "  -bf  compile to Brainfuck\n"
		   "  -bsc compile Brainfuck to Brainscrew\n");
}
