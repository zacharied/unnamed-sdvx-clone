#pragma once
#include <Graphics/IMesh.hpp>
#include <Graphics/IMaterial.hpp>
#include <Graphics/ITexture.hpp>
#include <Graphics/IFont.hpp>

namespace Graphics
{
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
		IMesh* mesh;
		// Material to use
		IMaterial* mat;
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
		IMesh* mesh;
		IMaterial* mat;
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
		void Draw(const Transform& worldTransform, IMesh* m, IMaterial* mat, const MaterialParameterSet& params = MaterialParameterSet());
		void Draw(const Transform& worldTransform, IText* text, IMaterial* mat, const MaterialParameterSet& params = MaterialParameterSet());
		void DrawScissored(Rect scissor, const Transform& worldTransform, IMesh* m, IMaterial* mat, const MaterialParameterSet& params = MaterialParameterSet());
		void DrawScissored(Rect scissor, const Transform& worldTransform, IText* text, IMaterial* mat, const MaterialParameterSet& params = MaterialParameterSet());

		// Draw for lines/points with point size parameter
		void DrawPoints(IMesh* m, IMaterial* mat, const MaterialParameterSet& params, float pointSize);

	private:
		RenderState m_renderState;
		Vector<unique_ptr<RenderQueueItem>> m_orderedCommands;
	};
}