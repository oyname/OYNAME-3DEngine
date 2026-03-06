
OYNAME-3DEngine
Technische Architekturdokumentation
DirectX 11 · Feature Level 11_0 · Shader Model 5.0 · Forward Renderer

Version: 2025  ·  Intern / Entwicklung

# 1. Overview und Design-Philosophie
OYNAME-3DEngine ist eine in C++ entwickelte 3D-Grafikengine, die DirectX 11 mit Feature Level 11_0 und Shader Model 5.0 verwendet. Das zentrale Designziel ist eine BlitzBasic-inspirierte Anwender-API, die die gesamte DirectX-Komplexität hinter übersichtlichen, direkten Functionsaufrufen verbirgt. Entwickler schreiben keine DirectX-Ressourcenverwaltung, keine COM-Interfaces und keine HLSL-Register-Konfiguration — all das liegt in der Engine.

Folgende Kernprinzipien prägen die Architektur:


# 2. Systemstruktur
## 2.1 Startsequenz
Die Engine wird in main.cpp über zwei Schichten initialisiert. Core::Init() richtet das Win32-Fenster, COM und den Timer ein. Core::CreateEngine() instantiiert den GDXEngine-Singleton und initialisiert alle Manager-Classn. Der eigentliche Game loop läuft auf einem separaten Thread, während der Haupt-Thread die Win32-Message-Loop bedient.


// WinMain - Initializationsreihenfolge:
Core::Init(hInst, WindowProc, desc);   // Fenster + COM + Timer
Core::CreateEngine();                   // GDXEngine-Singleton
std::thread gameThread([]{ main(); }); // Game loop-Thread
// Win32 Message-Loop im Hauptthread


## 2.2 GDXEngine (Zentraler Singleton)
GDXEngine ist der Kern der Engine. Er ist als Singleton ausgelegt und wird über Engine::engine als globaler Zeiger erreichbar. GDXEngine besitzt und koordiniert alle Manager-Instanzen. Kein Code außerhalb der Engine-Interna kommuniziert direkt mit den Managern — ausschließlich über die gidx.h-API.


## 2.3 File structure

# 3. Entity-System
## 3.1 Basisklasse Entity
Alle szenenbezogenen Objecte — Meshes, Cameras, Lighter — erben von Entity. Entity enthält einen Transform (Position, Rotation, Skalierung), eine Viewport-Struktur, MatrixSet für Constant-Buffer-Daten und optionale GPU-Ressourcen via EntityGpuData.

Der Zustand einer Entity wird über drei orthogonale Flags gesteuert: active (steuert Update und Rendering komplett), visible (Update läuft weiter, nur Rendering wird unterdrückt) und layerMask (Bitmask für Camera-Culling). EntityCastShadows steuert separat die Teilnahme am Shadow Pass.

## 3.2 Typsystem — kein dynamic_cast im Hot Path
Jede Entity trägt ein EntityType-Tag (Mesh=1, Camera=2, Light=3). Die Methoden IsMesh(), IsCamera(), IsLight() prüfen diesen Tag in O(1) ohne RTTI. AsMesh(), AsCamera(), AsLight() führen reinterpret_cast durch, nachdem IsMesh() positiv war. Das ist sicher, weil die Typen in der Konstruktionskette korrekt gesets werden und niemals getauscht werden.

## 3.3 Hierarchy — Parent/Child
Entities können beliebig hierarchisch verschachtelt werden. SetEntityParent(child, parent) registriert child als Kind und parent als Elternobjekt. Der lokale Transform bleibt unverändert; GetWorldMatrix() berechnet die vollständige Weltmatrix durch rekursive Multiplikation entlang der Parent-Chain. Space::Local und Space::World steuern, in welchem Koordinatensystem MoveEntity und RotateEntity wirken.

# 4. Mesh-Architektur
## 4.1 Komponenten-Modell
Das Mesh-System ist dreischichtig aufgebaut. MeshAsset enthält reine Geometrie als Vektor von Surface-Slots, ohne Transform und ohne Material. Surface enthält die eigentlichen Vertex- und Indexdaten (Position, Normal, Tangent, Color, UV1, UV2, Bone-Daten) sowie einen GPU-Puffer-Wrapper. MeshRenderer koppelt ein MeshAsset mit einem Material-Array pro Slot und gehört zur Mesh-Entity.


## 4.2 Asset sharing
ShareMeshAsset(source, target) lässt zwei Mesh-Entities dieselbe MeshAsset-Instanz referenzieren. Die Geometrie liegt nur einmal im GPU-Speicher. Jede Instanz hat ihren eigenen MeshRenderer und damit ein eigenes Material pro Slot, eigene Sichtbarkeit und eigenen Transform. Beim Löschen eines Meshes, das ein geteiltes Asset trägt, muss asset auf nullptr gesets werden, bevor DeleteMesh aufgerufen wird, um Doppelfreigabe zu verhindern.

## 4.3 Tombstoning beim Slot-Entfernen
RemoveSlot auf einem MeshAsset ersets den Slot durch nullptr statt den Vektor zu kompaktieren. Dadurch bleiben alle bestehenden Slot-Indizes stabil, solange das Asset aktiv ist. NumActiveSlots() zählt nur nicht-null Slots. Der Renderer überspringt Tombstone-Slots automatisch.

# 5. Material-System
## 5.1 MaterialData-Layout
MaterialData ist ein C-Struct, das exakt dem cbuffer MaterialBuffer (Register b2) im Pixel-Shader entspricht. Es ist 16-Byte-aligned. Das Struct enthält Classic-Felder (baseColor, specularColor), PBR-Felder (metallic, roughness, normalScale, occlusionStrength), Emissive- und UV-Tiling-Daten sowie ein Flags-Bitfeld und einen transparenten BlendMode-Wert.

## 5.2 Material-Flags

## 5.3 Texturee-Slots im Shader
Der Pixel-Shader liest Textureen über einen globalen TextureePool aus. Jede Texture erhält bei der Registrierung einen stabilen uint32-Index. Das Material speichert diese Indizes als albedoIndex, normalIndex, ormIndex, decalIndex usw. Die Slot-Konvention für MaterialTexturee ist: 0=Albedo, 1=Normal, 2=ORM, 3=Decal.

## 5.4 TextureePool
Der TextureePool ist ein zentraler SRV-Manager. GetOrAdd(srv) gibt einen stabilen Index zurück; existiert der SRV bereits, wird der vorhandene Index zurückgegeben. Default-Textureen werden beim Engine-Start angelegt: WhiteTexturee (Index 0), FlatNormalTexturee (Index 1), DefaultORM (Index 2). Materialien ohne explizite Texture-Zuweisung erhalten automatisch diese Default-Werte.

# 6. Render-Pipeline
## 6.1 Zweiphasiges Rendering
Jeder Frame durchläuft zwei Haupt-Phasen. Phase 1 ist der Shadow Pass: Der Szene-Graph wird aus Sicht des Directional Lights gerendert, nur Tiefeninformation wird geschrieben, der Shadow Map. Phase 2 ist der Normal Pass: Opaque-Geometrie wird in der RenderQueue nach SortKey (Shader-ID, Material-ID, Depth) sortiert und front-to-back gerendert. Danach folgt der Transparent Pass mit back-to-front-Sortierung.

## 6.2 RenderQueue und SortKey
Die RenderQueue sammelt RenderCommand-Objecte, von denen jedes einen 64-Bit-SortKey trägt. Der SortKey codiert Shader-Pointer (upper bits), Material-Pointer und Tiefe, sodass State-Changes minimiert werden. Für den Transparent Pass werden Paare aus (depth, RenderCommand) verwendet und nach Tiefe absteigend sortiert, sodass weiter entfernte Objecte zuerst gerendert werden.

## 6.3 Shadow Mapping
Das Shadow Mapping verwendet einen dedizierten Shadow-Map-Render-Target (ShadowMapTarget). Nur ein Directional Light kann gleichzeitig Schatten werfen. Der Shadow Pass rendert alle schattenwerfenden Meshes aus Sicht des Lights in eine Tiefen-Texture. Im Normal Pass wird dieser Tiefen-Buffer als SRV gebunden und per PCF-Filterung (Percentage Closer Filtering) für weiche Schatten ausgewertet. MaterialReceiveShadows steuert auf Material-Ebene, ob ein Object Schatten empfängt.

## 6.4 Render‑to‑Texture (RTT)
CreateRenderTexturee legt eine RenderTextureeTarget-Instanz an. SetRenderTarget leitet alle nachfolgenden RenderWorld-Aufrufe in diese Texture um. GetRTTTexturee liefert die resultierende Texture als LPTEXTURE für MaterialSetAlbedo oder EntityTexturee. ResetRenderTarget stellt den Backbuffer als aktives Render-Target wieder her. SRV-Hazards zwischen Shadow Pass und RTT werden durch explizites Unbinden vor Target-Wechseln verhindert.

## 6.5 Camera-Layer-Culling
Jede Entity trägt eine LayerMask (uint32). Jede Camera trägt eine CullMask. Beim Aufbau der RenderQueue werden nur Entities übergeben, deren LayerMask & CullMask != 0 ist. Damit lassen sich separate Render-Passes für UI, Reflexion oder andere Effekte realisieren, ohne Szenen-Kopien anzulegen. Konstanten LAYER_DEFAULT, LAYER_ALL und eigene Bitmasks werden in RenderLayers.h definiert.

# 7. Shader-System
## 7.1 Constant-Buffer-Belegung
Die Registervergabe ist fest und muss in allen Shadern konsistent sein. Konflikte auf einem Register sind ein stiller Errors (kein Compile-Error, falsches Rendering).


## 7.2 Vertex-Format-Flags
Der Shader-Compiler und das Input-Layout-System arbeiten mit DWORD-Flags aus CreateVertexFlags(). Jedes Flag aktiviert einen Vertex-Stream: D3DVERTEX_POSITION, D3DVERTEX_NORMAL, D3DVERTEX_COLOR, D3DVERTEX_TEX1, D3DVERTEX_TEX2, D3DVERTEX_TANGENT, D3DVERTEX_BONE_INDICES, D3DVERTEX_BONE_WEIGHTS. FillBuffer() liest diese Flags und erstellt nur die Vertex-Buffer, für die der Shader tatsächlich Daten erwartet.

## 7.3 Bekannte Einschränkungen (DX11 / SM5.0)
Dynamisches Texture-Array-Indexing (Texturee2D gTex[16]) ist mit SM5.0 nicht kompatibel. Der TextureePool verwendet daher einzelne, namentlich gebundene Texture-Bindings. UpdateSubresource() ist nur für DEFAULT-Usage-Buffer gültig; DYNAMIC-Buffer erfordern Map/Unmap. Operator-Overloads für Enums müssen in Header-Fileen definiert sein, nicht in .cpp-Fileen, damit sie in allen Translation Units sichtbar sind.

# 8. Skeletal animation
Das Skinning-System überträgt pro Vertex bis zu vier Bone-Indizes und vier Bone-Weights auf die GPU. VertexBoneData() sets diese Daten für einen einzelnen Vertex vor dem FillBuffer()-Aufruf. SetEntityBoneMatrices() lädt ein Array von XMMATRIX-Bone-Transformationen in den BoneBuffer (b4) auf der GPU. Der Skinning-Vertex-Shader transformiert jeden Vertex als gewichtete Summe der vier Bone-Matrizen. hasSkinning auf dem Mesh steuert, welcher Vertex-Shader-Pfad verwendet wird.

# 9. Timer und Frame-Steuerung
Der Timer ist ein Singleton. Core::BeginFrame() und Core::EndFrame() klammern den Update-Schritt und aktualisieren die internen Zeitmesswerte. Zugriff erfolgt ausschließlich über statische Getter.


# 10. Bekannte Architektur-Muster und Fallstricke
## 10.1 MeshAsset-Co-Deletion
DeleteMesh() löscht das mit dem Mesh verknüpfte MeshAsset standardmäßig mit. Wenn zwei Meshes dasselbe Asset über ShareMeshAsset() teilen, muss das Asset des zu löschenden Meshes vor dem Aufruf von DeleteMesh() auf nullptr gesets werden. Andernfalls wird das geteilte Asset doppelt freigegeben.

## 10.2 SRV-Hazards zwischen Passes
DirectX 11 lässt nicht zu, dass eine Texture gleichzeitig als Render-Target-View und als Shader-Resource-View gebunden ist. Vor jedem Wechsel des Render-Targets (Shadow Map → Backbuffer, Backbuffer → RTT) müssen alle SRV-Bindings explizit auf nullptr gesets werden. Der RenderManager erledigt das intern, aber eigene Backend-Erweiterungen müssen dies beachten.

## 10.3 Memory Leak durch Container-Ownership
Beim Löschen von Ressourcen muss die Ressource aus ihrem besitzenden Container im ObjectManager entfernt werden, nicht nur der Zeiger auf null gesets werden. Der ObjectManager ist der alleinige Eigentümer aller Entities, Materialien und Assets. Zeiger, die in anderen Strukturen gehalten werden, sind non-owning und dürfen nicht zum Löschen verwendet werden.

## 10.4 F0-Berechnung in PBR
Die F0-Berechnung (Fresnel-Basisreflektivität) für metallische Materialien muss das endgültige, blendete Albedo verwenden, nicht eine Vor-Blend-Approximation. Wird F0 vor der Detail-Map-Mischung berechnet, entstehen sichtbare Artefakte bei hohem Metallic-Wert mit Detail-Maps.

## 10.5 Debug output
Alle Debug::Log()-Aufrufe beginnen als erstes Element mit dem Filenamen, gefolgt von einer beschreibenden Meldung. Example: Debug::Log("gdxdevice.cpp: Comparison Sampler created"). Das ist die verbindliche Coding Rule der Engine.

# 11. Geplante Erweiterungen
