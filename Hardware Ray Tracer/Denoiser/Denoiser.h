#pragma once

class Denoiser {

public:
	Denoiser();
	~Denoiser();

	//Calls the Denoiser Pipeline to clean noise from the image produced by the ray tracing rendering pipeline
	void denoiseImage();
};