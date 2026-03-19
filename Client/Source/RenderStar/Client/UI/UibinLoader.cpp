#include "RenderStar/Client/UI/UibinLoader.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include "RenderStar/Common/Asset/IBinaryAsset.hpp"
#include <nlohmann/json.hpp>
#include <cstring>

namespace RenderStar::Client::UI
{
    namespace
    {
        uint32_t ReadUint32BE(const uint8_t* ptr)
        {
            return (static_cast<uint32_t>(ptr[0]) << 24)
                 | (static_cast<uint32_t>(ptr[1]) << 16)
                 | (static_cast<uint32_t>(ptr[2]) <<  8)
                 |  static_cast<uint32_t>(ptr[3]);
        }

        UIComponentKind ParseKind(const std::string& kind)
        {
            if (kind == "Transform")    return UIComponentKind::Transform;
            if (kind == "Image")        return UIComponentKind::Image;
            if (kind == "Text")         return UIComponentKind::Text;
            if (kind == "Panel")        return UIComponentKind::Panel;
            if (kind == "Button")       return UIComponentKind::Button;
            if (kind == "ProgressBar")  return UIComponentKind::ProgressBar;
            if (kind == "Toggle")       return UIComponentKind::Toggle;
            if (kind == "Dropdown")     return UIComponentKind::Dropdown;
            if (kind == "TextInput")    return UIComponentKind::TextInput;
            if (kind == "Icon")         return UIComponentKind::Icon;
            if (kind == "Sprite")       return UIComponentKind::Sprite;
            if (kind == "Tooltip")      return UIComponentKind::Tooltip;
            if (kind == "Modal")        return UIComponentKind::Modal;
            if (kind == "TabContainer") return UIComponentKind::TabContainer;
            if (kind == "RadialMenu")   return UIComponentKind::RadialMenu;
            if (kind == "Minimap")      return UIComponentKind::Minimap;
            if (kind == "DragSlot")     return UIComponentKind::DragSlot;
            if (kind == "ListRepeater") return UIComponentKind::ListRepeater;
            if (kind == "StackLayout")  return UIComponentKind::StackLayout;
            if (kind == "GridLayout")   return UIComponentKind::GridLayout;
            if (kind == "ScrollBox")    return UIComponentKind::ScrollBox;
            return UIComponentKind::Unknown;
        }

        UibinColor ParseColor(const nlohmann::json& j, const std::string& key, const UibinColor& defaultColor)
        {
            if (j.contains(key) && j[key].is_string())
                return UibinColor::Parse(j[key].get<std::string>());
            return defaultColor;
        }

        template<typename T>
        T GetOr(const nlohmann::json& j, const std::string& key, const T& defaultVal)
        {
            if (j.contains(key))
                return j[key].get<T>();
            return defaultVal;
        }

        bool ParseComponent(const nlohmann::json& j, UIComponent& out)
        {
            if (!j.contains("kind") || !j["kind"].is_string())
                return false;

            out.kind = ParseKind(j["kind"].get<std::string>());

            switch (out.kind)
            {
                case UIComponentKind::Transform:
                {
                    auto& t = out.transform;
                    t.x               = GetOr<double>(j, "x", 0.0);
                    t.y               = GetOr<double>(j, "y", 0.0);
                    t.scaleX          = GetOr<double>(j, "scaleX", 100.0);
                    t.scaleY          = GetOr<double>(j, "scaleY", 100.0);
                    t.rotationDegrees = GetOr<double>(j, "rotationDegrees", 0.0);
                    t.anchors         = GetOr<uint32_t>(j, "anchors", 5);
                    t.stretch         = GetOr<uint32_t>(j, "stretch", 0);
                    break;
                }
                case UIComponentKind::Image:
                {
                    auto& d = out.image;
                    d.imagePath = GetOr<std::string>(j, "imagePath", "");
                    d.tint      = ParseColor(j, "tint", {1, 1, 1, 1});
                    d.assetPath = GetOr<std::string>(j, "assetPath", "");
                    break;
                }
                case UIComponentKind::Text:
                {
                    auto& d = out.text;
                    d.text       = GetOr<std::string>(j, "text", "");
                    d.fontFamily = GetOr<std::string>(j, "fontFamily", "Inter");
                    d.pixelSize  = GetOr<int32_t>(j, "pixelSize", 24);
                    d.color      = ParseColor(j, "color", {1, 1, 1, 1});
                    d.fontPath   = GetOr<std::string>(j, "fontPath", "");
                    d.assetPath  = GetOr<std::string>(j, "assetPath", "");
                    d.alignment  = GetOr<uint32_t>(j, "alignment", 5);
                    break;
                }
                case UIComponentKind::Panel:
                {
                    auto& d = out.panel;
                    d.backgroundColor = ParseColor(j, "backgroundColor", d.backgroundColor);
                    d.borderColor     = ParseColor(j, "borderColor", d.borderColor);
                    d.borderWidth     = GetOr<double>(j, "borderWidth", 1.0);
                    d.cornerRadius    = GetOr<double>(j, "cornerRadius", 6.0);
                    break;
                }
                case UIComponentKind::Button:
                {
                    auto& d = out.button;
                    d.text            = GetOr<std::string>(j, "text", "Button");
                    d.backgroundColor = ParseColor(j, "backgroundColor", d.backgroundColor);
                    d.textColor       = ParseColor(j, "textColor", {1, 1, 1, 1});
                    d.fontFamily      = GetOr<std::string>(j, "fontFamily", "Inter");
                    d.pixelSize       = GetOr<int32_t>(j, "pixelSize", 24);
                    d.fontPath        = GetOr<std::string>(j, "fontPath", "");
                    d.assetPath       = GetOr<std::string>(j, "assetPath", "");
                    d.imagePath       = GetOr<std::string>(j, "imagePath", "");
                    d.sliceLeft       = GetOr<int32_t>(j, "sliceLeft", 6);
                    d.sliceTop        = GetOr<int32_t>(j, "sliceTop", 6);
                    d.sliceRight      = GetOr<int32_t>(j, "sliceRight", 6);
                    d.sliceBottom     = GetOr<int32_t>(j, "sliceBottom", 6);
                    break;
                }
                case UIComponentKind::ProgressBar:
                {
                    auto& d = out.progressBar;
                    d.value           = GetOr<double>(j, "value", 0.5);
                    d.fillColor       = ParseColor(j, "fillColor", d.fillColor);
                    d.backgroundColor = ParseColor(j, "backgroundColor", d.backgroundColor);
                    d.borderColor     = ParseColor(j, "borderColor", d.borderColor);
                    d.direction       = GetOr<int32_t>(j, "direction", 0);
                    d.cornerRadius    = GetOr<double>(j, "cornerRadius", 4.0);
                    break;
                }
                case UIComponentKind::Toggle:
                {
                    auto& d = out.toggle;
                    d.checked   = GetOr<bool>(j, "checked", false);
                    d.onColor   = ParseColor(j, "onColor", d.onColor);
                    d.offColor  = ParseColor(j, "offColor", d.offColor);
                    d.knobColor = ParseColor(j, "knobColor", {1, 1, 1, 1});
                    d.label     = GetOr<std::string>(j, "label", "Toggle");
                    break;
                }
                case UIComponentKind::Dropdown:
                {
                    auto& d = out.dropdown;
                    d.options         = GetOr<std::string>(j, "options", "Option A,Option B");
                    d.selectedIndex   = GetOr<int32_t>(j, "selectedIndex", 0);
                    d.backgroundColor = ParseColor(j, "backgroundColor", d.backgroundColor);
                    d.textColor       = ParseColor(j, "textColor", {1, 1, 1, 1});
                    d.borderColor     = ParseColor(j, "borderColor", d.borderColor);
                    d.fontFamily      = GetOr<std::string>(j, "fontFamily", "Inter");
                    d.pixelSize       = GetOr<int32_t>(j, "pixelSize", 16);
                    break;
                }
                case UIComponentKind::TextInput:
                {
                    auto& d = out.textInput;
                    d.placeholder      = GetOr<std::string>(j, "placeholder", "Enter text...");
                    d.text             = GetOr<std::string>(j, "text", "");
                    d.backgroundColor  = ParseColor(j, "backgroundColor", d.backgroundColor);
                    d.textColor        = ParseColor(j, "textColor", {1, 1, 1, 1});
                    d.placeholderColor = ParseColor(j, "placeholderColor", d.placeholderColor);
                    d.borderColor      = ParseColor(j, "borderColor", d.borderColor);
                    d.fontFamily       = GetOr<std::string>(j, "fontFamily", "Inter");
                    d.pixelSize        = GetOr<int32_t>(j, "pixelSize", 16);
                    break;
                }
                case UIComponentKind::Icon:
                {
                    auto& d = out.icon;
                    d.imagePath = GetOr<std::string>(j, "imagePath", "");
                    d.tintColor = ParseColor(j, "tintColor", {1, 1, 1, 1});
                    d.iconSize  = GetOr<int32_t>(j, "iconSize", 32);
                    d.assetPath = GetOr<std::string>(j, "assetPath", "");
                    break;
                }
                case UIComponentKind::Sprite:
                {
                    auto& d = out.sprite;
                    d.imagePath    = GetOr<std::string>(j, "imagePath", "");
                    d.frameWidth   = GetOr<int32_t>(j, "frameWidth", 64);
                    d.frameHeight  = GetOr<int32_t>(j, "frameHeight", 64);
                    d.frameCount   = GetOr<int32_t>(j, "frameCount", 1);
                    d.currentFrame = GetOr<int32_t>(j, "currentFrame", 0);
                    d.columns      = GetOr<int32_t>(j, "columns", 1);
                    d.assetPath    = GetOr<std::string>(j, "assetPath", "");
                    break;
                }
                case UIComponentKind::Tooltip:
                {
                    auto& d = out.tooltip;
                    d.tooltipText     = GetOr<std::string>(j, "tooltipText", "Tooltip text here");
                    d.backgroundColor = ParseColor(j, "backgroundColor", d.backgroundColor);
                    d.textColor       = ParseColor(j, "textColor", d.textColor);
                    d.borderColor     = ParseColor(j, "borderColor", d.borderColor);
                    d.fontFamily      = GetOr<std::string>(j, "fontFamily", "Inter");
                    d.pixelSize       = GetOr<int32_t>(j, "pixelSize", 13);
                    break;
                }
                case UIComponentKind::Modal:
                {
                    auto& d = out.modal;
                    d.overlayColor = ParseColor(j, "overlayColor", d.overlayColor);
                    d.panelColor   = ParseColor(j, "panelColor", d.panelColor);
                    d.borderColor  = ParseColor(j, "borderColor", d.borderColor);
                    d.cornerRadius = GetOr<double>(j, "cornerRadius", 8.0);
                    d.visible      = GetOr<bool>(j, "visible", true);
                    break;
                }
                case UIComponentKind::TabContainer:
                {
                    auto& d = out.tabContainer;
                    d.tabNames        = GetOr<std::string>(j, "tabNames", "Tab 1,Tab 2,Tab 3");
                    d.activeTab       = GetOr<int32_t>(j, "activeTab", 0);
                    d.tabHeight       = GetOr<int32_t>(j, "tabHeight", 32);
                    d.activeColor     = ParseColor(j, "activeColor", d.activeColor);
                    d.inactiveColor   = ParseColor(j, "inactiveColor", d.inactiveColor);
                    d.textColor       = ParseColor(j, "textColor", {1, 1, 1, 1});
                    d.backgroundColor = ParseColor(j, "backgroundColor", d.backgroundColor);
                    break;
                }
                case UIComponentKind::RadialMenu:
                {
                    auto& d = out.radialMenu;
                    d.sliceCount     = GetOr<int32_t>(j, "sliceCount", 6);
                    d.innerRadius    = GetOr<double>(j, "innerRadius", 40.0);
                    d.outerRadius    = GetOr<double>(j, "outerRadius", 100.0);
                    d.sliceColor     = ParseColor(j, "sliceColor", d.sliceColor);
                    d.borderColor    = ParseColor(j, "borderColor", d.borderColor);
                    d.highlightColor = ParseColor(j, "highlightColor", d.highlightColor);
                    d.highlightIndex = GetOr<int32_t>(j, "highlightIndex", -1);
                    break;
                }
                case UIComponentKind::Minimap:
                {
                    auto& d = out.minimap;
                    d.backgroundColor = ParseColor(j, "backgroundColor", d.backgroundColor);
                    d.borderColor     = ParseColor(j, "borderColor", d.borderColor);
                    d.viewportColor   = ParseColor(j, "viewportColor", d.viewportColor);
                    d.borderWidth     = GetOr<double>(j, "borderWidth", 2.0);
                    d.shape           = GetOr<int32_t>(j, "shape", 0);
                    break;
                }
                case UIComponentKind::DragSlot:
                {
                    auto& d = out.dragSlot;
                    d.slotSize        = GetOr<int32_t>(j, "slotSize", 64);
                    d.backgroundColor = ParseColor(j, "backgroundColor", d.backgroundColor);
                    d.borderColor     = ParseColor(j, "borderColor", d.borderColor);
                    d.emptyColor      = ParseColor(j, "emptyColor", d.emptyColor);
                    d.cornerRadius    = GetOr<double>(j, "cornerRadius", 4.0);
                    d.isEmpty         = GetOr<bool>(j, "isEmpty", true);
                    d.iconPath        = GetOr<std::string>(j, "iconPath", "");
                    d.assetPath       = GetOr<std::string>(j, "assetPath", "");
                    break;
                }
                case UIComponentKind::ListRepeater:
                {
                    auto& d = out.listRepeater;
                    d.itemCount      = GetOr<int32_t>(j, "itemCount", 5);
                    d.itemHeight     = GetOr<int32_t>(j, "itemHeight", 40);
                    d.spacing        = GetOr<double>(j, "spacing", 2.0);
                    d.direction      = GetOr<int32_t>(j, "direction", 0);
                    d.itemColor      = ParseColor(j, "itemColor", d.itemColor);
                    d.alternateColor = ParseColor(j, "alternateColor", d.alternateColor);
                    d.borderColor    = ParseColor(j, "borderColor", d.borderColor);
                    break;
                }
                case UIComponentKind::StackLayout:
                {
                    auto& d = out.stackLayout;
                    d.direction = GetOr<int32_t>(j, "direction", 0);
                    d.spacing   = GetOr<double>(j, "spacing", 8.0);
                    d.padding   = GetOr<double>(j, "padding", 8.0);
                    break;
                }
                case UIComponentKind::GridLayout:
                {
                    auto& d = out.gridLayout;
                    d.columns  = GetOr<int32_t>(j, "columns", 2);
                    d.spacingH = GetOr<double>(j, "spacingH", 8.0);
                    d.spacingV = GetOr<double>(j, "spacingV", 8.0);
                    d.padding  = GetOr<double>(j, "padding", 8.0);
                    break;
                }
                case UIComponentKind::ScrollBox:
                {
                    auto& d = out.scrollBox;
                    d.direction = GetOr<int32_t>(j, "direction", 0);
                    d.spacing   = GetOr<double>(j, "spacing", 8.0);
                    d.padding   = GetOr<double>(j, "padding", 8.0);
                    break;
                }
                case UIComponentKind::Unknown:
                    break;
            }

            return true;
        }

        bool ParseElement(const nlohmann::json& j, UIElement& out)
        {
            out.id   = GetOr<std::string>(j, "id", "");
            out.name = GetOr<std::string>(j, "name", "");

            if (j.contains("components") && j["components"].is_array())
            {
                for (const auto& compJson : j["components"])
                {
                    UIComponent comp;

                    if (ParseComponent(compJson, comp))
                        out.components.push_back(std::move(comp));
                }
            }

            if (j.contains("children") && j["children"].is_array())
            {
                for (const auto& childJson : j["children"])
                {
                    UIElement child;

                    if (ParseElement(childJson, child))
                        out.children.push_back(std::move(child));
                }
            }

            return true;
        }
    }

    std::optional<UibinScene> UibinLoader::Load(const Common::Asset::AssetLocation& location, Common::Asset::AssetModule& assetModule)
    {
        auto binaryAsset = assetModule.LoadBinary(location);

        if (!binaryAsset.IsValid())
            return std::nullopt;

        return Parse(binaryAsset.Get()->GetDataView());
    }

    std::optional<UibinScene> UibinLoader::Parse(std::span<const uint8_t> data)
    {
        if (data.size() < HEADER_SIZE)
            return std::nullopt;

        const uint8_t* ptr = data.data();
        const uint8_t* end = ptr + data.size();

        uint32_t magic = ReadUint32BE(ptr);

        if (magic != MAGIC)
            return std::nullopt;

        uint32_t version   = ReadUint32BE(ptr + 4);
        uint32_t jsonLen   = ReadUint32BE(ptr + 8);
        uint32_t assetCount = ReadUint32BE(ptr + 12);
        ptr += HEADER_SIZE;

        if (version != VERSION)
            return std::nullopt;

        if (ptr + jsonLen > end)
            return std::nullopt;

        std::string jsonStr(reinterpret_cast<const char*>(ptr), jsonLen);
        ptr += jsonLen;

        nlohmann::json rootJson;

        try
        {
            rootJson = nlohmann::json::parse(jsonStr);
        }
        catch (const nlohmann::json::parse_error&)
        {
            return std::nullopt;
        }

        UibinScene scene;

        if (!ParseElement(rootJson, scene.root))
            return std::nullopt;

        if (rootJson.contains("designWidth"))
            scene.designWidth = rootJson["designWidth"].get<double>();
        if (rootJson.contains("designHeight"))
            scene.designHeight = rootJson["designHeight"].get<double>();

        for (uint32_t i = 0; i < assetCount; ++i)
        {
            if (ptr + 4 > end) return std::nullopt;
            uint32_t keyLen = ReadUint32BE(ptr);
            ptr += 4;

            if (ptr + keyLen > end) return std::nullopt;
            std::string key(reinterpret_cast<const char*>(ptr), keyLen);
            ptr += keyLen;

            if (ptr + 4 > end) return std::nullopt;
            uint32_t dataLen = ReadUint32BE(ptr);
            ptr += 4;

            if (ptr + dataLen > end) return std::nullopt;
            scene.assets[key] = std::vector<uint8_t>(ptr, ptr + dataLen);
            ptr += dataLen;
        }

        return scene;
    }
}
