#include "RenderCommand.h"
#include "Mesh.h"
#include "Surface.h"
#include "Shader.h"
#include "gdxdevice.h"
#include "IRenderBackend.h"

void RenderCommand::Execute(const GDXDevice* device) const
{
    if (!mesh || !surface || !device) return;

    // Backend ist fuer Bindings zustaendig.
    // Ohne Backend: wir koennen trotzdem zeichnen, aber dann fehlen ggf. CB-Bindings.
    GDXDevice& dev = *const_cast<GDXDevice*>(device);

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
        // Bereits hochgeladen: nur Constant Buffer binden (backend-spezifisch)
        if (backend) backend->BindEntityConstants(dev, *mesh);
    }

    surface->gpu->Draw(device, flagsVertex);
}
