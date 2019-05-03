#pragma once

#include "ParticleEmitter.hpp"

namespace Graphics
{
	/*
		Particles system
		contains emitters and handles the cleanup/lifetime of them.
	*/
	class IParticleSystem
	{
	public:
		virtual ~IParticleSystem() = default;

		// Create a new emitter
		virtual void AddEmitter(ParticleEmitter* emitter) = 0;
		virtual void Render(const RenderState& rs, float deltaTime) = 0;
		// Removes all active particle systems
		virtual void Reset() = 0;
	};
}