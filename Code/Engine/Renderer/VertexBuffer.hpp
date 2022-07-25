struct ID3D11Buffer;

class VertexBuffer
{
    friend class Renderer;

public:
    VertexBuffer(size_t size);
    VertexBuffer(const VertexBuffer& copy) = delete;
    virtual ~VertexBuffer();

    unsigned int GetStride() const;

    ID3D11Buffer*    m_buffer    = nullptr;
    size_t           m_size      = 0;
    int              m_stride    = 0;
};
