//
// Created by 零蚀 on 2020-04-21.
//


#include <jni.h>
#include <android/log.h>
#include <unwind.h>
#include <setjmp.h>


#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "zero_cpp", __VA_ARGS__);
#define max_value_sample_rate (44100*2)


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <SDL_stdinc.h>
}




extern "C" JNIEXPORT void JNICALL
Java_com_example_ffmpegdemo_util_FFmpegUtil_FFmpegAudio(JNIEnv *env, jobject obj) {
    jclass pJclass = env->GetObjectClass(obj);
    jmethodID methodId = env->GetMethodID(pJclass, "getMovPath", "()Ljava/lang/String;");
    jmethodID methodId2 = env->GetMethodID(pJclass, "call_back", "(I)V");
    jstring path = static_cast<jstring>(env->CallObjectMethod(obj, methodId));
    const char *mov_path = env->GetStringUTFChars(path, 0);


    /**
     * step 1 打开音频文件
     */

    AVFormatContext *fmtContext = avformat_alloc_context();
    if (avformat_open_input(&fmtContext, mov_path, NULL, NULL) != 0) {
        LOGI("打开失败");
        env->CallObjectMethod(obj, methodId2, 301);

    }

    /**
     *step 2 获取音频信息
     */
    int fmt = avformat_find_stream_info(fmtContext, NULL);
    if (fmt < 0) {
        LOGI("获取音频失败");
        char *errorInfo = nullptr;
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "获取音频失败,错误信息：%c", *errorInfo);
        env->CallObjectMethod(obj, methodId2, 302);
    }

    /**
     * step 3 查找解码器
     */
    //查找音频流索引
    int av_audio_index = av_find_best_stream(fmtContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL,0);
    //查找音频流
    if (av_audio_index == -1) { //没有找到视频流
        LOGI("没找到音频流的位置");
        env->CallObjectMethod(obj, methodId2, 303);
    }
    LOGI("获取音频流成功");

    /**
     * step 4 找到解码器信息,得到的解码器
     */

    AVCodecParameters *pCodecParam = fmtContext->streams[av_audio_index]->codecpar;

    //申请内存空间
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);

    if (avcodec_parameters_to_context(pCodecCtx, pCodecParam) < 0) {
        LOGI("不存在解码器");
        env->CallObjectMethod(obj, methodId2, 304);
    }
    LOGI("找到解码器");
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    /**
     * step 5 打开解码器
     */

    int avOpen = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (avOpen != 0) {
        LOGI("解码器打开失败");
        env->CallObjectMethod(obj, methodId2, 305);
    }
    LOGI("打开解码器");
    /**
     * step 6  循环读取一帧的数据
     * 提供缓冲区
     */

    // 输出文件的路径
    const char *point_path = "/storage/emulated/0/ffmpeg/shoe.pcm";
    FILE *file = fopen(point_path, "wb+");
    if (file == NULL) {
        LOGI("文件不存在");
        env->CallObjectMethod(obj, methodId2, 306);

    }

    AVFrame *avFrame = av_frame_alloc(); // 缓存一帧的数据，解码前的数据
    AVPacket *avPacket = (AVPacket *) (av_malloc(sizeof(AVPacket)));

    // 缓冲区
    // 大小(HZ 16bit) = HZ*2/1024
    uint8_t *out = (uint8_t *) av_malloc(max_value_sample_rate);


    // 音频采样数据(音频的设置)
    SwrContext *swrCtx = swr_alloc();
    // 参数一：上下文
    // 参数二：输出的声道布局(单声道，双声道 ,参考channel_layout.h)
    // 参数三：输出的采样格式（采样精度，参考AVSampleFormat）
    // 参数四：输出的采样率。（max = 48000Hz,这里保持一致，如果不一致需要函数转化采样率，参考resampling_audio.h）
    // 参数五：输入的声道布局
    int64_t in_audio_channel_layout = av_get_default_channel_layout(pCodecCtx->channels);
    // 参数六：输入的采样格式
    // 参数七：输入的采样率
    // 参数八：偏移量
    // 参数九：自制上下文

    SwrContext *swrContext=swr_alloc_set_opts(
            swrCtx,
            AV_CH_LAYOUT_STEREO,
            AV_SAMPLE_FMT_S16,
            pCodecCtx->sample_rate,
            in_audio_channel_layout,
            pCodecCtx->sample_fmt,
            pCodecCtx->sample_rate,
            0,
            NULL
    );

    // 初始化上下文
    swr_init(swrContext);
    LOGI("音频配置结束")

    int frame_cnt = 0; // 记录帧数
    while (av_read_frame(fmtContext, avPacket) >= 0) {
        LOGI("----------------")
        // 开始不断读取每一帧数据
        int ret = avcodec_send_packet(pCodecCtx, avPacket);
        if (avPacket->stream_index == av_audio_index) { //判断是否为音频流

            if (ret < 0) {
                break;
            }

            // 解析每一帧的数据到avFrame
            while (avcodec_receive_frame(pCodecCtx, avFrame) == 0) {

                // 在这里音频最后一帧可能结束了，但是还是能receive到数据
                if(**(const uint8_t **) avFrame->data==0){
                    LOGI("音频数据读取结束")
                    break;
                }


                // 音频采样数据格式是PCM格式
                frame_cnt++;

                int channel_audio_nb = av_get_channel_layout_nb_channels(
                        (uint64_t) in_audio_channel_layout);
                int really_size = av_samples_get_buffer_size(NULL, channel_audio_nb,
                                                             avFrame->nb_samples, AV_SAMPLE_FMT_S16,
                                                             1);


                swr_convert(
                        swrContext,
                        &out,
                        really_size,
                        (const uint8_t **) avFrame->data,
                        avFrame->nb_samples
                );




                // 获取缓冲区填充后的有数据的空间大小
                // 参数 ：行大小，声道数量，输入大小,输出的音频的格式,字节对齐


                fwrite(out, 1, (size_t) really_size, file);

            }
        }

        av_packet_unref(avPacket);

    }

    //关闭流
    fclose(file);
    av_frame_free(&avFrame);
    av_free(out);
    swr_free(&swrContext);
    avcodec_free_context(&pCodecCtx);
    avformat_free_context(fmtContext);

    env->CallObjectMethod(obj, methodId2, 300);

}
