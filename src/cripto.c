#include "cripto.h"
#include <openssl/md5.h>
#include <openssl/evp.h>
int verificaPrimo(long long int n){
    int isCousin;

    if (n <= 1 || (n != 2 && n % 2 == 0))
        isCousin = FALSE;
    else
        isCousin = TRUE;

    int d = 3;
    while (isCousin  && d <= n / 2) {
        if (n % d == 0)
            isCousin = FALSE;
        d = d + 2;
    }

    return isCousin;
}

long long int inverse(int a, int c){
    long long int inverse;
    for(inverse = 0; inverse < c-1;inverse++){
        if((a * inverse) % c == 1)
            return inverse;
    }

    return -1;
}

int modulo(int a,int b,int n){
    long long x=1,y=a;
    while(b > 0){
        if(b%2 == 1){
            x=(x*y)%n;
        }
        y = (y*y)%n;
        b /= 2;
    }
    return x%n;
}

// 生成哈希
int generate_hash(const char *input, unsigned char *hash) {
    EVP_MD_CTX mdctx;
	const EVP_MD *md = EVP_md5();
	unsigned int md_len;
    // 初始化 OpenSSL 哈希上下文
    EVP_MD_CTX_init(&mdctx);

    // 初始化哈希上下文
    EVP_DigestInit_ex(&mdctx, md, NULL);

    // 更新哈希上下文
    EVP_DigestUpdate(&mdctx, input, strlen(input));

    // 获取哈希值
    EVP_DigestFinal_ex(&mdctx, hash, &md_len);

    // 释放哈希上下文
    EVP_MD_CTX_cleanup(&mdctx);
	return md_len;
}

void generate_file_hash(const char *filename, unsigned char *digest) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    MD5_CTX md5Context;
    MD5_Init(&md5Context);

    unsigned char buffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        MD5_Update(&md5Context, buffer, bytesRead);
    }

    MD5_Final(digest, &md5Context);

    fclose(file);
}

char* charToHex(char c) {
    static const char hexChars[] = "0123456789ABCDEF";
    char* result = (char*)malloc(3);  // 两个字符 + 结尾的 '\0'
    if (result == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    result[0] = hexChars[(c >> 4) & 0xF];
    result[1] = hexChars[c & 0xF];
    result[2] = '\0';
    return result;
}

char* stringToHex(char* input) {
    size_t len = strlen(input);
    char* result = (char*)malloc(len * 2 + 1);  // 每个字符转换为两个十六进制字符 + 结尾的 '\0'
    if (result == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < len; i++) {
        char* hexRepresentation = charToHex(input[i]);
        strcat(result, hexRepresentation);
        free(hexRepresentation);
    }
    result[len * 2]=0;
    return result;
}

