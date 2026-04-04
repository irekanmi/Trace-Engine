#pragma once

#include "EditorWindow.h"
#include "render/Camera.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "animation/BlendSpace2D.h"
#include "animation/Skeleton.h"
#include "animation/AnimationPose.h"

namespace trace {

	class HierachyPanel;
	class BlendSpacePanel;

	class BlendSpace2DWindow : public EditorWindow
	{

	public:

		bool OnCreate(TraceEditor* editor, const std::string& name, const std::string& file_path);
		virtual void OnDestroy(TraceEditor* editor);
		virtual void OnUpdate(float deltaTime);
		virtual void OnRender(float deltaTime);
		virtual void DockChildWindows();
		virtual void RenderViewport(std::vector<void*>& texture_handles);
		virtual void OnEvent(Event* p_event);

	private:
		Ref<BlendSpace2D> m_blendSpace;
		Camera m_camera;
		Animation::SkeletonInstance m_skeleton;
		UUID object_id;
		Scene* m_scene;
		int32_t view_index;
		HierachyPanel* m_hierachy;
		BlendSpacePanel* m_editor;
		glm::vec2 m_viewportSize;
		std::string hierachy_name;
		std::string blend_space_editor_name;
		std::string viewport_name;
		int gizmo_mode = 1;
		std::string blend_space_path;
		bool has_prefab = false;
		bool has_skeleton = false;
		Animation::Pose final_pose;
		Animation::Pose pose_a;
		Animation::Pose pose_b;
		Animation::Pose pose_c;


	protected:

	};

}
