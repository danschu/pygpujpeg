#pragma once

#include <sys/types.h>
#include <malloc.h>
#include <memory.h>
#include <iostream>
#include <exception>

class JpegCoderError: public std::runtime_error{
protected:
    int _code;
public:
    JpegCoderError(int code, const std::string& str):std::runtime_error(str){
        this->_code = code;
    }
    JpegCoderError(int code, const char* str):std::runtime_error(str){
        this->_code = code;
    }
    int code(){
        return this->_code;
    }
};


class JpegDecoderBytes{
public:
    unsigned char* data;
    size_t height;
    size_t width;
    short nChannel;
	size_t size;
	bool owns_data;
    
	void internal_copy() {
		unsigned char* old = this->data;
		this->data = (unsigned char*)malloc(this->size);
		memcpy(this->data, old, size);
        this->owns_data = true;
	}
		
	
    JpegDecoderBytes(size_t width, size_t height, short nChannel, unsigned char* data, size_t size) {	
		this->height = height;
        this->width = width;
        this->nChannel = nChannel;	
        
		this->data = data;
        this->size = size;
        this->owns_data = false;
	}
    ~JpegDecoderBytes(){
		if (this->owns_data) {
			free(this->data);
		}
	};
};


class JpegEncoderBytes{
public:
    size_t size;
    unsigned char* data;
	
    JpegEncoderBytes(unsigned char* data, size_t size){
        this->data = data;
        this->size = size;
	}

    ~JpegEncoderBytes() {
	};
};

class JpegCoder{
protected:
    static void* _global_context;
    void* _local_context;
public:
    JpegCoder();
    ~JpegCoder();
    void ensureThread(long threadIdent);
    JpegDecoderBytes* read(const char* jpegFile);
    JpegDecoderBytes* decode(uint8_t * jpegData, size_t length);
    JpegEncoderBytes* encode(size_t width, size_t height, size_t channels, uint8_t * bmpData, int quality);
    int write(const char* jpegFile, size_t width, size_t height, size_t channels, uint8_t * bmpData, int quality);
    static void cleanUpEnv();
};
