#pragma once

class HealthGauge
{
public:
	HealthGauge();
	void Render(IMesh* m, float deltaTime);
	Vector2 GetDesiredSize();

	// The fill rate of the gauge
	float rate = 0.5f;
	float colorBorder = 0.7f;
	Color upperColor = Colori(255, 102, 255);
	Color lowerColor = Colori(0, 204, 255);

	unique_ptr<IMaterial> fillMaterial;
	unique_ptr<IMaterial> baseMaterial;
	unique_ptr<ITexture> frontTexture;
	unique_ptr<ITexture> fillTexture;
	unique_ptr<ITexture> backTexture;
	unique_ptr<ITexture> maskTexture;
};