#ifndef BRAINSCREW_H
#define BRAINSCREW_H

// both return a heap allocated string
char *brainscrew_compile_c(const char *src);
char *brainscrew_compile_bf(const char *src);
char *brainscrew_compile_bsc(const char *src);

void brainscrew_interpret(const char *src);

#endif // BRAINSCREW_H
