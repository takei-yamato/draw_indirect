// 読み取り
StructuredBuffer<int> src : register(t0);
// 書き込み
RWStructuredBuffer<int> dest : register(u0);

[numthreads(4, 4, 1)] 
void CS(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID) {

    // 二次元のスレッドグループとグループ毎のスレッドIDを一次元配列の添え字に変換
    //uint groupId        = (groupID.y * 4) + groupID.x;
    //uint groupThreadId  = (groupThreadID.y * 4) + groupThreadID.x;
    //uint id = (groupId * (4 * 4)) + groupThreadId;
    uint id = (dispatchThreadID.y * (4 * 4)) + dispatchThreadID.x;

    dest[id] = src[id];
}