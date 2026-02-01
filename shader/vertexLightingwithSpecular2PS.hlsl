#include "common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

float4 main(in PS_IN3 In)  : SV_Target
{	
    float4 outDiffuse;

    float3 N = In.Normal.xyz;
    N = normalize(N);
	
    float d = -dot(Light.Direction.xyz, N);
    d = saturate(d);

    outDiffuse.xyz = In.Diffuse.xyz * Material.Diffuse.xyz * d * Light.Diffuse.xyz;
    outDiffuse.xyz += In.Diffuse.xyz * Material.Ambient.xyz * Light.Ambient.xyz;
    outDiffuse.xyz += Material.Emission.xyz;
    outDiffuse.a = In.Diffuse.a * Material.Diffuse.a;
		
	if (Material.TextureEnable)
	{
		outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
		outDiffuse *= In.Diffuse;
    }
	else
	{
		outDiffuse.xyz = In.Diffuse.xyz;
		outDiffuse = In.Diffuse;
		outDiffuse.a = In.Diffuse.a;
    }

    // 鏡面反射の計算
    // 視線ベクトルの計算
    float3 V = GetCameraPosition(View) - In.WPos;
    V = normalize(V); //ベクトルの長さを１にする 

	// 光の方向
    float3 L = Light.Direction.xyz;
    L = normalize(L);
    
	// ハーフベクトルを計算
    float3 H;
    H = reflect(L, V);
    H = normalize(H);

	// ブリンフォンモデルで計算（鏡面反射）
    float s = pow(max(0.0f, dot(H, N)), 300);
    float4 Specular = s * Material.Specular;
    Specular.a = 0.0f;
    
    return outDiffuse+Specular;
}