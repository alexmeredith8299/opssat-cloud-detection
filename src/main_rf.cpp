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
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

// custom headers
#include "constants.h"

//include csv2 for random forest
#include <vector>
#include <iterator>
#include <csv2/writer.hpp>
#include <csv2/reader.hpp>

#define INPUT_CSV_FILENAME  "ranger_input.csv"

using namespace csv2;

/* define convenience macros */
#define streq(s1, s2) (!strcmp((s1), (s2)))

using std::string;
// using cstd

int parse_options(int argc, char **argv, string *img_input_path, int *file_mode, string *img_output_path)
{
    /* get provider host and port from command arguments */
    int8_t argv_index_input = -1;
    int8_t argv_index_mode = -1;
    int8_t argv_index_output = -1;

    // --------------------------------------------------------------------------
    // parse the command arguments

    int8_t argn;
    for (argn = 1; argn < argc; argn++)
    {
        if (streq(argv[argn], "--help") || streq(argv[argn], "-?"))
        {
            printf("opssat_rf_segment [options] ...");
            printf("\n  --input    / -i       the file path of the input image or csv");
            printf("\n  --mode    / -m     the file conversion mode (img -> csv or csv -> img)"
                   "\n\t1 - image to csv (equivalent to not specifying the --write option)"
                   "\n\t2 - csv to image");
            printf("\n  --output   / -o   the output file name (optional)");
            printf("\n  --help     / -?       this information\n");

            /* program error exit code */
            /* 11 	EAGAIN 	Try again */
            return EAGAIN;
        }
        else if (streq(argv[argn], "--input") || streq(argv[argn], "-i"))
            argv_index_input = ++argn;
        else if (streq(argv[argn], "--mode") || streq(argv[argn], "-m"))
            argv_index_mode = ++argn;
        else if (streq(argv[argn], "--output") || streq(argv[argn], "-o"))
            argv_index_output = ++argn;
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
        printf("no image input path specified. Get help: opssat_rf_segment --help?\n");

        /* program error exit code */
        /* 22 	EINVAL 	Invalid argument */
        return EINVAL;
    }
    else
    {
        /* printf for documentation purposes only */
        printf("image to process: %s\n", argv[argv_index_input]);
        *img_input_path = string(argv[argv_index_input]);
    }

    if(argv_index_mode == -1)
    {
      /* print error message */
      printf("no image mode specified. Get help: opssat_rf_segment --help?\n");

      /* program error exit code */
      /* 22 	EINVAL 	Invalid argument */
      return EINVAL;
    }
    else
    {
      /* parse write mode option string to int */
      int8_t mode = atoi(argv[argv_index_mode]);

      // set to write mode
      // *img_write_mode = write_mode;
      printf("file conversion mode %i\n", mode);
      /* prompt which option was selected */
      switch (mode)
      {
      case 1:
          /* printf for documentation purposes only */
          *file_mode = mode;
          printf("image to csv \n");
          break;

      case 2:
          /* printf for documentation purposes only */
          *file_mode = mode;
          printf("csv to image \n");
          break;
      default:
          /* print error message */
          printf("invalid  mode option. Get help: opssat_rf_segment --help?\n");
          /* program error exit code */
          /* 22 	EINVAL 	Invalid argument */
          return EINVAL;
      }
    }


    // --------------------------------------------------------------------------
    // parse the output image write mode option, if given
    if (argv_index_output == -1)
    {
        /* printf for documentation purposes only */
        printf("no output filename specified, using default (out.csv or out.png)\n");
    }
    else
    {
        /* printf for documentation purposes only */
        printf("file output path: %s\n", argv[argv_index_output]);
        *img_output_path = string(argv[argv_index_output]);
    }

    return 0;
}

void log_vitals()
{
    throw "not implemented";
}

void write_img_to_csv(std::string img_path, std::string csv_path)
{
  int width, height, channels;
  // use 0 to have stb figure out components per pixel
  uint8_t *img = stbi_load(img_path.c_str(), &width, &height, &channels, 0);

  if (img == NULL)
  {
      printf("error loading image, reason: %s\n", stbi_failure_reason());
      exit(1);
  }

  printf("loaded image of size w, h, c, %i %i %i\n", width, height, channels);
  // size of image in memory
  int img_memory = sizeof(uint8_t) * width * height * channels;

  // TODO: error check, if !img etc
  // printf("image loaded\n");
  // luminosity based implementation - let's loop through every pixel
  // first allocate an output image...
  uint8_t *luminosity_out = (uint8_t *)malloc(img_memory);
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
          //std::cout<<"R, G, B="<<r_px<<", "<<g_px<<", "<<b_px<<"\n";

          if (r_px >= R_THRESH || g_px >= G_THRESH || b_px >= B_THRESH)
          {
              luminosity_out[offset] = 255;
              luminosity_out[offset + 1] = 255;
              luminosity_out[offset + 2] = 255;
          }
          else
          {
              luminosity_out[offset] = 0;
              luminosity_out[offset + 1] = 0;
              luminosity_out[offset + 2] = 0;
          }
      }
  }



  std::ofstream stream(csv_path);
    Writer<delimiter<','>> writer(stream);

    /*std::vector<std::vector<std::string>> rows =
        {
            {"a", "b", "c"},
            {"1", "2", "3"},
            {"4", "5", "6"}
        };*/
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> headers;
    for(int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        std::string red = "R";
        std::string blue = "B";
        std::string green = "G";
        std::string r_result, b_result, g_result;

        r_result = red + std::to_string(i) +std::to_string(j);
        g_result = green + std::to_string(i) +std::to_string(j);
        b_result = blue + std::to_string(i) +std::to_string(j);

        headers.push_back(r_result);
        headers.push_back(g_result);
        headers.push_back(b_result);
      }
    }
    headers.push_back("CLOUD");
    rows.push_back(headers);

    //RBF: used so our csv isn't obsecenly giant
    int i_start = width-200;
    int j_start = height-200;

    for (int i = i_start; i < width; i++)
    {
        for (int j = j_start; j < height; j++)
        {
            // memory offset from [0]
            // row major indexing
            std::vector<std::string> row;

            int offset = (channels) * ((width * j) + i);
            int r_22 = img[offset];
            int g_22 = img[offset + 1];
            int b_22 = img[offset + 2];
            std::cout<<"R, G, B="<<r_22<<", "<<g_22<<", "<<b_22<<"\n";
            std::cout<<"Offset "<<offset<<"\n";

            int jm = j-1;
            int jp = j+1;
            int im = i-1;
            int ip = i+1;
            if(i==0)
            {
              im = i;
            }
            if(i==width-1)
            {
              ip = i;
            }
            if(j==0)
            {
              jm = j;
            }
            if(j==height-1)
            {
              jp = j;
            }

            int offset_11 = (channels) * ((width * jm) + im);

            int r_11 = img[offset_11];
            int g_11 = img[offset_11 + 1];
            int b_11 = img[offset_11 + 2];

            row.push_back(std::to_string(r_11));
            row.push_back(std::to_string(g_11));
            row.push_back(std::to_string(b_11));


            int offset_12 = (channels) * ((width * j) + im);

            int r_12 = img[offset_12];
            int g_12 = img[offset_12 + 1];
            int b_12 = img[offset_12 + 2];

            row.push_back(std::to_string(r_12));
            row.push_back(std::to_string(g_12));
            row.push_back(std::to_string(b_12));

            int offset_13 = (channels) * ((width * jp) + im);

            int r_13 = img[offset_13];
            int g_13 = img[offset_13 + 1];
            int b_13 = img[offset_13 + 2];

            row.push_back(std::to_string(r_13));
            row.push_back(std::to_string(g_13));
            row.push_back(std::to_string(b_13));

            int offset_21 = (channels) * ((width * jm) +i);

            int r_21 = img[offset_21];
            int g_21 = img[offset_21 + 1];
            int b_21 = img[offset_21 + 2];

            row.push_back(std::to_string(r_21));
            row.push_back(std::to_string(g_21));
            row.push_back(std::to_string(b_21));

            row.push_back(std::to_string(r_22));
            row.push_back(std::to_string(g_22));
            row.push_back(std::to_string(b_22));

            int offset_23 = (channels) * ((width * jp) + i);

            int r_23 = img[offset_23];
            int g_23 = img[offset_23 + 1];
            int b_23 = img[offset_23 + 2];

            row.push_back(std::to_string(r_23));
            row.push_back(std::to_string(g_23));
            row.push_back(std::to_string(b_23));

            int offset_31 = (channels) * ((width * jm) + ip);

            int r_31 = img[offset_31];
            int g_31 = img[offset_31 + 1];
            int b_31 = img[offset_31 + 2];

            row.push_back(std::to_string(r_31));
            row.push_back(std::to_string(g_31));
            row.push_back(std::to_string(b_31));

            int offset_32 = (channels) * ((width * j) + ip);

            int r_32 = img[offset_32];
            int g_32 = img[offset_32 + 1];
            int b_32 = img[offset_32 + 2];

            row.push_back(std::to_string(r_32));
            row.push_back(std::to_string(g_32));
            row.push_back(std::to_string(b_32));

            int offset_33 = (channels) * ((width * jp) + ip);

            int r_33 = img[offset_33];
            int g_33 = img[offset_33 + 1];
            int b_33 = img[offset_33 + 2];

            row.push_back(std::to_string(r_33));
            row.push_back(std::to_string(g_33));
            row.push_back(std::to_string(b_33));

            row.push_back(std::to_string(1)); //placeholder for cloud status

            rows.push_back(row);

        }
    }

    writer.write_rows(rows);
    stream.close();

    //printf("writing image...\n");
    //stbi_write_png("yee.png", width, height, channels, luminosity_out, width * channels);

    // remember to free the image at the very end
    stbi_image_free(img);
    stbi_image_free(luminosity_out);
}

void write_csv_to_img(std::string csv_path, std::string img_path)
{
  csv2::Reader<delimiter<','>,
               quote_character<'"'>,
               first_row_is_header<true>,
               trim_policy::trim_whitespace> csv;

  if (csv.mmap(csv_path)) {
    const auto header = csv.header();
    for( const auto cell: header) {
      std::string val;
      cell.read_value(val);
      std::cout<<val<<"\n";
    }
    int height = 200;
    int width = 200;
    int channels = 3; //R, G, B

    int img_memory = sizeof(uint8_t) * width * height * channels;
    // TODO: error check, if !img etc
    // printf("image loaded\n");
    // luminosity based implementation - let's loop through every pixel
    // first allocate an output image...
    uint8_t *cloud_mask_out = (uint8_t *)malloc(img_memory);
    printf("%i\n bytes allocated", img_memory);

    int csv_row= 0;
    size_t nrows = csv.rows();
    std::cout<<"nrows="<<nrows;
    for (const auto row: csv) {
      for (const auto cell: row) {
        //Read cell value
        std::string val;
        cell.read_value(val);
        if (csv_row >= 2) {
          //int offset = (channels) * ((width * j) + i);
          //int r_px = img[offset];
          //int g_px = img[offset + 1];
          //int b_px = img[offset + 2];

          int idx = channels*(csv_row-2);
          std::cout<<"idx, val, ht="<<idx<<", "<<idx<<", "<<csv_row<<"\n";

          if (val == "1")
          {
            cloud_mask_out[idx] = 255;
            cloud_mask_out[idx+1] = 255;
            cloud_mask_out[idx+2] = 255;
          } else
          {
            cloud_mask_out[idx] = 0;
            cloud_mask_out[idx+1] = 0;
            cloud_mask_out[idx+2] = 0;
          }
        }
      }
      csv_row++;
    }
    std::cout<<"w, h = "<<width<<","<<height;
    printf("writing image...\n");
    const char* img_outpath = img_path.c_str();
    stbi_write_png(img_outpath, width, height, channels, cloud_mask_out, width * channels);

    // remember to free the image at the very end
    stbi_image_free(cloud_mask_out);
  }
}

int main(int argc, char **argv)
{
    int mode;
    // 0 - img to csv
    // 1 - csv to img
    string input_path;
    string output_path;

    parse_options(argc, argv, &input_path, &mode, &output_path);

    std::cout<<"here\n";
    std::cout<<"mode"<<mode<<"\n";

    if(input_path.length() != 0) {
      if(mode == 1) {
        if(output_path.length() == 0) {
          write_img_to_csv(input_path, "out.csv");
        }
        else {
          write_img_to_csv(input_path, output_path);
        }
      } else if(mode == 2) {
        std::cout<<"here\n";
        if(output_path.length() == 0) {
          write_csv_to_img(input_path, "out.png");
        }
        else {
          write_csv_to_img(input_path, output_path);
        }
      }
    }
}
