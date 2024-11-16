// シーンコンスタントバッファ
cbuffer sceneData : register(b0)
{
    // カメラ情報
	matrix viewProj;
};

// 平面情報
struct Plane
{
	float4 normal;
	float4 d;
};

// フラスタムコンスタントバッファ
cbuffer frustumData : register(b1)
{
	Plane planes[4];
};

// インスタンス情報
struct InstanceData
{
    // メッシュ用の情報
	matrix world;
	float4 color;
};
// インスタンス情報バッファ
StructuredBuffer<InstanceData> instanceData : register(t0);

// 描画するインスタンスのインデックス
RWStructuredBuffer<int> drawInstanceIndex : register(u0);

// 描画するインスタンスのカウント
RWByteAddressBuffer drawInstanceCount : register(u1);

[numthreads(8, 8, 1)]
void cs(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	
	// dispatchThreadID を利用して一次元配列の添え字を計算する
	uint id = (dispatchThreadID.z * 4096) + (dispatchThreadID.y * 64) + dispatchThreadID.x;
	float4 pos = instanceData[id].world[3];
	
	// カリングチェック
	int visible = 1;
	visible &= ((dot(planes[0].normal.xyz, pos.xyz) + planes[0].d.x) >= 0);
	visible &= ((dot(planes[1].normal.xyz, pos.xyz) + planes[1].d.x) >= 0);
	visible &= ((dot(planes[2].normal.xyz, pos.xyz) + planes[2].d.x) >= 0);
	visible &= ((dot(planes[3].normal.xyz, pos.xyz) + planes[3].d.x) >= 0);
	
	// カリングチェックを通過したので描画するインスタンスとして登録する
	if (visible) {		
		int index;
		drawInstanceCount.InterlockedAdd(0, 1, index);
		drawInstanceIndex[index] = id;
	}
}