#include "Blur.hlsli"

struct PixelShaderOutput {
	float4 highLum : SV_TARGET0;//高輝度
};

struct Blur {
	float4 bkWeight[2];
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

ConstantBuffer<Blur> gBlur : register(b0);

PixelShaderOutput main(VertexShaderOutput input) {
	PixelShaderOutput output;
	output.highLum = float32_t4(0, 0, 0, 0);
	float32_t4 color = gTexture.Sample(gSampler, input.texcoord);
	float w;
	float h;
	float level;
	gTexture.GetDimensions(0, w, h, level);
	float dx = 1.0f / w;
	float dy = 1.0f / h;

	output.highLum += gBlur.bkWeight[0] * color;
	output.highLum += gBlur.bkWeight[0][1] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 1));
	output.highLum += gBlur.bkWeight[0][1] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 1));
	output.highLum += gBlur.bkWeight[0][2] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 2));
	output.highLum += gBlur.bkWeight[0][2] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 2));
	output.highLum += gBlur.bkWeight[0][3] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 3));
	output.highLum += gBlur.bkWeight[0][3] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 3));
	output.highLum += gBlur.bkWeight[1][0] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 4));
	output.highLum += gBlur.bkWeight[1][0] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 4));
	output.highLum += gBlur.bkWeight[1][1] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 5));
	output.highLum += gBlur.bkWeight[1][1] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 5));
	output.highLum += gBlur.bkWeight[1][2] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 6));
	output.highLum += gBlur.bkWeight[1][2] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 6));
	output.highLum += gBlur.bkWeight[1][3] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 7));
	output.highLum += gBlur.bkWeight[1][3] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 7));
	output.highLum.a = color.a;
	return output;
}