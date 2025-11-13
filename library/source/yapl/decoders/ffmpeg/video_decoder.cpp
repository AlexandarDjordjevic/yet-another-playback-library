#include "yapl/decoders/ffmpeg/video_decoder.hpp"
#include "yapl/debug.hpp"
#include "yapl/media_sample.hpp"
#include "yapl/track_info.hpp"

#include <cstdio>
#include <cstring>
#include <fmt/format.h>
#include <libavutil/frame.h>
#include <memory>
#include <stdexcept>
#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/codec_id.h>
#include <libavcodec/codec_par.h>
#include <libavutil/pixdesc.h>
#ifdef __cplusplus
}
#endif

namespace yapl::decoders::ffmpeg {

void write_yuv420p_frame(AVFrame *frame, std::shared_ptr<media_sample> sample) {
    // For YUV420p format
    if (frame->format == AV_PIX_FMT_YUV420P) {
        sample->data.resize(frame->width * frame->height * 3 / 2);
        auto *ptr = sample->data.data();
        // Write Y plane (luma)
        for (int y = 0; y < frame->height; y++) {
            memcpy(ptr, frame->data[0] + y * frame->linesize[0], frame->width);
            ptr += frame->width;
        }

        // Write U plane (chroma)
        for (int y = 0; y < frame->height / 2; y++) {
            memcpy(ptr, frame->data[1] + y * frame->linesize[1],
                   frame->width / 2);
            ptr += frame->width / 2;
        }

        // Write V plane (chroma)
        for (int y = 0; y < frame->height / 2; y++) {
            memcpy(ptr, frame->data[2] + y * frame->linesize[2],
                   frame->width / 2);
            ptr += frame->width / 2;
        }
    }
}

video_decoder::video_decoder(AVCodecID codec_id) {
    m_codecpar = avcodec_parameters_alloc();
    m_codecpar->codec_id = codec_id;

    const AVCodec *codec = avcodec_find_decoder(codec_id);
    if (!codec) {
        throw std::runtime_error(
            fmt::format("Unsupported codec {}", static_cast<int>(codec_id)));
    }

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
}

video_decoder::~video_decoder() {
    avcodec_parameters_free(&m_codecpar);
    avcodec_free_context(&m_codec_ctx);
}

bool video_decoder::decode([[maybe_unused]] std::shared_ptr<track_info> info,
                           std::shared_ptr<media_sample> sample,
                           std::shared_ptr<media_sample> decoded_sample) {
    AVPacket *packet = av_packet_alloc();

    static FILE *decoder_input = fopen("decoder_input.h264", "wb");
    static size_t total_decoder_input = 0;

    fwrite(sample->data.data(), 1, sample->data.size(), decoder_input);
    total_decoder_input += sample->data.size();

    LOG_INFO("Debug sample id: {}", sample->debug_id);
    LOG_INFO("Total decoder input in bytes {}", total_decoder_input);

    // If sample is null, this is a flush request
    if (sample && sample->data.size() > 0) {
        packet->data = sample->data.data();
        packet->size = sample->data.size();

        LOG_DEBUG("Decoding sample - size: {}", packet->size);

        int ret = avcodec_send_packet(m_codec_ctx, packet);
        if (ret < 0) {
            char buffer[1024]{0};
            av_strerror(ret, buffer, 1024);
            LOG_CRITICAL("send_packet error: {}, {}. Sample debug id: {}", ret,
                         buffer, sample->debug_id);
            FILE *eof_sample = fopen(
                fmt::format("samples/after/sample_{}.h264", sample->debug_id)
                    .c_str(),
                "wb");
            fwrite(sample->data.data(), 1, sample->data.size(), eof_sample);
            fclose(eof_sample);
            av_packet_free(&packet);
            return false;
        }
    } else {
        // Send null packet to flush decoder
        printf("Null packets...\n");
    }

    av_packet_unref(packet);
    av_packet_free(&packet);

    static size_t frames_decoded = 0;
    while (true) {
        AVFrame *frame = av_frame_alloc();
        int ret = avcodec_receive_frame(m_codec_ctx, frame);
        if (ret == AVERROR(EAGAIN)) {
            av_frame_free(&frame);
            break; // Need more input
        } else if (ret == AVERROR_EOF) {
            av_frame_free(&frame);
            break; // End of stream
        } else if (ret < 0) {
            av_frame_free(&frame);
            break; // Error
        }

        frames_decoded++;

        // Debug information
        printf("Decoded frame %zu: %dx%d format=%d (%s) pts=%ld\n",
               frames_decoded, frame->width, frame->height, frame->format,
               av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format)),
               frame->pts);

        write_yuv420p_frame(frame, decoded_sample);
        // [[maybe_unused]] static auto once = [&] {
        //     LOG_INFO("Decoded sample size {}", decoded_sample->data.size());
        //     auto *fp = fopen("frame.yuv", "wb");
        //     fwrite(decoded_sample->data.data(), decoded_sample->data.size(),
        //     1,
        //            fp);

        //     fclose(fp);
        //     exit(-1);
        //     return true;
        // }();

        av_frame_free(&frame);
    }

    return true;
}

} // namespace yapl::decoders::ffmpeg
