struct TerrainVert
{
	float3 pos;
	float2 uv;
};

RWStructuredBuffer<TerrainVert> TerrainVB : register(u0);

[numthreads(1,1,1)]
void cs_main(uint3 threadID : SV_DispatchThreadID)
{
	// TODO: Put in CB
	uint TERRAIN_SIZE = 10000;
	uint TERRAIN_SIDE_VERTS = 200;
	float TILE_SIZE = (float)TERRAIN_SIZE / TERRAIN_SIDE_VERTS;
	float GRASS_TEX_SIZE = 30.0;
	float TERRAIN_HEIGHT = -300.0;
	float2 TERRAIN_POSITION = float2(TERRAIN_SIZE, TERRAIN_SIZE) / 2.0f;

	uint i = threadID.x;
	uint j = threadID.y;

	float2 modelPos = TILE_SIZE * float2(i, j);
	float2 pos2D = TERRAIN_POSITION - modelPos;

	TerrainVert terrainVert;
	terrainVert.pos = float3(pos2D.x, TERRAIN_HEIGHT, pos2D.y);
	terrainVert.uv = modelPos / float2(TERRAIN_SIZE, TERRAIN_SIZE);

	TerrainVB[j + TERRAIN_SIDE_VERTS * i] = terrainVert;
}