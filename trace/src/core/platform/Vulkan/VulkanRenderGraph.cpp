#include "pch.h"

#include "VulkanRenderGraph.h"
#include "render/GPipeline.h"


extern trace::VKHandle g_Vkhandle;

namespace vk {



	bool compute_pass_handle(trace::RenderGraphPass* pass, trace::RenderGraph* render_graph, trace::VKDeviceHandle* _device, trace::VKHandle* instance);
	bool compute_resource_handle( trace::RenderGraph* render_graph, trace::VKDeviceHandle* _device, trace::VKHandle* instance);
	bool compute_frame_buffer_handle(trace::RenderGraphPass* pass, trace::RenderGraph* render_graph, trace::VKDeviceHandle* _device, trace::VKHandle* instance);

	bool __BuildRenderGraph(trace::GDevice* device, trace::RenderGraph* render_graph)
	{
		bool result = true;

		if (!device || !render_graph)
		{
			TRC_ERROR("Unable to build render graph please enter a valid pointer, {} || {}", (const void*)device, (const void*)render_graph);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Unable to build render graph please enter a valid device, {}", (const void*)device->GetRenderHandle()->m_internalData);
			return false;
		}

		if (render_graph->GetRenderHandle()->m_internalData)
		{
			TRC_WARN("Render Graph has already been built, {}", (const void*)render_graph->GetRenderHandle()->m_internalData);
			return false;
		}

		trace::VKRenderGraph* handle = new trace::VKRenderGraph;

		trace::VKDeviceHandle* _device = reinterpret_cast<trace::VKDeviceHandle*>(device->GetRenderHandle()->m_internalData);
		trace::VKHandle* instance = &g_Vkhandle;
		handle->m_device = _device;
		handle->m_instance = instance;
		render_graph->GetRenderHandle()->m_internalData = handle;

		std::vector<uint32_t>& graph_passes = render_graph->GetSubmissionPasses();
		std::vector<trace::RenderGraphResource>& graph_resources = render_graph->GetResources();

		compute_resource_handle(render_graph, _device, instance);
		for (auto& pass_index : graph_passes)
		{
			trace::RenderGraphPass* pass = &render_graph->GetPass(pass_index);
			trace::VKRenderGraphPass* pass_handle = new trace::VKRenderGraphPass;
			pass->GetRenderHandle()->m_internalData = pass_handle;
			
			compute_pass_handle(pass, render_graph, _device, instance);

			for (auto& edge : pass->GetPassEdges())
			{
				bool from, to, _from, _to;
				from = to = _from = _to =  false;

				from = (edge.from != nullptr);
				to = (edge.to != nullptr);
				_from = (edge.from == pass) ;
				_to = (edge.to == pass);

				trace::RenderGraphResource* res = edge.resource;
				bool isTexture, isSwapchainImage;
				isTexture = res->resource_type == trace::RenderGraphResourceType::Texture;
				isSwapchainImage = res->resource_type == trace::RenderGraphResourceType::SwapchainImage;

				if (from && to)
				{
					if (_from && isTexture)
					{
						for (uint32_t i = 0; i < _device->frames_in_flight; i++)
						{
							trace::VKImage& img_handle = reinterpret_cast<trace::VKRenderGraphResource*>(res->render_handle.m_internalData)->resource[i].texture;
							VkImageMemoryBarrier img_bar_invalidate = {};
							img_bar_invalidate.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
							img_bar_invalidate.srcAccessMask = VK_ACCESS_NONE;
							img_bar_invalidate.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							img_bar_invalidate.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_invalidate.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_invalidate.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							img_bar_invalidate.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							img_bar_invalidate.pNext = nullptr;
							img_bar_invalidate.image = img_handle.m_handle;
							img_bar_invalidate.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							img_bar_invalidate.subresourceRange.baseArrayLayer = 0;
							img_bar_invalidate.subresourceRange.baseMipLevel = 0;
							img_bar_invalidate.subresourceRange.layerCount = 1;
							img_bar_invalidate.subresourceRange.levelCount = 1;

							VkImageMemoryBarrier img_bar_flush = {};
							img_bar_flush.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
							img_bar_flush.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							img_bar_flush.dstAccessMask = VK_ACCESS_NONE;
							img_bar_flush.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_flush.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_flush.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							img_bar_flush.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							img_bar_flush.pNext = nullptr;
							img_bar_flush.image = img_handle.m_handle;
							img_bar_flush.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							img_bar_flush.subresourceRange.baseArrayLayer = 0;
							img_bar_flush.subresourceRange.baseMipLevel = 0;
							img_bar_flush.subresourceRange.layerCount = 1;
							img_bar_flush.subresourceRange.levelCount = 1;

							if (res->resource_data.texture.attachment_type == trace::AttachmentType::DEPTH)
							{
								img_bar_invalidate.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
								img_bar_invalidate.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
								img_bar_invalidate.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

								img_bar_flush.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
								img_bar_flush.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
								img_bar_flush.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


							}

							pass_handle->invalidate_image_barriers[i].push_back(img_bar_invalidate);
							pass_handle->flush_image_barriers[i].push_back(img_bar_flush);
						}

						VkEventCreateInfo evnt_info = {};
						evnt_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
						evnt_info.pNext = nullptr;
						trace::VKEvntPair evnt_pair = {pass, res};
						VkEvent& evnt = handle->events.emplace_back(evnt_pair).evnt;

						VkResult _result = VK_ERROR_UNKNOWN;
						_result = vkCreateEvent(
							_device->m_device,
							&evnt_info,
							instance->m_alloc_callback,
							&evnt
						);
						VK_ASSERT(_result);
						pass_handle->signal_events.push_back(evnt);

					}
					else if (_to && isTexture)
					{
						for (uint32_t i = 0; i < _device->frames_in_flight; i++)
						{
							trace::VKImage& img_handle = reinterpret_cast<trace::VKRenderGraphResource*>(res->render_handle.m_internalData)->resource[i].texture;
							VkImageMemoryBarrier img_bar_invalidate = {};
							img_bar_invalidate.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
							img_bar_invalidate.srcAccessMask = VK_ACCESS_NONE;
							img_bar_invalidate.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
							img_bar_invalidate.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_invalidate.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_invalidate.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							img_bar_invalidate.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							img_bar_invalidate.pNext = nullptr;
							img_bar_invalidate.image = img_handle.m_handle;
							img_bar_invalidate.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							img_bar_invalidate.subresourceRange.baseArrayLayer = 0;
							img_bar_invalidate.subresourceRange.baseMipLevel = 0;
							img_bar_invalidate.subresourceRange.layerCount = 1;
							img_bar_invalidate.subresourceRange.levelCount = 1;

							VkImageMemoryBarrier img_bar_flush = {};
							img_bar_flush.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
							img_bar_flush.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
							img_bar_flush.dstAccessMask = VK_ACCESS_NONE;
							img_bar_flush.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_flush.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_flush.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							img_bar_flush.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							img_bar_flush.pNext = nullptr;
							img_bar_flush.image = img_handle.m_handle;
							img_bar_flush.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							img_bar_flush.subresourceRange.baseArrayLayer = 0;
							img_bar_flush.subresourceRange.baseMipLevel = 0;
							img_bar_flush.subresourceRange.layerCount = 1;
							img_bar_flush.subresourceRange.levelCount = 1;

							if (res->resource_data.texture.attachment_type == trace::AttachmentType::DEPTH)
							{
								img_bar_invalidate.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
								img_bar_invalidate.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
								img_bar_invalidate.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
								img_bar_invalidate.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

								img_bar_flush.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
								img_bar_flush.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
								img_bar_flush.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


							}

							pass_handle->invalidate_image_barriers[i].push_back(img_bar_invalidate);
							pass_handle->flush_image_barriers[i].push_back(img_bar_flush);
						}
						trace::VKEvntPair evnt_pair = { edge.from, res };
						auto it = std::find_if(handle->events.begin(), handle->events.end(), [&evnt_pair](trace::VKEvntPair& val)
							{
								return val.pass == evnt_pair.pass && val.resource == evnt_pair.resource;
							});
						VkEvent& evnt = it->evnt;
						pass_handle->wait_events.push_back(evnt);
					}
				}
				else if (from && !to)
				{
					if (_from)
					{

					}
				}
				else if (!from && to)
				{
					if (_to && isTexture)
					{
						for (uint32_t i = 0; i < _device->frames_in_flight; i++)
						{
							trace::VKImage& img_handle = reinterpret_cast<trace::VKRenderGraphResource*>(res->render_handle.m_internalData)->resource[i].texture;
							VkImageMemoryBarrier img_bar_invalidate = {};
							img_bar_invalidate.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
							img_bar_invalidate.srcAccessMask = VK_ACCESS_NONE;
							img_bar_invalidate.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
							img_bar_invalidate.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_invalidate.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_invalidate.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							img_bar_invalidate.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							img_bar_invalidate.pNext = nullptr;
							img_bar_invalidate.image = img_handle.m_handle;
							img_bar_invalidate.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							img_bar_invalidate.subresourceRange.baseArrayLayer = 0;
							img_bar_invalidate.subresourceRange.baseMipLevel = 0;
							img_bar_invalidate.subresourceRange.layerCount = 1;
							img_bar_invalidate.subresourceRange.levelCount = 1;

							VkImageMemoryBarrier img_bar_flush = {};
							img_bar_flush.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
							img_bar_flush.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
							img_bar_flush.dstAccessMask = VK_ACCESS_NONE;
							img_bar_flush.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_flush.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
							img_bar_flush.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							img_bar_flush.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							img_bar_flush.pNext = nullptr;
							img_bar_flush.image = img_handle.m_handle;
							img_bar_flush.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							img_bar_flush.subresourceRange.baseArrayLayer = 0;
							img_bar_flush.subresourceRange.baseMipLevel = 0;
							img_bar_flush.subresourceRange.layerCount = 1;
							img_bar_flush.subresourceRange.levelCount = 1;


							if (res->resource_data.texture.attachment_type == trace::AttachmentType::DEPTH)
							{
								img_bar_invalidate.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
								img_bar_invalidate.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
								img_bar_invalidate.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
								img_bar_invalidate.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

								img_bar_flush.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
								img_bar_flush.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
								img_bar_flush.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


							}

							pass_handle->invalidate_image_barriers[i].push_back(img_bar_invalidate);
							pass_handle->flush_image_barriers[i].push_back(img_bar_flush);
						}
					}
				}
				
			}
			compute_frame_buffer_handle(pass, render_graph, _device, instance);
		}

		return result;
	}
	bool __DestroyRenderGraph(trace::GDevice* device, trace::RenderGraph* render_graph)
	{
		bool result = true;

		if (!device || !render_graph)
		{
			TRC_ERROR("Unable to build render graph please enter a valid pointer, {} || {}", (const void*)device, (const void*)render_graph);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData || !render_graph->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Unable to build render graph please enter a valid device or render_graph, {} || {}", (const void*)device->GetRenderHandle()->m_internalData, (const void*)render_graph->GetRenderHandle()->m_internalData);
			return false;
		}

		trace::VKRenderGraph* handle = reinterpret_cast<trace::VKRenderGraph*>(render_graph->GetRenderHandle()->m_internalData);
		trace::VKHandle* instance = reinterpret_cast<trace::VKHandle*>(handle->m_instance);
		trace::VKDeviceHandle* _device = reinterpret_cast<trace::VKDeviceHandle*>(device->GetRenderHandle()->m_internalData);

		std::vector<trace::RenderGraphResource>& resources = render_graph->GetResources();

		for (auto& evnt : handle->events)
		{
			vkDestroyEvent(
				_device->m_device,
				evnt.evnt,
				instance->m_alloc_callback
			);
		}

		for (auto& res : resources)
		{
			bool isTexture = res.resource_type == trace::RenderGraphResourceType::Texture;
			bool isSwapchainImage = res.resource_type == trace::RenderGraphResourceType::SwapchainImage;
			if (isTexture)
			{
				trace::VKRenderGraphResource* res_handle = reinterpret_cast<trace::VKRenderGraphResource*>(res.render_handle.m_internalData);

				for (uint32_t i = 0; i < _device->frames_in_flight; i++)
				{
					_DestroyImage(
						instance,
						_device,
						&res_handle->resource[i].texture
					);
					_DestroySampler(
						instance,
						_device,
						res_handle->resource[i].texture.m_sampler
					);
				}

				delete res_handle;
				res.render_handle.m_internalData = nullptr;
			}
			if (isSwapchainImage)
			{
			
			}

			
		}

		std::vector<uint32_t>& graph_passes = render_graph->GetSubmissionPasses();
		for (auto& pass_index : graph_passes)
		{
			trace::RenderGraphPass* pass = &render_graph->GetPass(pass_index);
			trace::VKRenderGraphPass* pass_handle = reinterpret_cast<trace::VKRenderGraphPass*>(pass->GetRenderHandle()->m_internalData);

			_DestroyRenderPass(
				instance,
				_device,
				&pass_handle->physical_pass
			);

			for (uint32_t i = 0; i < _device->frames_in_flight; i++)
			{
				vkDestroyFramebuffer(
					_device->m_device,
					pass_handle->frame_buffers[i],
					instance->m_alloc_callback
				);
			}
			delete pass_handle;
			pass->GetRenderHandle()->m_internalData = nullptr;
		}

		delete handle;
		render_graph->GetRenderHandle()->m_internalData = nullptr;

		return result;
	}
	bool __BeginRenderGraphPass(trace::RenderGraph* render_graph, trace::RenderGraphPass* pass)
	{
		bool result = true;

		if (!pass || !render_graph)
		{
			TRC_ERROR("Unable to begin render graph pass please enter a valid pointer, {} || {}", (const void*)pass, (const void*)render_graph);
			return false;
		}

		if (!pass->GetRenderHandle()->m_internalData || !render_graph->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Unable to begin render graph pass please enter a valid device or render_graph, {} || {}", (const void*)pass->GetRenderHandle()->m_internalData, (const void*)render_graph->GetRenderHandle()->m_internalData);
			return false;
		}

		trace::VKRenderGraph* handle = reinterpret_cast<trace::VKRenderGraph*>(render_graph->GetRenderHandle()->m_internalData);
		trace::VKRenderGraphPass* pass_handle = reinterpret_cast<trace::VKRenderGraphPass*>(pass->GetRenderHandle()->m_internalData);
		trace::VKHandle* instance = reinterpret_cast<trace::VKHandle*>(handle->m_instance);
		trace::VKDeviceHandle* device = reinterpret_cast<trace::VKDeviceHandle*>(handle->m_device);


		bool wait_evnt = !pass_handle->wait_events.empty();


		if (wait_evnt)
		{
			vkCmdWaitEvents(
				device->m_graphicsCommandBuffers[device->m_imageIndex].m_handle,
				static_cast<uint32_t>(pass_handle->wait_events.size()),
				pass_handle->wait_events.data(),
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				0,
				nullptr,
				0,
				nullptr,
				0,
				nullptr
			);
		}


		//if (!pass_handle->invalidate_image_barriers[device->m_imageIndex].empty())
		//{
		//	
		//	vkCmdPipelineBarrier(
		//		device->m_graphicsCommandBuffers[device->m_imageIndex].m_handle,
		//		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		//		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		//		0,
		//		0,
		//		nullptr,
		//		0,
		//		nullptr,
		//		static_cast<uint32_t>(pass_handle->invalidate_image_barriers[device->m_imageIndex].size()),
		//		pass_handle->invalidate_image_barriers[device->m_imageIndex].data()
		//	);
		//}


		uint32_t image_bar_count = 0;
		VkImageMemoryBarrier image_bars[10] = {};
		for (auto& edge : pass->GetPassEdges())
		{
			bool from = (pass == edge.from);
			bool to = (pass == edge.to);
			bool from_to = (edge.to && edge.from);


		}

		_BeginRenderPass(
			instance,
			device,
			&pass_handle->physical_pass,
			&device->m_graphicsCommandBuffers[device->m_imageIndex],
			pass_handle->frame_buffers[device->m_imageIndex]
		);

		return result;
	}
	bool __EndRenderGraphPass(trace::RenderGraph* render_graph, trace::RenderGraphPass* pass)
	{
		bool result = true;

		if (!pass || !render_graph)
		{
			TRC_ERROR("Unable to end render graph pass please enter a valid pointer, {} || {}", (const void*)pass, (const void*)render_graph);
			return false;
		}

		if (!pass->GetRenderHandle()->m_internalData || !render_graph->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Unable to end render graph pass please enter a valid device or render_graph, {} || {}", (const void*)pass->GetRenderHandle()->m_internalData, (const void*)render_graph->GetRenderHandle()->m_internalData);
			return false;
		}

		trace::VKRenderGraph* handle = reinterpret_cast<trace::VKRenderGraph*>(render_graph->GetRenderHandle()->m_internalData);
		trace::VKRenderGraphPass* pass_handle = reinterpret_cast<trace::VKRenderGraphPass*>(pass->GetRenderHandle()->m_internalData);
		trace::VKHandle* instance = reinterpret_cast<trace::VKHandle*>(handle->m_instance);
		trace::VKDeviceHandle* device = reinterpret_cast<trace::VKDeviceHandle*>(handle->m_device);

		_EndRenderPass(
			instance,
			device,
			&device->m_graphicsCommandBuffers[device->m_imageIndex]
		);


		//if (!pass_handle->flush_image_barriers[device->m_imageIndex].empty())
		//{
		//	vkCmdPipelineBarrier(
		//		device->m_graphicsCommandBuffers[device->m_imageIndex].m_handle,
		//		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		//		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		//		0,
		//		0,
		//		nullptr,
		//		0,
		//		nullptr,
		//		static_cast<uint32_t>(pass_handle->flush_image_barriers[device->m_imageIndex].size()),
		//		pass_handle->flush_image_barriers[device->m_imageIndex].data()
		//	);
		//}

		uint32_t image_bar_count = 0;
		VkImageMemoryBarrier image_bars[10] = {};
		for (auto& edge : pass->GetPassEdges())
		{
			bool from = (pass == edge.from);
			bool to = (pass == edge.to);
			bool from_to = (edge.to && edge.from);

			trace::RenderGraphResource* res = edge.resource;
			bool isTexture, isSwapchainImage;
			isTexture = res->resource_type == trace::RenderGraphResourceType::Texture;
			isSwapchainImage = res->resource_type == trace::RenderGraphResourceType::SwapchainImage;

			trace::VKImage& img_handle = reinterpret_cast<trace::VKRenderGraphResource*>(res->render_handle.m_internalData)->resource[device->m_imageIndex].texture;


			if (from)
			{
				VkImageMemoryBarrier img_bar = {};
				img_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				img_bar.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				img_bar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				img_bar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				img_bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				img_bar.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				img_bar.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				img_bar.pNext = nullptr;
				img_bar.image = img_handle.m_handle;
				img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				img_bar.subresourceRange.baseArrayLayer = 0;
				img_bar.subresourceRange.baseMipLevel = 0;
				img_bar.subresourceRange.layerCount = 1;
				img_bar.subresourceRange.levelCount = 1;
				image_bars[image_bar_count++] = img_bar;
			}
		}

		if (image_bar_count > 0)
		{
				vkCmdPipelineBarrier(
				device->m_graphicsCommandBuffers[device->m_imageIndex].m_handle,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0,
				nullptr,
				0,
				nullptr,
				image_bar_count,
				image_bars
			);
		}

		bool signal_evnt = !pass_handle->signal_events.empty();

		if (signal_evnt)
		{
			for (auto& evnt : pass_handle->signal_events)
			{
				vkCmdSetEvent(
					device->m_graphicsCommandBuffers[device->m_imageIndex].m_handle,
					evnt,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
				);
			}
		}

		return result;
	}
	bool __BeginRenderGraph(trace::RenderGraph* render_graph)
	{
		bool result = true;

		if (!render_graph)
		{
			TRC_ERROR("Unable to begin render graph please enter a valid pointer, {}", (const void*)render_graph);
			return false;
		}

		if (!render_graph->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Unable to begin render graph  please enter a valid device or render_graph, {}",  (const void*)render_graph->GetRenderHandle()->m_internalData);
			return false;
		}

		trace::VKRenderGraph* handle = reinterpret_cast<trace::VKRenderGraph*>(render_graph->GetRenderHandle()->m_internalData);
		trace::VKHandle* instance = reinterpret_cast<trace::VKHandle*>(handle->m_instance);
		trace::VKDeviceHandle* device = reinterpret_cast<trace::VKDeviceHandle*>(handle->m_device);


		return result;
	}
	bool __EndRenderGraph(trace::RenderGraph* render_graph)
	{
		bool result = true;

		if (!render_graph)
		{
			TRC_ERROR("Unable to end render graph please enter a valid pointer, {}", (const void*)render_graph);
			return false;
		}

		if (!render_graph->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Unable to end render graph  please enter a valid device or render_graph, {}", (const void*)render_graph->GetRenderHandle()->m_internalData);
			return false;
		}

		trace::VKRenderGraph* handle = reinterpret_cast<trace::VKRenderGraph*>(render_graph->GetRenderHandle()->m_internalData);
		trace::VKHandle* instance = reinterpret_cast<trace::VKHandle*>(handle->m_instance);
		trace::VKDeviceHandle* device = reinterpret_cast<trace::VKDeviceHandle*>(handle->m_device);


		for (auto& evnt : handle->events)
		{
			vkCmdResetEvent(
				device->m_graphicsCommandBuffers[device->m_imageIndex].m_handle,
				evnt.evnt,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
			);
		}


		return result;
	}
	bool __BindRenderGraphResource(trace::RenderGraph* render_graph, trace::GPipeline* pipeline, const std::string& bind_name, trace::ShaderResourceStage resource_stage, trace::RenderGraphResource* resource, uint32_t index)
	{
		bool result = true;

		if (!render_graph || !pipeline)
		{
			TRC_ERROR("Unable to bind render graph resource please enter a valid pointer, {} || {}", (const void*)render_graph, (const void*)pipeline );
			return false;
		}

		if (!render_graph->GetRenderHandle()->m_internalData || !pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Unable to bind render graph resource  please enter a valid render_graph or pipeline, {}", (const void*)render_graph->GetRenderHandle()->m_internalData, (const void*)pipeline->GetRenderHandle()->m_internalData);
			return false;
		}

		trace::VKRenderGraph* handle = reinterpret_cast<trace::VKRenderGraph*>(render_graph->GetRenderHandle()->m_internalData);
		trace::VKHandle* instance = reinterpret_cast<trace::VKHandle*>(handle->m_instance);
		trace::VKDeviceHandle* device = reinterpret_cast<trace::VKDeviceHandle*>(handle->m_device);
		trace::VKPipeline* pipe_handle = reinterpret_cast<trace::VKPipeline*>(pipeline->GetRenderHandle()->m_internalData);

		bool isTexture = (resource->resource_type == trace::RenderGraphResourceType::Texture);

		if (!isTexture)
		{
			TRC_WARN("Only texture resource can be binded 'for now' ");
			return false;
		}

		trace::VKRenderGraphResource* res_handle = reinterpret_cast<trace::VKRenderGraphResource*>(resource->render_handle.m_internalData);

		uint32_t hash_id = pipeline->_hashTable.Get(bind_name);

		if (hash_id == INVALID_ID)
		{
			TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized");
			return false;
		}

		if ((void*)pipe_handle->last_tex_update[device->m_imageIndex] == (void*)&res_handle->resource[device->m_imageIndex].texture)
			return false;



		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;


		trace::UniformMetaData& meta_data = pipeline->Scene_uniforms[hash_id];
		VkDescriptorImageInfo image_info = {};
		image_info.sampler = res_handle->resource[device->m_imageIndex].texture.m_sampler;
		image_info.imageView = res_handle->resource[device->m_imageIndex].texture.m_view;
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		write.descriptorCount = 1; //HACK: Fix
		write.dstBinding = meta_data._slot;
		write.pImageInfo = &image_info;
		write.dstArrayElement = index;
		switch (meta_data._resource_type)
		{
		case trace::ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER:
		{
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		}
		}

		switch (resource_stage)
		{


		case trace::ShaderResourceStage::RESOURCE_STAGE_GLOBAL:
		{
			write.dstSet = pipe_handle->Scene_sets[device->m_imageIndex];

			break;
		}

		case trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE:
		{
			write.dstSet = pipe_handle->Instance_sets[device->m_imageIndex];
			break;
		}

		case trace::ShaderResourceStage::RESOURCE_STAGE_LOCAL:
		{
			write.dstSet = pipe_handle->Local_sets[device->m_imageIndex];
			break;
		}


		}

		vkUpdateDescriptorSets(
			device->m_device,
			1,
			&write,
			0,
			nullptr
		);
		pipe_handle->last_tex_update[device->m_imageIndex] = (void*)&res_handle->resource[device->m_imageIndex].texture;

		return result;
	}


	bool compute_pass_handle(trace::RenderGraphPass* pass, trace::RenderGraph* render_graph, trace::VKDeviceHandle* _device, trace::VKHandle* instance)
	{
		bool result = true;

		std::vector<uint32_t>& outputs = pass->GetAttachmentOutputs();

		VkResult _result = VK_ERROR_UNKNOWN;
		VkRenderPass pass_handle = VK_NULL_HANDLE;
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference> attach_ref;
		VkAttachmentDescription depth_desc = {};
		VkAttachmentReference depth_ref = {};
		VkSubpassDescription subpass = {};


		for (uint32_t i = 0; i < static_cast<uint32_t>(outputs.size()); i++)
		{
			trace::RenderGraphResource* tex = &render_graph->GetResource(outputs[i]);
			VkAttachmentDescription att_desc = {};
			VkAttachmentReference att_ref = {};

			att_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			att_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			att_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			if (tex->resource_type == trace::RenderGraphResourceType::SwapchainImage)
			{
				trace::VKSwapChain* swapchain = reinterpret_cast<trace::VKSwapChain*>(tex->render_handle.m_internalData);
				att_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				att_desc.format = swapchain->m_format.format;
				att_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				att_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				att_desc.samples = VK_SAMPLE_COUNT_1_BIT; //TODO: Configurable
				att_ref.attachment = i;
				att_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				attachments.push_back(att_desc);
				attach_ref.push_back(att_ref);
			}

			if (tex->resource_type == trace::RenderGraphResourceType::Texture)
			{
				att_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				att_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				att_desc.format = convertFmt(tex->resource_data.texture.format);
				att_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				att_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				att_desc.samples = VK_SAMPLE_COUNT_1_BIT; //TODO: Configurable
				att_ref.attachment = i;
				att_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				attachments.push_back(att_desc);
				attach_ref.push_back(att_ref);
			}


		}

		subpass.colorAttachmentCount = static_cast<uint32_t>(attach_ref.size());
		subpass.pColorAttachments = attach_ref.data();
		if (pass->GetDepthStencilOutput() != INVALID_ID)
		{
			trace::RenderGraphResource* tex = &render_graph->GetResource(pass->GetDepthStencilOutput());
			depth_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depth_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depth_desc.format = convertFmt(tex->resource_data.texture.format);
			depth_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depth_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depth_desc.samples = VK_SAMPLE_COUNT_1_BIT; //TODO: Configurable
			depth_ref.attachment = static_cast<uint32_t>(attachments.size());
			depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachments.push_back(depth_desc);
			subpass.pDepthStencilAttachment = &depth_ref;
		}
		else if (pass->GetDepthStencilInput() != INVALID_ID)
		{
			trace::RenderGraphResource* tex = &render_graph->GetResource(pass->GetDepthStencilInput());
			depth_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depth_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
			depth_desc.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
			depth_desc.format = convertFmt(tex->resource_data.texture.format);
			depth_desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depth_desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_desc.samples = VK_SAMPLE_COUNT_1_BIT; //TODO: Configurable
			depth_ref.attachment = static_cast<uint32_t>(attachments.size());
			depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
			attachments.push_back(depth_desc);
			subpass.pDepthStencilAttachment = &depth_ref;
		}

		VkRenderPassCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		create_info.pAttachments = attachments.data();
		create_info.subpassCount = 1;
		create_info.pSubpasses = &subpass;
		create_info.pNext = nullptr;
		create_info.dependencyCount = 0;
		create_info.pDependencies = nullptr;

		_result = vkCreateRenderPass(
			_device->m_device,
			&create_info,
			instance->m_alloc_callback,
			&pass_handle
		);

		trace::VKRenderGraphPass* handle = reinterpret_cast<trace::VKRenderGraphPass*>(pass->GetRenderHandle()->m_internalData);

		handle->physical_pass.m_handle = pass_handle;
		handle->physical_pass.clear_color = &pass->clearColor;
		handle->physical_pass.render_area = &pass->renderArea;
		handle->physical_pass.depth_value = &pass->depthValue;
		handle->physical_pass.stencil_value = &pass->stencilValue;

		result = (_result == VK_SUCCESS);

		return result;
	}

	bool compute_resource_handle(trace::RenderGraph* render_graph, trace::VKDeviceHandle* _device, trace::VKHandle* instance)
	{
		bool result = true;

		std::vector<trace::RenderGraphResource>& resources = render_graph->GetResources();

		for (auto& res : resources)
		{
			bool isTexture = res.resource_type == trace::RenderGraphResourceType::Texture;
			bool isSwapchainImage = res.resource_type == trace::RenderGraphResourceType::SwapchainImage;
			if (isTexture)
			{
				trace::VKRenderGraphResource* res_handle = new trace::VKRenderGraphResource;
				res.render_handle.m_internalData = res_handle;
				VkImageUsageFlags image_usage = 0;
				VkImageAspectFlags aspect_flags = 0;
				VkMemoryPropertyFlags memory_property = 0;
				VkImageCreateFlags flags = 0;
				image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				aspect_flags |= VK_IMAGE_ASPECT_COLOR_BIT;
				if (res.resource_data.texture.attachment_type == trace::AttachmentType::DEPTH)
				{
					image_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
					aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
				}
				memory_property |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				
				

				VkImageViewCreateInfo create_info = {};
				create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				create_info.format = vk::convertFmt(res.resource_data.texture.format);
				create_info.viewType = vk::convertImageViewType(res.resource_data.texture.image_type); // TODO: Configurable view type
				create_info.subresourceRange.aspectMask = aspect_flags;
				create_info.subresourceRange.baseArrayLayer = 0; // TODO: Configurable array layer
				create_info.subresourceRange.baseMipLevel = 0; // TODO: Configurable
				create_info.subresourceRange.layerCount = 1;
				create_info.subresourceRange.levelCount = 1; // TODO: Configurable

				VkSamplerCreateInfo samp_create_info = {};
				samp_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samp_create_info.addressModeU = convertAddressMode(res.resource_data.texture.addressModeU);
				samp_create_info.addressModeV = convertAddressMode(res.resource_data.texture.addressModeV);
				samp_create_info.addressModeW = convertAddressMode(res.resource_data.texture.addressModeW);
				samp_create_info.anisotropyEnable = VK_TRUE;
				samp_create_info.maxAnisotropy = 16;
				samp_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
				samp_create_info.unnormalizedCoordinates = VK_FALSE;
				samp_create_info.minFilter = convertFilter(res.resource_data.texture.min);
				samp_create_info.magFilter = convertFilter(res.resource_data.texture.mag);
				/// TODO Check Docs for more info
				samp_create_info.compareEnable = VK_FALSE;
				samp_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
				samp_create_info.maxLod = 1.0f;
				samp_create_info.minLod = 0.0f;
				samp_create_info.mipLodBias = 0.0f;
				samp_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

				


				for (uint32_t i = 0; i < _device->frames_in_flight; i++)
				{
					vk::_CreateImage(
						instance,
						_device,
						&res_handle->resource[i].texture,
						vk::convertFmt(res.resource_data.texture.format),
						VK_IMAGE_TILING_OPTIMAL,
						image_usage,
						memory_property,
						aspect_flags,
						flags,
						vk::convertImageType(res.resource_data.texture.image_type),
						res.resource_data.texture.width,
						res.resource_data.texture.height,
						1,
						1
					);

					create_info.image = res_handle->resource[i].texture.m_handle;
					VkResult _result = (vkCreateImageView(_device->m_device, &create_info, instance->m_alloc_callback, &res_handle->resource[i].texture.m_view));				
					VK_ASSERT(_result);

					_result = vkCreateSampler(
						_device->m_device,
						&samp_create_info,
						instance->m_alloc_callback,
						&res_handle->resource[i].texture.m_sampler
					);
					VK_ASSERT(_result);
				}

			}
		}

		return result;
	}

	bool compute_frame_buffer_handle(trace::RenderGraphPass* pass, trace::RenderGraph* render_graph, trace::VKDeviceHandle* _device, trace::VKHandle* instance)
	{
		bool result = true;

		std::vector<uint32_t>& outputs = pass->GetAttachmentOutputs();
		trace::VKRenderGraphPass* handle = reinterpret_cast<trace::VKRenderGraphPass*>(pass->GetRenderHandle()->m_internalData);
		std::vector<VkImageView> attachments;

		VkFramebufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create_info.layers = 1;
		create_info.width = pass->renderArea.z;
		create_info.height = pass->renderArea.w;
		create_info.renderPass = handle->physical_pass.m_handle;
		
		VkResult _result = VK_ERROR_UNKNOWN;

		for (uint32_t i = 0; i < _device->frames_in_flight; i++)
		{
			for (auto& res_index : outputs)
			{
				trace::RenderGraphResource* res = &render_graph->GetResource(res_index);
				bool isTexture = res->resource_type == trace::RenderGraphResourceType::Texture;
				bool isSwapchainImage = res->resource_type == trace::RenderGraphResourceType::SwapchainImage;
				if (isTexture)
				{
					trace::VKRenderGraphResource* res_handle = reinterpret_cast<trace::VKRenderGraphResource*>(res->render_handle.m_internalData);
					attachments.push_back(res_handle->resource[i].texture.m_view);
				}
				if (isSwapchainImage)
				{
					trace::VKSwapChain* swapchain = reinterpret_cast<trace::VKSwapChain*>(res->render_handle.m_internalData);
					attachments.push_back(swapchain->m_imageViews[i]);
				}
			}

			if (pass->GetDepthStencilOutput() != INVALID_ID)
			{
				trace::VKRenderGraphResource* res_handle = reinterpret_cast<trace::VKRenderGraphResource*>(render_graph->GetResource(pass->GetDepthStencilOutput()).render_handle.m_internalData);
				attachments.push_back(res_handle->resource[i].texture.m_view);
			}
			else if (pass->GetDepthStencilInput() != INVALID_ID)
			{
				trace::VKRenderGraphResource* res_handle = reinterpret_cast<trace::VKRenderGraphResource*>(render_graph->GetResource(pass->GetDepthStencilInput()).render_handle.m_internalData);
				attachments.push_back(res_handle->resource[i].texture.m_view);
			}

			create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			create_info.pAttachments = attachments.data();

			_result = vkCreateFramebuffer(
				_device->m_device,
				&create_info,
				instance->m_alloc_callback,
				&handle->frame_buffers[i]
			);
			VK_ASSERT(_result);

			attachments.clear();
		}


		return result;
	}

}