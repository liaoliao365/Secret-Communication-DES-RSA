#include <stdio.h>
#include "cripto.h"
#include "socket.h"
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <des.h>
#include "rsa.h"
#include <openssl/rsa.h>
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

	if (strcmp(argv[1], ACTION_GENERATE_KEY) == 0) {  // Generate key file
        int p, alfa, bezao;
        server();
        printf("等待 Bob 的消息");
        readClient(&bezao);
        readClient(&alfa);
        readClient(&p);

        printf("\n\n生成 a...\n");
        sleep(1);
        long long int a = rand() % (p-2) + 2;
        printf("a 的值是: %lld\n", a);

        printf("\n\n----- 从 Bob 收到的 p, alfa, B-----\n");
        printf("\nP: %d\nalfa: %d\nB: %d", p,alfa,bezao);

        printf("\n\n计算 A.....");
        sleep(1);
        long long int azao = modulo(alfa,a,p);
        printf("\nA(%lld) = alfa(%d)^a(%d) mod p(%d)",azao,alfa,a,p);

        sendClient(&azao);

        printf("\n\n计算 B^a...\n");
        sleep(1);
        long long int key = modulo(bezao,a,p);
        printf("key(%d) = B(%d)^a(%d) mod p(%d)\n\n",key,bezao,a,p);

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
        key_file = fopen(argv[2], "wb");
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
	}else if((strcmp(argv[1], ACTION_SECRET_DATA) == 0)){//发送加密文件
        server();
        sendClientFile(argv[2]);
    }else if((strcmp(argv[1], ACTION_GENERATE_RSA) == 0)){//发送签名
		server();
		//生成并保存rsa公私钥对
		struct public_key_class pub;
  		struct private_key_class priv;
 		rsa_gen_keys(&pub, &priv, PRIME_SOURCE_FILE);
		printf("Private Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)priv.modulus, (long long) priv.exponent);
  		printf("Public Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)pub.modulus, (long long) pub.exponent);

		//存储公私钥
		FILE *privfile = fopen(argv[2], "wb");
		if (privfile == NULL) {
			error("Error opening privfile");
		}
		FILE *pubfile = fopen(argv[3], "wb");
		if (pubfile == NULL) {
			error("Error opening pubfile");
		}
		if (fwrite(&priv, 1, sizeof(struct private_key_class), privfile) != sizeof(struct private_key_class)) {
            error("Error writing to privfile");
        }
		if (fwrite(&pub, 1, sizeof(struct public_key_class), pubfile) != sizeof(struct public_key_class)) {
            error("Error writing to pubfile");
        }
		unsigned long long int modulus = pub.modulus;
		unsigned long long int exponent=pub.exponent;
		sendClientll(&modulus);
		sendClientll(&exponent);
		printf("111\n");
		//发送公钥给Bob
		// send(new_socket , &pub , sizeof(struct public_key_class) , 0 );
		// 将结构体数据序列化为字节流
		// char buffer[sizeof(struct public_key_class)+1];
		// memcpy(buffer, &pub, sizeof(struct public_key_class));

		// // 发送序列化后的数据
		// if (send(new_socket, (char*)&pub, sizeof(struct public_key_class), 0) == -1) {
		// 	error("Error sending data");
		// }
    }else if((strcmp(argv[1], ACTION_SIGNATRUE) == 0)){//发送文件签名
		server();
		//计算文件哈希
  		unsigned char digest[MD5_DIGEST_LENGTH+1];
    	generate_file_hash(argv[2], digest);
		digest[MD5_DIGEST_LENGTH]=0;
		printf("MD5 hash = ");
		print_chars(digest, MD5_DIGEST_LENGTH);

		char* result = stringToHex(digest);
		printf("result=%s\n",result);
		

		//读取私钥
		// struct public_key_class pub[1];
		struct private_key_class priv[1];
		FILE *file = fopen(argv[3], "rb");
		if (file == NULL) {
			error("Error opening file for reading priv");
		}
		fread(priv, sizeof(struct private_key_class), 1, file);
		fclose(file);
		printf("Private Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)priv->modulus, (long long) priv->exponent);

		// file = fopen("./AliceData/alice.pub", "rb");
		// if (file == NULL) {
		// 	error("Error opening file for reading pub");
		// }
		// fread(pub, sizeof(struct public_key_class), 1, file);
		// fclose(file);
		// printf("Public Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)pub->modulus, (long long) pub->exponent);

		//用私钥生成文件签名
		long long *encrypted = rsa_encrypt(result, sizeof(result), priv);
		if (!encrypted){
			fprintf(stderr, "Error in encryption!\n");
			return 1;
		}
		printf("Encrypted:\n");
		for(int i=0; i < strlen(result); i++){
			printf("%lld\n", (long long)encrypted[i]);
		}  
		printf("\n");
		//发送签名给Bob
		for(int i=0; i < strlen(result); i++){
			sendClientull(encrypted+i);
		}  
        
    }else {
		printf("Invalid action: %s. First parameter must be [ -g | -e | -d | -sec | -sig ].", argv[1]);
		return 1;
	}

    return 0;
}