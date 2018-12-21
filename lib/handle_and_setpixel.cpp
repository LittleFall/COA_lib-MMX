/* 可参考句柄内容 */
#include <Windows.h> 

#define TITLE_NAME "尝试"
#define WIN_WIDTH   800
#define WIN_HEIGHT  400

// 窗口过程函数原型  
//      HWND hwnd,              handle to window  
//      UINT uMsg,              message identifier  
//      WPARAM wParam,          first message parameter  
//      LPARAM lParam           second message parameter  
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 我的绘图任务函数
//      hdc                     传入的hdc参数
void MY_PAINT_MISSION(HDC hdc);

// 窗口主函数，相当于 DOS 下的main函数  
//      HINSTANCE hInstance,        handle to current instance  
//      HINSTANCE hPrevInstance,    handle to previous instance  
//      LPSTR lpCmdLine,            pointer to command line  
//      int nCmdShow                show state of window  
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    HWND hWnd;
    hWnd = CreateWindow(TITLE_NAME, TITLE_NAME, WS_OVERLAPPEDWINDOW, 0, 0, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    MSG msg;

    // hdc init
    HDC hdc = GetDC(hWnd);

    // 绘图
    MY_PAINT_MISSION(hdc);

    // 消息循环  
    while (GetMessage(&msg, 0, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // release
    ReleaseDC(hWnd, hdc);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
        break;
    }
    return 0;
}

// 此处为绘图任务内容
void MY_PAINT_MISSION(HDC hdc) {
    for(int i=1;i<=200;++i)
        for(int j=1;j<=200;++j)
            SetPixel(hdc, i, j, RGB(1, 1, 1));

    SetPixel(hdc, 400, 400, RGB(1, 1, 1));
    SetPixel(hdc, 200, 200, RGB(255, 0, 0));
}