#pragma once

class HealthGauge
{
public:
	HealthGauge();
	void Render(shared_ptr<Mesh> m, float deltaTime);
	Vector2 GetDesiredSize();

	// The fill rate of the gauge
	float rate = 0.5f;
	float colorBorder = 0.7f;
	Color upperColor = Colori(255, 102, 255);
	Color lowerColor = Colori(0, 204, 255);

	unique_ptr<Material> fillMaterial;
	unique_ptr<Material> baseMaterial;
	unique_ptr<Texture> frontTexture;
	unique_ptr<Texture> fillTexture;
	unique_ptr<Texture> backTexture;
	unique_ptr<Texture> maskTexture;
};