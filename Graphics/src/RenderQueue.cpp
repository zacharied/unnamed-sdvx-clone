#include "stdafx.h"
#include "RenderQueue.hpp"
#include "OpenGL.hpp"
using Utility::Cast;

namespace Graphics
{
	RenderQueue::RenderQueue(const RenderState& rs)
	{
		m_renderState = rs;
	}

	RenderQueue::RenderQueue(RenderQueue&& other) noexcept
	{
		m_orderedCommands = move(other.m_orderedCommands);
		m_renderState = other.m_renderState;
	}

	RenderQueue& RenderQueue::operator=(const RenderQueue& other)
	{
		m_renderState = other.m_renderState;
		return *this;
	}

	RenderQueue& RenderQueue::operator=(RenderQueue&& other) noexcept
	{
		m_orderedCommands = move(other.m_orderedCommands);
		m_renderState = other.m_renderState;
		return *this;
	}

	void RenderQueue::Process(bool clearQueue)
	{
		bool scissorEnabled = false;
		bool blendEnabled = false;
		auto activeBlendMode = (MaterialBlendMode)-1;

		Set<IMaterial*> initializedShaders;
		IMesh* currentMesh = nullptr;
		IMaterial* currentMaterial = nullptr;

		// Create a new list of items
		for(auto& item : m_orderedCommands)
		{
			auto SetupMaterial = [&](IMaterial* mat, MaterialParameterSet& params)
			{
				// Only bind params if material is already bound to context
				if(currentMaterial == mat)
					mat->BindParameters(params, m_renderState.worldTransform);
				else
				{
					if(initializedShaders.Contains(mat))
					{
						// Only bind params and rebind
						mat->BindParameters(params, m_renderState.worldTransform);
						mat->BindToContext();
						currentMaterial = mat;
					}
					else
					{
						mat->Bind(m_renderState, params);
						initializedShaders.Add(mat);
						currentMaterial = mat;
					}
				}

				// Setup Render state for transparent object
				if(mat->GetOpaque())
				{
					if(blendEnabled)
					{
						glDisable(GL_BLEND);
						blendEnabled = false;
					}
				}
				else
				{
					if(!blendEnabled)
					{
						glEnable(GL_BLEND);
						blendEnabled = true;
					}
					if(activeBlendMode != mat->blendMode)
					{
						switch(mat->blendMode)
						{
						case MaterialBlendMode::Normal:
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							break;
						case MaterialBlendMode::Additive:
							glBlendFunc(GL_SRC_ALPHA, GL_ONE);
							break;
						case MaterialBlendMode::Multiply:
							glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
							break;
						}
					}
				}
			};

			// Draw mesh helper
			auto DrawOrRedrawMesh = [&](IMesh* mesh)
			{
				if(currentMesh == mesh)
					mesh->Redraw();
				else
				{
					mesh->Draw();
					currentMesh = mesh;
				}
			};

			if(auto sdc = dynamic_cast<SimpleDrawCall*>(item.get()); sdc)
			{
				m_renderState.worldTransform = sdc->worldTransform;
				SetupMaterial(sdc->mat, sdc->params);

				// Check if scissor is enabled
				bool useScissor = (sdc->scissorRect.size.x >= 0);
				if(useScissor)
				{
					// Apply scissor
					if(!scissorEnabled)
					{
						glEnable(GL_SCISSOR_TEST);
						scissorEnabled = true;
					}
					float scissorY = m_renderState.viewportSize.y - sdc->scissorRect.Bottom();
					glScissor((int32)sdc->scissorRect.Left(), (int32)scissorY,
						(int32)sdc->scissorRect.size.x, (int32)sdc->scissorRect.size.y);
				}
				else
				{
					if(scissorEnabled)
					{
						glDisable(GL_SCISSOR_TEST);
						scissorEnabled = false;
					}
				}

				DrawOrRedrawMesh(sdc->mesh);
			}
			else if(auto pdc = dynamic_cast<PointDrawCall*>(item.get()); pdc)
			{
				if(scissorEnabled)
				{
					// Disable scissor
					glDisable(GL_SCISSOR_TEST);
					scissorEnabled = false;
				}

				m_renderState.worldTransform = Transform();
				SetupMaterial(pdc->mat, pdc->params);
				PrimitiveType pt = pdc->mesh->GetPrimitiveType();

				if(pt >= PrimitiveType::LineList && pt <= PrimitiveType::LineStrip)
					glLineWidth(pdc->size);
				else
					glPointSize(pdc->size);
				
				DrawOrRedrawMesh(pdc->mesh);
			}
		}

		// Disable all states that were on
		glDisable(GL_BLEND);
		glDisable(GL_SCISSOR_TEST);

		if(clearQueue)
			m_orderedCommands.clear();
	}

	void RenderQueue::Draw(const Transform& worldTransform, IMesh* m, IMaterial* mat, const MaterialParameterSet& params)
	{
		auto sdc = make_unique<SimpleDrawCall>();
		sdc->mat = mat;
		sdc->mesh = m;
		sdc->params = params;
		sdc->worldTransform = worldTransform;
		m_orderedCommands.push_back(std::move(sdc));
	}

	void RenderQueue::Draw(const Transform& worldTransform, IText* text, IMaterial* mat, const MaterialParameterSet& params)
	{
		auto sdc = make_unique<SimpleDrawCall>();
		sdc->mat = mat;
		sdc->mesh = text->GetMesh();
		sdc->params = params;
		// Set Font texture map
		sdc->params.SetParameter("mainTex", text->GetTexture());
		sdc->worldTransform = worldTransform;
		m_orderedCommands.push_back(std::move(sdc));
	}

	void RenderQueue::DrawScissored(Rect scissor, const Transform& worldTransform, IMesh* m, IMaterial* mat, const MaterialParameterSet& params)
	{
		auto sdc = make_unique<SimpleDrawCall>();
		sdc->mat = mat;
		sdc->mesh = m;
		sdc->params = params;
		sdc->worldTransform = worldTransform;
		sdc->scissorRect = scissor;
		m_orderedCommands.push_back(std::move(sdc));
	}

	void RenderQueue::DrawScissored(Rect scissor, const Transform& worldTransform, IText* text, IMaterial* mat, const MaterialParameterSet& params)
	{
		auto sdc = make_unique<SimpleDrawCall>();
		sdc->mat = mat;
		sdc->mesh = text->GetMesh();
		sdc->params = params;
		// Set Font texture map
		sdc->params.SetParameter("mainTex", text->GetTexture());
		sdc->worldTransform = worldTransform;
		sdc->scissorRect = scissor;
		m_orderedCommands.push_back(std::move(sdc));
	}

	void RenderQueue::DrawPoints(IMesh* m, IMaterial* mat, const MaterialParameterSet& params, float pointSize)
	{
		auto pdc = make_unique<PointDrawCall>();
		pdc->mat = mat;
		pdc->mesh = m;
		pdc->params = params;
		pdc->size = pointSize;
		m_orderedCommands.push_back(std::move(pdc));
	}
}
