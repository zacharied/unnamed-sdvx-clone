#include "stdafx.h"
#include "HealthGauge.hpp"
#include "Application.hpp"

HealthGauge::HealthGauge()
{
}

void HealthGauge::Render(IMesh* m, float deltaTime)
{
	auto drawTextured = [&](Color c, unique_ptr<ITexture>& t)
	{
		Transform transform;
		MaterialParameterSet params;
		params.SetParameter("mainTex", t.get());
		params.SetParameter("color", c);
		g_application->GetRenderQueueBase()->Draw(transform, m, baseMaterial.get(), params);
	};

	if(backTexture)
		drawTextured(Color::White, backTexture);

	MaterialParameterSet params;
	params.SetParameter("mainTex", fillTexture.get());
	params.SetParameter("maskTex", maskTexture.get());
	params.SetParameter("rate", rate);

	Color color;
	if(rate >= colorBorder)
	{
		color = upperColor;
	}
	else
	{
		color = lowerColor;
	}
	params.SetParameter("barColor", color);
	Transform trans;
	g_application->GetRenderQueueBase()->Draw(trans, m, fillMaterial.get(), params);

	//// Draw frame last
	drawTextured(Color::White, frontTexture);
}

Vector2 HealthGauge::GetDesiredSize()
{
	//return GUISlotBase::ApplyFill(FillMode::Fit, frontTexture->GetSize(), rd.area).size;
	return Vector2();
}
