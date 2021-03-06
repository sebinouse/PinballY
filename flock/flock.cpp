// flock - Windows equivalent of Unix "flock" program
//
// Usage:
//
//   flock -m <mutex> <command line>
//
// <mutex> is the name a mutex, which is used for synchronization
// between this process and any other processes trying to perform
// the same command.  If any other process is holding the same
// mutex, we wait indefinitely for it to exit.
//
// <command line> is passed to CMD for processing.
//

#include "stdafx.h"

typedef std::basic_string<TCHAR> TSTRING;
typedef std::basic_regex<TCHAR> TREGEX;

int main()
{
	// get the original command line
	TCHAR *cmdline = GetCommandLine();

	// parse out the file lock portion
	TREGEX optsPat(_T("\\s*(?:\\w+|\"[^\"]*\")\\s+-m\\s+([\\w._]+)\\s+(.*)"));
	std::match_results<const TCHAR*> m;
	if (!std::regex_match(cmdline, m, optsPat))
	{
		fprintf(stderr, "usage: flock -m <mutex> <command line>\n");
		return 1;
	}

	// pull out the mutex name
	TSTRING mutexName = m[1].str();

	// build the CMD.EXE command params
	TSTRING subcmd = _T(" /c ");
	subcmd += m[2].str();

	// acquire the mutex while running our command
	HANDLE hMutex = CreateMutex(NULL, FALSE, mutexName.c_str());

	// wait for the mutex
	if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0)
	{
		fprintf(stderr, "mutex wait failed\n");
		return 2;
	}

	// get CMD.EXE
	TCHAR comspec[1024] = { 0 };
	GetEnvironmentVariable(_T("COMSPEC"), comspec, sizeof(comspec)/sizeof(comspec[0]));
	if (comspec[0] == 0)
	{
		fprintf(stderr, "unable to find cmd.exe (COMSPEC environment variable not set)\n");
		return 3;
	}

	// set up the startup info
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW | STARTF_USESTDHANDLES;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	// create the process
	PROCESS_INFORMATION pi;
	if (!CreateProcess(comspec, subcmd.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		DWORD err = GetLastError();
		fprintf(stderr, "CreateProcess failed, error code %d\n", (int)err);
		return 3;
	}

	// wait for the process
	WaitForSingleObject(pi.hProcess, INFINITE);

	// get the exit code
	DWORD exitCode;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	// close the process and thread handles
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	// release the mutex
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

	// return the result from the subprocess
	return (int)exitCode;
}

