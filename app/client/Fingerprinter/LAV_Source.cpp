/*
 * LAV_Source: use FFmpeg/Libav to extract audio for Last.fm fingerprinting
 * Copyright (C) 2012 John Stamp <jstamp@mehercule.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LAV_Source.h"

// Needed by libavutil/common.h
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS 1
#endif

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/mem.h>

#if defined(HAVE_SWRESAMPLE)
# include <libswresample/swresample.h>
#elif defined (HAVE_AVRESAMPLE)
# include <libavresample/avresample.h>
# include <libavutil/opt.h>
#endif
}

#include <iostream>
#include <limits>
#include <stdexcept>

#include <QFile>

using namespace std;


/*
   Don't change these values!
   The fingerprinter expects signed 16 bit pcm w/ 1 or 2 channels
*/
const AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;
const int outSampleSize           = av_get_bytes_per_sample(outSampleFmt);
const int outMaxChannels          = 2;


class LAV_SourcePrivate
{
public:
    LAV_SourcePrivate()
        : inFormatContext(NULL)
        , inCodecContext(NULL)
#if defined(HAVE_SWRESAMPLE) || defined(HAVE_AVRESAMPLE)
        , resampleContext(NULL)
#endif
        , streamIndex(-1)
        , duration(0)
        , bitrate(0)
        , eof(false)
        , overflowSize(0)
    {
    }
    uint8_t * decodeOneFrame(int &dataSize, int &channels, int& nSamples);

    AVFormatContext *inFormatContext;
    AVCodecContext *inCodecContext;
#if defined(HAVE_SWRESAMPLE)
    SwrContext *resampleContext;
#elif defined(HAVE_AVRESAMPLE)
    AVAudioResampleContext *resampleContext;
#endif
    int streamIndex;
    int duration;
    int bitrate;
    bool eof;
    uint8_t outBuffer[AVCODEC_MAX_AUDIO_FRAME_SIZE*4];
    uint8_t overflow[AVCODEC_MAX_AUDIO_FRAME_SIZE*4];
    size_t overflowSize;
};


/** This reads the audio data from one frame, converts it to an acceptable
 * format (if needed), and returns a pointer to the the decoded data.
 *
 * @param dataSize bytes of decoded data
 * @param channels number of decoded channels
 * @param nSamples number of decoded samples
 */
uint8_t * LAV_SourcePrivate::decodeOneFrame(int &dataSize, int &channels, int& nSamples)
{
    char buf[256];
    AVPacket packet;
    AVFrame *decodedFrame = avcodec_alloc_frame();
    av_init_packet(&packet);

    int frameFinished = 0;
    dataSize = 0;
    channels = 0;
    nSamples = 0;

    while (!frameFinished)
    {
        int ret = av_read_frame(inFormatContext, &packet);
        if ( ret < 0)
        {
            if (ret == AVERROR_EOF || inFormatContext->pb->eof_reached)
            {
                eof = true;
            }
            else
            {
                av_strerror(ret, buf, sizeof(buf));
                cerr << "Error reading frame: " << buf << endl;
            }
            av_free_packet(&packet);
            break;
        }
        if ( packet.stream_index != streamIndex )
        {
            // Not the frame we're looking for; just read another
            av_free_packet(&packet);
            continue;
        }

        // Just skip this frame if there's a decode error
        ret = avcodec_decode_audio4(inCodecContext, decodedFrame, &frameFinished, &packet);
        if (ret < 0)
        {
            av_strerror(ret, buf, sizeof(buf));
            cerr << "Error decoding audio: " << buf << endl;
            av_free_packet(&packet);
            break;
        }

        if (frameFinished) {
            dataSize = av_samples_get_buffer_size(NULL, inCodecContext->channels,
                                                       decodedFrame->nb_samples,
                                                       inCodecContext->sample_fmt, 1);
            channels = inCodecContext->channels;

            // As necessary, convert to outSampleFmt and outMaxChannels
#if defined(HAVE_SWRESAMPLE)
            if (channels > outMaxChannels)
                channels = outMaxChannels;
            if (!resampleContext &&
                (inCodecContext->sample_fmt != outSampleFmt || inCodecContext->channels != channels))
            {
                int64_t inChannelLayout =
                    (inCodecContext->channel_layout && inCodecContext->channels == av_get_channel_layout_nb_channels(inCodecContext->channel_layout)) ?
                    inCodecContext->channel_layout : av_get_default_channel_layout(inCodecContext->channels);
                int64_t outChannelLayout = av_get_default_channel_layout(channels);

                resampleContext = swr_alloc_set_opts(NULL,
                                                 outChannelLayout, outSampleFmt, decodedFrame->sample_rate,
                                                 inChannelLayout, inCodecContext->sample_fmt, inCodecContext->sample_rate,
                                                 0, NULL);
                if (!resampleContext || swr_init(resampleContext) < 0)
                {
                    cerr << "Cannot create sample rate converter from "
                         << inCodecContext->sample_rate << " Hz "
                         << av_get_sample_fmt_name(inCodecContext->sample_fmt) << " "
                         << inCodecContext->channels << " channels to "
                         << inCodecContext->sample_rate << " Hz "
                         << av_get_sample_fmt_name(outSampleFmt) << " "
                         << channels << " channels." << endl;
                    av_free_packet(&packet);
                    break;
                }
            }

            uint8_t *pOutBuffer = outBuffer;
            if (resampleContext)
            {
                const uint8_t **in = const_cast<const uint8_t **>(decodedFrame->extended_data);
                int maxOutSamples = sizeof(outBuffer) / channels / outSampleSize;
                int nSamplesOut = swr_convert(resampleContext, &pOutBuffer, maxOutSamples, in, decodedFrame->nb_samples);
                if (nSamplesOut < 0)
                {
                    cerr << "swr_convert failed" << endl;
                    av_free_packet(&packet);
                    break;
                }
                nSamples = nSamplesOut;
                dataSize = nSamplesOut * channels * outSampleSize;
            }
            else
#elif defined(HAVE_AVRESAMPLE)
            if (channels > outMaxChannels)
                channels = outMaxChannels;
            if (!resampleContext &&
                (inCodecContext->sample_fmt != outSampleFmt || inCodecContext->channels != channels))
            {
                int64_t inChannelLayout =
                    (inCodecContext->channel_layout && inCodecContext->channels == av_get_channel_layout_nb_channels(inCodecContext->channel_layout)) ?
                    inCodecContext->channel_layout : av_get_default_channel_layout(inCodecContext->channels);
                int64_t outChannelLayout = av_get_default_channel_layout(channels);

                resampleContext = avresample_alloc_context();
                if ( ! resampleContext )
                {
                    cerr << "Cannot allocate AVAudioResampleContext" << endl;
                    av_free_packet(&packet);
                    break;
                }

                av_opt_set_int(resampleContext, "in_channel_layout",  inChannelLayout,             0);
                av_opt_set_int(resampleContext, "in_sample_fmt",      inCodecContext->sample_fmt,  0);
                av_opt_set_int(resampleContext, "in_sample_rate",     inCodecContext->sample_rate, 0);
                av_opt_set_int(resampleContext, "out_channel_layout", outChannelLayout,            0);
                av_opt_set_int(resampleContext, "out_sample_fmt",     outSampleFmt,                0);
                av_opt_set_int(resampleContext, "out_sample_rate",    inCodecContext->sample_rate, 0);

                // If both the input and output formats are s16 or u8, use s16
                // as the internal sample format
                if (av_get_bytes_per_sample(inCodecContext->sample_fmt) <= 2 &&
                    av_get_bytes_per_sample(outSampleFmt) <= 2)
                {
                    av_opt_set_int(resampleContext, "internal_sample_fmt", AV_SAMPLE_FMT_S16P, 0);
                }

                ret = avresample_open(resampleContext);
                if (ret < 0)
                {
                    cerr << "Error opening libavresample" << endl;
                    av_free_packet(&packet);
                    break;
                }
            }

            void *pOutBuffer = outBuffer;
            if (resampleContext)
            {
                void **in = (void**)decodedFrame->extended_data;
                int outLinesize;
                av_samples_get_buffer_size(&outLinesize,
                                            channels,
                                            decodedFrame->nb_samples,
                                            outSampleFmt, 0);
                int maxOutSamples = sizeof(outBuffer) / channels / outSampleSize;
                int nSamplesOut = avresample_convert(resampleContext, &pOutBuffer,
                                            outLinesize,
                                            maxOutSamples,
                                            in,
                                            decodedFrame->linesize[0],
                                            decodedFrame->nb_samples);
                if (nSamplesOut < 0)
                {
                    cerr << "avresample_convert failed" << endl;
                    av_free_packet(&packet);
                    break;
                }
                nSamples = nSamplesOut;
                dataSize = nSamplesOut * channels * outSampleSize;
            }
            else
#endif
            {
                nSamples = decodedFrame->nb_samples;
                memcpy(outBuffer, decodedFrame->data[0], dataSize);
            }
        }
        av_free_packet(&packet);
    }
    av_free(decodedFrame);
    return outBuffer;
}


LAV_Source::LAV_Source()
    : d(new LAV_SourcePrivate())
{
    av_register_all();
    avformat_network_init();
}


LAV_Source::~LAV_Source()
{
    release();
    avformat_network_deinit();
    delete d;
}

bool LAV_Source::eof() const
{
    return (d->eof || d->inFormatContext->pb->eof_reached);
}


void LAV_Source::init(const QString& fileName)
{
    // Assume that we want to start fresh
    if ( d->inFormatContext || d->inCodecContext )
        release();

    if ( avformat_open_input(&d->inFormatContext, QFile::encodeName(fileName), NULL, NULL ) < 0 )
    {
        throw std::runtime_error ("Cannot open the media file!");
    }
    if( avformat_find_stream_info(d->inFormatContext, NULL) < 0)
    {
        release();
        throw std::runtime_error ("Cannot find stream info in the file!");
    }

    // If there's a default video stream, we want the audio associated with it.
    // Otherwise just get the default audio stream.
    AVCodec* codec;
    int videoIndex = av_find_best_stream(d->inFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if ( videoIndex < 0 )
        videoIndex = -1;
    d->streamIndex = av_find_best_stream(d->inFormatContext, AVMEDIA_TYPE_AUDIO, -1, videoIndex, &codec, 0);

    if ( d->streamIndex < 0 )
    {
        release();
        throw std::runtime_error ("No audio streams found in the file!");
    }

    int64_t duration = d->inFormatContext->streams[d->streamIndex]->duration;

    // If the stream duration isn't available, get the duration from
    // AVFormatContext.  This should give us the right duration for 99 point
    // whatever percent of cases.
    if ( duration < 1 )
    {
        d->duration = d->inFormatContext->duration / AV_TIME_BASE;
    }
    else
    {
        AVRational timeBase = d->inFormatContext->streams[d->streamIndex]->time_base;
        d->duration = duration * timeBase.num / timeBase.den;
    }

    d->inCodecContext = d->inFormatContext->streams[d->streamIndex]->codec;
    if ( !avcodec_open2(d->inCodecContext, codec, NULL) < 0 )
    {
        release();
        throw std::runtime_error ("Unable to open a compatible audio codec for the file!");
    }

    if ( d->inCodecContext->bit_rate > 0 )
    {
        d->bitrate = d->inCodecContext->bit_rate;
    }

#if !defined(HAVE_SWRESAMPLE) && !defined(HAVE_AVRESAMPLE)
    if (d->inCodecContext->sample_fmt != outSampleFmt
        || d->inCodecContext->channels < 1
        || d->inCodecContext->channels > 2)
    {
        release();
        throw std::runtime_error ("The file has an incompatible sample format!");
    }
#endif

}


void LAV_Source::getInfo(int& lengthSecs, int& samplerate, int& bitrate, int& nchannels )
{
    lengthSecs = d->duration;
    samplerate = d->inCodecContext->sample_rate;
    bitrate = d->bitrate;

    nchannels = d->inCodecContext->channels;
#if defined(HAVE_SWRESAMPLE) || defined(HAVE_AVRESAMPLE)
    // What we promise to have if we need to convert the audio
    if (nchannels > outMaxChannels)
        nchannels = outMaxChannels;
#endif
}


void LAV_Source::release()
{
    if ( d->inCodecContext && d->inCodecContext->codec_id != CODEC_ID_NONE )
    {
        avcodec_close(d->inCodecContext);
    }
    if ( d->inFormatContext)
    {
        avformat_close_input(&d->inFormatContext);
    }
#if defined(HAVE_SWRESAMPLE) || defined(HAVE_AVRESAMPLE)
    if ( d->resampleContext)
    {
#if defined(HAVE_SWRESAMPLE)
        swr_free(&d->resampleContext);
#elif defined(HAVE_AVRESAMPLE)
        avresample_free(&d->resampleContext);
#endif
    }
#endif
    d->inCodecContext = NULL;
    d->inFormatContext = NULL;
    d->streamIndex = -1;
    d->duration = 0;
    d->bitrate = 0;
    d->eof = false;
    d->overflowSize = 0;
}


void LAV_Source::skipSilence(double silenceThreshold /* = 0.0001 */)
{
    silenceThreshold *= static_cast<double>( numeric_limits<short>::max() );

    int dataSize, channels, nSamples;
    uint8_t *out = d->decodeOneFrame(dataSize, channels, nSamples);
    while(dataSize > 0)
    {
        double sum = 0;
        int16_t *buf = (int16_t*)out;
        switch ( channels )
        {
            case 1:
                for (int j = 0; j < nSamples; j++)
                    sum += abs( buf[j] );
                break;
            case 2:
                for (int j = 0; j < nSamples; j+=2)
                    sum += abs( (buf[j] >> 1) + (buf[j+1] >> 1) );
                break;
        }
        if ( sum >= silenceThreshold * static_cast<double>(nSamples) )
        {
            break;
        }
        out = d->decodeOneFrame(dataSize, channels, nSamples);
    }

    avcodec_flush_buffers(d->inCodecContext);
}


void LAV_Source::skip(const int mSecs)
{
    char buf[256];
    if ( mSecs <= 0 || d->streamIndex < 0 || !d->inFormatContext )
        return;

    // Skip ahead from current frame position
    AVRational time_base = d->inFormatContext->streams[d->streamIndex]->time_base;
    int64_t frameNum = av_rescale(mSecs, time_base.den, time_base.num) + d->inCodecContext->frame_number;
    frameNum /= 1000;
    int ret = avformat_seek_file( d->inFormatContext, d->streamIndex, 0, frameNum, frameNum, 0);
    if (ret < 0)
    {
        // FIXME?
        // We might still get pretty close to where we want to be even if the
        // seek returns an error.
        av_strerror(ret, buf, sizeof(buf));
        cerr << "Problem seeking in file: " << buf << endl;
    }

    avcodec_flush_buffers(d->inCodecContext);
}


int LAV_Source::updateBuffer(signed short* pBuffer, size_t bufferSize)
{
    size_t bufferFill = 0;
    int dataSize;

    if ( d->overflowSize )
    {
        memcpy( pBuffer, d->overflow, d->overflowSize );
        bufferFill = d->overflowSize/outSampleSize;
        d->overflowSize = 0;
    }

    uint8_t * out;
    while(bufferFill < bufferSize)
    {
        int channels, nb_samples;

        out = d->decodeOneFrame(dataSize, channels, nb_samples);
        if (!dataSize)
            break;

        // Only put as many bytes in pBuffer as will fit; save the rest for the
        // next call to updateBuffer
        int bytesToBuffer = (bufferSize - bufferFill)*outSampleSize;

        if ( bytesToBuffer < dataSize )
        {
            d->overflowSize = dataSize - bytesToBuffer;
            memcpy( d->overflow, out + bytesToBuffer, d->overflowSize);
        }
        else
        {
            bytesToBuffer = dataSize;
        }
        memcpy( pBuffer + bufferFill, out, bytesToBuffer );

        bufferFill += bytesToBuffer/outSampleSize;
    }

    return bufferFill;
}
