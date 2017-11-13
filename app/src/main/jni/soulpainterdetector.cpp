#include <android/bitmap.h>
#include <android/log.h>

#include <jni.h>

#include <string>
#include <vector>

// ncnn
#include "net.h"

#include "lenet.id.h"

#include <sys/time.h>
#include <unistd.h>

static struct timeval tv_begin;
static struct timeval tv_end;
static double elasped;

static void bench_start()
{
    gettimeofday(&tv_begin, NULL);
}

static void bench_end(const char* comment)
{
    gettimeofday(&tv_end, NULL);
    elasped = ((tv_end.tv_sec - tv_begin.tv_sec) * 1000000.0f + tv_end.tv_usec - tv_begin.tv_usec) / 1000.0f;
//     fprintf(stderr, "%.2fms   %s\n", elasped, comment);
    __android_log_print(ANDROID_LOG_DEBUG, "SoulPaintDetector", "%.2fms   %s", elasped, comment);
}

static std::vector<unsigned char> param_bin;
static std::vector<unsigned char> model_bin;
static ncnn::Net ncnnNet;

static std::vector<std::string> split_string(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> strings;

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(delimiter, prev)) != std::string::npos)
    {
        strings.push_back(str.substr(prev, pos - prev));
        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.push_back(str.substr(prev));

    return strings;
}

extern "C" {

// public native boolean Init(byte[] param, byte[] bin, byte[] words);
JNIEXPORT jint JNICALL Java_com_ttpic_soulpainter_SoulPaintDetector_nativeInit(JNIEnv* env,
    jobject thiz,
    jbyteArray bin,
    jbyteArray param)
{
    // init param
    {
        int len = env->GetArrayLength(param);
        param_bin.resize(len);
        env->GetByteArrayRegion(param, 0, len, (jbyte*)param_bin.data());
        int ret = ncnnNet.load_param(param_bin.data());
        __android_log_print(ANDROID_LOG_DEBUG, "SoulPaintDetector", "load_param %d %d", ret, len);
    }

    // init bin
    {
        int len = env->GetArrayLength(bin);
        model_bin.resize(len);
        env->GetByteArrayRegion(bin, 0, len, (jbyte*)model_bin.data());
        int ret = ncnnNet.load_model(model_bin.data());
        __android_log_print(ANDROID_LOG_DEBUG, "SoulPaintDetector", "load_model %d %d", ret, len);
    }

    return 0;
}

// public native String Detect(Bitmap bitmap);
JNIEXPORT
jint
JNICALL
Java_com_ttpic_soulpainter_SoulPaintDetector_nativeDetect(JNIEnv* env,
    jobject thiz,
    jobject bitmap)
{
    bench_start();

    // ncnn from bitmap
    ncnn::Mat in;
    {
        AndroidBitmapInfo info;
        AndroidBitmap_getInfo(env, bitmap, &info);
        int width = info.width;
        int height = info.height;
        //if (width != 227 || height != 227)
          //  return -1;
        if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
            return -2;

        void* indata;
        AndroidBitmap_lockPixels(env, bitmap, &indata);

        in = ncnn::Mat::from_pixels((const unsigned char*)indata, ncnn::Mat::PIXEL_RGBA2GRAY, width, height);

        AndroidBitmap_unlockPixels(env, bitmap);
    }

    // ncnnNet
    std::vector<float> cls_scores;
    {
        const float mean_vals[3] = {104.f, 117.f, 123.f};
        in.substract_mean_normalize(mean_vals, 0);

        ncnn::Extractor ex = ncnnNet.create_extractor();
        ex.set_light_mode(true);
        ex.set_num_threads(4);

        ex.input(lenet_param_id::BLOB_data, in);

        ncnn::Mat out;
        ex.extract(lenet_param_id::BLOB_prob, out);

        cls_scores.resize(out.c);
        for (int j=0; j<out.c; j++)
        {
            const float* prob = out.data + out.cstep * j;
            cls_scores[j] = prob[0];
        }
    }

    // return top class
    int top_class = 0;
    float max_score = 0.f;
    for (size_t i=0; i<cls_scores.size(); i++)
    {
        float s = cls_scores[i];
        __android_log_print(ANDROID_LOG_DEBUG, "SoulPaintDetector", "%d %.20f", i, s);
        if (s > max_score)
        {
            top_class = i;
            max_score = s;
        }
    }
    bench_end("detect");
    __android_log_print(ANDROID_LOG_DEBUG, "SoulPaintDetector", "result: %d, max_score: %.20f", top_class, max_score);
    return top_class;
}

}
