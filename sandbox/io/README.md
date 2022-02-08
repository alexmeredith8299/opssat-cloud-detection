# i/o demo
A reference program with input and output expected by the [OPS-SAT SmartCam](https://github.com/georgeslabreche/opssat-smartcam) classification pipeline.

## Compile and run the program

For local development:
```bash
make
./io_demo -?
./io_demo -i img_msec_1604269262792_2.png
```

For the spacecraft:
```bash
make TARGET=arm
./io_demo -?
./io_demo -i img_msec_1604269262792_2_thumbnail.jpeg
```

## SmartCam classification pipeline configuration
- Details documented [here](https://github.com/georgeslabreche/opssat-smartcam#33-building-an-image-classification-pipeline).
- Example [here](https://github.com/georgeslabreche/opssat-smartcam/blob/78eede8dee44b2ebde1d7276271983c70cce773f/home/exp1000/config.ini#L33-L41) for TensorFlow Lite classification configuration.

Configuring how the program is plugged into the SmartCam's classification pipeline is achieved via the SmartCam's config file, e.g.:

```conf
[model_my_exec_bin]
my_exec_bin.bin_model                   = /home/exp185/cloud_detection
my_exec_bin.labels                      = /home/exp185/labels.txt
my_exec_bin.labels_keep                 = ["cloudy_0_25","cloudy_26_50"]
my_exec_bin.input_format                = jpeg
my_exec_bin.write_mode                  = 1
my_exec_bin.args                        = -a 1 -b 2 -c 3
kmeans_imgseg.confidence_threshold      = 0.70
```

Meaning:
- **bin_model** is the path to the executable binary (make sure to chmod +x the binary file to make it executable).
- **labels** is the path of the labels text file that lists all possible labels that can be applied to an image, exemple [here](https://github.com/georgeslabreche/opssat-smartcam/blob/78eede8dee44b2ebde1d7276271983c70cce773f/home/exp1000/models/default/labels.txt).
- **labels_keep** are the classified images that should be kept. If an image is classified with a label not listed in this property then it will be discarded from downlink (or not fed to the next model or program in the classification pipeline).
- **input_format** is the image type to feed yhe program as the image input. The SmartCam's application directory will have 3 different formats of the same image:
    - *ims_rgb* is a proprietary raw format generated be the onboard camera (2048 x 1944).
    - *png* is the high resolution png image generated from the ims_rgb (2048 x 1944).
    - *jpeg* is the low resolution thunmbnail image generated from the ims_rgb (614 x 583).
- **write mode** is optional and can be one of the following values:
    - 0: do not write a new image (equivalent to not specifying the --write option).
    - 1: write a new image as a new file.
    - 2: write a new image that overwrites the input image file.
    - 3: same as option 2 but backs up the original input image.
- **confidence_threshold** is the minimum confidence threshold that a classification must achieve to keep the image (otherwise it will be discarded). 

## Program inputs
Refer to `./io_demo -?`:

```bash
io_demo [options] ...
  --input    / -i       the file path of the input image
  --write    / -w       the write mode of the output image (optional)
        0 - do not write a new image (equivalent to not specifying the --write option)
        1 - write a new image as a new file
        2 - write a new image that overwrites the input image file
        3 - same as option 2 but backs up the original input image
  --help     / -?       this information
```

SmartCam integration:
- the **--input** option is set by the `input_format` property in the SmartCam's config file.
- the **--write** option is set the `write_mode` property in the SmartCam's config file.
- additional options defined by `args` property in the SmartCam's config file need to be explicitely supported and parsed by the program.

## Other configurations
Experimenters are free to create their own configuration file to be read and parsed by their program. This config file would simply reside in the same experiment folder as their program's executable binary, e.g.: /home/exp185/config.ini.