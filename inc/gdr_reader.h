/*
* gdrreader.h
* Simple GDR file reader for fixed-size boards and solutions
*/

#ifndef GDR_SIMPLE_H
#define GDR_SIMPLE_H

#include <genesis.h>

#define GDR_MAX_WIDTH 16
#define GDR_MAX_HEIGHT 16
#define GDR_MAX_DATA (GDR_MAX_WIDTH * GDR_MAX_HEIGHT)
#define GDR_MAX_FOLDS 64

typedef struct {
    u8 width;
    u8 height;
    u8 startX;
    u8 startY;
    u8 exitX;
    u8 exitY;
    u16 par;
    u8 idle;
    char data[GDR_MAX_DATA];
} GDRBoard;

typedef struct {
    char move;
    char data[GDR_MAX_DATA];
} GDRFold;

typedef struct {
    u16 score;
    u8 foldCount;
    GDRFold folds[GDR_MAX_FOLDS];
} GDRSolution;

bool GDR_LoadBoardData(const u8* gdrData, u16 boardIndex, GDRBoard* board, GDRSolution* solution);
u16 GDR_GetBoardCount(const u8* gdrData);

#endif