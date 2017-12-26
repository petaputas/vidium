/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CameraIP.cpp
 * Author: moi
 * 
 * Created on 17 de diciembre de 2017, 19:01
 */

#include "CameraIP.h"


void CameraIP::openInputStream(char* str)
{
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
                LOGI("Error al crear el video SDL");
				exit(-1);
	}

	Player::openStream(str);
        if ( Player::isOpenedStream() == true )
        {
            std::cout << "Opened stream successfully..decoding video data..\n" << std::endl;
            	Player::allocateMemory();
            //Creamos la ventana...
            Player::crearVideo();
            Player::initParameters();
            Player::leerFramesDeVideo();
        }else
        {
            std::cout << "Error opening video stream.." << std::endl;
        }
}


   
JNIEXPORT void JNICALL Java_videoplayer_VideoStreamPlayer_openStream
  (JNIEnv * env, jclass, jstring  url)
{
    char      *urlPtr =   (char    *  )calloc(1,sizeof(char));
    jboolean copy;
    CameraIP * cam  = new CameraIP();
    urlPtr  =   (char     *  ) env->GetStringUTFChars(url,&copy);
    cam->openInputStream(urlPtr);
    return;
}

JNIEXPORT void JNICALL Java_videoplayer_VideoStreamPlayer_objectDetectionEnabled
  (JNIEnv *env, jclass s,bool ptr)
{
    Player::applyDetectionFilter(ptr);
}