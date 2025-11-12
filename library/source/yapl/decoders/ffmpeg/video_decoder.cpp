#include "yapl/decoders/ffmpeg/video_decoder.hpp"
#include "yapl/debug.hpp"
#include "yapl/track_info.hpp"

#include <cstdio>
#include <fmt/format.h>
#include <memory>
#include <stdexcept>
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

video_decoder::video_decoder(AVCodecID codec_id) {
    LOG_INFO("video_decoder initialized");
    m_codecpar = avcodec_parameters_alloc();
    m_codecpar->codec_id = codec_id;

    const AVCodec *codec = avcodec_find_decoder(codec_id);
    if (!codec) {
        throw std::runtime_error(
            fmt::format("Unsupported codec {}", static_cast<int>(codec_id)));
    }

    LOG_INFO("Codec name {}, long name {}", codec->name, codec->long_name);

    m_codec_ctx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(m_codec_ctx, m_codecpar) < 0) {
        avcodec_parameters_free(&m_codecpar);
        avcodec_free_context(&m_codec_ctx);
        throw std::runtime_error(
            fmt::format("Failed to copy codec params to context"));
    }
    if (avcodec_open2(m_codec_ctx, codec, nullptr) < 0) {
        throw std::runtime_error("Could not open codec");
    }

    m_frame = av_frame_alloc();
}

video_decoder::~video_decoder() {
    LOG_INFO("video_decoder destroyed");
    avcodec_parameters_free(&m_codecpar);
    avcodec_free_context(&m_codec_ctx);
    av_frame_free(&m_frame);
}

bool video_decoder::decode(std::shared_ptr<track_info> info,
                           std::shared_ptr<media_sample> sample,
                           std::shared_ptr<media_sample> decoded_sample) {
    // Decode data using FFmpeg
    LOG_INFO("Decoding data with video_decoder");
    (void)decoded_sample;
    (void)sample;
    (void)info;

    AVPacket packet = {};
    packet.data = sample->data.data();
    packet.size = sample->data.size();

    LOG_DEBUG("Decoding sample - size: {}", packet.size);

    // static size_t sample_id = 0;

    static FILE *fp = fopen("to_decode_.bin", "wb");

    fwrite(sample->data.data(), sample->data.size(), 1, fp);

    // fclose(fp);

    int ret = avcodec_send_packet(m_codec_ctx, &packet);
    if (ret < 0) {
        char buffer[1024]{0};
        av_strerror(ret, buffer, 1024);
        printf("send_packet error: %d, %s\n", ret, buffer);
        return false;
    }

    while (true) {
        ret = avcodec_receive_frame(m_codec_ctx, m_frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        static size_t frame_id = 0;
        FILE *fp = fopen(
            fmt::format("decoded_frames/frame_{}.bin", frame_id++).c_str(),
            "wb");

        fwrite(m_frame->data, m_frame->width * m_frame->height * 3 / 2, 1, fp);

        fclose(fp);

        printf("Decoded frame %dx%d pts=%ld\n", m_frame->width, m_frame->height,
               m_frame->pts);
    }

    return true; // Return true if decoding was successful
}

} // namespace yapl::decoders::ffmpeg
