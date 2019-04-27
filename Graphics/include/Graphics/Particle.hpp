#pragma once

#include <Shared/Color.hpp>
#include <Graphics/VertexFormat.hpp>
#include "ParticleEmitter.hpp"

namespace Graphics
{
	struct ParticleVertex : VertexFormat<Vector3, Vector4, Vector4>
	{
		Vector3 pos;
		Color color;
		// X = scale
		// Y = rotation
		// Z = animation frame
		Vector4 params;

		ParticleVertex(Vector3 pos, Color color, Vector4 params)
			: pos(pos), color(color), params(params)
		{};
	};

	// Particle instance class
	class Particle
	{
	public:
		float life = 0.0f;
		float maxLife = 0.0f;
		float rotation = 0.0f;
		float startSize = 0.0f;
		Color startColor;
		Vector3 pos;
		Vector3 velocity;
		float scale;
		float fade;
		float drag;

		bool IsAlive() const;
		void Init(class ParticleEmitter* emitter);
		void Simulate(class ParticleEmitter* emitter, float deltaTime);
	};
}