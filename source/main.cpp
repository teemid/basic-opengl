#include <Windows.h>
#include <Wingdi.h>

#include "gl.h"
#include "gl/wglext.h"

#include "assert.h"

#define GL_DEF(type, name) type name;
#include "glfunctions.def"

static bool running = false;

static PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

HGLRC CreateGlContext(HDC deviceContext, GLint major, GLint minor);
void DeleteGlContext(HGLRC * renderContext);
void LoadGlFunctions();
LRESULT CALLBACK Win32WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE _prev, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = Win32WindowProc;
    windowClass.hIcon = NULL;
    windowClass.hCursor = NULL;
    windowClass.lpszClassName = "Learn OpenGL Class";
    windowClass.hIconSm = NULL;

    Assert(RegisterClassEx(&windowClass), "Could not register Window Class.\n");

    HWND windowHandle = CreateWindow(
        "Learn OpenGL Class",
        "Learn OpenGL",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        0, 0,
        instance,
        0
    );

    Assert(windowHandle, "Failed to create window.\n");

    HDC deviceContext = GetDC(windowHandle);

    Assert(deviceContext != NULL, "Failed to get a device context.\n");

    ShowWindow(windowHandle, nCmdShow);

    HGLRC renderContext = CreateGlContext(deviceContext, 3, 3);
    LoadGlFunctions();

    running = true;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    while (running)
    {
        MSG message;
        while (PeekMessage(&message, windowHandle, 0, 0, PM_REMOVE))
        {
            switch (message.message)
            {
                case WM_KEYDOWN:
                {
                    if (message.wParam == VK_ESCAPE)
                    {
                        PostQuitMessage(0);
                        running = false;
                    }
                } break;
                default:
                {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                } break;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        SwapBuffers(deviceContext);
    }

    DeleteGlContext(&renderContext);

    return 0;
}

LRESULT CALLBACK Win32WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
        case WM_QUIT:
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            running = false;
        } break;
        default:
        {
            return DefWindowProc(windowHandle, message, wParam, lParam);
        } break;
    }

    return 0;
}

HGLRC CreateGlContext(HDC deviceContext, GLint major, GLint minor)
{
    PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
    desiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    desiredPixelFormat.nVersion = 1;
    desiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    desiredPixelFormat.cColorBits = 24;
    desiredPixelFormat.cAlphaBits = 8;
    desiredPixelFormat.cAccumBits = 0;
    desiredPixelFormat.cDepthBits = 24;
    desiredPixelFormat.cStencilBits = 8;

    // NOTE (Emil): https://msdn.microsoft.com/en-us/library/windows/desktop/dd318284(v=vs.85).aspx
    int pixelFormatResult = ChoosePixelFormat(deviceContext, &desiredPixelFormat);
    Assert(pixelFormatResult, "Failed to choose a pixel format.");

    // NOTE (Emil): https://msdn.microsoft.com/en-us/library/windows/desktop/dd369049(v=vs.85).aspx
    BOOL isPixelFormatSet = SetPixelFormat(deviceContext, pixelFormatResult, &desiredPixelFormat);
    Assert(isPixelFormatSet, "Failed to set pixel format.");

    HGLRC temporaryRenderContext = wglCreateContext(deviceContext);
    Assert(temporaryRenderContext, "Failed to create temporary render context.");

    wglMakeCurrent(deviceContext, temporaryRenderContext);

    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    Assert(wglGetExtensionsStringARB, "wglCreateContextAttribsARB not found.");

    // TODO (Emil): Do we have to free some of these strings?
    const char * extensions = wglGetExtensionsStringARB(deviceContext);

    const char * hasWglArbCreateContext = strstr(extensions, "WGL_ARB_create_context");
    const char * hasWglArbCreateContextProfile = strstr(extensions, "WGL_ARB_create_context_profile");

    if (hasWglArbCreateContext && hasWglArbCreateContextProfile)
    {
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(temporaryRenderContext);

    pixelFormatResult = ChoosePixelFormat(deviceContext, &desiredPixelFormat);
    Assert(pixelFormatResult, "Failed to choose a pixel format.");

    isPixelFormatSet = SetPixelFormat(deviceContext, pixelFormatResult, &desiredPixelFormat);
    Assert(isPixelFormatSet, "Failed to set pixel format.");

    int attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
        NULL, NULL
    };

    auto renderContext = wglCreateContextAttribsARB(deviceContext, 0, attributes);

    Assert(renderContext, "wglCreateContextAttribsARB failed to create an OpenGL render context.");
    Assert(wglMakeCurrent(deviceContext, renderContext), "Failed to make the render context currrent.");

    return renderContext;
}

void DeleteGlContext(HGLRC * renderContext)
{
    if (!renderContext) { return; }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(*renderContext);

    *renderContext = NULL;
}

void LoadGlFunctions(void)
{
    HMODULE dll = LoadLibrary("opengl32.dll");
    Assert(dll, "Failed to load opengl32.dll");

    auto LoadFunction = [dll](const char * functionName) -> void *
    {
        void * proc = wglGetProcAddress(functionName);

        /*
         * Although the specification says wglGetProcAddress only returns NULL on failure.
         * Some implementations also return 1, 2, 3 and -1.
         *
         * https://msdn.microsoft.com/en-us/library/windows/desktop/dd374386%28v=vs.85%29.aspx
         * https://www.opengl.org/wiki/Load_OpenGL_Functions#Windows
         */
        if (    proc == 0
            || (proc == (void *)0x1)
            || (proc == (void *)0x2)
            || (proc == (void *)0x3)
            || (proc == (void *)-1)
        )
        {
            proc = (void *)GetProcAddress(dll, functionName);
        }

        Assert(proc, "Failed to load OpenGL procedure: %s.", functionName);

        return proc;
    };

    #define GL_DEF(type, name) name = (type)LoadFunction(#name);
    #include "glfunctions.def"

    FreeLibrary(dll);
}
