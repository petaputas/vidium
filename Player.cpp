#include "Player.h"
#include "selective.h"

using namespace std;

//struct para audio
struct SwrContext *swrCtx = NULL;
AVFrame wanted_frame; 
AudioPacket audioq;
static AVFrame * frame = av_frame_alloc();
static int videoStream = -1;
static pthread_mutex_t  * p;
 static bool ptr_op = false;
 static bool objectFilter = false;
static int counts = 0;



	//stream de audio
	static int audioStream = -1;

	//contem informa��es sobre o arquivo de v�deo, incluindo os codecs, etc
	static AVFormatContext *pFormatCtx = avformat_alloc_context();

	//contem informa��es do codec do v�deo, obtidas atraves de
	//pFormatCtx->streams[i]->codecpar
	//olhando o codec_type e vendo se � transmissao de video do tipo AVMEDIA_TYPE_VIDEO

	//informa��es do codecParameters, por�m copiadas. o pCodecParameters serve como um backup das informa��es do v�deo
	static AVCodecContext *pCodecCtx  = NULL;

	static AVCodecContext *pCodecAudioCtx = NULL;

	static SDL_AudioSpec wantedSpec = { 0 }, audioSpec = { 0 };

	//guarda o codec do v�deo
	static AVCodec *pCodec = NULL;

	//guarda o codec do audio
	static AVCodec *pAudioCodec = NULL;

	//estrutura que guarda o frame RGB
	static AVFrame *pFrameRGB = av_frame_alloc();

	//buffer para leitura dos frames
	static uint8_t *buffer = NULL;
	static AVFrame * pFrame = av_frame_alloc();
static AVFrame * result_ptr = av_frame_alloc();
static AVFrame *dst  = av_frame_alloc();
static AVFrame* ptrYuv = av_frame_alloc();

	//Audio COdec Parametrs
	//estrutura que armazena a convers�o para RGB
	static struct SwsContext *sws_ctx;

	SDL_Event evt;

	//surface window para exibir o video
	//pode ter m�ltiplas screens
    static SDL_Window *screen;

	static SDL_Renderer *renderer;

	static SDL_Texture* bmp;
void audio_callback(void*, Uint8*, int);
static cv::Mat ptr;



void Player::applyDetectionFilter(bool ptr)
{
    objectFilter = ptr;
}
bool Player::isOpenedStream()
{
    return (ptr_op);
}
//exibe informa��o dos streams do arquivo de v�deo espec�fico
void Player::obtenerInfoDeStream(void) {

	av_dump_format(pFormatCtx, 0, pFormatCtx->filename, 0);

}

//exibe o erro, descrevendo-o
void Player::errorThrow(int erro) {

	char errobuf[ERROR_SIZE];
	av_strerror(erro, errobuf, ERROR_SIZE);
	LOGI(errobuf);

}

//obtem o stream do video
int Player::obtenerCodecParametros(void) {

  int videoStream = -1;
	for ( int i = 0 ; i < pFormatCtx->nb_streams; i++ )
        {
            if ( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                videoStream = i;
                
            }
            if ( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                audioStream = i;
            }
        }
        
        if ( videoStream != -1 )
        {
            LOGI("Video context assigned = !=NULL");
            pCodecCtx =  (AVCodecContext  *  )pFormatCtx->streams[videoStream]->codec;
            
        }if ( audioStream != -1 )
        {
            pCodecAudioCtx =  (AVCodecContext  * )pFormatCtx->streams[audioStream]->codec;
        }
        
        return (videoStream);

}

cv::Mat  Player::convertFrameToMat()
{
    /*cv::Mat imagePtr;
    imagePtr = new cv::Mat(ptr->pCodecCtx->height, ptr->pCodecCtx->width, CV_8UC3, pFrame->data[0]);
    return (imagePtr);*/

    result_ptr = av_frame_alloc();

    int size_buf = avpicture_get_size(AV_PIX_FMT_BGR24,pCodecCtx->width,pCodecCtx->height);
    uint8_t * output =  (uint8_t * ) malloc(size_buf);
    //    avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), dst_pix_fmt, dst_pCodecCtx->width, dst_pCodecCtx->height);
    avpicture_fill((AVPicture*)result_ptr,output,AV_PIX_FMT_BGR24,pCodecCtx->width,pCodecCtx->height);
    struct SwsContext * swsContext;
    swsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height,
            pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
            AV_PIX_FMT_BGR24,
            SWS_BICUBIC,
            NULL,
            NULL,
            NULL);

    sws_scale(swsContext, pFrame->data, pFrame->linesize, 0, pCodecCtx->height,result_ptr->data, result_ptr->linesize);
    cv::Mat image(pCodecCtx->height, pCodecCtx->width, CV_8UC3, output, result_ptr->linesize[0]);
    return (image);
}

AVFrame* Player::convertMatToFrame(cv::Mat *  ptr)
{


          int numBytes = avpicture_get_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
          uint8_t * frame2_buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
          memcpy(frame2_buffer,(uint8_t*)ptr->data,numBytes);
          cv::Size frameSize = ptr->size();
                   dst->width =pCodecCtx->width;
                   dst->height  = pCodecCtx->height;



          avpicture_fill((AVPicture*)dst, frame2_buffer, AV_PIX_FMT_BGR24, pCodecCtx->width,pCodecCtx->height);

         avpicture_fill((AVPicture*)ptrYuv,(uint8_t*)dst->data[0],AV_PIX_FMT_YUV420P,pCodecCtx->width,pCodecCtx->height);
           struct SwsContext * swsContext;
             swsContext = sws_getContext(pCodecCtx->width,pCodecCtx->height,
                     AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height,
                     AV_PIX_FMT_YUV420P,
                     SWS_BICUBIC,
                     NULL,
                     NULL,
                     NULL);

   sws_scale(swsContext, dst->data, dst->linesize, 0, pCodecCtx->height,ptrYuv->data, ptrYuv->linesize);
   return (ptrYuv);
}

int Player::leerCodecVideo(void) {
  pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
  int res = 0;
        if (pCodecAudioCtx != NULL)
        {
            	pAudioCodec = avcodec_find_decoder(pCodecAudioCtx->codec_id);
       res = avcodec_open2(pCodecAudioCtx, pAudioCodec, NULL);
       if ( res <0  )
       {
           LOGI("Error opening audio codec..");
       }
	if (res < 0) {
		LOGI("Failed to open audio codec");
                return ( 1 );
	}
        }else
        {
            LOGI("Error abriendo el codec de audio, o no hay audio en el flujo");
        }

	if (pCodec == NULL) {
		LOGI("BAD VIDEO CODEC!!");
		return -1; // Codec not found
	}


	res = avcodec_open2(pCodecCtx, pCodec, NULL);
	if(res < 0){
		LOGI("Failed to open video CODEC!!");
	}
        return 1;
}



int Player::allocateMemory(void) {


   
	swrCtx = swr_alloc();
	if(swrCtx == NULL){
		LOGI("Failed to load audio");
		exit(-1);
	}
        if (pCodecAudioCtx != NULL)
        {
            //audio context
	av_opt_set_channel_layout(swrCtx, "in_channel_layout", pCodecAudioCtx->channel_layout, 0);
	av_opt_set_channel_layout(swrCtx, "out_channel_layout", pCodecAudioCtx->channel_layout, 0);
	av_opt_set_int(swrCtx, "in_sample_rate", pCodecAudioCtx->sample_rate, 0);
	av_opt_set_int(swrCtx, "out_sample_rate", pCodecAudioCtx->sample_rate, 0);
	av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", pCodecAudioCtx->sample_fmt, 0);
	av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
        
        
	
        int res = swr_init(swrCtx);
        if ( res != 0 )
        {
            errorThrow(res);
        }
        
                        	memset(&wantedSpec, 0, sizeof(wantedSpec));
	wantedSpec.channels = pCodecAudioCtx->channels;
	wantedSpec.freq = pCodecAudioCtx->sample_rate;
	wantedSpec.format = AUDIO_S16SYS;
	wantedSpec.silence = 0;
	wantedSpec.samples = SDL_AUDIO_BUFFER_SIZE;
	wantedSpec.userdata = pCodecAudioCtx;
	wantedSpec.callback = audio_callback;
	
	if (SDL_OpenAudio(&wantedSpec, &audioSpec) < 0) {
		cout<<"Error opening audio"<<endl;
		exit(-1);
	}
	wanted_frame.format = AV_SAMPLE_FMT_S16;	
	wanted_frame.sample_rate = audioSpec.freq;
	wanted_frame.channel_layout = av_get_default_channel_layout(audioSpec.channels);
	wanted_frame.channels = audioSpec.channels;
	
	initAudioPacket(&audioq);
	SDL_PauseAudio(0);
        
        }
        

        

	



	pFrame = av_frame_alloc();
	if (pFrame == NULL) {
		return -1;
	}

	pFrameRGB = av_frame_alloc();
	if (pFrameRGB == NULL) {
		return -1;
	}


	int numBytes = av_image_get_buffer_size(FORMATO, pCodecCtx->width, pCodecCtx->height,1);

	buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	//associa o buffer ao Frame
	int res = av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, FORMATO, pCodecCtx->width, pCodecCtx->height, 1);
	if (res < 0) {
		errorThrow(res);
		return res;
	}
	return 1;
}

void Player::initAudioPacket(AudioPacket *q)
{
    q->last = NULL;
    q->first = NULL;
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
}

int Player::putAudioPacket(AudioPacket *q, AVPacket *pkt)
{
    AVPacketList *pktl;
    AVPacket *newPkt  = (AVPacket *  )av_mallocz(sizeof(AVPacket));
    newPkt = (AVPacket*)av_mallocz_array(1, sizeof(AVPacket));
    if (av_packet_ref(newPkt, pkt) < 0)
        return -1;

    pktl = (AVPacketList*)av_malloc(sizeof(AVPacketList));
    if (!pktl)
        return -1;

    pktl->pkt = *newPkt;
    pktl->next = NULL;

    SDL_LockMutex(q->mutex);

    if (!q->last)
        q->first = pktl;
    else
        q->last->next = pktl;

    q->last = pktl;

    q->nb_packets++;
    q->size += newPkt->size;

    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);

    return 0;
}

int Player::getAudioPacket(AudioPacket* q, AVPacket* pkt, int block){

	AVPacketList* pktl;
    int ret;

    SDL_LockMutex(q->mutex);

    while (1)
    {
        pktl = q->first;
        if (pktl)
        {
            q->first = pktl->next;
            if (!q->first)
                q->last = NULL;

            q->nb_packets--;
            q->size -= pktl->pkt.size;

            *pkt = pktl->pkt;
            av_free(pktl);
            ret = 1;
            break;
        }
        else if (!block)
        {
            ret = 0;
            break;
        }
        else
        {
            SDL_CondWait(q->cond, q->mutex);
        }
    }

    SDL_UnlockMutex(q->mutex);

    return ret;

}

int audio_decode_frame(AVCodecContext* aCodecCtx, uint8_t* audio_buf, int buf_size){

		static AVPacket pkt;
                av_init_packet(&pkt);
    static uint8_t* audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    static AVFrame frame;
    static SwrContext * swr_ctx = swr_alloc();

    int len1;
    int data_size = 0;

    
    int copyBytes;
    

    while (1)
    {
        while (audio_pkt_size > 0)
        {
            int got_frame = 0;

            int res = avcodec_decode_audio4(aCodecCtx,&frame,&got_frame,&pkt);
            if ( res < 0 )
            {
                LOGI("Error decoding audio data...");
            }
            
			len1 = frame.pkt_size;
            if (len1 < 0)
            {
                audio_pkt_size = 0;
                break;
            }

            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            data_size = 0;
            if (got_frame)
            {
                int linesize = 1;
                data_size = av_samples_get_buffer_size(&linesize, aCodecCtx->channels, frame.nb_samples, aCodecCtx->sample_fmt, 1);
                memcpy(audio_buf, frame.data[0], data_size);
            }
            if (frame.channels > 0 && frame.channel_layout == 0)
                frame.channel_layout = av_get_default_channel_layout(frame.channels);
            else if (frame.channels == 0 && frame.channel_layout > 0)
                frame.channels = av_get_channel_layout_nb_channels(frame.channel_layout);

            if (swr_ctx)
            {
                swr_free(&swr_ctx);
                swr_ctx = NULL;
            }

            swr_ctx = swr_alloc_set_opts(NULL, wanted_frame.channel_layout, (AVSampleFormat) wanted_frame.format, wanted_frame.sample_rate,
                frame.channel_layout, (AVSampleFormat) frame.format, frame.sample_rate, 0, NULL);

            if (!swr_ctx || swr_init(swr_ctx) < 0)
            {
               LOGI("swr_init failed");
                break;
            }

            int dst_nb_samples = (int)av_rescale_rnd(swr_get_delay(swr_ctx, frame.sample_rate) + frame.nb_samples,
                wanted_frame.sample_rate, wanted_frame.format, AV_ROUND_INF);
            int len2 = swr_convert(swr_ctx, &audio_buf, dst_nb_samples,
                (const uint8_t**)frame.data, frame.nb_samples);
            if (len2 < 0)
            {
               LOGE("swr_convert failed");
                break;
            }

            return wanted_frame.channels * len2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            if (data_size <= 0)
                continue;

            return data_size;
        }

        if (pkt.data)
            av_packet_unref(&pkt);

        if (Player::getAudioPacket(&audioq, &pkt, 1) < 0)
            return -1;

        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
    }

}

void audio_callback(void* userdata, Uint8* stream, int len){

	AVCodecContext* aCodecCtx = (AVCodecContext*)userdata;
    int len1, audio_size;

    static uint8_t audio_buff[192000 * 3 / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;

    SDL_memset(stream, 0, len);

    while (len > 0)
    {
        if (audio_buf_index >= audio_buf_size)
        {
            audio_size = audio_decode_frame(aCodecCtx, audio_buff, sizeof(audio_buff));
            if (audio_size < 0)
            {
                audio_buf_size = 1024;
                memset(audio_buff, 0, audio_buf_size);
            }
            else
                audio_buf_size = audio_size;

            audio_buf_index = 0;
        }
        len1 = audio_buf_size - audio_buf_index;
        if (len1 > len)
            len1 = len;

        SDL_MixAudio(stream, audio_buff + audio_buf_index, len, SDL_MIX_MAXVOLUME);


        //memcpy(stream, (uint8_t*)(audio_buff + audio_buf_index), audio_buf_size);
        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
}


void Player::applyDetection(void)
{

    auto proposals = ss::selectiveSearch( ptr, 500, 0.8, 50, 20000, 100000, 2.5 );

	for ( auto &&rect : proposals )
	{
		cv::rectangle( ptr, rect, cv::Scalar( 0, 255, 0 ), 3, 8 );
	}


	cv::imshow( "Vidium -Online CCTV Services", ptr );
}





//L� a "data" que vem do stream de v�deo

void Player::initParameters()
{
      	//video context
      	sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                  pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                  AV_PIX_FMT_YUV420P,
                  SWS_BILINEAR,
                  NULL,
                  NULL,
                  NULL);
              int copyBytes;
               int yPlaneSz = pCodecCtx->width * pCodecCtx->height;
          int uvPlaneSz = pCodecCtx->width * pCodecCtx->height / 4;
          uint8_t *yPlane =  (uint8_t*)malloc(uvPlaneSz);
          uint8_t * uPlane = (uint8_t*)malloc(uvPlaneSz);
          uint8_t *  vPlane = (uint8_t*)malloc(uvPlaneSz);
          int uvPitch;
          uvPitch =  pCodecCtx->width  / 2;


      	//lendo e colocando no packet
}
void Player::leerFramesDeVideo(void) {
    int copyBytes;
    AVPacket packet;
    av_init_packet(&packet);
	while (av_read_frame(pFormatCtx, &packet) >= 0) {


		if (packet.stream_index == audioStream) {
			putAudioPacket(&audioq, &packet);
                        LOGI("Audio!");
		}
		//verifica��o se foi transmitido pelo Stream do Context
		if (packet.stream_index == videoStream) {
                    LOGI("Correcto!");




			//processo de decodifica��o
			/*int res = avcodec_send_packet(pCodecCtx, &packet);
			if (res < 0) {
				errorThrow(res);
				continue;
			}*/

                                /*int res = avcodec_send_packet(pCodecCtx, &packet);
                    			if (res < 0) {
                    				errorThrow(res);
                    				continue;
                    			}

                                res = avcodec_receive_frame(pCodecCtx, pFrame);
                                if ( res <  0 )
                                {
                                    errorThrow(res);
                                    continue;
                                }*/
                    
                    int res = avcodec_decode_video2(pCodecCtx,pFrame,&copyBytes,&packet);
                    if ( res < 0 )
                    {
                        LOGI("Error decoding video data..");
                    }
                    if ( copyBytes )
                    {
                        
                         cv::Mat  ptrFrame  = convertFrameToMat();
                         ptr = ptrFrame;
                         if ( objectFilter == true )
                         {
                             applyDetection();
                         }else
                         {
                             objectFilter = false;
                         }
                         LOGI("Decoding data successfully......");


                        			SDL_UpdateYUVTexture(bmp, NULL, pFrame->data[0], pFrame->linesize[0],
				pFrame->data[1],pFrame->linesize[1],
				pFrame->data[2], pFrame->linesize[2]);

				//SDL_UpdateTexture(bmp,NULL,frame->data[0],frame->linesize[0]);
			SDL_RenderCopy(renderer, bmp, NULL, NULL);
			SDL_RenderPresent(renderer);
			SDL_UpdateWindowSurface(screen);
			SDL_Delay(1000/30);
                    }





		}
		SDL_PollEvent(&evt);


	}



}

void Player::openStream(std::string str)
{
         audioStream = -1;

		//init ffmpeg
		av_register_all();
        avcodec_register_all();

        avformat_network_init();
        
        pFormatCtx = avformat_alloc_context();



		//open video
		int res = avformat_open_input(&pFormatCtx, str.c_str(), NULL, NULL);



		if (res!=0){
			errorThrow(res);
                        ptr_op = false;
		}

		else
                {
                    //get video info
		res = avformat_find_stream_info(pFormatCtx, NULL);
		if (res < 0) {
			errorThrow(res);
                        ptr_op = false;
		}

                //obtenerInfoDeStream();

		//get video stream
		videoStream = obtenerCodecParametros();
		if (videoStream == -1) {
			LOGI("Error opening your video using AVCodecParameters, does not have codecpar_type type AVMEDIA_TYPE_VIDEO");
			ptr_op = false;
		}


		if (leerCodecVideo() < 0)
                {
                    LOGI("Error receiving video data..\n");
                    ptr_op = false;
                }
                ptr_op = true; //Opened stream successfully.
                }
                
}






 void Player::setDetectionVisible()
 {
     this->isDetectionVisible =  true;
 }

int Player::crearVideo(void) {
 int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

	screen = SDL_CreateWindow("En directo",
    		SDL_WINDOWPOS_UNDEFINED,
    	    SDL_WINDOWPOS_UNDEFINED,
    		pCodecCtx->width,pCodecCtx->height,
    		flags);

	if (!screen) {
		 LOGE("No ha sido posible crear la ventana de ese tamaño.");
		return -1;
	}

	renderer = SDL_CreateRenderer(screen, -1, 0);

	   bmp = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_YV12,
            SDL_TEXTUREACCESS_STREAMING,
            pCodecCtx->width,
            pCodecCtx->height      );

	return 1;
}
