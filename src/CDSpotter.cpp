#include <stdio.h>
#include <string>

#include "CDSpotter.h"
#include "include/base_types.h"
#include "include/DSpotterSDKApi_Const.h"
#include "include/DSpotterSDKApi.h"
#include "include/RingBuffer.h"
#include "include/CybModelInfor.h"

#define MAX_COMMAND_TIME  500

CDSpotter::CDSpotter()
{
	m_hCybModel = NULL;
	m_hRingBuffer = NULL;
	m_hDSpotter = NULL;
	m_bEnableAGC = false;
	m_nScalePercentage = 100;
	m_nActiveGroupIndex = -1;
	m_nRecordLostCount = 0;
	m_nActiveCommandsGroup = 1;
}

CDSpotter::~CDSpotter()
{
	Release();
}

int CDSpotter::GetMemoryUsage(const uint32_t *lpdwModel, int nRecordCacheTimeMS)
{
	BYTE byaCybModelMem[CYBMODEL_GET_MEM_USAGE()];
	HANDLE hCybModel;
	int nGroupCount;
	BYTE *lppbyModel[1];
	int nMemSize, nMemUsage;

	if (lpdwModel == NULL)
		return IllegalParam;

	if (nRecordCacheTimeMS < 30)
		return IllegalParam;

	hCybModel  = CybModelInit((const BYTE*)lpdwModel, byaCybModelMem, sizeof(byaCybModelMem), NULL);
	nGroupCount = CybModelGetGroupCount(hCybModel);
	nMemUsage = 0;
	for (int nGroup = 0; nGroup < nGroupCount; nGroup++)
	{
		lppbyModel[0] = (BYTE*)CybModelGetGroup(hCybModel, nGroup);
		nMemSize = DSpotter_GetMemoryUsage_Multi((BYTE *)CybModelGetBase(hCybModel), lppbyModel, 1, MAX_COMMAND_TIME);
		if (nMemUsage < nMemSize)
			nMemUsage = nMemSize;
	}
	CybModelRelease(hCybModel);

	return CYBMODEL_GET_MEM_USAGE() + RingBufferGetMemUsage(nRecordCacheTimeMS * 32) + nMemUsage + sizeof(int);
}

const char* CDSpotter::GetVerInfor()
{
	return DSpotter_VerInfo();
}


const char* CDSpotter::GetSerialNumber()
{
	return CDSpotter_GetSerialNumber();
}

int CDSpotter::Init(const uint32_t *lpdwLicense, int nLicenseSize, const uint32_t *lpdwModel, int nRecordCacheTimeMS, unsigned char *lpbyMemPool, int nMemSize)
{
	int nRet;
	
	if (lpdwLicense == NULL || lpdwModel == NULL || lpbyMemPool == NULL)
		return IllegalParam;
	
	if (nRecordCacheTimeMS < 60)
		return IllegalParam;

	if (nMemSize < GetMemoryUsage(lpdwModel, nRecordCacheTimeMS))
		return LeaveNoMemory;

	if (nLicenseSize > 256)
		return LeaveNoMemory;

	memcpy(m_byaLicense, lpdwLicense, nLicenseSize);
	m_nLicenseSize = nLicenseSize;

	Release();

	// Align lpbyMemPool to 4 bytes.
	lpbyMemPool = (BYTE *) ((((ULONG)lpbyMemPool + 3) >> 2) << 2);
	m_hCybModel = CybModelInit((const BYTE*)lpdwModel, lpbyMemPool, CYBMODEL_GET_MEM_USAGE(), NULL);
	RingBufferInit(lpbyMemPool + CYBMODEL_GET_MEM_USAGE(), RingBufferGetMemUsage(nRecordCacheTimeMS * 32), nRecordCacheTimeMS * 32, &m_hRingBuffer);
	m_lpbyMemPool = lpbyMemPool + CYBMODEL_GET_MEM_USAGE() + RingBufferGetMemUsage(nRecordCacheTimeMS * 32);

	nRet = InitRecognition(0);
	if (nRet != Success)
		Release();

	return nRet;
}

int CDSpotter::Release()
{
	ReleaseRecognition();
	CybModelRelease(m_hCybModel);
	m_hCybModel = NULL;
	RingBufferRelease(m_hRingBuffer);
	m_hRingBuffer = NULL;
	m_bEnableAGC = false;
	m_nScalePercentage = 100;
	return Success;
}

int CDSpotter::Start()
{
	int nRet = Success;

	if (m_nActiveGroupIndex != 0)
		InitRecognition(0);

	RingBufferReset(m_hRingBuffer);
	m_bStart = true;

	return nRet;
}

int CDSpotter::Stop()
{
	m_bStart = false;
	RingBufferReset(m_hRingBuffer);

	return Success;
}

int CDSpotter::SetAGC(bool bEnableAGC, int nScalePercentage)
{
	if (m_hDSpotter == NULL)
		return NotInit;

	m_bEnableAGC = bEnableAGC;
	m_nScalePercentage = nScalePercentage;;
	
	if (bEnableAGC)
	{
		DSpotterAGC_Enable(m_hDSpotter);
		DSpotterAGC_SetMaxGain(m_hDSpotter, (float)(nScalePercentage / 100.0));
	}
	else
	{
		DSpotterAGC_Disable(m_hDSpotter);
	}

	return Success;
}

int CDSpotter::SetActiveCommandGroup(int nGroupActive)
{
	int n;
	
	if(m_hCybModel == NULL)
		return NotInit;
	
	//need check nGroupActive is legal or not
	n = CybModelGetGroupCount(m_hCybModel);
	if(nGroupActive == 0 || nGroupActive >= n)
		return IllegalParam;
	
	m_nActiveCommandsGroup = nGroupActive;
	
	return Success;
}

int CDSpotter::SetCommandStageProperty(int nTimeout, bool bCommandStageRepeatUntilTimeout)
{
	m_nCommandStageTimeout = nTimeout;
	if (m_nCommandStageTimeout < 1000)
		m_nCommandStageTimeout = 1000;
	else if (m_nCommandStageTimeout > 30000)
		m_nCommandStageTimeout = 30000;
	m_bCommandStageRepeatUntilTimeout = bCommandStageRepeatUntilTimeout;

	return Success;
}

int CDSpotter::PutRecordData(const short *lpsSample, int nNumSample)
{
	int nDataSize = nNumSample * sizeof(short);

	if (m_hRingBuffer == NULL)
		return NotInit;

	if (!m_bStart)
		return Stopped;

	// Check free size of ring-buffer
	if (RingBufferGetFreeSize(m_hRingBuffer) < nDataSize)
	{
		//printf("ERROR : No enough free ring buffer!\r\n");
		m_nRecordLostCount++;
		return LeaveNoMemory;
	}

	// Put Mic record data to ring buffer
	RingBufferPutData(m_hRingBuffer, lpsSample, nDataSize);

	return Success;
}

int CDSpotter::GetRecordLostCount()
{
	return m_nRecordLostCount;
}

int CDSpotter::DoRecognition(int *pnCurrentState)
{
	int nRet;
	const int nNumSample = 160;
	short *lpsData1 = NULL, *lpsData2 = NULL;
	int nData1Size = 0, nData2Size = 0;
	int nStage = 0;

	if(m_nActiveGroupIndex == 0)
		nStage = TriggerStage;
	else if(m_nActiveGroupIndex >= 1)
		nStage = CommandStage;
	else
		nStage = InitStage;
	
	if (pnCurrentState != NULL)
		*pnCurrentState = nStage;

	if (m_hRingBuffer == NULL || m_hDSpotter == NULL)
		return NotInit;

	if (!m_bStart)
		return Stopped;

	if (RingBufferGetDataSize(m_hRingBuffer) < nNumSample*sizeof(short))
		return NeedMoreSample;

	RingBufferGetDataBuffer(m_hRingBuffer, nNumSample * sizeof(short), (void **)&lpsData1, &nData1Size,
		(void **)&lpsData2, &nData2Size);

	nRet = DSpotter_AddSample(m_hDSpotter, lpsData1, nData1Size/sizeof(short));
	if (nData2Size > 0)
		nRet = DSpotter_AddSample(m_hDSpotter, lpsData2, nData2Size / sizeof(short));
	if (nData1Size + nData2Size != nNumSample * sizeof(short))
		printf("Ring buffer problem!\r\n");
	RingBufferDequeueData(m_hRingBuffer, nData1Size + nData2Size);
	if (nRet == DSPOTTER_ERR_NeedMoreSample)
	{
		//if (m_nActiveGroupIndex == 1)
		if (m_nActiveGroupIndex >= 1)
		{
			// Check timeout for command recognition stage
			m_nCommandRecordSample += nNumSample;
			if (m_nCommandRecordSample > 16000 / 1000 * m_nCommandStageLimit)
			{
				if (DSpotter_IsKeywordAlive(m_hDSpotter))
				{
					if (m_nCommandStageLimit < m_nCommandStageTimeout + 2000)
					{
						m_nCommandStageLimit += 500;
						if (pnCurrentState != NULL)
							*pnCurrentState = nStage;//m_nActiveGroupIndex;
						return nRet;
					}
				}
				// Timeout for command stage, switch to trigger stage.
				InitRecognition(0);
			}
		}
	}
	else if (nRet == DSPOTTER_SUCCESS)
	{
		DSpotter_GetResultScore(m_hDSpotter, &m_nConfi, &m_nSGDiff, NULL);
		m_nCmdIndex = DSpotter_GetResult(m_hDSpotter);
		m_nCmdEnergy = DSpotter_GetCmdEnergy(m_hDSpotter);
		m_nRecogGroupIndex = m_nActiveGroupIndex;
		DSpotter_Reset(m_hDSpotter);
		m_nCommandRecordSample = 0;
		m_nCommandStageLimit = m_nCommandStageTimeout;

		if (m_nActiveGroupIndex == 0)
		{
			// Recognize trigger word, switch to command stage.
			//InitRecognition(1);
			InitRecognition(m_nActiveCommandsGroup);
		}
		else
		{
			int nID;

			CybModelGetCommandInfo(m_hCybModel, m_nRecogGroupIndex, m_nCmdIndex, NULL, 0, &nID, NULL);

			// If nID == 100 ~ 109(trigger word), we always keep in command stage.
			if (nID / 10 != 10)
			{
				if (!m_bCommandStageRepeatUntilTimeout)
					InitRecognition(0);
			}
		}
	}

	if (pnCurrentState != NULL)
		*pnCurrentState = nStage;

	return nRet;
}

int CDSpotter::GetRecogResult(int *pnID, char *lpszCommand, int nCmdLength, int *pnConfi, int *pnSGDiff, int *pnCmdEnergy)
{
	int nID;

	if (m_hDSpotter == NULL)
		return NotInit;

	if (lpszCommand != NULL)
	{
		CybModelGetCommandInfo(m_hCybModel, m_nRecogGroupIndex, m_nCmdIndex, lpszCommand, nCmdLength, &nID, NULL);
		if (strrchr(lpszCommand, '^') != NULL)
			strrchr(lpszCommand, '^')[0] = '\0';
		if (lpszCommand[strlen(lpszCommand) - 1] == ' ')
			lpszCommand[strlen(lpszCommand) - 1] = 0;
	}

	if (pnConfi != NULL)
		*pnConfi = m_nConfi;
	if (pnSGDiff != NULL)
		*pnSGDiff = m_nSGDiff;
	if (pnCmdEnergy != NULL)
		*pnCmdEnergy = m_nCmdEnergy;
	if (pnID != NULL)
		*pnID = nID;

	return Success;
}

int CDSpotter::GetCommandCount(int nStage)
{
	int nCount = 0;
	int nOrgCount;
	char szCommand[72];

	if (m_hCybModel == NULL)
		return NotInit;

	nOrgCount = CybModelGetCommandCount(m_hCybModel, nStage);
	for (int i = 0; i < nOrgCount; i++)
	{
		CybModelGetCommandInfo(m_hCybModel, nStage, i, szCommand, sizeof(szCommand), NULL, NULL);
		if (strlen(szCommand) > 0 && strchr(szCommand, '^') == NULL)
			nCount++;
	}

	return nCount;
}

int CDSpotter::GetCommand(int nStage, int nIndex, char *lpszCommand, int nCmdLength, int *pnID)
{
	int nCount = 0;
	int nOrgCount;

	if (m_hCybModel == NULL)
		return NotInit;

	nOrgCount = CybModelGetCommandCount(m_hCybModel, nStage);
	for (int i = 0; i < nOrgCount; i++)
	{
		CybModelGetCommandInfo(m_hCybModel, nStage, i, lpszCommand, nCmdLength, pnID, NULL);
		if (strlen(lpszCommand) > 0 && strchr(lpszCommand, '^') == NULL)
		{
			nCount++;
			if (nIndex == nCount - 1)
				return Success;
		}
	}

	if (lpszCommand != NULL && nCmdLength > 0)
		lpszCommand[0] = 0;

	return IllegalParam;
}

int CDSpotter::InitRecognition(int nGroupIndex)
{
	BYTE *lppbyModel[1];
	int nMemSize;
	int nRet;

	ReleaseRecognition();

	lppbyModel[0] = (BYTE*)CybModelGetGroup(m_hCybModel, nGroupIndex);
	nMemSize = DSpotter_GetMemoryUsage_Multi((BYTE *)CybModelGetBase(m_hCybModel), lppbyModel, 1, MAX_COMMAND_TIME);

	m_hDSpotter = DSpotter_Init_Multi((BYTE *)CybModelGetBase(m_hCybModel), lppbyModel, 1, MAX_COMMAND_TIME,
		m_lpbyMemPool, nMemSize, m_byaLicense, m_nLicenseSize, &nRet);

	if (m_hDSpotter != NULL)
		m_nActiveGroupIndex = nGroupIndex;
	else
		m_nActiveGroupIndex = -1;

	SetAGC(m_bEnableAGC, m_nScalePercentage);

	return nRet;
}

void CDSpotter::ReleaseRecognition()
{
	if (m_hDSpotter != NULL)
	{
		DSpotter_Release(m_hDSpotter);
		m_hDSpotter = NULL;
	}
	m_nActiveGroupIndex = -1;
}
