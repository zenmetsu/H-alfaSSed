#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <cstring>
#include <cstddef>
#include <vector>
#include <inttypes.h>
#include "ser.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;

int serHeaderSize = 178;

void ser_get_details(std::ifstream& f, int& width, int& height, int &depth, int &count, size_t &s) {
  f.seekg(0, std::ios::end);
  s = f.tellg();
  f.seekg(26);
  f.read((char*)&width,  sizeof(width));
  f.read((char*)&height, sizeof(height));
  f.read((char*)&depth,  sizeof(depth));
  f.read((char*)&count,  sizeof(count));
}

void ser_get_offset(uint64_t f, int w, int h, int d, uint64_t &o) {
  o = serHeaderSize + (f - 1)*(w * h * (d / 8));
}

void unfux_frame(std::ifstream& serFile, std::ofstream& outFile, uint64_t currentFrame, uint64_t frameCount){
	int iWidth,iHeight,iDepth = 0;
	uint64_t offset,count     = 0;
	size_t serLength;
	int maxBright             = 0;
	// index for debayered arrays
	uint64_t r0               = 0;
	uint64_t g0               = 0;
	uint64_t g1               = 0;
	uint64_t b0               = 0;
	uint64_t c0               = 0;
	// running sum of observed pixel values
	uint64_t Sr0              = 0;
	uint64_t Sg0              = 0;
	uint64_t Sg1              = 0;
	uint64_t Sb0              = 0;
	// observed mean for each debayered array
	float Mr0              = 0;
	float Mg0              = 0;
	float Mg1              = 0;
	float Mb0              = 0;
	// value of pixel from source
	uint16_t valueRead        = 0;
	// calculated correction values
	float blueCor             = 0;
	float redCor              = 0;
	
	// seek to end of file to get size
	serFile.seekg(0, std::ios::end);
	serLength = serFile.tellg();
	// seek to necessary part of .ser header to get image details
 	serFile.seekg(26);
	// record image width, height, and pixel depth
 	serFile.read((char*)&iWidth,  sizeof(iWidth));
 	serFile.read((char*)&iHeight, sizeof(iHeight));
	serFile.read((char*)&iDepth,  sizeof(iDepth));
	// each debayered array will contain 1/4 of the frame data
	count = iWidth * iHeight / 4; 

        // arrays to hold debayered image layers
	std::vector<uint16_t> arrayR0(count);  
	std::vector<uint16_t> arrayG0(count);
	std::vector<uint16_t> arrayG1(count);
	std::vector<uint16_t> arrayB0(count);

	// array to hold corrected data, all four channels merged back into one
	std::vector<uint16_t> arrayCorrected(count*4);

	std::cout << "Frame: " << currentFrame << "/" << frameCount << "\t";
	//calculate offset and seek
	ser_get_offset(currentFrame, iWidth, iHeight, iDepth, offset);
	serFile.seekg(offset);

        // if 8 bit depth, each pixel is one byte
	if (iDepth == 8){
		for(uint64_t i=0;i<iHeight;i++){
			for(uint64_t j=0;j<iWidth;j++){
				char buf[1];
				// read one byte
				serFile.read(buf, 1);
				// de-bayer frame using modulo function
				//       even odd
				// even   R0   G0
				// odd    G1   B0
				if(i %2 == 0) {
					if(j %2 == 0) {
						arrayR0.at(r0)=(uint16_t)buf[0]*256;
						Sr0 += (float)buf[0];
						r0++;
					} else {
						arrayG0.at(g0)=(uint16_t)buf[0]*256;
						Sg0 += (float)buf[0];
						g0++;
					}
				} else {
					if(j %2 == 0) {
						arrayG1.at(g1)=(uint16_t)buf[0]*256;
						Sg1 += (float)buf[0];
						g1++;
					} else {
						arrayB0.at(b0)=(uint16_t)buf[0]*256;
						Sb0 += (float)buf[0];
						b0++;
					}
				}
			}
		}
	// if 16 it depth, each pixel is two bytes
	} else {
		for(uint64_t i=0;i<iHeight;i++){
			for(uint64_t j=0;j<iWidth;j++){
				uint8_t buf[2];
				// read two bytes
				serFile.read((char*)buf, 2);
				// de-bayer frame using modulo function
				//       even odd
				// even   R0   G0
				// odd    G1   B0
				if(i %2 == 1) {
					if(j %2 == 0) {
					        valueRead = (((uint16_t)buf[1] << 8 ) | buf[0]);
						arrayR0.at(r0)=valueRead;
						Sr0 += valueRead;
						r0++;
					} else {
					        valueRead = (((uint16_t)buf[1] << 8 ) | buf[0]);
						arrayG0.at(g0)=valueRead;
						Sg0 += valueRead;
						g0++;
					}
				} else {
					if(j %2 == 1) {
					        valueRead = (((uint16_t)buf[1] << 8 ) | buf[0]);
						arrayG1.at(g1)=valueRead;
						Sg1 += valueRead;
						g1++;
					} else {
					        valueRead = (((uint16_t)buf[1] << 8 ) | buf[0]);
						arrayB0.at(b0)=valueRead;
						Sb0 += valueRead;
						b0++;
					}
				}
			}
		}
	}

	// calculate observed mean for each channel
	Mr0 = (float)(Sr0 / r0);
	Mg0 = (float)(Sg0 / g0);
	Mg1 = (float)(Sg1 / g1);
	Mb0 = (float)(Sb0 / b0);
	printf("mean: [r,g,g,b] %05ld,%05ld,%05ld,%05ld ", Sr0/count,Sg0/count,Sg1/count,Sb0/count);

        // determine simple correction values
	//   red channel will always be over-exposed
	//   blue channel will always be under-exposed
	//   both green channels will be nearly equal
	//
	//   take the mean of both green channels
	//   and apply normalization correction so red and blue
	//   channels have a similar mean.
	redCor = (float)((Mg0 + Mg1)/2) / (float)Mr0;
	blueCor = (float)((Mg0 + Mg1)/2) / (float)Mb0;
	std::cout << "Red Corr: " << redCor << "\t";
	std::cout << "Blue Corr: " << blueCor << "\n";

	r0=0;
	g0=0;
	g1=0;
	b0=0;

	// reconstruct frame from four channels, applying corrections
                for(uint64_t i=0;i<iHeight;i++){
                        for(uint64_t j=0;j<iWidth;j++){
                                if(i %2 == 1) {
                                        if(j %2 == 0) {
						arrayCorrected.at(c0)=(int16_t)(redCor*arrayR0.at(r0));
                                               	r0++;
						c0++;
                                        } else {
						arrayCorrected.at(c0)=arrayG0.at(g0);
                                                g0++;
						c0++;
                                        }
                                } else {
                                        if(j %2 == 1) {
						arrayCorrected.at(c0)=arrayG1.at(g1);
                                                g1++;
						c0++;
                                        } else {
						arrayCorrected.at(c0)=(int16_t)blueCor*arrayB0.at(b0);
                                                b0++;
						c0++;
                                        }
                                }
                        }
                }
	// write data to file
	outFile.write(reinterpret_cast<char *>(arrayCorrected.data()), count*8);	
}
