/**
 * @file hbpipe.c
 * @author Lailton (lailton@paysoft.com.br)
 * @brief Communication between process using PIPE
 */

#include <windows.h>
#include <stdio.h>

#include "hbapi.h"

typedef struct _HB_PIPE
{
	HANDLE hPipe;
	OVERLAPPED overlapped;
	BOOL isServer;
} HB_PIPE, *PHB_PIPE;

HB_FUNC(PIPE_CREATE)
{
	PHB_PIPE pPipe = NULL;
	const char *pipeName = hb_parc(1);
	DWORD timeout = hb_parnldef(2, 5000);

	SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES), NULL, FALSE};

	HANDLE hPipe = CreateNamedPipeA(
		pipeName,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		8 * 1024,
		8 * 1024,
		timeout,
		&saAttr);

	if (hPipe != INVALID_HANDLE_VALUE)
	{
		pPipe = (PHB_PIPE)hb_xgrabz(sizeof(HB_PIPE));
		pPipe->hPipe = hPipe;
		pPipe->isServer = TRUE;

		ZeroMemory(&pPipe->overlapped, sizeof(OVERLAPPED));
		pPipe->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (pPipe->overlapped.hEvent)
		{
			hb_retptr(pPipe);
		}
		else
		{
			CloseHandle(hPipe);
			hb_xfree(pPipe);
			pPipe = NULL;
		}
	}
}

HB_FUNC(PIPE_CONNECT)
{
	PHB_PIPE pPipe = NULL;
	const char *pipeName = hb_parc(1);

	HANDLE hPipe = CreateFileA(
		pipeName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (hPipe != INVALID_HANDLE_VALUE)
	{
		pPipe = (PHB_PIPE)hb_xgrabz(sizeof(HB_PIPE));
		pPipe->hPipe = hPipe;
		pPipe->isServer = FALSE;

		ZeroMemory(&pPipe->overlapped, sizeof(OVERLAPPED));
		pPipe->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (pPipe->overlapped.hEvent)
		{
			hb_retptr((PHB_PIPE)pPipe);
		}
		else
		{
			CloseHandle(hPipe);
			hb_xfree(pPipe);
			pPipe = NULL;
		}
	}
}

HB_FUNC(PIPE_DISCONNECT)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fVal = HB_FALSE;

	if (pPipe && pPipe->hPipe != INVALID_HANDLE_VALUE && pPipe->isServer)
	{
		FlushFileBuffers(pPipe->hPipe);
		fVal = DisconnectNamedPipe(pPipe->hPipe) ? HB_TRUE : HB_FALSE;
	}

	hb_retl(fVal);
}

HB_FUNC(PIPE_WAIT)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fVal = HB_FALSE;

	if (pPipe && pPipe->hPipe != INVALID_HANDLE_VALUE && pPipe->isServer)
	{
		ZeroMemory(&pPipe->overlapped, sizeof(OVERLAPPED));
		pPipe->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (ConnectNamedPipe(pPipe->hPipe, &pPipe->overlapped))
		{
			fVal = HB_TRUE;
		}
		else
		{
			DWORD error = GetLastError();

			if (error == ERROR_IO_PENDING)
			{
				DWORD wait = WaitForSingleObject(pPipe->overlapped.hEvent, INFINITE);

				if (wait == WAIT_OBJECT_0)
				{
					DWORD bytes;

					if (GetOverlappedResult(pPipe->hPipe, &pPipe->overlapped, &bytes, TRUE))
					{
						fVal = HB_TRUE;
					}
				}
			}
		}

		CloseHandle(pPipe->overlapped.hEvent);
		pPipe->overlapped.hEvent = NULL;
	}

	hb_retl(fVal);
}

HB_FUNC(PIPE_READ)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fVal = HB_FALSE;
	DWORD timeout = hb_parnldef(3, 1000);

	if (pPipe && pPipe->hPipe != INVALID_HANDLE_VALUE)
	{
		char *buffer = (char *)hb_xgrab(8192);
		DWORD bytesRead = 0;

		ZeroMemory(&pPipe->overlapped, sizeof(OVERLAPPED));
		pPipe->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (ReadFile(pPipe->hPipe, buffer, 8192, &bytesRead, &pPipe->overlapped))
		{
			if (bytesRead > 0)
			{
				if (HB_ISBYREF(2))
				{
					hb_storclen_buffer(buffer, bytesRead, 2);
				}
				else
				{
					hb_xfree(buffer);
				}

				fVal = HB_TRUE;
			}
		}
		else if (GetLastError() == ERROR_IO_PENDING)
		{
			if (WaitForSingleObject(pPipe->overlapped.hEvent, timeout) == WAIT_OBJECT_0)
			{
				if (GetOverlappedResult(pPipe->hPipe, &pPipe->overlapped, &bytesRead, TRUE) && bytesRead > 0)
				{
					if (HB_ISBYREF(2))
					{
						hb_storclen_buffer(buffer, bytesRead, 2);
					}
					else
					{
						hb_xfree(buffer);
					}

					fVal = HB_TRUE;
				}
			}
		}

		CloseHandle(pPipe->overlapped.hEvent);
		pPipe->overlapped.hEvent = NULL;

		if (!fVal)
		{
			hb_xfree(buffer);
		}
	}

	hb_retl(fVal);
}

HB_FUNC(PIPE_WRITE)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fVal = HB_FALSE;
	DWORD timeout = hb_parnldef(3, 1000);

	if (pPipe && pPipe->hPipe != INVALID_HANDLE_VALUE)
	{
		const char *data = hb_parc(2);
		DWORD dataLen = (DWORD)strlen(data);
		DWORD bytesWritten = 0;

		ZeroMemory(&pPipe->overlapped, sizeof(OVERLAPPED));
		pPipe->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (WriteFile(pPipe->hPipe, data, dataLen, &bytesWritten, &pPipe->overlapped))
		{
			if (bytesWritten == dataLen)
			{
				fVal = HB_TRUE;
			}
		}
		else if (GetLastError() == ERROR_IO_PENDING)
		{
			if (WaitForSingleObject(pPipe->overlapped.hEvent, timeout) == WAIT_OBJECT_0)
			{
				if (GetOverlappedResult(pPipe->hPipe, &pPipe->overlapped, &bytesWritten, TRUE) && bytesWritten == dataLen)
				{
					fVal = HB_TRUE;
				}
			}
		}

		CloseHandle(pPipe->overlapped.hEvent);
		pPipe->overlapped.hEvent = NULL;
	}

	hb_retl(fVal);
}

HB_FUNC(PIPE_FLUSH)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fVal = HB_FALSE;

	if (pPipe && pPipe->hPipe != INVALID_HANDLE_VALUE)
	{
		fVal = FlushFileBuffers(pPipe->hPipe) ? HB_TRUE : HB_FALSE;
	}

	hb_retl(fVal);
}

HB_FUNC(PIPE_FREE)
{
	PHB_PIPE pPipe = (PHB_PIPE)hb_parptr(1);
	HB_BOOL fVal = HB_FALSE;

	if (pPipe && pPipe->hPipe != INVALID_HANDLE_VALUE)
	{
		DisconnectNamedPipe(pPipe->hPipe);
		CloseHandle(pPipe->hPipe);
		pPipe->hPipe = INVALID_HANDLE_VALUE;

		if (pPipe->overlapped.hEvent)
		{
			CloseHandle(pPipe->overlapped.hEvent);
		}

		hb_xfree(pPipe);
		pPipe = NULL;

		fVal = HB_TRUE;
	}

	hb_retl(fVal);
}

HB_FUNC(PIPE_ERROR)
{
	hb_retnl(GetLastError());
}
