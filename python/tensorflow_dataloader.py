import numpy as np
import matplotlib.pyplot as plt
import os
import random
import math
import pickle
from PIL import Image
import tensorflow as tf
from tensorflow.data import Dataset
import tensorflow_io as tfio

# constants
EPOCHS = 10000

# load data
ROOT_PATH="data"
TRAINSET_PATH="train"
TESTSET_PATH="validate"
LEARNING_RATE = 0.001

class CloudDataset():
    def __init__(self, root, folder, filenames=None):

        self.root = root

        if(filenames is not None):
            self.images_list = filenames
            self.dataset_folder = os.path.join(root, folder)
#             print(self.dataset_folder)
        else:
            self.dataset_folder =  os.path.join(root, folder)
            self.images_list = list(set(["_".join(x.split("_")[0:-1]) for x in os.listdir(self.dataset_folder)]))

    def __len__(self):
        return len(self.images_list)

    # assume 10. doesn't need to be fancy
    def get_k_folds(self):
        sets = []
        for k in range(10):
            kth_filenames = self.images_list[k * (len(self)//10): (k+1) * (len(self)//10) ]
            sets.append(CloudDataset(self.root, self.dataset_folder, kth_filenames))

        return sets

    def transform(self, rgb_img, ref):
        #Center crop to 256px by 256px
        rgb_img = self.center_crop(rgb_img, 256, 256)
#         ir_img = self.center_crop(ir_img, 256, 256)
        ref = self.center_crop(ref, 256, 256)

        # Random vertical flipping
        if random.random() > 0.5:
            rgb_img = rgb_img.transpose(Image.FLIP_TOP_BOTTOM)
#             ir_img = ir_img.transpose(Image.FLIP_TOP_BOTTOM)
            ref = ref.transpose(Image.FLIP_TOP_BOTTOM)

        # Random vertical flipping
        if random.random() > 0.5:
            rgb_img = rgb_img.transpose(Image.FLIP_LEFT_RIGHT)
#             ir_img = ir_img.transpose(Image.FLIP_LEFT_RIGHT)
            ref = ref.transpose(Image.FLIP_LEFT_RIGHT)

        #Resize to 144px by 144px
#         rgb_img = rgb_img.resize((144,144))
#         ir_img = ir_img.resize((144,144))
#         ref = ref.resize((144,144))

        rgb_img = tf.keras.preprocessing.image.img_to_array(rgb_img)
#         ir_img = tf.keras.preprocessing.image.img_to_array(ir_img)
        ref = tf.keras.preprocessing.image.img_to_array(ref)

        return rgb_img, ref

    def center_crop(self, im, new_width, new_height):
        width, height = im.size   # Get dimensions

        left = (width - new_width)/2
        top = (height - new_height)/2
        right = (width + new_width)/2
        bottom = (height + new_height)/2

        # Crop the center of the image
        im = im.crop((left, top, right, bottom))

        return im

    def shuffle(self):
        random.shuffle(self.images_list)


    # returns both raw and validation
    def __getitem__(self, idx):
        trainimage_name = self.images_list[idx]
        category = trainimage_name.split("_")[0]

        rgbimg_name = f"{trainimage_name}_rgb.tif"
#         irimg_name = f"{trainimage_name}_lwir.tif"
        ref_name = f"{trainimage_name}_ref.tif"

        rgb_img = Image.open(os.path.join(self.dataset_folder, rgbimg_name))#(io.imread(os.path.join(self.dataset_folder, rgbimg_name)))
#         ir_img = Image.open(os.path.join(self.dataset_folder, irimg_name))#(io.imread(os.path.join(self.dataset_folder, irimg_name)))
        refmask = Image.open(os.path.join(self.dataset_folder, ref_name))#(io.imread(os.path.join(self.dataset_folder, ref_name)))

        rgb_img, refmask = self.transform(rgb_img, refmask)

#         print(rgb_img.shape)
#         print(ir_img.shape)
#         img = np.concatenate((rgb_img, ir_img), axis=2)

        sample = {'img': rgb_img/255.0, 'ref': refmask/255.0, 'category': category}

        return sample

    def get_imgs_and_masks(self):
        imgs = []
        masks = []
        for i in range(self.__len__()):
            sample = self.__getitem__(i)
            imgs.append(sample['img'])
            masks.append(sample['ref'])
        return imgs, masks
    
    def load_img(trainimage_name):
        rgbimg_name = "./data/train/" + trainimage_name + "_rgb.tif"
#         irimg_name = f"{trainimage_name}_lwir.tif"
        ref_name =  "./data/train/" + trainimage_name + "_ref.tif"

        rgb_img = tf.io.read_file(rgbimg_name)#(io.imread(os.path.join(self.dataset_folder, rgbimg_name)))
#         ir_img = Image.open(os.path.join(self.dataset_folder, irimg_name))#(io.imread(os.path.join(self.dataset_folder, irimg_name)))
        refmask = tf.io.read_file(ref_name)#(io.imread(os.path.join(self.dataset_folder, ref_name)))

        rgb_img = tfio.experimental.image.decode_tiff(rgb_img)
        rgb_img = rgb_img[0:256,0:256,0:3]
        rgb_img = tf.image.convert_image_dtype(rgb_img, tf.float32)
        rgb_img.set_shape([256, 256, 3])

#         rgb_img = tf.image.crop_to_bounding_box(rgb_img, 0, 0, 256, 256)
#         print(rgb_img.get_shape())
        
        refmask = tfio.experimental.image.decode_tiff(refmask)
        refmask = tf.expand_dims(refmask[0:256, 0:256, 0], axis=-1)
        refmask = tf.image.convert_image_dtype(refmask, tf.float32)
        refmask.set_shape([256, 256, 1])

#         refmask = tf.image.crop_to_bounding_box(rgb_img, 0, 0, 256, 256)
#         rgb_img, refmask = self.transform(rgb_img, refmask)

#         print(rgb_img.shape)
#         print(ir_img.shape)
#         img = np.concatenate((rgb_img, ir_img), axis=2)

        sample = (rgb_img, refmask)
        return sample

    