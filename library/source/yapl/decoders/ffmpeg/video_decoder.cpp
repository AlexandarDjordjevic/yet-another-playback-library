#include "yapl/decoders/ffmpeg/video_decoder.hpp"
#include "yapl/debug.hpp"
#include "yapl/track_info.hpp"
#include <memory>
#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/codec_id.h>
#include <libavcodec/codec_par.h>
#ifdef __cplusplus
}
#endif

namespace yapl::decoders::ffmpeg {

video_decoder::video_decoder() {
    // Initialize FFmpeg decoder
    LOG_INFO("video_decoder initialized");
}

video_decoder::~video_decoder() {
    // Cleanup FFmpeg decoder
    LOG_INFO("video_decoder destroyed");
}

bool video_decoder::decode(
    std::shared_ptr<track_info> info, std::shared_ptr<media_sample> sample,
    std::shared_ptr<decoded_media_sample> decoded_sample) {
    // Decode data using FFmpeg
    LOG_INFO("Decoding data with video_decoder");
    (void)decoded_sample;
    (void)sample;
    AVCodecParameters *codecpar = avcodec_parameters_alloc();
    codecpar->codec_id = (AVCodecID)info->codec_id;

    const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        LOG_ERROR("Unsupported codec {}", info->codec_id);
        return false;
    }

    LOG_INFO("Codec name {}, long name {}", codec->name, codec->long_name);

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codec_ctx, codecpar) < 0) {
        avcodec_parameters_free(&codecpar);
        avcodec_free_context(&codec_ctx);
        LOG_ERROR("Failed to copy codec params to context");
        return false;
    }
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        LOG_ERROR("Could not open codec.");
        return false;
    }

    avcodec_parameters_free(&codecpar);
    avcodec_free_context(&codec_ctx);
    return true; // Return true if decoding was successful
}

} // namespace yapl::decoders::ffmpeg
