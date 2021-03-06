﻿
#include "ntghk.h"

#include <psapi.h>
#include <process.h>


//--------------------------------------
// DLL間で共有する変数
//--------------------------------------
namespace shared
{
#pragma data_seg(".shared")

	// キャプチャしたジェスチャーを通知するウィンドウ
	HWND daemonWnd = 0;

	// キャプチャのターゲットとなるプロセス名
	char processName[256] = {'\0'}; // 初期化しないと何故か共有されないので注意

	// _gTargetProcessExeName の大きさ
	size_t processNameLength = 0;

	// マウスジェスチャー有効
	bool isMouseGestureEnabled = true;
	
	// ロッカージェスチャー有効
	bool isRockerGestureEnabled = true;

	// ホイールジェスチャー有効
	bool isWheelGestureEnabled = true;

#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")
}



//--------------------------------------
// ユーティリティ関数
//--------------------------------------
namespace
{


	// エラーメッセージをフォーマットして返す
	std::string getLastErrorMessage()
	{
		DWORD code = ::GetLastError();
		LPVOID *lpMsgBuf;
		::FormatMessage(
			  FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			code,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPTSTR>(&lpMsgBuf),
			0,
			NULL
			);

		std::ostringstream oss;
		oss
			<< "GetLastError=" << code
			<< ", Message="    << reinterpret_cast<LPTSTR>(lpMsgBuf)
			;
		::LocalFree(lpMsgBuf);
		return oss.str();
	}

	
	// ウィンドウハンドルからプロセス名を取得
	bool getProcessNameByWindowHandle(HWND hWnd, char *name, size_t bufLen, size_t &len)
	{

		DWORD pid;
		DWORD tid = ::GetWindowThreadProcessId(hWnd, &pid);
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if(hProcess == NULL) {
			DEBUG_LOG("Failed to OpenProcess:"+ getLastErrorMessage());
			return false;
		}
		len = ::GetModuleBaseName(hProcess, NULL, name, bufLen);
		::CloseHandle(hProcess);
		return true;
	}
}



//--------------------------------------
// ジェスチャーのエンジン
//--------------------------------------
namespace
{


	// マウスジェスチャーの開始を通知
	void notifyMouseGestureBegin(HWND hWnd, int arrow)
	{
		//DEBUG_LOG("notifyMouseGestureBegin: called! daemonWnd="+ string_cast(shared::daemonWnd));
		if(shared::daemonWnd) {
			::PostMessage(shared::daemonWnd, ntg::WM_MGESTUREBEGIN, WPARAM(hWnd), LPARAM(arrow));
		}
	}

	
	// マウスジェスチャーの終了を通知
	void notifyMouseGestureEnd(uint32_t arrows, HWND hWnd)
	{
		//DEBUG_LOG("notifyMouseGestureEnd: called! daemonWnd="+ string_cast(shared::daemonWnd));
		if(shared::daemonWnd) {
			::PostMessage(shared::daemonWnd, ntg::WM_MGESTUREEND, arrows, LPARAM(hWnd));
		}
	}

	
	// マウスジェスチャーの進行を通知
	void notifyMouseGestureProgress(int arrow)
	{
		//DEBUG_LOG("notifyMouseGestureProgress: called! daemonWnd="+ string_cast(shared::daemonWnd));
		if(shared::daemonWnd) {
			::PostMessage(shared::daemonWnd, ntg::WM_MGESTUREPROGRESS, WPARAM(arrow), 0);
		}
	}
	
	
	// ロッカージェスチャーを通知
	void notifyRockerGestureEnd(bool isLeft, HWND hWnd)
	{
		//DEBUG_LOG("notifyRockerGestureEnd: called! daemonWnd="+ string_cast(shared::daemonWnd));
		if(shared::daemonWnd) {
			::PostMessage(shared::daemonWnd, ntg::WM_RGESTUREEND, (isLeft ? 0 : 1), LPARAM(hWnd));
		}
	}
	
	
	// ホイールジェスチャーを通知
	void notifyWheelGestureEnd(bool isUp, HWND hWnd)
	{
		//DEBUG_LOG("notifyRockerGestureEnd: called! daemonWnd="+ string_cast(shared::daemonWnd));
		if(shared::daemonWnd) {
			::PostMessage(shared::daemonWnd, ntg::WM_WGESTUREEND, (isUp ? 0 : 1), LPARAM(hWnd));
		}
	}



	// マウスジェスチャーの方向の列を記録する
	class Arrows
	{
	public:

		static const size_t kMaxArrows = 15;
		
		static const uint32_t kEmpty = 1U << 4;

		enum arrow_t { EAST, WEST, NORTH, SOUTH, INVALID_ARROW };
		
		Arrows()
		:arrows(kEmpty)
		,count(0)
		,_arrowPrev(INVALID_ARROW)
		{}

		bool add(arrow_t arrow)
		{
			if(arrow == _arrowPrev || count == kMaxArrows) {
				return false;
			}
			if(!count) {
				arrows = arrow & 0x03;
			}
			else {
				int a = arrow;
				if(arrow < _arrowPrev) {
					++a;
				}
				arrows |= (a & 0x03) << count * 2;
			}
			++count;
			_arrowPrev = arrow;
			return true;
		}

		static std::string stringify(uint32_t arrows)
		{
			if(arrows == kEmpty) {
				return "";
			}
			arrow_t arrow = static_cast<arrow_t>(arrows & 0x03);
			std::ostringstream oss;
			goto print;
			for(;;) {
				int tmp = (arrows >>= 2) & 0x03;
				if(!tmp) {
					break;
				}
				arrow = static_cast<arrow_t>( tmp <= arrow ? tmp - 1 : tmp );
			print:
				char c = 0;
				switch(arrow) {
				case INVALID_ARROW:
					// unreachable
					break;
				case  EAST: c = 'E'; break;
				case  WEST: c = 'W'; break;
				case NORTH: c = 'N'; break;
				case SOUTH: c = 'S'; break;
				}
				oss << c;
			}
			return oss.str();
		}

		uint32_t arrows;
		size_t count;

	private:
		arrow_t _arrowPrev;

	};



	// マウスジェスチャーを認識する人
	class GestureRecognizer
	{
	public:

		GestureRecognizer()
		:mouseGestureActive(false)
		,rockerGestureActive(false)
		,wheelGestureActive(false)
		,isCaptured(false)
		{}
		
		bool isActive() const
		{
			return mouseGestureActive || rockerGestureActive || wheelGestureActive;
		}
		
		void setGestureTarget(HWND hWnd)
		{
			//DEBUG_LOG("GestureRecognizer.setGestureTarget: called!");
			gestureTarget = hWnd;
		}
		
		HWND getGestureTarget() const
		{
			return gestureTarget;
		}
		
		// マウスジェスチャーのセッションを開始する
		void startMouseGesture(int x, int y)
		{
			mouseGestureActive = true;
			origX = x;
			origY = y;
			arrows = Arrows();
		}
		
		// ロッカージェスチャーの（ｒｙ
		void startRockerGesture(bool isLeft)
		{
			rockerGestureActive   = true;
			rockerGestureHasFired = false;
			isLeftDown  =  isLeft;
			isRightDown = !isLeft;
		}
		
		// ホイ（ｒｙ
		void startWheelGesture()
		{
			wheelGestureActive   = true;
			wheelGestureHasFired = false;
		}
		
		void move(int x, int y)
		{
			if(!mouseGestureActive) {
				return;
			}
			int dx = x - origX;
			int dy = y - origY;
			if(dx*dx + dy*dy < 30*30) {
				return;
			}
			
			static double const cos45 = 0.707106781;
			
			// [[cos(t), -sin(t)] ,[sin(t),  cos(t)] * [x, y]
			double dx_ = dx * cos45 - dy * cos45;
			double dy_ = dx * cos45 + dy * cos45;
			Arrows::arrow_t arrow
				= dx_ < 0
					? dy_ < 0 ? Arrows::WEST  : Arrows::SOUTH
					: dy_ < 0 ? Arrows::NORTH : Arrows::EAST
					;
			
			origX = x;
			origY = y;
			if(arrows.add(arrow)) {
				if(arrows.count == 1) {
					setCapture();
					notifyMouseGestureBegin(gestureTarget, int(arrow));
				}
				else {
					notifyMouseGestureProgress(int(arrow));
				}
			}
		}
		
		bool leftUp()
		{
			if(!(rockerGestureActive && isLeftDown)) {
				return false;
			}
			isLeftDown = false;
			if(!isRightDown) {
				setRockerGestureInactive();
			}
			return rockerGestureHasFired;
		}
		
		bool rightUp()
		{
			bool preventDefault = false;
			if(rockerGestureActive && isRightDown) {
				isRightDown = false;
				if(!isLeftDown) {
					setRockerGestureInactive();
				}
				preventDefault = rockerGestureHasFired;
			}
			if(wheelGestureActive) {
				setWheelGestureInactive();
				preventDefault = preventDefault || wheelGestureHasFired;
			}
			if(!mouseGestureActive) {
				return preventDefault;
			}
			setMouseGestureInactive();
			//tracerWindow.hide();
			if(!arrows.count) {
				notifyMouseGestureEnd(Arrows::kEmpty, 0);
				return preventDefault;
			}
			//DEBUG_LOG("COMPLETE GESTURE:"+ Arrows::stringify(arrows.arrows) +","+ string_cast(arrows.arrows));
			notifyMouseGestureEnd(arrows.arrows, gestureTarget);
			return true;
		}
		
		bool leftDown()
		{
			if(!rockerGestureActive || isLeftDown) {
				return false;
			}
			if(!rockerGestureHasFired) {
				rockerGestureHasFired = true;
				setCapture();
			}
			isLeftDown = true;
			notifyRockerGestureEnd(true, gestureTarget);
			return true;
		}
		
		bool rightDown()
		{
			if(!rockerGestureActive || isRightDown) {
				return false;
			}
			if(!rockerGestureHasFired) {
				rockerGestureHasFired = true;
				setCapture();
			}
			isRightDown = true;
			notifyRockerGestureEnd(false, gestureTarget);
			return true;
		}
		
		bool wheelUp()
		{
			if(!wheelGestureActive) {
				return false;
			}
			if(!wheelGestureHasFired) {
				wheelGestureHasFired = true;
				setCapture();
			}
			notifyWheelGestureEnd(true, gestureTarget);
			return true;
		}
		
		bool wheelDown()
		{
			if(!wheelGestureActive) {
				return false;
			}
			if(!wheelGestureHasFired) {
				wheelGestureHasFired = true;
				setCapture();
			}
			notifyWheelGestureEnd(false, gestureTarget);
			return true;
		}

	private:
	
		void setCapture()
		{
			if(!isCaptured) {
				::SetCapture(gestureTarget);
				isCaptured = true;
			}
		}
	
		void setRockerGestureInactive()
		{
			rockerGestureActive = false;
			inactivate();
		}
	
		void setMouseGestureInactive()
		{
			mouseGestureActive = false;
			inactivate();
		}
		
		void setWheelGestureInactive()
		{
			wheelGestureActive = false;
			inactivate();
		}
		
		void inactivate()
		{
			if(!isActive()) {
				//DEBUG_LOG("GestureRecognizer.inactivate: call ::ReleaseCapture()");
				if(isCaptured && !::ReleaseCapture()) {
					//DEBUG_LOG("GestureRecognizer.inactivate: failed to ::ReleaseCapture: "+ getLastErrorMessage());
				}
				isCaptured = false;
			}
		}

		bool mouseGestureActive;
		int origX;
		int origY;
		Arrows arrows;
		
		bool rockerGestureActive;
		bool rockerGestureHasFired;
		bool isLeftDown;
		bool isRightDown;
		
		bool wheelGestureActive;
		bool wheelGestureHasFired;
		
		HWND gestureTarget;
		bool isCaptured;

	};

}



//--------------------------------------
// メイン処理
//--------------------------------------
namespace
{


	HINSTANCE _hInstance;
	HHOOK _hHook = 0;
	GestureRecognizer _recognizer;
	enum { UNCHECKED, PASSED, IGNORED } _state = UNCHECKED;

	
	// マウスイベントを GestureRecognizer に通知。false を返したときは、メッセージをウィンドウに送らない
	inline bool dispatchMouseEvent(WPARAM msg, MOUSEHOOKSTRUCT &mouseHook)
	{
		switch(msg) {
		case WM_MOUSEMOVE:
			_recognizer.move(mouseHook.pt.x, mouseHook.pt.y);
			break;
			
		case WM_MOUSEWHEEL: {
				int delta = HIWORD(reinterpret_cast<MOUSEHOOKSTRUCTEX&>(mouseHook).mouseData);
				if(delta & 0x8000) {
					delta |= -1 << 0x10;
				}
				if(-WHEEL_DELTA < delta && delta < WHEEL_DELTA) {
					break;
				}
				return delta > 0 ? _recognizer.wheelUp() : _recognizer.wheelDown() ;
			}

		case   WM_LBUTTONUP: return _recognizer.leftUp();
		case   WM_RBUTTONUP: return _recognizer.rightUp();
		case WM_LBUTTONDOWN: return _recognizer.leftDown();
		case WM_RBUTTONDOWN: return _recognizer.rightDown();
		}
		return false;
	}
	
	
	// マウスメッセージのフックプロシージャ
	LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if(nCode != HC_ACTION || _state == IGNORED) {
			goto fallthrough;
		}
		if(!_recognizer.isActive()) {
			switch(wParam) {
			default:
				goto fallthrough;
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
				break;
			}
			if(_state == UNCHECKED) {
				
				// 指定プロセス(shared::processName)の表示領域宛のメッセージかどうかチェック
				HANDLE hProcess = ::GetCurrentProcess();
				if(hProcess == NULL) {
					//DEBUG_LOG("failed to ::GetCurrentProcess: "+ getLastErrorMessage());
					goto fallthrough;
				}
				char name[1024];
				size_t len = ::GetModuleBaseName(hProcess, NULL, name, sizeof(name));
				DEBUG_LOG("MouseProc: name="
					+ std::string(name, len) +", "
					+ std::string(shared::processName, shared::processNameLength));

				if(len != shared::processNameLength
					|| !std::equal(name, name + len, shared::processName))
				{
					_state = IGNORED;
					goto fallthrough;
				}
			}
			_state = PASSED;
			MOUSEHOOKSTRUCT *mouseHook = reinterpret_cast<MOUSEHOOKSTRUCT*>(lParam);
			HWND hWnd = mouseHook->hwnd;
			if(hWnd == NULL) {
				goto fallthrough;
			}
			_recognizer.setGestureTarget(hWnd);
			
			bool isLeft = wParam == WM_LBUTTONDOWN;
			if(!isLeft) {
				if(shared::isMouseGestureEnabled) _recognizer.startMouseGesture(mouseHook->pt.x, mouseHook->pt.y);
				if(shared::isWheelGestureEnabled) _recognizer.startWheelGesture();
			}
			if(shared::isRockerGestureEnabled) {
				_recognizer.startRockerGesture(isLeft);
			}
			
			//DEBUG_LOG("MouseProc: initialized. pid="+ string_cast(::GetCurrentProcessId()) +", id="+ string_cast((unsigned)_hInstance));
			
			goto fallthrough;
		}
		if(dispatchMouseEvent(wParam, *reinterpret_cast<MOUSEHOOKSTRUCT*>(lParam))) {
			//DEBUG_LOG("MouseProc: preventDefault!");
			::CallNextHookEx(_hHook, nCode, wParam, lParam);
			return 1;
		}
	fallthrough:
		return ::CallNextHookEx(_hHook, nCode, wParam, lParam);
	}

}


BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpvReserved)
{
	_hInstance = hInst;
	switch(fdwReason) {
	case DLL_PROCESS_ATTACH: break;
	case DLL_PROCESS_DETACH: break;
	case  DLL_THREAD_ATTACH: break;
	case  DLL_THREAD_DETACH: break;
	}
	return  TRUE;
}


int WINAPI ntghk_installHook()
{
	if(!_hHook) {
		_hHook = ::SetWindowsHookEx(WH_MOUSE, MouseProc, _hInstance, 0);
		if(!_hHook) {
			return 1;
		}
	}
	return 0;
}


void WINAPI ntghk_uninstallHook()
{
	if(_hHook) {
		::UnhookWindowsHookEx(_hHook);
		_hHook = 0;
	}
}


int WINAPI ntghk_setProcessName(const char *name, size_t len)
{
	DEBUG_LOG("ntghk_setProcessName: name="+ std::string(name, len));
	if(len == 0 || len > sizeof(shared::processName)) {
		return 1;
	}
	std::copy(name, name + len, shared::processName);
	shared::processNameLength = len;
	return 0;
}


void WINAPI ntghk_setDaemonHandle(HWND hWnd)
{
	DEBUG_LOG("ntghk_setDaemonHandle: hWnd="+ string_cast((unsigned)hWnd));
	shared::daemonWnd = hWnd;
}


void WINAPI ntghk_setGestureMask(int mask)
{
	DEBUG_LOG("ntghk_setGestureMask: mask="+ string_cast(mask));
	shared::isMouseGestureEnabled  = !!(mask & 1);
	shared::isRockerGestureEnabled = !!(mask & 2);
	shared::isWheelGestureEnabled  = !!(mask & 4);
}

