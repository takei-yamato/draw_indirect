// シーンコンスタントバッファ
cbuffer global : register(b0)
{
    // メッシュ用の情報
	matrix viewProj;
};

// 平面情報
struct Plane
{
	float4 normal;
	float4 d;
};
// フラスタムコンスタントバッファ
cbuffer frustum : register(b1)
{
	Plane planes[4];
};

// インスタンス情報
struct InstanceInfo
{
    // メッシュ用の情報
	matrix world;
	float4 color;
};
// インスタンス情報バッファ
StructuredBuffer<InstanceInfo> instanceInfo : register(t0);

// 書き込み先
RWStructuredBuffer<int> dest : register(u0);

[numthreads(8, 8, 1)]
void cs(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	
	// dispatchThreadID を利用して一次元配列の添え字を計算する
	uint id = (dispatchThreadID.z * 4096) + (dispatchThreadID.y * 64) + dispatchThreadID.x;
	float4 pos = instanceInfo[id].world[3];
	
	int visible = 1;
	visible &= ((dot(planes[0].normal.xyz, pos.xyz) + planes[0].d.x) >= 0);
	visible &= ((dot(planes[1].normal.xyz, pos.xyz) + planes[1].d.x) >= 0);
	visible &= ((dot(planes[2].normal.xyz, pos.xyz) + planes[2].d.x) >= 0);
	visible &= ((dot(planes[3].normal.xyz, pos.xyz) + planes[3].d.x) >= 0);

	dest[id] = visible;
}