#include <stdio.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int NumArgs, char **Args)
{
    if (NumArgs == 1)
    {
        printf("\nusage: imagetrimmer [image_filepath]\n");
        return 0;
    }

    for (int ArgIdx = 1; ArgIdx < NumArgs; ArgIdx++)
    {
        // Load the image.
        int32_t ChannelCount;
        int ImageWidth;
        int ImageHeight;
        uint32_t *ImageData = (uint32_t *)stbi_load(Args[ArgIdx], &ImageWidth, &ImageHeight, &ChannelCount, 0);
        if (ImageData == NULL)
        {
            printf("error: could not load image\n");
            return 1;
        }
        if (ChannelCount != 4)
        {
            printf("error: channel count not 4 (only 4 is supported)\n");
            return 1;
        }

        // Get the bounding box of the non-empty portion of the image.
        int MinX = INT_MAX, MinY = INT_MAX;
        int MaxX = -1, MaxY = -1;
        for (int X = 0; X < ImageWidth; X++)
        {
            for (int Y = 0; Y < ImageHeight; Y++)
            {
                uint32_t Pixel = ImageData[Y*ImageWidth + X];
                if (Pixel != 0)
                {
                    if (X < MinX)
                        MinX = X;
                    if (Y < MinY)
                        MinY = Y;
                    if (X > MaxX)
                        MaxX = X;
                    if (Y > MaxY)
                        MaxY = Y;
                }
            }
        }

        // Ensure the trimmed image width is a multiple of 4; we do this because OpenGL crashes otherwise.
        int TrimmedImageWidth = MaxX - MinX + 1;
        if (TrimmedImageWidth % 4 != 0)
        {
            MinX--;
            TrimmedImageWidth++;
            if (TrimmedImageWidth % 4 != 0)
            {
                MaxX++;
                TrimmedImageWidth++;
                if (TrimmedImageWidth % 4 != 0)
                {
                    MinX--;
                    TrimmedImageWidth++;
                    assert(TrimmedImageWidth % 4 == 0);
                }
            }
        }

        // Write out the trimmed image, replacing the original.
        int TrimmedImageHeight = MaxY - MinY + 1;
        int DestImageWriteResult = stbi_write_png(Args[ArgIdx], TrimmedImageWidth, TrimmedImageHeight, 4, &ImageData[MinY*ImageWidth + MinX], 4*ImageWidth);
        if (DestImageWriteResult == 0)
        {
            printf("error: failed to write image\n");
            return 1;
        }
    }

    return 0;
}