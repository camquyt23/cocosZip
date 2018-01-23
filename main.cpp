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

char _xxteaKey[1024];
size_t _xxteaSignLen = 6;
size_t _xxteaKeyLen = 5;
int encrypt_type = 0;
std::string in_file;
std::string out_dir;

std::string basename(std::string filename) {
	return filename.substr(filename.find_last_of("/") + 1);
}

void print_help() {
	printf("cocosZip --encrypt xxtea --xxtea-key b5730 --xxtea-sign-size 6  --in filename --dir outdir");
}

void parse_arg(int argc, char** argv) {
	int c;
	while (1) {
		static struct option long_options[] = {
			{ "help", no_argument, 0, 'h' },
			{ "encrypt", optional_argument, 0, 'e' },
			{ "xxtea-key", optional_argument, 0, 'k' },
			{ "xxtea-sign-size", optional_argument, 0, 'z' },
			{ "file", required_argument, 0, 'f' },
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
		case 'e':
			if (strcmp(optarg, "xxtea") == 0) {
				encrypt_type = 1;
				_xxteaKeyLen = strlen(optarg);
			}
			break;
		case 'k':
			memset(_xxteaKey, 0, 1024);
			strcpy(_xxteaKey, optarg);
			break;
		case 'z':
			_xxteaSignLen = atoi(optarg);
			break;
		case 'd':
			out_dir = optarg;
			break;
		case 'f':
			in_file = optarg;
			break;
		default:
			abort();
		}
	}
}


int main(int argc, char **argv) {
	memset(_xxteaKey, 0, 1025);
	strcpy(_xxteaKey, "b5730");

	parse_arg(argc, argv);

	FileUtils *utils = FileUtils::getInstance();
	std::string zipFilePath = std::string(in_file);

	void *buffer = nullptr;
	ZipFile *zip = nullptr;
	Data zipFileData(utils->getDataFromFile(zipFilePath));
	Data zipData2;
	unsigned char* bytes = zipFileData.getBytes();
	ssize_t size = zipFileData.getSize();

	CCLOG("ORIGIN size %ld, off %lx \n", size, (size - _xxteaSignLen));

	if (encrypt_type == 1) { // decrypt XXTEA
		xxtea_long len = 0;
		buffer = xxtea_decrypt(bytes + _xxteaSignLen,
				(xxtea_long) size - (xxtea_long) _xxteaSignLen,
				(unsigned char*) _xxteaKey, (xxtea_long) _xxteaKeyLen, &len);
		CCLOG("XXTEA size %d \n", len);
		zipData2.copy((unsigned char*) buffer, len);
		zip = ZipFile::createWithBuffer(buffer, len);
	} else {
		if (size > 0) {
			zip = ZipFile::createWithBuffer(bytes, (unsigned long) size);
			zipData2.copy(bytes, size);
		}
	}

	if (zip) {
		CCLOG("lua_loadChunksFromZIP() - load zip file: %s%s \n",
				zipFilePath.c_str(), encrypt_type ? "*" : "");
		std::string f = out_dir + basename(in_file);
		utils->writeDataToFile(zipData2, "/home/tran/Desktop/framework.zip");
		delete zip;
	} else {
		CCLOG("lua_loadChunksFromZIP() - not found or invalid zip file: %s \n",
				zipFilePath.c_str());
	}

	if (buffer) {
		free(buffer);
	}

	return 0;
}

