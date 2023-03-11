#program once

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <chrono>
#include <stack>
#include <opencv2/opencv.hpp>

#include "MvCameraControl.h"

class hkvs_cam
{
    private:
        int nRet = MV_OK;
        void* handle = NULL;
        MVCC_INTVALUE stParam;
        unsigned int nIndex = 0;
        unsigned int nDataSize = 0;
        unsigned char *pData = NULL;
        unsigned int nDataSize = 0; 
        unsigned int g_nPayloadSize = 0;
        std::chrono::system_clock::time_point start;
        std::time_t start_t;
        std::chrono::system_clock::time_point now;
        std::time_t now_t;
        pthread_t nThreadID;
        cv::Mat frame;
        stack<cv::Mat> stack_img;
        pthread_spinlock_t mutex;
        long timestamp;
    public:
        hkvs_cam();
        ~hkvs_cam();
        void InitCam();//相机初始化函数
        void PrintDeviceInfo();//打印相机参数
        void StartGrab();//开始取图
        void* WorkThread();//工作线程
        void ReadImg(cv::Mat &src,long &Timestamp);//读图函数

};
hkvs_cam::hkvs_cam(/* args */)
{
    NULL;
}
hkvs_cam::~hkvs_cam()
{
    pthread_join(this->nThreadID, NULL);
    // 停止取流
    this->nRet = MV_CC_StopGrabbing(this->handle);
    if (MV_OK != nRet) {
        printf("MV_CC_StopGrabbing fail! nRet [%x]\n", nRet);
        exit(-1);
    }
    printf("MV_CC_StopGrabbing succeed.\n");
    // 关闭设备
    this->nRet = MV_CC_CloseDevice(this->handle);
    if (MV_OK != nRet) {
        printf("MV_CC_CloseDevice fail! nRet [%x]\n", nRet);
        exit(-1);
    }
    printf("MV_CC_CloseDevice succeed.\n");
    // 销毁句柄
    this->nRet = MV_CC_DestroyHandle(this->handle);
    if (MV_OK != nRet) 
    {
        printf("MV_CC_DestroyHandle fail! nRet [%x]\n", nRet);
        exit(-1);
    }
    printf("MV_CC_DestroyHandle succeed.\n");

}

inline void hkvs_cam::InitCam()
{
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    /*枚举设备*/
    this->nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if (MV_OK != this->nRet)
    {
        printf("MV_CC_EnumDevices fail! nRet [%x]\n", this->nRet);
        break;
    }
    if (stDeviceList.nDeviceNum > 0)
    {
        for (int i = 0; i < stDeviceList.nDeviceNum; i++)
        {
            printf("[device %d]:\n", i);
            MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
            if (NULL == pDeviceInfo)
            {
               break;
           } 
           PrintDeviceInfo(pDeviceInfo);            
        }  
    } 
    else
    {
        printf("Find No Devices!\n");
        break;
    }
    /*选择设备创建句柄*/
    this->nRet = MV_CC_CreateHandle(&this->handle, stDeviceList.pDeviceInfo[this->nIndex]);
    if (MV_OK != this->nRet)
    {
        printf("MV_CC_CreateHandle fail! nRet [%x]\n", this->nRet);
        break;
    }
    /*打开相机*/
    this->nRet = MV_CC_OpenDevice(this->handle);
    if (MV_OK != this->nRet)
    {
        printf("MV_CC_OpenDevice fail! nRet [%x]\n", this->nRet);
        break;
    }
    /*设置触发模式*/
    this->nRet = MV_CC_SetEnumValue(this->handle, "TriggerMode", 0);
    if (MV_OK != this->nRet)
    {
        printf("MV_CC_SetTriggerMode fail! nRet [%x]\n", this->nRet);
        break;
    }
    /*获取数据包大小*/
    memset(&this->stParam, 0, sizeof(MVCC_INTVALUE));
    this->nRet = MV_CC_GetIntValue(this->handle, "PayloadSize", &this->stParam);
    if (MV_OK != this->nRet)
    {
        printf("Get PayloadSize fail! this->nRet [0x%x]\n", this->nRet);
        break;
    }
    this->g_nPayloadSize = stParam.nCurValue;
    /*设置取图格式*/
    this->nRet = MV_CC_SetEnumValue(this->handle, "PixelFormat", PixelType_Gvsp_BayerRG8); // 目前 RGB
    if (MV_OK !== this->nRet) 
    {
        printf("MV_CC_SetPixelFormat fail! this->nRet [%x]\n", this->nRet);
        break;
    }
    /*开始计时*/
    this->start = std::chrono::system_clock::now();
    this->start_t = std::chrono::system_clock::to_time_t(this->start);
}

inline void hkvs_cam::PrintDeviceInfo()
{
    return 0;
}

inline void hkvs_cam::StartGrab()
{
    /*开始取流*/
    this->nRet = MV_CC_StartGrabbing(this->handle);
    if (MV_OK != this->nRet)
    {
        printf("MV_CC_StartGrabbing fail! this->nRet [%x]\n", this->nRet);
        exit(-1);
    }
    nRet = pthread_create(&this->nThreadID, NULL ,this->WorkThread);
    if (nRet != 0)
    {
        printf("thread create failed.ret = %d\n",nRet);
        break;
    }

}
inline void *hkvs_cam::WorkThread()
{
    unsigned char *m_pBufForDriver = (unsigned char *) malloc(sizeof(unsigned char) * this->g_nPayloadSize);
    unsigned char *m_pBufForSaveImage = (unsigned char *) malloc(this->g_nPayloadSize * 3);
    MV_FRAME_OUT_INFO_EX stImageInfo = {0};
    MV_CC_PIXEL_CONVERT_PARAM stConvertParam = {0};
    MV_FRAME_OUT stOutFrame = {0};
    memset(&stOutFrame, 0, sizeof(MV_FRAME_OUT));
    while(1)
    {
        nRet = MV_CC_GetImageBuffer(this->handle,&stOutFrame,10);
        stConvertParam.nWidth = stOutFrame.stFrameInfo.nWidth; 
        stConvertParam.nHeight = stOutFrame.stFrameInfo.nHeight;               //ch:图像高 | en:image height
        stConvertParam.pSrcData = stOutFrame.pBufAddr;                  //ch:输入数据缓存 | en:input data buffer
        stConvertParam.nSrcDataLen = this->g_nPayloadSize;        //ch:输入数据大小 | en:input data size
        stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed; //ch:输出像素格式 | en:output pixel format     //! 输出格式 RGB
        stConvertParam.pDstBuffer = m_pBufForSaveImage;             //ch:输出数据缓存 | en:output data buffer
        stConvertParam.nDstBufferSize = this->g_nPayloadSize * 3;     //ch:输出缓存大小 | en:output buffer size
        stConvertParam.enSrcPixelType = stOutFrame.stFrameInfo.enPixelType;    //ch:输入像素格式 | en:input pixel format      //! 输入格式 RGB
        this->nRet = MV_CC_ConvertPixelType(this->handle, &stConvertParam);
        if (MV_OK != this->nRet)
        {
            std::cout<<"convert fail!"<<this->nRet<<std::endl;
        }
        this->frame = cv::Mat(stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nWidth, CV_8UC3,m_pBufForSaveImage).clone(); //tmp.clone();
        this->now = std::chrono::system_clock::now();
        this->now_t = std::chrono::system_clock::to_time_t(this->now); 
        this->nRet = MV_CC_FreeImageBuffer(this->handle, &stOutFrame); 
        pthread_spin_lock(&this->mutex);
        this->timestamp = static_cast<long>(this->now_t-this->start_t);
        this->
        pthread_spin_unlock(&this->mutex); 
    }
    free(m_pBufForDriver);
    free(m_pBufForSaveImage);
    return 0;
}

inline void hkvs_cam::ReadImg(cv::Mat &src,long &Timestamp)
{

    if(this->stack_img().size > 0)
    {
        src = this->stack_img.back().clone();
    }
    Timestamp = this->timestamp;
}
