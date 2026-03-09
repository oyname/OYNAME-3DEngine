#include "RenderCommand.h"
#include "Mesh.h"
#include "Surface.h"
#include "Shader.h"
#include "gdxdevice.h"
#include "IRenderBackend.h"

void RenderCommand::Execute(const GDXDevice* device) const
{
    if (!mesh || !surface || !device) return;

    GDXDevice& dev = *const_cast<GDXDevice*>(device);

    // Upload the entity matrix CB (b0) on first draw this frame;
    // for subsequent draws of the same mesh only re-bind the existing buffer.
    // The ring buffer inside EntityGpuData ensures no GPU hazard between passes.
    if (!mesh->IsUpdatedThisFrame())
    {
        // matrixSet is written by RenderManager before the flush.
        mesh->Update(device, &mesh->matrixSet);
        mesh->MarkUpdated();
    }
    else
    {
        if (backend) backend->BindEntityConstants(dev, *mesh);
    }

    // Bone palette CB (b4) — routed through the backend, no DX11 in Execute.
    if (mesh->hasSkinning)
    {
        if (backend) backend->BindBoneConstants(dev, *mesh);
    }

    surface->gpu->Draw(device, flagsVertex);
}
