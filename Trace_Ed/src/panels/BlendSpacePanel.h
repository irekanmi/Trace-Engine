#pragma once


#include "imgui.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <functional>



namespace trace {


    // ============================================================
    // NOTE: This class is generated with AI - Claude
    //  BlendSpacePanel  –  Unreal-style 2D blend space widget
    //
    //  Usage:
    //      static BlendSpacePanel bs("Movement", "Speed", "Direction");
    //      bs.Draw();   // call once per frame inside an ImGui window
    // ============================================================

    struct BlendSample
    {
        std::string name;       // animation clip name shown as a label
        float       x = 0.f;   // horizontal axis value (normalized 0-1 inside storage)
        float       y = 0.f;   // vertical   axis value (normalized 0-1 inside storage)
        bool        selected = false;
    };

    class BlendSpacePanel
    {
    public:
        // ---- public configuration ---------------------------------------------------
        std::string title = "Blend Space";
        std::string xAxisLabel = "X Axis";
        std::string yAxisLabel = "Y Axis";

        float xMin = -180.f, xMax = 180.f;   // user-facing axis ranges
        float yMin = 0.f, yMax = 600.f;

        bool  showTriangulation = true;   // draw Delaunay triangulation lines
        bool  showGrid = true;
        bool  showTooltips = true;

        // Current blend position (the green preview dot)
        float previewX = 0.f;
        float previewY = 300.f;

        // Callback: fired whenever previewX / previewY change
        std::function<void(float x, float y)> onPreviewMoved;
        // Callback: fired when a sample is added / removed / moved
        std::function<void()> onSamplesChanged;

        // ---- samples ----------------------------------------------------------------
        std::vector<BlendSample> samples;

        void AddSample(const std::string& name, float x, float y);

        void RemoveSampleAt(int idx);

        // ---- constructor ------------------------------------------------------------
        BlendSpacePanel() = default;
        BlendSpacePanel(const std::string& title,
            const std::string& xLabel,
            const std::string& yLabel);

        // ---- main draw call ---------------------------------------------------------
        void Draw();

    private:
        // Internal geometry state
        ImVec2 canvasMin_{}, canvasMax_{};
        ImVec2 plotMin_{}, plotMax_{};

        int    dragSampleIdx_ = -1;
        bool   draggingPreview_ = false;
        float  pendingAddX_ = 0.f;
        float  pendingAddY_ = 0.f;
        int    selectedSample_ = -1;

        // Triangulation: each entry is {i0, i1, i2} index into samples[]
        struct Triangle { int a, b, c; };
        std::vector<Triangle> triangles_;

        // ------- coordinate helpers --------------------------------------------------
        float WorldToPlotX(float wx, float pw) const;
        float WorldToPlotY(float wy, float ph) const;
        float PlotToWorldX(float px, float pw) const;
        float PlotToWorldY(float py, float ph) const;
        bool InPlot(ImVec2 p) const;
        float Clamp(float v, float lo, float hi) const;

        // ------- triangulation (simple incremental – good enough for blend spaces) ---
        // We use a straightforward O(n²) approach; blend spaces rarely exceed ~20 pts.
        void RebuildTriangulation();

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

        std::vector<float> weights_;
    };

}
