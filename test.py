import os

#python is not including path dirs to search path -> nvidia dlls are missing
#for p in os.environ["PATH"].split(";"):
#    if p != "" and os.path.exists(p):
#        os.add_dll_directory(p) 

os.add_dll_directory(os.environ["CUDA_PATH"] + "/bin")
os.add_dll_directory(os.path.abspath("./GPUJPEG/build/Release"))

from gpujpeg import GpuJpeg

import numpy as np
import time
import cv2

benchmark_img_sizes = [
    (640, 480),
    (1280, 1024),
    (1920, 1080),
    (3840, 2160)
]

benchmark_quality = [
  30, 50, 70, 85, 90,95, 100
]

frames_test = 1000
frames_test_cv = frames_test//20

def generate_random_image(width: int, height: int):
    img = np.random.rand(height, width, 3) * 255
    img = img.astype(np.int8)
    return img


def benchmark():
    gp = GpuJpeg()
    print("FPS for Image Encoding and Decoding (more = better)")
    print("Imagesize \t Quality \t OpenCV-Encode \t OpenCV-Decode \t PYGPU-Encode \t PYGPU-Decode \t Factor Encode \t Factor Decode")
            
    #os.makedirs("benchmark", exist_ok = True)
    for quality in benchmark_quality:
        for imgsize in benchmark_img_sizes:
        
            # create random image
    
            # if we want to use fast decoding
            # the images should be written withj pygpujpeg or opencv
            # with restart interval enabled (e.g.):
            
            # it we dont use restart inverval, then opecv ist ~ 2x faster!

            img = generate_random_image(imgsize[0], imgsize[1])

            params = [
                cv2.IMWRITE_JPEG_QUALITY, quality
                ,cv2.IMWRITE_JPEG_RST_INTERVAL, 8
            ]
            #filename = f"benchmark/benchmark_opencv_{imgsize[0]}x{imgsize[1]}_q{quality}.jpg"
            
            # writing opencv
            t1 = time.time()
            for i in range(0, frames_test_cv):
                imgj = cv2.imencode(".jpg", img, params)[1]
            t2 = time.time()
            fps_write_opencv = frames_test_cv/((t2-t1)+0.001)
            imgj = np.array(imgj)
            
            # reading opencv
            t1 = time.time()
            for i in range(0, frames_test_cv):
                img = cv2.imdecode(imgj, 1)
            t2 = time.time()
            fps_read_opencv = frames_test_cv/((t2-t1)+0.001)
 
            #filename = f"benchmark/benchmark_pygpujpeg_{imgsize[0]}x{imgsize[1]}_q{quality}.jpg"
            
            img = generate_random_image(imgsize[0], imgsize[1])
            
            # writing pygpujpeg
            t1 = time.time()
            for i in range(0, frames_test):
                imgj = gp.imencode(img, quality)
            t2 = time.time()
            fps_write_pygpujpeg = frames_test/((t2-t1)+0.001)
            # writing pygpujpeg
            t1 = time.time()
            for i in range(0, frames_test):
                img = gp.imdecode(imgj)
            t2 = time.time()
            fps_read_pygpujpeg = frames_test/((t2-t1)+0.001)

            sz = f"{imgsize[0]}x{imgsize[1]}"
            print(f"{sz:10} \t {quality:10} \t " \
                f"{fps_write_opencv:10.1f} \t {fps_read_opencv:10.1f} \t " \
                f"{fps_write_pygpujpeg:10.1f} \t {fps_read_pygpujpeg:10.1f} \t " \
                f"{fps_write_pygpujpeg/fps_write_opencv:10.1f} \t {fps_read_pygpujpeg/fps_read_opencv:10.1f}"                 
                )
    
if __name__ == "__main__":
    benchmark()
     
