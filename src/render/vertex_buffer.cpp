#include "vertex_buffer.h"
#include <assert.h>
#include <algorithm>

VertexBuffer::VertexBuffer(ComponentsFlags format, uint32_t num_tex_channels) :
    m_tex_channels_count(num_tex_channels),
    m_components(format),
    m_state(State::NODATA)
{
}

VertexBuffer::~VertexBuffer()
{
    clear();
}

void VertexBuffer::insertVertices(uint32_t const index, float const * pos,
                                  std::vector<float const *> const & tex, float const * norm,
                                  uint32_t const vcount)
{
    assert(index * 3 < m_pos.size());
    assert(pos);
    assert(tex.size() == m_texs.size());

    auto flt_it = m_pos.begin() + index * 3;
    m_pos.insert(flt_it, pos, pos + vcount * 3);
    if(m_components[ComponentsBitPos::tex])
    {
        assert(index * 2 < m_texs[0].size());

        for(uint32_t i = 0; i < tex.size(); ++i)
        {
            auto tex_it = m_texs[i].begin() + index * 2;
            m_texs[i].insert(tex_it, tex[i], tex[i] + vcount * 2);
        }
    }
    if(m_components[ComponentsBitPos::normal])
    {
        assert(index * 3 < m_norm.size());
        assert(norm);

        auto norm_it = m_pos.begin() + index * 3;
        m_norm.insert(norm_it, norm, norm + vcount * 3);
    }

    m_state = State::INITDATA;
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

    uint32_t vstart = static_cast<uint32_t>(m_pos.size() / 3);
    uint32_t istart = static_cast<uint32_t>(m_indices.size());

    m_pos.insert(m_pos.end(), pos, pos + vcount * 3);
    if(m_components[ComponentsBitPos::tex])
    {
        assert(tex.size() == m_texs.size());

        for(uint32_t i = 0; i < tex.size(); ++i)
        {
            m_texs[i].insert(m_texs[i].end(), tex[i], tex[i] + vcount * 2);
        }
    }
    if(m_components[ComponentsBitPos::normal])
        m_norm.insert(m_norm.end(), norm, norm + vcount * 3);
    m_indices.insert(m_indices.end(), indices, indices + icount);

    for(uint32_t i = 0; i < icount; i++)
    {
        m_indices[istart + i] += vstart;
    }

    m_state = State::INITDATA;
}

void VertexBuffer::eraseVertices(uint32_t const first, uint32_t const last)
{
    assert(last > first);
    assert(last * 3 < m_pos.size());
    assert(last < m_indices.size());

    m_pos.erase(m_pos.begin() + first * 3, m_pos.begin() + last * 3);
    if(m_components[ComponentsBitPos::tex])
    {
        for(uint32_t i = 0; i < m_texs.size(); ++i)
        {
            m_texs[i].erase(m_texs[i].begin() + first * 2, m_texs[i].begin() + last * 2);
        }
    }
    if(m_components[ComponentsBitPos::normal])
        m_norm.erase(m_norm.begin() + first * 3, m_norm.begin() + last * 3);
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

    m_pos.clear();
    for(auto & ch : m_texs)
        ch.clear();
    m_norm.clear();
    m_indices.clear();
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
