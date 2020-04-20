//
// Created by 零蚀 on 2020-04-18.
//

#include <android/log.h>


#include <SDL.h>
#include <SDL_main.h>


#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"zero_main" ,__VA_ARGS__) // 定义LOGI类型

int main(int argc, char *argv[]){

    LOGI("开始了主函数");
    /**
     * step 1 开始初始化
     */
    int init=SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER);
    if(init==-1){
        LOGI("SDL_Init failed %s", SDL_GetError());
        LOGI("SDL初始化失败");
        return 0;
    }
    LOGI("SDL初始化成功");

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

    /**
     *step 5 读取本地路径
     */

    FILE *file=fopen("/storage/emulated/0/ffmpeg/shoe.yuv","rb+");
    if(file==NULL){
        LOGI("打开YUV视频文件失败");
    }
    LOGI("打开YUV视频文件");

    /**
     * step 6 循环读取每一帧的像素数据 & 显示
     */

    SDL_Rect rect ;

    rect.x=0;
    rect.y=0;
    rect.w=screen_width*2;
    rect.h=screen_height*2;
    char *buffer_pixels[screen_width*screen_height*3/2]; // YUV=4:1:1,y=w*h ,yuv=1.5 * w * h
    LOGI("1");
    while(true){
        // 读取文件
        fread(buffer_pixels, 1, static_cast<size_t>(screen_width * screen_height * 3/2), file);
        // 文件是否读取（播放）完毕
        if(feof(file)) {
            break;
        }
        // 更新纹理数据,
        // 参数二：Rect渲染的矩形区域，NULL为默认视频宽高
        // 参数三：帧数据
        // 帧画面的linesize(宽=width)
        LOGI("2");


        int update=SDL_UpdateTexture(texture,NULL,buffer_pixels,screen_width);

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

    fclose(file);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_Quit();
    return 0;
}


