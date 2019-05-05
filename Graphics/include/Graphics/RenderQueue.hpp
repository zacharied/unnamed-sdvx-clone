#pragma once
#include <Graphics/Mesh.hpp>
#include <Graphics/Material.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Font.hpp>

namespace Graphics
{

	using Shared::Rect;
	/*
		Represents a draw command that can be executed in a render queue
	*/
	class RenderQueueItem
	{
	public:
		virtual ~RenderQueueItem() = default;
	};

	// Most basic draw command that only contains a material, it's parameters and a world transform
	class SimpleDrawCall : public RenderQueueItem
	{
	public:
		SimpleDrawCall()
				: scissorRect(Vector2(), Vector2(-1))
		{}

		// The mesh to draw
		shared_ptr<IMesh> mesh;
		// Material to use
		shared_ptr<IMaterial> mat;
		MaterialParameterSet params;
		// The world transform
		Transform worldTransform; 
		// Scissor rectangle
		Rect scissorRect;
	};

	// Command for points/lines with size/width parameter
	class PointDrawCall : public RenderQueueItem
	{
	public:
		// List of points/lines
		shared_ptr<IMesh> mesh;
		shared_ptr<IMaterial> mat;
		MaterialParameterSet params;
		float size;
	};

	/*
		This class is a queue that collects draw commands
		each of these is stored together with their wanted render state.

		When Process is called, the commands are sorted and grouped, then sent to the graphics pipeline.
	*/
	class RenderQueue : public Unique
	{
	public:
		RenderQueue() = default;
		explicit RenderQueue(const RenderState& rs);
		RenderQueue(RenderQueue&& other) noexcept;
		RenderQueue& operator=(const RenderQueue& other);
		RenderQueue& operator=(RenderQueue&& other) noexcept;
		~RenderQueue() = default;

		// Processes all render commands
		void Process(bool clearQueue = true);
		// Clears all the render commands in the queue
		void Draw(const Transform& worldTransform, shared_ptr<Mesh>& m,    shared_ptr<Material>& mat, const MaterialParameterSet& params = MaterialParameterSet());
		void Draw(const Transform& worldTransform, shared_ptr<Text>& text, shared_ptr<Material>& mat, const MaterialParameterSet& params = MaterialParameterSet());
		void DrawScissored(Rect scissor, const Transform& worldTransform, shared_ptr<Mesh>& m, shared_ptr<Material>& mat, const MaterialParameterSet& params = MaterialParameterSet());
		void DrawScissored(Rect scissor, const Transform& worldTransform, shared_ptr<Text>& text, shared_ptr<Material>& mat, const MaterialParameterSet& params = MaterialParameterSet());

		// Draw for lines/points with point size parameter
		void DrawPoints(shared_ptr<Mesh>& m, shared_ptr<Material>& mat, const MaterialParameterSet& params, float pointSize);

	private:
		RenderState m_renderState;
		Vector<unique_ptr<RenderQueueItem>> m_orderedCommands;
	};
}