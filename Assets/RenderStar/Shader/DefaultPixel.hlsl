
struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 textureCoordinates : TEXTURECOORDINATES;
};

float4 Main(PixelInputType input) : SV_TARGET
{
    return input.color;
}