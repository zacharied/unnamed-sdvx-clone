#pragma once
#include <Graphics/IParticleSystem.hpp>
#include <Graphics/ParticleEmitter.hpp>

namespace Graphics
{
	/*
		Particles system
		contains emitters and handles the cleanup/lifetime of them.
	*/
	class ParticleSystem : public IParticleSystem
	{
	public:
		~ParticleSystem() override = default;

		static auto Create() -> optional<unique_ptr<ParticleSystem>>;

		// Create a new emitter
		void AddEmitter(ParticleEmitter* emitter) override;
		void Render(const RenderState& rs, float deltaTime) override;
		// Removes all active particle systems
		void Reset() override;

	private:
		Vector<ParticleEmitter*> m_emitters;
	};
}