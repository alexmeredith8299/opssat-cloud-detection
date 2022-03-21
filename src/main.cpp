#define DEBUG

#include <stdio.h>
#include <iostream>
#include <string.h> /* for string functions like strcmp */
#include <stdlib.h> /* for casting function like atoi */
#include <errno.h>  /* standard linux error codes: https://www.thegeekstuff.com/2010/10/linux-error-codes/ */
#include <stdint.h> /* for portability when dealing with integer data types */

// relevant STB headers
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// tensorflow headers
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#include <csv2/reader.hpp> // include csv2 for random forest

#ifdef DEBUG
#include "tensorflow/lite/optional_debug_tools.h"
#endif

// custom headers
#include "constants.h"
#include "unet.h"
#include "helper.h"

/* define convenience macros */
#define streq(s1, s2) (!strcmp((s1), (s2)))
#define MINIMAL_CHECK(x)                                         \
    if (!(x))                                                    \
    {                                                            \
        fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
        exit(1);                                                 \
    }

using std::string;

// todos...
// be able to do patchwise computations
// proper debugging print statements
// implement proper filenames

int parse_options(int argc, char **argv, string *img_write_path, int *img_write_mode)
{
    /* get provider host and port from command arguments */
    int8_t argv_index_input = -1;
    int8_t argv_index_write = -1;

    // --------------------------------------------------------------------------
    // parse the command arguments

    int8_t argn;
    for (argn = 1; argn < argc; argn++)
    {
        if (streq(argv[argn], "--help") || streq(argv[argn], "-?"))
        {
            printf("io_demo [options] ...");
            printf("\n  --input    / -i       the file path of the input image");
            printf("\n  --write    / -w       the write mode of the output image (optional)"
                   "\n\t0 - do not write a new image (equivalent to not specifying the --write option)"
                   "\n\t1 - write a new image as a new file"
                   "\n\t2 - write a new image that overwrites the input image file"
                   "\n\t3 - same as option 2 but backs up the original input image");
            printf("\n  --help     / -?       this information\n");

            /* program error exit code */
            /* 11 	EAGAIN 	Try again */
            return EAGAIN;
        }
        else if (streq(argv[argn], "--input") || streq(argv[argn], "-i"))
            argv_index_input = ++argn;
        else if (streq(argv[argn], "--write") || streq(argv[argn], "-w"))
            argv_index_write = ++argn;
        else
        {
            /* print error message */
            printf("Unknown option. Get help: ./io_demo -?");

            /* program error exit code */
            /* 22 	EINVAL 	Invalid argument */
            return EINVAL;
        }
    }

    // --------------------------------------------------------------------------
    // parse the input image file path

    if (argv_index_input == -1)
    {
        /* print error message */
        printf("no image input path specified. Get help: ./io_demo -?\n");

        /* program error exit code */
        /* 22 	EINVAL 	Invalid argument */
        return EINVAL;
    }
    else
    {
        /* printf for documentation purposes only */
        printf("image to process: %s\n", argv[argv_index_input]);
        *img_write_path = string(argv[argv_index_input]);
    }

    // --------------------------------------------------------------------------
    // parse the output image write mode option, if given
    if (argv_index_write == -1)
    {
        /* printf for documentation purposes only */
        printf("no output image will be written\n");
    }
    else
    {
        /* parse write mode option string to int */
        int8_t write_mode = atoi(argv[argv_index_write]);

        // set to write mode
        *img_write_mode = write_mode;
        // printf("write mode %i\n", write_mode);
        /* prompt which option was selected */
        switch (write_mode)
        {
        case 0:
            /* printf for documentation purposes only */
            printf("don't write a new image (equivalent to not specifying the --write option)\n");
            break;

        case 1:
            /* printf for documentation purposes only */
            printf("write a new image as a new file\n");
            break;

        case 2:
            /* printf for documentation purposes only */
            printf("write a new image that overwrites the input image file\n");
            break;

        case 3:
            /* printf for documentation purposes only */
            printf("write a new image that overwrites the input image file but backs up the original input image\n");
            break;

        default:
            /* print error message */
            printf("invalid write mode option. Get help: ./io_demo -?\n");
            /* program error exit code */
            /* 22 	EINVAL 	Invalid argument */
            return EINVAL;
        }
    }

    return 0;
}

void log_vitals()
{
    throw "not implemented";
}

int main(int argc, char **argv)
{
    int write_mode;
    // 0 - do not write new img
    // 1 - write new image
    // 2 - overwrite original image
    // 3 - overwrite but back up original
    string img_path;
    parse_options(argc, argv, &img_path, &write_mode);

    // try loading an image
    // height, width, number of components
    int width, height, channels;

    // use 0 to have stb figure out components per pixel
    uint8_t *img = stbi_load(img_path.c_str(), &width, &height, &channels, 0);

    if (img == nullptr)
    {
        printf("error loading image, reason: %s\n", stbi_failure_reason());
        exit(1);
    }

#ifdef DEBUG
    printf("loaded image of size w, h, c, %i %i %i\n", width, height, channels);
#endif

    // think if i really need this tbh
    // size of image in memory
    const int img_memory = sizeof(uint8_t) * width * height * channels;

    // TODO: error check, if !img etc
    // printf("image loaded\n");
    // luminosity based implementation - let's loop through every pixel
    // first allocate an output image...
    uint8_t *out_buffer = (uint8_t *)malloc(img_memory);

    // TODO: calculate cloud cover using luminosity for

    printf("%i\n bytes allocated", img_memory);
    // TODO: implement write mode checking :)
    // loop over the image...
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // memory offset from [0]
            // row major indexing
            int offset = (channels) * ((width * j) + i);
            int r_px = img[offset];
            int g_px = img[offset + 1];
            int b_px = img[offset + 2];
            // std::cout<<"R, G, B="<<r_px<<", "<<g_px<<", "<<b_px<<"\n";

            // if the pixel is greater than all the thresholds... [0-255]
            if (r_px >= R_THRESH || g_px >= G_THRESH || b_px >= B_THRESH)
            {
                // ...assign it white
                out_buffer[offset] = 255;
                out_buffer[offset + 1] = 255;
                out_buffer[offset + 2] = 255;
            }
            else
            {
                // ...otherwise assign it black
                out_buffer[offset] = 0;
                out_buffer[offset + 1] = 0;
                out_buffer[offset + 2] = 0;
            }
        }
    }

#ifdef DEBUG
    printf("writing image...\n");
#endif
    // todo: make it build a filename, lol
    stbi_write_png("test_out.png", width, height, channels, out_buffer, width * channels);

    // TFLITE STUFF here
    // Load model
    std::unique_ptr<tflite::FlatBufferModel>
        model =
            tflite::FlatBufferModel::BuildFromFile(MODELPATH);

    MINIMAL_CHECK(model != nullptr);

    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    std::unique_ptr<tflite::Interpreter> interpreter;
    builder(&interpreter);

    MINIMAL_CHECK(interpreter != nullptr);

    // Allocate tensor buffers.
    MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);
    printf("=== Pre-invoke Interpreter State ===\n");
    tflite::PrintInterpreterState(interpreter.get());

    // Fill input buffers
    // Note: The buffer of the input tensor with index `i` of type T can
    // be accessed with `T* input = interpreter->typed_input_tensor<T>(i);`
    // index refers to tensor ID, does not refer to size & shape of input

    // figure out how many patches are in the image...
    int pwidth_max = width / MODELPATCH;
    int pheight_max = height / MODELPATCH;

#ifdef DEBUG
    printf("width %i, height %i, wmax %i, hmax %i\n", width, height, pwidth_max, pheight_max);
    printf("\n\n=== Allocating images... ===\n");
#endif

    uint8_t *input = interpreter->typed_input_tensor<uint8_t>(0);

    for (int w = 0; w < pwidth_max; w++)
    {
        for (int h = 0; h < pheight_max; h++)
        {
            // j = height
            // i = width
            printf("write_mode %i\n", write_mode);

            for (int j = h * MODELPATCH; j < (h + 1) * MODELPATCH; j++)
            {
                for (int i = w * MODELPATCH; i < (w + 1) * MODELPATCH; i++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        // allocate the image
                        int offset = ((channels) * ((width * j) + i)) + k;
                        input[((channels) * ((MODELPATCH * j) + i)) + k] = img[offset];
                    }
                }
            }

#ifdef DEBUG
            printf("\n\n=== Images allocated ===\n");
            // Run inference
#endif
            MINIMAL_CHECK(interpreter->Invoke() == kTfLiteOk);
            printf("\n\n=== Post-invoke Interpreter State ===\n");
            tflite::PrintInterpreterState(interpreter.get());

            // Read output buffers
            // Note: The buffer of the output tensor with index `i` of type T can
            // be accessed with `T* output = interpreter->typed_output_tensor<T>(i);`

            uint8_t *unet_output = interpreter->typed_output_tensor<uint8_t>(0);

            // first build the extension name...

            string patch = std::to_string(w) + std::to_string(h);
            string ml_out_filename = build_image_output_filename(write_mode, img_path, ".png", patch);

#ifdef DEBUG
            printf("writing image out to %s...\n", ml_out_filename.c_str());
#endif

            if (write_mode > 0)
            {
                stbi_write_png(ml_out_filename.c_str(), MODELPATCH, MODELPATCH, channels, unet_output, MODELPATCH * channels);
            }
        }
    }

    // remember to free the image at the very end
    stbi_image_free(img);
    stbi_image_free(out_buffer);

    return 0;
}
