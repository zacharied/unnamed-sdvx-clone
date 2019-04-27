#include "ParticleEmitter.hpp"
#include <Graphics/Mesh.hpp>

namespace Graphics
{
	ParticleEmitter::ParticleEmitter()
	{
		Set_Lifetime(new PPRandomRange<float>(0.6f, 0.8f));
		Set_ScaleOverTime(new PPRange<float>(1.0f, 0.8f));
		Set_FadeOverTime(new PPRange<float>(1, 0));
		Set_StartColor(new PPConstant<Color>(Color::White));
		Set_StartVelocity(new PPConstant<Vector3>(Vector3{0.0f}));
		Set_SpawnVelocityScale(new PPRandomRange<float>(0.8f, 1.0f));
		Set_StartSize(new PPRandomRange<float>(0.5, 1));
		Set_StartRotation(new PPRandomRange<float>(0, Math::pi * 2));
		Set_StartPosition(new PPSphere(0.2f));
		Set_StartDrag(new PPConstant<float>(0));
		Set_Gravity(new PPConstant<Vector3>({0, 0, 0}));
		Set_SpawnRate(new PPConstant<float>(20));
	}

	ParticleEmitter::~ParticleEmitter()
	{
		// Cleanup particle parameters
		delete[] m_particles;
	}

	void ParticleEmitter::m_ReallocatePool(uint32 newCapacity)
	{
		Particle* oldParticles = m_particles;
		uint32 oldSize = m_poolSize;

		// Create new pool
		m_poolSize = newCapacity;
		if (newCapacity > 0)
		{
			m_particles = new Particle[m_poolSize];
			memset(m_particles, 0, m_poolSize * sizeof(Particle));
		}
		else
		{
			m_particles = nullptr;
		}

		if (oldParticles && m_particles)
		{
			memcpy(m_particles, oldParticles, Math::Min(oldSize, m_poolSize) * sizeof(Particle));
		}

		delete[] oldParticles;
	}

	void ParticleEmitter::Render(const RenderState& rs, float deltaTime)
	{
		if (m_finished)
			return;

		auto maxDuration = (uint32) ceilf(m_param_Lifetime->GetMax());
		auto maxSpawns = (uint32) ceilf(m_param_SpawnRate->GetMax());
		uint32 maxParticles = maxSpawns * maxDuration;
		// Round up to 64
		maxParticles = (uint32) ceil((float) maxParticles / 64.0f) * 64;

		if (maxParticles > m_poolSize)
			m_ReallocatePool(maxParticles);

		// Resulting vertex bufffer
		Vector<ParticleVertex> verts;

		// Increment emitter time
		m_emitterTime += deltaTime;
		while (m_emitterTime > duration)
		{
			m_emitterTime -= duration;
			m_emitterLoopIndex++;
		}
		m_emitterRate = m_emitterTime / duration;

		// Increment spawn counter
		m_spawnCounter += deltaTime * m_param_SpawnRate->Sample(m_emitterRate);

		uint32 numSpawns = 0;
		float spawnTimeOffset = 0.0f;
		float spawnTimeOffsetStep = 0;
		if (loops > 0 && m_emitterLoopIndex >= loops) // Should spawn particles ?
			m_deactivated = true;

		if (!m_deactivated)
		{
			// Calculate number of new particles to spawn
			float spawnsf;
			m_spawnCounter = modff(m_spawnCounter, &spawnsf);
			numSpawns = (uint32) spawnsf;
			spawnTimeOffsetStep = deltaTime / spawnsf;
		}

		bool updatedSomething = false;
		for (uint32 i = 0; i < m_poolSize; i++)
		{
			Particle& p = m_particles[i];

			bool render = false;
			if (!m_particles[i].IsAlive())
			{
				// Try to spawn a new particle in this slot
				if (numSpawns > 0)
				{
					p.Init(this);
					p.Simulate(this, spawnTimeOffset);
					spawnTimeOffset += spawnTimeOffsetStep;
					numSpawns--;
					render = true;
				}
			}
			else
			{
				p.Simulate(this, deltaTime);
				render = true;
				updatedSomething = true;
			}

			if (render)
				verts.Add(ParticleVertex{p.pos, Color{p.startColor, p.fade}, Vector4(p.startSize * p.scale, p.rotation, 0, 0)});
		}

		if (m_deactivated)
		{
			m_finished = !updatedSomething;
		}

		MaterialParameterSet params;
		if (texture)
		{
			params.SetParameter("mainTex", texture);
		}
		material->Bind(rs, params);

		// Select blending mode based on material
		switch (material->blendMode)
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

		// Create vertex buffer
		auto mesh = Mesh::Create();
		assert(mesh); // TODO: (factory)

		((IMesh*)(*mesh).get())->SetData(verts);
		(*mesh)->SetPrimitiveType(PrimitiveType::PointList);

		(*mesh)->Draw();
	}

	void ParticleEmitter::Reset()
	{
		m_deactivated = false;
		m_finished = false;
		delete[] m_particles;
		m_particles = nullptr;
		m_emitterLoopIndex = 0;
		m_emitterTime = 0;
		m_spawnCounter = 0;
		m_poolSize = 0;
	}

	void ParticleEmitter::Deactivate()
	{
		m_deactivated = true;
	}
}