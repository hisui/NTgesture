#ifndef _NTG_H_
#define _NTG_H_

#define _WIN32_WINNT 0x0500

#include <npapi.h>
#include <npfunctions.h>
#include <npruntime.h>
#include <windows.h>
#include <stdlib.h>

#include <stdexcept>
#include <string>
#include <fstream>
#include <sstream>

namespace ntg {

	enum {
		
		// マウスジェスチャー通知用のメッセージ
		WM_MGESTUREBEGIN = WM_APP + 1,
		WM_MGESTUREEND,
		WM_MGESTUREPROGRESS,
		
		// ロッカージェスチャー（ｒｙ
		WM_RGESTUREEND,
		
		// ホイほい
		WM_WGESTUREEND,
	};
}

#ifdef DEBUG_ME
#define DEBUG_LOG(MSG) debugPrint(MSG)

static void debugPrint(const std::string &msg)
{
	std::ofstream fout("C:\\Devel\\tmp\\log.txt", std::ios::out | std::ios::app);
	fout << msg << std::endl;
}

template<typename T> std::string string_cast(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

#else
#define DEBUG_LOG(MSG) 

#endif

#endif
