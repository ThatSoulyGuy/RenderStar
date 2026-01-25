#include "RenderStar/Client/Render/Vulkan/VulkanShaderModule.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <filesystem>
#include <array>

#ifdef _WIN32
#include <windows.h>
#endif

namespace RenderStar::Client::Render::Vulkan
{
    namespace
    {
        int ExecuteCommandWithStatus(const std::string& command, std::string& output)
        {
#ifdef _WIN32
            std::array<char, 4096> buffer;
            output.clear();

            FILE* pipe = _popen((command + " 2>&1").c_str(), "r");
            if (!pipe)
                return -1;

            while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
                output += buffer.data();

            return _pclose(pipe);
#else
            std::array<char, 4096> buffer;
            output.clear();

            FILE* pipe = popen((command + " 2>&1").c_str(), "r");
            if (!pipe)
                return -1;

            while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
                output += buffer.data();

            int status = pclose(pipe);
            return WEXITSTATUS(status);
#endif
        }
    }

    VulkanShaderModule::VulkanShaderModule() : logger(spdlog::default_logger()->clone("VulkanShaderModule")), device(VK_NULL_HANDLE) { }

    VulkanShaderModule::~VulkanShaderModule()
    {
        Destroy();
    }

    void VulkanShaderModule::Create(VkDevice deviceHandle)
    {
        device = deviceHandle;
        logger->info("Vulkan shader module created");
    }

    void VulkanShaderModule::Destroy()
    {
        if (device == VK_NULL_HANDLE)
            return;

        device = VK_NULL_HANDLE;
        logger->info("Vulkan shader module destroyed");
    }

    VulkanShader VulkanShaderModule::LoadShaderFromSpirv(const std::vector<uint32_t>& spirvCode, const VulkanShaderStage stage) const
    {
        VulkanShader shader{};
        shader.stage = stage;

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
        createInfo.pCode = spirvCode.data();

        if (const VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shader.module); result != VK_SUCCESS)
        {
            logger->error("Failed to create shader module from SPIR-V");
            throw std::runtime_error("Failed to create shader module");
        }

        return shader;
    }

    VulkanShader VulkanShaderModule::LoadShaderFromGlsl(const std::string& glslSource, const VulkanShaderStage stage, const std::string& filename)
    {
        std::vector<uint32_t> spirv = CompileGlslToSpirv(glslSource, stage, filename);
        return LoadShaderFromSpirv(spirv, stage);
    }

    VulkanShader VulkanShaderModule::LoadShaderFromFile(const std::string& filePath, VulkanShaderStage stage)
    {
        std::ifstream file(filePath);

        if (!file.is_open())
        {
            logger->error("Failed to open shader file: {}", filePath);
            throw std::runtime_error("Failed to open shader file: " + filePath);
        }

        std::stringstream buffer;

        buffer << file.rdbuf();

        std::string glslSource = buffer.str();

        file.close();

        logger->info("Loaded GLSL shader from file: {}", filePath);

        return LoadShaderFromGlsl(glslSource, stage, filePath);
    }

    std::vector<uint32_t> VulkanShaderModule::CompileGlslToSpirv(const std::string& glslSource, VulkanShaderStage stage, const std::string& filename) const
    {
        std::filesystem::path tempDir = std::filesystem::temp_directory_path();
        std::filesystem::path inputPath = tempDir / ("shader_input_" + std::to_string(std::hash<std::string>{}(filename)) + ".glsl");
        std::filesystem::path outputPath = tempDir / ("shader_output_" + std::to_string(std::hash<std::string>{}(filename)) + ".spv");

        {
            std::ofstream inputFile(inputPath);

            if (!inputFile.is_open())
            {
                logger->error("Failed to create temporary shader file: {}", inputPath.string());
                throw std::runtime_error("Failed to create temporary shader file");
            }

            inputFile << glslSource;
        }

        std::string stageFlag;

        switch (stage)
        {
            case VulkanShaderStage::VERTEX:
                stageFlag = "-fshader-stage=vertex";
                break;
            case VulkanShaderStage::FRAGMENT:
                stageFlag = "-fshader-stage=fragment";
                break;
            case VulkanShaderStage::COMPUTE:
                stageFlag = "-fshader-stage=compute";
                break;
        }

        std::string command = "glslc " + stageFlag + " --target-env=vulkan1.3 -O \"" + inputPath.string() + "\" -o \"" + outputPath.string() + "\"";

        std::string compileOutput;
        int result = ExecuteCommandWithStatus(command, compileOutput);

        std::filesystem::remove(inputPath);

        if (result != 0)
        {
            std::filesystem::remove(outputPath);

            logger->error("Shader compilation failed for {}: {}", filename, compileOutput);

            throw std::runtime_error("Shader compilation failed: " + compileOutput);
        }

        std::ifstream spirvFile(outputPath, std::ios::ate | std::ios::binary);

        if (!spirvFile.is_open())
        {
            logger->error("Failed to read compiled shader: {}", outputPath.string());
            throw std::runtime_error("Failed to read compiled shader");
        }

        size_t fileSize = spirvFile.tellg();
        std::vector<uint32_t> spirvCode(fileSize / sizeof(uint32_t));

        spirvFile.seekg(0);
        spirvFile.read(reinterpret_cast<char*>(spirvCode.data()), static_cast<long long>(fileSize));
        spirvFile.close();

        std::filesystem::remove(outputPath);

        logger->info("Compiled GLSL to SPIR-V using glslc: {}", filename);

        return spirvCode;
    }

    void VulkanShaderModule::DestroyShader(VulkanShader& shader) const
    {
        if (shader.module != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(device, shader.module, nullptr);
            shader.module = VK_NULL_HANDLE;
        }
    }

    VkPipelineShaderStageCreateInfo VulkanShaderModule::GetShaderStageInfo(const VulkanShader& shader)
    {
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = GetVkShaderStage(shader.stage);
        stageInfo.module = shader.module;
        stageInfo.pName = "main";

        return stageInfo;
    }

    VkShaderStageFlagBits VulkanShaderModule::GetVkShaderStage(const VulkanShaderStage stage)
    {
        switch (stage)
        {
            case VulkanShaderStage::VERTEX:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case VulkanShaderStage::FRAGMENT:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case VulkanShaderStage::COMPUTE:
                return VK_SHADER_STAGE_COMPUTE_BIT;
        }

        return VK_SHADER_STAGE_VERTEX_BIT;
    }
}
