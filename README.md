# glez

*3D modeling and texture painting*

This is a school project based on **Mesh Color Textures**, a model introduced in [this article](http://www.cemyuksel.com/research/meshcolors/mesh_color_textures.pdf).

Mesh Color Textures provide an alternative way to standard UV-unwrapping for manipulating and rendering textures in a real-time 3D graphics pipeline.

The code for this project falls into 3 parts : the glez DLL, need4speed and graffiti.

## glez DLL

The glez DLL defines the structures and algorithm needed for working with 3D objects that either use standard UV-unwrapping or Mesh Color Textures instead.

It also provides some basic utilities like a base class for creating scenes, already equipped with a camera.

The glez DLL doesn't perform any rendering of it's 3D objects, it only provides the textures and rendering data in a convenient format.
It is up to the client applications to perform the actual rendering process.

Third-party libraries : 
- [glm](https://github.com/g-truc/glm)
- [spdlog](https://github.com/gabime/spdlog)

## need4speed

The need4speed application is a performance measurement program meant to compare the rendering speed and texture space of standard UV-unwrapping and Mesh Color Textures.

It starts from an unwrapped mesh stored in an OBJ file and a 2D texture stored as an image on disk.
Then it creates a Mesh Color object with an identical mesh and a similar texture by transferring the color data.

Third-party libraries : 
- [stb_image](https://github.com/nothings/stb)
- [GLFW](https://www.glfw.org/)
- [GLEW](http://glew.sourceforge.net/)

## graffiti

Graffiti is a small modeling and texture painting software entirely based on Mesh Color Textures that allows simultaneous painting and mesh editing.

Users start with a cube and can easily perform loop cuts and extrusions, as well as spray painting (hence the name) and face filling.

This can be useful for beginners who just want to play around with 3D objects without having to dig into complex 3D software.

Third-party libraries : 
- [Dear ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://www.glfw.org/)
- [GLEW](http://glew.sourceforge.net/)

Notes : 
- there are still some unresolved bugs, mainly with the loop cut operation
- a lot of cool new features could be added to make this software more interesting !
