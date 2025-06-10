#include <windows.h>
#include <stdio.h>

#include "hbapi.h"

typedef struct _HB_PIPE
{
	void *hPipe;
} HB_PIPE, *PHB_PIPE;

HB_FUNC(PIPE_CREATE)
{
	PHB_PIPE pPipe = NULL;
	const char *pipeName = hb_parc(1);

	HANDLE hPipe = CreateNamedPipeA(
		pipeName,
		PIPE_ACCESS_DUPLEX, // | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		128,
		128,
		0,
		NULL);

	if (hPipe != INVALID_HANDLE_VALUE)
	{
		pPipe = (PHB_PIPE)hb_xgrab(sizeof(HB_PIPE));
		pPipe->hPipe = hPipe;

		DWORD mode = hb_parnldef(2, PIPE_READMODE_MESSAGE);
		SetNamedPipeHandleState(pPipe->hPipe, &mode, NULL, NULL);

		hb_retptr((PHB_PIPE)pPipe);
	}
}

HB_FUNC(PIPE_FREE)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fVal = HB_FALSE;

	if (pPipe)
	{
		if (pPipe->hPipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(pPipe->hPipe);
			pPipe->hPipe = INVALID_HANDLE_VALUE;
		}
		hb_xfree(pPipe);
		pPipe = NULL;
		fVal = HB_TRUE;
	}

	hb_retl(fVal);
}

HB_FUNC(PIPE_WAIT)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fRet = HB_FALSE;

	if (pPipe && pPipe->hPipe)
	{
		BOOL result = ConnectNamedPipe(pPipe->hPipe, NULL);
		if (result)
		{
			fRet = HB_TRUE;
		}
	}

	hb_retl(fRet);
}

HB_FUNC(PIPE_DISCONNECT)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fRet = HB_FALSE;
	if (pPipe && pPipe->hPipe)
	{
		fRet = DisconnectNamedPipe(pPipe->hPipe) ? HB_TRUE : HB_FALSE;
	}
	hb_retl(fRet);
}

HB_FUNC(PIPE_CONNECT)
{
	HANDLE hPipe = CreateFileA(
		hb_parc(1),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0, //FILE_FLAG_OVERLAPPED,
		NULL);

	if (hPipe != INVALID_HANDLE_VALUE)
	{
		PHB_PIPE pPipe = (PHB_PIPE)hb_xgrab(sizeof(HB_PIPE));
		pPipe->hPipe = hPipe;

		hb_retptr((PHB_PIPE)pPipe);
	}
}

HB_FUNC(PIPE_READ)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fVal = HB_FALSE;

	if (pPipe && pPipe->hPipe)
	{
		HB_SIZE nLen = 128;
		HB_SIZE totalRead = 0;
		char *buffer = NULL;
		DWORD bytesRead = 0;
		BOOL result;

		do
		{

			DWORD available = 0;
			if (PeekNamedPipe(pPipe->hPipe, NULL, 0, NULL, &available, NULL) && available > 0)
			{

				buffer = (char *)hb_xrealloc(buffer, totalRead + nLen + 1);
				if (!buffer)
					break;

				result = ReadFile(pPipe->hPipe, buffer + totalRead, (DWORD)nLen, &bytesRead, NULL);
				totalRead += bytesRead;
			}
			else
			{
				break;
			}

		} while (GetLastError() == ERROR_MORE_DATA);

		if (totalRead > 0 && buffer)
		{
			if (HB_ISBYREF(2))
				hb_storclen_buffer(buffer, totalRead, 2);
			else
				hb_xfree(buffer);

			fVal = HB_TRUE;
		}
	}

	hb_retl(fVal);
}

HB_FUNC(PIPE_WRITE)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fVal = HB_FALSE;
	if (pPipe && pPipe->hPipe != INVALID_HANDLE_VALUE)
	{
		const char *data = hb_parc(2);
		if( data )
		{
			DWORD dataLen = (DWORD) strlen(data);
			DWORD bytesWritten;
			BOOL result = WriteFile(pPipe->hPipe, data, dataLen, &bytesWritten, NULL);
			if (result && bytesWritten == dataLen)
			{
				fVal = HB_TRUE;
			}
		}
	}

	hb_retl(fVal);
}

HB_FUNC(PIPE_ERROR)
{
	hb_retnl(GetLastError());
}