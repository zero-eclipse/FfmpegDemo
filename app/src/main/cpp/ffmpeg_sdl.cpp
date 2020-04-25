//
// Created by 零蚀 on 2020-04-20.
//

#include <jni.h>
#include <android/log.h>


#define TAG "zero_cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define MAX_AUDIO_FRAME_SIZE (44100 * 2)
extern "C" {
#include <libswresample/swresample.h>
#include <SDL.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};



/**
 * 回调函数
 * 播放一个声音节点，需要通过fill_audio读取很多帧
 * @param userdata
 * @param stream
 * @param len
 * @return
 */

//读到那个位置
static Uint8 *audio_pos;
//读取长度
static int audio_len;

//播放单个声音节点需要读取多少帧
void fill_audio(void *userdata, Uint8 *stream, int len) {
    //读取数据
    SDL_memset(stream, 0, len);
    if (audio_len == 0) {
        //读取完毕
        return;
    }
    len = ((Uint32) len > audio_len ? audio_len : len);
    //SDL_MixAudio: 完成混音
    //参数一：数据源
    //参数二：读取到那一个位置
    //参数三：总共读取的长度
    //参数四：音量
    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    //指针往下移动
    audio_pos += len;
    audio_len -= len;
}


int main(int argc, char *argv[]) {

    // -------------------- ffmpeg 打开视频流 --------------------

    // 第一步：封装
    AVFormatContext *fmtContext = avformat_alloc_context();
    int result = avformat_open_input(&fmtContext, "/storage/emulated/0/ffmpeg/shoe.mp4", 0, 0);
    if (result != 0) {
        LOGI("打开失败");
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "result=%d", result);
    }
    LOGI("打开成功");

    //第二步：获取文件信息
    int fmt = avformat_find_stream_info(fmtContext, NULL);
    if (fmt < 0) {
        LOGI("获取视频失败");
        char *errorInfo = nullptr;
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "获取视频失败,错误信息：%c", *errorInfo);
    }

    // 第三步：遍历流，找到我们需要的流
    //视频的位置索引
    int av_index = av_find_best_stream(fmtContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);//查找视频流索引


    if (av_index == -1) { //没有找到视频流
        LOGI("没找到视频流的位置");
    }

    LOGI("获取视频流成功");
    // 第四步：找到解码器信息
    //得到的解码器
    AVCodecParameters *pCodecParam = fmtContext->streams[av_index]->codecpar;

    //申请内存空间
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);

    if (avcodec_parameters_to_context(pCodecCtx, pCodecParam) < 0) {

        LOGI("不存在解码器");

    }

    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    LOGI("成功找到解码器");


    // 第五步 ：打开解码器
    int avOpen = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (avOpen != 0) {
        LOGI("解码器打开失败");
    }

    AVFrame *avFrame = av_frame_alloc(); // 缓存一帧的数据，解码前的数据
    AVFrame *avFrame_YUV420P = av_frame_alloc(); // 输出的格式类型缓冲区，解码后的数据
    AVFrame *audio_in_frame = av_frame_alloc();


    // -------------------- SDL 打开音频--------------------
    //查找音频流索引
    int av_audio_index = av_find_best_stream(fmtContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    // 第四步：找到音频解码器信息
    //得到的解码器
    AVCodecParameters *pAudioCodecParam = fmtContext->streams[av_audio_index]->codecpar;

    //申请内存空间
    AVCodecContext *audio_av_cd_ctx = avcodec_alloc_context3(NULL);

    if (avcodec_parameters_to_context(audio_av_cd_ctx, pAudioCodecParam) < 0) {

        LOGI("不存在解码器");

    }

    AVCodec *pAudioCodec = avcodec_find_decoder(audio_av_cd_ctx->codec_id);

    // 第五步 ：打开解码器
    int avAudioOpen = avcodec_open2(audio_av_cd_ctx, pAudioCodec, NULL);
    if (avAudioOpen != 0) {
        LOGI("解码器打开失败");
    }



    //输出的声道布局（立体、环绕..）
    int64_t audio_out_ch_layout = AV_CH_LAYOUT_STEREO;
    //输出音频的采样格式 16位 R L R L
    AVSampleFormat audio_out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输出音频采样率仍然等于输入的音频采样率
    //如果一定要转，要进行音频重采样
    int audio_out_sample_rate = audio_av_cd_ctx->sample_rate;


    //根据输出的声道布局获取输出的声道个数
    int audio_out_nb_channels = av_get_channel_layout_nb_channels(audio_out_ch_layout);

    int audio_out_nb_samples = audio_av_cd_ctx->frame_size;



    //输入声道布局
    int64_t audio_in_ch_layout = av_get_default_channel_layout(audio_av_cd_ctx->channels);

    //SwrContext:音频采样数据上下文
    SwrContext *audio_swrCtx = swr_alloc();
    //swr_alloc_set_opts: 设置音频采样配置(例如：输出声道、采样格式、采样率等等......)
    //参数一：音频采样上下文
    //参数二：输出声道布局
    //参数三：输出音频的采样格式
    //参数四：输出音频采样率
    //参数五：输入声道布局
    //参数六：输入音频采样格式
    //参数七：输入音频采样率
    //参数八：偏移量
    //参数九：日志上下文
    swr_alloc_set_opts(audio_swrCtx,
                       audio_out_ch_layout,
                       audio_out_sample_fmt,
                       audio_out_sample_rate,
                       audio_in_ch_layout,
                       audio_av_cd_ctx->sample_fmt,
                       audio_av_cd_ctx->sample_rate,
                       0,
                       NULL);
    swr_init(audio_swrCtx);

    //转换时用的缓冲区
    //44100 32bit
    //大小: size = 48000 * 4 / 1024 = 187KB
    uint8_t *audio_out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRAME_SIZE);

    //参数设置
    SDL_AudioSpec desired;
    //采样率
    desired.freq = audio_av_cd_ctx->sample_rate;
    //音频数据格式
    desired.format = AUDIO_S16SYS;
    //声道个数
    desired.channels = static_cast<Uint8>(audio_out_nb_channels);
    //音频缓冲区的采样个数(帧大小)
    //AAC:1024  MP3:1152
    desired.samples = audio_out_nb_samples;
    //回调函数
    desired.callback = fill_audio;
    desired.userdata = audio_av_cd_ctx;

    //2.打开音频设备
    if (SDL_OpenAudio(&desired, NULL) < 0) {
        printf("无法打开音频设备.\n");
        return -1;
    }

    // ------------------audio-stop ----------------------
    // -------------------- SDL 渲染视频--------------------

    /**
     * SDL初始化
     */

    if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
        LOGI("SDL 初始化失败");
    }

    /**
    * step 2 创建窗口
    * title 窗体的名字
    * x 窗体的x坐标
    * y 窗体的y坐标
    * w width
    * h height
    * flags 窗体的状态
    */
    int screen_width = 544, screen_height = 544; // yuv的宽高
    SDL_Window *window = SDL_CreateWindow("自定义了一个窗体",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          screen_width,
                                          screen_height,
                                          SDL_WINDOW_OPENGL);

    if (window == NULL) {
        LOGI("创建窗口失败");
    }
    LOGI("创建窗口成功");


    /**
     * step 3 创建渲染器
     * window 窗口
     * index -1 位置起始位置（-1从头开始）
     * flag 用什么方式 参考 SDL_RendererFlags
     */

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    /**
     * step 4 创建纹理
     * 渲染器
     * 纹理格式 YUV420P
     * 绘制方式 参考SDL_TextureAccess
     */

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_IYUV,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             screen_width,
                                             screen_height
    );



    // 像素格式 ，像素的宽，高，位置(通用的设置为1)
    //缓冲数组（AVFrame中的data指向的内存）
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
                                               pCodecCtx->height, 1);
    uint8_t *out = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(buffer_size)));



    //只是用来分配内存，缓冲数组和pFrameYUV联系起来
    av_image_fill_arrays(
            avFrame_YUV420P->data,  // 填充的数据
            avFrame_YUV420P->linesize, //每一行的大小
            out,  //输出的缓冲区
            AV_PIX_FMT_YUV420P,  // 数据格式
            pCodecCtx->width,
            pCodecCtx->height,
            1
    );


    LOGI("数据填充成功");

    AVPacket *avPacket = (AVPacket *) (av_malloc(sizeof(AVPacket)));

    // 转格式需要的数据
    SwsContext *swsContext = sws_getContext(
            pCodecCtx->width,
            pCodecCtx->height,
            pCodecCtx->pix_fmt,
            pCodecCtx->width,   // 输出宽度
            pCodecCtx->height,  // 输出高度
            AV_PIX_FMT_YUV420P, // 输出格式
            SWS_BICUBIC  // 选择一个适合的格式转换算法
            , nullptr, nullptr, nullptr   // 算法

    ); // 上下文

    LOGI("数据转格式成功");
    SDL_Rect rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = screen_width * 2;
    rect.h = screen_height * 2;

    //开始播放
    SDL_PauseAudio(0);

    int frame_cnt = 0; // 记录帧数
    while (av_read_frame(fmtContext, avPacket) >= 0) {


        if (avPacket->stream_index == av_audio_index) { // 判断是否是音频流
            // 开始不断读取每一帧数据
            avcodec_send_packet(audio_av_cd_ctx, avPacket);
            // 解析每一帧的数据到avFrame
            while (avcodec_receive_frame(audio_av_cd_ctx, audio_in_frame) == 0) {
                // 在这里音频最后一帧可能结束了，但是还是能receive到数据
                if (**(const uint8_t **) audio_in_frame->data == 0) {
                    LOGI("音频数据读取结束");
                    break;
                }



                //读取out_buffer中有效的音频采样数据，写入pcm文件中
                //参数一：行大小
                //参数二：输出的声道个数
                //参数三：输入大小
                //参数四：输出音频的采样格式
                //参数五：字节对齐方式
                int out_buffer_size = av_samples_get_buffer_size(
                        NULL,
                        audio_out_nb_channels,
                        audio_out_nb_samples,
                        audio_out_sample_fmt,
                        1);

                swr_convert(audio_swrCtx,
                            &audio_out_buffer,
                            out_buffer_size,
                            (const uint8_t **) audio_in_frame->data,
                            audio_in_frame->nb_samples);


                audio_pos = audio_out_buffer;
                audio_len = out_buffer_size;

                SDL_Delay(14);

            }
        }

        if (avPacket->stream_index == av_index) { //判断是否为视频流
            // 开始不断读取每一帧数据
            avcodec_send_packet(pCodecCtx, avPacket);
            // 解析每一帧的数据到avFrame
            while (avcodec_receive_frame(pCodecCtx, avFrame) == 0) {

                // 将AVFrame转换为YUV420P
                // 上下文，像素数据，输入每一行的大小，输入开始的位置，输入画面每一行的转码的位置，输出的数据，输出每一行的大小
                sws_scale(
                        swsContext,
                        avFrame->data,
                        avFrame->linesize,
                        0,
                        pCodecCtx->height,
                        avFrame_YUV420P->data,
                        avFrame_YUV420P->linesize);

                // ---------------------------------- SDL 开始渲染 ----------------------------------
                // 这里传入的数据是Y（data[0]）
                SDL_UpdateTexture(texture, NULL, avFrame_YUV420P->data[0],
                                  avFrame_YUV420P->linesize[0]);

                // 清空帧画面
                SDL_RenderClear(renderer);
                //重新绘制

                SDL_RenderCopy(renderer, texture, NULL, &rect);
                // 显示播放
                SDL_RenderPresent(renderer);
                // 播放帧的速度
                // SDL_Delay(25);
            }
        }

    }

    //关闭流
    av_packet_free(&avPacket);
    av_frame_free(&avFrame);
    av_frame_free(&avFrame_YUV420P);
    av_frame_free(&audio_in_frame);
    av_free(audio_out_buffer);
    av_free(out);
    avcodec_free_context(&audio_av_cd_ctx);
    avcodec_free_context(&pCodecCtx);
    avformat_free_context(fmtContext);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}