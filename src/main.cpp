// #define DEBUG

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
// #include "tensorflow/lite/interpreter.h"
// #include "tensorflow/lite/kernels/register.h"
// #include "tensorflow/lite/model.h"

// #include <csv2/reader.hpp> // include csv2 for random forest
// #include <csv2/writer.hpp>
#include <vector>
#include <iterator>

// #define INPUT_CSV_FILENAME "ranger_input.csv"
// #define OUTPUT_CSV_FILENAME "ranger_output.csv"

// using namespace csv2;

#ifdef DEBUG
#include "tensorflow/lite/optional_debug_tools.h"
#endif

// custom headers
#include "constants.h"
#include "helper.h"

// TODO: add a labels.txt which is the classification labels

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

int parse_options(int argc, char **argv, string *img_write_path, int *img_write_mode, string *model_path)
{
    /* get provider host and port from command arguments */
    int8_t argv_index_input = -1;
    int8_t argv_index_write = -1;
    int8_t argv_index_model = -1;

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
                   "\n\t2 - write a new image that overwrites the input image file");
            printf("\n  --model    / -m       location of tflite model");
            printf("\n  --help     / -?       this information\n");

            /* program error exit code */
            /* 11 	EAGAIN 	Try again */
            return EAGAIN;
        }
        else if (streq(argv[argn], "--input") || streq(argv[argn], "-i"))
            argv_index_input = ++argn;
        else if (streq(argv[argn], "--write") || streq(argv[argn], "-w"))
            argv_index_write = ++argn;
        else if (streq(argv[argn], "--model") || streq(argv[argn], "-m"))
            argv_index_model = ++argn;
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
#ifdef DEBUG
        printf("image to process: %s\n", argv[argv_index_input]);
#endif
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
    }

    // parse the output image write mode option, if given
    if (argv_index_write == -1)
    {
        /* printf for documentation purposes only */
#ifdef DEBUG
        printf("no model given\n");
#endif
    }
    else
    {
        *model_path = string(argv[argv_index_model]);
    }

    return 0;
}

void print_smartcam_output(float cloud_coverage)
{
    // print output for smartcam
    // code stolen shamelessly from Georges

    /* mark which 100% confidence which label to apply to the image */
    uint8_t cloudy_0_25 = cloud_coverage >= 0 && cloud_coverage <= 0.25 ? 1 : 0;
    uint8_t cloudy_26_50 = cloud_coverage > 0.25 && cloud_coverage <= 0.50 ? 1 : 0;
    uint8_t cloudy_51_75 = cloud_coverage > 0.50 && cloud_coverage <= 0.75 ? 1 : 0;
    uint8_t cloudy_76_100 = cloud_coverage > 0.75 ? 1 : 0;

    /* create classification result json object */
    printf("{");
    printf("\"mit_cloudy_0_25\": %d, ", cloudy_0_25);
    printf("\"mit_cloudy_26_50\": %d, ", cloudy_26_50);
    printf("\"mit_cloudy_51_75\": %d, ", cloudy_51_75);
    printf("\"mit_cloudy_76_100\": %d, ", cloudy_76_100);
    printf("\"_cloud_coverage\": %f", cloud_coverage); /* prefixed by an underscore means it's metadata, not a label */
    printf("}");
}

// void write_img_to_csv(uint8_t *img, int width, int height, int channels, std::string csv_path)
// {
//     std::ofstream stream(csv_path);
//     Writer<delimiter<','>> writer(stream);

void white_balance(uint8_t *img, int width, int height, int channels)
{
    int r_hist[256] = {};
    int g_hist[256] = {};
    int b_hist[256] = {};

    // create histograms
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int offset = (channels) * ((width * j) + i);

            int r_px = img[offset];
            int g_px = img[offset + 1];
            int b_px = img[offset + 2];

            r_hist[r_px]++;
            g_hist[g_px]++;
            b_hist[b_px]++;
        }
    }

    // create 0.05% threshold
    int threshold = (int)(THRESH * width * height);

    // loop through histograms and search for 5% threshold
    // loop once per direction
    int r_l = -1;
    int g_l = -1;
    int b_l = -1;
    int r_h = -1;
    int g_h = -1;
    int b_h = -1;

    int r_suml = 0;
    int g_suml = 0;
    int b_suml = 0;
    int r_sumh = 0;
    int g_sumh = 0;
    int b_sumh = 0;

    for (int i = 0; i < 256; i++)
    {
        if (r_suml >= threshold && r_l == -1)
        {
            r_l = i;
        }

        if (g_suml >= threshold && g_l == -1)
        {
            g_l = i;
        }

        if (b_suml >= threshold && b_l == -1)
        {
            b_l = i;
        }

        if (r_sumh >= threshold && r_h == -1)
        {
            r_h = 255 - i;
        }

        if (g_sumh >= threshold && g_h == -1)
        {
            g_h = 255 - i;
        }

        if (b_sumh >= threshold && b_h == -1)
        {
            b_h = 255 - i;
        }

        r_suml += r_hist[i];
        g_suml += g_hist[i];
        b_suml += b_hist[i];

        r_sumh += r_hist[255 - i];
        g_sumh += g_hist[255 - i];
        b_sumh += b_hist[255 - i];

        if (r_l > -1 && g_l > -1 && b_l > -1 && r_h > -1 && g_h > -1 && b_h > -1)
        {
            break;
        }
    }

    // now generate LUTs from the higher and upper thresholds...
    // first allocate the luts
    int r_lut[256] = {};
    int g_lut[256] = {};
    int b_lut[256] = {};

    for (int i = 0; i < 256; i++)
    {
        if (i <= r_l)
        {
            r_lut[i] = 0;
        }
        else if (i >= r_h)
        {
            r_lut[i] = 255;
        }
        else
        {
            r_lut[i] = (int)(255 * (i - r_l) / ((float)(r_h - r_l)));
        }

        if (i <= g_l)
        {
            g_lut[i] = 0;
        }
        else if (i >= g_h)
        {
            g_lut[i] = 255;
        }
        else
        {
            g_lut[i] = (int)(255 * (i - g_l) / ((float)(g_h - g_l)));
        }

        if (i <= b_l)
        {
            b_lut[i] = 0;
        }
        else if (i >= b_h)
        {
            b_lut[i] = 255;
        }
        else
        {
            b_lut[i] = (int)(255 * (i - b_l) / ((float)(b_h - b_l)));
        }
    }

    // now finally, apply the LUT to the image
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // memory offset from [0]
            // row major indexing
            int offset = (channels) * ((width * j) + i);

            img[offset] = r_lut[img[offset]];
            img[offset + 1] = g_lut[img[offset + 1]];
            img[offset + 2] = b_lut[img[offset + 2]];
        }
    }

    // done!
}

int main(int argc, char **argv)
{
    int write_mode;
    // 0 - do not write new img
    // 1 - write new image
    // 2 - overwrite original image
    string img_path;
    string model_path;
    parse_options(argc, argv, &img_path, &write_mode, &model_path);

    // try loading an image
    // height, width, number of components
    int width, height, channels;

    // use 0 to have stb figure out components per pixel
    // TODO: check if img path is empty first
    uint8_t *img;

    // if (!img_path.empty())
    // {
    img = stbi_load(img_path.c_str(), &width, &height, &channels, 0);
    // }

    if (img == nullptr)
    {
        printf("error loading image, reason: %s\n", stbi_failure_reason());
        exit(1);
    }

    white_balance(img, width, height, channels);

#ifdef DEBUG
    printf("loaded image of size w, h, c, %i %i %i\n", width, height, channels);
#endif

    // think if i really need this tbh
    // size of image in memory
    const int img_memory = sizeof(uint8_t) * width * height * channels;

    // luminosity based implementation - let's loop through every pixel
    // first allocate an output image...
    uint8_t *out_buffer = (uint8_t *)malloc(img_memory);

    // TODO: calculate cloud cover using luminosity for return
#ifdef DEBUG
    printf("%i bytes allocated\n", img_memory);
#endif
    // loop over the image...
    int cloudy_px = 0;
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
                cloudy_px++;
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

    float cloud_coverage = cloudy_px / ((double)width * (double)height);

#ifdef DEBUG
    printf("writing image...\n");
#endif
    // todo: make it build a filename, lol
    stbi_write_png(build_image_output_filename(write_mode, img_path, ".png", "lum").c_str(), width, height, channels, out_buffer, width * channels);
    stbi_image_free(out_buffer);

    // TFLITE STUFF here
    // Load model
    std::unique_ptr<tflite::FlatBufferModel>
        model =
            tflite::FlatBufferModel::BuildFromFile(model_path.c_str());

    MINIMAL_CHECK(model != nullptr);

    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    std::unique_ptr<tflite::Interpreter> interpreter;
    builder(&interpreter);

    MINIMAL_CHECK(interpreter != nullptr);

    // Allocate tensor buffers.
    MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);

#ifdef DEBUG
    printf("=== Pre-invoke Interpreter State ===\n");
    tflite::PrintInterpreterState(interpreter.get());
#endif
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
            // first allocate the tensor input buffer
            // j = height
            // i = width
            for (int j = h * MODELPATCH; j < (h + 1) * MODELPATCH; j++)
            {
                for (int i = w * MODELPATCH; i < (w + 1) * MODELPATCH; i++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        // printf("j %i\n", j);
                        // allocate the image
                        int offset = (((channels) * ((width * j) + i)) + k);
                        // move i, j back into nominal coordinates
                        input[((channels) * ((MODELPATCH * (j - h * MODELPATCH)) + (i - w * MODELPATCH))) + k] = img[offset];
                    }
                }
            }

#ifdef DEBUG
            printf("\n\n=== Images allocated ===\n");
            // Run inference
#endif
            MINIMAL_CHECK(interpreter->Invoke() == kTfLiteOk);

#ifdef DEBUG
            printf("\n\n=== Post-invoke Interpreter State ===\n");
            tflite::PrintInterpreterState(interpreter.get());
#endif

            // Read output buffers
            // Note: The buffer of the output tensor with index `i` of type T can
            // be accessed with `T* output = interpreter->typed_output_tensor<T>(i);`

            uint8_t *unet_output = interpreter->typed_output_tensor<uint8_t>(0);
            string patch = "ml_" + std::to_string(w) + std::to_string(h);
            string ml_out_filename = build_image_output_filename(write_mode, img_path, ".png", patch);

#ifdef DEBUG
            printf("writing image out to %s...\n", ml_out_filename.c_str());
#endif

            // only 1 channel output!
            if (write_mode > 0)
            {
                stbi_write_png(ml_out_filename.c_str(), MODELPATCH, MODELPATCH, 1, unet_output, MODELPATCH * 1);
            }
        }
    }

    // remember to free the image at the very end
    stbi_image_free(img);

    print_smartcam_output(cloud_coverage);
    return 0;
}
