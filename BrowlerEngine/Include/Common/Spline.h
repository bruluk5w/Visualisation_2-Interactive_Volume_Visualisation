#pragma once // (c) 2020 Lukas Brunner

BRWL_NS

namespace Splines
{

	void CentripetalCatmullRom(const Vec2& first, const Vec2* const samples, const Vec2& last, size_t numSamples, Vec2* const output, size_t numOutput)
	{
		if (numOutput == 0) return;

		const Vec2* a = &first;
		const Vec2* b = samples;
		const Vec2* c = numSamples == 1 ? samples : samples + 1;
		const Vec2* d = samples + 1;
		Vec2 delta1 = *b - *a;
		Vec2 delta2 = *c - *b;
		Vec2 delta3 = *d - *c;
		float t0 = 0;
		float t1 = std::sqrtf(std::sqrtf(delta1.x * delta1.x + delta1.y * delta1.y));
		float preT2 = std::sqrtf(std::sqrtf(delta2.x * delta1.x + delta2.y * delta2.y));
		float preT3 = std::sqrtf(std::sqrtf(delta3.x * delta3.x + delta3.y * delta3.y));
		double dt = ((double)numSamples - 1.0) / ((double)numOutput - 1.0);
		

		float t2 = preT2 + t1;
		float t3 = preT3 + t2;
		double t = 0;
		for (unsigned int i = 0; i < numSamples - 1; ++i, t+=dt)
		{
			if (dt >= 1)
			{
				dt -= 1;
				a = b;
				b = c;
				c = d;
				if (i == numSamples - 2) d = &last;
				else d = ++d;
					

				delta1 = delta2;
				delta2 = delta3;
				delta3 = *d - *c;

				t1 = preT2;
				preT2 = preT3;
				preT3 = std::sqrtf(std::sqrtf(delta3.x * delta3.x + delta3.y * delta3.y));
				float t2 = preT2 + t1;
				float t3 = preT3 + t2;
			}

			const double t1MinusT0 = t1 - t0;
			const double t2MinusT1 = t2 - t1;
			const double t3MinusT1 = t3 - t1;
			const double t2MinusT0 = t2 - t0;
			const double t3MinusT2 = t3 - t2;
			const double t2MinusT = t2 - t;
			const double t3MinusT = t3 - t;
			const double tMinusT0 = t - t0;
			const double tMinusT1 = t - t1;
			const Vec2 A1 = (float)((t1 - t) / t1MinusT0) * *a + (float)(tMinusT0 / t1MinusT0) * *b;
			const Vec2 A2 = (float)(t2MinusT / t2MinusT1) * *b + (float)(tMinusT1 / t2MinusT1) * *c;
			const Vec2 A3 = (float)(t3MinusT / t3MinusT2) * *c + (float)((t - t2) / t3MinusT2) * *c;

			Vec2 B1 = (float)(t2MinusT / t2MinusT0) * A1 + (float)(tMinusT0 / t2MinusT0) * A2;
			Vec2 B2 = (float)(t3MinusT / t3MinusT1) * A2 + (float)(tMinusT1 / t3MinusT1) * A3;
			output[i] = (float)(t2MinusT / t2MinusT1) * B1 + (float)(tMinusT1 / t2MinusT1) * B2;
		}

	}

	void CentripetalCatmullRom(const Vec2* samples, unsigned int numSamples, Vec2* const output, unsigned int numOutput)
	{
		if (numSamples < 2) return;
		//if (numSamples == 3) return CentripetalCatmullRom(samples[0], &samples[1], samples[2], 1, output, numOutput);
		return CentripetalCatmullRom(samples[0], &samples[1], samples[numSamples - 1], numSamples - 2, output, numOutput);
	}
}

BRWL_NS_END