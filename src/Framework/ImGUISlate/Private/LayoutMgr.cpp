
#include "LayoutMgr.h"
#include "Widget.h"

#include <algorithm>

namespace ImGUISlate{

void LayoutManager::AddWidget(
    SlateCore::Widget* widget,
    const LayoutParams& params)
{
    Elements.push_back(
        {
            widget,
            params
        });
}


inline void LayoutManager::RemoveWidget(SlateCore::Widget* widget)
{
    Elements.erase(
        std::remove_if(
            Elements.begin(),
            Elements.end(),
            [&](auto& e)
            {
                return e.WidgetPtr == widget;
            }),
        Elements.end());
}



//----------------------------------------
// Vertical Layout
//----------------------------------------

inline void VerticalLayout::Layout(float width, float height)
{
    float fixedHeight = 0;
    float totalWeight = 0;


    for (auto& e : Elements)
    {
        if (e.Params.Weight > 0)
        {
            totalWeight += e.Params.Weight;
        }
        else
        {
            fixedHeight += e.Params.Height;
        }
    }


    float remainHeight = height - fixedHeight;

    float y = 0;


    for (auto& e : Elements)
    {
        float h;

        if (e.Params.Weight > 0)
        {
            h = remainHeight *
                e.Params.Weight /
                totalWeight;
        }
        else
        {
            h = e.Params.Height;
        }


        auto& m = e.Params.Margin;


        e.WidgetPtr->SetGeometry(
            m.Left,
            y + m.Top,
            width - m.Left - m.Right,
            h - m.Top - m.Bottom);


        y += h;
    }
}



//----------------------------------------
// Horizontal Layout
//----------------------------------------

inline void HorizontalLayout::Layout(float width, float height)
{
    float fixedWidth = 0;
    float totalWeight = 0;


    for (auto& e : Elements)
    {
        if (e.Params.Weight > 0)
        {
            totalWeight += e.Params.Weight;
        }
        else
        {
            fixedWidth += e.Params.Width;
        }
    }


    float remainWidth = width - fixedWidth;

    float x = 0;


    for (auto& e : Elements)
    {
        float w;


        if (e.Params.Weight > 0)
        {
            w = remainWidth *
                e.Params.Weight /
                totalWeight;
        }
        else
        {
            w = e.Params.Width;
        }


        auto& m = e.Params.Margin;


        e.WidgetPtr->SetGeometry(
            x + m.Left,
            m.Top,
            w - m.Left - m.Right,
            height - m.Top - m.Bottom);


        x += w;
    }
}



//----------------------------------------
// Grid Layout
//----------------------------------------

inline GridLayout::GridLayout(uint32_t rows, uint32_t columns)
    :
    Rows(rows),
    Columns(columns)
{
}



inline void GridLayout::Layout(float width, float height)
{
    float cellWidth = width / Columns;
    float cellHeight = height / Rows;


    for (auto& e : Elements)
    {
        auto& p = e.Params;
        auto& m = p.Margin;


        float x = p.Column * cellWidth;
        float y = p.Row * cellHeight;


        float w = cellWidth * p.ColumnSpan;
        float h = cellHeight * p.RowSpan;


        e.WidgetPtr->SetGeometry(
            x + m.Left,
            y + m.Top,
            w - m.Left - m.Right,
            h - m.Top - m.Bottom);
    }
}
}