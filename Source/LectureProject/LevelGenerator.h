// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGenerator.generated.h"

UCLASS()
class LECTUREPROJECT_API ALevelGenerator : public AActor
{
	GENERATED_BODY()

public:
	ALevelGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
		class UPaperTileSet* TileSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
		int FloorTileIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
		int WallTileIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
		int TileSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
		int GridWidth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
		int GridHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
		float WallProbability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
		int CellularAutomataIterations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
		TSubclassOf<AActor> billionaire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
		TArray<class UPaperSprite*> BillionaireSprites;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
		TArray<class UPaperSprite*> TokenSprites;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
		TArray<FString> TeamNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
		UStaticMesh* CubeMesh;

	virtual void OnConstruction(const FTransform& Transform) override;

private:

	enum class ETileType { Floor, Wall };

	// Dealing with the Tiles
	class UPaperTileMap* CreateTileMap();
	void SetTileLayers(class UPaperTileMap* TileMap);

	// Make structure to hold all the tile "points"
	struct FRegion
	{
		TArray<FIntPoint> Tiles;
	};

	TArray<TArray<ETileType>> Grid;
	TArray<FRegion> Regions;

	void GenerateGrid();
	void CellularAutomata();
	void IdentifyRegions();
	void CreateCorridors();
	void SpawnBillionaires();
	void CreateBlockingVolumes();

	// Helpers
	int GetWallCount(int X, int Y);
	void FloodFill(FIntPoint Start, TArray<FIntPoint>& Region);
	void ConnectTwoRegions(FRegion& A, FRegion& B);
	bool CheckForFloorSquare(int X, int Y);
};
