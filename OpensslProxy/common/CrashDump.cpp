#include "CrashDump.h"

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#pragma warning(push)
#pragma warning(disable : 4091)
#include <DbgHelp.h>
#pragma warning(pop)
#include <exception>
#include <signal.h>
#include <stdio.h>
#include <Shlobj.h>



#define PROCESS_CALLBACK_FILTER_ENABLED 0x1

BOOL
(WINAPI*
	MiniDumpWriteDumpPtr)(
		_In_ HANDLE hProcess,
		_In_ DWORD ProcessId,
		_In_ HANDLE hFile,
		_In_ MINIDUMP_TYPE DumpType,
		_In_opt_ PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		_In_opt_ PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		_In_opt_ PMINIDUMP_CALLBACK_INFORMATION CallbackParam
		);

BOOL
(WINAPI*
	SetProcessUserModeExceptionPolicyPtr)(
		_In_ DWORD PolicyFlags
		);

BOOL
(WINAPI*
	GetProcessUserModeExceptionPolicyPtr)(
		_Out_ LPDWORD PolicyFlags
		);

void CrashDumpFatal(const char* Format, ...)
{
    char buffer[1024] = { 0 };
	va_list va;

	va_start(va, Format);
	vsnprintf_s(buffer, _TRUNCATE, Format, va);
	va_end(va);

	MessageBoxA(nullptr, buffer, "Error", MB_ICONERROR);
}

void CrashDumpCreate(EXCEPTION_POINTERS* ExceptionPointers)
{
	// Generate a crash dump file in the minidump directory
	wchar_t dumpFile[MAX_PATH] = {0};
	wchar_t dumpDir[MAX_PATH] = { 0 };
	wchar_t moduleName[MAX_PATH] = { 0 };
	wchar_t messageerror[MAX_PATH] = {0};

	if (S_OK != SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_DEFAULT, dumpDir))
	{
		CrashDumpFatal("Unable to obtain current directory during crash dump");
		return;
	}

	//MessageBox(NULL, dumpDir, dumpDir, MB_OK);

	wcscat_s(dumpDir, DumpDirName);
	CreateDirectoryW(dumpDir,nullptr);

	wcscat_s(dumpDir, DumpMiniName);
	CreateDirectoryW(dumpDir, nullptr);

	// Append the name with generated timestamp
	SYSTEMTIME st;
	GetLocalTime(&st);

	swprintf_s(dumpFile, L"%ws\\trasitservice_%04d%02d%02d_%02d%02d%02d%04d.dmp", dumpDir,
		st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond,
		st.wMilliseconds);

	//MessageBox(NULL, L"ddd", L"ddd", MB_OK);
	//MessageBox(NULL, dumpFile, dumpFile, MB_OK);
	// Open the file
	HANDLE fileHandle = CreateFileW(dumpFile, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		CrashDumpFatal("Failed to open file path '%ws' while generating crash dump", dumpFile);
		return;
	}

	// Create the minidump with DbgHelp
	MINIDUMP_EXCEPTION_INFORMATION info;
	memset(&info, 0, sizeof(MINIDUMP_EXCEPTION_INFORMATION));

	info.ThreadId = GetCurrentThreadId();
	info.ExceptionPointers = ExceptionPointers;
	info.ClientPointers = TRUE;

	if (!MiniDumpWriteDumpPtr(GetCurrentProcess(), GetCurrentProcessId(), fileHandle, MiniDumpNormal, &info, nullptr, nullptr))
				CrashDumpFatal("MiniDumpWriteDump failed. Error: %u", GetLastError());

	// Close the file & done
	CloseHandle(fileHandle);

	GetModuleFileName(NULL, moduleName, MAX_PATH);

	if ( '\0' != moduleName[0])
	{
		swprintf_s(messageerror, L"%s:\n程序程序发生异常，请联系管理员!", moduleName);
		FatalAppExit(1, messageerror);
	}
	//MessageBoxA(NULL, pcStr, pcStr, MB_OK);
}

LONG CALLBACK CrashDumpExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	// Any "exception" under 0x1000 is usually a failed remote procedure call (RPC)
	if (ExceptionInfo && ExceptionInfo->ExceptionRecord->ExceptionCode > 0x1000)
	{
		switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
		{
		case DBG_PRINTEXCEPTION_C:  // OutputDebugStringA
		case 0x4001000A:            // OutputDebugStringW
		case STATUS_INVALID_HANDLE: // Invalid TitanEngine handle
		case 0x406D1388:            // SetThreadName
			return EXCEPTION_CONTINUE_SEARCH;
		}

		CrashDumpCreate(ExceptionInfo);
	}

	//return EXCEPTION_CONTINUE_SEARCH;

	return EXCEPTION_EXECUTE_HANDLER;
}

void InvalidParameterHandler(const wchar_t* Expression, const wchar_t* Function, const wchar_t* File, unsigned int Line, uintptr_t Reserved)
{
	CrashDumpFatal("Invalid parameter passed to CRT function! Program will now generate an exception.\n\nFile: %ws\nFunction: %ws\nExpression: %ws",
		Function ? Function : L"???",
		File ? File : L"???",
		Expression ? Expression : L"???");

	UNREFERENCED_PARAMETER(Reserved);
	UNREFERENCED_PARAMETER(Line);
	// RaiseException: avoid the use of throw/C++ exceptions and possible __fastfail calls
	RaiseException(0x78646267, EXCEPTION_NONCONTINUABLE, 0, nullptr);
}

void TerminateHandler()
{
	CrashDumpFatal("Process termination was requested in an unusual way. Program will now generate an exception.");

	// See InvalidParameterHandler()
	RaiseException(0x78646267, EXCEPTION_NONCONTINUABLE, 0, nullptr);
}

void AbortHandler(int Signal)
{
	TerminateHandler();
	UNREFERENCED_PARAMETER(Signal);
}


void CrashDumpInitialize()
{
	// Get handles to kernel32 and dbghelp
	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
	HMODULE hDbghelp = LoadLibraryA("dbghelp.dll");

	if (hDbghelp)
		*(FARPROC*)&MiniDumpWriteDumpPtr = GetProcAddress(hDbghelp, "MiniDumpWriteDump");

	if (hKernel32)
	{
		*(FARPROC*)&SetProcessUserModeExceptionPolicyPtr = GetProcAddress(hKernel32, "SetProcessUserModeExceptionPolicy");
		*(FARPROC*)&GetProcessUserModeExceptionPolicyPtr = GetProcAddress(hKernel32, "GetProcessUserModeExceptionPolicy");
	}

	if (MiniDumpWriteDumpPtr)
		SetUnhandledExceptionFilter(CrashDumpExceptionHandler);

	if (SetProcessUserModeExceptionPolicyPtr && GetProcessUserModeExceptionPolicyPtr)
	{
		DWORD flags = 0;

		if (GetProcessUserModeExceptionPolicyPtr(&flags))
			SetProcessUserModeExceptionPolicyPtr(flags & ~PROCESS_CALLBACK_FILTER_ENABLED);
	}

	// If not running under a debugger, redirect purecall, terminate, abort, and
	// invalid parameter callbacks to force generate a dump.
	if (!IsDebuggerPresent())
	{
		_set_purecall_handler(TerminateHandler);                // https://msdn.microsoft.com/en-us/library/t296ys27.aspx
		_set_invalid_parameter_handler(InvalidParameterHandler);// https://msdn.microsoft.com/en-us/library/a9yf33zb.aspx
		set_terminate(TerminateHandler);                        // http://en.cppreference.com/w/cpp/error/set_terminate
		signal(SIGABRT, AbortHandler);                          // https://msdn.microsoft.com/en-us/library/xdkz3x12.aspx
	}
}