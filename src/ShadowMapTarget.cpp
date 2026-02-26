#include "ShadowMapTarget.h"

// Schritt 2:
// ShadowMapTarget ist nur noch ein Tag/Handle.
// Alle API-Calls (Bind/Clear/Unbind/Viewport/SRV-Hazard) werden im Backend erledigt.
//
// Diese Datei bleibt bewusst bestehen, damit alte Projektdateien, die ShadowMapTarget.cpp
// noch in der Build-Liste haben, weiterhin kompilieren (copy + redirect).
