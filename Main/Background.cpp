#include "stdafx.h"
#include "Background.hpp"
#include "Application.hpp"
#include "Background.hpp"
#include "Game.hpp"
#include "Track.hpp"
#include "Camera.hpp"

/* Background template for fullscreen effects */
class FullscreenBackground : public Background
{
public:

	virtual void SetAudioSampler(float* sampleBuffer, uint32 bufferLength) override
	{
		double channels = 3.0;
		if (!sampleBuffer)
			return;
		
		for (uint32 i = 0; i < fftArraySize; i++)
		{
			amplitudeArray[i / 2].w = amplitudeArray[i / 2].z;
			amplitudeArray[i / 2].z = amplitudeArray[i / 2].y;
			amplitudeArray[i / 2].y = amplitudeArray[i / 2].x;
			//amplitudeArray[i / 2].x = 0;
			fftArray[i] = 0;
		}

		for (uint32 k = 0; k < bufferLength / (channels); k++) {
			float window = -.5*cos(2.*Math::pi*(double)k / ((double)bufferLength / channels)) + .5;
			fftArray[k*2] = sampleBuffer[k] * window;
			//fftArray[2 * k + 1] = 0.;
		}
		smbFft(fftArray, bufferLength / channels,-1);
		for (uint32 i = 0; i < bufferLength / (channels * 4); i++)
		{
			float ampl = (sqrt(pow(fftArray[2 * i], 2) + pow(fftArray[2 * i + 1], 2))) * 1;
			ampl = 100 * (log10(ampl)+1);
			amplitudeArray[i].x = (uint8)Math::Clamp(ampl, 0.0f, 255.0f);
			amplitudeArray[i].x = (amplitudeArray[i].x + amplitudeArray[i].y + amplitudeArray[i].z + amplitudeArray[i].w) / 4.0f;
		}
	}

	void swap(float & a, float & b)
	{
		int temp = a;
		a = b;
		b = temp;
	}


	void smbFft(float *fftBuffer, long fftFrameSize, long sign)
		/*
		FFT routine, (C)1996 S.M.Bernsee. Sign = -1 is FFT, 1 is iFFT (inverse)
		Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
		time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes
		and returns the cosine and sine parts in an interleaved manner, ie.
		fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. fftFrameSize
		must be a power of 2. It expects a complex input signal (see footnote 2),
		ie. when working with 'common' audio signals our input signal has to be
		passed as {in[0],0.,in[1],0.,in[2],0.,...} asf. In that case, the transform
		of the frequencies of interest is in fftBuffer[0...fftFrameSize].
		*/
	{
		float wr, wi, arg, *p1, *p2, temp;
		float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
		long i, bitm, j, le, le2, k;

		for (i = 2; i < 2 * fftFrameSize - 2; i += 2) {
			for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) {
				if (i & bitm) j++;
				j <<= 1;
			}
			if (i < j) {
				p1 = fftBuffer + i; p2 = fftBuffer + j;
				temp = *p1; *(p1++) = *p2;
				*(p2++) = temp; temp = *p1;
				*p1 = *p2; *p2 = temp;
			}
		}
		for (k = 0, le = 2; k < (long)(log(fftFrameSize) / log(2.) + .5); k++) {
			le <<= 1;
			le2 = le >> 1;
			ur = 1.0;
			ui = 0.0;
			arg = Math::pi / (le2 >> 1);
			wr = cos(arg);
			wi = sign*sin(arg);
			for (j = 0; j < le2; j += 2) {
				p1r = fftBuffer + j; p1i = p1r + 1;
				p2r = p1r + le2; p2i = p2r + 1;
				for (i = j; i < 2 * fftFrameSize; i += le) {
					tr = *p2r * ur - *p2i * ui;
					ti = *p2r * ui + *p2i * ur;
					*p2r = *p1r - tr; *p2i = *p1i - ti;
					*p1r += tr; *p1i += ti;
					p1r += le; p1i += le;
					p2r += le; p2i += le;
				}
				tr = ur*wr - ui*wi;
				ui = ur*wi + ui*wr;
				ur = tr;
			}
		}
	}

	virtual bool Init() override
	{
		memset(fftArray, 0, fftArraySize * sizeof(float));
		memset(amplitudeArray, 0, fftArraySize * sizeof(Colori) / 2);
		fullscreenMesh = MeshGenerators::Quad(g_gl, Vector2(-1.0f), Vector2(2.0f));
		return true;
	}
	void UpdateRenderState(float deltaTime)
	{
		renderState = g_application->GetRenderStateBase();
	}
	virtual void Render(float deltaTime) override
	{
		assert(fullscreenMaterial);

		// Render a fullscreen quad
		RenderQueue rq(g_gl, renderState);
		rq.Draw(Transform(), fullscreenMesh, fullscreenMaterial, fullscreenMaterialParams);
		rq.Process();
	}

	RenderState renderState;
	Mesh fullscreenMesh;
	Material fullscreenMaterial;
	Texture side;
	MaterialParameterSet fullscreenMaterialParams;
	const static uint32 fftArraySize = 2048;
	float fftArray[fftArraySize];
	float clearTransition = 0.0f;
	Colori amplitudeArray[fftArraySize / 2];

};

class TestBackground : public FullscreenBackground
{
	virtual bool Init() override
	{
		if(!FullscreenBackground::Init())
			return false;

		CheckedLoad(fullscreenMaterial = g_application->LoadMaterial("background"));
		CheckedLoad(side = g_application->LoadTexture("side_1.png"));

		return true;
	}
	virtual void Render(float deltaTime) override
	{
		UpdateRenderState(deltaTime);

		Vector3 timing;
		const TimingPoint& tp = game->GetPlayback().GetCurrentTimingPoint();
		timing.x = game->GetPlayback().GetBarTime();
		timing.z = game->GetPlayback().GetLastTime() / 1000.0f;
		bool cleared = game->GetScoring().currentGauge >= 0.75f;

		if (cleared)
			clearTransition += deltaTime / tp.beatDuration * 1000;
		else
			clearTransition -= deltaTime / tp.beatDuration * 1000;

		clearTransition = Math::Clamp(clearTransition, 0.0f, 1.0f);
		// every 1/4 tick
		float tickTime = fmodf(timing.x * (float)tp.numerator, 1.0f);
		//timing.y = powf(tickTime, 2);
		timing.y = powf(1.0f-tickTime, 1);
		//if(tickTime > 0.7f)
		//	timing.y += ((tickTime - 0.7f) / 0.3f) * 0.8f; // Gradual build up again
		Texture fft = TextureRes::Create(g_gl);
		
		fft->Init(Vector2i(1024, 1), Graphics::TextureFormat::RGBA8);
		fft->SetData(Vector2i(1024, 1), amplitudeArray);
		Vector3 trackEndWorld = Vector3(0.0f, 25.0f, 0.0f);
		Vector2i screenCenter = game->GetCamera().Project(trackEndWorld);
		float tilt = game->GetCamera().GetRoll();
		float test = amplitudeArray[2].x / 255.0f;

		fullscreenMaterialParams.SetParameter("clearTransition", clearTransition);
		fullscreenMaterialParams.SetParameter("fft", fft);
		fullscreenMaterialParams.SetParameter("tilt", tilt);
		fullscreenMaterialParams.SetParameter("mainTex", side);
		fullscreenMaterialParams.SetParameter("screenCenter", screenCenter);
		fullscreenMaterialParams.SetParameter("timing", timing);

		FullscreenBackground::Render(deltaTime);
	}
};

Background* CreateBackground(class Game* game)
{
	Background* bg = new TestBackground();
	bg->game = game;
	if(!bg->Init())
	{
		delete bg;
		return nullptr;
	}
	return bg;
}