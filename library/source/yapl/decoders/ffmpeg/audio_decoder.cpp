#include "yapl/decoders/ffmpeg/audio_decoder.hpp"
#include "yapl/debug.hpp"
#include "yapl/utilities.hpp"
#include <fmt/format.h>

namespace yapl::decoders::ffmpeg {

audio_decoder::audio_decoder(AVCodecID codec_id,
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
    m_codec_ctx->extradata_size = static_cast<int>(extra_data.size());

    if (avcodec_open2(m_codec_ctx, codec, nullptr) < 0) {
        throw std::runtime_error("Could not open codec");
    }
}

audio_decoder::~audio_decoder() {
    avcodec_parameters_free(&m_codecpar);
    avcodec_free_context(&m_codec_ctx);
}

bool audio_decoder::decode([[maybe_unused]] std::shared_ptr<track_info> info,
                           std::shared_ptr<media_sample> sample,
                           std::shared_ptr<media_sample> decoded_sample) {
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    utilities::raii_cleanup clean{[&] {
        av_packet_unref(packet);
        av_packet_free(&packet);
        av_frame_free(&frame);
    }};

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
        LOG_INFO("An empty audio frame!");
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
            LOG_DEBUG("Max recevide audio frames: {}", received_frames);
        }

        switch (frame->format) {
        case AV_SAMPLE_FMT_FLTP: {
            decoded_sample->data.resize(frame->ch_layout.nb_channels *
                                        frame->nb_samples * sizeof(float));
            switch (frame->ch_layout.nb_channels) {
            case 2: {
                float *left = (float *)frame->extended_data[0];
                float *right = (float *)frame->extended_data[1];
                float *out =
                    reinterpret_cast<float *>(decoded_sample->data.data());

                for (int i = 0; i < frame->nb_samples; i++) {
                    out[i * 2 + 0] = left[i];
                    out[i * 2 + 1] = right[i];
                }
            } break;
            default: {
                LOG_CRITICAL("Unsuported audio number of channels {}",
                             frame->ch_layout.nb_channels);
            } break;
            }
        } break;
        default:
            LOG_CRITICAL("Unsuported audio frame format {}",
                         av_get_sample_fmt_name(
                             static_cast<AVSampleFormat>(frame->format)));
            break;
        }
    }
    return true;
}

} // namespace yapl::decoders::ffmpeg
