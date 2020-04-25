//
// Created by 零蚀 on 2020-04-24.
//
#include <jni.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"zero_cpp",__VA_ARGS__)
extern "C" JNIEXPORT void JNICALL
Java_com_example_ffmpegdemo_util_FFmpegUtil_FFmpegCompressMov(
        JNIEnv *env,jobject obj,jobjectArray argv
        )
{

    int c = env->GetArrayLength(argv);
    char **v;
    int i;
    for (i = 0; i < c; i++) {
        jstring js = static_cast<jstring>(env->GetObjectArrayElement(argv, i));
        v[i] = (char*) env->GetStringUTFChars(js, 0);
    }
    LOGI("----------begin---------");
    // TODO 调用commond的main函数
}

