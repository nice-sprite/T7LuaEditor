/*Render graph
 * - RenderPass: 
 *   - setup_pass(graph_builder_o* ) // define named resources to crw 
 *   - execute_pass()
 * */
#ifndef RENDER_GRAPH_H
#define RENDER_GRAPH_H
#include <d3d11.h>

struct RenderPassResources
{
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* constantBuffer;
    ID3D11ShaderResourceView* textureAtlas;
    ID3D11VertexShader* vertexShader;
    ID3D11InputLayout* vertexInputLayout;
    ID3D11PixelShader* pixelShader;
};

struct PassDependencies
{
    wchar_t* vertexShaderPath;
    wchar_t* pixelShaderPath;
    D3D11_INPUT_ELEMENT_DESC *il;
    UINT ilSize;
    size_t constantBufferSize, vertexBufferSize, indexBufferSize, atlasSlot;
};


constexpr size_t PassCount = 4;
struct RenderGraph 
{
    RenderPassResources resourcesPerPass[PassCount]; // if you need more passes just increase passes!
    char* pass[PassCount];
    RenderPassResources& linear_search_get_pass_by_name(char* passname);
    RenderPassResources& get_pass_resources(size_t passIndex);
};


#endif
