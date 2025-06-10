#ifndef WINDOW_H
#define WINDOW_H

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "input/input.h"
#include "render/vertex_buffer.h"
#include "render/texture.h"

class GLFWvidmode;
class GLFWwindow;
class RendererBase;

class Window
{
    // window state
    bool                m_is_fullscreen    = false;
    GLFWvidmode const * mp_base_video_mode = nullptr;
    GLFWwindow *        mp_glfw_win        = nullptr;
    glm::ivec2 const    m_size;   // initial size
    glm::ivec2          m_vp_size;
    std::string         m_title;

    std::unique_ptr<Input>        m_input_ptr;
    std::unique_ptr<RendererBase> m_render_ptr;

    // Scene
    VertexBuffer     m_pyramid;
    VertexBuffer     m_plane;
    VertexBuffer     m_sphere;
    Texture          m_render_texture;
    Texture          m_base_texture;
    Texture          m_second_texture;
    Texture          m_decal_texture;
    Texture          m_marble_texture;
    Texture          m_shadow_texture;
    Texture          m_reflection_texture;
	Texture          m_cube_map;
    TextureProjector m_decal_prj;
    TextureProjector m_shadow_prj;
    TextureProjector m_camera_prj;
	TextureProjector m_cube_map_prj;
	Light            m_light;

public:
    Window(int width, int height, char const * title);
    ~Window();

    Window(Window const &)             = delete;
    Window & operator=(Window const &) = delete;

    bool isFullscreen() const { return m_is_fullscreen; }

    void createWindow();
    void initScene();
    void fullscreen(bool is_fullscreen);
    void run();

    // keys
    void key_f1();
};

#endif   // WINDOW_H
