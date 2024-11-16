
// シーンコンスタントバッファ
cbuffer sceneData : register(b0)
{
    // カメラ情報
    matrix viewProj;
};

// インスタンス情報
struct InstanceData
{
    // メッシュ用の情報
	matrix world;
	float4 color;
};

// インスタンス情報
StructuredBuffer<InstanceData> instanceData : register(t0);

// 描画するインスタンスのインデックス
StructuredBuffer<int> drawInstanceIndex : register(t1);

// 頂点シェーダ入力内容
struct VS_INPUT {
    float4 pos : POSITION;
    float4 uv : TEXCOORD;

    uint instanceId : SV_InstanceID;
};

// 頂点シェーダ出力内容
struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float4 uv : TEXCOORD0;
    float4 color : COLOR;
};

//---------------------------------------------------------------------------------
/**
 * @brief
 * 頂点シェーダ
 */
VS_OUTPUT vs(VS_INPUT input) {
    VS_OUTPUT output = (VS_OUTPUT)0;

	// 描画対象のオブジェクトインデックスをインスタンスIDから取得する
	uint objIndex = drawInstanceIndex[input.instanceId];

	// 描画に使う情報を objIndex で取得する
	output.pos = mul(mul(input.pos, instanceData[objIndex].world), viewProj);
	output.color = instanceData[objIndex].color;

    return output;
}

//---------------------------------------------------------------------------------
/**
 * @brief
 * ピクセルシェーダ
 */
float4 ps(VS_OUTPUT input) : SV_Target {
    return input.color;
}