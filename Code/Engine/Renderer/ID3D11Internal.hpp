#ifdef DEFINE_D3D11_LIBS

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#pragma comment( lib, "d3d11.lib" )	// Link in the d3d11.lib static library
#pragma comment( lib, "dxgi.lib" )	// Link in the dxgi.lib static library
#pragma comment( lib, "d3dcompiler.lib" )	// Link in the d3dcompiler.lib static library

#endif

#define DX_SAFE_RELEASE(dxObject)                                \
{                                                                \
    if (( dxObject) != nullptr)                                  \
    {                                                            \
        (dxObject)->Release();                                   \
        (dxObject) = nullptr;                                    \
    }                                                            \
}
