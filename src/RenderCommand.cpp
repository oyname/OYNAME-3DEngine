#include "RenderCommand.h"
#include "Mesh.h"
#include "Surface.h"
#include "Shader.h"
#include "gdxdevice.h"
#include "IRenderBackend.h"

void RenderCommand::Execute(const GDXDevice* device) const
{
    if (!mesh || !surface || !device) return;

    // The backend is responsible for bindings.
    // Ohne Backend: wir koennen trotzdem zeichnen, aber dann fehlen ggf. CB-Bindings.
    GDXDevice& dev = *const_cast<GDXDevice*>(device);

    // Upload mesh matrix (only on first occurrence per frame)
    if (!mesh->IsUpdatedThisFrame())
    {
        // MatrixSet is set by RenderManager before the flush --
        // hier greifen wir direkt auf das bereits korrekt befuellte matrixSet zu.
        mesh->Update(device, &mesh->matrixSet);
        mesh->MarkUpdated();
    }
    else
    {
        // Bereits hochgeladen: nur Constant Buffer binden (backend-spezifisch)
        if (backend) backend->BindEntityConstants(dev, *mesh);
    }

    // Bone-Constant-Buffer auf b4 binden wenn Mesh Skinning verwendet
    if (mesh->hasSkinning && mesh->boneConstantBuffer)
    {
        ID3D11DeviceContext* ctx = device->GetDeviceContext();
        if (ctx) ctx->VSSetConstantBuffers(4, 1, &mesh->boneConstantBuffer);
    }

    surface->gpu->Draw(device, flagsVertex);
}
