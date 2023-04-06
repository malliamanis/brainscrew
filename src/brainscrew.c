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
static void lex_match_token(TokenType *tokens, char *lexeme);

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

	add_instruction(output, &output_size, "\n");
	
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
		char current_char = 0;
		int32_t src_index = 0;
		uint32_t loop_depth = 0;

		size_t cells_size = 100;
		uint8_t *cells = calloc(cells_size, sizeof(uint8_t));
		uint8_t *ptr = cells;

		while ((current_char = src[src_index++]) != 0) {
			switch (current_char) {
				case '+':
					++(*ptr);
					break;
				case '-':
					--(*ptr);
					break;
				case '<':
					if (ptr - cells == 0) {
						fputs("error: data pointer out of range\n", stderr);
						exit(EXIT_FAILURE);
					}

					--ptr;
					break;
				case '>':
					if (ptr - cells == cells_size - 1) {
						ptr -= (uintptr_t)cells;

						cells = realloc(cells, cells_size * 2);
						memset(cells + cells_size, 0, cells_size - 1);
						cells_size *= 2;

						ptr += (uintptr_t)cells;
					}

					++ptr;
					break;
				case '.':
					putchar(*ptr);
					break;
				case ',':
					*ptr = getchar();
					break;
				case '[':
					if (*ptr != 0)
						continue;
					
					while ((current_char = src[src_index++]) != 0) {
						if (current_char == '[') 
							++loop_depth;
						else if (current_char == ']') {
							if (loop_depth == 0) 
								break;
							else
								--loop_depth;
						}
					}
					break;
				case ']':
					if (*ptr == 0)
						continue;
					
					while (1) {
						src_index -= 2;
						if (src_index < 0) {
							fputs("error: unbalanced brackets\n", stderr);
							exit(EXIT_FAILURE);
						}
						current_char = src[src_index++];
						
						if (current_char == ']') 
							++loop_depth;
						else if (current_char == '[') {
							if (loop_depth == 0) 
								break;
							else 
								--loop_depth;
						}
					}
					break;
				default:
					continue;
			}
		}

		free(cells);	
}

static TokenType *lex(const char *src)
{
	TokenType *tokens = malloc(64 * sizeof(TokenType));
	size_t tokens_size = 64;

	char lexeme[16] = {0};

	// while ()

	return tokens;
}
