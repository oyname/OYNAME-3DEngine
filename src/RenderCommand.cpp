#include "RenderCommand.h"
#include "Mesh.h"
#include "Surface.h"
#include "Shader.h"
#include "gdxdevice.h"

void RenderCommand::Execute(const GDXDevice* device) const
{
    if (!mesh || !surface || !device) return;

    // Mesh-Matrix hochladen (nur beim ersten Auftritt pro Frame)
    if (!mesh->IsUpdatedThisFrame())
    {
        // MatrixSet wird vom RenderManager vor dem Flush gesetzt --
        // hier greifen wir direkt auf das bereits korrekt befuellte matrixSet zu.
        mesh->Update(device, &mesh->matrixSet);
        mesh->MarkUpdated();
    }
    else
    {
        // Bereits hochgeladen: nur Constant Buffer binden
        ID3D11Buffer* cb = mesh->constantBuffer;
        if (cb)
        {
            device->GetDeviceContext()->VSSetConstantBuffers(0, 1, &cb);
            device->GetDeviceContext()->PSSetConstantBuffers(0, 1, &cb);
        }
    }

    surface->gpu->Draw(device, flagsVertex);
}
