#include <stdio.h>
#include <iostream>
#include <string.h>     /* for string functions like strcmp */
#include <stdlib.h>     /* for casting function like atoi */
#include <errno.h>      /* standard linux error codes: https://www.thegeekstuff.com/2010/10/linux-error-codes/ */
#include <stdint.h>     /* for portability when dealing with integer data types */

// relevant STB headers
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

/* define convenience macros */
#define streq(s1,s2)    (!strcmp ((s1), (s2)))

using std::string;

int parse_options(int argc, char** argv, string* img_write_path, int* img_write_mode)
{
   /* get provider host and port from command arguments */
    int8_t argv_index_input = -1;
    int8_t argv_index_write = -1;

    // --------------------------------------------------------------------------
    // parse the command arguments

    int8_t argn;
    for (argn = 1; argn < argc; argn++)
    {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-?"))
        {
            printf("io_demo [options] ...");
            printf("\n  --input    / -i       the file path of the input image");
            printf("\n  --write    / -w       the write mode of the output image (optional)"
                   "\n\t0 - do not write a new image (equivalent to not specifying the --write option)"
                   "\n\t1 - write a new image as a new file"
                   "\n\t2 - write a new image that overwrites the input image file"
                   "\n\t3 - same as option 2 but backs up the original input image"
                  );
            printf("\n  --help     / -?       this information\n");
            
            /* program error exit code */
            /* 11 	EAGAIN 	Try again */
            return EAGAIN;
        }
        else
        if (streq (argv [argn], "--input")
        ||  streq (argv [argn], "-i"))
            argv_index_input = ++argn;
        else
        if (streq (argv [argn], "--write")
        ||  streq (argv [argn], "-w"))
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

    if(argv_index_input == -1)
    {
        /* print error message */
        printf("no image input path specified. Get help: ./io_demo -?\n");

        /* program error exit code */
        /* 22 	EINVAL 	Invalid argument */
        return EINVAL;
    }
    else{
        /* printf for documentation purposes only */
        printf("image to process: %s\n", argv[argv_index_input]);
        *img_write_path = string(argv[argv_index_input]);
    }


    // --------------------------------------------------------------------------
    // parse the output image write mode option, if given
    if(argv_index_write == -1)
    {
        /* printf for documentation purposes only */
        printf("no output image will be written\n");
    }
    else
    {
        /* parse write mode option string to int */
        int8_t write_mode = atoi(argv[argv_index_write]);

        // set to write mode
        // *img_write_mode = write_mode;
        printf("write mode %i\n", write_mode);
        /* prompt which option was selected */
        switch(write_mode)
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

int main(int argc, char **argv)
{
  int write_mode;
  string img_path;

  parse_options(argc, argv, &img_path, &write_mode);

  return 0;
  // try loading an image
}
    