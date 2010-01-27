#ifndef _NTG_NTGHK_H_
#define _NTG_NTGHK_H_

#include "ntg.h"


BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpvReserved);

// ジェスチャーのためのフックを登録する
int WINAPI installHook();

// 登録されているフックを解除して、ジェスチャーに関わるリソースを開放する
void WINAPI uninstallHook();

// ジェスチャーの対象となるプロセスの名前を設定する
int WINAPI setProcessName(const char *name, size_t len);

// ジェスチャーを通知するウィンドウを設定する
void WINAPI setDaemonHandle(HWND hWnd);

#endif
