# -*- coding: UTF-8 -*-

import numpy as np
import os
import sys
import cv2


size = (550, 300)

if __name__ == '__main__':

	base_dir = '../season'
	save_path = base_dir + '/skew_polyhedron_palette/season.video'
	frm_cnt = 50

	imgs = []
   
	# 0. 改这里时间长度
	for j in range(0, frm_cnt):
	# 1. 改这里导入文件的目录
		img = np.load(base_dir + '/%05d.img.npz.npy' % j)
		#cv2.imshow("src", img[:, :, ::-1])
		shrink = cv2.resize(img, size, interpolation=cv2.INTER_AREA)
		imgs.append(shrink)
	
	imgs = np.array(imgs)
	imgs = imgs.astype(np.float) / 255.

	# 3. 改这里输出的 video 文件
	with open(save_path, 'wb') as f:
	    f.write(np.array(imgs.shape, dtype=np.int32).tobytes())
	    f.write(imgs.tobytes())