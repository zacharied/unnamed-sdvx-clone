#include "stdafx.h"
#include "OpenGL.hpp"
#include "ParticleSystem.hpp"
#include "Mesh.hpp"

namespace Graphics
{
	auto ParticleSystem::Create() -> optional<unique_ptr<ParticleSystem>>
	{
		struct EnableMaker : public ParticleSystem { using ParticleSystem::ParticleSystem; };
		return make_unique<EnableMaker>();
	}

	void ParticleSystem::Render(const RenderState& rs, float deltaTime)
	{
		// Enable blending for all particles
		glEnable(GL_BLEND);

		// Tick all emitters and remove old ones
		for(auto it = m_emitters.begin(); it != m_emitters.end();)
		{
			(*it)->Render(rs, deltaTime);

			if((*it)->HasFinished())
			{
				// Remove unreferenced and finished emitters
				it = m_emitters.erase(it);
				continue;
			}
			else if((*it)->loops == 0)
			{
				// Deactivate unreferenced infinte duration emitters
				(*it)->Deactivate();
			}

			it++;
		}
	}

	void ParticleSystem::AddEmitter(ParticleEmitter* emitter)
	{
		m_emitters.Add(emitter);
	}

	void ParticleSystem::Reset()
	{
		m_emitters.clear();
	}
}
