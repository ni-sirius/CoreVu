CoreVu
CoreVu is a Vulkan-based rendering engine designed for high-performance graphics applications. It provides a robust framework for rendering 3D models, handling multiple light sources, and managing textures efficiently.

Features
Vulkan Integration: Utilizes Vulkan API for rendering, providing fine-grained control over GPU resources.
Model Loading: Supports loading models using TinyOBJLoader.
Multiple Light Sources: Implements multiple point lights with flexible rendering approaches.
Texture Management: Efficient handling of textures with support for various layout transitions.
Pipeline Configuration: Customizable graphics pipeline with support for blending, depth testing, and more.
Getting Started
Prerequisites
Vulkan SDK: Ensure you have the Vulkan SDK installed on your system.
C++ Compiler: A modern C++ compiler that supports C++17 or later.
CMake: Build system generator to configure and build the project.
Building the Project
Clone the repository:

Create a build directory and navigate to it:

Configure the project using CMake:

Build the project:

Running the Application
After building the project, you can run the application from the build directory:

Project Structure
src/: Contains the source code for the CoreVu engine.
corevu_model.cpp: Handles model loading and rendering.
corevu_pipeline.cpp: Configures the Vulkan graphics pipeline.
corevu_texture.cpp: Manages texture loading and layout transitions.
systems/point_light_system.cpp: Implements point light rendering.
include/: Contains the header files for the CoreVu engine.
corevu_model.hpp: Declares the CoreVuModel class and related structures.
corevu_pipeline.hpp: Declares the pipeline configuration functions.
corevu_texture.hpp: Declares the texture management functions.
systems/point_light_system.hpp: Declares the PointLightSystem class.
Usage
Loading a Model
To load a model, use the CreateModelFromPath function:

Rendering with Multiple Point Lights
Configure the point lights using the PointLightSystem:

Texture Management
Handle texture layout transitions in corevu_texture.cpp:

Contributing
Contributions are welcome! Please fork the repository and submit a pull request with your changes.

License
This project is licensed under the MIT License - see the LICENSE file for details.

Acknowledgements
Vulkan SDK: For providing the tools and libraries necessary for Vulkan development.
TinyOBJLoader: For enabling easy loading of OBJ models.