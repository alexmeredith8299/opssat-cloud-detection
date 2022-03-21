#include <string.h> /* for string functions like strcmp */
#include <stdio.h>
using std::string;
// --------------------------------------------------------------------------
// build file name output string (the file name of the output image that will be written)

// todo: convert this to c++ strings. probably better
string build_image_output_filename(const int write_mode, string inimg_filename, string image_file_ext, string patch)
{
    switch (write_mode)
    {
    case 1: /* write a new image as a new file */
        /* create new file name for the output image file */
        string outimg_filename = inimg_filename.substr(0, inimg_filename.find_last_of(".")) ;
        outimg_filename.append(".segmented.");
        outimg_filename.append(patch);
        outimg_filename.append(image_file_ext);
        return outimg_filename;
        break;

    case 2: /* write a new image that overwrites the input image file */

        /* use existing input image file name as the the output image file name */
        return inimg_filename;
        break;
    // TODO: implement this with strings (god bless C++)
    // case 3: /*  write a new image that overwrites the input image file but back up the original input image */

    //     char inimg_filename_new[100] = {0};
    //     strncpy(inimg_filename_new, inimg_filename, strcspn(inimg_filename, "."));
    //     strcat(inimg_filename_new, ".original.");
    //     strcat(inimg_filename_new, image_file_ext);
    //     rename(inimg_filename, inimg_filename_new);

    //     /* use existing input image file name as the output image file name */
    //     strcpy(outimg_filename, inimg_filename);

    //     break;
    }

    /* success */
    return 0;
}