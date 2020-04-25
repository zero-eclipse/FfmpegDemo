//
// Created by 零蚀 on 2020-04-24.
//


#include <android/log.h>

#define LOG_I_ARGS(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"main",FORMAT,__VA_ARGS__);
#define LOG_I(FORMAT) LOG_I_ARGS(FORMAT,0);

#include "SDL.h"


//播放音频文件

//读到那个位置
static Uint8 * audio_pos;
//读取长度
static int audio_len;

//播放单个声音节点需要读取多少帧
void fill_audio(void *userdata, Uint8 * stream, int len){
    //读取数据
    SDL_memset(stream,0,len);
    if (audio_len == 0){
        //读取完毕
        return;
    }
    len=(len>audio_len?audio_len:len);
    //SDL_MixAudio: 完成混音
    //参数一：数据源
    //参数二：读取到那一个位置
    //参数三：总共读取的长度
    //参数四：音量
    SDL_MixAudio(stream,audio_pos,AUDIO_S16SYS,len,SDL_MIX_MAXVOLUME);
    //指针往下移动
    audio_pos+=len;
    audio_len-=len;
}

int main(int argc, char *argv[]) {
    //测试是否通过
    //第一步：初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1) {
        LOG_I_ARGS("SDL_Init failed %s", SDL_GetError());
        return 0;
    }

    //第二步：打开音频设备(打开PCM格式)
    //参数一：打开的音频设备信息
    SDL_AudioSpec desired;
    //采样率
    desired.freq = 44100;
    //音频数据格式(16位)
    desired.format = AUDIO_S16SYS;
    //声道数量(双声道：立体声)
    desired.channels = 2;
    //帧大小（音频缓冲区采样个数）
    //AAC: 1024   MP3: 1152
    desired.samples = 1024;
    //回调函数
    desired.callback = fill_audio;
    //打开设备
    if (SDL_OpenAudio(&desired,NULL) < 0){
        return -1;
    }

    LOG_I("SDL_Init Success!");

    //打开文件
    FILE* file_pcm = fopen("/storage/emulated/0/ffmpeg/shoe.pcm","rb+");
    if (file_pcm == NULL){
        LOG_I("文件不存在");
        return -1;
    }

    //开始播放
    SDL_PauseAudio(0);

    //定义缓冲区大小(固定的4096)
    int pcm_buffer_size = 4096;
    //定义缓冲区
    char* pcm_buffer = (char*)SDL_malloc(pcm_buffer_size);

    //读取数据
    int frame_index = 0;
    while (true){
        fread(pcm_buffer,1,pcm_buffer_size,file_pcm);
        if(feof(file_pcm)){
            break;
        }

        //继续读区音频采样数据
        audio_pos = (Uint8*)pcm_buffer;
        audio_len = pcm_buffer_size;

        frame_index++;
        LOG_I_ARGS("当前帧：%d",frame_index);

        //什么时候读取下一部分数据
        //直到缓冲区数据全部读取完毕用于完成混音，才读取下一帧pcm数据
        if (audio_len > 0){
            SDL_Delay(25);
        }
    }

    //释放资源
    free(pcm_buffer);
    fclose(file_pcm);

    SDL_Quit();
    return 0;
}

