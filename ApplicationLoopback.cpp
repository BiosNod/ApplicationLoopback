// ApplicationLoopback.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <iostream>
#include "LoopbackCapture.h"
#include <conio.h>

void usage()
{
    std::wcout <<
        L"Usage: ApplicationLoopback <pid> <includetree|excludetree> <outputfilename>\n"
        L"\n"
        L"<pid> is the process ID to capture or exclude from capture\n"
        L"includetree includes audio from that process and its child processes\n"
        L"excludetree includes audio from all processes except that process and its child processes\n"
        L"<outputfilename> is the WAV file to receive the captured audio, or '-stream' to output to stdout\n"
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
        L"ApplicationLoopback 1234 includetree -stream > output.wav\n"
        L"\n"
        L"  Captures audio from process 1234 and outputs to stdout, which can be redirected.\n";
        L"ApplicationLoopback 1234 includetree -stream\n"
        L"\n"
        L"  Captures audio from process 1234 and outputs to stdout without redirection.\n";
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
    if (argc != 4)
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

    // ≈сли используетс€ режим потока, не выводим сообщени€ в консоль, чтобы не смешивать с бинарными данными
    bool isStreamMode = (wcscmp(outputFile, L"-stream") == 0);

    CLoopbackCapture loopbackCapture;
    HRESULT hr = loopbackCapture.StartCaptureAsync(processId, includeProcessTree, outputFile);
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
            std::wcout << L"Capturing audio. Press 'Q' to stop..." << std::endl;
        }

        // ќжидаем нажати€ клавиши Q дл€ остановки
        bool stopCapture = false;
        while (!stopCapture)
        {
            if (_kbhit()) // ѕровер€ем, нажата ли клавиша
            {
                int key = _getch(); // ѕолучаем код нажатой клавиши
                if (key == 'q' || key == 'Q')
                {
                    stopCapture = true;
                }
            }
            Sleep(100); // Ќебольша€ задержка дл€ снижени€ нагрузки на CPU
        }

        loopbackCapture.StopCaptureAsync();

        if (!isStreamMode)
        {
            std::wcout << L"Finished.\n";
        }
    }
    return 0;
}