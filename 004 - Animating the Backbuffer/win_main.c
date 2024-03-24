
#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint32_t uint32;


global_variable int Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void
Win32RenderWeirdGradient(int XOffset, int YOffset) {
  int Width = BitmapWidth;
  int Height = BitmapHeight;
  int Pitch = Width*BytesPerPixel;
  uint8* Row = (uint8*)BitmapMemory;
  for(int Y = 0;
      Y < BitmapHeight;
      ++Y) {
    uint8* Pixel = (uint8*) Row;
    for (int X = 0;
         X < BitmapWidth;
         ++X) {
      *Pixel = (uint8)(X + XOffset);
      ++Pixel;
      
      *Pixel = (uint8)(Y + YOffset);
      ++Pixel;
      
      *Pixel = 0;
      ++Pixel;
      
      *Pixel = 0;
      ++Pixel;
    }
    Row += Pitch;
  }  
}

internal void 
Win32ResizeDIBSection(int Width, int Height) {

  //TODO: bulletproof this
  //maybe dont free first

  if ( BitmapMemory ) {
    VirtualFree(BitmapMemory,0,MEM_RELEASE);
  } 

  BitmapWidth = Width;
  BitmapHeight = Height;

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = BitmapWidth;
  BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  int BitmapMemorySize = (Width * Height) * BytesPerPixel;
  BitmapMemory = VirtualAlloc (0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT* ClientRect, int X,int Y,int Width,int Height) {

  int WindowWidth = ClientRect->right - ClientRect->left;
  int WindowHeight = ClientRect->bottom - ClientRect->top;
  
  StretchDIBits(
    DeviceContext,
    0,0,BitmapWidth,BitmapHeight,
    0,0,WindowWidth,WindowHeight,
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

      RECT ClientRect;
      GetClientRect(Window,&ClientRect);
                  
      Win32UpdateWindow(DeviceContext,&ClientRect,X,Y,Width,Height);
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
    HWND Window = CreateWindowEx(
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

    if ( Window ) { 
      MSG Message;
      Running = 1;
      int XOffset = 0;
      int YOffset = 0;
      while ( Running ) {

        while(PeekMessage( &Message, 0, 0,0,PM_REMOVE )){
          if(Message.message == WM_QUIT) {
            Running = 0;
          }

          TranslateMessage(&Message);
          DispatchMessageA(&Message);
        }

        Win32RenderWeirdGradient(XOffset,YOffset);
        HDC DeviceContext  = GetDC(Window);
        RECT ClientRect;
        GetClientRect(Window,&ClientRect);
        int WindowWidth = ClientRect.right - ClientRect.left;
        int WindowHeight = ClientRect.bottom - ClientRect.top;
        Win32UpdateWindow(DeviceContext,&ClientRect,0,0,WindowWidth,WindowHeight);
        ++XOffset;
      }
      
    } else {
      //TODO: logging
    }
  } else {
    //TODO: Logging
  }

  
    
  return(0);  }

