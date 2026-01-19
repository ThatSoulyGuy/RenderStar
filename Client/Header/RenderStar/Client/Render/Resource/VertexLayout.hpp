#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace RenderStar::Client::Render
{
    enum class VertexAttributeType
    {
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        INT,
        INT2,
        INT3,
        INT4,
        UINT,
        UINT2,
        UINT3,
        UINT4
    };

    struct VertexAttribute
    {
        uint32_t location;
        VertexAttributeType type;
        bool normalized;
        size_t offset;

        static constexpr int32_t GetComponentCount(VertexAttributeType type)
        {
            switch (type)
            {
                case VertexAttributeType::FLOAT:
                case VertexAttributeType::INT:
                case VertexAttributeType::UINT:
                    return 1;
                case VertexAttributeType::FLOAT2:
                case VertexAttributeType::INT2:
                case VertexAttributeType::UINT2:
                    return 2;
                case VertexAttributeType::FLOAT3:
                case VertexAttributeType::INT3:
                case VertexAttributeType::UINT3:
                    return 3;
                case VertexAttributeType::FLOAT4:
                case VertexAttributeType::INT4:
                case VertexAttributeType::UINT4:
                    return 4;
            }
            return 0;
        }

        static constexpr size_t GetSize(VertexAttributeType type)
        {
            switch (type)
            {
                case VertexAttributeType::FLOAT:  return sizeof(float);
                case VertexAttributeType::FLOAT2: return sizeof(float) * 2;
                case VertexAttributeType::FLOAT3: return sizeof(float) * 3;
                case VertexAttributeType::FLOAT4: return sizeof(float) * 4;
                case VertexAttributeType::INT:    return sizeof(int32_t);
                case VertexAttributeType::INT2:   return sizeof(int32_t) * 2;
                case VertexAttributeType::INT3:   return sizeof(int32_t) * 3;
                case VertexAttributeType::INT4:   return sizeof(int32_t) * 4;
                case VertexAttributeType::UINT:   return sizeof(uint32_t);
                case VertexAttributeType::UINT2:  return sizeof(uint32_t) * 2;
                case VertexAttributeType::UINT3:  return sizeof(uint32_t) * 3;
                case VertexAttributeType::UINT4:  return sizeof(uint32_t) * 4;
            }
            return 0;
        }
    };

    struct VertexLayout
    {
        std::vector<VertexAttribute> attributes;
        int32_t stride;

        static VertexLayout PositionNormalUV()
        {
            return VertexLayout{
                {
                    { 0, VertexAttributeType::FLOAT3, false, 0 },
                    { 1, VertexAttributeType::FLOAT3, false, sizeof(float) * 3 },
                    { 2, VertexAttributeType::FLOAT2, false, sizeof(float) * 6 }
                },
                static_cast<int32_t>(sizeof(float) * 8)
            };
        }

        static VertexLayout PositionColor()
        {
            return VertexLayout{
                {
                    { 0, VertexAttributeType::FLOAT3, false, 0 },
                    { 1, VertexAttributeType::FLOAT4, false, sizeof(float) * 3 }
                },
                static_cast<int32_t>(sizeof(float) * 7)
            };
        }

        static VertexLayout PositionOnly()
        {
            return VertexLayout{
                {
                    { 0, VertexAttributeType::FLOAT3, false, 0 }
                },
                static_cast<int32_t>(sizeof(float) * 3)
            };
        }
    };
}
