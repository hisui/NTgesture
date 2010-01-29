#ifndef _NTG_NPP_H_
#define _NTG_NPP_H_

#include "ntg.h"

namespace ntg
{

	void fireMouseGestureBegin();
	void fireMouseGestureEnd(uint32_t arrows, HWND hWnd);
	void fireMouseGestureProgress();

	void fireRockerGestureEnd(bool isLeft, HWND hWnd);
	
	void fireWheelGestureEnd(bool isUp, HWND hWnd);
}

#endif
