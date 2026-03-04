
# OYNAME-3DEngine — Anwenderhandbuch
DirectX 11 · C++ · BlitzBasic-inspirierte API

Dieses Dokument beschreibt die Anwendung der **OYNAME-3DEngine**.

## Minimales Programm

```cpp
#include "gidx.h"

int main()
{
    Engine::Graphics(1280,720);

    LPENTITY camera=nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera,0,0,-5);

    while(Windows::MainLoop())
    {
        Engine::Cls(40,40,60);

        Engine::UpdateWorld();
        Engine::RenderWorld();

        Engine::Flip();
    }
}
```

## Mesh und Surface

```
Mesh
 ├ Surface
 ├ Surface
 └ Surface
```

Ein **Mesh** ist das Objekt in der Szene.  
Eine **Surface** ist ein Submesh mit eigener Geometrie.

## Erstes 3D Objekt

```cpp
LPENTITY mesh=nullptr;
Engine::CreateMesh(&mesh,nullptr);

LPSURFACE surf=nullptr;
Engine::CreateSurface(&surf,mesh);

Engine::AddVertex(surf,-1,-1,0);
Engine::AddVertex(surf,1,-1,0);
Engine::AddVertex(surf,0,1,0);

Engine::AddTriangle(surf,0,1,2);

Engine::FillBuffer(surf);
```

## Texturen

```cpp
LPTEXTURE tex=nullptr;
Engine::LoadTexture(&tex,L"..\\media\\brick.png");

LPMATERIAL mat=nullptr;
Engine::CreateMaterial(&mat);

Engine::MaterialSetAlbedo(mat,tex);

Engine::EntityMaterial(mesh,mat);
```

## Licht

```cpp
LPENTITY light=nullptr;

Engine::CreateLight(&light,D3DLIGHT_DIRECTIONAL);
Engine::TurnEntity(light,45,0,0);

Engine::SetDirectionalLight(light);
```
