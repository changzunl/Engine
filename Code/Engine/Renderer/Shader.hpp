#include <vector>
#include <string>

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
class VertexFormat;

struct ShaderConfig
{
	std::string m_name;
	std::string m_vertexEntryPoint = "VertexMain";
	std::string m_pixelEntryPoint  = "PixelMain";
	std::vector<VertexFormat> m_formats;
};

class Shader
{
	friend class Renderer;

private:
	Shader(const ShaderConfig& config);
	Shader(const Shader& copy) = delete;
	~Shader();

public:
	const std::string& GetName() const;
	int GetInputFormatSize() const;
	const VertexFormat& GetInputFormat(int index) const;

private:
	ShaderConfig		m_config;
	ID3D11VertexShader* m_vertexShader             = nullptr;
	ID3D11PixelShader*  m_pixelShader              = nullptr;
	ID3D11InputLayout*  m_inputLayout              = nullptr;
};
