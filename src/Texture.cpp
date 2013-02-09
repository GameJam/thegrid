#include "Texture.h"
#include "Render.h"

#include <FreeImage.h>
#include <stdio.h>
#include <malloc.h>

bool Texture_LoadFromMemory(Texture& texture, const void* buffer, size_t bufferLength)
{

    static int          init = 0;

    FIMEMORY*           stream = NULL;
    FIBITMAP*           bitmap = NULL;
    FREE_IMAGE_FORMAT   format;
    FREE_IMAGE_TYPE     type;
    int                 xSize;
    int                 ySize;
    unsigned int        bpp;
    unsigned int        pitch;

    if (!init)
    {
        FreeImage_Initialise(0);
        init = 1;
    }
    
    stream = FreeImage_OpenMemory( (BYTE*)buffer, (DWORD)bufferLength );
    format = FreeImage_GetFileTypeFromMemory(stream, 0);

    bitmap = FreeImage_LoadFromMemory( format, stream, 0 );

    FreeImage_CloseMemory(stream);
    stream = NULL;

    xSize  = FreeImage_GetWidth(bitmap);
    ySize  = FreeImage_GetHeight(bitmap);
    bpp    = FreeImage_GetBPP(bitmap);
    pitch  = FreeImage_GetPitch(bitmap);

    type = FreeImage_GetImageType(bitmap);

    if (type != FIT_BITMAP)
    {
        FreeImage_Unload(bitmap);
        return false;
    }

    texture.handle = 0;

    if (bpp == 32)
    {
        unsigned char* pixel = (unsigned char*)malloc( 4 * xSize * ySize );

        for (int y = 0; y < ySize; ++y)
        {

            BYTE* scanline;
            scanline = FreeImage_GetScanLine(bitmap, ySize - y - 1);

            for (int x = 0; x < xSize; ++x)
            {
                pixel[ (x + y * xSize) * 4 + 0 ] = scanline[x * 4 + FI_RGBA_RED];
                pixel[ (x + y * xSize) * 4 + 1 ] = scanline[x * 4 + FI_RGBA_GREEN];
                pixel[ (x + y * xSize) * 4 + 2 ] = scanline[x * 4 + FI_RGBA_BLUE];
                pixel[ (x + y * xSize) * 4 + 3 ] = scanline[x * 4 + FI_RGBA_ALPHA];
            }
            

        }

        texture.handle = Render_CreateTexture(xSize, ySize, pixel, 0);
        free(pixel);

    }
    else if (bpp == 24)
    {

        unsigned char* pixel = (unsigned char*)malloc( 4 * xSize * ySize );

        for (int y = 0; y < ySize; ++y)
        {

            BYTE* scanline;
            scanline = FreeImage_GetScanLine(bitmap, ySize - y - 1);

            for (int x = 0; x < xSize; ++x)
            {
                pixel[ (x + y * xSize) * 4 + 0 ] = scanline[x * 3 + FI_RGBA_RED];
                pixel[ (x + y * xSize) * 4 + 1 ] = scanline[x * 3 + FI_RGBA_GREEN];
                pixel[ (x + y * xSize) * 4 + 2 ] = scanline[x * 3 + FI_RGBA_BLUE];
                pixel[ (x + y * xSize) * 4 + 3 ] = 0xFF;
            }
            
        }

        texture.handle = Render_CreateTexture(xSize, ySize, pixel, 0);
        free(pixel);


    }

    texture.xSize = xSize;
    texture.ySize = ySize;
    
    FreeImage_Unload(bitmap);
    bitmap = NULL;

    return texture.handle != 0;

}

bool Texture_Load(Texture& texture, const char* fileName)
{

    FILE*   file            = NULL;
    void*   buffer          = NULL;
    size_t  bufferLength;

    file = fopen(fileName, "rb");

    if (file == NULL)
    {
        return false;
    }

    fseek(file, 0, SEEK_END);
    bufferLength = ftell(file);
    
    buffer = malloc(bufferLength);
    fseek(file, 0, SEEK_SET);
    fread(buffer, 1, bufferLength, file);

    fclose(file);

    bool result = Texture_LoadFromMemory(texture, buffer, bufferLength);
    free(buffer);

    return true;

}
