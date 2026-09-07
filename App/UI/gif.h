// gif.h
// Public Domain / MIT License
// Single-header C GIF encoder library by Charlie Tangora.

#ifndef GIF_H
#define GIF_H

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wcast-align"
#endif


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Simple struct to represent a GIF image writer
typedef struct GifWriter {
    FILE* f;
    uint8_t* oldImage;
    bool firstFrame;
} GifWriter;

// Output a byte to the GIF file
static inline void GifWriteByte(uint8_t b, FILE* f)
{
    fputc(b, f);
}

// Output a word to the GIF file (little endian)
static inline void GifWriteWord(uint16_t w, FILE* f)
{
    fputc(static_cast<uint8_t>(w & 0xff), f);
    fputc(static_cast<uint8_t>((w >> 8) & 0xff), f);
}

// Write string to GIF file
static inline void GifWriteString(const char* str, FILE* f)
{
    while (*str)
    {
        fputc(static_cast<uint8_t>(*str++), f);
    }
}

// Structure to hold a color palette tree node
typedef struct {
    uint32_t bitDepth;
    uint8_t r[256];
    uint8_t g[256];
    uint8_t b[256];
    uint8_t k[256];
    // Tree data
    int32_t tree[256][256];
} GifPalette;

// Helper to calculate color distance
static inline int32_t GifColorDist(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2)
{
    int32_t dr = static_cast<int32_t>(r1) - static_cast<int32_t>(r2);
    int32_t dg = static_cast<int32_t>(g1) - static_cast<int32_t>(g2);
    int32_t db = static_cast<int32_t>(b1) - static_cast<int32_t>(b2);
    return dr * dr + dg * dg + db * db;
}

// Build color palette for a 32-bit RGBA image
static inline void GifMakePalette(const uint8_t* image, uint32_t width, uint32_t height, int32_t bitDepth, GifPalette* pPal)
{
    pPal->bitDepth = static_cast<uint32_t>(bitDepth);
    uint32_t numColors = 1u << bitDepth;

    // Seed default essential circuit & LED colors into palette
    static const uint8_t defaultSeed[][3] = {
        {255, 255, 255}, {0, 0, 0},       {255, 0, 0},     {0, 255, 0},
        {0, 0, 255},     {255, 255, 0},   {0, 255, 255},   {255, 0, 255},
        {255, 128, 0},   {128, 128, 128}, {200, 200, 200}, {128, 0, 0},
        {0, 128, 0},     {0, 0, 128},     {70, 130, 180},  {220, 20, 60}
    };
    uint32_t seedCount = static_cast<uint32_t>(sizeof(defaultSeed) / sizeof(defaultSeed[0]));

    for (uint32_t i = 0; i < seedCount && i < numColors; ++i) {
        pPal->r[i] = defaultSeed[i][0];
        pPal->g[i] = defaultSeed[i][1];
        pPal->b[i] = defaultSeed[i][2];
    }

    uint32_t palCount = seedCount;

    // Build 5-bit per channel histogram (32768 bins)
    static uint32_t counts[32768];
    memset(counts, 0, sizeof(counts));

    uint32_t totalPixels = width * height;
    for (uint32_t i = 0; i < totalPixels; ++i) {
        uint8_t r = image[i * 4 + 0];
        uint8_t g = image[i * 4 + 1];
        uint8_t b = image[i * 4 + 2];
        uint32_t bin = static_cast<uint32_t>(((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3));
        counts[bin]++;
    }

    typedef struct {
        uint32_t bin;
        uint32_t count;
    } BinCount;

    static BinCount activeBins[2048];
    uint32_t activeCount = 0;

    for (uint32_t b = 0; b < 32768; ++b) {
        if (counts[b] > 0) {
            if (activeCount < 2048) {
                activeBins[activeCount].bin = b;
                activeBins[activeCount].count = counts[b];
                activeCount++;
            }
        }
    }

    // Sort active bins by frequency descending
    for (uint32_t i = 0; i < activeCount; ++i) {
        for (uint32_t j = i + 1; j < activeCount; ++j) {
            if (activeBins[j].count > activeBins[i].count) {
                BinCount tmp = activeBins[i];
                activeBins[i] = activeBins[j];
                activeBins[j] = tmp;
            }
        }
    }

    // Add active colors to palette if not already represented
    for (uint32_t i = 0; i < activeCount && palCount < numColors; ++i) {
        uint32_t bin = activeBins[i].bin;
        uint8_t r = static_cast<uint8_t>(((bin >> 10) & 0x1f) << 3);
        uint8_t g = static_cast<uint8_t>(((bin >> 5) & 0x1f) << 3);
        uint8_t b = static_cast<uint8_t>((bin & 0x1f) << 3);

        bool exists = false;
        for (uint32_t k = 0; k < palCount; ++k) {
            int32_t dist = GifColorDist(r, g, b, pPal->r[k], pPal->g[k], pPal->b[k]);
            if (dist < 64) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            pPal->r[palCount] = r;
            pPal->g[palCount] = g;
            pPal->b[palCount] = b;
            palCount++;
        }
    }

    // Fill remaining slots with white
    for (uint32_t i = palCount; i < numColors; ++i) {
        pPal->r[i] = 255;
        pPal->g[i] = 255;
        pPal->b[i] = 255;
    }
}

// Pick nearest color index from palette
static inline uint8_t GifGetNearestColor(const GifPalette* pPal, uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t numColors = 1u << pPal->bitDepth;
    int32_t bestDist = 0x7fffffff;
    uint8_t bestIdx = 0;

    for (uint32_t i = 0; i < numColors; ++i)
    {
        int32_t dist = GifColorDist(r, g, b, pPal->r[i], pPal->g[i], pPal->b[i]);
        if (dist < bestDist)
        {
            bestDist = dist;
            bestIdx = static_cast<uint8_t>(i);
            if (dist == 0) break;
        }
    }
    return bestIdx;
}

// LZW Compression structures
typedef struct {
    uint32_t chunkIndex;
    uint8_t chunk[256];
} GifBuffer;

static inline void GifWriteChunk(GifBuffer* buf, FILE* f)
{     if (buf->chunkIndex > 0)
    {
        fputc(static_cast<uint8_t>(buf->chunkIndex), f);
        fwrite(buf->chunk, 1, buf->chunkIndex, f);
        buf->chunkIndex = 0;
    }
}

static inline void GifWriteCode(GifBuffer* buf, FILE* f, uint32_t code, uint32_t codeLen, uint32_t* pAccum, uint32_t* pAccumBits)
{
    *pAccum |= (code << (*pAccumBits));
    *pAccumBits += codeLen;

    while (*pAccumBits >= 8)
    {
        buf->chunk[buf->chunkIndex++] = static_cast<uint8_t>(*pAccum & 0xff);
        if (buf->chunkIndex == 255)
        {
            GifWriteChunk(buf, f);
        }
        *pAccum >>= 8;
        *pAccumBits -= 8;
    }
}

// Write LZW-compressed frame pixels
static inline void GifWriteLZW(FILE* f, const uint8_t* pixels, uint32_t numPixels, int32_t minCodeLen)
{
    GifBuffer buf;
    buf.chunkIndex = 0;

    uint32_t accum = 0;
    uint32_t accumBits = 0;

    uint32_t clearCode = 1u << minCodeLen;
    uint32_t eofCode = clearCode + 1;
    uint32_t nextCode = eofCode + 1;
    uint32_t codeLen = static_cast<uint32_t>(minCodeLen) + 1;

    // Dictionary hash table
    int32_t dict[4096][256];
    memset(dict, -1, sizeof(dict));

    GifWriteCode(&buf, f, clearCode, codeLen, &accum, &accumBits);

    if (numPixels == 0)
    {
        GifWriteCode(&buf, f, eofCode, codeLen, &accum, &accumBits);
        if (accumBits > 0)
        {
            buf.chunk[buf.chunkIndex++] = static_cast<uint8_t>(accum & 0xff);
        }
        GifWriteChunk(&buf, f);
        fputc(0, f);
        return;
    }

    uint32_t curCode = pixels[0];

    for (uint32_t i = 1; i < numPixels; ++i)
    {
        uint8_t pixel = pixels[i];
        int32_t match = dict[curCode][pixel];

        if (match >= 0)
        {
            curCode = static_cast<uint32_t>(match);
        }
        else
        {
            GifWriteCode(&buf, f, curCode, codeLen, &accum, &accumBits);

            if (nextCode < 4096)
            {
                dict[curCode][pixel] = static_cast<int32_t>(nextCode);
                nextCode++;
                if (nextCode > (1u << codeLen) && codeLen < 12)
                {
                    codeLen++;
                }
            }
            else
            {
                GifWriteCode(&buf, f, clearCode, codeLen, &accum, &accumBits);
                memset(dict, -1, sizeof(dict));
                nextCode = eofCode + 1;
                codeLen = static_cast<uint32_t>(minCodeLen) + 1;
            }

            curCode = pixel;
        }
    }

    GifWriteCode(&buf, f, curCode, codeLen, &accum, &accumBits);
    GifWriteCode(&buf, f, eofCode, codeLen, &accum, &accumBits);

    if (accumBits > 0)
    {
        buf.chunk[buf.chunkIndex++] = static_cast<uint8_t>(accum & 0xff);
    }
    GifWriteChunk(&buf, f);
    fputc(0, f); // Block terminator
}

// Initialize GIF file output
static inline bool GifBegin(GifWriter* writer, const char* filename, uint32_t width, uint32_t height, uint32_t delay, int32_t bitDepth, bool dither)
{
    static_cast<void>(dither);
    writer->f = fopen(filename, "wb");
    if (!writer->f) return false;

    writer->firstFrame = true;
    writer->oldImage = NULL;

    // GIF Header
    GifWriteString("GIF89a", writer->f);

    // Screen Descriptor
    GifWriteWord(static_cast<uint16_t>(width), writer->f);
    GifWriteWord(static_cast<uint16_t>(height), writer->f);
    GifWriteByte(static_cast<uint8_t>(0xf0 | (bitDepth - 1)), writer->f); // Global color table flag + size
    GifWriteByte(0, writer->f); // Background color index
    GifWriteByte(0, writer->f); // Pixel aspect ratio

    // Dummy Global Color Table (written during first frame or header)
    uint32_t numColors = 1u << bitDepth;
    for (uint32_t i = 0; i < numColors; ++i)
    {
        GifWriteByte(0, writer->f);
        GifWriteByte(0, writer->f);
        GifWriteByte(0, writer->f);
    }

    // Netscape Application Extension for looping infinitely
    GifWriteByte(0x21, writer->f); // Extension Introducer
    GifWriteByte(0xff, writer->f); // Application Extension Label
    GifWriteByte(11, writer->f);   // Block Size
    GifWriteString("NETSCAPE2.0", writer->f);
    GifWriteByte(3, writer->f);    // Sub-block size
    GifWriteByte(1, writer->f);    // Loop count ID
    GifWriteWord(0, writer->f);    // Infinite loop count
    GifWriteByte(0, writer->f);    // Block terminator

    static_cast<void>(delay);
    return true;
}

// Write a single RGBA frame to the GIF file
static inline bool GifWriteFrame(GifWriter* writer, const uint8_t* image, uint32_t width, uint32_t height, uint32_t delay, int32_t bitDepth, bool dither)
{
    static_cast<void>(dither);
    if (!writer || !writer->f) return false;

    // Graphic Control Extension
    GifWriteByte(0x21, writer->f); // Extension Introducer
    GifWriteByte(0xf9, writer->f); // Graphic Control Label
    GifWriteByte(4, writer->f);    // Block size
    GifWriteByte(0x04, writer->f); // Disposal method (001: do not dispose)
    GifWriteWord(static_cast<uint16_t>(delay), writer->f); // Delay time in hundredths of a second
    GifWriteByte(0, writer->f);    // Transparent color index
    GifWriteByte(0, writer->f);    // Block terminator

    // Image Descriptor
    GifWriteByte(0x2c, writer->f); // Image separator
    GifWriteWord(0, writer->f);    // Image left position
    GifWriteWord(0, writer->f);    // Image top position
    GifWriteWord(static_cast<uint16_t>(width), writer->f);
    GifWriteWord(static_cast<uint16_t>(height), writer->f);

    GifPalette pal;
    GifMakePalette(image, width, height, bitDepth, &pal);

    // Local Color Table Flag
    GifWriteByte(static_cast<uint8_t>(0x80 | (bitDepth - 1)), writer->f);

    // Write Local Color Table
    uint32_t numColors = 1u << bitDepth;
    for (uint32_t i = 0; i < numColors; ++i)
    {
        GifWriteByte(pal.r[i], writer->f);
        GifWriteByte(pal.g[i], writer->f);
        GifWriteByte(pal.b[i], writer->f);
    }

    // Convert RGBA image to palette index array
    uint32_t totalPixels = width * height;
    uint8_t* indexedPixels = static_cast<uint8_t*>(malloc(totalPixels));
    if (!indexedPixels) return false;

    for (uint32_t i = 0; i < totalPixels; ++i)
    {
        uint8_t r = image[i * 4 + 0];
        uint8_t g = image[i * 4 + 1];
        uint8_t b = image[i * 4 + 2];
        indexedPixels[i] = GifGetNearestColor(&pal, r, g, b);
    }

    // Write LZW Minimum Code Size
    int32_t minCodeLen = bitDepth < 2 ? 2 : bitDepth;
    GifWriteByte(static_cast<uint8_t>(minCodeLen), writer->f);

    // Write LZWCompressed image data
    GifWriteLZW(writer->f, indexedPixels, totalPixels, minCodeLen);

    free(indexedPixels);
    return true;
}

// Close and finalize GIF file
static inline bool GifEnd(GifWriter* writer)
{
    if (!writer || !writer->f) return false;

    GifWriteByte(0x3b, writer->f); // GIF Trailer
    fclose(writer->f);
    writer->f = NULL;
    if (writer->oldImage)
    {
        free(writer->oldImage);
        writer->oldImage = NULL;
    }
    return true;
}

#ifdef __cplusplus
}
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif // GIF_H
