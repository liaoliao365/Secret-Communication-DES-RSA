#ifndef CRIPTO_H_
#define CRIPTO_H_

#include <stdio.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

int verificaPrimo(long long int n);
long long int inverse(int a, int c);
int modulo(int a,int b,int n);
int generate_hash(const char *input, unsigned char *hash);
void generate_file_hash(const char *filename, unsigned char *digest);
char* stringToHex(char* input);
#endif