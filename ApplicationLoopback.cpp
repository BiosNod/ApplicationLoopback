// ApplicationLoopback.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <iostream>
#include "LoopbackCapture.h"
#include <conio.h>

void usage()
{
    std::wcout <<
        L"Usage: ApplicationLoopback <pid> <includetree|excludetree> <outputfilename> [-silence] [-skipheaders]\n"
        L"\n"
        L"<pid> is the process ID to capture or exclude from capture\n"
        L"includetree includes audio from that process and its child processes\n"
        L"excludetree includes audio from all processes except that process and its child processes\n"
        L"<outputfilename> is the WAV file to receive the captured audio, or '-stream' to output to stdout\n"
        L"[-silence] is an optional flag to skip recording silence (only record when sound is present)\n"
        L"[-skipheaders] is an optional flag to skip WAV headers when using -stream mode\n"
        L"\n"
        L"Examples:\n"
        L"\n"
        L"ApplicationLoopback 1234 includetree CapturedAudio.wav\n"
        L"\n"
        L"  Captures audio from process 1234 and its children.\n"
        L"\n"
        L"ApplicationLoopback 1234 excludetree CapturedAudio.wav\n"
        L"\n"
        L"  Captures audio from all processes except process 1234 and its children.\n"
        L"\n"
        L"ApplicationLoopback 1234 includetree -stream\n"
        L"\n"
        L"  Captures audio from process 1234 and outputs to stdout without redirection.\n"
        L"ApplicationLoopback 1234 includetree -stream > output.wav\n"
        L"\n"
        L"  Captures audio from process 1234 and outputs to stdout, which can be redirected.\n"
        L"\n"
        L"ApplicationLoopback 1234 includetree -stream -silence\n"
        L"\n"
        L"  Captures audio from process 1234, outputs to stdout without redirection, and skips silence.\n"
        L"\n"
        L"ApplicationLoopback 1234 includetree CapturedAudio.wav -silence\n"
        L"\n"
        L"  Captures audio from process 1234 and its children, skipping periods of silence.\n"
        L"\n"
        L"ApplicationLoopback 1234 includetree -stream -skipheaders\n"
        L"\n"
        L"  Captures audio from process 1234 and outputs to stdout without WAV headers.\n"
        L"\n"
        L"ApplicationLoopback 1234 includetree -stream -silence -skipheaders\n"
        L"\n"
        L"  Captures audio from process 1234, skips silence, and outputs to stdout without WAV headers.\n";
}

bool ProcessExists(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess != NULL)
    {
        CloseHandle(hProcess);
        return true;
    }
    return false;
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 4 || argc > 6)
    {
        usage();
        return 0;
    }

    DWORD processId = wcstoul(argv[1], nullptr, 0);
    if (processId == 0)
    {
        usage();
        return 0;
    }

    // Check if the process exists
    if (!ProcessExists(processId))
    {
        std::wcerr << L"Error: Process with ID " << processId << L" does not exist or is not accessible.\n";
        return 1;
    }

    bool includeProcessTree;
    if (wcscmp(argv[2], L"includetree") == 0)
    {
        includeProcessTree = true;
    }
    else if (wcscmp(argv[2], L"excludetree") == 0)
    {
        includeProcessTree = false;
    }
    else
    {
        usage();
        return 0;
    }

    PCWSTR outputFile = argv[3];

    // Check for optional flags
    bool skipSilence = false;
    bool skipHeaders = false;

    for (int i = 4; i < argc; i++)
    {
        if (wcscmp(argv[i], L"-silence") == 0)
        {
            skipSilence = true;
        }
        else if (wcscmp(argv[i], L"-skipheaders") == 0)
        {
            skipHeaders = true;
        }
        else
        {
            std::wcerr << L"Unknown parameter: " << argv[i] << L"\n";
            usage();
            return 1;
        }
    }

    // Check if skipHeaders is used with a file output (not valid)
    if (skipHeaders && wcscmp(outputFile, L"-stream") != 0)
    {
        std::wcerr << L"Error: -skipheaders flag can only be used with -stream output mode.\n";
        return 1;
    }

    // If stream mode is chosen, inform about status
    bool isStreamMode = (wcscmp(outputFile, L"-stream") == 0);

    CLoopbackCapture loopbackCapture;
    HRESULT hr = loopbackCapture.StartCaptureAsync(processId, includeProcessTree, outputFile, skipSilence, skipHeaders);
    if (FAILED(hr))
    {
        if (!isStreamMode)
        {
            wil::unique_hlocal_string message;
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, hr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (PWSTR)&message, 0, nullptr);
            std::wcout << L"Failed to start capture\n0x" << std::hex << hr << L": " << message.get() << L"\n";
        }
    }
    else
    {
        if (!isStreamMode)
        {
            std::wcout << L"Capturing audio" << (skipSilence ? L" (skipping silence)" : L"")
                << (skipHeaders && isStreamMode ? L" (skipping WAV headers)" : L"")
                << L". Press 'Q' to stop..." << std::endl;
        }

        // Wait for Q key to stop
        bool stopCapture = false;
        while (!stopCapture)
        {
            if (_kbhit()) // Check if key was pressed
            {
                int key = _getch(); // Get the key code
                if (key == 'q' || key == 'Q')
                {
                    stopCapture = true;
                }
            }
            Sleep(100); // Small delay to prevent high CPU usage
        }

        loopbackCapture.StopCaptureAsync();

        if (!isStreamMode)
        {
            std::wcout << L"Finished.\n";
        }
    }
    return 0;
}