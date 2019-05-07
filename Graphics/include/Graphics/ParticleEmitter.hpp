#pragma once

#include <Graphics/Material.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Particle.hpp>
#include <Graphics/ParticleParameter.hpp>

namespace Graphics
{
	/*
		Particle Emitter, which is a component of a particle system that handles the emission of particles together with the properties of the emitter particles
	*/
	class ParticleEmitter
	{
	public:
		// Constructed by particle system
		ParticleEmitter();
		~ParticleEmitter();

		// Material used for the particle
		IMaterial* material;

		// Texture to use for the particle
		ITexture* texture;

		// Emitter location
		Vector3 position;

		// Emitter duration
		float duration = 5.0f;

		float scale = 1.0f;

		// Amount of loops to make
		// 0 = forever
		uint32 loops = 0;

		// Particle parameter accessors
		void Set_Lifetime(IParticleParameter<float>* param)
		{
			delete m_param_Lifetime;
			m_param_Lifetime = param;
		}

		void Set_FadeOverTime(IParticleParameter<float>* param)
		{
			delete m_param_FadeOverTime;
			m_param_FadeOverTime = param;
		}

		void Set_ScaleOverTime(IParticleParameter<float>* param)
		{
			delete m_param_ScaleOverTime;
			m_param_ScaleOverTime = param;
		}

		void Set_StartColor(IParticleParameter<Color>* param)
		{
			delete m_param_StartColor;
			m_param_StartColor = param;
		}

		void Set_StartVelocity(IParticleParameter<Vector3>* param)
		{
			delete m_param_StartVelocity;
			m_param_StartVelocity = param;
		}

		void Set_StartSize(IParticleParameter<float>* param)
		{
			delete m_param_StartSize;
			m_param_StartSize = param;
		}

		void Set_StartRotation(IParticleParameter<float>* param)
		{
			delete m_param_StartRotation;
			m_param_StartRotation = param;
		}

		void Set_StartPosition(IParticleParameter<Vector3>* param)
		{
			delete m_param_StartPosition;
			m_param_StartPosition = param;
		}

		void Set_StartDrag(IParticleParameter<float>* param)
		{
			delete m_param_StartDrag;
			m_param_StartDrag = param;
		}

		void Set_Gravity(IParticleParameter<Vector3>* param)
		{
			delete m_param_Gravity;
			m_param_Gravity = param;
		}

		void Set_SpawnVelocityScale(IParticleParameter<float>* param)
		{
			delete m_param_SpawnVelocityScale;
			m_param_SpawnVelocityScale = param;
		}

		void Set_SpawnRate(IParticleParameter<float>* param)
		{
			delete m_param_SpawnRate;
			m_param_SpawnRate = param;
		}

		// True after all loops are done playing
		bool HasFinished() const
		{
			return m_finished;
		}

		// Restarts a particle emitter
		void Reset();

		// Stop spawning any particles
		void Deactivate();

		// render
		void Render(const RenderState& rs, float deltaTime);

	private:
		void m_ReallocatePool(uint32 newCapacity);

		float m_spawnCounter = 0;
		float m_emitterTime = 0;
		float m_emitterRate;
		bool m_deactivated = false;
		bool m_finished = false;
		uint32 m_emitterLoopIndex = 0;

		friend class Particle;
		class Particle* m_particles = nullptr;

		uint32 m_poolSize = 0;

		// Particle parameters private
		IParticleParameter<float>* m_param_Lifetime;
		IParticleParameter<float>* m_param_FadeOverTime;
		IParticleParameter<float>* m_param_ScaleOverTime;
		IParticleParameter<Color>* m_param_StartColor;
		IParticleParameter<Vector3>* m_param_StartVelocity;
		IParticleParameter<float>* m_param_StartSize;
		IParticleParameter<float>* m_param_StartRotation;
		IParticleParameter<Vector3>* m_param_StartPosition;
		IParticleParameter<float>* m_param_StartDrag;
		IParticleParameter<Vector3>* m_param_Gravity;
		IParticleParameter<float>* m_param_SpawnVelocityScale;
		IParticleParameter<float>* m_param_SpawnRate;
	};
}