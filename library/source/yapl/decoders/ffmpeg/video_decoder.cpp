#include "yapl/decoders/ffmpeg/video_decoder.hpp"
#include "yapl/debug.hpp"
#include "yapl/media_sample.hpp"
#include "yapl/track_info.hpp"
#include "yapl/utilities.hpp"

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

video_decoder::video_decoder(AVCodecID codec_id,
                             std::span<uint8_t> extra_data) {
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

    m_codec_ctx->extradata =
        (uint8_t *)av_malloc(extra_data.size() + AV_INPUT_BUFFER_PADDING_SIZE);
    memcpy(m_codec_ctx->extradata, extra_data.data(), extra_data.size());
    memset(m_codec_ctx->extradata + extra_data.size(), 0,
           AV_INPUT_BUFFER_PADDING_SIZE);
    m_codec_ctx->extradata_size = extra_data.size();

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
    AVFrame *frame = av_frame_alloc();

    utilities::raii_cleanup clean{[&] {
        av_packet_unref(packet);
        av_packet_free(&packet);
        av_frame_free(&frame);
    }};

    // If sample is null, this is a flush request
    if (sample && sample->data.size() > 0) {
        packet->data = sample->data.data();
        packet->size = static_cast<int>(sample->data.size());

        int ret = avcodec_send_packet(m_codec_ctx, packet);
        if (ret < 0) {
            char buffer[1024]{0};
            av_strerror(ret, buffer, 1024);
            LOG_CRITICAL("send_packet error: {}, {}. Sample debug id: {}", ret,
                         buffer, sample->debug_id);
            av_packet_free(&packet);
            return false;
        }
    } else {
        // Send null packet to flush decoder
        printf("Null packets...\n");
    }

    static int max_rcvd_frames = -1;
    auto received_frames = 0;

    while (true) {
        int ret = avcodec_receive_frame(m_codec_ctx, frame);
        if (ret == AVERROR(EAGAIN)) {
            break; // Need more input
        } else if (ret == AVERROR_EOF) {
            break; // End of stream
        } else if (ret < 0) {
            char buffer[1024]{0};
            av_strerror(ret, buffer, 1024);
            LOG_ERROR("Decode error {}", buffer);
            return false;
        }

        if (max_rcvd_frames < ++received_frames) {
            max_rcvd_frames = received_frames;
            LOG_DEBUG("Max recevide video frames: {}", received_frames);
        }
        write_yuv420p_frame(frame, decoded_sample);
    }

    return true;
}

} // namespace yapl::decoders::ffmpeg
