
#include "BlendSpacePanel.h"
#include "animation/Animation.h"
#include "../utils/ImGui_utils.h"

namespace trace {



	void BlendSpacePanel::AddSample(float x, float y)
	{
		if (!m_blendSpace)
		{
			return;
		}
		std::vector<glm::vec2>& points = m_blendSpace->GetPoints();
		points.push_back({ x, y });
		m_blendSpace->RebuildTriangulation();
	}

	void BlendSpacePanel::RemoveSampleAt(int idx)
	{
		if (!m_blendSpace)
		{
			return;
		}
		std::vector<glm::vec2>& points = m_blendSpace->GetPoints();
		if (idx >= 0 && idx < (int)points.size())
		{
			points.erase(points.begin() + idx);
			m_blendSpace->GetAnimationMap().erase(idx);
			m_blendSpace->RebuildTriangulation();
		}
	}

	BlendSpacePanel::BlendSpacePanel(const std::string& title, const std::string& xLabel, const std::string& yLabel)
		: title(title), xAxisLabel(xLabel), yAxisLabel(yLabel)
	{


	}

	void BlendSpacePanel::Draw()
	{
		ImGui::PushID(title.c_str());

		DrawToolbar();

		// Reserve canvas space (fills available width, fixed height)
		ImVec2 canvasSize = { ImGui::GetContentRegionAvail().x,
							  ImGui::GetContentRegionAvail().y - 80.f };
		if (canvasSize.y < 200.f) canvasSize.y = 200.f;

		ImVec2 canvasPos = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();

		// Invisible button captures mouse for the canvas
		ImGui::InvisibleButton("canvas", canvasSize,
			ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
		bool canvasHovered = ImGui::IsItemHovered();
		bool canvasActive = ImGui::IsItemActive();

		canvasMin_ = canvasPos;
		canvasMax_ = { canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y };

		// Margins for axis labels
		const float marginL = 50.f, marginR = 10.f;
		const float marginT = 10.f, marginB = 40.f;
		plotMin_ = { canvasMin_.x + marginL, canvasMin_.y + marginT };
		plotMax_ = { canvasMax_.x - marginR, canvasMax_.y - marginB };
		float pw = plotMax_.x - plotMin_.x;
		float ph = plotMax_.y - plotMin_.y;

		// ---- background ---------------------------------------------------------
		dl->AddRectFilled(canvasMin_, canvasMax_,
			IM_COL32(30, 30, 30, 255), 6.f);
		dl->AddRect(canvasMin_, canvasMax_,
			IM_COL32(80, 80, 80, 255), 6.f);

		dl->AddRectFilled(plotMin_, plotMax_, IM_COL32(20, 20, 20, 255));

		// ---- grid ---------------------------------------------------------------
		if (showGrid)
		{
			const int gridLines = 10;
			ImU32 gridCol = IM_COL32(50, 50, 50, 255);
			for (int i = 0; i <= gridLines; ++i)
			{
				float tx = plotMin_.x + pw * i / gridLines;
				float ty = plotMin_.y + ph * i / gridLines;
				dl->AddLine({ tx, plotMin_.y }, { tx, plotMax_.y }, gridCol);
				dl->AddLine({ plotMin_.x, ty }, { plotMax_.x, ty }, gridCol);
			}
		}

		// ---- axis labels & ticks ------------------------------------------------
		DrawAxes(dl, pw, ph);


		// ---- triangulation ------------------------------------------------------
		if (showTriangulation && m_blendSpace && m_blendSpace->GetTriangles().size() > 0)
		{
			DrawTriangulation(dl, pw, ph);
		}

		// ---- blend weight influence visualization -------------------------------
		DrawInfluenceRegions(dl, pw, ph);

		 // ---- samples ------------------------------------------------------------
		HandleSampleInteraction(dl, pw, ph, canvasHovered);

		// ---- preview point ------------------------------------------------------
		HandlePreviewInteraction(dl, pw, ph, canvasHovered, canvasActive);

		// ---- right-click context menu -------------------------------------------
		if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImVec2 mp = ImGui::GetMousePos();
			if (InPlot(mp))
			{
				pendingAddX_ = PlotToWorldX(mp.x, pw);
				pendingAddY_ = PlotToWorldY(mp.y, ph);
				ImGui::OpenPopup("BlendSpaceCtx");
			}
		}
		DrawContextMenu();

		// ---- canvas title -------------------------------------------------------
		dl->AddText({ canvasMin_.x + 6, canvasMin_.y + 4 },
			IM_COL32(200, 200, 200, 200), title.c_str());

		// ---- properties panel ---------------------------------------------------
		DrawPropertiesPanel();

		ImGui::PopID();

	}

	void BlendSpacePanel::SetBlendSpace(Ref<BlendSpace2D> blend_space)
	{
		m_blendSpace = blend_space;
	}

	void BlendSpacePanel::Shutdown()
	{
		m_blendSpace.free();
	}

	void BlendSpacePanel::DrawAxes(ImDrawList* dl, float pw, float ph)
	{
		ImU32 axisCol = IM_COL32(140, 140, 140, 255);
		ImU32 labelCol = IM_COL32(180, 180, 180, 255);

		// Border
		dl->AddRect(plotMin_, plotMax_, IM_COL32(80, 80, 80, 255));

		// X-axis ticks & labels
		const int ticks = 5;
		for (int i = 0; i <= ticks; ++i)
		{
			float t = (float)i / ticks;
			float wx = xMin + t * (xMax - xMin);
			float px = plotMin_.x + t * pw;

			dl->AddLine({ px, plotMax_.y }, { px, plotMax_.y + 5.f }, axisCol);
			char buf[16]; snprintf(buf, sizeof(buf), "%.0f", wx);
			ImVec2 ts = ImGui::CalcTextSize(buf);
			dl->AddText({ px - ts.x * 0.5f, plotMax_.y + 7.f }, labelCol, buf);
		}

		// Y-axis ticks & labels
		for (int i = 0; i <= ticks; ++i)
		{
			float t = (float)i / ticks;
			float wy = yMin + t * (yMax - yMin);
			float py = plotMax_.y - t * ph;  // flip

			dl->AddLine({ plotMin_.x - 5.f, py }, { plotMin_.x, py }, axisCol);
			char buf[16]; snprintf(buf, sizeof(buf), "%.0f", wy);
			ImVec2 ts = ImGui::CalcTextSize(buf);
			dl->AddText({ plotMin_.x - ts.x - 8.f, py - ts.y * 0.5f }, labelCol, buf);
		}

		// Axis name labels
		ImVec2 xLabelSz = ImGui::CalcTextSize(xAxisLabel.c_str());
		dl->AddText({ plotMin_.x + pw * 0.5f - xLabelSz.x * 0.5f,
					 plotMax_.y + 22.f }, IM_COL32(220, 180, 80, 255), xAxisLabel.c_str());

		// Y label (rotated via AddText tricks – just place it vertically alongside)
		ImVec2 yLabelSz = ImGui::CalcTextSize(yAxisLabel.c_str());
		dl->AddText({ plotMin_.x - 42.f,
					 plotMin_.y + ph * 0.5f - yLabelSz.y * 0.5f },
			IM_COL32(220, 180, 80, 255), yAxisLabel.c_str());



	}

	void BlendSpacePanel::DrawTriangulation(ImDrawList* dl, float pw, float ph)
	{
		if (!m_blendSpace)
		{
			return;
		}

		ImU32 triCol = IM_COL32(60, 120, 200, 80);
		ImU32 edgeCol = IM_COL32(80, 150, 220, 160);

		for (auto& tri : m_blendSpace->GetTriangles())
		{
			ImVec2 pa = { WorldToPlotX(tri.triangle.vertex0.x, pw),
						  WorldToPlotY(tri.triangle.vertex0.y, ph) };
			ImVec2 pb = { WorldToPlotX(tri.triangle.vertex1.x, pw),
						  WorldToPlotY(tri.triangle.vertex1.y, ph) };
			ImVec2 pc = { WorldToPlotX(tri.triangle.vertex2.x, pw),
						  WorldToPlotY(tri.triangle.vertex2.y, ph) };

			dl->AddTriangleFilled(pa, pb, pc, triCol);
			dl->AddTriangle(pa, pb, pc, edgeCol, 1.f);
		}
	}

	void BlendSpacePanel::DrawInfluenceRegions(ImDrawList* dl, float pw, float ph)
	{

		if (!m_blendSpace)
		{
			return;
		}
		std::vector<glm::vec2>& points = m_blendSpace->GetPoints();

		if (points.empty()) return;

		// Compute barycentric weights for the preview position
		float wx = previewX, wy = previewY;
		weights_.assign(points.size(), 0.f);

		bool found = false;
		for (auto& tri : m_blendSpace->GetTriangles())
		{
			float ax = points[tri.index0].x, ay = points[tri.index0].y;
			float bx = points[tri.index1].x, by = points[tri.index1].y;
			float cx = points[tri.index2].x, cy = points[tri.index2].y;

			float denom = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy);
			if (fabsf(denom) < 1e-7f) continue;

			float la = ((by - cy) * (wx - cx) + (cx - bx) * (wy - cy)) / denom;
			float lb = ((cy - ay) * (wx - cx) + (ax - cx) * (wy - cy)) / denom;
			float lc = 1.f - la - lb;

			if (la >= -1e-4f && lb >= -1e-4f && lc >= -1e-4f)
			{
				weights_[tri.index0] = la;
				weights_[tri.index1] = lb;
				weights_[tri.index2] = lc;
				found = true;
				break;
			}
		}

		

		// Draw influence circles scaled by weight
		for (int i = 0; i < (int)points.size(); ++i)
		{
			if (weights_[i] < 0.01f) continue;
			float px = WorldToPlotX(points[i].x, pw);
			float py = WorldToPlotY(points[i].y, ph);
			float r = weights_[i] * 40.f;
			ImU32 col = IM_COL32(255, 160, 40,
				(int)(weights_[i] * 120.f));
			dl->AddCircleFilled({ px, py }, r, col);
		}
	}

	void BlendSpacePanel::HandleSampleInteraction(ImDrawList* dl, float pw, float ph, bool hovered)
	{
		if (!m_blendSpace)
		{
			return;
		}

		const float hitRadius = 8.f;

		std::vector<glm::vec2>& points = m_blendSpace->GetPoints();

		// Mouse drag update
		if (dragSampleIdx_ >= 0)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				ImVec2 mp = ImGui::GetMousePos();
				points[dragSampleIdx_].x = Clamp(PlotToWorldX(mp.x, pw), xMin, xMax);
				points[dragSampleIdx_].y = Clamp(PlotToWorldY(mp.y, ph), yMin, yMax);
				m_blendSpace->RebuildTriangulation();
				if (onSamplesChanged) onSamplesChanged();
			}
			else dragSampleIdx_ = -1;
		}

		// Draw & hit-test samples
		for (int i = 0; i < (int)points.size(); ++i)
		{
			float px = WorldToPlotX(points[i].x, pw);
			float py = WorldToPlotY(points[i].y, ph);
			bool  sel = (selectedSample_ == i);

			// Diamond shape (like UE)
			float r = sel ? 9.f : 7.f;
			ImVec2 pts[4] = {
				{px,   py - r},
				{px + r, py  },
				{px,   py + r},
				{px - r, py  }
			};
			ImU32 fillCol = sel ? IM_COL32(255, 200, 50, 255) : IM_COL32(200, 200, 200, 255);
			dl->AddConvexPolyFilled(pts, 4, fillCol);
			dl->AddPolyline(pts, 4, IM_COL32(30, 30, 30, 255), ImDrawFlags_Closed, 1.5f);

			// Label
			std::string label = "Animation Clip";
			if (Ref<AnimationClip> clip = m_blendSpace->GetPointAnimation(i))
			{
				label = clip->GetName();
			}
			ImVec2 ts = ImGui::CalcTextSize(label.c_str());
			dl->AddText({ px - ts.x * 0.5f, py + r + 2.f },
				IM_COL32(220, 220, 220, 220), label.c_str());

			// Hover / click detection
			if (hovered && dragSampleIdx_ < 0 && !draggingPreview_)
			{
				ImVec2 mp = ImGui::GetMousePos();
				float  dx = mp.x - px, dy = mp.y - py;
				if (dx * dx + dy * dy < hitRadius * hitRadius)
				{
					ImGui::SetTooltip("%s\n(%.1f, %.1f)",
						label.c_str(), points[i].x, points[i].y);

					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						selectedSample_ = i;
						dragSampleIdx_ = i;
					}
				}
			}
		}
	}

	void BlendSpacePanel::HandlePreviewInteraction(ImDrawList* dl, float pw, float ph, bool hovered, bool)
	{
		const float r = 10.f;
		float px = WorldToPlotX(previewX, pw);
		float py = WorldToPlotY(previewY, ph);

		if (draggingPreview_)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				ImVec2 mp = ImGui::GetMousePos();
				previewX = Clamp(PlotToWorldX(mp.x, pw), xMin, xMax);
				previewY = Clamp(PlotToWorldY(mp.y, ph), yMin, yMax);
				if (onPreviewMoved) onPreviewMoved(previewX, previewY);
				px = WorldToPlotX(previewX, pw);
				py = WorldToPlotY(previewY, ph);
			}
			else draggingPreview_ = false;
		}

		// Cross-hair lines
		dl->AddLine({ plotMin_.x, py }, { plotMax_.x, py },
			IM_COL32(80, 200, 80, 60));
		dl->AddLine({ px, plotMin_.y }, { px, plotMax_.y },
			IM_COL32(80, 200, 80, 60));

		// Outer ring + fill
		dl->AddCircleFilled({ px, py }, r, IM_COL32(40, 180, 60, 200));
		dl->AddCircle({ px, py }, r, IM_COL32(180, 255, 180, 255), 0, 2.f);
		dl->AddCircleFilled({ px, py }, 3.f, IM_COL32(255, 255, 255, 255));

		if (showTooltips)
		{
			ImVec2 mp = ImGui::GetMousePos();
			float  dx = mp.x - px, dy = mp.y - py;
			if (hovered && dx * dx + dy * dy < r * r)
				ImGui::SetTooltip("Preview\n(%.1f, %.1f)", previewX, previewY);
		}

		// Click-to-start-drag on preview
		if (hovered && !draggingPreview_ && dragSampleIdx_ < 0)
		{
			ImVec2 mp = ImGui::GetMousePos();
			float  dx = mp.x - px, dy = mp.y - py;
			if (dx * dx + dy * dy < r * r && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				draggingPreview_ = true;
		}
	}

	void BlendSpacePanel::DrawContextMenu()
	{
		if (!m_blendSpace)
		{
			return;
		}
		std::vector<glm::vec2>& points = m_blendSpace->GetPoints();

		if (ImGui::BeginPopup("BlendSpaceCtx"))
		{
			ImGui::TextDisabled("Add Sample at (%.1f, %.1f)",
				pendingAddX_, pendingAddY_);
			ImGui::Separator();
			if (ImGui::Button("Add Sample", { 120.f, 0.f }))
			{
				AddSample(pendingAddX_, pendingAddY_);
				if (onSamplesChanged) onSamplesChanged();
				ImGui::CloseCurrentPopup();
			}

			if (selectedSample_ >= 0 && selectedSample_ < (int)points.size())
			{
				ImGui::Separator();
				std::string lbl = "Remove Animation Clip";
				if (Ref<AnimationClip> clip = m_blendSpace->GetPointAnimation(selectedSample_))
				{
					lbl = "Remove \"" + clip->GetName() + "\"";
				}
				if (ImGui::MenuItem(lbl.c_str()))
				{
					RemoveSampleAt(selectedSample_);
					selectedSample_ = -1;
					if (onSamplesChanged) onSamplesChanged();
				}
			}

			ImGui::EndPopup();
		}
	}

	void BlendSpacePanel::DrawToolbar()
	{
		ImGui::Text("%s", title.c_str());
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		ImGui::DragFloat("##pvX", &previewX, 0.5f, xMin, xMax, "X:%.1f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		ImGui::DragFloat("##pvY", &previewY, 0.5f, yMin, yMax, "Y:%.1f");
		ImGui::SameLine();
		ImGui::Checkbox("Grid", &showGrid);
		ImGui::SameLine();
		ImGui::Checkbox("Tris", &showTriangulation);
		ImGui::SameLine();
		if (ImGui::SmallButton("Add Sample"))
		{
			pendingAddX_ = (xMin + xMax) * 0.5f;
			pendingAddY_ = (yMin + yMax) * 0.5f;
			ImGui::OpenPopup("BlendSpaceCtx");
		}
		DrawContextMenu();
		ImGui::Separator();
	}

	void BlendSpacePanel::DrawPropertiesPanel()
	{
		if (!m_blendSpace)
		{
			return;
		}
		std::vector<glm::vec2>& points = m_blendSpace->GetPoints();

		if (selectedSample_ < 0 || selectedSample_ >= (int)points.size()) return;

		ImGui::Separator();

		std::string clip_name = "None(Animation Clip)";
		if (Ref<AnimationClip> clip = m_blendSpace->GetPointAnimation(selectedSample_))
		{
			clip_name = clip->GetName();
		}
		ImGui::Text("Animation Clip: ");
		ImGui::SameLine();
		ImGui::Button(clip_name.c_str());
		if (Ref<AnimationClip> new_clip = ImGuiDragDropResource<AnimationClip>(ANIMATION_CLIP_FILE_EXTENSION))
		{
			m_blendSpace->GetAnimationMap()[selectedSample_] = new_clip;
		}

		auto& s = points[selectedSample_];
		bool changed = false;
		ImGui::SetNextItemWidth(80.f);
		if (ImGui::DragFloat("X##sp", &s.x, 0.5f, xMin, xMax)) changed = true;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		if (ImGui::DragFloat("Y##sp", &s.y, 0.5f, yMin, yMax)) changed = true;
		ImGui::SameLine();
		
		
		if (changed)
		{
			m_blendSpace->RebuildTriangulation();
			if (onSamplesChanged) onSamplesChanged();
		}

		// Weight readout
		if (!weights_.empty())
		{
			ImGui::TextDisabled(" Weight: %.3f", weights_[selectedSample_]);
		}
	}

	float BlendSpacePanel::Clamp(float v, float lo, float hi)
	{
		return v < lo ? lo : (v > hi ? hi : v);
	}

	float BlendSpacePanel::PlotToWorldX(float px, float pw)
	{
		return xMin + (px - plotMin_.x) / pw * (xMax - xMin);
	}

	float BlendSpacePanel::PlotToWorldY(float py, float ph)
	{
		return yMin + (plotMax_.y - py) / ph * (yMax - yMin);
	}

	float BlendSpacePanel::WorldToPlotX(float wx, float pw)
	{
		return plotMin_.x + (wx - xMin) / (xMax - xMin) * pw;
	}

	float BlendSpacePanel::WorldToPlotY(float wy, float ph)
	{
		// Y is flipped (world yMax -> plot top)
		return plotMax_.y - (wy - yMin) / (yMax - yMin) * ph;
	}

	bool BlendSpacePanel::InPlot(ImVec2 p)
	{
		return p.x >= plotMin_.x && p.x <= plotMax_.x &&
			p.y >= plotMin_.y && p.y <= plotMax_.y;
	}

}