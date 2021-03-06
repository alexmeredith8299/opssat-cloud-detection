#include <stdio.h>
#include <string.h>     /* for string functions like strcmp */
#include <stdlib.h>     /* for casting function like atoi */
#include <errno.h>      /* standard linux error codes: https://www.thegeekstuff.com/2010/10/linux-error-codes/ */
#include <stdint.h>     /* for portability when dealing with integer data types */

/* define convenience macros */
#define streq(s1,s2)    (!strcmp ((s1), (s2)))

int main(int argc, char *argv [])
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
                printf("invale write mode option. Get help: ./io_demo -?\n");

                /* program error exit code */
                /* 22 	EINVAL 	Invalid argument */
                return EINVAL;
        }
        
    }




    // --------------------------------------------------------------------------
    // run your logic

    /* here be yer app code */




    // --------------------------------------------------------------------------
    // stdout the classification results as a JSON object string


    /** 
     * the stdout is a JSON object string that is parsed by the SmartCam (jank, but it works)
     * each key-value pair represents <label, prediction_confidence>
     * come up with the labels you want to use, e.g.:
     * 
     *  {
     *      cloudy_0_25: 0.30,          // 30% prediction confidence that the image is 0-25% cloudy
     *      cloudy_26_50: 0.50,         // 50% prediction confidence that the image is 26-50% cloudy
     *      cloudy_51_75: 0.11,         // 11% prediction confidence that the image is 51-75% cloudy 
     *      cloudy_76_100: 0.09,        // 9% prediction confidence that the image is 76-100% cloudy
     *      _metadata_prop1: 10,        // a peroperty prefixed by an underscore means it's metadata, not a label
     *      _metadata_prop3: 20"        // a peroperty prefixed by an underscore means it's metadata, not a label
     *      
     *  }
     * 
     * for some algorithms the exact cloudiness percentage can be calculated via pixel-by-pixel scan of the segmented image
     * consider a case in which we know that an image is 35.492% cloudy, the JSON woud look like this:
     * 
     *  {
     *      cloudy_0_25: 0,
     *      cloudy_26_50: 1,
     *      cloudy_51_75: 0,
     *      cloudy_76_100: 0,
     *      _metadata_cloud_coverage: 0.35492
     *  }
     * 
     * 
     * in the following example the string is hardcoded but in practice it would have to be built
     * example on how a JSON string is built fin the SmartCam's image classifier: 
     * https://github.com/georgeslabreche/kmeans-image-segmentation/blob/a4bcd2efb695305c8e9a1942b6bd31e87586e632/src/main.cpp#L436-L453
     */
    printf("{"
        "cloudy_0_25: 0.30, "
        "cloudy_26_50: 0.50, "
        "cloudy_51_75: 0.11, "
        "couldy_76_100: 0.09, "
        "_metadata_prop1: 10, "
        "_metadata_prop3: 20"
    "}");


    // --------------------------------------------------------------------------
    // program success, no errors 

    /**
     * IMPORTANT: Make sure the JSON object string is the only printf output of your program's happy path
     * this is because the SmartCam will expect the entire stdout to only be the JSON object string
     * this reference program has a bunch of printf for documentation purposes only
     * an actual program would have no such printf outputs
     */

    /* this tells the SmartCam that this program's stdout is OK to parse as a JSON object string */
    return 0;
}