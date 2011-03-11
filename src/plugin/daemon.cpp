
#include "daemon.h"
#include "npp.h"
#include "ntghk.h"

#include <deque>
#include <process.h>

#define ARROW_ICON_WIDTH  32
#define ARROW_ICON_HEIGHT 32

namespace
{


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

	
	// マウスジェスチャー実行時に表示を行う人
	class TracerWindow
	{
	public:

		TracerWindow()
		:_hWnd(0)
		,_timerId(0)
		{
		
			::CreateWindowEx(
				  WS_EX_LAYERED
				| WS_EX_TOPMOST
				| WS_EX_TRANSPARENT
				| WS_EX_NOACTIVATE
				, reinterpret_cast<LPCSTR>(getWindowClass()),
				"",
				WS_VISIBLE | WS_POPUP,   // ウィンドウの種類
				0,                       // ウィンドウを表示する位置（X座標）
				0,                       // ウィンドウを表示する位置（Y座標）
				0,                       // ウィンドウの幅
				0,                       // ウィンドウの高さ
				NULL,                    // 親ウィンドウのウィンドウハンドル
				NULL,                    // メニューハンドル
				::GetModuleHandle(NULL), // インスタンスハンドル
				this                     // その他の作成データ
			);
			//::SetLayeredWindowAttributes(_hWnd, 0, 50, LWA_ALPHA);
			::SetLayeredWindowAttributes(_hWnd, 0, 0, LWA_COLORKEY);
			
		}
		
		~TracerWindow()
		{
			if(_hWnd) {
				::DestroyWindow(_hWnd);
			}
		}
		
		void show(HWND hWnd)
		{
			if(!_hWnd) {
				return;
			}
			RECT rect;
			::GetWindowRect(hWnd, &rect);

			int width  = ARROW_ICON_WIDTH * 15;
			int height = ARROW_ICON_HEIGHT;
			::SetWindowPos(_hWnd, NULL, rect.left, rect.bottom - height, width, height, SWP_SHOWWINDOW);
				
			//::SetWindowPos(_hWnd, NULL,
			//	(rect.right  + rect.left - width ) / 2,
			//	(rect.bottom + rect.top  - height) / 2,
			//	width, height, SWP_SHOWWINDOW);
		}
		
		void hide()
		{
			if(_hWnd) {
				::ShowWindow(_hWnd, SW_HIDE);
				_animationState.step = 2;
				_animationQueue.clear();
				_displayIcons.clear();
			}
		}
		
		void addArrow(int arrow)
		{
			if(_hWnd && 0 <= arrow && arrow < 4) {
				//DEBUG_LOG("TracerWindow.addArrow: arrow="+ string_cast(arrow));
				_animationQueue.push_back(arrow);
				if(!_timerId) {
					_timerId = ::SetTimer(_hWnd, 1, 15, NULL);
				}
			}
		}
		
		void getSize(size_t &width, size_t &height) const
		{
			RECT rect;
			::GetWindowRect(_hWnd, &rect);
			width  = rect.right  - rect.left ;
			height = rect.bottom - rect.top  ;
		}

	private:

		static int getWindowClass()
		{
			static int classAtom = 0;
			if(!classAtom) {

				WNDCLASSEX wc;

				wc.cbSize = sizeof(wc);
				wc.style = CS_HREDRAW | CS_VREDRAW;
				wc.lpfnWndProc = &WndProc;
				wc.cbClsExtra = 0;
				wc.cbWndExtra = 0;
				wc.hInstance = ::GetModuleHandle(NULL);
				wc.hIcon   = NULL;
				wc.hIconSm = NULL;
				wc.hCursor = NULL;
				wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
				wc.lpszMenuName = NULL;
				wc.lpszClassName = "ntg::TracerWindow";

				if((classAtom = ::RegisterClassEx( &wc )) == 0) {
					DEBUG_LOG("failed to RegisterClassEx: "+ getLastErrorMessage());
				}
			}
			return classAtom;
		}

		LRESULT onMessage(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
		{

			switch( msg ) {

			case WM_DESTROY:
				::PostQuitMessage(0);
				return 0;

			case WM_CREATE:
				_onCreate();
				break;

			case WM_TIMER:
				//DEBUG_LOG("TracerWindow.onMessage: WM_TIMER");
				::InvalidateRect(hWnd, NULL, FALSE);
				break;
				
			case WM_PAINT: {
			
					HDC hDC = ::GetDC(hWnd);
			
					size_t width;
					size_t height;
					getSize(width, height);

					// ダボぉバッファリングの準備
					HDC hCompatibleDC = ::CreateCompatibleDC(hDC);
					HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, width, height);
					HBITMAP hOldBitmap = HBITMAP(::SelectObject(hCompatibleDC, hBitmap));

					//背景
					RECT rect;
					rect.left   = 0;
					rect.top    = 0;
					rect.right  = width;
					rect.bottom = height;
					::FillRect( hCompatibleDC, &rect, HBRUSH(::GetStockObject(BLACK_BRUSH)) );
				
					_onPaint(hCompatibleDC);
					
					PAINTSTRUCT ps;
					hDC = ::BeginPaint( hWnd, &ps );
					::BitBlt(hDC, 0, 0, width, height, hCompatibleDC, 0, 0, SRCCOPY);
					::EndPaint( hWnd, &ps );
					
					::SelectObject(hCompatibleDC, hOldBitmap);
					::DeleteObject(hBitmap);
					::DeleteDC(hCompatibleDC);
				}
				break;
			}
			return ::DefWindowProc( hWnd, msg, wp, lp );
		}
		
		void _onCreate()
		{
	
			//DEBUG_LOG("TracerWindow._onCreate: called! hWnd="+ string_cast(_hWnd));
			HDC hDC = ::GetDC(_hWnd);
			
			#define _LOAD_IMAGE(DEST, RESOURCE_ID)                           \
				DEST.hCompatibleDC = ::CreateCompatibleDC(hDC);              \
				DEST.hBitmap = HBITMAP(                                      \
					::LoadBitmap(gHInstance, MAKEINTRESOURCE(RESOURCE_ID))); \
				::SelectObject(DEST.hCompatibleDC, DEST.hBitmap);            \
				::GetObject(DEST.hBitmap, sizeof(BITMAP), &DEST.bitmap);

			_LOAD_IMAGE(_arrowIcon.east , IDB_BITMAP1);
			_LOAD_IMAGE(_arrowIcon.west , IDB_BITMAP2);
			_LOAD_IMAGE(_arrowIcon.north, IDB_BITMAP3);
			_LOAD_IMAGE(_arrowIcon.south, IDB_BITMAP4);

			::ReleaseDC(_hWnd, hDC);

			#undef _LOAD_IMAGE
		}
		
		void _onPaint(HDC hDC)
		{

			//DEBUG_LOG("TracerWindow._onPaint: called! step="+ string_cast(_animationState.step));
			
			size_t width;
			size_t height;
			getSize(width, height);
			
			const int y = (height - ARROW_ICON_HEIGHT) / 2;
			
			//#define _PAINT_ARROWS(X_OFFSET)                                      \
			//	do {                                                               \
			//		int x = (width - _displayIcons.size() * ARROW_ICON_WIDTH) / 2; \
			
			#define _PAINT_ARROWS(X_OFFSET)                                        \
				do {                                                               \
					int x = 0;                                                     \
					x += (X_OFFSET);                                               \
					for(display_icons_t::const_iterator i = _displayIcons.begin()  \
						; i != _displayIcons.end(); ++i)                           \
					{                                                              \
						::BitBlt(hDC, x, y, ARROW_ICON_WIDTH, ARROW_ICON_HEIGHT,   \
							(*i)->hCompatibleDC, 0, 0, SRCCOPY);                   \
						x += ARROW_ICON_WIDTH;                                     \
					}                                                              \
				} while(false)

			switch(_animationState.step) {
			case 0: {
					_animationState.fade += 70;
					_PAINT_ARROWS(0);		

					//int x = (width + _displayIcons.size() * ARROW_ICON_WIDTH) / 2;
					int x = _displayIcons.size() * ARROW_ICON_WIDTH;
					
					BLENDFUNCTION blendFunc;
					blendFunc.BlendOp             = AC_SRC_OVER;
					blendFunc.BlendFlags          = 0;
					blendFunc.SourceConstantAlpha = _animationState.fade;
					blendFunc.AlphaFormat         = 0;
					
					::AlphaBlend(hDC, x, y, ARROW_ICON_WIDTH, ARROW_ICON_HEIGHT,
						_animationState.icon->hCompatibleDC, 0, 0, ARROW_ICON_WIDTH, ARROW_ICON_HEIGHT, blendFunc);
				}
				if(_animationState.fade + 133 > 255) {
					_displayIcons.push_back(_animationState.icon);
					//++_animationState.step;
					_animationState.step += 2;
				}
				break;

			case 1: {
					double slide = 9 - _animationState.slide++;
					_PAINT_ARROWS(
						int(ARROW_ICON_WIDTH / 81.0 * slide * slide / 2.0));

					if(slide == 0) {
						++_animationState.step;
					}
				}
				break;
			
			case 2:
				_PAINT_ARROWS(0);
				if(_animationQueue.empty()) {
					if(_timerId) {
						::KillTimer(_hWnd, _timerId);
						_timerId = 0;
					}
					return;
				}
				_animationState = animation_state_t(_arrowIcon.list[ _animationQueue.front() ]);
				_animationQueue.pop_front();
				//DEBUG_LOG("TracerWindow._onPaint: start animation!");
				break;
			}

			#undef _PAINT_ARROWS
		}

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			TracerWindow *self;
			if(msg == WM_CREATE) {
				self = static_cast<TracerWindow*>(reinterpret_cast<CREATESTRUCT*>(lp)->lpCreateParams);
				self->_hWnd = hWnd;
				::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
			}
			else {
				self = reinterpret_cast<TracerWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
			}
			return self->onMessage(hWnd, msg, wp, lp);
		}


		HWND _hWnd;
		UINT_PTR _timerId;
		
		// 矢印のアイコンね
		struct arrow_icon_t
		{
			HDC hCompatibleDC;
			HBITMAP hBitmap;
			BITMAP bitmap;
		};
		
		union {
			arrow_icon_t list[4];
			struct {
				arrow_icon_t east;
				arrow_icon_t west;
				arrow_icon_t north;
				arrow_icon_t south;
			};
		} _arrowIcon;
		
		struct animation_state_t
		{

			int step;
			arrow_icon_t *icon;
			int fade;
			double slide;

			animation_state_t(arrow_icon_t &arrow)
			:step(0)
			,icon(&arrow)
			,fade(0)
			,slide(0)
			{}

			animation_state_t()
			:step(2)
			{}
		};
		animation_state_t _animationState;
		
		typedef std::deque<int> animation_queue_t;
		animation_queue_t _animationQueue;
		
		typedef std::deque<arrow_icon_t*> display_icons_t;
		display_icons_t _displayIcons;
	};


	// ntghk と通信する人
	class DaemonWindow
	{
	public:

		DaemonWindow(TracerWindow &tracerWindow)
		:_hWnd(0)
		,_tracerWindow(tracerWindow)
		{

			// プロセス間通信用の空のウィンドウを作る
			_hWnd = ::CreateWindowEx(0, reinterpret_cast<LPCSTR>(getWindowClass()),
				"", 0, 0, 0, 0, 0, NULL, NULL, ::GetModuleHandle(NULL), NULL);
			
			::SetWindowLongPtr(_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		}
		
		~DaemonWindow()
		{
			if(_hWnd) {
				::DestroyWindow(_hWnd);
			}
		}
		
		HWND getHandle() const
		{
			return _hWnd;
		}

	private:

		LRESULT onMessage(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			
			//DEBUG_LOG("DaemonWindow.onMessage: called!");
			switch( msg ) {
			case WM_DESTROY:
				::PostQuitMessage(0);
				return 0;
				
			case ntg::WM_MGESTUREBEGIN:
				_tracerWindow.show(HWND(wp));
				_tracerWindow.addArrow(int(lp));
				ntg::fireMouseGestureBegin();
				return 0;
				
			case ntg::WM_MGESTUREEND:
				_tracerWindow.hide();
				ntg::fireMouseGestureEnd(uint32_t(wp), HWND(lp));
				return 0;
				
			case ntg::WM_MGESTUREPROGRESS:
				_tracerWindow.addArrow(int(wp));
				ntg::fireMouseGestureProgress();
				return 0;
				
			case ntg::WM_RGESTUREEND:
				ntg::fireRockerGestureEnd(wp == 0, HWND(lp));
				return 0;
				
			case ntg::WM_WGESTUREEND:
				ntg::fireWheelGestureEnd(wp == 0, HWND(lp));
				return 0;
			}
			return ::DefWindowProc( hWnd, msg, wp, lp );
		}

		static int getWindowClass()
		{
			static int classAtom = 0;
			if(!classAtom) {

				WNDCLASSEX wc;

				wc.cbSize        = sizeof(WNDCLASSEX);
				wc.style         = 0;
				wc.lpfnWndProc   = WndProc;
				wc.cbClsExtra    = 0;
				wc.cbWndExtra    = 0;
				wc.hInstance     = ::GetModuleHandle(NULL);
				wc.hIcon         = NULL;
				wc.hCursor       = NULL;
				wc.hbrBackground = NULL;
				wc.lpszMenuName  = NULL;
				wc.lpszClassName = "ntg::DaemonWindow";
				wc.hIconSm       = NULL;

				if((classAtom = ::RegisterClassEx( &wc )) == 0) {
					DEBUG_LOG("Failed to RegisterClassEx: "+ getLastErrorMessage());
				}
			}
			return classAtom;
		}

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			DaemonWindow *self = reinterpret_cast<DaemonWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
			if(msg == WM_CREATE) {
				return ::DefWindowProc(hWnd, msg, wp, lp);
			}
			return self->onMessage(hWnd, msg, wp, lp);
		}

		HWND _hWnd;
		TracerWindow &_tracerWindow;
	};
	
	unsigned __stdcall ThreadProc(void *param)
	{
		
		TracerWindow tracerWindow;
		DaemonWindow daemonWindow(tracerWindow);
		
		// ntghk に通知
		ntghk_setDaemonHandle(daemonWindow.getHandle());
		
		MSG msg;
		while(::GetMessage( &msg, NULL, 0, 0 ) > 0) {
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}
		_endthread();
		return 0;
	}
}

namespace ntg
{

	static bool _hookInstalled = false;

	void execDaemon()
	{
		if(!_hookInstalled) {
			_hookInstalled = true;
			ntghk_setProcessName("chrome.exe", 10);
			ntghk_installHook();
			
			_beginthreadex(NULL, 0,
				reinterpret_cast<unsigned (__stdcall*)(void*)>(&ThreadProc), 0, 0, NULL);
		}
	}

	void stopDaemon()
	{
		if(_hookInstalled) {
			ntghk_uninstallHook();
			_hookInstalled = false;
			
			// TODO:ここでスレッドにjoin
		}
	}

}

