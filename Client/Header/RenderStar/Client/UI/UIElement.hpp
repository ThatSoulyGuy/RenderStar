#pragma once

#include "RenderStar/Client/UI/UibinComponents.hpp"
#include <string>
#include <vector>

namespace RenderStar::Client::UI
{
    struct UIElement
    {
        std::string id;
        std::string name;
        std::vector<UIComponent> components;
        std::vector<UIElement> children;

        const TransformData* GetTransform() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Transform)
                    return &c.transform;
            return nullptr;
        }

        const ImageData* GetImage() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Image)
                    return &c.image;
            return nullptr;
        }

        const TextData* GetText() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Text)
                    return &c.text;
            return nullptr;
        }

        const PanelData* GetPanel() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Panel)
                    return &c.panel;
            return nullptr;
        }

        const ButtonData* GetButton() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Button)
                    return &c.button;
            return nullptr;
        }

        const ProgressBarData* GetProgressBar() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::ProgressBar)
                    return &c.progressBar;
            return nullptr;
        }

        const ToggleData* GetToggle() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Toggle)
                    return &c.toggle;
            return nullptr;
        }

        ToggleData* GetToggle()
        {
            for (auto& c : components)
                if (c.kind == UIComponentKind::Toggle)
                    return &c.toggle;
            return nullptr;
        }

        const DropdownData* GetDropdown() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Dropdown)
                    return &c.dropdown;
            return nullptr;
        }

        DropdownData* GetDropdown()
        {
            for (auto& c : components)
                if (c.kind == UIComponentKind::Dropdown)
                    return &c.dropdown;
            return nullptr;
        }

        const TextInputData* GetTextInput() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::TextInput)
                    return &c.textInput;
            return nullptr;
        }

        TextInputData* GetTextInput()
        {
            for (auto& c : components)
                if (c.kind == UIComponentKind::TextInput)
                    return &c.textInput;
            return nullptr;
        }

        const ModalData* GetModal() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Modal)
                    return &c.modal;
            return nullptr;
        }

        const IconData* GetIcon() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Icon)
                    return &c.icon;
            return nullptr;
        }

        const SpriteData* GetSprite() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Sprite)
                    return &c.sprite;
            return nullptr;
        }

        const StackLayoutData* GetStackLayout() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::StackLayout)
                    return &c.stackLayout;
            return nullptr;
        }

        const GridLayoutData* GetGridLayout() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::GridLayout)
                    return &c.gridLayout;
            return nullptr;
        }

        const ScrollBoxData* GetScrollBox() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::ScrollBox)
                    return &c.scrollBox;
            return nullptr;
        }

        const TabContainerData* GetTabContainer() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::TabContainer)
                    return &c.tabContainer;
            return nullptr;
        }

        const RadialMenuData* GetRadialMenu() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::RadialMenu)
                    return &c.radialMenu;
            return nullptr;
        }

        const MinimapData* GetMinimap() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Minimap)
                    return &c.minimap;
            return nullptr;
        }

        const DragSlotData* GetDragSlot() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::DragSlot)
                    return &c.dragSlot;
            return nullptr;
        }

        const ListRepeaterData* GetListRepeater() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::ListRepeater)
                    return &c.listRepeater;
            return nullptr;
        }

        const TooltipData* GetTooltip() const
        {
            for (const auto& c : components)
                if (c.kind == UIComponentKind::Tooltip)
                    return &c.tooltip;
            return nullptr;
        }

        UIElement* FindById(const std::string& searchId)
        {
            if (id == searchId) return this;
            for (auto& child : children)
                if (auto* found = child.FindById(searchId))
                    return found;
            return nullptr;
        }

        UIElement* FindByName(const std::string& searchName)
        {
            if (name == searchName) return this;
            for (auto& child : children)
                if (auto* found = child.FindByName(searchName))
                    return found;
            return nullptr;
        }
    };
}
