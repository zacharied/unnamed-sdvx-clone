#pragma once

class HealthGauge
{
public:
	HealthGauge();
	void Render(shared_ptr<Mesh>& m, float deltaTime);
	Vector2 GetDesiredSize();

	// The fill rate of the gauge
	float rate = 0.5f;
	float colorBorder = 0.7f;
	Color upperColor = Colori(255, 102, 255);
	Color lowerColor = Colori(0, 204, 255);

	shared_ptr<Material> fillMaterial;
	shared_ptr<Material> baseMaterial;
	shared_ptr<Texture> frontTexture;
	shared_ptr<Texture> fillTexture;
	shared_ptr<Texture> backTexture;
	shared_ptr<Texture> maskTexture;
};