#include "window.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

#include "render/renderer.h"
#include "input/inputglfw.h"
#include "scene_data.h"

namespace
{
constexpr char const *                diffuse_tex_fname = "uv.tga";
constexpr char const *                base_tex_fname    = "color.tga";
constexpr char const *                decal_tex_fname   = "decal.tga";
constexpr char const *                marble_tex_fname  = "marble.tga";
constexpr std::array<char const *, 6> cube_map_names    = {
    {"cm_left.tga", "cm_right.tga", "cm_top.tga", "cm_bottom.tga", "cm_back.tga", "cm_front.tga"}
};
}   // namespace

Window::Window(int width, int height, char const * title)
    : m_size{width, height},
      m_title{title},
      m_pyramid{VertexBuffer::pos_norm_tex, 2}
{
    // Initialise GLFW
    if(!glfwInit())
    {
        throw std::runtime_error{"Failed to initialize GLFW"};
    }

    mp_base_video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    m_light.m_type     = Light::LightType::Point;
    m_light.m_range    = 100.f;
    m_light.m_diffuse  = glm::vec4(1.f);
    m_light.m_specular = glm::vec4(1.f);
    m_light.m_ambient  = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    m_light.m_position = glm::vec4(0.f, 4.5f, 3.3f, 1.0f);

    m_render_texture.m_type        = Texture::Type::TEXTURE_2D;
    m_render_texture.m_format      = Texture::Format::R8G8B8A8;
    m_render_texture.m_width       = 1024;
    m_render_texture.m_height      = 1024;
    m_render_texture.m_gen_mips    = false;
    m_render_texture.m_sampler.max = Texture::Filter::LINEAR;
    m_render_texture.m_sampler.min = Texture::Filter::LINEAR;
    m_render_texture.m_sampler.r   = Texture::Wrap::CLAMP_TO_EDGE;
    m_render_texture.m_sampler.s   = Texture::Wrap::CLAMP_TO_EDGE;
    m_render_texture.m_sampler.t   = Texture::Wrap::CLAMP_TO_EDGE;

    m_base_texture.m_type        = Texture::Type::TEXTURE_2D;
    m_base_texture.m_format      = Texture::Format::R8G8B8A8;
    m_base_texture.m_width       = 1024;
    m_base_texture.m_height      = 1024;
    m_base_texture.m_sampler.max = Texture::Filter::LINEAR;
    m_base_texture.m_sampler.min = Texture::Filter::LINEAR_MIPMAP_LINEAR;
    m_base_texture.m_sampler.r   = Texture::Wrap::REPEAT;
    m_base_texture.m_sampler.s   = Texture::Wrap::REPEAT;
    m_base_texture.m_sampler.t   = Texture::Wrap::REPEAT;

    m_second_texture.m_type        = Texture::Type::TEXTURE_2D;
    m_second_texture.m_format      = Texture::Format::R8G8B8A8;
    m_second_texture.m_width       = 1024;
    m_second_texture.m_height      = 1024;
    m_second_texture.m_sampler.max = Texture::Filter::LINEAR;
    m_second_texture.m_sampler.min = Texture::Filter::LINEAR_MIPMAP_LINEAR;
    m_second_texture.m_sampler.r   = Texture::Wrap::REPEAT;
    m_second_texture.m_sampler.s   = Texture::Wrap::REPEAT;
    m_second_texture.m_sampler.t   = Texture::Wrap::REPEAT;

    m_marble_texture.m_type        = Texture::Type::TEXTURE_2D;
    m_marble_texture.m_format      = Texture::Format::R8G8B8A8;
    m_marble_texture.m_width       = 1024;
    m_marble_texture.m_height      = 1024;
    m_marble_texture.m_sampler.max = Texture::Filter::LINEAR;
    m_marble_texture.m_sampler.min = Texture::Filter::LINEAR_MIPMAP_LINEAR;
    m_marble_texture.m_sampler.r   = Texture::Wrap::REPEAT;
    m_marble_texture.m_sampler.s   = Texture::Wrap::REPEAT;
    m_marble_texture.m_sampler.t   = Texture::Wrap::REPEAT;

    m_decal_texture.m_type        = Texture::Type::TEXTURE_2D;
    m_decal_texture.m_format      = Texture::Format::R8G8B8A8;
    m_decal_texture.m_width       = 1024;
    m_decal_texture.m_height      = 1024;
    m_decal_texture.m_sampler.max = Texture::Filter::LINEAR;
    m_decal_texture.m_sampler.min = Texture::Filter::LINEAR;
    m_decal_texture.m_sampler.r   = Texture::Wrap::CLAMP_TO_BORDER;
    m_decal_texture.m_sampler.s   = Texture::Wrap::CLAMP_TO_BORDER;
    m_decal_texture.m_sampler.t   = Texture::Wrap::CLAMP_TO_BORDER;

    m_shadow_texture.m_type                 = Texture::Type::TEXTURE_2D;
    m_shadow_texture.m_format               = Texture::Format::DEPTH;
    m_shadow_texture.m_width                = 1024;
    m_shadow_texture.m_height               = 1024;
    m_shadow_texture.m_gen_mips             = false;
    m_shadow_texture.m_sampler.max          = Texture::Filter::LINEAR;
    m_shadow_texture.m_sampler.min          = Texture::Filter::LINEAR;
    m_shadow_texture.m_sampler.r            = Texture::Wrap::CLAMP_TO_EDGE;
    m_shadow_texture.m_sampler.s            = Texture::Wrap::CLAMP_TO_EDGE;
    m_shadow_texture.m_sampler.t            = Texture::Wrap::CLAMP_TO_EDGE;
    m_shadow_texture.m_sampler.compare_mode = true;

    m_reflection_texture.m_type        = Texture::Type::TEXTURE_2D;
    m_reflection_texture.m_format      = Texture::Format::R8G8B8A8;
    m_reflection_texture.m_width       = 1024;
    m_reflection_texture.m_height      = 1024;
    m_reflection_texture.m_gen_mips    = false;
    m_reflection_texture.m_sampler.max = Texture::Filter::LINEAR;
    m_reflection_texture.m_sampler.min = Texture::Filter::LINEAR;
    m_reflection_texture.m_sampler.r   = Texture::Wrap::CLAMP_TO_EDGE;
    m_reflection_texture.m_sampler.s   = Texture::Wrap::CLAMP_TO_EDGE;
    m_reflection_texture.m_sampler.t   = Texture::Wrap::CLAMP_TO_EDGE;

    m_cube_map_texture.m_type        = Texture::Type::TEXTURE_CUBE;
    m_cube_map_texture.m_format      = Texture::Format::R8G8B8A8;
    m_cube_map_texture.m_width       = 128;
    m_cube_map_texture.m_height      = 128;
    m_cube_map_texture.m_gen_mips    = false;
    m_cube_map_texture.m_sampler.max = Texture::Filter::LINEAR;
    m_cube_map_texture.m_sampler.min = Texture::Filter::LINEAR;
    m_cube_map_texture.m_sampler.r   = Texture::Wrap::CLAMP_TO_EDGE;
    m_cube_map_texture.m_sampler.s   = Texture::Wrap::CLAMP_TO_EDGE;
    m_cube_map_texture.m_sampler.t   = Texture::Wrap::CLAMP_TO_EDGE;

    m_decal_prj.projected_texture = &m_decal_texture;
    m_decal_prj.modelview = glm::lookAt({0.0f, 0.0f, 3.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});

    m_shadow_prj.projected_texture = &m_shadow_texture;
    m_shadow_prj.modelview           = glm::lookAt(m_light.m_position, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    m_shadow_prj.is_ortho          = true;

    m_camera_prj.projected_texture = &m_reflection_texture;
    m_camera_prj.is_reflection     = true;
    glm::mat4 mtx = glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, -7.0f});
    mtx           = glm::rotate(mtx, glm::radians(45.0f), {1.0f, 0.0f, 0.0f});
    mtx           = glm::rotate(mtx, glm::radians(60.0f), {0.0f, 1.0f, 0.0f});
    m_camera_prj.modelview = mtx;

    m_cube_map_prj.projected_texture = &m_cube_map_texture;
    m_cube_map_prj.is_cube_map       = true;
}

Window::~Window()
{
    // Cleanup VBO and textures
    if(mp_glfw_win && m_render_ptr->isInit())
    {
        m_render_ptr->unloadBuffer(m_pyramid);
        m_render_ptr->deleteBuffer(m_pyramid);

        m_render_ptr->unloadBuffer(m_plane);
        m_render_ptr->deleteBuffer(m_plane);

        m_render_ptr->unloadBuffer(m_sphere);
        m_render_ptr->deleteBuffer(m_sphere);

        m_render_ptr->destroyTexture(m_render_texture);
        m_render_ptr->destroyTexture(m_base_texture);
        m_render_ptr->destroyTexture(m_second_texture);
        m_render_ptr->destroyTexture(m_marble_texture);
        m_render_ptr->destroyTexture(m_decal_texture);
        m_render_ptr->destroyTexture(m_shadow_texture);
        m_render_ptr->destroyTexture(m_reflection_texture);
        m_render_ptr->destroyTexture(m_cube_map_texture);

        m_render_ptr->terminate();
    }

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

void Window::createWindow()
{
    GLFWmonitor * mon;
    if(m_is_fullscreen)
    {
        mon         = glfwGetPrimaryMonitor();
        m_vp_size.x = mp_base_video_mode->width;
        m_vp_size.y = mp_base_video_mode->height;
    }
    else
    {
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        mon         = nullptr;
        m_vp_size.x = m_size.x;
        m_vp_size.y = m_size.y;
    }

    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    GLFWwindow * new_window{nullptr};
    new_window = glfwCreateWindow(m_vp_size.x, m_vp_size.y, "", mon, mp_glfw_win);
    if(mp_glfw_win != nullptr)
        glfwDestroyWindow(mp_glfw_win);

    mp_glfw_win = new_window;
    if(mp_glfw_win == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error{"Failed to create GLFW window"};
    }
    glfwMakeContextCurrent(mp_glfw_win);
    glfwSetWindowTitle(mp_glfw_win, m_title.c_str());

    m_render_ptr = std::make_unique<RendererBase>();

    m_render_ptr->setViewport(0, 0, m_vp_size.x, m_vp_size.y);
    auto prj_mtx = glm::perspective(
        glm::radians(45.0f), static_cast<float>(m_vp_size.x) / static_cast<float>(m_vp_size.y), 0.1f, 100.0f);
    m_render_ptr->setMatrix(RendererBase::MatrixType::PROJECTION, prj_mtx);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(mp_glfw_win, GLFW_STICKY_KEYS, GL_TRUE);

    m_render_ptr->init();

    m_render_ptr->addLight(m_light);

    // input backend
    m_input_ptr = std::make_unique<InputGLFW>(mp_glfw_win);
}

void Window::fullscreen(bool is_fullscreen)
{
    if(is_fullscreen == m_is_fullscreen)
        return;

    m_is_fullscreen = is_fullscreen;
    createWindow();
}

void Window::initScene()
{
    m_pyramid.pushBack(pyr_vertex_buffer_data, {pyr_tex_buffer_data0, pyr_tex_buffer_data1},
                       pyr_normal_buffer_data, sizeof(pyr_vertex_buffer_data) / (sizeof(float) * 3),
                       pyr_index_buffer_data, sizeof(pyr_index_buffer_data) / sizeof(unsigned int));
    m_render_ptr->uploadBuffer(m_pyramid);

    m_plane.pushBack(plane_vertex_buffer_data, {plane_tex_buffer_data}, plane_normal_buffer_data,
                     sizeof(plane_vertex_buffer_data) / (sizeof(float) * 3), plane_index_buffer_data,
                     sizeof(plane_index_buffer_data) / sizeof(unsigned int));
    m_render_ptr->uploadBuffer(m_plane);

    m_sphere.pushBack(sphere_vertex_buffer_data, {sphere_tex_buffer_data}, sphere_normal_buffer_data,
                      sizeof(sphere_vertex_buffer_data) / (sizeof(float) * 3), sphere_index_buffer_data,
                      sizeof(sphere_index_buffer_data) / sizeof(unsigned int));
    m_render_ptr->uploadBuffer(m_sphere);

    // create textures
    if(!m_second_texture.loadImageDataFromFile(diffuse_tex_fname, *m_render_ptr))
        throw std::runtime_error("Texture not found");

    if(!m_base_texture.loadImageDataFromFile(base_tex_fname, *m_render_ptr))
        throw std::runtime_error("Texture not found");

    if(!m_decal_texture.loadImageDataFromFile(decal_tex_fname, *m_render_ptr))
        throw std::runtime_error("Texture not found");

    if(!m_marble_texture.loadImageDataFromFile(marble_tex_fname, *m_render_ptr))
        throw std::runtime_error("Texture not found");

    if(!m_cube_map_texture.loadCubeMapFromFiles(cube_map_names, *m_render_ptr))
        throw std::runtime_error("Texture not found");
}

void Window::run()
{
    bool        once = true;
    glm::mat4   prj_mtx, mtx;
    TextureSlot slot;

    do
    {
        m_input_ptr->update();

        // render shadow
        if(m_render_ptr->bindTextureAsFrameBuffer(nullptr, &m_shadow_texture))
        {
            m_render_ptr->setMatrix(RendererBase::MatrixType::PROJECTION, m_shadow_prj.getProjectionMatrix());
            m_render_ptr->setMatrix(RendererBase::MatrixType::MODELVIEW, m_shadow_prj.getModelviewMatrix());

            m_render_ptr->clearDepthBuffer();

            auto        old_offset = m_render_ptr->getOffsetState();
            OffsetState temp_offset_state;
            temp_offset_state.fill_enabled  = true;
            temp_offset_state.line_enabled  = false;
            temp_offset_state.point_enabled = false;
            temp_offset_state.scale         = 4.f;
            temp_offset_state.bias          = 4.f;
            m_render_ptr->setOffsetState(temp_offset_state);

            m_render_ptr->bindVertexBuffer(&m_plane);
            m_render_ptr->draw(m_plane);
            m_render_ptr->unbindVertexBuffer();

            m_render_ptr->bindVertexBuffer(&m_pyramid);
            m_render_ptr->draw(m_pyramid);
            m_render_ptr->unbindVertexBuffer();

            m_render_ptr->bindVertexBuffer(&m_sphere);
            m_render_ptr->draw(m_sphere);
            m_render_ptr->unbindVertexBuffer();

            m_render_ptr->setOffsetState(old_offset);

            m_render_ptr->setIdentityMatrix(RendererBase::MatrixType::MODELVIEW);
            m_render_ptr->setIdentityMatrix(RendererBase::MatrixType::PROJECTION);
            m_render_ptr->unbindTexturesFromFrameBuffer();
            m_render_ptr->bindDefaultFbo();
        }

        // // render to texture
        if(m_render_ptr->bindTextureAsFrameBuffer(&m_render_texture))
        {
            prj_mtx = glm::perspective(glm::radians(45.0f), 1.f, 0.1f, 100.0f);
            m_render_ptr->setMatrix(RendererBase::MatrixType::PROJECTION, prj_mtx);
            mtx = glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, -3.0f});
            m_render_ptr->setMatrix(RendererBase::MatrixType::MODELVIEW, mtx);

            m_render_ptr->setClearColor(glm::vec4(0.0f, 0.4f, 0.0f, 0.0f));
            m_render_ptr->clearColorBuffer();
            m_render_ptr->clearDepthBuffer();

            m_render_ptr->bindLights();

            slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
            slot.tex_channel_num   = 0;
            slot.texture           = &m_second_texture;
            slot.projector         = nullptr;
            slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
            m_render_ptr->addTextureSlot(slot);
            m_render_ptr->bindSlots();
            m_render_ptr->bindVertexBuffer(&m_pyramid);
            m_render_ptr->draw(m_pyramid);
            m_render_ptr->unbindVertexBuffer();
            m_render_ptr->unbindAndClearSlots();

            m_render_ptr->unbindLights();

            m_render_ptr->setIdentityMatrix(RendererBase::MatrixType::MODELVIEW);
            m_render_ptr->setIdentityMatrix(RendererBase::MatrixType::PROJECTION);
            m_render_ptr->unbindTexturesFromFrameBuffer();
            m_render_ptr->bindDefaultFbo();
        }

        // // render reflection
        if(m_render_ptr->bindTextureAsFrameBuffer(&m_reflection_texture))
        {
            // make reflection matrix
            // reflect modelview
            // set clip plane
            // render scene with prj matrix and new modelview

            glm::vec3 v0(plane_vertex_buffer_data[0], plane_vertex_buffer_data[1],
                         plane_vertex_buffer_data[2]),
                v1(plane_vertex_buffer_data[3], plane_vertex_buffer_data[4], plane_vertex_buffer_data[5]),
                v2(plane_vertex_buffer_data[6], plane_vertex_buffer_data[7], plane_vertex_buffer_data[8]);

            auto plane              = TextureProjector::GetPlaneFromPoints(v0, v1, v2);
            m_camera_prj.reflection = TextureProjector::GetReflectionMatrix(plane);

            m_render_ptr->setClearColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
            m_render_ptr->clearColorBuffer();
            m_render_ptr->clearDepthBuffer();

            m_render_ptr->setMatrix(RendererBase::MatrixType::PROJECTION, m_camera_prj.getProjectionMatrix());
            m_render_ptr->setMatrix(RendererBase::MatrixType::MODELVIEW,
                                    m_camera_prj.getModelviewMatrix() * m_camera_prj.reflection);

            auto      old_cull = m_render_ptr->getCullState();
            CullState cull;
            cull.ccw_order = false;
            cull.enabled   = true;
            m_render_ptr->setCullState(cull);

            m_render_ptr->enableClipPlane(0, plane);

            m_render_ptr->bindLights();

            slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
            slot.tex_channel_num   = 0;
            slot.texture           = &m_base_texture;
            slot.projector         = nullptr;
            slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
            m_render_ptr->addTextureSlot(slot);
            slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
            slot.tex_channel_num   = 1;
            slot.texture           = &m_render_texture;
            slot.projector         = nullptr;
            slot.combine_mode.mode = CombineStage::CombineMode::DECAL;
            m_render_ptr->addTextureSlot(slot);
            slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_GENERATED;
            slot.texture           = nullptr;
            slot.projector         = &m_decal_prj;
            slot.combine_mode.mode = CombineStage::CombineMode::DECAL;
            m_render_ptr->addTextureSlot(slot);
            m_render_ptr->bindSlots();
            m_render_ptr->bindVertexBuffer(&m_pyramid);
            m_render_ptr->draw(m_pyramid);
            m_render_ptr->unbindVertexBuffer();
            m_render_ptr->unbindAndClearSlots();

            slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
            slot.tex_channel_num   = 0;
            slot.texture           = &m_marble_texture;
            slot.projector         = nullptr;
            slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
            m_render_ptr->addTextureSlot(slot);
            slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_GENERATED;
            slot.texture           = nullptr;
            slot.projector         = &m_decal_prj;
            slot.combine_mode.mode = CombineStage::CombineMode::DECAL;
            m_render_ptr->addTextureSlot(slot);

            m_render_ptr->bindSlots();
            m_render_ptr->bindVertexBuffer(&m_plane);
            m_render_ptr->draw(m_plane);
            m_render_ptr->unbindVertexBuffer();
            m_render_ptr->unbindAndClearSlots();

            auto & slot_ref            = m_render_ptr->getTextureSlot(m_render_ptr->addTextureSlot({}));
            slot_ref.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
            slot_ref.tex_channel_num   = 0;
            slot_ref.texture           = &m_base_texture;
            slot_ref.combine_mode.mode = CombineStage::CombineMode::MODULATE;

            auto const slot_num = m_render_ptr->addTextureSlot({});
            m_render_ptr->getTextureSlot(slot_num).coord_source =
                TextureSlot::TexCoordSource::TEX_COORD_GENERATED;
            m_render_ptr->getTextureSlot(slot_num).projector         = &m_decal_prj;
            m_render_ptr->getTextureSlot(slot_num).combine_mode.mode = CombineStage::CombineMode::DECAL;

            m_render_ptr->bindSlots();
            m_render_ptr->bindVertexBuffer(&m_sphere);
            m_render_ptr->draw(m_sphere);
            m_render_ptr->unbindVertexBuffer();
            m_render_ptr->unbindAndClearSlots();

            m_render_ptr->unbindLights();

            AABB test_box({-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f});
            m_render_ptr->drawBBox(test_box, glm::mat4(1.f), {1.0f, 0.0f, 0.0f});

            m_render_ptr->disableClipPlane(0);
            m_render_ptr->setCullState(old_cull);

            m_render_ptr->setIdentityMatrix(RendererBase::MatrixType::MODELVIEW);
            m_render_ptr->setIdentityMatrix(RendererBase::MatrixType::PROJECTION);
            m_render_ptr->unbindTexturesFromFrameBuffer();
            m_render_ptr->bindDefaultFbo();
        }

        if(once)
        {
            tex::ImageData image;

            m_render_ptr->get2DTextureData(m_reflection_texture, image);
            tex::WriteTGA("reflection.tga", image);

            once = false;
        }

        //         Render scene:
        // bind lights
        // for each mesh:
        //      set slots
        //      bind textures
        //      bind VBO
        //      draw object
        //      unbind VBO
        //      unbind textures
        //      clear slots
        // unbind lights
        m_render_ptr->setClearColor(glm::vec4(0.0f, 0.0f, 0.4f, 1.0f));
        m_render_ptr->clearBuffers();

        prj_mtx =
            glm::perspective(glm::radians(45.0f),
                             static_cast<float>(m_vp_size.x) / static_cast<float>(m_vp_size.y), 0.1f, 100.0f);
        m_render_ptr->setMatrix(RendererBase::MatrixType::PROJECTION, prj_mtx);
        m_render_ptr->setMatrix(RendererBase::MatrixType::MODELVIEW, m_camera_prj.getModelviewMatrix());

        m_render_ptr->bindLights();

        slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
        slot.tex_channel_num   = 0;
        slot.texture           = &m_base_texture;
        slot.projector         = nullptr;
        slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
        m_render_ptr->addTextureSlot(slot);
        slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
        slot.tex_channel_num   = 1;
        slot.texture           = &m_render_texture;
        slot.projector         = nullptr;
        slot.combine_mode.mode = CombineStage::CombineMode::DECAL;
        m_render_ptr->addTextureSlot(slot);
        slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_GENERATED;
        slot.texture           = nullptr;
        slot.projector         = &m_decal_prj;
        slot.combine_mode.mode = CombineStage::CombineMode::DECAL;
        m_render_ptr->addTextureSlot(slot);
        slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_GENERATED;
        slot.texture           = nullptr;
        slot.projector         = &m_cube_map_prj;
        slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
        m_render_ptr->addTextureSlot(slot);
        m_render_ptr->bindSlots();
        m_render_ptr->bindVertexBuffer(&m_pyramid);
        m_render_ptr->draw(m_pyramid);
        m_render_ptr->unbindVertexBuffer();
        m_render_ptr->unbindAndClearSlots();

        slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
        slot.tex_channel_num   = 0;
        slot.texture           = &m_marble_texture;
        slot.projector         = nullptr;
        slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
        m_render_ptr->addTextureSlot(slot);

        CombineStage blend_combine;
        blend_combine.mode           = CombineStage::CombineMode::COMBINE;
        blend_combine.rgb_func       = CombineStage::CombineFunctions::INTERPOLATE;
        blend_combine.alpha_func     = CombineStage::CombineFunctions::REPLACE;
        blend_combine.rgb_src0       = CombineStage::SrcType::TEXTURE_STAGE;
		blend_combine.rgb_stage0 = 0;
        blend_combine.rgb_src1       = CombineStage::SrcType::TEXTURE_STAGE;
		blend_combine.rgb_stage1 = 1;
        blend_combine.rgb_src2       = CombineStage::SrcType::TEXTURE_STAGE;
		blend_combine.rgb_stage2 = 1;
        blend_combine.alpha_src0     = CombineStage::SrcType::TEXTURE_STAGE;
		blend_combine.alpha_stage0 = 1;
        blend_combine.alpha_src1     = CombineStage::SrcType::TEXTURE_STAGE;
		blend_combine.alpha_stage1 = 1;
        blend_combine.alpha_src2     = CombineStage::SrcType::TEXTURE_STAGE;
		blend_combine.alpha_stage2 = 1;
        blend_combine.rgb_operand0   = CombineStage::OperandType::SRC_COLOR;
        blend_combine.rgb_operand1   = CombineStage::OperandType::SRC_COLOR;
        blend_combine.rgb_operand2   = CombineStage::OperandType::SRC_ALPHA;
        blend_combine.alpha_operand0 = CombineStage::OperandType::SRC_ALPHA;
        blend_combine.alpha_operand1 = CombineStage::OperandType::SRC_ALPHA;
        blend_combine.alpha_operand2 = CombineStage::OperandType::SRC_ALPHA;
        slot.coord_source            = TextureSlot::TexCoordSource::TEX_COORD_GENERATED;
        slot.texture                 = nullptr;
        slot.projector               = &m_camera_prj;
        slot.combine_mode            = blend_combine;
        // slot.combine_mode.mode = CombineStage::CombineMode::DECAL;
        m_render_ptr->addTextureSlot(slot);

        slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_GENERATED;
        slot.texture           = nullptr;
        slot.projector         = &m_shadow_prj;
        slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
        m_render_ptr->addTextureSlot(slot);
        slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_GENERATED;
        slot.texture           = nullptr;
        slot.projector         = &m_decal_prj;
        slot.combine_mode.mode = CombineStage::CombineMode::DECAL;
        m_render_ptr->addTextureSlot(slot);

        m_render_ptr->bindSlots();
        m_render_ptr->bindVertexBuffer(&m_plane);
        m_render_ptr->draw(m_plane);
        m_render_ptr->unbindVertexBuffer();
        m_render_ptr->unbindAndClearSlots();

        auto & slot_ref            = m_render_ptr->getTextureSlot(m_render_ptr->addTextureSlot({}));
        slot_ref.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
        slot_ref.tex_channel_num   = 0;
        slot_ref.texture           = &m_base_texture;
        slot_ref.combine_mode.mode = CombineStage::CombineMode::MODULATE;

        auto const slot_num = m_render_ptr->addTextureSlot({});
        m_render_ptr->getTextureSlot(slot_num).coord_source =
            TextureSlot::TexCoordSource::TEX_COORD_GENERATED;
        m_render_ptr->getTextureSlot(slot_num).projector         = &m_decal_prj;
        m_render_ptr->getTextureSlot(slot_num).combine_mode.mode = CombineStage::CombineMode::DECAL;

        m_render_ptr->bindSlots();
        m_render_ptr->bindVertexBuffer(&m_sphere);
        m_render_ptr->draw(m_sphere);
        m_render_ptr->unbindVertexBuffer();
        m_render_ptr->unbindAndClearSlots();

        m_render_ptr->unbindLights();

        AABB test_box({-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f});
        m_render_ptr->drawBBox(test_box, glm::mat4(1.f), {1.0f, 0.0f, 0.0f});

        m_render_ptr->setIdentityMatrix(RendererBase::MatrixType::MODELVIEW);

        // Swap buffers
        glfwSwapBuffers(mp_glfw_win);
        glfwPollEvents();

        if(m_input_ptr->isKeyPressed(KeyboardKey::Key_F1))
            key_f1();
    }   // Check if the ESC key was pressed or the window was closed
    while(!m_input_ptr->isKeyPressed(KeyboardKey::Key_Escape) && glfwWindowShouldClose(mp_glfw_win) == 0);
}

void Window::key_f1()
{
    fullscreen(!m_is_fullscreen);
}
