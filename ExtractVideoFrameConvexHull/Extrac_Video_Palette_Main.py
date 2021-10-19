#!/usr/bin/env python
# coding: utf-8

from skvideo.io import FFmpegReader
import PIL.Image as Image
import time
import scipy
import json
import Additive_mixing_layers_extraction
from scipy.spatial import ConvexHull, Delaunay
import scipy.sparse
import PIL.Image as Image
import RGBXY_method
import numpy as np
from numpy import *
Additive_mixing_layers_extraction.DEMO=True

import threading
import sys

def compute_rgb_convex_hull(img_data, cvx_path) : 
    start=time.time()
    palette_rgb = Additive_mixing_layers_extraction.Hull_Simplification_determined_version(img_data, cvx_path)
    end=time.time()    
    
    M=len(palette_rgb)
    print("palette size: ", M)
    print("palette extraction time: ", end-start)

if __name__ == '__main__':

    file_path = "./data/test.mp4"
    save_path = "./data"

    k = 0
    all_imgs = []

    reader = FFmpegReader(file_path)
    for img in reader.nextFrame():
        img_data_path = save_path + "/%05d" % k
        Image.fromarray(img).save(img_data_path+".png")
        np.save(img_data_path + ".img.npz", img)

        img = img / 255.0
        all_imgs.append(img)
        k = k + 1

    k = 0
    while k < len(all_imgs):
        threads = []
        
        img_data = all_imgs[k]
        img_data = img_data[::2,::2,:]    
        convexhull_path = save_path + "/rgb_cvx_%05d" % k
        
        t1 = threading.Thread(target=compute_rgb_convex_hull,args = (img_data,convexhull_path))
        threads.append(t1)
        
        k = k + 1         
        if k < len(all_imgs):
            img_data = all_imgs[k]
            img_data = img_data[::2,::2,:]
            convexhull_path = save_path + "/rgb_cvx_%05d" % k
            
            t2 = threading.Thread(target=compute_rgb_convex_hull,args = (img_data,convexhull_path))
            threads.append(t2)     
        
        k = k + 1
        if k < len(all_imgs):
            img_data = all_imgs[k]
            img_data = img_data[::2,::2,:]
            convexhull_path = save_path + "/rgb_cvx_%05d" % k
            
            t3 = threading.Thread(target=compute_rgb_convex_hull,args = (img_data,convexhull_path))
            threads.append(t3)
            
        k = k + 1
        for t in threads:
            t.setDaemon(True)
            t.start()
            
        for t in threads:
            t.join()

    print("finish!")