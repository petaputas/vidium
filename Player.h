#include <assert.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/ml.hpp>
typedef void * (*THREADFUNCPTR)(void *);
using namespace cv;
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <pthread.h>
}
#include "SDL2/SDL.h"
#include <unistd.h>
#include "SDL2/SDL_thread.h"
#include "SDL2/SDL_syswm.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_audio.h"
#define LOGI(x) std::cout << x << std::endl;
#define LOGE(x) std::cout << x << std::endl;
#define SDL_MAIN_HANDLED
#define ERROR_SIZE 128
#define FORMATO AV_PIX_FMT_RGB24
#define SDL_AUDIO_BUFFER_SIZE 1024;
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

typedef struct _AudioPacket
	{
		AVPacketList *first, *last;
		int nb_packets, size;
  		SDL_mutex *mutex;
  		SDL_cond *cond;
	} AudioPacket;

class Player {

public:





	

	static void openStream(std::string str);
	static void obtenerInfoDeStream(void);
	static int allocateMemory(void);
	static void  leerFramesDeVideo(void);
	static int crearVideo(void);
        static cv::Mat convertFrameToMat();
       static AVFrame *  convertMatToFrame(cv::Mat *  ptr);
        static  void    applyDetection(void);
        void setDetectionVisible();
        
       
        
        static bool isOpenedStream();
        
        static void applyDetectionFilter(bool ptr);


	static void initParameters();

        static void runDetectionThread(void);
        
        bool isDetectionVisible = false;
        
	
	static int getAudioPacket(AudioPacket*, AVPacket*, int);

		static void memsetAudioPacket(AudioPacket * pq);
    	//armazena o �ndice do determinado Stream a ser transmitido


    	//exibe o erro com rela��o ao seu respectivo c�digo
    	static void errorThrow(int erro);

    	static int obtenerCodecParametros(void);

    	static int leerCodecVideo(void);

    	static int PacketQueuePut(AudioPacket *, const AVPacket *);

    	static void initAudioPacket(AudioPacket *);

    	static int putAudioPacket(AudioPacket *, AVPacket *);




};