GPUJPEG - Python
---------------------------

Fast Jpeg Encoding/Decoding on GPU based on [gpujpeg](https://github.com/CESNET/GPUJPEG) and [nvjpeg-python](https://github.com/UsingNet/nvjpeg-python).

## Require
* cuda >= 10.2
* numpy >= 1.7
* python >= 3.6
* gcc >= 7.5
* make >= 4.1

## System
* Windows (not tested on Linux yet)

## Installing

Run [build_and_test.cmd](build_and_test.cmd):
-  it downloads and builds gpujpeg
-  creates virtual environment ad activates it
-  installs numpy and opencv-python
-  compiles pygpujpeg and
-  runs a simple benchmark.

## Usage

Have look at  [test.py](test.py). 

### 0. Init PyGpuJpeg
```python
from gpujpeg import GpuJpeg 
gp = GpuJpeg()
```

### 1. Use PyGpuJpeg

#### Read Jpeg File to Numpy
```python
with open("testimg.jpg", "rb") as f:
    data = f.read()
img = gp.imdecode(data)
```

or

```python
img = gp.imread("testimg.jpg")
```

#### Write Numpy to Jpeg File
```python
data = gp.imencode(img, 85) 
with open(f"testimg.jpg", 'wb') as f:    
    f.write(data)   
```

or 

```python
gp.imwrite(img, "testimg.jpg", 85) 
```

### 3. FAQ
Q: Why is image decoding sometimes slow?

A: Huffman decoding can not be parallelized in gpujpeg, if no restart markers are set when image was encoded Pgpujpeg (based on gpujpeg) uses restart markers, therefore all images written with this library can be decoded much faster. If you are using OpenCV for encoding and need fast decoding, then set the restart marker in OpenCV:
```python
params = [
    cv2.IMWRITE_JPEG_QUALITY, quality
    ,cv2.IMWRITE_JPEG_RST_INTERVAL, 8
]

imgj = cv2.imencode(".jpg", img, params)[1]
```