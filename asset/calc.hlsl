// 読み取り
StructuredBuffer<int> src : register(t0);
// 書き込み
RWStructuredBuffer<int> dest : register(u0);

[numthreads(4, 4, 1)] 
void CS(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID) {

    // 二次元のスレッドグループIDを一次元の連番に変換
    uint groupId        = (groupID.y * 4) + groupID.x;
    // グループ単位の二次元のスレッドIDを一次元の連番に変換
    uint groupThreadId  = (groupThreadID.y * 4) + groupThreadID.x;
	// 二つの連番から一次元配列の添え字を計算する
    uint id = (groupId * (4 * 4)) + groupThreadId;
    
	// dispatchThreadID を利用した下記の計算でも一次元配列の添え字を計算できる
	//uint id = (dispatchThreadID.y * (4 * 4)) + dispatchThreadID.x;

	// 配列の内容を複製する
	dest[id] = src[id];
}