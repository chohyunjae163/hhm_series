
#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint32_t uint32;


typedef struct {
  BITMAPINFO Info;
  void* Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
} win32_offscreen_buffer;

global_variable int Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

internal void
Win32RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset) {
  int Width = Buffer.Width;
  int Height = Buffer.Height;
  
  uint8* Row = (uint8*)Buffer.Memory;
  for(int Y = 0;
      Y < Height;
      ++Y) {
    uint32* Pixel = (uint32*) Row;
    for (int X = 0;
         X < Width;
         ++X) {

      uint8 Blue  = (X + XOffset);
      uint8 Green = (Y + YOffset);
      *Pixel++ = ((Green << 8) | Blue);
    }
    Row += Buffer.Pitch;
  }  
}

internal void 
Win32ResizeDIBSection(win32_offscreen_buffer* Buffer, int Width, int Height) {

  //TODO: bulletproof this
  //maybe dont free first

  if ( Buffer->Memory ) {
    VirtualFree(Buffer->Memory,0,MEM_RELEASE);
  } 

  Buffer->Width = Width;
  Buffer->Height = Height;

  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;

  int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
  Buffer->Memory = VirtualAlloc (0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);


  Buffer->Pitch = Width*Buffer->BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, RECT ClientRect, 
  win32_offscreen_buffer Buffer,
  int X,int Y,int Width,int Height) {

  int WindowWidth = ClientRect.right - ClientRect.left;
  int WindowHeight = ClientRect.bottom - ClientRect.top;
  
  StretchDIBits(
    DeviceContext,
    0,0,Buffer.Width,Buffer.Height,
    0,0,WindowWidth,WindowHeight,
    Buffer.Memory,
    &Buffer.Info,
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
      Win32ResizeDIBSection(&GlobalBackBuffer,Width,Height);
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
                  
      Win32DisplayBufferInWindow(DeviceContext,ClientRect,GlobalBackBuffer,X,Y,Width,Height);
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
  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = Win32MainWindowCallback;
  WindowClass.hInstance = Instance;
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
      Running = 1;
      int XOffset = 0;
      int YOffset = 0;
      while ( Running ) {
        MSG Message;
        while(PeekMessage( &Message, 0, 0,0,PM_REMOVE )){
          if(Message.message == WM_QUIT) {
            Running = 0;
          }

          TranslateMessage(&Message);
          DispatchMessageA(&Message);
        }

        Win32RenderWeirdGradient(GlobalBackBuffer,XOffset,YOffset);
        HDC DeviceContext  = GetDC(Window);
        RECT ClientRect;
        GetClientRect(Window,&ClientRect);
        int WindowWidth = ClientRect.right - ClientRect.left;
        int WindowHeight = ClientRect.bottom - ClientRect.top;
        Win32DisplayBufferInWindow(DeviceContext,ClientRect,GlobalBackBuffer,0,0,WindowWidth,WindowHeight);
        ++XOffset;
      }
      
    } else {
      //TODO: logging
    }
  } else {
    //TODO: Logging
  }

  
    
  return(0);  }

