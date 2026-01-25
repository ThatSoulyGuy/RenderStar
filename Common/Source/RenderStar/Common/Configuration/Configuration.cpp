#include "RenderStar/Common/Configuration/Configuration.hpp"
#include <sstream>
#include <utility>

namespace RenderStar::Common::Configuration
{
    Configuration::Configuration(std::string configNamespace, std::string classScope, std::shared_ptr<pugi::xml_document> sharedDocument, std::filesystem::path sourceFilePath) : logger(spdlog::default_logger()), configNamespace(std::move(configNamespace)), classScope(std::move(classScope)), document(std::move(sharedDocument)), filePath(std::move(sourceFilePath))
    {
        InitializeScopedElement();
    }

    Configuration::Configuration(std::string configNamespace, std::string classScope, std::shared_ptr<pugi::xml_document> sharedDocument) : logger(spdlog::default_logger()), configNamespace(std::move(configNamespace)), classScope(std::move(classScope)), document(std::move(sharedDocument))
    {
        InitializeScopedElement();
    }

    void Configuration::InitializeScopedElement()
    {
        pugi::xml_node root = document->document_element();

        if (root.empty())
        {
            logger->error("Configuration document has no root element");
            return;
        }

        if (std::string(root.name()) != configNamespace)
        {
            logger->error("Root element '{}' does not match namespace '{}'", root.name(), configNamespace);
            return;
        }

        scopedElement = root.child(classScope.c_str());

        if (scopedElement.empty())
        {
            logger->debug("No configuration found for class scope '{}', creating section", classScope);
            scopedElement = root.append_child(classScope.c_str());
        }
    }

    std::optional<std::string> Configuration::GetString(const std::string& path) const
    {
        const auto node = NavigateToNode(path);

        if (node.empty())
            return std::nullopt;

        std::string content = node.text().as_string();

        if (content.size() >= 2 && content.front() == '"' && content.back() == '"')
            content = content.substr(1, content.size() - 2);

        return content;
    }

    std::optional<int32_t> Configuration::GetInteger(const std::string& path) const
    {
        const auto node = NavigateToNode(path);

        if (node.empty())
            return std::nullopt;

        return node.text().as_int();
    }

    std::optional<float> Configuration::GetFloat(const std::string& path) const
    {
        const auto node = NavigateToNode(path);

        if (node.empty())
            return std::nullopt;

        return node.text().as_float();
    }

    std::optional<bool> Configuration::GetBoolean(const std::string& path) const
    {
        const auto node = NavigateToNode(path);

        if (node.empty())
            return std::nullopt;

        std::string value = node.text().as_string();
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        return value == "true" || value == "1" || value == "yes";
    }

    std::vector<std::string> Configuration::GetStringList(const std::string& path) const
    {
        std::vector<std::string> result;
        const auto node = NavigateToNode(path);

        if (node.empty())
            return result;

        std::string content = node.text().as_string();
        std::istringstream stream(content);
        std::string item;

        while (std::getline(stream, item, ','))
        {
            const size_t start = item.find_first_not_of(" \t");

            if (const size_t end = item.find_last_not_of(" \t"); start != std::string::npos && end != std::string::npos)
                result.push_back(item.substr(start, end - start + 1));
        }

        return result;
    }

    void Configuration::SetString(const std::string& path, const std::string& value)
    {
        const auto node = NavigateOrCreateNode(path);
        node.text().set(value.c_str());
    }

    void Configuration::SetInteger(const std::string& path, int32_t value)
    {
        const auto node = NavigateOrCreateNode(path);
        node.text().set(value);
    }

    void Configuration::SetFloat(const std::string& path, float value)
    {
        const auto node = NavigateOrCreateNode(path);
        node.text().set(value);
    }

    void Configuration::SetBoolean(const std::string& path, bool value)
    {
        const auto node = NavigateOrCreateNode(path);
        node.text().set(value ? "true" : "false");
    }

    void Configuration::Save()
    {
        if (filePath.empty())
        {
            logger->warn("Cannot save configuration loaded from stream");
            return;
        }

        (void)document->save_file(filePath.c_str(), "    ");

        logger->debug("Configuration saved to {}", filePath.string());
    }

    void Configuration::Reload()
    {
        if (filePath.empty())
        {
            logger->warn("Cannot reload configuration loaded from stream");
            return;
        }

        document->load_file(filePath.c_str());
        InitializeScopedElement();
    }

    const std::string& Configuration::GetNamespace() const
    {
        return configNamespace;
    }

    const std::string& Configuration::GetClassScope() const
    {
        return classScope;
    }

    pugi::xml_node Configuration::NavigateToNode(const std::string& path) const
    {
        if (scopedElement.empty())
            return {};

        pugi::xml_node current = scopedElement;
        std::istringstream stream(path);
        std::string segment;

        while (std::getline(stream, segment, '.'))
        {
            current = current.child(segment.c_str());

            if (current.empty())
                return {};
        }

        return current;
    }

    pugi::xml_node Configuration::NavigateOrCreateNode(const std::string& path) const
    {
        if (scopedElement.empty())
        {
            logger->error("Cannot create node, configuration not loaded");
            return {};
        }

        pugi::xml_node current = scopedElement;
        std::istringstream stream(path);
        std::string segment;

        while (std::getline(stream, segment, '.'))
        {
            auto child = current.child(segment.c_str());

            if (child.empty())
                child = current.append_child(segment.c_str());

            current = child;
        }

        return current;
    }
}
