

cbuffer DefaultMatrixBuffer : register(b0)
{
    matrix projectionMatrix;
    matrix viewMatrix;
    matrix worldMatrix;
};

struct VertexInputType
{
    float3 position : POSITION;
    float3 color : COLOR;
    float3 normal : NORMAL;
    float2 textureCoordinates : TEXTURECOORDINATES;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 textureCoordinates : TEXTURECOORDINATES;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;

    float4 worldPosition = float4(input.position, 1.0f);
    
    worldPosition = mul(worldPosition, worldMatrix);
    worldPosition = mul(worldPosition, viewMatrix);
    float4 clipPosition = mul(worldPosition, projectionMatrix);
    
    output.position = clipPosition;
    output.color = float4(input.color, 1.0f);
    output.normal = input.normal;
    output.textureCoordinates = input.textureCoordinates;

    return output;
}