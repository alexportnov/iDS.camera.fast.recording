#include <stdio.h>
#include <stddef.h>
#include <sys/time.h>
#include <cstdio>
#include <ueye.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

// config set
#define CAPTURE_WIDTH  (1280/2)
#define CAPTURE_HEIGHT (1024/2)
#define FPS (400.00)
#define SET_EXPOSURE_VAL  3.5
#define SET_ENABLE_AUTO_WHITEBALANCE 0
#define SET_ENABLE_AUTO_GAIN 0
#define SET_MASTER_GAIN 100
#define SET_GAMMA 150

#define NUM_IMG_BUFFERS 3
//#define DEBUG_TIMING

using namespace cv;
using namespace std;

// https://en.ids-imaging.com/manuals/uEye_SDK/EN/uEye_Manual/index.html?sdk_allgemeines_programmierung.html

int main(int argc, char** argv)
{
    HIDS hCam = 1;
    SENSORINFO sensor_info;
    CAMINFO camera_info;
    char* m_pcImageMemory[NUM_IMG_BUFFERS] = {0};
    int m_lMemoryId[NUM_IMG_BUFFERS] = {0};
    int nRet = 0;
    int pnNumCams = 0;
    int duration_sec = 3;

    if(argc < 2)
    {
        printf("Error: missing argument\n");
        printf("EXE ./OUT_PATH.AVI DURATION_SEC\n");
        return -1;
    }

    if(argc >= 2)
    {
        duration_sec = atoi(argv[2]);
    }

    printf("Saving to %s duration=%d\n", argv[1], duration_sec);
    
    if(IS_SUCCESS != is_GetNumberOfCameras(&pnNumCams))
    {
        printf("Error: is_GetNumberOfCameras\n");
        return -1;
    }

    if(pnNumCams > 0)
    {
        hCam = pnNumCams;
    }
    else
    {
        printf("Error: No camera detected\n");
        return -1;
    }

    if(IS_SUCCESS != is_InitCamera(&hCam, NULL))
    {
        printf("Error: is_InitCamera\n");
        return -1;
    }

    if(IS_SUCCESS != is_GetCameraInfo(hCam, &camera_info))
    {
        printf("Error: is_GetCameraInf\n");
        return -1;
    } 

    if(IS_SUCCESS != is_GetSensorInfo(hCam, &sensor_info))
    {
        printf("Error: is_GetSensorInfo\n");
        return -1;
    }

    printf("s/N: %s\n", camera_info.SerNo);
    printf("ID: %s\n", camera_info.ID);
    printf("Model: %s\n", sensor_info.strSensorName);
    printf("Size: %d:%d \n", sensor_info.nMaxWidth, sensor_info.nMaxHeight);

    UINT nNumberOfSupportedPixelClocks = 0;
    UINT nPixelClockList[128] = {0};
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET_NUMBER, (void*)&nNumberOfSupportedPixelClocks,sizeof(nNumberOfSupportedPixelClocks));
    if ((nRet == IS_SUCCESS) && (nNumberOfSupportedPixelClocks > 0))
    {
        
        ZeroMemory(&nPixelClockList, sizeof(nPixelClockList));

        nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET_LIST,
                        (void*)nPixelClockList,
                        nNumberOfSupportedPixelClocks * sizeof(UINT));

        printf("Number of clocks = %d\n",nNumberOfSupportedPixelClocks);
    }

    int nPixelClockDefault = nPixelClockList[nNumberOfSupportedPixelClocks - 1];
    if(IS_SUCCESS != is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET,
                        (void*)&nPixelClockDefault,
                        sizeof(nPixelClockDefault)))
    {
        printf("Error: is_PixelClock\n");
    }

    if(IS_SUCCESS != is_SetDisplayMode(hCam, IS_SET_DM_DIB))
    {
        printf("Error: is_SetDisplayMode\n");
    }
        
    if(IS_SUCCESS != is_SetSubSampling(hCam, IS_SUBSAMPLING_2X_VERTICAL | IS_SUBSAMPLING_2X_HORIZONTAL))
    {
        printf("Error: is_SetSubSampling\n");
    }
    
    IS_RECT rectAOI;
    rectAOI.s32X = 0;
    rectAOI.s32Y = 0;
    rectAOI.s32Width = CAPTURE_WIDTH;
    rectAOI.s32Height = CAPTURE_HEIGHT;
    if(IS_SUCCESS != is_AOI(hCam, IS_AOI_IMAGE_SET_AOI, (void*)&rectAOI, sizeof(rectAOI)))
    {
        printf("Error: is_AOI\n");
    }
    
    double newFPS = 0;
    if(IS_SUCCESS != is_SetFrameRate(hCam, FPS, &newFPS))
    {
        printf("Error: is_SetFrameRate\n");
    }
    else
    {
        printf("FPS set to %lf\n", newFPS);
    }

    if(IS_SUCCESS != is_SetColorMode(hCam, IS_CM_MONO8))
    //if(IS_SUCCESS != is_SetColorMode(hCam, IS_CM_BGR8_PACKED))
    {
        printf("Error: is_SetColorMode\n");
        return -1;
    }

    double strenght_factor = 1.0;
    if(IS_SUCCESS != is_SetColorCorrection(hCam, IS_CCOR_DISABLE, &strenght_factor))
    {
        printf("Error: is_SetColorCorrection\n");
        return -1;
    }

    // Set exposure manually
    double newExposure = SET_EXPOSURE_VAL;
    if(IS_SUCCESS != is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, &newExposure, sizeof(newExposure)))
    {
        printf("Error: is_Exposure\n");
        return -1;
    }

    double pval = SET_ENABLE_AUTO_WHITEBALANCE;
    if(IS_SUCCESS != is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &pval, 0))
    {
        printf("Error: is_SetAutoParameter\n");
        return -1;
    }

    double gval = SET_ENABLE_AUTO_GAIN;
    if(IS_SUCCESS != is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_GAIN, &gval, 0))
    {
        printf("Error: is_SetAutoParameter\n");
        return -1;
    }
    
    if(IS_SUCCESS != is_SetHardwareGain(hCam, SET_MASTER_GAIN, 60, 60, 60))
    {
        printf("Error: is_SetHardwareGain\n");
        return -1;
    }

    int gamma = SET_GAMMA;
    if(IS_SUCCESS != is_Gamma(hCam, IS_GAMMA_CMD_SET, &gamma, sizeof(gamma)))
    {
        printf("Error: is_Gamma\n");
        return -1;
    }
/*
    int blMode = IS_AUTO_BLACKLEVEL_ON;
    if(IS_SUCCESS != is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_SET_MODE, (void*)&blMode , sizeof(blMode)))
    {
        printf("Error: is_Blacklevel\n");
        return -1;
    }
*/
    printf("Ready ... \n");

    int imgCounter = 0;
    VideoWriter outputVideo;
    outputVideo.open(argv[1], VideoWriter::fourcc('F','M','P','4'), (int)FPS, Size(CAPTURE_WIDTH, CAPTURE_HEIGHT), false);
    if (!outputVideo.isOpened())
    {
        printf("Error: VideoWriter\n");
    }

    for(int i = 0; i < NUM_IMG_BUFFERS; ++i)
    {
        if(IS_SUCCESS != is_AllocImageMem(hCam, CAPTURE_WIDTH, CAPTURE_HEIGHT, 8/*24*/, &m_pcImageMemory[i], &m_lMemoryId[i]))
        {
            printf("Error: is_AllocImageMem\n");
            return -1;
        }

        if(IS_SUCCESS != is_AddToSequence(hCam, m_pcImageMemory[i], m_lMemoryId[i]))
        {
            printf("Error: is_AddToSequence\n");
            return -1;
        }
    }

    if(IS_SUCCESS != is_CaptureVideo(hCam, IS_DONT_WAIT))
    {
        printf("Error: is_CaptureVideo\n");
    }
    
    if(IS_SUCCESS != is_EnableEvent(hCam, IS_SET_EVENT_FRAME))
    {
        printf("Error: is_EnableEvent\n");
    }

    char *pMemVoid;
    struct timeval t1, t0;

#ifdef DEBUG_TIMING
    struct timeval tA;
    double prevTime = 0;
    long int prevTimeUs = 0;
    #define HISTO_SIZE 999
    int histo[HISTO_SIZE] = {0};
    int histo2[10] = {0};
#endif

    gettimeofday(&t0, 0);

    while(IS_SUCCESS == is_WaitEvent(hCam, IS_SET_EVENT_FRAME, 1000))
    {
 #ifdef DEBUG_TIMING
        gettimeofday(&tA, 0);
        double t = double(tA.tv_sec * 1e6 + tA.tv_usec)/1e6;
        //printf ("dt=%.8lf ms\n", (t - prevTime)*1000);
        int idx = (int)((t - prevTime)*1000 * 100 + 0.5);
        if(imgCounter > FPS && idx >=0 && idx < HISTO_SIZE)
        {
            histo[idx]++;
        }
        prevTime = t;
#endif
        
        if(IS_SUCCESS != is_GetImageMem(hCam, (void**)&pMemVoid))
        {
            printf("Error: is_GetImageMem\n");
        }
  
        /*
        is_GetActSeqBuf(hCam, NULL, NULL, &pMemVoid);
        is_LockSeqBuf(hCam, IS_IGNORE_PARAMETER, pMemVoid);
        //is_CopyImageMem(hCam, pMemVoid, buffers[ptr], (char*)image.data());
        */

        Mat frame(CAPTURE_HEIGHT, CAPTURE_WIDTH, CV_8UC1, pMemVoid);
       /*
        int iid = 0;
        for(int i = 0; i < NUM_IMG_BUFFERS; ++i)
        {
            if(m_pcImageMemory[i] == pMemVoid)
            {
                iid = m_lMemoryId[i];
                break;
            }
        }
        
        UEYEIMAGEINFO imageInfo;
        is_GetImageInfo(hCam, iid, &imageInfo, sizeof(imageInfo));
        int dt = (int)(imageInfo.u64TimestampDevice - prevTimeUs);
        int iidx =  (int)(dt/10000);
        if(iidx < 10 && iidx >=0)
            histo2[iidx]++;
        //printf("dt=%u i=%d\n", (int)(dt/10000), iid);
        prevTimeUs = imageInfo.u64TimestampDevice;
        */
        
        outputVideo << frame;
        //is_UnlockSeqBuf(hCam, IS_IGNORE_PARAMETER, pMemVoid);

        /*
        imshow("Image",frame);
        waitKey(0);
        */

        imgCounter++;
        if(imgCounter > duration_sec*FPS)
        {
            break;
        }
    }
    
    
    gettimeofday(&t1, 0);
    double dif = double((t1.tv_sec * 1e6 + t1.tv_usec - (t0.tv_sec * 1e6 + t0.tv_usec)))/1e6;
    printf ("dt=%.8lf fps=%.2lf \n", dif, imgCounter/dif);
    
#ifdef DEBUG_TIMING
    for(int i = 0; i < 10; ++i)
    {
        printf("bin=%d count=%d\n", i, histo2[i]);
    }
   
    printf("-------------------------------------------------------------\n");
    for(int i = 100; i < HISTO_SIZE - 600; ++i)
    {
        printf("bin=%.2fms count=%d\n", i/100.f, histo[i]);
    }
#endif
    
    is_StopLiveVideo(hCam, IS_WAIT);
    
    for(int i = 0; i < NUM_IMG_BUFFERS; ++i)
    {
        if(IS_SUCCESS != is_FreeImageMem(hCam, m_pcImageMemory[i], m_lMemoryId[i]))
        {
            printf("is_FreeImageMem failed\n");
        }
    }
    
    if(IS_SUCCESS != is_ExitCamera(hCam))
    {
        printf("is_ExitCamera failed\n");
    } 

    printf("Done ... \n");
    return 0;
}
