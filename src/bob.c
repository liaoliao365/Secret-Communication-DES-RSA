#include <stdio.h>
#include "cripto.h"
#include "socket.h"
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <des.h>
#include "rsa.h"
#define DES_KEY_SIZE 8
// Declare file handlers
static FILE *key_file, *input_file, *output_file;
// Declare action parameters
#define ACTION_GENERATE_KEY "-g"
#define ACTION_ENCRYPT "-e"
#define ACTION_DECRYPT "-d"
#define ACTION_SECRET_DATA "-sec"
#define ACTION_GENERATE_RSA "-pub"
#define ACTION_SIGNATRUE "-sig"

print_chars(unsigned char* str, int len){
    for (int i = 0; i < len; i++) {
        printf("%02x", str[i]);
    }
    printf("\n");
}
int main(int argc, char* argv[]){
	clock_t start, finish;
	double time_taken;
	unsigned long file_size;
	unsigned short int padding;
    time_t t;

    srand((unsigned) time(&t));


	if (strcmp(argv[1], ACTION_GENERATE_KEY) == 0) { // Generate key file
        client();
        int p;
        printf("输入素数p: ");
        scanf("%d", &p);

        if(verificaPrimo(p) == 0){
            printf("\n不是素数!\n");
            return -1;
        }

        printf("\n生成alfa...\n");
        sleep(1);
        long long int alfa = rand() % (p-2) + 2;
        printf("alfa的值是: %lld\n", alfa);


        printf("\n生成 b...\n");
        sleep(1);
        long long int b = rand() % (p-2) + 2;
        printf("b 的值是: %lld\n", b);

        printf("\n计算 B...\n");
        sleep(1);
        int bezao = modulo(alfa,b,p);
        printf("B(%d) = alfa(%d)^b(%d) mod p(%d)\n", bezao,alfa,b,p);

        printf("\n#######\nB: %lld\nalfa: %d\nP: %lld\n",bezao,alfa,p);



        printf("\n----- 发送 (B,alfa,p) 给Alice -----\n");
        sleep(1.5);
        printf("\n----- 等待 Alice 回复 -----\n");

        sendServer(&bezao);
        sendServer(&alfa);
        sendServer(&p);

        int azao;

        readServer(&azao);

        printf("\n\n ----- 从 Alice 处收到的 A 的值 -----\n\n");
        printf("A: %d\n", azao);

        printf("\n\n计算 A^b...\n");
        sleep(1);
        long long int key = modulo(azao,b,p);
        printf("key(%d) = A(%d)^b(%d) mod p(%d)\n\n",key,azao,b,p);

        //将key转为char[]类型的keyArray
            // 确定 char[] 的大小
        int bufferSize = snprintf(NULL, 0, "%lld", key);
        printf("bufferSize: %lld\n", bufferSize);
            // 分配足够的空间
        unsigned char keyArray[bufferSize + 1];
            // 使用 snprintf 转换为 char[]
        snprintf(keyArray, sizeof(keyArray), "%lld", key);
            //打印结果
        printf("key: %lld\n", key);
        printf("keyArray: %s\n", keyArray);

        //生成keyHash
        unsigned char keyHash[EVP_MAX_MD_SIZE];// printf("EVP_MAX_MD_SIZE=%d\n",EVP_MAX_MD_SIZE);//EVP_MAX_MD_SIZE=64
        const char *data = "Hello, World!";
        int keyHash_len = generate_hash(keyArray,keyHash);
        printf("len of keyhash = %d\n",keyHash_len);
        printf("keyHash: ");
        print_chars(keyHash, keyHash_len);

        //生成des_key
        unsigned char desKey[DES_KEY_SIZE];
        for (int i = 0; i < DES_KEY_SIZE; ++i) {  // 64 位哈希值对应 DES_KEY_SIZE=8 个字节
            desKey[i] =  keyHash[i]%255;
        }
        printf("desKey: ");
        print_chars(desKey, DES_KEY_SIZE);

        //保存desKey
        FILE *key_file = fopen(argv[2], "wb");
        if (!key_file) {
            printf("Could not open file to write key.");
            return 1;
        }
        short int bytes_written = fwrite(desKey, 1, DES_KEY_SIZE, key_file);
        if (bytes_written != DES_KEY_SIZE) {
            printf("Error writing key to output file.");
            fclose(key_file);
            return 1;
        }
        fclose(key_file);
    }else if ((strcmp(argv[1], ACTION_ENCRYPT) == 0) || (strcmp(argv[1], ACTION_DECRYPT) == 0)) { // Encrypt or decrypt
		if (argc != 5) {
			printf("Invalid # of parameters (%d) specified. Usage: run_des [-e|-d] keyfile.key input.file output.file", argc);
			return 1;
		}

		// Read key file
		key_file = fopen(argv[2], "rb");
		if (!key_file) {
			printf("Could not open key file to read key.");
			return 1;
		}

		short int bytes_read;
		unsigned char* des_key = (unsigned char*) malloc(8*sizeof(char));
		bytes_read = fread(des_key, sizeof(unsigned char), DES_KEY_SIZE, key_file);
		if (bytes_read != DES_KEY_SIZE) {
			printf("Key read from key file does nto have valid key size.");
			fclose(key_file);
			return 1;
		}
		fclose(key_file);

		// Open input file
		input_file = fopen(argv[3], "rb");
		if (!input_file) {
			printf("Could not open input file to read data.");
			return 1;
		}

		// Open output file
		output_file = fopen(argv[4], "wb");
		if (!output_file) {
			printf("Could not open output file to write data.");
			return 1;
		}

		// Generate DES key set
		short int bytes_written, process_mode;
		unsigned long block_count = 0, number_of_blocks;
		unsigned char* data_block = (unsigned char*) malloc(8*sizeof(char));
		unsigned char* processed_block = (unsigned char*) malloc(8*sizeof(char));
		key_set* key_sets = (key_set*)malloc(17*sizeof(key_set));

		start = clock();
		generate_sub_keys(des_key, key_sets);
		finish = clock();
		time_taken = (double)(finish - start)/(double)CLOCKS_PER_SEC;

		// Determine process mode
		if (strcmp(argv[1], ACTION_ENCRYPT) == 0) {
			process_mode = ENCRYPTION_MODE;
			printf("Encrypting..\n");
		} else {
			process_mode = DECRYPTION_MODE;
			printf("Decrypting..\n");
		}

		// Get number of blocks in the file
		fseek(input_file, 0L, SEEK_END);
		file_size = ftell(input_file);

		fseek(input_file, 0L, SEEK_SET);
		number_of_blocks = file_size/8 + ((file_size%8)?1:0);

		start = clock();

		// Start reading input file, process and write to output file
		while(fread(data_block, 1, 8, input_file)) {
			block_count++;
			if (block_count == number_of_blocks) {
				if (process_mode == ENCRYPTION_MODE) {
					padding = 8 - file_size%8;
					if (padding < 8) { // Fill empty data block bytes with padding
						memset((data_block + 8 - padding), (unsigned char)padding, padding);
					}

					process_message(data_block, processed_block, key_sets, process_mode);
					bytes_written = fwrite(processed_block, 1, 8, output_file);

					if (padding == 8) { // Write an extra block for padding
						memset(data_block, (unsigned char)padding, 8);
						process_message(data_block, processed_block, key_sets, process_mode);
						bytes_written = fwrite(processed_block, 1, 8, output_file);
					}
				} else {
					process_message(data_block, processed_block, key_sets, process_mode);
					padding = processed_block[7];

					if (padding < 8) {
						bytes_written = fwrite(processed_block, 1, 8 - padding, output_file);
					}
				}
			} else {
				process_message(data_block, processed_block, key_sets, process_mode);
				bytes_written = fwrite(processed_block, 1, 8, output_file);
			}
			memset(data_block, 0, 8);
		}

		finish = clock();

		// Free up memory
		free(des_key);
		free(data_block);
		free(processed_block);
		fclose(input_file);
		fclose(output_file);

		// Provide feedback
		time_taken = (double)(finish - start)/(double)CLOCKS_PER_SEC;
		printf("Finished processing %s. Time taken: %lf seconds.\n", argv[3], time_taken);
		return 0;
	}else if((strcmp(argv[1], ACTION_SECRET_DATA) == 0)){//接收加密文件
        client();
        readServerFile(argv[2]);
    }else if((strcmp(argv[1], ACTION_GENERATE_RSA) == 0)){//接收公钥和文件签名
		printf("111\n");
        client();
		printf("111\n");
		//接收公钥
		struct public_key_class pub;
		unsigned long long int modulus;
		unsigned long long int exponent;
		readServerll(&modulus);
		readServerll(&exponent);
		printf("Public Key:\n Modulus: %lld\n Exponent: %lld\n", (unsigned long long)modulus, (unsigned long long) exponent);
		pub.modulus = modulus;
		pub.exponent = exponent;
		// read(sock , &pub, sizeof(struct public_key_class));
		// char buffer[1024];
		// ssize_t bytes_received = read(sock, buffer, 1024);
		// if (bytes_received == -1) {
		// 	error("Error receiving data");
		// }
		// 将接收到的字节流反序列化为结构体
		// memcpy(&pub, buffer, sizeof(struct public_key_class));
		// printf("Public Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)pub.modulus, (long long) pub.exponent);

		FILE *file = fopen(argv[2], "wb");
		if (file == NULL) {
			error("Error opening file");
		}
		if (fwrite(&pub, 1, sizeof(struct public_key_class), file) != sizeof(struct public_key_class)) {
            error("Error writing to file");
        }
    }else if((strcmp(argv[1], ACTION_SIGNATRUE) == 0)){//检验文件签名
        client();
		//计算文件哈希得到A
  		unsigned char digest[MD5_DIGEST_LENGTH];
    	generate_file_hash(argv[2], digest);
		digest[MD5_DIGEST_LENGTH]=0;
		printf("MD5 hash = ");
		print_chars(digest,MD5_DIGEST_LENGTH);

		char* result = stringToHex(digest);
		printf("result=%s\n",result);

		//读取公钥
		struct public_key_class pub[1];
		FILE *file = fopen(argv[3], "rb");
		if (file == NULL) {
			error("Error opening file for reading pub");
		}
		fread(pub, sizeof(struct public_key_class), 1, file);
		fclose(file);
		printf("Public Key:\n Modulus: %lld\n Exponent: %lld\n", (unsigned long long)pub->modulus, (unsigned long long) pub->exponent);

		//接受签名
		long long encrypted[strlen(result)];
		long long tmp;
		for(int i=0; i < strlen(result); i++){
			readServerull(&tmp);
			encrypted[i] = tmp;
			printf("%lld", (long long)encrypted[i]);
		}  
		printf("\n");
		//公钥解密签名得到B
		char *decrypted = rsa_decrypt(encrypted, 8*sizeof(result),pub);
		if (!decrypted){
			fprintf(stderr, "Error in decryption!\n");
			return 1;
		}

		printf("Decrypted:\n");
		for(int i=0; i < strlen(result); i++){
			printf("%lld\n", (long long)decrypted[i]);
		}  

		// printf("decrypted:\n");
		// printf("result=%s\n",decrypted);

		// printf("decrypted:\n");
		// print_chars(decrypted, strlen(result));
		// printf("digest:\n");
		// print_chars(digest, strlen(result));
		//比对A和B是否相等

    }else {
		printf("Invalid action: %s. First parameter must be [ -g | -e | -d | -sec | -sig ].", argv[1]);
		return 1;
	}
    return 0;
}