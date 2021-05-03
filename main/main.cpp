#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <cstring>
#include <cstddef>
#include <getopt.h>
#include <memory>
#include <stdexcept>

#include "ser.h"

std::string serFile  = "";
std::string outFile  = "";

static void show_usage();
void process_args(int argc, char** argv);
size_t getSizeToEnd(std::ifstream& inStream);

int main(int argc, char* argv[]) {
  std::ifstream inStream;
  std::ofstream outStream;
  size_t serSize         = 0;
  int imageWidth         = 0;
  int imageHeight        = 0;
  int imageDepth         = 0;
  int imageCount         = 0;
  uint64_t frameSize     = 0;
  uint64_t frameToGrab   = 0;
  uint64_t prevFrame     = 0;
  uint64_t frameOffset   = 1;


  process_args(argc, argv);
	if ("" == serFile) {
		std::cerr << "H-alfaSSed: missing input file operand\n";
		std::cerr << "Try 'H-alfaSSed --help' for more information.\n";
		exit(1);
	}

	if ("" == outFile) {
		std::cerr << "H-alfaSSed: missing output file operand\n";
		std::cerr << "Try 'H-alfaSSed --help' for more information.\n";
		exit(1);
	}

 	inStream.open( serFile, std::ios::in|std::ios::binary );
 	outStream.open( outFile, std::ios::out|std::ios::binary );

        ser_get_details(inStream, imageWidth, imageHeight, imageDepth, imageCount, serSize);

        // seek to beginning of file
	inStream.seekg(0);
	// read 178 byte header
	std::vector<char> buff(178);
	inStream.read(buff.data(), buff.size());
	// copy header to output stream since it won't be modified
        outStream.write(buff.data(), buff.size());

	// for each frame, debayer and correct channel data, then write frame to output stream
	for(int frameNumber=1;frameNumber<=imageCount;frameNumber++){
		unfux_frame(inStream, outStream, frameNumber, imageCount);
	}
	// similar to header, copy the tail to the output stream
	std::vector<char> tail(getSizeToEnd(inStream), 0);
	inStream.read(tail.data(), tail.size());
	outStream.write(tail.data(), tail.size());
	// close both file handles
	inStream.close();
	outStream.close();
}


static void show_usage() {
	std::cout <<
	        "Hydrogen-alfa Sony Sensor error deleter"
	        "Usage: H-alfaSSed [OPTION]... --infile [SERFILE] --outfile [OUTFILE]\n"
                "Correct SERFILE and generate OUTFILE\n\n"

		"-i, --infile <fname>	.ser file to correct\n"
		"-o, --outfile <fname>  .ser file to write\n" 
		"-h, --help		Show this message\n";
	exit(1);
}

void process_args(int argc, char** argv) {
	const char* const short_opts = "i:o:h";
	const option long_opts[] = {
		{"infile", required_argument, nullptr, 'i'},
		{"outfile", required_argument, nullptr, 'o'},
		{"help", no_argument, nullptr, 'h'},
		{nullptr, no_argument, nullptr, 0}
	};
	while (true) {
		const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

		if (-1 == opt)
			break;

		switch (opt) {
			case 'i':
				serFile = std::string(optarg);
				break;
			case 'o':
				outFile = std::string(optarg);
				break;

			case 'h': // -h or --help
			case '?': // User has a confused
			default:
				show_usage();
				break;
			}
	}
}

size_t getSizeToEnd(std::ifstream& inStream)
{
    auto currentPosition = inStream.tellg();
    inStream.seekg(0, inStream.end);
    auto length = inStream.tellg() - currentPosition;
    inStream.seekg(currentPosition, inStream.beg);
    return length;
}
