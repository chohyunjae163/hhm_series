
#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint32_t uint32;
typedef int16_t int16;


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


typedef struct {
  int Width;
  int Height;
} win32_window_dimension;


#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
  return (0);
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex,XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
  return (0);
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void
Win32LoadXInput(void) {
  HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");

  if(XInputLibrary){
    XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary,"XInputGetState");
    XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary,"XInputSetState");
  }
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window) {
  win32_window_dimension Result;

  RECT ClientRect;
  GetClientRect(Window,&ClientRect);
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;

  return (Result);
}

internal void
Win32RenderWeirdGradient(win32_offscreen_buffer* Buffer, int XOffset, int YOffset) {
  int Width = Buffer->Width;
  int Height = Buffer->Height;
  
  uint8* Row = (uint8*)Buffer->Memory;
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
    Row += Buffer->Pitch;
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
  Buffer->BytesPerPixel = 4;

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
Win32DisplayBufferInWindow(win32_offscreen_buffer* Buffer,
  HDC DeviceContext, int WindowWidth,int WindowHeight, 
  int X,int Y,int Width,int Height) {

  //TODO : Aspect ratio correction
  StretchDIBits(
    DeviceContext,
    0,0,WindowWidth,WindowHeight,
    0,0,Buffer->Width,Buffer->Height,
    Buffer->Memory,
    &Buffer->Info,
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
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP: {
      uint32 VKCode = WParam;
      BOOL WasDown = ((LParam & (1 << 30)) != 0);
      BOOL IsDown = ((LParam & (1 << 31)) == 0);
      if(WasDown != IsDown) {
        if(VKCode == 'W'){
  
        } else if(VKCode == 'A'){
  
        } else if(VKCode == 'S'){
  
        } else if(VKCode == 'D'){
  
        } else if(VKCode == 'Q'){
  
        } else if(VKCode == 'E'){
  
        } else if(VKCode == VK_UP){
  
        } else if(VKCode == VK_LEFT){
  
        } else if(VKCode == VK_DOWN){
  
        } else if(VKCode == VK_RIGHT){
  
        } else if(VKCode == VK_ESCAPE){
  
        } else if(VKCode == VK_SPACE){
          if(IsDown){
            OutputDebugStringA("IsDown\n ");
          } else if (WasDown){
            OutputDebugStringA("WasDown\n");
          }
        }
      }
 
    }break;
    case WM_PAINT: {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(Window,&Paint);
      int X = Paint.rcPaint.left;
      int Y = Paint.rcPaint.top;
      int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
      int Width  = Paint.rcPaint.right - Paint.rcPaint.left;

      win32_window_dimension Dimension = Win32GetWindowDimension(Window);
      Win32DisplayBufferInWindow(&GlobalBackBuffer,DeviceContext,
        Dimension.Width,Dimension.Height,
        X,Y,Width,Height);

      EndPaint(Window, &Paint);
    }break;
    default: {
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
  Win32LoadXInput();
  WNDCLASS WindowClass =  { 0 };

  Win32ResizeDIBSection(&GlobalBackBuffer,1280,720);

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

        //note : deal with inputs
        //TODO: should we poll this more frequently?
        for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ ControllerIndex){
          XINPUT_STATE ControllerState;
          if( XInputGetState( ControllerIndex, &ControllerState ) == ERROR_SUCCESS){
            //note : the controller is plugged in
            XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;
            BOOL Up = ( Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP );
            BOOL Down = ( Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN );
            BOOL Left = ( Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT );
            BOOL Right = ( Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT );
            BOOL Start = ( Pad->wButtons & XINPUT_GAMEPAD_START );
            BOOL Back = ( Pad->wButtons & XINPUT_GAMEPAD_BACK );
            BOOL LeftShoulder = ( Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER );
            BOOL RightShoulder = ( Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER );
            BOOL AButton = ( Pad->wButtons & XINPUT_GAMEPAD_A );
            BOOL BButton = ( Pad->wButtons & XINPUT_GAMEPAD_B );
            BOOL XButton = ( Pad->wButtons & XINPUT_GAMEPAD_X );
            BOOL YButton = ( Pad->wButtons & XINPUT_GAMEPAD_Y );

            int16 StickX = Pad->sThumbLX;
            int16 StickY = Pad->sThumbLY;

            if(AButton) {
              YOffset += 2;
            }

          } else {
            //note : the controller is not available
          }
        }

        Win32RenderWeirdGradient(&GlobalBackBuffer,XOffset,YOffset);
        HDC DeviceContext = GetDC(Window);
        
        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        Win32DisplayBufferInWindow(&GlobalBackBuffer,DeviceContext,
          Dimension.Width,Dimension.Height,
          0,
          0,
          Dimension.Width,
          Dimension.Height);
        ++XOffset;
        
      }
      
    } else {
      //TODO: logging
    }
  } else {
    //TODO: Logging
  }

  
    
  return(0);  }

