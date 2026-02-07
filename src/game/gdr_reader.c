/*
* gdr_reader.c - Reader for GDR board files from binary resources
*/

#include "gdr_reader.h"
#include <string.h>

static u8 gdrBuffer[512]; /* RLE buffer */

static u8 readByte(const u8** ptr) {
    u8 b = **ptr;
    (*ptr)++;
    return b;
}

static u16 readUInt16(const u8** ptr) {
    u8 lo = readByte(ptr);
    u8 hi = readByte(ptr);
    return (hi << 8) | lo;
}

static bool readPackedByte(const u8** ptr, u8* val1, u8* val2) {
    u8 packed = readByte(ptr);
    if (packed == 0xFF) {
        *val1 = readByte(ptr);
        *val2 = readByte(ptr);
    } else {
        *val1 = (packed >> 4) & 0x0F;
        *val2 = packed & 0x0F;
    }
    return TRUE;
}

static u16 decompressRLE(const u8** ptr, char* output, u16 maxLen) {
    u16 compressedLen = readUInt16(ptr);
    u16 outPos = 0;
    u16 i;
    
    if (compressedLen == 0 || compressedLen > sizeof(gdrBuffer) || maxLen == 0) {
        return 0;
    }
    
    memcpy(gdrBuffer, *ptr, compressedLen);
    *ptr += compressedLen;
    
    /* Decompress RLE: [count][char] pairs */
    for (i = 0; i < compressedLen; i += 2) {
        u8 count = gdrBuffer[i];
        char ch = (char)gdrBuffer[i + 1];
        u8 j;
        
        for (j = 0; j < count && outPos < maxLen; j++) {
            output[outPos++] = ch;
        }
    }
    
    return outPos;
}

u16 GDR_GetBoardCount(const u8* gdrData) {
    const u8* ptr = gdrData;
    
    if (!gdrData) {
        return 0;
    }
    
    /* magic */
    if (ptr[0] != 'G' || ptr[1] != 'D' || ptr[2] != '1') {
        return 0;
    }
    
    ptr += 3;
    
    return readUInt16(&ptr);
}

bool GDR_LoadBoardData(const u8* gdrData, u16 boardIndex, GDRBoard* board, GDRSolution* solution) {
    const u8* ptr = gdrData;
    u16 totalBoards;
    u16 i;
    u8 parByte, scoreByte;
    
    if (!gdrData) {
        return FALSE;
    }
    
    /* magic */
    if (ptr[0] != 'G' || ptr[1] != 'D' || ptr[2] != '1') {
        return FALSE;
    }
    
    ptr += 3;
    totalBoards = readUInt16(&ptr);
    if (boardIndex >= totalBoards) {
        return FALSE;
    }
    
    for (i = 0; i < boardIndex; i++) {
        u16 compressedLen;
        u8 foldCount, j;
        u8 byte;
        
        /* Skip dimensions */
        byte = readByte(&ptr);
        if (byte == 0xFF) {
            readByte(&ptr);
            readByte(&ptr);
        }
        
        /* Skip start position */
        byte = readByte(&ptr);
        if (byte == 0xFF) {
            readByte(&ptr);
            readByte(&ptr);
        }
        
        /* Skip exit position */
        byte = readByte(&ptr);
        if (byte == 0xFF) {
            readByte(&ptr);
            readByte(&ptr);
        }
        
        /* Skip par */
        byte = readByte(&ptr);
        if (byte == 0xFF) {
            readUInt16(&ptr);
        }
        
        /* Skip idle */
        readByte(&ptr);
        
        /* Skip board data */
        compressedLen = readUInt16(&ptr);
        ptr += compressedLen;
        
        /* Skip score */
        byte = readByte(&ptr);
        if (byte == 0xFF) {
            readUInt16(&ptr);
        }
        
        /* Skip folds */
        foldCount = readByte(&ptr);
        for (j = 0; j < foldCount; j++) {
            readByte(&ptr); /* move */
            compressedLen = readUInt16(&ptr);
            ptr += compressedLen;
        }
    }
    
    /* Read the board */
    readPackedByte(&ptr, &board->width, &board->height);
    readPackedByte(&ptr, &board->startX, &board->startY);
    readPackedByte(&ptr, &board->exitX, &board->exitY);
    
    parByte = readByte(&ptr);
    board->par = (parByte == 0xFF) ? readUInt16(&ptr) : parByte;
    
    board->idle = readByte(&ptr);
    
    /* Read board data */
    if (decompressRLE(&ptr, board->data, GDR_MAX_DATA) == 0) {
        return FALSE;
    }
    
    /* Read solution */
    scoreByte = readByte(&ptr);
    solution->score = (scoreByte == 0xFF) ? readUInt16(&ptr) : scoreByte;
    
    solution->foldCount = readByte(&ptr);
    if (solution->foldCount > GDR_MAX_FOLDS) {
        solution->foldCount = GDR_MAX_FOLDS;
    }
    
    for (i = 0; i < solution->foldCount; i++) {
        solution->folds[i].move = (char)readByte(&ptr);
        decompressRLE(&ptr, solution->folds[i].data, GDR_MAX_DATA);
    }
    
    return TRUE;
}
