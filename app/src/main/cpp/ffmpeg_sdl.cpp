//
// Created by 零蚀 on 2020-04-20.
//

#include <jni.h>
#include <android/log.h>

#define TAG "zero_cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)

extern "C"{
#include <SDL.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};

int main(int argc,char *argv[]){

    // -------------------- ffmpeg --------------------

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
        LOGI( "获取视频失败");
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

    // -------------------- SDL --------------------

    /**
     * SDL初始化
     */

    if(SDL_Init(SDL_INIT_EVERYTHING)==-1){
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


    // -------------------- 将数据转为yuv420p保存在本地
    // 第六步： 循环读取一帧的数据
    //打开文件
    const char *point_path = "/storage/emulated/0/ffmpeg/shoe.yuv";
    FILE *file = fopen(point_path, "wb");
    if (file == NULL) {
        LOGI("文件不存在");

    }


    int y_size = 0, u_size = 0, v_size = 0;
    AVFrame *avFrame = av_frame_alloc(); // 缓存一帧的数据，解码前的数据
    AVFrame *avFrame_YUV420P = av_frame_alloc(); // 输出的格式类型缓冲区，解码后的数据

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
    SDL_Rect rect ;

    rect.x=0;
    rect.y=0;
    rect.w=screen_width*2;
    rect.h=screen_height*2;

    int frame_cnt=0; // 记录帧数
    while (av_read_frame(fmtContext,avPacket) >= 0) {

        // 开始不断读取每一帧数据
        int ret = avcodec_send_packet(pCodecCtx, avPacket);
        if(avPacket->stream_index == av_index){ //判断是否为视频流
            if(ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                break;
            }

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
                frame_cnt++;

                // ---------------------------------- SDL 开始渲染 ----------------------------------
                // 这里传入的数据是Y（data[0]）
                int update=SDL_UpdateTexture(texture,NULL,avFrame_YUV420P->data[0],avFrame_YUV420P->linesize[0]);

                LOGI("update= %d",update);
                // 清空帧画面
                LOGI("3");
                SDL_RenderClear(renderer);
                //重新绘制

                LOGI("4");
                SDL_RenderCopy(renderer,texture,NULL,&rect);
                LOGI("5");
                // 显示播放
                SDL_RenderPresent(renderer);
                LOGI("6");
                // 播放帧的速度
                SDL_Delay(20);
            }
        }

        av_packet_unref(avPacket);

    }
    //关闭流
    fclose(file);
    av_frame_free(&avFrame);
    av_frame_free(&avFrame_YUV420P);
    avcodec_free_context(&pCodecCtx);
    avformat_free_context(fmtContext);



}