#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include "./resource.h"

char capsPressed = 0;

HWND windowHandle;

unsigned int xMousePosition = 0;
unsigned int yMousePosition = 0;

UINT customMsg;

char lMouseButtonPressed = 0;

HFONT mainFont = {0};
HFONT smallFont = {0};

LRESULT CALLBACK WindowProc(HWND windowHandle, UINT windowMessage, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam);

INT_PTR Dlgproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); 

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR pCmdline, int nCmdShow)
{

    WNDCLASS windowClass = {0};

    windowClass.lpszClassName = "Key Detect Window";
    windowClass.hInstance = hInstance;
    windowClass.lpfnWndProc = WindowProc; 

    HICON appIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 0,0,
                                    LR_DEFAULTCOLOR |LR_DEFAULTSIZE);

    if(appIcon == NULL)
    {
        printf("Unable to load app icon\n");

        return 0;
    }

    windowClass.hIcon = appIcon; 

    if(windowClass.hIcon == NULL)
    {
        printf("Unable to load icon!\n");

        //return 0;
    }

    HBRUSH windowBackgroundBrush = CreateSolidBrush(RGB(255,255,255));

    windowClass.hbrBackground = windowBackgroundBrush;

    if(RegisterClass(&windowClass) == 0) 
    {
        printf("unable to register window class\n");
        return 0;
    }

    DeleteBrush(windowBackgroundBrush);

    HMONITOR monitorHandle = MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST) ;

    if(monitorHandle == NULL) 
    {
        printf("Error getting monitor handle!\n");

        return 0;
    }

    MONITORINFO monitorInfo = {0};

    monitorInfo.cbSize = sizeof(MONITORINFO);

    if(GetMonitorInfo(monitorHandle,&monitorInfo) == 0) 
    {
        printf("Error!\n"); 
    }

    windowHandle = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, 
            "Key Detect Window", 
            "Key Detect", 
            WS_POPUP | WS_SYSMENU,
            monitorInfo.rcWork.right * 0.5f - (128 * 0.5), 
            monitorInfo.rcWork.bottom * 0.5f - (128 * 0.5), 
            128, 128, 
            NULL, NULL,hInstance, NULL);

    if(windowHandle == NULL) 
    {
        printf("Window Handle is null!\n");
        return 0;
    }

    SetLayeredWindowAttributes(windowHandle, 0, 128, LWA_ALPHA);

    ShowWindow(windowHandle, nCmdShow);

    //Hook to check if key is pressed even when if this app is not focused...
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance,0);

    if(hook == NULL)
    {
        printf("Hook is null!\n");
        return 0;
    }
    
    //Font Transparency...
    SetWindowRgn(windowHandle, (HRGN)SIMPLEREGION, TRUE);

    mainFont = CreateFont(48, 0, 0 ,0, FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial");

    if(mainFont == NULL)
    {
        printf("Font is null!\n");

        return 0;
    }

    customMsg = RegisterWindowMessage("closeWindow");

    MSG msg = {0};

    while(GetMessage(&msg, NULL, 0,0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    printf("Shutting down app!\n");
    
    return 0;
}

LRESULT CALLBACK WindowProc(HWND windowHandle, UINT windowMessage, WPARAM wParam, LPARAM lParam)
{
    if(windowMessage == customMsg)
    {
        PostQuitMessage(0);
    }
    switch(windowMessage)
    {
        case WM_CREATE:
            {
                DialogBox(NULL, MAKEINTRESOURCE(IDI_STARTDIALOG), windowHandle, Dlgproc);
                return 0;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_KEYDOWN:
            {
                if(wParam == 'X')
                {
                    SHORT x = GetAsyncKeyState(VK_CONTROL);
                    if((x >> 15 & 1) == 1)
                    {
                        PostQuitMessage(0);
                    }
                }

                if(wParam == VK_ESCAPE)
                {
                    PostQuitMessage(0);
                }
            }

            break;

        case WM_LBUTTONDOWN:
            if(wParam == MK_LBUTTON)
            {
                lMouseButtonPressed = 1;
                xMousePosition = GET_X_LPARAM(lParam);
                yMousePosition = GET_Y_LPARAM(lParam);
            }
            break;

        case WM_LBUTTONUP:
            lMouseButtonPressed = 0;
            break;


        case WM_PAINT:
            {
                PAINTSTRUCT ps = {0};

                HDC dc = BeginPaint(windowHandle, &ps);

                RECT windowRect;

                GetWindowRect(windowHandle, &windowRect);

                RECT r = {0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top};

                HBRUSH blueImage = CreateSolidBrush(RGB(0 , 0, 25));
                HBRUSH greenImage = CreateSolidBrush(RGB(0 , 128, 0));

                SetBkMode(dc, TRANSPARENT); 

                if(capsPressed == 0)
                {
                    FillRect(dc, &r , blueImage);
                    SetTextColor(dc, RGB(120, 120, 120));
                    SelectObject(dc, mainFont);
                    DrawText(dc, "off", -1,&r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
                }
                else 
                {
                    FillRect(dc, &r , greenImage);
                    SetTextColor(dc, RGB(0, 255, 0));
                    SelectObject(dc, mainFont);
                    DrawText(dc, "on", -1 ,&r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
                }

                DeleteBrush(blueImage);

                DeleteBrush(greenImage);

                EndPaint(windowHandle, &ps);
            }
            break;

        case WM_MOUSEMOVE:
            {
                if(lMouseButtonPressed)
                {
                    POINT screenPoint; 

                    GetCursorPos(&screenPoint);

                    RECT r = {0};           

                    GetWindowRect(windowHandle, &r);

                    SetWindowPos(windowHandle, HWND_TOPMOST, screenPoint.x, screenPoint.y,100, 
                            100, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
                }
            }
            break;

        case WM_NCHITTEST:
            return HTCAPTION;

    }
     
    return DefWindowProc(windowHandle, windowMessage, wParam, lParam);
}

LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
    if(code == HC_ACTION)
    {
        if(wParam == WM_KEYDOWN)
        {
            KBDLLHOOKSTRUCT keyPressedHookStruct = {0}; 

            //Get the key pressed info...
            keyPressedHookStruct = *(KBDLLHOOKSTRUCT *)lParam;

            //if caps was pressed...
            if(keyPressedHookStruct.vkCode == VK_CAPITAL)
            {
                //Test lowest bit to check if on or off...
                if((GetKeyState(keyPressedHookStruct.vkCode) << 0 & 1) == 0)
                {
                    capsPressed = 1;
                }
                else 
                {
                    capsPressed = 0;
                }

                InvalidateRect(windowHandle, NULL, TRUE);
            }
        }
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}

INT_PTR Dlgproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
            
            smallFont = CreateFont(15, 0, 0 ,0, FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Helvetica");

            if(smallFont == NULL)
            {
                printf("Font is null!\n");

                return 0;
            }
            printf("Dialog init!\n");
            break;

        case WM_PAINT:
            PAINTSTRUCT ps;

            HDC dc = BeginPaint(hwnd, &ps);

            SetBkMode(dc,TRANSPARENT);

            SetTextColor(dc,RGB(200,200,200));

            RECT windowRect; 
            GetClientRect(hwnd, &windowRect);

            HBRUSH bgColor = CreateSolidBrush(RGB(34, 139, 161));

            FillRect(dc, &windowRect, bgColor);

            RECT r = {0, -30, windowRect.right - windowRect.left, 
                windowRect.bottom - windowRect.top};

            SelectObject(dc, smallFont);
            DrawText(dc, "CapsDetect", -1, &r, DT_CENTER 
                    | DT_VCENTER | DT_SINGLELINE);
            r.top = 0;
            DrawText(dc, "by Zain", -1, &r, DT_CENTER 
                    | DT_VCENTER | DT_SINGLELINE);

            r.top = 50;
            DrawText(dc, "'A' Start", -1, &r, DT_CENTER 
                    | DT_VCENTER | DT_SINGLELINE);

            r.top = 90;

            DrawText(dc, "'CTRL-X' Close", -1, &r, DT_CENTER 
                    | DT_VCENTER | DT_SINGLELINE);

            HDC memDC = CreateCompatibleDC(dc);

            HBITMAP icon = (HBITMAP)LoadImage(GetModuleHandle(NULL), 
                            MAKEINTRESOURCE(IDI_ICON), 
                            IMAGE_BITMAP, 
                            0 , 0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR);

            if(icon == NULL) printf("Icon is null!\n");

            HBITMAP memBitmap = (HBITMAP)SelectObject(memDC, icon);

            StretchBlt(dc, 48, 6, 32, 32, 
                    memDC, 0, 0, 64, 64, SRCCOPY);

            SelectObject(memDC, memBitmap);

            DeleteDC(memDC);

            DeleteBrush(bgColor);

            DeleteObject(icon);

            EndPaint(hwnd, &ps);

            break;

        case WM_KEYDOWN:
            if(wParam == 'A')
            {
                EndDialog(hwnd, 0);
            }

            if(wParam == 'X')
            {
                SHORT ctrlKey = GetAsyncKeyState(VK_CONTROL);

                if((ctrlKey >> 15 & 1) == 1)
                {
                    SendMessage(HWND_BROADCAST, customMsg, wParam, lParam);
                }
            }

            //Pressing esc polls this event in Dialog box...
        case WM_COMMAND:
            if(wParam == IDCANCEL)
            {
                SendMessage(HWND_BROADCAST, customMsg, wParam, lParam);
            }
            break;
    }
    return 0;
}
