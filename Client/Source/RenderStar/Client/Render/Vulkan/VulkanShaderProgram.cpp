#include "RenderStar/Client/Render/Vulkan/VulkanShaderProgram.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanDescriptorModule.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanShaderProgram::VulkanShaderProgram()
        : logger(spdlog::default_logger())
        , device(VK_NULL_HANDLE)
        , shaderModule(nullptr)
        , descriptorModule(nullptr)
        , vertexShader{}
        , fragmentShader{}
        , computeShader{}
        , pipeline(VK_NULL_HANDLE)
        , pipelineLayout(VK_NULL_HANDLE)
        , descriptorSetLayout(VK_NULL_HANDLE)
        , isCompute(false)
        , valid(false)
    {
    }

    VulkanShaderProgram::~VulkanShaderProgram()
    {
        DestroyPipeline();

        if (shaderModule)
        {
            if (vertexShader.module != VK_NULL_HANDLE)
                shaderModule->DestroyShader(vertexShader);

            if (fragmentShader.module != VK_NULL_HANDLE)
                shaderModule->DestroyShader(fragmentShader);

            if (computeShader.module != VK_NULL_HANDLE)
                shaderModule->DestroyShader(computeShader);
        }
    }

    void VulkanShaderProgram::Initialize(
        VkDevice vulkanDevice,
        VkRenderPass renderPass,
        VulkanShaderModule* module,
        VulkanDescriptorModule* descModule,
        VulkanShader vertex,
        VulkanShader fragment,
        const VertexLayout& vertexLayout)
    {
        device = vulkanDevice;
        shaderModule = module;
        descriptorModule = descModule;
        vertexShader = vertex;
        fragmentShader = fragment;
        isCompute = false;

        BuildPipeline(renderPass, vertexLayout);
        valid = (pipeline != VK_NULL_HANDLE);
    }

    void VulkanShaderProgram::InitializeCompute(
        VulkanShaderModule* module,
        VulkanShader compute)
    {
        shaderModule = module;
        computeShader = compute;
        isCompute = true;
        valid = true;
    }

    void VulkanShaderProgram::BuildPipeline(VkRenderPass renderPass, const VertexLayout& vertexLayout)
    {
        descriptorSetLayout = descriptorModule->GetOrCreateMVPLayout();

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = GetShaderStages();

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = static_cast<uint32_t>(vertexLayout.stride);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.reserve(vertexLayout.attributes.size());

        for (const auto& attribute : vertexLayout.attributes)
        {
            VkVertexInputAttributeDescription desc{};
            desc.binding = 0;
            desc.location = attribute.location;
            desc.format = GetVulkanFormat(attribute.type);
            desc.offset = static_cast<uint32_t>(attribute.offset);
            attributeDescriptions.push_back(desc);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create pipeline layout: {}", static_cast<int>(result));
            return;
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create graphics pipeline: {}", static_cast<int>(result));
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
            return;
        }

        logger->info("Created Vulkan graphics pipeline");
    }

    void VulkanShaderProgram::DestroyPipeline()
    {
        if (device == VK_NULL_HANDLE)
            return;

        if (pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(device, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }

        if (pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }
    }

    VkFormat VulkanShaderProgram::GetVulkanFormat(VertexAttributeType type) const
    {
        switch (type)
        {
            case VertexAttributeType::FLOAT:  return VK_FORMAT_R32_SFLOAT;
            case VertexAttributeType::FLOAT2: return VK_FORMAT_R32G32_SFLOAT;
            case VertexAttributeType::FLOAT3: return VK_FORMAT_R32G32B32_SFLOAT;
            case VertexAttributeType::FLOAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
            case VertexAttributeType::INT:    return VK_FORMAT_R32_SINT;
            case VertexAttributeType::INT2:   return VK_FORMAT_R32G32_SINT;
            case VertexAttributeType::INT3:   return VK_FORMAT_R32G32B32_SINT;
            case VertexAttributeType::INT4:   return VK_FORMAT_R32G32B32A32_SINT;
            case VertexAttributeType::UINT:   return VK_FORMAT_R32_UINT;
            case VertexAttributeType::UINT2:  return VK_FORMAT_R32G32_UINT;
            case VertexAttributeType::UINT3:  return VK_FORMAT_R32G32B32_UINT;
            case VertexAttributeType::UINT4:  return VK_FORMAT_R32G32B32A32_UINT;
        }
        return VK_FORMAT_R32G32B32_SFLOAT;
    }

    void VulkanShaderProgram::Bind()
    {
    }

    void VulkanShaderProgram::Unbind()
    {
    }

    void VulkanShaderProgram::BindPipeline(VkCommandBuffer commandBuffer)
    {
        if (pipeline != VK_NULL_HANDLE)
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    void VulkanShaderProgram::BindDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet)
    {
        if (pipelineLayout != VK_NULL_HANDLE && descriptorSet != VK_NULL_HANDLE)
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    }

    bool VulkanShaderProgram::IsValid() const
    {
        return valid;
    }

    void VulkanShaderProgram::SetUniform(const std::string& name, int32_t value)
    {
        (void)name;
        (void)value;
    }

    void VulkanShaderProgram::SetUniform(const std::string& name, float value)
    {
        (void)name;
        (void)value;
    }

    void VulkanShaderProgram::SetUniform(const std::string& name, const glm::vec2& value)
    {
        (void)name;
        (void)value;
    }

    void VulkanShaderProgram::SetUniform(const std::string& name, const glm::vec3& value)
    {
        (void)name;
        (void)value;
    }

    void VulkanShaderProgram::SetUniform(const std::string& name, const glm::vec4& value)
    {
        (void)name;
        (void)value;
    }

    void VulkanShaderProgram::SetUniform(const std::string& name, const glm::mat3& value)
    {
        (void)name;
        (void)value;
    }

    void VulkanShaderProgram::SetUniform(const std::string& name, const glm::mat4& value)
    {
        (void)name;
        (void)value;
    }

    int32_t VulkanShaderProgram::GetUniformLocation(const std::string& name) const
    {
        (void)name;
        return -1;
    }

    const VulkanShader& VulkanShaderProgram::GetVertexShader() const
    {
        return vertexShader;
    }

    const VulkanShader& VulkanShaderProgram::GetFragmentShader() const
    {
        return fragmentShader;
    }

    const VulkanShader& VulkanShaderProgram::GetComputeShader() const
    {
        return computeShader;
    }

    std::vector<VkPipelineShaderStageCreateInfo> VulkanShaderProgram::GetShaderStages() const
    {
        std::vector<VkPipelineShaderStageCreateInfo> stages;

        if (isCompute)
        {
            stages.push_back(shaderModule->GetShaderStageInfo(computeShader));
        }
        else
        {
            stages.push_back(shaderModule->GetShaderStageInfo(vertexShader));
            stages.push_back(shaderModule->GetShaderStageInfo(fragmentShader));
        }

        return stages;
    }

    VkPipeline VulkanShaderProgram::GetPipeline() const
    {
        return pipeline;
    }

    VkPipelineLayout VulkanShaderProgram::GetPipelineLayout() const
    {
        return pipelineLayout;
    }

    VkDescriptorSetLayout VulkanShaderProgram::GetDescriptorSetLayout() const
    {
        return descriptorSetLayout;
    }
}
