// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelThread.h"
#include "VoxelProceduralMeshComponent.h"
#include "TransitionDirection.h"
#include <atomic>
#include "VoxelChunkComponent.generated.h"

class FVoxelRender;
class FChunkOctree;
class FVoxelPolygonizer;

/**
* Voxel Chunk actor class
*/
UCLASS()
class UVoxelChunkComponent : public UVoxelProceduralMeshComponent
{
	GENERATED_BODY()

public:
	UVoxelChunkComponent();
	~UVoxelChunkComponent();

	UMaterialInstanceDynamic* DynamicMaterial;

	/**
	* Init this
	* @param	NewPosition		Position of this (Minimal corner)
	* @param	NewDepth		Width = 16 * 2^Depth
	* @param	NewWorld		VoxelWorld
	*/
	void Init(FChunkOctree* NewOctree);

	/**
	* Update this for terrain changes
	* @param	bAsync
	*/
	bool Update(bool bAsync);

	bool UpdateFoliage();

	/**
	* Check if an adjacent chunk has changed its resolution, and update async if needed
	*/
	void CheckTransitions();

	/**
	* Schedule destruction of this chunk
	*/
	void Unload();

	void Delete();

	void SetVoxelMaterial(UMaterialInterface* Material);

	bool HasChunkHigherRes(TransitionDirection Direction);

	/**
	* Copy Task section to PrimaryMesh section
	*/
	// Must be thread safe
	void OnMeshComplete(const FVoxelProcMeshSection& InSection, FAsyncPolygonizerTask* InTask);

	void ApplyNewMesh();

	// Must be thread safe
	void OnFoliageComplete();

	void ApplyNewFoliage();

	void ResetRender();

	// Must be thread safe
	FVoxelPolygonizer* CreatePolygonizer(FAsyncPolygonizerTask* Task = nullptr);

protected:
	void Serialize(FArchive& Ar) override;

private:
	TArray<UHierarchicalInstancedStaticMeshComponent*> FoliageComponents;

	FVoxelProcMeshSection Section;

	FTimerHandle DeleteTimer;

	// ChunkHasHigherRes[TransitionDirection] if Depth != 0
	TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes;

	// Async process tasks
	FAsyncPolygonizerTask* MeshBuilder;
	FCriticalSection MeshBuilderLock;
	TArray<FAsyncTask<FAsyncFoliageTask>*> FoliageTasks;
	
	FCriticalSection RenderLock;
	
	FChunkOctree* CurrentOctree;
	FVoxelRender* Render;

	// For when CurrentOctree is deleted
	FIntVector Position;
	int Size;

	bool Flag = true;

	FThreadSafeCounter CompletedFoliageTaskCount;

	void OnAllFoliageComplete();

	void DeleteTasks();
};