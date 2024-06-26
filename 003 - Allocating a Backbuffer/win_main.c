
#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static


global_variable int Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void 
Win32ResizeDIBSection(int Width, int Height) {

  //TODO: bulletproof this
  //maybe dont free first

  if ( BitmapHandle ) {
    DeleteObject(BitmapHandle);
  } 
  if ( !BitmapDeviceContext ) {
    BitmapDeviceContext = CreateCompatibleDC(0);
  }

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = Height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;
  
  BitmapHandle = CreateDIBSection(
    BitmapDeviceContext,
    &BitmapInfo,
    DIB_RGB_COLORS,
    &BitmapMemory,
    0,0);

}

internal void
Win32UpdateWindow(HDC DeviceContext,int X,int Y,int Width,int Height) {
  StretchDIBits(
    DeviceContext,
    X,Y,Width,Height, //dest
    X,Y,Width,Height, //src
    BitmapMemory,
    &BitmapInfo,
    DIB_RGB_COLORS,
    SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(
  HWND Window, // a handle to the window
  UINT Message, // the message
  WPARAM WParam, 
  LPARAM LParam) {

  LRESULT Result = 0;

  switch(Message) {
    case WM_SIZE:{
      RECT ClientRect;
      GetClientRect(Window,&ClientRect);
      int Width = ClientRect.right - ClientRect.left;
      int Height = ClientRect.bottom - ClientRect.top;
      Win32ResizeDIBSection(Width,Height);
      OutputDebugStringA("WM_SIZE\n");
    } break;
    case WM_DESTROY:{
      Running = 0;
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    case WM_CLOSE:{
      Running = 0;
      OutputDebugStringA("WM_CLOSE\n");
    } break;
    case WM_ACTIVATEAPP: {
      OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT: {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(Window,&Paint);
      int X = Paint.rcPaint.left;
      int Y = Paint.rcPaint.top;
      int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
      int Width  = Paint.rcPaint.right - Paint.rcPaint.left;
      Win32UpdateWindow(DeviceContext,X,Y,Width,Height);
      PatBlt(DeviceContext,X,Y,Width,Height,WHITENESS);
      EndPaint(Window, &Paint);
    }break;
    default: {
      OutputDebugStringA("default\n");
      Result = DefWindowProc(Window,Message,WParam,LParam);
    }break;
  }  

  return(Result);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode) {

  WNDCLASS WindowClass =  { 0 };
  WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = Win32MainWindowCallback;
  WindowClass.hInstance = Instance;
  //WindowClass.hIcon =;
  //LPCSTR lpszMenuName;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";

  //@see https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassa
  //register class returns "atom"..whatever that is.. i will just ignore it :)
  if( RegisterClassA(&WindowClass) ) {
    HWND WindowHandle = CreateWindowEx(
      0,
      WindowClass.lpszClassName,
      "Handmade Hero",
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      0,
      0,
      Instance,
      0 );

    if ( WindowHandle ) { 
      MSG Message;
      Running = 1;
      while ( Running ) {
        BOOL MessageResult = GetMessageA( &Message, 0, 0,0 );
        if ( MessageResult > 0 ) {
          TranslateMessage(&Message);
          DispatchMessageA(&Message);
          
        } else {
          break;
        }
      }
      
    } else {
      //TODO: logging
    }
  } else {
    //TODO: Logging
  }

  
    
  return(0);  }

