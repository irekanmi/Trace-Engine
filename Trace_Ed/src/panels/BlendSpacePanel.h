#pragma once

#include "animation/BlendSpace2D.h"

#include "imgui.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <functional>



namespace trace {


    class BlendSpacePanel
    {
    
    public:
        BlendSpacePanel() = default;
        BlendSpacePanel(const std::string& title,
            const std::string& xLabel,
            const std::string& yLabel);

        void Draw();
        void SetBlendSpace(Ref<BlendSpace2D> blend_space);
        void Shutdown();

    public:
        // Current blend position (the green preview dot)
        float previewX = 0.f;
        float previewY = 300.f;

    private:

        void AddSample(float x, float y);

        void RemoveSampleAt(int idx);

    private:
        Ref<BlendSpace2D> m_blendSpace;
        std::string title = "Blend Space";
        std::string xAxisLabel = "X Axis";
        std::string yAxisLabel = "Y Axis";

        float xMin = -180.f, xMax = 180.f;   // user-facing axis ranges
        float yMin = 0.f, yMax = 600.f;

        bool  showTriangulation = true;   // draw Delaunay triangulation lines
        bool  showGrid = true;
        bool  showTooltips = true;

        

        // Callback: fired whenever previewX / previewY change
        std::function<void(float x, float y)> onPreviewMoved;
        // Callback: fired when a sample is added / removed / moved
        std::function<void()> onSamplesChanged;


        // Internal geometry state
        ImVec2 canvasMin_{}, canvasMax_{};
        ImVec2 plotMin_{}, plotMax_{};

        int    dragSampleIdx_ = -1;
        bool   draggingPreview_ = false;
        float  pendingAddX_ = 0.f;
        float  pendingAddY_ = 0.f;
        int    selectedSample_ = -1;


        // ------- draw helpers --------------------------------------------------------
        void DrawAxes(ImDrawList* dl, float pw, float ph);

        void DrawTriangulation(ImDrawList* dl, float pw, float ph);

        void DrawInfluenceRegions(ImDrawList* dl, float pw, float ph);

        void HandleSampleInteraction(ImDrawList* dl, float pw, float ph, bool hovered);

        void HandlePreviewInteraction(ImDrawList* dl, float pw, float ph,
            bool hovered, bool /*active*/);

        void DrawContextMenu();

        void DrawToolbar();

        void DrawPropertiesPanel();

        float Clamp(float v, float lo, float hi);
        float PlotToWorldX(float px, float pw);
        float PlotToWorldY(float py, float ph);
        float WorldToPlotX(float wx, float pw);
        float WorldToPlotY(float wy, float ph);
        bool InPlot(ImVec2 p);

        std::vector<float> weights_;
    };

}
