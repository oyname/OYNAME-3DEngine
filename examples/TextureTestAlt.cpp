#include "gidx.h"
#include <chrono>
#include <thread>

// Function declaration
static void CreateCube(LPENTITY* mesh, MATERIAL* material = nullptr);
static void CreateSphere(LPENTITY* mesh, MATERIAL* material, int numSegments);
static void MoveObjectInCircle(LPENTITY mesh, float centerX, float centerZ, float radius, float angle);
static void CreateBox2(LPENTITY* mesh, MATERIAL* material, float sizeX, float sizeY, float sizeZ);
static void CreateBox(LPENTITY* mesh, MATERIAL* material, std::vector<DirectX::XMFLOAT3>& points);
static void InitializeBoxPoints(std::vector<DirectX::XMFLOAT3>& points, DirectX::XMFLOAT3 minCorner, DirectX::XMFLOAT3 maxCorner);
static void UpdateBoxWithCorners(LPENTITY mesh, unsigned int indexSurface, const DirectX::XMFLOAT3 corners[8]);
static void UpdateBoxPointsEx(std::vector<DirectX::XMFLOAT3>& points, DirectX::XMFLOAT3 minCorner, DirectX::XMFLOAT3 maxCorner);
static void UpdateBox(LPENTITY mesh, unsigned int indexSurface, std::vector<DirectX::XMFLOAT3>& points, DirectX::XMFLOAT3 minCorner, DirectX::XMFLOAT3 maxCorner);
static void CreateCollisionBox(LPSURFACE wuerfel, std::vector<DirectX::XMFLOAT3>& points);

static     LPSHADER g_shadertest;

int main()
{
    bool sw = true;

    sw == true ? Engine::Graphics(1200, 650) : Engine::Graphics(1980, 1080, false);


    g_shadertest = nullptr;
    Engine::CreateShader(&g_shadertest, L"..\\shaders\\VertexShaderRot.hlsl", "main", L"..\\shaders\\PixelShaderRot.hlsl", "main", Engine::CreateVertexFlags(true, false, false, false, false));



    // Texturen erstellen
    //
    LPTEXTURE texture1 = nullptr;
    Engine::LoadTexture(&texture1, L"..\\media\\engine.png");

    LPTEXTURE texture2 = nullptr;
    Engine::LoadTexture(&texture2, L"..\\media\\face.bmp");

    LPTEXTURE texture3 = nullptr;
    Engine::LoadTexture(&texture3, L"..\\media\\color3.png");

    // Das erste Material wird von der Engine bei Start erstellt
    //
    LPMATERIAL material2;
    Engine::CreateMaterial(&material2);
    // So kann man auch einem Material eine Textur zuweisen
    Engine::MaterialTexture(material2, texture1);

    LPMATERIAL material3;
    //Engine::CreateMaterial(&material3, g_shadertest);
    Engine::CreateMaterial(&material3);
    // So kann man auch einem Material eine Textur zuweisen
    Engine::MaterialTexture(material3, texture2);

    // Creating camera object
    LPENTITY camera;
    Engine::CreateCamera(&camera);

    // Positioning the camera
    Engine::RotateEntity(camera, 10.0f, 0.0f, 0.0f);
    Engine::PositionEntity(camera, 0.0f, 25.0f, -5.0f);

    // Light erstellen
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);    // Nutzt LightManager
    Engine::PositionEntity(light, 0.0f, 20.0f, 10.0f);
    Engine::RotateEntity(light, 90, 0, 0);                // Funktioniert
    Engine::LightColor(light, 0.4f, 0.4f, 0.5f);

    LPENTITY light2 = nullptr;
    Engine::CreateLight(&light2, D3DLIGHT_DIRECTIONAL);  // Nutzt LightManager
    Engine::PositionEntity(light2, 0.0f, 10.0f, -10.0f);
    Engine::RotateEntity(light2, -45, 0, 0);                // Funktioniert
    Engine::LightColor(light2, 0.5f, 0.0f, 0.0f);

    Engine::SetDirectionalLight(light);
    Engine::SetAmbientColor(0.1f, 0.1f, 0.1f);

    LPENTITY cube;
    CreateCube(&cube, material3);
    Engine::PositionEntity(cube, 0.0f, 10.0f, 10.0f);
    Engine::RotateEntity(cube, 0.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(cube, 4.0f, 4.0f, 4.0f);
    Engine::EntityCollisionMode(cube, COLLISION::BOX);

    LPENTITY cube2;
    CreateCube(&cube2, material2);
    Engine::PositionEntity(cube2, 0.0f, -10.0f, 10.0f);
    Engine::ScaleEntity(cube2, 50.0f, 0.2f, 50.0f);
    Engine::RotateEntity(cube2, 0.0f, 180.0f, 0.0f);
    Engine::EntityCollisionMode(cube2, COLLISION::BOX);

    Engine::EntityTexture(cube, texture2);

    Engine::engine->GetOM().AddMeshToMaterial(material2, dynamic_cast<Mesh*>(cube2));
    Engine::engine->GetOM().AddMeshToMaterial(material3, dynamic_cast<Mesh*>(cube));

    // Winkel inkrementieren, um das Objekt im Kreis zu bewegen
    double angle = 0.0;
    float speed = 100.0f;

    std::vector<DirectX::XMFLOAT3> points;
    InitializeBoxPoints(points, Engine::EntitySurface(cube, 0)->minPoint, Engine::EntitySurface(cube, 0)->maxPoint);
    std::vector<DirectX::XMFLOAT3> points2;
    InitializeBoxPoints(points2, Engine::EntitySurface(cube2, 0)->minPoint, Engine::EntitySurface(cube2, 0)->maxPoint);

    LPENTITY box;
    CreateBox(&box, material2, points);
    Engine::EntitySurface(box, 0)->test = true; // Debug...wenn true, dann wird die Box die Kontur dargestellt
    
    LPENTITY box2;
    CreateBox(&box2, material2, points2);
    Engine::EntitySurface(box2, 0)->test = true;// Debug...wenn true, dann wird die Box die Kontur dargestellt


    // Definiere die gewünschte Framerate (z. B. 60 FPS)
    const double TARGET_FPS = 60.0;
    const double FRAME_TIME = 1.0 / TARGET_FPS;

    auto lastTime = std::chrono::high_resolution_clock::now();


    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration<double>(startTime - lastTime).count();
        lastTime = startTime;

        Core::BeginFrame();

        const float dt = static_cast<float>(Core::GetDeltaTime());

        if (elapsedTime > 0.5) elapsedTime = 0.5;

        if ((GetAsyncKeyState(VK_ADD) & 0x8000))
        {
            Engine::MoveEntity(camera, 0.0f, 5 * dt, 0.0f);
        }
        if ((GetAsyncKeyState(VK_SUBTRACT) & 0x8000))
        {
            Engine::MoveEntity(camera, 0.0f, -5 * dt, 0.0f);
        }
        else
        {
            Engine::TurnEntity(cube, speed * dt, speed * dt, 0.0f);
        }
        
        if ((GetAsyncKeyState(VK_F1) & 0x8000))
        {
            Engine::MoveEntity(camera, 0.0f, 0.0f * dt, 2.0f * dt);
        }


        if (Engine::EntityCollision(cube, cube2))
           Debug::LogOnce("Kollision");
        else
            Debug::LogOnce("Keine Kollision");

        // Extrahiere OBB Corners und visualisiere
        DirectX::XMFLOAT3 corners[8];
        Engine::EntityOBB(cube)->GetCorners(corners);  // ← GEÄNDERT
        UpdateBoxWithCorners(box, 0, corners);

        DirectX::XMFLOAT3 corners1[8];
        Engine::EntityOBB(cube2)->GetCorners(corners1);  // ← GEÄNDERT
        UpdateBoxWithCorners(box2, 0, corners);

        Engine::LookAt(camera, DirectX::XMVectorGetX(cube->transform.GetPosition()),
            DirectX::XMVectorGetY(cube->transform.GetPosition()),
            DirectX::XMVectorGetZ(cube->transform.GetPosition()));

        // DEBUG: Zeige die Kamera-Richtung
        //DirectX::XMVECTOR LightForward = light->transform.getLookAt();
        //Debug::Log("LIGHT Forward: ",
        //    XMVectorGetX(LightForward), ", ",
        //    XMVectorGetY(LightForward), ", ",
        //    XMVectorGetZ(LightForward));


         // Rendering
        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
        // FPS nur jede N Frames ausgeben:
        //static int frameCount = 0;
        //if (frameCount++ % 150 == 0) {
        //    Debug::Log("FPS: ", Timer::GetFPS());
        //    frameCount = 0;
        //}
    }

    // Shutdown the engine
    return 0;
}

void MoveObjectInCircle(LPENTITY mesh, float centerX, float centerZ, float radius, float angle) {
    // Berechnen der neuen Position basierend auf dem Winkel
    float newX = centerX + radius * cos(angle);
    float newZ = centerZ + radius * sin(angle);

    mesh->transform.Position(newX, 7.0f, newZ);
}

void CreateCube(LPENTITY* mesh, MATERIAL* material)
{
    LPSURFACE wuerfel = NULL;

    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&wuerfel, (*mesh));

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    // Back (verts 0-3)
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    // Front (verts 4-7)
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    // Left (verts 8-11)
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    // Right (verts 12-15)
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    // Bottom (verts 16-19)
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    // Top (verts 20-23)
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);

    Engine::AddTriangle(wuerfel, 0, 1, 2); Engine::AddTriangle(wuerfel, 3, 2, 1);
    Engine::AddTriangle(wuerfel, 6, 5, 4); Engine::AddTriangle(wuerfel, 6, 7, 5);
    Engine::AddTriangle(wuerfel, 8, 9, 10); Engine::AddTriangle(wuerfel, 10, 9, 11);
    Engine::AddTriangle(wuerfel, 14, 13, 12); Engine::AddTriangle(wuerfel, 14, 15, 13);
    Engine::AddTriangle(wuerfel, 16, 17, 18); Engine::AddTriangle(wuerfel, 18, 17, 19);
    Engine::AddTriangle(wuerfel, 21, 22, 23); Engine::AddTriangle(wuerfel, 22, 21, 20);

    Engine::FillBuffer(wuerfel);
}

void CreateBox2(LPENTITY* mesh, MATERIAL* material, float sizeX, float sizeY, float sizeZ)
{
    LPSURFACE cubeSurface = NULL;

    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&cubeSurface, (*mesh));

    DirectX::XMVECTOR p1{ -sizeX / 2.0f, -sizeY / 2.0f, -sizeZ / 2.0f };
    DirectX::XMVECTOR p2{ -sizeX / 2.0f, sizeY / 2.0f, -sizeZ / 2.0f };
    DirectX::XMVECTOR p3{ sizeX / 2.0f, -sizeY / 2.0f, -sizeZ / 2.0f };
    DirectX::XMVECTOR p4{ sizeX / 2.0f, sizeY / 2.0f, -sizeZ / 2.0f };
    DirectX::XMVECTOR p5{ -sizeX / 2.0f, -sizeY / 2.0f, sizeZ / 2.0f };
    DirectX::XMVECTOR p6{ -sizeX / 2.0f, sizeY / 2.0f, sizeZ / 2.0f };
    DirectX::XMVECTOR p7{ sizeX / 2.0f, -sizeY / 2.0f, sizeZ / 2.0f };
    DirectX::XMVECTOR p8{ sizeX / 2.0f, sizeY / 2.0f, sizeZ / 2.0f };

    // Füge die Eckpunkte für jede Seite des Würfels hinzu
    Engine::AddVertex(cubeSurface, p1); Engine::VertexNormal(cubeSurface, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(cubeSurface, 224, 224, 224); Engine::VertexTexCoord(cubeSurface, 0.0f, 1.0f);
    Engine::AddVertex(cubeSurface, p2); Engine::VertexNormal(cubeSurface, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(cubeSurface, 224, 224, 224); Engine::VertexTexCoord(cubeSurface, 0.0f, 0.0f);
    Engine::AddVertex(cubeSurface, p3); Engine::VertexNormal(cubeSurface, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(cubeSurface, 224, 224, 224); Engine::VertexTexCoord(cubeSurface, 1.0f, 1.0f);
    Engine::AddVertex(cubeSurface, p4); Engine::VertexNormal(cubeSurface, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(cubeSurface, 224, 224, 224); Engine::VertexTexCoord(cubeSurface, 1.0f, 0.0f);

    Engine::AddVertex(cubeSurface, p5); Engine::VertexNormal(cubeSurface, 0.0f, 0.0f, 1.0f);  Engine::VertexColor(cubeSurface, 224, 224, 224); Engine::VertexTexCoord(cubeSurface, 0.0f, 1.0f);
    Engine::AddVertex(cubeSurface, p6); Engine::VertexNormal(cubeSurface, 0.0f, 0.0f, 1.0f);  Engine::VertexColor(cubeSurface, 224, 224, 224); Engine::VertexTexCoord(cubeSurface, 0.0f, 0.0f);
    Engine::AddVertex(cubeSurface, p7); Engine::VertexNormal(cubeSurface, 0.0f, 0.0f, 1.0f);  Engine::VertexColor(cubeSurface, 224, 224, 224); Engine::VertexTexCoord(cubeSurface, 1.0f, 1.0f);
    Engine::AddVertex(cubeSurface, p8); Engine::VertexNormal(cubeSurface, 0.0f, 0.0f, 1.0f);  Engine::VertexColor(cubeSurface, 224, 224, 224); Engine::VertexTexCoord(cubeSurface, 1.0f, 0.0f);

    // Definition der Dreiecke für jede Seite des Würfels
    Engine::AddTriangle(cubeSurface, 0, 1, 2);
    Engine::AddTriangle(cubeSurface, 3, 2, 1);

    Engine::AddTriangle(cubeSurface, 6, 5, 4);
    Engine::AddTriangle(cubeSurface, 5, 6, 7);

    Engine::AddTriangle(cubeSurface, 1, 5, 3);
    Engine::AddTriangle(cubeSurface, 3, 5, 7);

    Engine::AddTriangle(cubeSurface, 6, 0, 2);
    Engine::AddTriangle(cubeSurface, 4, 0, 6);

    Engine::FillBuffer(cubeSurface);
}

void CreateSphere(LPENTITY* mesh, MATERIAL* material, int numSegments)
{
    LPSURFACE sphere = NULL;
    const float radius = 1.0f;

    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&sphere, (*mesh));

    // Erzeugung der Kugelgeometrie
    for (int i = 0; i <= numSegments; ++i)
    {
        float theta1 = i * 3.14f / numSegments;
        float theta2 = (i + 1) * 3.14f / numSegments;

        for (int j = 0; j <= numSegments; ++j)
        {
            float phi = j * 2 * 3.14f / numSegments;

            float x = radius * sin(theta1) * cos(phi);
            float y = radius * sin(theta1) * sin(phi);
            float z = radius * cos(theta1);

            float u = static_cast<float>(j) / numSegments;
            float v = static_cast<float>(i) / numSegments;

            Engine::AddVertex(sphere, x, y, z);
            Engine::VertexNormal(sphere, x / radius, y / radius, z / radius);
            Engine::VertexColor(sphere, 255, 255, 255, 255);
            Engine::VertexTexCoord(sphere, u, v);
        }
    }

    // Definition der Dreiecke für die Kugel
    for (int i = 0; i < numSegments; ++i)
    {
        for (int j = 0; j < numSegments; ++j)
        {
            int p0 = i * (numSegments + 1) + j;
            int p1 = p0 + 1;
            int p2 = (i + 1) * (numSegments + 1) + j;
            int p3 = p2 + 1;

            Engine::AddTriangle(sphere, p0, p2, p1);
            Engine::AddTriangle(sphere, p2, p3, p1);
        }
    }

    Engine::FillBuffer(sphere);
}

void InitializeBoxPoints(std::vector<DirectX::XMFLOAT3>& points, DirectX::XMFLOAT3 minCorner, DirectX::XMFLOAT3 maxCorner)
{
    points.clear();

    points.push_back(minCorner);
    points.push_back(DirectX::XMFLOAT3(maxCorner.x, minCorner.y, minCorner.z));
    points.push_back(DirectX::XMFLOAT3(maxCorner.x, maxCorner.y, minCorner.z));
    points.push_back(DirectX::XMFLOAT3(minCorner.x, maxCorner.y, minCorner.z));

    points.push_back(DirectX::XMFLOAT3(minCorner.x, minCorner.y, maxCorner.z));
    points.push_back(DirectX::XMFLOAT3(maxCorner.x, minCorner.y, maxCorner.z));
    points.push_back(maxCorner);
    points.push_back(DirectX::XMFLOAT3(minCorner.x, maxCorner.y, maxCorner.z));
}

void UpdateBoxWithCorners(LPENTITY mesh, unsigned int indexSurface, const DirectX::XMFLOAT3 corners[8])
{
    // Definition der Eckpunkte für jede Seite des Würfels
    Engine::AddVertex(0, Engine::EntitySurface(mesh,indexSurface), corners[0]);
    Engine::AddVertex(1, Engine::EntitySurface(mesh,indexSurface), corners[1]);
    Engine::AddVertex(2, Engine::EntitySurface(mesh,indexSurface), corners[2]);
    Engine::AddVertex(3, Engine::EntitySurface(mesh,indexSurface), corners[3]);

    Engine::AddVertex(4, Engine::EntitySurface(mesh,indexSurface), corners[4]);
    Engine::AddVertex(5, Engine::EntitySurface(mesh,indexSurface), corners[5]);
    Engine::AddVertex(6, Engine::EntitySurface(mesh,indexSurface), corners[6]);
    Engine::AddVertex(7, Engine::EntitySurface(mesh,indexSurface), corners[7]);

    Engine::AddVertex(8, Engine::EntitySurface(mesh,indexSurface), corners[0]);
    Engine::AddVertex(9, Engine::EntitySurface(mesh,indexSurface), corners[4]);
    Engine::AddVertex(10, Engine::EntitySurface(mesh,indexSurface), corners[1]);
    Engine::AddVertex(11, Engine::EntitySurface(mesh,indexSurface), corners[5]);

    Engine::AddVertex(12, Engine::EntitySurface(mesh,indexSurface), corners[2]);
    Engine::AddVertex(13, Engine::EntitySurface(mesh,indexSurface), corners[6]);
    Engine::AddVertex(14, Engine::EntitySurface(mesh,indexSurface), corners[3]);
    Engine::AddVertex(15, Engine::EntitySurface(mesh,indexSurface), corners[7]);

    Engine::AddVertex(16, Engine::EntitySurface(mesh,indexSurface), corners[1]);
    Engine::AddVertex(17, Engine::EntitySurface(mesh,indexSurface), corners[2]);
    Engine::AddVertex(18, Engine::EntitySurface(mesh,indexSurface), corners[5]);
    Engine::AddVertex(19, Engine::EntitySurface(mesh,indexSurface), corners[6]);

    Engine::AddVertex(20, Engine::EntitySurface(mesh,indexSurface), corners[0]);
    Engine::AddVertex(21, Engine::EntitySurface(mesh,indexSurface), corners[3]);
    Engine::AddVertex(22, Engine::EntitySurface(mesh,indexSurface), corners[4]);
    Engine::AddVertex(23, Engine::EntitySurface(mesh,indexSurface), corners[7]);

    Engine::UpdateVertexBuffer(Engine::EntitySurface(mesh, indexSurface));
}

void UpdateBoxPointsEx(std::vector<DirectX::XMFLOAT3>& points, DirectX::XMFLOAT3 minCorner, DirectX::XMFLOAT3 maxCorner) {
    if (points.size() != 8) {
        InitializeBoxPoints(points, minCorner, maxCorner);
    }
    else {
        points.push_back(DirectX::XMFLOAT3(minCorner.x, minCorner.y, minCorner.z));
        points.push_back(DirectX::XMFLOAT3(maxCorner.x, minCorner.y, minCorner.z));
        points.push_back(DirectX::XMFLOAT3(maxCorner.x, maxCorner.y, minCorner.z));
        points.push_back(DirectX::XMFLOAT3(minCorner.x, maxCorner.y, minCorner.z));
        points.push_back(DirectX::XMFLOAT3(minCorner.x, minCorner.y, maxCorner.z));
        points.push_back(DirectX::XMFLOAT3(maxCorner.x, minCorner.y, maxCorner.z));
        points.push_back(DirectX::XMFLOAT3(maxCorner.x, maxCorner.y, maxCorner.z));
        points.push_back(DirectX::XMFLOAT3(minCorner.x, maxCorner.y, maxCorner.z));
    }
}

void CreateBox(LPENTITY* mesh, MATERIAL* material, std::vector<DirectX::XMFLOAT3>& points)
{
    LPSURFACE wuerfel = NULL;

    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&wuerfel, (*mesh));

    DirectX::XMFLOAT3 p1 = points[0];
    DirectX::XMFLOAT3 p2 = points[1];
    DirectX::XMFLOAT3 p3 = points[2];
    DirectX::XMFLOAT3 p4 = points[3];
    DirectX::XMFLOAT3 p5 = points[4];
    DirectX::XMFLOAT3 p6 = points[5];
    DirectX::XMFLOAT3 p7 = points[6];
    DirectX::XMFLOAT3 p8 = points[7];

    // Definition der Eckpunkte für jede Seite des Würfels
    Engine::AddVertex(wuerfel, p1);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p2);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p3);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p4);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::AddVertex(wuerfel, p5);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p6);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p7);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p8);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::AddVertex(wuerfel, p1);  Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p5);  Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p2);  Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p6);  Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::AddVertex(wuerfel, p3);  Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p7);  Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p4);  Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p8);  Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::AddVertex(wuerfel, p2); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p3); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p6); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p7); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::AddVertex(wuerfel, p1);  Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p4);  Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p5);  Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p8);  Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    // Definition der Dreiecke für jede Seite des Würfels
    Engine::AddTriangle(wuerfel, 2, 1, 0);
    Engine::AddTriangle(wuerfel, 0, 3, 2);

    Engine::AddTriangle(wuerfel, 4, 5, 6);
    Engine::AddTriangle(wuerfel, 6, 7, 4);

    Engine::AddTriangle(wuerfel, 10, 9, 8);
    Engine::AddTriangle(wuerfel, 11, 9, 10);

    Engine::AddTriangle(wuerfel, 14, 13, 12);
    Engine::AddTriangle(wuerfel, 14, 15, 13);

    Engine::AddTriangle(wuerfel, 16, 17, 18);
    Engine::AddTriangle(wuerfel, 18, 17, 19);

    Engine::AddTriangle(wuerfel, 21, 22, 23);
    Engine::AddTriangle(wuerfel, 22, 21, 20);

    Engine::FillBuffer(wuerfel);
}

void CreateCollisionBox(LPSURFACE wuerfel, std::vector<DirectX::XMFLOAT3>& points)
{
    DirectX::XMFLOAT3 p1 = points[0];
    DirectX::XMFLOAT3 p2 = points[1];
    DirectX::XMFLOAT3 p3 = points[2];
    DirectX::XMFLOAT3 p4 = points[3];
    DirectX::XMFLOAT3 p5 = points[4];
    DirectX::XMFLOAT3 p6 = points[5];
    DirectX::XMFLOAT3 p7 = points[6];
    DirectX::XMFLOAT3 p8 = points[7];

    // Definition der Eckpunkte für jede Seite des Würfels
    Engine::AddVertex(wuerfel, p1);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p2);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p3);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p4);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::AddVertex(wuerfel, p5);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p6);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p7);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p8);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::AddVertex(wuerfel, p1);  Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p5);  Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p2);  Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p6);  Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::AddVertex(wuerfel, p3);  Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p7);  Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p4);  Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p8);  Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::AddVertex(wuerfel, p2); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p3); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p6); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p7); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::AddVertex(wuerfel, p1);  Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p4);  Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p5);  Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);
    Engine::AddVertex(wuerfel, p8);  Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 244);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    // Definition der Dreiecke für jede Seite des Würfels
    Engine::AddTriangle(wuerfel, 2, 1, 0);
    Engine::AddTriangle(wuerfel, 0, 3, 2);

    Engine::AddTriangle(wuerfel, 4, 5, 6);
    Engine::AddTriangle(wuerfel, 6, 7, 4);

    Engine::AddTriangle(wuerfel, 10, 9, 8);
    Engine::AddTriangle(wuerfel, 11, 9, 10);

    Engine::AddTriangle(wuerfel, 14, 13, 12);
    Engine::AddTriangle(wuerfel, 14, 15, 13);

    Engine::AddTriangle(wuerfel, 16, 17, 18);
    Engine::AddTriangle(wuerfel, 18, 17, 19);

    Engine::AddTriangle(wuerfel, 21, 22, 23);
    Engine::AddTriangle(wuerfel, 22, 21, 20);

    Engine::FillBuffer(wuerfel);
}