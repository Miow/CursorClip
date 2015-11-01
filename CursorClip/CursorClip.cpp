// CursorClip.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CursorClip.h"

#define APPLICATION_NAME L"CursorClip"

#define MAX_LOADSTRING 100
#define MAX_CLIPPED_WINDOW_NAME_SIZE 255

#define ID_TRAY_APP_ICON 6415
#define WM_TRAYICON ( WM_USER + 1 )
#define TRAYCONTEXTMENU_EXIT ( WM_USER + 2 )

#define HOTKEY_KEYID VK_F11
#define HOTKEY_GUID 100




// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

NOTIFYICONDATA notificationIconData = {};		// Systray icon
HMENU systrayMenu;								// Systray contextual menu


bool isClippingEnabled = false;
wchar_t clippedWindowName[MAX_CLIPPED_WINDOW_NAME_SIZE];
wchar_t selectedWindowNameBuffer[MAX_CLIPPED_WINDOW_NAME_SIZE];



// Clip the cursor to the window
VOID clipCursorToWindow(HWND hWnd) {
	// Getting the rectagle from the window
	RECT clippingRECT;
	GetWindowRect(hWnd, &clippingRECT);

	// Locking the cursor in the recangle
	ClipCursor(&clippingRECT);

	// Activating the window
	SetActiveWindow(hWnd);


	//OutputDebugString(L"\nCursor clipped");
}


VOID CALLBACK WinEventProcCallback(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	/*
	TCHAR windowName[NAMEBUFFERSIZE];
	GetWindowText(hWnd, windowName, NAMEBUFFERSIZE);
	_wcsicmp(windowArray[i].name, windowName);
	*/
	if (isClippingEnabled) {
		GetWindowText(hWnd, selectedWindowNameBuffer, MAX_CLIPPED_WINDOW_NAME_SIZE);

		if (!wcsncmp(clippedWindowName, selectedWindowNameBuffer, MAX_CLIPPED_WINDOW_NAME_SIZE)) {
			clipCursorToWindow(hWnd);
		}
		
	}
}


VOID toggleCursorClipping() {
	if (isClippingEnabled) {
		// Freeing the cursor
		ClipCursor(NULL);

		// Removing the systray tooltip
		notificationIconData.uFlags &= !NIF_TIP;

	}
	else {
		HWND hWnd = GetForegroundWindow();

		if (hWnd != NULL) {
			GetWindowText(hWnd, clippedWindowName, MAX_CLIPPED_WINDOW_NAME_SIZE);
			
			// Changing the systray tooltip
			_snwprintf_s(
				notificationIconData.szTip,
				sizeof(notificationIconData.szTip),
				L"Cursor clipped to: %s",
				clippedWindowName);
			notificationIconData.uFlags |= NIF_TIP;


			clipCursorToWindow(hWnd);
		}

	}


	Shell_NotifyIcon(NIM_MODIFY, &notificationIconData);
	isClippingEnabled = !isClippingEnabled;
}







// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
VOID				cleanAndExit();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
VOID				initNotificationIcon(HWND);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);



	// Registering for the event that tells us when a window comes to the foreground
	SetWinEventHook(EVENT_SYSTEM_FOREGROUND,
		EVENT_SYSTEM_FOREGROUND, NULL,
		WinEventProcCallback, 0, 0,
		WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);




    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CURSORCLIP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CURSORCLIP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS mainWNDC = {};

	mainWNDC.lpfnWndProc = WndProc;
	mainWNDC.hInstance = hInstance;
	mainWNDC.lpszClassName = szWindowClass;

	return RegisterClass(&mainWNDC);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, 0,
      0, 0, 0, 0, 0, 0, hInstance, 0);

   if (!hWnd)
   {
      return FALSE;
   }

   // Registering hotkeys
   if (!RegisterHotKey(hWnd, HOTKEY_GUID, MOD_SHIFT, HOTKEY_KEYID))
   {
	   OutputDebugString(L"\nFailed to register hotkey.");
   }

   initNotificationIcon(hWnd);

   return TRUE;
}


VOID cleanAndExit()
{
	Shell_NotifyIcon(NIM_DELETE, &notificationIconData);
	PostQuitMessage(0);
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
   /* case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;*/
	case WM_CREATE :

		// Creating the contextual menu
		systrayMenu = CreatePopupMenu();
		//AppendMenu(systrayMenu, MF_SEPARATOR, TRAYCONTEXTMENU_SEPARATOR, NULL);
		AppendMenu(systrayMenu, MF_STRING, TRAYCONTEXTMENU_EXIT, TEXT("Exit"));
	case WM_TRAYICON:
		if (lParam == WM_RBUTTONUP)
		{
			// Show the contextual menu
			// Getting the cursor position
			POINT curPoint;
			GetCursorPos(&curPoint);
			//SetForegroundWindow(hWnd); // Bugfix to properly hide the contextual menu

			UINT clicked = TrackPopupMenu(
				systrayMenu,
				TPM_RETURNCMD	// Does not send notification when a menu item is clicked
				| TPM_NONOTIFY, // The function returns the menu item identifier of the user's selection in the return value.
				curPoint.x,
				curPoint.y,
				0,
				hWnd,
				NULL
				);

			PostMessage(hWnd, WM_NULL, 0, 0); // Bugfix to properly hide the contextual menu


			switch (clicked)
			{
			case TRAYCONTEXTMENU_EXIT:
				cleanAndExit();
				break;

			}
		}
	case WM_HOTKEY:
		if (wParam == HOTKEY_GUID)
		{
			toggleCursorClipping();
		}
		break;
	case WM_DESTROY:
		cleanAndExit();
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}




VOID initNotificationIcon(HWND hWnd) {
	
	// Initialyze the struct for the systray icon
	notificationIconData.cbSize = sizeof(NOTIFYICONDATA);	// Size of the struct
	notificationIconData.hWnd = hWnd;						// Handle to the window
	notificationIconData.uID = ID_TRAY_APP_ICON;			// ID of the notify objects
	notificationIconData.uFlags = 
		  NIF_MESSAGE				// Send a message to the callback function
		| NIF_ICON;					// Promise a valid icon
	notificationIconData.uCallbackMessage = WM_TRAYICON;		// Message to be sent to the callback
	notificationIconData.uVersion = NOTIFYICON_VERSION_4;
	notificationIconData.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_WINLOGO));


	Shell_NotifyIcon(NIM_ADD, &notificationIconData);
}

