Welcome to the world of 3D development!

The OYNMAE 3DEngine is a personal hobby project focused on developing a simple 3D engine based on DirectX11. It has been created purely out of personal interest, and there is no claim to professional or commercial quality.

The main goal of this project is to explore the fundamentals of 3D programming and to experiment with creative ideas. The engine is intended as a learning platform where new concepts can be tested and improved without pressure or strict requirements.

Everyone who is interested is welcome to exchange ideas, learn together, and explore the possibilities of 3D development. The joy of discovery and experimentation is the core motivation behind this project.

Thank you for your interest in the OYNMAE 3DEngine!


PROJECT OVERVIEW

The OYNMAE 3DEngine is a 3D engine written in C++ and built on DirectX 11. It provides a basic framework for rendering 3D graphics, loading textures, and experimenting with shaders and rendering techniques.

This project is designed for learning and experimentation rather than for production use.


SYSTEM REQUIREMENTS

To build and run the engine locally, the following components are required:

- Visual Studio 2022  
- Installed workload: C++ Desktop Development  
- Windows 10 or Windows 11 SDK  
- DirectX 11 (already included in the Windows SDK)

No additional external libraries or tools need to be installed.


DOWNLOADING THE SOURCE CODE

The complete source code of the engine is hosted on GitHub:

[https://github.com/OYNMAE 3DEngine/OYNMAE 3DEngine-Engine](https://github.com/oyname/OYNAME-3DEngine)

To download the project locally, use the following command:

git clone [https://github.com/OYNMAE 3DEngine/OYNMAE 3DEngine-Engine.git](https://github.com/oyname/OYNAME-3DEngine.git)

After cloning, the entire project will be available on your local machine.


PROJECT STRUCTURE

After cloning the repository, the directory layout will look similar to this:

OYNMAE 3DEngine/
    include/
    src/
    shaders/
    third_party/
    oyname.sln

Description of the main folders:

include/      – contains all header files of the engine

src/          – contains the C++ source code  

shaders/      – contains all HLSL shader files  

third_party/  – includes external components used by the engine  

oynmae.sln    – Visual Studio solution file  


BUILDING THE ENGINE

Opening the Project

To open the project, navigate to the cloned repository folder and open the file:

oyname.sln

Visual Studio will automatically load all project settings and source files.


Compiling the Engine

Inside Visual Studio:

1. Set the configuration to Debug or Release  
2. Set the platform to x64  
3. Choose: Build -> Build Solution  

After the build process completes, the engine can be started directly.


TROUBLESHOOTING

If problems occur when running the application, check the following settings:

Working Directory

Make sure that the Working Directory in Visual Studio is set to:

$(SolutionDir)

This ensures that shaders and assets can be found correctly at runtime.

Shader Paths

All shaders are loaded relative to the "shaders" folder.  
No manual path adjustments are required after cloning the repository.


THIRD-PARTY LIBRARIES

The engine uses only one external library:

stb_image – for loading textures

This library is header-only and is already included in the repository under:

third_party/

Therefore, no additional installation steps are necessary.


CONTRIBUTING

Anyone interested is welcome to test the engine, experiment with it, or contribute improvements.

Typical contribution workflow:

1. Create a new branch  
2. Implement changes or new features  
3. Commit and push your work  
4. Create a pull request on GitHub  

All required files and configurations are already included, so you can start working immediately without further setup.


FINAL WORDS

The OYNMAE 3DEngine is an open playground for learning, experimenting, and improving skills in 3D graphics programming.

Have fun exploring, testing, and further developing the OYNAME 3DEngine!
The oyname Engine is an open playground for learning, experimenting, and improving skills in 3D graphics programming.

Have fun exploring, testing, and further developing the oyname Engine!

