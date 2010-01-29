#ifndef _NTG_NTGHK_H_
#define _NTG_NTGHK_H_

#include "ntg.h"


BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpvReserved);

// ジェスチャーのためのフックを登録する
int WINAPI ntghk_installHook();

// 登録されているフックを解除して、ジェスチャーに関わるリソースを開放する
void WINAPI ntghk_uninstallHook();

// ジェスチャーの対象となるプロセスの名前を設定する
int WINAPI ntghk_setProcessName(const char *name, size_t len);

// ジェスチャーを通知するウィンドウを設定する
void WINAPI ntghk_setDaemonHandle(HWND hWnd);

// ジェスチャーマスク(各ジェスチャーの有効・無効を表す)を設定する
void WINAPI ntghk_setGestureMask(int mask);

#endif
