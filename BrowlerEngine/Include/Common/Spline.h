#pragma once // (c) 2020 Lukas Brunner

BRWL_NS

namespace Splines
{

	void CentripetalCatmullRom(const Vec2& first, const Vec2* const samples, const Vec2& last, size_t numSamples, Vec2* const output, size_t numOutput, int numExpectedPoints);

	void CentripetalCatmullRom(const Vec2* samples, unsigned int numSamples, Vec2* const output, unsigned int numOutput, int numExpectedPoints);
}

BRWL_NS_END
