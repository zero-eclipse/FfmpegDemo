#include <jni.h>
#include <string>
#include <android/log.h>


#define TAG "zero-cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <SDL_stdinc.h>
}

// 获取视频信息
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ffmpegdemo_util_FFmpegUtil_FFmpegMovInfo(JNIEnv *env,jobject obj){
    using namespace std;
    jclass pJclass = env->GetObjectClass(obj);
    jmethodID methodId = env->GetMethodID(pJclass, "getMovPath", "()Ljava/lang/String;");
    jstring path = static_cast<jstring>(env->CallObjectMethod(obj, methodId));
    const char *mov_path = env->GetStringUTFChars(path, 0);
    __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "sdcard: %s", mov_path);
    // ---------- ffmpeg-------------
    // 第一步：封装
    AVFormatContext *fmtContext = avformat_alloc_context();
    int result = avformat_open_input(&fmtContext, mov_path, 0, 0);
    if (result != 0) {
        LOGI("打开失败");
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "result=%d", result);
        return env->NewStringUTF("文件打开失败");
    }
    LOGI("打开成功");

    //第二步：获取文件信息
    int fmt = avformat_find_stream_info(fmtContext, NULL);
    if (fmt < 0) {
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "获取视频失败");
        char *errorInfo = nullptr;
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "获取视频失败,错误信息：%c", *errorInfo);
        return env->NewStringUTF("获取视频失败");
    }






    // 第三步：遍历流，找到我们需要的流
    //视频的位置索引
    int av_index = av_find_best_stream(fmtContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);//查找视频流索引
    if (av_index == -1) { //没有找到视频流
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "没找到视频流的位置");
        return env->NewStringUTF("没找到视频流的位置");
    }

    LOGI("获取视频流成功");
    // 第四步：找到解码器信息
    //得到的解码器
    AVCodecParameters *pCodecParam = fmtContext->streams[av_index]->codecpar;

    //申请内存空间
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);

    if (avcodec_parameters_to_context(pCodecCtx, pCodecParam) < 0) {

        LOGI("不存在解码器");
        return env->NewStringUTF("不存在解码器");

    }




    LOGI("打开解码器成功");

    char buff[1024];
    char width[10];
    char height[10];
    char time[10];
    SDL_itoa(pCodecCtx->width, width, 10); // 值；转换的数组；进制
    SDL_itoa(pCodecCtx->height, height, 10);
    SDL_itoa(static_cast<int>(fmtContext->duration), time, 10);
    strcat(buff, "width= <");
    strcat(buff, width);
    strcat(buff, " > height= <");
    strcat(buff, height);
    strcat(buff, " > duration= <");
    strcat(buff, time);
    strcat(buff, " >");
    auto jInfo = env->NewStringUTF(buff);

    avcodec_free_context(&pCodecCtx);

    return jInfo;
};

// SDL播放本地YUV视频
extern "C" JNIEXPORT void JNICALL
Java_com_example_ffmpegdemo_util_FFmpegUtil_FFmpegMov2YUV(
        JNIEnv *env,
        jobject obj) {
    using namespace std;
    jclass pJclass = env->GetObjectClass(obj);
    jmethodID methodId = env->GetMethodID(pJclass, "getMovPath", "()Ljava/lang/String;");
    jmethodID methodId2 = env->GetMethodID(pJclass, "call_back", "()V");
    jstring path = static_cast<jstring>(env->CallObjectMethod(obj, methodId));
    const char *mov_path = env->GetStringUTFChars(path, 0);
    __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "sdcard: %s", mov_path);
    // ---------- ffmpeg-------------
    // 第一步：封装
    AVFormatContext *fmtContext = avformat_alloc_context();
    int result = avformat_open_input(&fmtContext, mov_path, 0, 0);
    if (result != 0) {
        LOGI("打开失败");
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "result=%d", result);
    }
    LOGI("打开成功");

    //第二步：获取文件信息
    int fmt = avformat_find_stream_info(fmtContext, NULL);
    if (fmt < 0) {
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "获取视频失败");
        char *errorInfo = nullptr;
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "获取视频失败,错误信息：%c", *errorInfo);
    }





    // 第三步：遍历流，找到我们需要的流
    //视频的位置索引
    int av_index = av_find_best_stream(fmtContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);//查找视频流索引
    if (av_index == -1) { //没有找到视频流
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "没找到视频流的位置");
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
        __android_log_print(ANDROID_LOG_INFO, "zero_cpp", "解码器打开失败");
    }

    LOGI("打开解码器成功");

    char buff[1024];
    char width[10];
    char height[10];
    char time[10];
    SDL_itoa(pCodecCtx->width, width, 10); // 值；转换的数组；进制
    SDL_itoa(pCodecCtx->height, height, 10);
    SDL_itoa(static_cast<int>(fmtContext->duration), time, 10);
    strcat(buff, "width= <");
    strcat(buff, width);
    strcat(buff, " > height= <");
    strcat(buff, height);
    strcat(buff, " > duration= <");
    strcat(buff, time);
    strcat(buff, " >");
    auto jInfo = env->NewStringUTF(buff);



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
                __android_log_print(ANDROID_LOG_INFO,"zero","------------------------------------------------");

                __android_log_print(ANDROID_LOG_INFO,"zero","avFrame->data[0]=%d",avFrame->data[0]);
                __android_log_print(ANDROID_LOG_INFO,"zero","avFrame->data[1]=%d",avFrame->data[1]);
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


                // 计算亮度Y，y=width*height
                y_size = pCodecCtx->width * pCodecCtx->height;
                // 计算 UV他们都是Y的1/4
                u_size = y_size / 4;
                v_size = y_size / 4;

                // 写入文件
                fwrite(avFrame_YUV420P->data[0], 1, y_size, file);
                fwrite(avFrame_YUV420P->data[1], 1, u_size, file);
                fwrite(avFrame_YUV420P->data[2], 1, v_size, file);
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

    LOGI("内容已经完成");
    env->CallObjectMethod(obj, methodId2 , 200);

}




