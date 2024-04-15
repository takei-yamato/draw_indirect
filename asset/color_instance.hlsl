
// インスタンス数（同一の物を一度に描画する数）
#define instanceNum (4)

// シーンコンスタントバッファ
cbuffer global : register(b0) {
	// ビュープロジェクション
    matrix viewProj;

    // オブジェクト用の情報
    matrix world[instanceNum];
    float4 color[instanceNum];

    // 描画インスタンスのインデックス
    // ※「int 配列」にしてしまうとパッキング規則からパディングが入る
    uint4 index;
};

// 頂点シェーダ入力内容
struct VS_INPUT {
    float4 pos : POSITION;
    float4 uv : TEXCOORD;

    uint instanceId : SV_InstanceID;	// Draw Call 関数で指定した個数に合わせて、インスタンスの ID (インデックス)が入る
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

	// 描画するインスタンスをインスタンスIDから取得する
    uint instanceIndex = index[input.instanceId];

	// 描画に使う情報を instanceIndex で取得する
    output.pos   = mul(mul(input.pos, world[instanceIndex]), viewProj);
    output.color = color[instanceIndex];

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