import cv2
from cv2 import VideoWriter, VideoWriter_fourcc, imread, resize
import os
from PIL import Image
import sys
     
 
def Pic2Video(imgPath,videoPath ):
    images = os.listdir(imgPath)
    fps = 15  # 每秒15帧数
 
    # VideoWriter_fourcc为视频编解码器 ('I', '4', '2', '0') —>(.avi) 、('P', 'I', 'M', 'I')—>(.avi)、('X', 'V', 'I', 'D')—>(.avi)、('T', 'H', 'E', 'O')—>.ogv、('F', 'L', 'V', '1')—>.flv、('m', 'p', '4', 'v')—>.mp4
    fourcc = VideoWriter_fourcc('m', 'p', '4', 'v')
 
    image = Image.open(imgPath + "/" + images[0])
    videoWriter = cv2.VideoWriter(videoPath + "/recolored.mp4", fourcc, fps, image.size)


    for i in range(len(images)):

        frame = cv2.imread(imgPath + "/" +  str(i) + ".png")  # 这里的路径只能是英文路径
        print(imgPath + "/" +  str(i) + ".png")
        videoWriter.write(frame)
    print("图片转视频结束！")
    videoWriter.release()
    cv2.destroyAllWindows()
     
if __name__ == '__main__':
    imgPath = sys.argv[1]
    videoPath = sys.argv[2]
    Pic2Video(imgPath,videoPath)