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
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

// #include <csv2/reader.hpp> // include csv2 for random forest
// #include <csv2/writer.hpp>
#include <vector>
#include <iterator>

#define INPUT_CSV_FILENAME "ranger_input.csv"
#define OUTPUT_CSV_FILENAME "ranger_output.csv"

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

void log_vitals()
{
    throw "not implemented";
}

// void write_img_to_csv(uint8_t *img, int width, int height, int channels, std::string csv_path)
// {
//     std::ofstream stream(csv_path);
//     Writer<delimiter<','>> writer(stream);

//     /*std::vector<std::vector<std::string>> rows =
//         {
//             {"a", "b", "c"},
//             {"1", "2", "3"},
//             {"4", "5", "6"}
//         };*/
//     std::vector<std::vector<std::string>> rows;
//     std::vector<std::string> headers;
//     for (int i = 0; i < 3; i++)
//     {
//         for (int j = 0; j < 3; j++)
//         {
//             std::string red = "R";
//             std::string blue = "B";
//             std::string green = "G";
//             std::string r_result, b_result, g_result;

//             r_result = red + std::to_string(i) + std::to_string(j);
//             g_result = green + std::to_string(i) + std::to_string(j);
//             b_result = blue + std::to_string(i) + std::to_string(j);

//             headers.push_back(r_result);
//             headers.push_back(g_result);
//             headers.push_back(b_result);
//         }
//     }
//     headers.push_back("CLOUD");
//     rows.push_back(headers);

//     // Placeholder: center crop to 256 x 256
//     int i_start = 0;    // std::max(0, (int)(ceil(width/2)-128));
//     int j_start = 0;    // std::max(0, (int)(ceil(height/2)-128));
//     int i_end = width;  // std::min(width, (int)(ceil(width/2)+128));
//     int j_end = height; // std::min(height, (int)(ceil(height/2)+128));

//     for (int i = i_start; i < i_end; i++)
//     {
//         for (int j = j_start; j < j_end; j++)
//         {
//             // memory offset from [0]
//             // row major indexing
//             std::vector<std::string> row;

//             int offset = (channels) * ((width * j) + i);
//             int r_22 = img[offset];
//             int g_22 = img[offset + 1];
//             int b_22 = img[offset + 2];
//             // std::cout<<"R, G, B="<<r_22<<", "<<g_22<<", "<<b_22<<"\n";
//             // std::cout<<"Offset "<<offset<<"\n";

//             int jm = j - 1;
//             int jp = j + 1;
//             int im = i - 1;
//             int ip = i + 1;
//             if (i == 0)
//             {
//                 im = i;
//             }
//             if (i == width - 1)
//             {
//                 ip = i;
//             }
//             if (j == 0)
//             {
//                 jm = j;
//             }
//             if (j == height - 1)
//             {
//                 jp = j;
//             }

//             int offset_11 = (channels) * ((width * jm) + im);

//             int r_11 = img[offset_11];
//             int g_11 = img[offset_11 + 1];
//             int b_11 = img[offset_11 + 2];

//             row.push_back(std::to_string(r_11));
//             row.push_back(std::to_string(g_11));
//             row.push_back(std::to_string(b_11));

//             int offset_12 = (channels) * ((width * j) + im);

//             int r_12 = img[offset_12];
//             int g_12 = img[offset_12 + 1];
//             int b_12 = img[offset_12 + 2];

//             row.push_back(std::to_string(r_12));
//             row.push_back(std::to_string(g_12));
//             row.push_back(std::to_string(b_12));

//             int offset_13 = (channels) * ((width * jp) + im);

//             int r_13 = img[offset_13];
//             int g_13 = img[offset_13 + 1];
//             int b_13 = img[offset_13 + 2];

//             row.push_back(std::to_string(r_13));
//             row.push_back(std::to_string(g_13));
//             row.push_back(std::to_string(b_13));

//             int offset_21 = (channels) * ((width * jm) + i);

//             int r_21 = img[offset_21];
//             int g_21 = img[offset_21 + 1];
//             int b_21 = img[offset_21 + 2];

//             row.push_back(std::to_string(r_21));
//             row.push_back(std::to_string(g_21));
//             row.push_back(std::to_string(b_21));

//             row.push_back(std::to_string(r_22));
//             row.push_back(std::to_string(g_22));
//             row.push_back(std::to_string(b_22));

//             int offset_23 = (channels) * ((width * jp) + i);

//             int r_23 = img[offset_23];
//             int g_23 = img[offset_23 + 1];
//             int b_23 = img[offset_23 + 2];

//             row.push_back(std::to_string(r_23));
//             row.push_back(std::to_string(g_23));
//             row.push_back(std::to_string(b_23));

//             int offset_31 = (channels) * ((width * jm) + ip);

//             int r_31 = img[offset_31];
//             int g_31 = img[offset_31 + 1];
//             int b_31 = img[offset_31 + 2];

//             row.push_back(std::to_string(r_31));
//             row.push_back(std::to_string(g_31));
//             row.push_back(std::to_string(b_31));

//             int offset_32 = (channels) * ((width * j) + ip);

//             int r_32 = img[offset_32];
//             int g_32 = img[offset_32 + 1];
//             int b_32 = img[offset_32 + 2];

//             row.push_back(std::to_string(r_32));
//             row.push_back(std::to_string(g_32));
//             row.push_back(std::to_string(b_32));

//             int offset_33 = (channels) * ((width * jp) + ip);

//             int r_33 = img[offset_33];
//             int g_33 = img[offset_33 + 1];
//             int b_33 = img[offset_33 + 2];

//             row.push_back(std::to_string(r_33));
//             row.push_back(std::to_string(g_33));
//             row.push_back(std::to_string(b_33));

//             row.push_back(std::to_string(1)); // placeholder for cloud status

//             rows.push_back(row);
//         }
//     }

//     writer.write_rows(rows);
//     stream.close();
// }

// void write_csv_to_img(std::string csv_path, std::string img_path)
// {
//     csv2::Reader<delimiter<','>,
//                  quote_character<'"'>,
//                  first_row_is_header<true>,
//                  trim_policy::trim_whitespace>
//         csv;

//     if (csv.mmap(csv_path))
//     {
//         const auto header = csv.header();
//         for (const auto cell : header)
//         {
//             std::string val;
//             cell.read_value(val);
//             // std::cout<<val<<"\n";
//         }
//         // int height = 200;
//         // int width = 200;
//         int channels = 3; // R, G, B

//         size_t nrows = csv.rows();
//         // std::cout<<"nrows="<<nrows<<"\n";
//         int img_memory = sizeof(uint8_t) * nrows * channels;
//         // TODO: error check, if !img etc
//         // printf("image loaded\n");
//         // luminosity based implementation - let's loop through every pixel
//         // first allocate an output image...
//         uint8_t *cloud_mask_out = (uint8_t *)malloc(img_memory);
//         // printf("%i\n bytes allocated", img_memory);

//         int csv_row = 0;
//         for (const auto row : csv)
//         {
//             for (const auto cell : row)
//             {
//                 // Read cell value
//                 std::string val;
//                 cell.read_value(val);
//                 if (csv_row >= 2)
//                 {
//                     // int offset = (channels) * ((width * j) + i);
//                     // int r_px = img[offset];
//                     // int g_px = img[offset + 1];
//                     // int b_px = img[offset + 2];

//                     int idx = channels * (csv_row - 2);
//                     // std::cout<<"idx, val, ht="<<idx<<", "<<idx<<", "<<csv_row<<"\n";

//                     if (val == "1")
//                     {
//                         cloud_mask_out[idx] = 255;
//                         cloud_mask_out[idx + 1] = 255;
//                         cloud_mask_out[idx + 2] = 255;
//                     }
//                     else
//                     {
//                         cloud_mask_out[idx] = 0;
//                         cloud_mask_out[idx + 1] = 0;
//                         cloud_mask_out[idx + 2] = 0;
//                     }
//                 }
//             }
//             csv_row++;
//         }
//         // Assume square.
//         int width = (int)(pow(nrows, 0.5));
//         int height = (int)(pow(nrows, 0.5));
//         // printf("writing image...\n");
//         const char *img_outpath = img_path.c_str();
//         stbi_write_png(img_outpath, width, height, channels, cloud_mask_out, width * channels);

//         // remember to free the image at the very end
//         stbi_image_free(cloud_mask_out);
//     }
// }

void white_balance(uint8_t *img, int width, int height, int channels)
{
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

            r_px = R_LUT[r_px];
            g_px = G_LUT[r_px];
            b_px = B_LUT[r_px];
        }
    }
}

int main(int argc, char **argv)
{
    int write_mode;
    // 0 - do not write new img
    // 1 - write new image
    // 2 - overwrite original image
    string img_path;
    parse_options(argc, argv, &img_path, &write_mode);

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

    // RANGER STUFF HERE
    // img -> csv
    // write_img_to_csv(img, width, height, channels, INPUT_CSV_FILENAME);
    // // train
    // system("./include/ranger/cpp_version/ranger --file ranger_input.csv --depvarname CLOUD --treetype 1 --ntree 10 --write");
    // // test
    // system("./include/ranger/cpp_version/ranger --file ranger_input.csv --predict ranger_out.forest");
    // system("mv ranger_out.prediction ranger_out.csv");
    // // write img
    // std::string rf_outpath = build_image_output_filename(write_mode, img_path, ".png", "rf");
    // write_csv_to_img("ranger_out.csv", rf_outpath);

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

            // first build the extension name...

            string patch = "ml_" + std::to_string(w) + std::to_string(h);
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

    print_smartcam_output(cloud_coverage);
    return 0;
}
