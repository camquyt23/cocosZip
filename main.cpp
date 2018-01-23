/*
 * main.cpp
 *
 *  Created on: Jan 22, 2018
 *      Author: Tran Hoan
 */

#include "platform/CCFileUtils.h"
#include "base/ZipUtils.h"
#include "xxtea/xxtea.h"
#include <getopt.h>

using namespace cocos2d;

#define XXTEA_KEY_SIZE 1024
enum {
	UNPACK = 1,
	PACK = 2,
	DECRYPT_XXTEA = 3,
	ENCRYPT_XXTEA = 4
};
char _xxteaKey[XXTEA_KEY_SIZE];
char _xxteaHeader[XXTEA_KEY_SIZE];
size_t _xxteaSignLen = 6;
size_t _xxteaKeyLen = 5;
int command = 1;
std::string in_file;
std::string out_dir;

std::string basename(std::string filename) {
	return filename.substr(filename.find_last_of("/") + 1);
}

void print_help() {
	printf("cocosZip --command decrypt-xxtea --xxtea-key b5730 --xxtea-sign-size 6  --in filename --dir outdir");
}

void parse_arg(int argc, char** argv) {
	int c;
	while (1) {
		static struct option long_options[] = {
			{ "help", no_argument, 0, 'h' },
			{ "command", required_argument, 0, 'c' },
			{ "xxtea-key", required_argument, 0, 'k' },
			{ "xxtea-header", required_argument, 0, 'p' },
			{ "xxtea-sign-size", required_argument, 0, 'z' },
			{ "file", required_argument, 0, 'i' },
			{ "dir", required_argument, 0, 'd' },
			{ 0, 0, 0, 0 }
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;
		c = getopt_long(argc, argv, ":e:k:z:i:d:", long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;
		switch (c) {
		case 'h':
			print_help();
			return;
		case 'c':
			if (strcmp(optarg, "decrypt-xxtea") == 0) {
				command = DECRYPT_XXTEA;
			}else if (strcmp(optarg, "encrypt-xxtea") == 0) {
				command = ENCRYPT_XXTEA;
			}
			break;
		case 'k':
			memset(_xxteaKey, 0, XXTEA_KEY_SIZE);
			strcpy(_xxteaKey, optarg);
			_xxteaKeyLen = strlen(optarg);
			break;
		case 'p':
			memset(_xxteaHeader, 0, XXTEA_KEY_SIZE);
			strcpy(_xxteaHeader, optarg);
			break;
		case 'z':
			_xxteaSignLen = atoi(optarg);
			break;
		case 'd':
			out_dir = optarg;
			break;
		case 'i':
			in_file = optarg;
			break;
		case '?':
			break;
		default:
			CCLOG("wrong option %c \n", c);
			abort();
		}
	}
}


int main(int argc, char **argv) {
	memset(_xxteaKey, 0, XXTEA_KEY_SIZE);
	strcpy(_xxteaKey, "b5730");
	memset(_xxteaHeader, 0, XXTEA_KEY_SIZE);
	strcpy(_xxteaHeader, "pocket");

	parse_arg(argc, argv);

	CCLOG("Input : %s \n", in_file.c_str());
	CCLOG("Output : %s \n", out_dir.c_str());
	CCLOG("XXTEA : %s , %lu, %lu\n", _xxteaKey, _xxteaKeyLen, _xxteaSignLen);

	FileUtils *utils = FileUtils::getInstance();
	std::string zipFilePath = std::string(in_file);

	void *buffer = nullptr;
	ZipFile *zip = nullptr;
	Data zipFileData(utils->getDataFromFile(zipFilePath));
	Data zipData2;
	unsigned char* bytes = zipFileData.getBytes();
	ssize_t size = zipFileData.getSize();

	CCLOG("ORIGIN size %ld, off %lx \n", size, (size - _xxteaSignLen));

	switch(command) { // decrypt XXTEA
	case DECRYPT_XXTEA: {
		CCLOG("DECRYPT XXTEA \n");
		xxtea_long len = 0;
		buffer = xxtea_decrypt(bytes + _xxteaSignLen,
				(xxtea_long) size - (xxtea_long) _xxteaSignLen,
				(unsigned char*) _xxteaKey, (xxtea_long) _xxteaKeyLen, &len);
		zipData2.copy((unsigned char*) buffer, len);
		zip = ZipFile::createWithBuffer(buffer, len);
		break;
	}
	case ENCRYPT_XXTEA: {
		CCLOG("ENCRYPT XXTEA \n");
		xxtea_long len = 0;
		buffer = xxtea_encrypt(bytes, size, (unsigned char*) _xxteaKey, (xxtea_long) _xxteaKeyLen, &len);
		unsigned char* ebuf = (unsigned char*) malloc(6 + len);
		memcpy(ebuf, _xxteaHeader, 6);
		memcpy(ebuf + 6, buffer, len);
		zipData2.copy(ebuf, len + 6);
		break;
	}
	default:
		CCLOG("UNKNOWN COMMAND \n");
		if (size > 0) {
			zip = ZipFile::createWithBuffer(bytes, (unsigned long) size);
			zipData2.copy(bytes, size);
		}
		break;
	}

	if (zip) {
		CCLOG("cocosZip load zip file: %s%s \n",
				zipFilePath.c_str(), command ? "*" : "");
		std::string f = out_dir + "/" + basename(in_file);
		CCLOG("cocosZip write to %s \n", f.c_str());
		utils->writeDataToFile(zipData2, f);
		delete zip;
	} else {
		CCLOG("cocosZip not found or invalid zip file: %s \n",
				zipFilePath.c_str());
	}

	if (buffer) {
		free(buffer);
	}

	return 0;
}

