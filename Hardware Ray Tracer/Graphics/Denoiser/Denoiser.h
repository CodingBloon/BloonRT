#pragma once

namespace Extensions {

	/*
	 * Denoiser includes
	 * Temporal Accumulation (with reprojection)
	 * History Clamping (to prevent ghosting)
	 * Variance Estimation
	 * Atrous Wavelet Denoiser
	 * Bilateral Pass
	 */

	class Denoiser {
	public:
		Denoiser();
		~Denoiser();

		void denoise();
	};

}