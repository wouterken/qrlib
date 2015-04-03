//
//  File.c
//  SampleDylib
//
//  Created by Wouter Coppieters on 8/1/13.
//  Copyright (c) 2013 Wouter Coppieters. All rights reserved.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "qrencode.h"
#include <math.h>

int writeEPS(QRcode *qrcode, const char *outfile, double r, double g, double b);
int writeEPSDots(char * blocks, int blockCount, int imageSize, int blockSize, const char *outfile, double r, double g, double b);
void makeDotPattern(char * c, const char *outfile, double r, double g, double b);
void makeQR(char * str, const char * outfile, double r, double g, double b);


void makeQR(char * str, const char * outfile, double r, double g, double b){
    QRcode * code = QRcode_encodeString(str ,0,QR_ECLEVEL_L, QR_MODE_8, 1);
    writeEPS(code, outfile, r, g, b);
}

FILE *openFile(const char *outfile)
{
	FILE *fp;
    
	if(outfile == NULL || (outfile[0] == '-' && outfile[1] == '\0')) {
		fp = stdout;
	} else {
		fp = fopen(outfile, "wb");
		if(fp == NULL) {
			fprintf(stderr, "Failed to create file: %s\n", outfile);
			perror(NULL);
            return NULL;
		}
	}
    
	return fp;
}



int writeEPS(QRcode *qrcode, const char *outfile, double r, double g, double b)
{
	FILE *fp;
	unsigned char *row, *p;
	int x, y, yy;
	int realwidth;
    int margin = 0;
    int size = 10;
    
	fp = openFile(outfile);
    
	realwidth = (qrcode->width + margin * 2) * size;
	/* EPS file header */
	fprintf(fp, "%%!PS-Adobe-2.0 EPSF-1.2\n"
            "%%%%BoundingBox: 0 0 %d %d\n"
            "%%%%Pages: 1 1\n"
            "%%%%EndComments\n", realwidth, realwidth);
    fprintf(fp, "%f %f %f setrgbcolor\n", r, g, b);
	/* draw point */
	fprintf(fp, "/p { "
            "moveto "
            "0 1 rlineto "
            "1 0 rlineto "
            "0 -1 rlineto "
            "fill "
            "} bind def "
            "%d %d scale ", size, size);
	
	/* data */
	p = qrcode->data;
	for(y=0; y<qrcode->width; y++) {
		row = (p+(y*qrcode->width));
		yy = (margin + qrcode->width - y - 1);
		
		for(x=0; x<qrcode->width; x++) {
			if(*(row+x)&0x1) {
				fprintf(fp, "%d %d p ", margin + x,  yy);
			}
		}
	}
    
	fprintf(fp, "\n%%%%EOF\n");
	fclose(fp);
    
	return 0;
}


void makeDotPattern(char * c, const char *outfile, double r, double g, double b){
    int i,byte = 0;
    unsigned long len = strlen(c);
    char bitstring[8 * len];
    char * bitPtr = bitstring;
    
    for(i = 0;i< len; i++){
        char character = c[i];
        while(byte <= 7){
            *bitPtr++ = (character >> (7-byte)) & 1;
            printf("%d",(character >> (7-byte)) & 1);
            byte++;
        }
        printf("\n");
        byte = 0;
    }
    
    
    writeEPSDots(bitstring, (int)len, 490, 3, outfile, r, g, b);
    
}


int writeEPSDots(char * blocks, int blockCount, int imageSize, int blockSize, const char *outfile, double r, double g, double b)
{
	FILE *fp;
    int i = 0;
    
	fp = openFile(outfile);
    /* EPS file header */
	fprintf(fp, "%%!PS-Adobe-2.0 EPSF-1.2\n"
            "%%%%BoundingBox: 0 0 %d %d\n"
            "%%%%Pages: 1 1\n"
            "%%%%EndComments\n", imageSize, imageSize);
    fprintf(fp, "%f %f %f setrgbcolor\n", r, g, b);
	/* draw point */
	fprintf(fp, "/p { "
            "moveto "
            "0 1 rlineto "
            "1 0 rlineto "
            "0 -1 rlineto "
            "fill "
            "} bind def "
            "%d %d scale \n", blockSize, blockSize);
    
    double width = ceil(sqrt(blockCount));
    int row=0;
    int column=0;
    
    double spacing = ((double)imageSize/width)/3.0/(double)blockSize;
    for(i=0; i < blockCount; i++){
        int b1 = blocks[i * 8];
        int b2 = blocks[i * 8 + 1];
        int b3 = blocks[i * 8 + 2];
        int b4 = blocks[i * 8 + 3];
        int b5 = blocks[i * 8 + 4];
        int b6 = blocks[i * 8 + 5];
        int b7 = blocks[i * 8 + 6];
        int b8 = blocks[i * 8 + 7];
        printf("\n%d%d%d%d-%d%d%d%d", b1,b2,b3,b4,b5,b6,b7,b8);
        b1 && fprintf(fp, "%d %d p \n", (int)spacing * column, (int)spacing * row);
        b2 && fprintf(fp, "%d %d p \n", (int)spacing * column, (int)spacing * (row + 1));
        b3 && fprintf(fp, "%d %d p \n", (int)spacing * column, (int)spacing * (row + 2));
        b4 && fprintf(fp, "%d %d p \n", (int)spacing * (column + 1), (int)spacing * row);
        fprintf(fp, "%d %d p\n ", (int)spacing * (column + 1), (int)spacing * (row + 1));
        b5 && fprintf(fp, "%d %d p \n", (int)spacing * (column + 1), (int)spacing * (row + 2));
        b6 && fprintf(fp, "%d %d p \n", (int)spacing * (column + 2), (int)spacing * row);
        b7 && fprintf(fp, "%d %d p \n", (int)spacing * (column + 2), (int)spacing * (row + 1));
        b8 && fprintf(fp, "%d %d p \n", (int)spacing * (column + 2), (int)spacing * (row + 2));
        column += 3;
        if(column >= width * 3){
            column = 0;
            row += 3;
        }
        
    }
    fprintf(fp, "\n%%%%EOF\n");
	fclose(fp);
    
    return 1;
}