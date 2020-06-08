#include "MainShader.h"



namespace {
    const ::BRWL::Vec4 quad[] ={
        { 0, 0, 0, 0 },
        { 1, 0, 0, 0 },
        { 1, 1, 0, 0 },
        { 1, 0, 0, 0 },
        { 1, 1, 0, 0 },
        { 1, 0, 0, 0 }
    };
}

BRWL_RENDERER_NS


MainShader::MainShader() :
    rootSignature(nullptr),
    pipelineState(nullptr),
    initialized(false)
{ }

bool MainShader::create(ID3D12Device* device)
{
    return false;
}

void MainShader::render()
{
    if (!initialized) return;
}

void MainShader::destroy()
{
    if (!initialized) return;
    pipelineState = nullptr;
    rootSignature = nullptr;
}

BRWL_RENDERER_NS_END