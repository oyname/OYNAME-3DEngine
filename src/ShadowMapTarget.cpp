#include "ShadowMapTarget.h"

// ShadowMapTarget ist nur noch ein Tag/Handle.
// All API calls (bind/clear/unbind/viewport/SRV hazard) are handled in the backend.
//
// Diese Datei bleibt bewusst bestehen, damit alte Projektdateien, die ShadowMapTarget.cpp
// noch in der Build-Liste haben, weiterhin kompilieren (copy + redirect).
