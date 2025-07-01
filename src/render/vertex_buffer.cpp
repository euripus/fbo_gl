#include "vertex_buffer.h"
#include <assert.h>
// #include <algorithm>

VertexBuffer::VertexBuffer(ComponentsFlags format, uint32_t num_tex_channels)
    : m_tex_channels_count(num_tex_channels),
      m_components(format),
      m_state(State::NODATA)
{}

VertexBuffer::~VertexBuffer()
{
    clear();
}

void VertexBuffer::insertVertices(uint32_t const index, float const * pos,
                                  std::vector<float const *> const & tex, float const * norm,
                                  uint32_t const vcount)
{
    assert(index < m_vertex_count);
    assert(pos);

    if(m_components[ComponentsBitPos::normal])
    {
        assert(norm);

        auto norm_it = m_dynamic_buffer.begin() + m_vertex_count * 3 + index * 3;
        m_dynamic_buffer.insert(norm_it, norm, norm + vcount * 3);
    }
    auto pos_it = m_dynamic_buffer.begin() + index * 3;
    m_dynamic_buffer.insert(pos_it, pos, pos + vcount * 3);

    if(m_components[ComponentsBitPos::tex])
    {
        assert(tex.size() == m_tex_channels_count);

        for(int32_t i = static_cast<int32_t>(m_tex_channels_count) - 1; i >= 0; --i)
        {
            uint32_t ui     = static_cast<uint32_t>(i);
            auto     tex_it = m_static_bufffer.begin() + m_vertex_count * 2 * ui + index * 2;
            m_static_bufffer.insert(tex_it, tex[ui], tex[ui] + vcount * 2);
        }
    }

    m_vertex_count += vcount;
    m_state         = State::INITDATA;
}

void VertexBuffer::insertIndices(uint32_t const index, uint32_t const * indices, uint32_t const icount)
{
    assert(index < m_indices.size());
    assert(indices);

    auto ind_it = m_indices.begin() + index;
    m_indices.insert(ind_it, indices, indices + icount);

    m_state = State::INITDATA;
}

void VertexBuffer::pushBack(float const * pos, std::vector<float const *> const & tex, float const * norm,
                            uint32_t const vcount, uint32_t const * indices, uint32_t const icount)
{
    assert(pos);
    assert(indices);

    uint32_t vstart = m_vertex_count;
    uint32_t istart = static_cast<uint32_t>(m_indices.size());

    auto pos_it_end = m_dynamic_buffer.begin() + m_vertex_count * 3;

    m_dynamic_buffer.insert(pos_it_end, pos, pos + vcount * 3);
    if(m_components[ComponentsBitPos::normal])
        m_dynamic_buffer.insert(m_dynamic_buffer.end(), norm, norm + vcount * 3);

    if(m_components[ComponentsBitPos::tex])
    {
        assert(tex.size() > 0);
        assert(tex.size() == m_tex_channels_count);

        for(int32_t i = static_cast<int32_t>(m_tex_channels_count) - 1; i >= 0; --i)
        {
            uint32_t ui         = static_cast<uint32_t>(i);
            auto     tex_end_it = m_static_bufffer.begin() + m_vertex_count * ui * 2;
            m_static_bufffer.insert(tex_end_it, tex[ui], tex[ui] + vcount * 2);
        }
    }

    m_indices.insert(m_indices.end(), indices, indices + icount);

    for(uint32_t i = 0; i < icount; i++)
    {
        m_indices[istart + i] += vstart;
    }

    m_vertex_count += vcount;
    m_state         = State::INITDATA;
}

void VertexBuffer::eraseVertices(uint32_t const first, uint32_t const last)
{
    assert(last > first);
    assert(last < m_vertex_count);

    if(m_components[ComponentsBitPos::tex])
    {
        for(int32_t i = static_cast<int32_t>(m_tex_channels_count) - 1; i >= 0; --i)
        {
            uint32_t ui = static_cast<uint32_t>(i);
            m_static_bufffer.erase(m_static_bufffer.begin() + m_vertex_count * 2 * ui + first * 2,
                                   m_static_bufffer.begin() + m_vertex_count * 2 * ui + last * 2);
        }
    }

    if(m_components[ComponentsBitPos::normal])
        m_dynamic_buffer.erase(m_dynamic_buffer.begin() + m_vertex_count * 3 + first * 3,
                               m_dynamic_buffer.begin() + m_vertex_count * 3 + last * 3);
    m_dynamic_buffer.erase(m_dynamic_buffer.begin() + first * 3, m_dynamic_buffer.begin() + last * 3);

    m_indices.erase(m_indices.begin() + first, m_indices.begin() + last);

    for(uint32_t i = 0; i < m_indices.size(); i++)
    {
        if(m_indices[i] > first)
        {
            m_indices[i] -= (last - first);
        }
    }

    m_state = State::INITDATA;
}

void VertexBuffer::clear()
{
    m_state = State::NODATA;

    m_static_bufffer.resize(0);
    m_dynamic_buffer.resize(0);
}

void Add2DRectangle(VertexBuffer & vb, float x0, float y0, float x1, float y1, float s0, float t0, float s1,
                    float t1)
{
    assert(vb.getNumTexChannels() == 1);

    uint32_t indices[6] = {0, 1, 2, 0, 3, 1};

    float vertices[4 * 3]  = {x0, y0, 0.f, x1, y1, 0.f, x0, y1, 0.f, x1, y0, 0.f};
    float tex_coord[4 * 2] = {s0, t0, s1, t1, s0, t1, s1, t0};

    vb.pushBack(vertices, {tex_coord}, nullptr, 4, indices, 6);
}
