#!/usr/bin/env python
import sys
import os
import platform
import glob
from setuptools import setup, find_packages, Extension
import numpy

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

from distutils.core import setup, Extension

if platform.system() == 'Linux':
    if os.path.exists('/usr/src/jetson_multimedia_api'):
        # Jetson
        extension_gpujpeg = Extension('gpujpeg', 
            ['gpujpeg-python.cpp', 'src/jetson/JpegCoder.cpp',], 
            ['include', numpy.get_include()], 
            [('JPEGCODER_ARCH', 'jetson')],
            library_dirs=['/usr/lib/aarch64-linux-gnu/tegra', 'build/lib'],
            libraries=['gpujpeg']
        )
    else:
        # x86 or x86_64
        extension_gpujpeg = Extension('gpujpeg', 
            ['gpujpeg-python.cpp', 'src/x86/JpegCoder.cpp'], 
            ['include', numpy.get_include()], 
            [('JPEGCODER_ARCH', 'x86')],
            libraries=['gpujpeg']
        )
elif platform.system() == 'Windows':
    extension_gpujpeg = Extension('gpujpeg', 
        ['gpujpeg-python.cpp', 'src\\x86\\JpegCoder.cpp'], 
        ['include', numpy.get_include(),  ".\\GPUJPEG"], 
        [('JPEGCODER_ARCH', 'x86')],
        library_dirs=['.\\GPUJPEG\\build\\Release'],
        libraries=['gpujpeg']
    )


setup(name='pygpujpeg',
    version='0.0.1',
    ext_modules=[extension_gpujpeg],
    author="Daniel S.",
    author_email="",
    license="MIT",
    description="Python interface for gpujpeg. Encode/Decode Jpeg with Nvidia GPU Hardware Acceleration.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    # packages=setuptools.find_packages(),
    classifiers=[
        "Development Status :: 4 - Beta",
        "Programming Language :: Python :: 3 :: Only",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
        "Operating System :: Microsoft :: Windows"
    ],
    keywords=[
        "gpujpeg",
        "pygpujpeg",
        "jpeg",
        "jpg",
        "encode",
        "decode",
        "jpg encode",
        "jpg decode",
        "jpeg encode",
        "jpeg decode",
        "gpu",
        "nvidia"
    ],
    python_requires=">=3.6",
    project_urls={
    },
    install_requires=['numpy>=1.17']
)
