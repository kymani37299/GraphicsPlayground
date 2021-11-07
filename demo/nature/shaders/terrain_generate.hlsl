struct TerrainVert
{
	float3 pos;
	float2 uv;
};

cbuffer TerrainCreateInfo : register(b0)
{
	uint terrainSize;
	uint terrainSideVerts;
	float terrainHeight;
	float2 terrainPosition;
};

RWStructuredBuffer<TerrainVert> TerrainVB : register(u0);

[numthreads(1,1,1)]
void cs_main(uint3 threadID : SV_DispatchThreadID)
{
	const float terrainTileSize = (float)terrainSize / terrainSideVerts;

	uint i = threadID.x;
	uint j = threadID.y;

	float2 modelPos = terrainTileSize * float2(i, j);
	float2 pos2D = terrainPosition - modelPos;

	TerrainVert terrainVert;
	terrainVert.pos = float3(pos2D.x, terrainHeight, pos2D.y);
	terrainVert.uv = modelPos / float2(terrainSize, terrainSize);

	TerrainVB[j + terrainSideVerts * i] = terrainVert;
}