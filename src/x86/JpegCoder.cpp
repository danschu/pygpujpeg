#include <JpegCoder.hpp>
#include <libgpujpeg/gpujpeg_decoder.h>
#include <libgpujpeg/gpujpeg_encoder.h>
#include <libgpujpeg/gpujpeg_common.h>

#include <chrono>
#include <ctime>  

void* JpegCoder::_global_context = nullptr;

typedef struct
{
  struct gpujpeg_encoder* encoder;
  struct gpujpeg_decoder* decoder;
  size_t last_width = -1;
  size_t last_height = -1;
  size_t last_comp_count = -1;
  
} GpuJpegGlobalContext;

#define JPEGCODER_GLOBAL_CONTEXT ((GpuJpegGlobalContext*)(JpegCoder::_global_context))


JpegCoder::JpegCoder() {
    if (JpegCoder::_global_context == nullptr){
		JpegCoder::_global_context = malloc(sizeof(GpuJpegGlobalContext));	
		if (gpujpeg_init_device(0, 0) ) {
			//std::cout << "Failed gpujpeg_init_device";
			return;
		}
			
		
		JPEGCODER_GLOBAL_CONTEXT->decoder = gpujpeg_decoder_create(0);		
		if ( JPEGCODER_GLOBAL_CONTEXT->decoder == NULL ) {
			//std::cout << "Failed init decoder";
			return;
		}
		
		
		JPEGCODER_GLOBAL_CONTEXT->encoder = gpujpeg_encoder_create(0);		
		if ( JPEGCODER_GLOBAL_CONTEXT->encoder == NULL ) {
			//std::cout << "Failed init encoder";
			return;
		}
			
    }
}

JpegCoder::~JpegCoder() {
	
}

void JpegCoder::cleanUpEnv() {
    if (JpegCoder::_global_context != nullptr) {
        if (JPEGCODER_GLOBAL_CONTEXT->encoder != NULL) {
            gpujpeg_encoder_destroy(JPEGCODER_GLOBAL_CONTEXT->encoder);
        }
        if (JPEGCODER_GLOBAL_CONTEXT->decoder != NULL) {
            gpujpeg_decoder_destroy(JPEGCODER_GLOBAL_CONTEXT->decoder);
		}	
		free(JpegCoder::_global_context);
		JpegCoder::_global_context = nullptr;
    }
}

void JpegCoder::ensureThread(long threadIdent) {
	
}

JpegDecoderBytes* JpegCoder::read(const char* jpegFile) {
	size_t length = 0;
	uint8_t * jpegData = NULL;
	if ( gpujpeg_image_load_from_file(jpegFile, &jpegData, &length) != 0 )
		return nullptr;
	JpegDecoderBytes* res = decode(jpegData, length);
	
	//res->internal_copy();
	gpujpeg_image_destroy(jpegData);
	return res;
}


JpegDecoderBytes* JpegCoder::decode(uint8_t * jpegData, size_t length) {
	
	
	struct gpujpeg_parameters param;
	gpujpeg_set_default_parameters(&param);	

	struct gpujpeg_image_parameters param_image;
	gpujpeg_image_set_default_parameters(&param_image);
	
	if ( gpujpeg_decoder_get_image_info(jpegData, length, &param_image, &param, NULL) != 0 )
		return nullptr;	
	
	if ((JPEGCODER_GLOBAL_CONTEXT->last_width != param_image.width) ||
		(JPEGCODER_GLOBAL_CONTEXT->last_height != param_image.height) ||
		(JPEGCODER_GLOBAL_CONTEXT->last_comp_count != param_image.comp_count)) {
			
		JPEGCODER_GLOBAL_CONTEXT->last_width = param_image.width;
		JPEGCODER_GLOBAL_CONTEXT->last_height = param_image.height;
		JPEGCODER_GLOBAL_CONTEXT->last_comp_count = param_image.comp_count;	
		
		gpujpeg_decoder_destroy(JPEGCODER_GLOBAL_CONTEXT->decoder);
		JPEGCODER_GLOBAL_CONTEXT->decoder = gpujpeg_decoder_create(0);
	}
	
	
	if ( gpujpeg_decoder_init(JPEGCODER_GLOBAL_CONTEXT->decoder, &param, &param_image) != 0 )
		return nullptr;
			
	struct gpujpeg_decoder_output decoder_output;
	gpujpeg_decoder_output_set_default(&decoder_output);
	
	if ( gpujpeg_decoder_decode(JPEGCODER_GLOBAL_CONTEXT->decoder, jpegData, length, &decoder_output) != 0 )
		return nullptr;	


	
	JpegDecoderBytes* bmpData = new JpegDecoderBytes(param_image.width, param_image.height,
		param_image.comp_count, decoder_output.data, decoder_output.data_size);	
    return bmpData;
}

int JpegCoder::write(const char* jpegFile, size_t width, size_t height, size_t channels, uint8_t * bmpData, int quality) {	
	JpegEncoderBytes* jpegData = encode(width, height, channels, bmpData, quality);
	int ret = -1;
	if ( gpujpeg_image_save_to_file(jpegFile, jpegData->data, jpegData->size, NULL) != 0) {
		delete jpegData;
	}
	ret = (int)jpegData->size;
	delete jpegData;
	return ret;
}

JpegEncoderBytes* JpegCoder::encode(size_t width, size_t height, size_t channels, uint8_t * bmpData, int quality) {	
	struct gpujpeg_parameters param;
	gpujpeg_set_default_parameters(&param);
	
	gpujpeg_parameters_chroma_subsampling_420(&param);
	param.quality = quality;
		
	struct gpujpeg_encoder_input encoder_input;
    gpujpeg_encoder_input_set_image(&encoder_input, bmpData);
	
	struct gpujpeg_image_parameters param_image;
    gpujpeg_image_set_default_parameters(&param_image);
	
	param_image.width = (int)width;
	param_image.height = (int)height;
	param_image.comp_count = (int)channels;
	param_image.color_space = GPUJPEG_RGB;
		
	uint8_t *image_compressed = NULL;
    size_t image_compressed_size = 0;
    if (gpujpeg_encoder_encode(JPEGCODER_GLOBAL_CONTEXT->encoder, &param, &param_image,
		&encoder_input, &image_compressed, &image_compressed_size) != 0)
		return nullptr;
		
	JpegEncoderBytes* jpegData = new JpegEncoderBytes(image_compressed, image_compressed_size);	
    return jpegData;
}

