#pragma once

#include "RenderStar/Client/UI/UibinTypes.hpp"
#include <string>
#include <cstdint>

namespace RenderStar::Client::UI
{
    struct TransformData
    {
        double x = 0.0, y = 0.0;
        double scaleX = 100.0, scaleY = 100.0;
        double rotationDegrees = 0.0;
        uint32_t anchors = 5;
        uint32_t stretch = 0;
    };

    struct ImageData
    {
        std::string imagePath;
        UibinColor tint = {1, 1, 1, 1};
        std::string assetPath;
    };

    struct TextData
    {
        std::string text;
        std::string fontFamily = "Inter";
        int32_t pixelSize = 24;
        UibinColor color = {1, 1, 1, 1};
        std::string fontPath;
        std::string assetPath;
        uint32_t alignment = 5;
    };

    struct PanelData
    {
        UibinColor backgroundColor = UibinColor::Parse("#C8323237");
        UibinColor borderColor     = UibinColor::Parse("#FF64646E");
        double borderWidth   = 1.0;
        double cornerRadius  = 6.0;
    };

    struct ButtonData
    {
        std::string text = "Button";
        UibinColor backgroundColor = UibinColor::Parse("#FF282828");
        UibinColor textColor       = {1, 1, 1, 1};
        std::string fontFamily = "Inter";
        int32_t pixelSize = 24;
        std::string fontPath;
        std::string assetPath;
        std::string imagePath;
        int32_t sliceLeft = 6, sliceTop = 6, sliceRight = 6, sliceBottom = 6;
    };

    struct ProgressBarData
    {
        double value = 0.5;
        UibinColor fillColor       = UibinColor::Parse("#FF32C850");
        UibinColor backgroundColor = UibinColor::Parse("#FF28282D");
        UibinColor borderColor     = UibinColor::Parse("#FF50505A");
        int32_t direction   = 0;
        double cornerRadius = 4.0;
    };

    struct ToggleData
    {
        bool checked = false;
        UibinColor onColor   = UibinColor::Parse("#FF32C850");
        UibinColor offColor  = UibinColor::Parse("#FF50505A");
        UibinColor knobColor = {1, 1, 1, 1};
        std::string label = "Toggle";
    };

    struct DropdownData
    {
        std::string options = "Option A,Option B";
        int32_t selectedIndex = 0;
        UibinColor backgroundColor = UibinColor::Parse("#FF2D2D32");
        UibinColor textColor       = {1, 1, 1, 1};
        UibinColor borderColor     = UibinColor::Parse("#FF5A5A64");
        std::string fontFamily = "Inter";
        int32_t pixelSize = 16;
    };

    struct TextInputData
    {
        std::string placeholder = "Enter text...";
        std::string text;
        UibinColor backgroundColor  = UibinColor::Parse("#FF1E1E23");
        UibinColor textColor        = {1, 1, 1, 1};
        UibinColor placeholderColor = UibinColor::Parse("#FF787882");
        UibinColor borderColor      = UibinColor::Parse("#FF50505A");
        std::string fontFamily = "Inter";
        int32_t pixelSize = 16;
    };

    struct IconData
    {
        std::string imagePath;
        UibinColor tintColor = {1, 1, 1, 1};
        int32_t iconSize = 32;
        std::string assetPath;
    };

    struct SpriteData
    {
        std::string imagePath;
        int32_t frameWidth = 64, frameHeight = 64;
        int32_t frameCount = 1, currentFrame = 0, columns = 1;
        std::string assetPath;
    };

    struct TooltipData
    {
        std::string tooltipText = "Tooltip text here";
        UibinColor backgroundColor = UibinColor::Parse("#E619191E");
        UibinColor textColor       = UibinColor::Parse("#FFDCDCE1");
        UibinColor borderColor     = UibinColor::Parse("#FF50505A");
        std::string fontFamily = "Inter";
        int32_t pixelSize = 13;
    };

    struct ModalData
    {
        UibinColor overlayColor = UibinColor::Parse("#8C000000");
        UibinColor panelColor   = UibinColor::Parse("#FF2D2D32");
        UibinColor borderColor  = UibinColor::Parse("#FF5A5A64");
        double cornerRadius = 8.0;
        bool visible = true;
    };

    struct TabContainerData
    {
        std::string tabNames = "Tab 1,Tab 2,Tab 3";
        int32_t activeTab = 0;
        int32_t tabHeight = 32;
        UibinColor activeColor   = UibinColor::Parse("#FF3C3C44");
        UibinColor inactiveColor = UibinColor::Parse("#FF28282D");
        UibinColor textColor     = {1, 1, 1, 1};
        UibinColor backgroundColor = UibinColor::Parse("#FF323237");
    };

    struct RadialMenuData
    {
        int32_t sliceCount = 6;
        double innerRadius = 40.0, outerRadius = 100.0;
        UibinColor sliceColor     = UibinColor::Parse("#C832323A");
        UibinColor borderColor    = UibinColor::Parse("#FF646473");
        UibinColor highlightColor = UibinColor::Parse("#6450A0FF");
        int32_t highlightIndex = -1;
    };

    struct MinimapData
    {
        UibinColor backgroundColor = UibinColor::Parse("#C8141E14");
        UibinColor borderColor     = UibinColor::Parse("#FF647864");
        UibinColor viewportColor   = UibinColor::Parse("#3CFFFFFF");
        double borderWidth = 2.0;
        int32_t shape = 0;
    };

    struct DragSlotData
    {
        int32_t slotSize = 64;
        UibinColor backgroundColor = UibinColor::Parse("#FF282830");
        UibinColor borderColor     = UibinColor::Parse("#FF50505F");
        UibinColor emptyColor      = UibinColor::Parse("#78373741");
        double cornerRadius = 4.0;
        bool isEmpty = true;
        std::string iconPath;
        std::string assetPath;
    };

    struct ListRepeaterData
    {
        int32_t itemCount = 5;
        int32_t itemHeight = 40;
        double spacing = 2.0;
        int32_t direction = 0;
        UibinColor itemColor      = UibinColor::Parse("#FF2D2D34");
        UibinColor alternateColor = UibinColor::Parse("#FF32323A");
        UibinColor borderColor    = UibinColor::Parse("#FF464650");
    };

    struct StackLayoutData
    {
        int32_t direction = 0;
        double spacing = 8.0;
        double padding = 8.0;
    };

    struct GridLayoutData
    {
        int32_t columns = 2;
        double spacingH = 8.0, spacingV = 8.0;
        double padding = 8.0;
    };

    struct ScrollBoxData
    {
        int32_t direction = 0;
        double spacing = 8.0;
        double padding = 8.0;
    };

    enum class UIComponentKind
    {
        Transform, Image, Text, Panel, Button, ProgressBar,
        Toggle, Dropdown, TextInput, Icon, Sprite,
        Tooltip, Modal, TabContainer, RadialMenu, Minimap,
        DragSlot, ListRepeater,
        StackLayout, GridLayout, ScrollBox,
        Unknown
    };

    struct UIComponent
    {
        UIComponentKind kind = UIComponentKind::Unknown;

        TransformData    transform;
        ImageData        image;
        TextData         text;
        PanelData        panel;
        ButtonData       button;
        ProgressBarData  progressBar;
        ToggleData       toggle;
        DropdownData     dropdown;
        TextInputData    textInput;
        IconData         icon;
        SpriteData       sprite;
        TooltipData      tooltip;
        ModalData        modal;
        TabContainerData tabContainer;
        RadialMenuData   radialMenu;
        MinimapData      minimap;
        DragSlotData     dragSlot;
        ListRepeaterData listRepeater;
        StackLayoutData  stackLayout;
        GridLayoutData   gridLayout;
        ScrollBoxData    scrollBox;
    };
}
