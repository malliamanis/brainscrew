#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "brainscrew.h"

typedef enum {
	TOKEN_INCREMENT,
	TOKEN_DECREMENT,
	TOKEN_LEFT,
	TOKEN_RIGHT,
	TOKEN_OUTPUT,
	TOKEN_INPUT,
	TOKEN_LOOP_START,
	TOKEN_LOOP_END,
	TOKEN_EOF
} TokenType;

static char *add_instruction(char *dest, size_t *dest_size, const char *src);

static TokenType *lex(const char *src);
static TokenType lex_match_token(char *lexeme);

char *brainscrew_compile_c(const char *src)
{
	TokenType *tokens = lex(src);

	size_t output_size = 512;
	char *output = calloc(output_size, sizeof(char));

	output = add_instruction(output, &output_size, "#include<stdio.h>\n"
	                                               "#include<stdlib.h>\n"
	                                               "#include<stdint.h>\n"
	                                               "#include<string.h>\n"
	                                               ""
	                                               // macros to reduce file size
	                                               "#define I ++*p;\n"          // increment
	                                               "#define D --*p;\n"          // decrement
	                                               "#define L --p;\n"           // left
	                                               "#define R d(&c,&p,&s);++p;\n" // right
	                                               "#define O putchar(*p);\n"   // output
	                                               "#define G *p=getchar();\n"  // get
	                                               "#define S while(*p){\n"     // start (loop)
	                                               "#define E }\n"              // end (loop)
	                                               ""
	                                               "void d(uint8_t**c,uint8_t**p,size_t*s)" // double cells size if needed
	                                               "{"
	                                                "if(*p-*c==*s-1){"
	                                                 "*p-=(uintptr_t)*c;"
	                                                 "*c=realloc(*c,*s*2);"
	                                                 "memset(*c+*s,0,*s-1);"
	                                                 "*s*=2;"
	                                                 "*p+=(uintptr_t)*c;"
	                                                "}"
	                                                "++p;"
	                                               "}"
	                                               ""
	                                               "int main(void)"
	                                               "{"
	                                                "size_t s=256;"                              // cells size
	                                                "uint8_t*c=calloc(s,sizeof(unsigned char));" // cells
	                                                "uint8_t*p=c;");                             // data pointer

	for (uint32_t i = 0; tokens[i] != TOKEN_EOF; ++i) {
		switch (tokens[i]) {
			case TOKEN_INCREMENT:
				output = add_instruction(output, &output_size, "I ");
				break;
			case TOKEN_DECREMENT:
				output = add_instruction(output, &output_size, "D ");
				break;
			case TOKEN_LEFT:
				output = add_instruction(output, &output_size, "L ");
				break;
			case TOKEN_RIGHT:
				output = add_instruction(output, &output_size, "R ");
				break;
			case TOKEN_OUTPUT:
				output = add_instruction(output, &output_size, "O ");
				break;
			case TOKEN_INPUT:
				output = add_instruction(output, &output_size, "G ");
				break;
			case TOKEN_LOOP_START:
				output = add_instruction(output, &output_size, "S ");
				break;
			case TOKEN_LOOP_END:
				output = add_instruction(output, &output_size, "E ");
				break;
			default:
				break;
		}
	}
	
	output = add_instruction(output, &output_size,  "free(c);"
	                                               "}\n");

	free(tokens);

	return output;
}

char *brainscrew_compile_bf(const char *src)
{
	TokenType *tokens = lex(src);

	size_t output_size = 512;
	char *output = calloc(output_size, sizeof(char));

	for (uint32_t i = 0; tokens[i] != TOKEN_EOF; ++i) {
		switch (tokens[i]) {
			case TOKEN_INCREMENT:
				output = add_instruction(output, &output_size, "+");
				break;
			case TOKEN_DECREMENT:
				output = add_instruction(output, &output_size, "-");
				break;
			case TOKEN_LEFT:
				output = add_instruction(output, &output_size, "<");
				break;
			case TOKEN_RIGHT:
				output = add_instruction(output, &output_size, ">");
				break;
			case TOKEN_OUTPUT:
				output = add_instruction(output, &output_size, ".");
				break;
			case TOKEN_INPUT:
				output = add_instruction(output, &output_size, ",");
				break;
			case TOKEN_LOOP_START:
				output = add_instruction(output, &output_size, "[");
				break;
			case TOKEN_LOOP_END:
				output = add_instruction(output, &output_size, "]");
				break;
			default:
				break;
		}
	}

	output = add_instruction(output, &output_size, "\n");

	free(tokens);
	
	return output;
}

char *brainscrew_compile_bsc(const char *src)
{
	size_t output_size = 512;
	char *output = calloc(output_size, sizeof(char));

	for (uint32_t i = 0; src[i] != 0; ++i) {
		switch (src[i]) {
			case '+':
				output = add_instruction(output, &output_size, "increment ");
				break;
			case '-':
				output = add_instruction(output, &output_size, "decrement ");
				break;
			case '<':
				output = add_instruction(output, &output_size, "left ");
				break;
			case '>':
				output = add_instruction(output, &output_size, "right ");
				break;
			case '.':
				output = add_instruction(output, &output_size, "output ");
				break;
			case ',':
				output = add_instruction(output, &output_size, "input ");
				break;
			case '[':
				output = add_instruction(output, &output_size, "startloop ");
				break;
			case ']':
				output = add_instruction(output, &output_size, "endloop ");
				break;
			default:
				break;
		}
	}

	output = add_instruction(output, &output_size, "\n");
	
	return output;
}

static char *add_instruction(char *dest, size_t *dest_size, const char *src)
{
	if (strlen(dest) + 1 + strlen(src) >= *dest_size) {
		size_t src_length = strlen(src);
		dest = realloc(dest, *dest_size * 2 + src_length + 1);
		memset(dest + *dest_size, 0, *dest_size + src_length + 1);
		*dest_size = *dest_size * 2 + src_length + 1;
	}

	strcat(dest, src);

	return dest;
}

void brainscrew_interpret(const char *src)
{
	TokenType *tokens = lex(src);

	size_t cells_size = 256;
	uint8_t *cells = calloc(cells_size, sizeof(uint8_t));
	uint8_t *ptr = cells;

	uint32_t loop_depth = 0;

	TokenType current;

	for (int32_t i = 0; (current = tokens[i++]) != TOKEN_EOF;) {
		switch (current) {
			case TOKEN_INCREMENT:
				++(*ptr);
				break;
			case TOKEN_DECREMENT:
				--(*ptr);
				break;
			case TOKEN_LEFT:
				if (ptr - cells == 0) {
					fputs("error: data pointer out of range\n", stderr);
					exit(EXIT_FAILURE);
				}

				--ptr;
				break;
			case TOKEN_RIGHT:
				if (ptr - cells == cells_size - 1) {
					ptr -= (uintptr_t)cells;

					cells = realloc(cells, cells_size * 2);
					memset(cells + cells_size, 0, cells_size - 1);
					cells_size *= 2;

					ptr += (uintptr_t)cells;
				}

				++ptr;
				break;
			case TOKEN_OUTPUT:
				putchar(*ptr);
				break;
			case TOKEN_INPUT:
				*ptr = getchar();
				break;
			case TOKEN_LOOP_START:
				if (*ptr != 0)
					continue;
				
				while ((current = tokens[i++]) != TOKEN_EOF) {
					if (current == TOKEN_LOOP_START) 
						++loop_depth;
					else if (current == TOKEN_LOOP_END) {
						if (loop_depth == 0) 
							break;
						else
							--loop_depth;
					}
				}

				break;
			case TOKEN_LOOP_END:
				if (*ptr == 0)
					continue;
				
				while (1) {
					i -= 2;
					if (i < 0) {
						fputs("error: unbalanced brackets\n", stderr);
						exit(EXIT_FAILURE);
					}
					current = tokens[i++];
					
					if (current == TOKEN_LOOP_END) 
						++loop_depth;
					else if (current == TOKEN_LOOP_START) {
						if (loop_depth == 0) 
							break;
						else 
							--loop_depth;
					}
				}

				break;
			default:
				break;
		}
	}

	free(cells);	
	free(tokens);
}

static TokenType *lex(const char *src)
{
	TokenType *tokens = malloc(64 * sizeof(TokenType));
	size_t tokens_size = 64;

	uint32_t src_index = 0;
	char lexeme[128] = {0};

	for (uint32_t tokens_index = 0;; ++tokens_index) {
		char c = 0;
		memset(lexeme, 0, 128);

		while (isspace(c = src[src_index]))
			++src_index;

		for (uint32_t lexeme_index = 0; !isspace(c = src[src_index]); ++lexeme_index) {
			if (c == 0)
				break;

			if (isalpha(c)) {
				lexeme[lexeme_index] = c;
				++src_index;

				continue;
			}
			else if (c == '/') {
				c = src[++src_index];

				if (c == '/') {
					while ((c = src[++src_index])) {
						if (c == '\n')
							break;
						else if (c == 0) {
							fprintf(stderr, "error: unclosed comment\n");
							exit(EXIT_FAILURE);
						}
					}
				}
				else if (c == '*') {
					while ((c = src[++src_index])) {
						if (c == '*') {
							c = src[++src_index];

							if (c == '/')
								break;
							else if (c == 0) {
								fprintf(stderr, "error: unclosed comment\n");
								exit(EXIT_FAILURE);
							}
						}
						else if (c == 0) {
							fprintf(stderr, "error: unclosed comment\n");
						}
					}
				}
				else if (c == 0) {
					fprintf(stderr, "error: unknown lexeme \'/'\n");
					exit(EXIT_FAILURE);
				}
			}
			else {
				fprintf(stderr, "error: unknown lexeme \'%c\'\n", c);
				exit(EXIT_FAILURE);
			}

			++src_index;
		}

		if (c == 0) {
			tokens[tokens_index] = TOKEN_EOF;
			return tokens;
		}
		else {
			if (tokens_index == tokens_size)
				tokens = realloc(tokens, (tokens_size *= 2) * sizeof(TokenType));

			tokens[tokens_index] = lex_match_token(lexeme);
		}
	}
}

static TokenType lex_match_token(char *lexeme)
{
	if (strcmp(lexeme, "increment") == 0)
		return TOKEN_INCREMENT;
	else if (strcmp(lexeme, "decrement") == 0)
		return TOKEN_DECREMENT;
	else if (strcmp(lexeme, "left") == 0)
		return TOKEN_LEFT;
	else if (strcmp(lexeme, "right") == 0)
		return TOKEN_RIGHT;
	else if (strcmp(lexeme, "output") == 0)
		return TOKEN_OUTPUT;
	else if (strcmp(lexeme, "input") == 0)
		return TOKEN_INPUT;
	else if (strcmp(lexeme, "startloop") == 0)
		return TOKEN_LOOP_START;
	else if (strcmp(lexeme, "endloop") == 0)
		return TOKEN_LOOP_END;
	else {
		fprintf(stderr, "error: unknown lexeme \'%s\'\n", lexeme);
		exit(EXIT_FAILURE);
	}

	return 0;
}
