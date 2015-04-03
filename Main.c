//
//  Main.c
//  SRLLib
//
//  Created by Wouter Coppieters on 8/1/13.
//  Copyright (c) 2013 Wouter Coppieters. All rights reserved.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "qrencode.h"
#include "dmtx.h"
#include <math.h>


void getQREPS(const char *data, double requestedSize, double xPos, double yPos,  int rotate, char * EPS);
void getDotsEPS(const char *data, double requestedSize, double xPos, double yPos, int rotate, char * EPS);
void getMatrixEPS(const char * data, double requestedSize, double xPos, double yPos,int rotate, char * EPS);

int writeEPS(QRcode *qrcode, const char *outfile, double c, double m, double y, double k);
int writeEPSDots(char * blocks, int blockCount, int imageSize, int blockSize, const char *outfile, double c, double m, double y, double k);
int writeDataMatrix(char * data, double size, double c, double m, double y, double k, const char * outfile);

void makeDotPattern(char * str, const char *outfile, double c, double m, double y, double k);
void makeQR(char * str, const char * outfile, double c, double m, double y, double k);
void makeMatrix(char * str, const char * outfile, double c, double m, double y, double k);

void makeMatrix(char * str, const char * outfile, double c, double m, double y, double k){
    writeDataMatrix(str, 1024, c, m, y, k, outfile);
}
void makeQR(char * str, const char * outfile, double c, double m, double y, double k){
    QRcode * code = QRcode_encodeString(str ,0,QR_ECLEVEL_L, QR_MODE_8, 1);
    writeEPS(code, outfile, c, m, y, k);
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

void getMatrixEPS(const char * data, double requestedSize, double xPos, double yPos, int rotate, char * EPS){
    
    
    /* Save Matrix stack */
    EPS += sprintf(EPS, "gsave ");
    EPS += sprintf(EPS, "%f %f translate ", xPos, yPos);
    
    if(rotate){
        EPS += sprintf(EPS, "90 rotate ");
        EPS += sprintf(EPS, "0 -%f translate ", requestedSize);
    }
    
    
    size_t          width, height, bytesPerPixel;
    unsigned char  *pxl;
    unsigned char  *pxlStart;
    DmtxEncode     *enc;
    double size;
    
    
    
    /* 1) ENCODE a new Data Matrix barcode image (in memory only) */
    
    enc = dmtxEncodeCreate();
    
    dmtxEncodeSetProp(enc, DmtxPropMarginSize, 0);
    dmtxEncodeSetProp(enc, DmtxPropModuleSize, 1);
    dmtxEncodeDataMatrix(enc, strlen(data), (unsigned char*)data);
    
    /* 2) COPY the new image data before releasing encoding memory */
    
    width = dmtxImageGetProp(enc->image, DmtxPropWidth);
    height = dmtxImageGetProp(enc->image, DmtxPropHeight);
    bytesPerPixel = dmtxImageGetProp(enc->image, DmtxPropBytesPerPixel);
    
    size = (requestedSize / width);
    
    pxl = (unsigned char *)malloc(width * height * bytesPerPixel);
    pxlStart = pxl;
    
    memcpy(pxl, enc->image->pxl, width * height * bytesPerPixel);
    EPS += sprintf(EPS, "/p { "
                   "moveto "
                   "0 1 rlineto "
                   "1 0 rlineto "
                   "0 -1 rlineto "
                   "fill "
                   "} bind def "
                   "%f %f scale \n", size, size);
    
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            if(!pxl[(x + y * width) * bytesPerPixel]){
                EPS += sprintf(EPS, "%d %ld p ",x, height - y - 1);
            }
            
        }
        printf("\n\n");
    }
    dmtxEncodeDestroy(&enc);
    
    EPS += sprintf(EPS, "\ngrestore\n");
    
    free(pxl);
    
}

void getDotsEPS(const char * data, double requestedSize, double xPos, double yPos, int rotate, char * EPS){
    
    
    /* Save Matrix stack */
    EPS += sprintf(EPS, "gsave ");
    EPS += sprintf(EPS, "%f %f translate ", xPos, yPos);
    
    if(rotate){
        EPS += sprintf(EPS, "90 rotate ");
        EPS += sprintf(EPS, "0 -%f translate ", requestedSize);
    }
    
    
    size_t          width, height, bytesPerPixel;
    unsigned char  *pxl;
    unsigned char  *pxlStart;
    DmtxEncode     *enc;
    double dotSize = 0.255;
    double size;
    
    
    
    /* 1) ENCODE a new Data Matrix barcode image (in memory only) */
    
    enc = dmtxEncodeCreate();
    
    dmtxEncodeSetProp(enc, DmtxPropMarginSize, 0);
    dmtxEncodeSetProp(enc, DmtxPropModuleSize, 1);
    dmtxEncodeDataMatrix(enc, strlen(data), (unsigned char*)data);
    
    /* 2) COPY the new image data before releasing encoding memory */
    
    width = dmtxImageGetProp(enc->image, DmtxPropWidth);
    height = dmtxImageGetProp(enc->image, DmtxPropHeight);
    bytesPerPixel = dmtxImageGetProp(enc->image, DmtxPropBytesPerPixel);
    
    size = (requestedSize / width);
    dotSize = dotSize / size;
    
    pxl = (unsigned char *)malloc(width * height * bytesPerPixel);
    pxlStart = pxl;
    
    memcpy(pxl, enc->image->pxl, width * height * bytesPerPixel);
    EPS += sprintf(EPS, "/p { "
                   "moveto "
                   "0 %f rlineto "
                   "%f 0 rlineto "
                   "0 -%f rlineto "
                   "fill "
                   "} bind def "
                   "%f %f scale \n", dotSize, dotSize, dotSize, size, size);
    
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            if(!pxl[(x + y * width) * bytesPerPixel]){
                EPS += sprintf(EPS, "%d %ld p ",x, height - y - 1);
            }
            
        }
        printf("\n\n");
    }
    dmtxEncodeDestroy(&enc);
    
    EPS += sprintf(EPS, "\ngrestore\n");
    
    free(pxl);
    
}


void getQREPS(const char *data, double requestedSize, double xPos, double yPos, int rotate, char * EPS){
    QRcode * qrcode = QRcode_encodeString(data ,0,QR_ECLEVEL_L, QR_MODE_8, 1);
    unsigned char *row, *p;
    int x, y, yy;
    int margin = 0;
    double size;
    
    
    size = (requestedSize / qrcode->width);
    
    /* EPS file header */
    
    /* Save Matrix stack */
    EPS += sprintf(EPS, "gsave ");
    EPS += sprintf(EPS, "%f %f translate ", xPos, yPos);
    
    if(rotate){
        EPS += sprintf(EPS, "90 rotate ");
        EPS += sprintf(EPS, "0 -%f translate ", requestedSize);
    }
    
    /* draw point */
    EPS += sprintf(EPS, "/p { "
                   "moveto "
                   "0 1 rlineto "
                   "1 0 rlineto "
                   "0 -1 rlineto "
                   "fill "
                   "} bind def "
                   "%f %f scale ", size, size);
    
    /* data */
    p = qrcode->data;
    for(y=0; y<qrcode->width; y++) {
        row = (p+(y*qrcode->width));
        yy = (margin + qrcode->width - y - 1);
        
        for(x=0; x<qrcode->width; x++) {
            if(*(row+x)&0x1) {
                EPS += sprintf(EPS, "%d %d p ",margin + x,  yy);
            }
        }
    }
    EPS += sprintf(EPS, "\ngrestore\n");
    QRcode_free(qrcode);
    
}

int writeEPS(QRcode *qrcode, const char *outfile, double c, double m, double Y, double k)
{
    FILE *fp;
    unsigned char *row, *p;
    int x, y, yy;
    int realwidth;
    int margin = 0;
    int size = 25;
    
    fp = openFile(outfile);
    
    realwidth = (qrcode->width + margin * 2) * size;
    
    /* EPS file header */
    fprintf(fp, "%%!PS-Adobe-2.0 EPSF-1.2\n"
            "%%%%BoundingBox: 0 0 %d %d\n"
            "%%%%Pages: 1 1\n"
            "%%%%EndComments\n", realwidth, realwidth);
    
    fprintf(fp, "%f %f %f %f setcmykcolor\n", c, m, Y, k);
    
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
    QRcode_free(qrcode);
    return 0;
}



void makeDotPattern(char * str, const char *outfile, double c, double m, double y, double k){
    int i,byte = 0;
    unsigned long len = strlen(str);
    char bitstring[8 * len];
    char * bitPtr = bitstring;
    
    for(i = 0;i< len; i++){
        char character = str[i];
        while(byte <= 7){
            *bitPtr++ = (character >> (7-byte)) & 1;
            printf("%d",(character >> (7-byte)) & 1);
            byte++;
        }
        printf("\n");
        byte = 0;
    }
    
    
    writeEPSDots(bitstring, (int)len, 490, 3, outfile, c, m, y, k);
    
}

int writeEPSDots(char * blocks, int blockCount, int imageSize, int blockSize, const char *outfile, double c, double m, double y, double k)
{
    FILE *fp;
    int i = 0;
    
    fp = openFile(outfile);
    /* EPS file header */
    fprintf(fp, "%%!PS-Adobe-2.0 EPSF-1.2\n"
            "%%%%BoundingBox: 0 0 %d %d\n"
            "%%%%Pages: 1 1\n"
            "%%%%EndComments\n", imageSize, imageSize);
    fprintf(fp, "%f %f %f %f setcmykcolor\n", c, m, y, k);
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

int writeDataMatrix(char * data, double size, double c, double m, double y, double k, const char * outfile)
{
    FILE *fp;
    
    fp = openFile(outfile);
    /* EPS file header */
    fprintf(fp, "%%!PS-Adobe-2.0 EPSF-1.2\n"
            "%%%%BoundingBox: 0 0 %f %f\n"
            "%%%%Pages: 1 1\n"
            "%%%%EndComments\n", size, size);
    char EPS[1024 * 1024];
    
    
    fprintf(fp, "%f %f %f %f setcmykcolor\n", c, m, y, k);
    
    getMatrixEPS(data, size, 0, 0, 0, EPS);
    
    fprintf(fp, "%s", EPS);
    fclose(fp);
    
    return 1;
}