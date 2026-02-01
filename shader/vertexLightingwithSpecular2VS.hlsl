#include "common.hlsl"

PS_IN3 main(in VS_IN In)
{
    PS_IN3 Out;

	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);
	
    // 法線変換行列を計算（拡縮成分を取り除く）
    float3x3 normalMatrix = Inverse3x3(float3x3(World._11, World._12, World._13,
                                             World._21, World._22, World._23,
                                             World._31, World._32, World._33));
    // 転置
    normalMatrix = transpose(normalMatrix);

    // 法線ベクトルの方向をワールド座標系に変換
	float3 worldNormal, normal;
	normal = In.Normal.xyz;
    
	worldNormal = mul(normal, normalMatrix);
	worldNormal = normalize(worldNormal);

    
    Out.Position = mul(In.Position, wvp);
    Out.TexCoord = In.TexCoord;

    Out.WPos.xyz = mul(In.Position.xyz, (float3x3) World);
    Out.Normal = worldNormal;

    Out.Diffuse = In.Diffuse;
	
    return Out;
}

