// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelGenerator.h"
#include "PaperTileMap.h"
#include "PaperTileMapComponent.h"
#include "PaperTileSet.h"
#include "PaperSprite.h"
#include "Billionaire.h"
#include "Engine/BlockingVolume.h"

ALevelGenerator::ALevelGenerator()
{
	GridWidth = 36;
	GridHeight = 20;
	WallProbability = 0.4f;
	CellularAutomataIterations = 5;
	TileSize = 128;
}

// Called at construction of object
void ALevelGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GenerateGrid();
	CellularAutomata();
	IdentifyRegions();
	CreateCorridors();
	SpawnBillionaires();
	CreateBlockingVolumes();
	// Spawn the tiles.
	UPaperTileMap* TileMap = CreateTileMap();

	UPaperTileMapComponent* TileMapComponent = NewObject<UPaperTileMapComponent>(this);
	TileMapComponent->RegisterComponent();
	TileMapComponent->SetTileMap(TileMap);
	TileMapComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);


	// Test prihnt out the grid
	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		FString GridRow;

		for (int32 X = 0; X < GridWidth; ++X)
		{
			GridRow += (Grid[Y][X] == ETileType::Floor) ? "F " : "W ";
		}

		UE_LOG(LogTemp, Log, TEXT("%s"), *GridRow);
	}
}

// Generates initial random grid of tiles
void ALevelGenerator::GenerateGrid()
{
	Grid.SetNum(GridHeight);
	for (int i = 0; i < GridHeight; ++i)
	{
		Grid[i].SetNum(GridWidth);
		for (int j = 0; j < GridWidth; ++j)
		{
			Grid[i][j] = FMath::RandRange(0.0f, 1.0f) < WallProbability ? ETileType::Wall : ETileType::Floor;
		}
	}
}

// Use cellular automata rules to regularize the random generation.
// >= 4 Wall Neighbors => become a wall ; otherwise become a floor
void ALevelGenerator::CellularAutomata()
{
	TArray<TArray<ETileType>> NewGrid = Grid;

	for (int Iteration = 0; Iteration < CellularAutomataIterations; ++Iteration)
	{
		for (int i = 0; i < GridHeight; ++i)
		{
			for (int X = 0; X < GridWidth; ++X)
			{
				int WallCount = GetWallCount(X, i);

				if (WallCount >= 4)
					NewGrid[i][X] = ETileType::Wall;
				else if (WallCount <= 3)
					NewGrid[i][X] = ETileType::Floor;
			}
		}

		Grid = NewGrid;
	}
}

// Count surrounding walls for a given tile i,j
int ALevelGenerator::GetWallCount(int X, int Y)
{

	int WallCount = 0;

	for (int NeighborY = Y - 1; NeighborY <= Y + 1; ++NeighborY)
	{
		for (int NeighborX = X - 1; NeighborX <= X + 1; ++NeighborX)
		{
			// Check if in-bounds grid neighbor is wall
			if (NeighborX >= 0 && NeighborX < GridWidth && NeighborY >= 0 && NeighborY < GridHeight)
			{
				// Not Self
				if ((NeighborX != X || NeighborY != Y) && Grid[NeighborY][NeighborX] == ETileType::Wall)
				{
					++WallCount;
				}
			}
			else
			{
				// On border
				++WallCount;
			}
		}
	}

	return WallCount;
}


UPaperTileMap* ALevelGenerator::CreateTileMap()
{
	UPaperTileMap* TileMap = NewObject<UPaperTileMap>();

	TileMap->TileWidth = TileSize;
	TileMap->TileHeight = TileSize;

	TileMap->MapWidth = GridWidth;
	TileMap->MapHeight = GridHeight;
	TileMap->SeparationPerLayer = 0.0f;
	TileMap->SeparationPerTileX = 0.0f;
	TileMap->SeparationPerTileY = 0.0f;

	SetTileLayers(TileMap);

	return TileMap;
}

void ALevelGenerator::SetTileLayers(UPaperTileMap* TileMap)
{
	UPaperTileLayer* Layer = NewObject<UPaperTileLayer>();

	// Initialize the layer with the desired width and height
	Layer->DestructiveAllocateMap(GridWidth, GridHeight);

	for (int Y = 0; Y < GridHeight; ++Y)
	{
		for (int X = 0; X < GridWidth; ++X)
		{
			FPaperTileInfo TileInfo;
			TileInfo.TileSet = TileSet;

			if (Grid[Y][X] == ETileType::Floor)
			{
				TileInfo.PackedTileIndex = FloorTileIndex;
			}
			else if (Grid[Y][X] == ETileType::Wall)
			{
				TileInfo.PackedTileIndex = WallTileIndex;
			}

			Layer->SetCell(X, Y, TileInfo);
		}
	}

	TileMap->TileLayers.Add(Layer);
}

// Use flood fill algorithm to get distinct regions in the grid.
void ALevelGenerator::IdentifyRegions()
{
	TArray<TArray<bool>> Visited;
	Visited.SetNum(GridHeight);
	for (int Y = 0; Y < GridHeight; ++Y)
	{
		Visited[Y].SetNum(GridWidth);
		FMemory::Memset(Visited[Y].GetData(), 0, GridWidth * sizeof(bool));
	}

	for (int Y = 0; Y < GridHeight; ++Y)
	{
		for (int X = 0; X < GridWidth; ++X)
		{
			if (!Visited[Y][X] && Grid[Y][X] == ETileType::Floor)
			{
				FRegion NewRegion;
				FloodFill(FIntPoint(X, Y), NewRegion.Tiles);
				for (const FIntPoint& Tile : NewRegion.Tiles)
				{
					Visited[Tile.Y][Tile.X] = true;
				}
				Regions.Add(NewRegion);
			}
		}
	}
}

// Flood fill implementation helper
void ALevelGenerator::FloodFill(FIntPoint Start, TArray<FIntPoint>& Region)
{
	TArray<FIntPoint> OpenSet;
	OpenSet.Push(Start);

	while (OpenSet.Num() > 0)
	{
		FIntPoint Current = OpenSet.Pop();

		for (int NeighborY = Current.Y - 1; NeighborY <= Current.Y + 1; ++NeighborY)
		{
			for (int NeighborX = Current.X - 1; NeighborX <= Current.X + 1; ++NeighborX)
			{
				if (NeighborX >= 0 && NeighborX < GridWidth && NeighborY >= 0 && NeighborY < GridHeight)
				{
					if (Grid[NeighborY][NeighborX] == ETileType::Floor && !Region.Contains(FIntPoint(NeighborX, NeighborY)))
					{
						Region.Add(FIntPoint(NeighborX, NeighborY));
						OpenSet.Push(FIntPoint(NeighborX, NeighborY));
					}
				}
			}
		}
	}
}

// Encasing function for ConnectTwoRegions
void ALevelGenerator::CreateCorridors()
{
	for (int i = 1; i < Regions.Num(); ++i)
	{
		ConnectTwoRegions(Regions[i - 1], Regions[i]);
	}
}

// Generates a floor path between two regions
void ALevelGenerator::ConnectTwoRegions(FRegion& A, FRegion& B)
{
	FIntPoint BestTileA, BestTileB;
	float BestDistance = FLT_MAX;

	// Exaustively check for minimum distance tiles between teh two regions
	for (const FIntPoint& TileA : A.Tiles)
	{
		for (const FIntPoint& TileB : B.Tiles)
		{
			float Distance = FVector::DistSquared(FVector(TileA, 0), FVector(TileB, 0));
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestTileA = TileA;
				BestTileB = TileB;
			}
		}
	}

	// Create a corridor between the two closest tiles
	FIntPoint Current = BestTileA;

	while (Current != BestTileB)
	{
		if (FMath::RandBool())
		{
			Current.X += FMath::Sign(BestTileB.X - Current.X);
		}
		else
		{
			Current.Y += FMath::Sign(BestTileB.Y - Current.Y);
		}

		// Make sure the current position is within grid bounds
		Current.X = FMath::Clamp(Current.X, 0, GridWidth - 1);
		Current.Y = FMath::Clamp(Current.Y, 0, GridHeight - 1);

		Grid[Current.Y][Current.X] = ETileType::Floor;

		// widen path by using a "brush"
		Grid[FMath::Clamp(Current.Y + 1, 0, GridHeight - 1)][Current.X] = ETileType::Floor; // Above
		Grid[FMath::Clamp(Current.Y - 1, 0, GridHeight - 1)][Current.X] = ETileType::Floor; // Below
		Grid[Current.Y][FMath::Clamp(Current.X + 1, 0, GridWidth - 1)] = ETileType::Floor; // Right
		Grid[Current.Y][FMath::Clamp(Current.X - 1, 0, GridWidth - 1)] = ETileType::Floor; // Left

		Grid[FMath::Clamp(Current.Y + 1, 0, GridHeight - 1)][FMath::Clamp(Current.X + 1, 0, GridWidth - 1)] = ETileType::Floor; // top right
		Grid[FMath::Clamp(Current.Y - 1, 0, GridHeight - 1)][FMath::Clamp(Current.X - 1, 0, GridWidth - 1)] = ETileType::Floor; // bottom left
		Grid[FMath::Clamp(Current.Y + 1, 0, GridHeight - 1)][FMath::Clamp(Current.X - 1, 0, GridWidth - 1)] = ETileType::Floor; // top left
		Grid[FMath::Clamp(Current.Y - 1, 0, GridHeight - 1)][FMath::Clamp(Current.X + 1, 0, GridWidth - 1)] = ETileType::Floor; // bottom right
	}
}


void ALevelGenerator::SpawnBillionaires()
{
	int TotalFloorTiles = 0;
	for (int Y = 0; Y < GridHeight; ++Y)
	{
		for (int X = 0; X < GridWidth; ++X)
		{
			if (Grid[Y][X] == ETileType::Floor)
			{
				TotalFloorTiles++;
			}
		}
	}

	int BillionaireCount = 0;
	TArray<FVector> BillionaireLocations;
	while (BillionaireCount < 4)
	{
		int X = FMath::RandRange(0, GridWidth - 1);
		int Y = FMath::RandRange(0, GridHeight - 1);

		// Check if random point is at least 5 tiles away
		bool DistCheck = true;
		for (const FVector& Point : BillionaireLocations)
		{
			float Distance = FVector::Distance(Point, FVector(X * TileSize + (TileSize / 2),230, X * TileSize + (TileSize / 2)));

			// Check if they are too close
			if (Distance < 300)
			{
				DistCheck =  false;
			}
		}

		if (DistCheck && CheckForFloorSquare(X,Y))
		{
			// Spawn the billionaire at the center of the tile
			FVector Location = FVector(X * TileSize + (TileSize/2), 230, -Y * TileSize - (TileSize / 2));
			FRotator Rotation = FRotator::ZeroRotator;
			BillionaireLocations.Add(FVector(X * TileSize + (TileSize / 2), 230, -Y * TileSize - (TileSize / 2)));
			ABillionaire* NewBillionaire = GetWorld()->SpawnActor<ABillionaire>(billionaire, Location, Rotation);
			NewBillionaire->SetupBillionaire(TeamNames[BillionaireCount], BillionaireSprites[BillionaireCount], TokenSprites[BillionaireCount]);
			UE_LOG(LogTemp, Warning, TEXT("Spawning Billion"));
			BillionaireCount++;
		}
	}
}

bool ALevelGenerator::CheckForFloorSquare(int X, int Y) 
{
	return Grid[Y][X] == ETileType::Floor && Grid[Y+1][X+1] == ETileType::Floor && Grid[Y - 1][X + 1] == ETileType::Floor && Grid[Y + 1][X - 1] == ETileType::Floor && Grid[Y - 1][X - 1] == ETileType::Floor && Grid[Y][X + 1] == ETileType::Floor && Grid[Y][X - 1] == ETileType::Floor && Grid[Y + 1][X] == ETileType::Floor && Grid[Y - 1][X] == ETileType::Floor;
}

void ALevelGenerator::CreateBlockingVolumes()
{
	// Iterate through the grid and create a cube on top of each wall tile
	int cubeCount = 0;
	for (int Y = 0; Y < GridHeight; ++Y)
	{
		for (int X = 0; X < GridWidth; ++X)
		{
			if (Grid[Y][X] == ETileType::Wall)
			{
				// tile location
				FVector Location = FVector(X * TileSize + TileSize + 20, 100 / 2, -Y * TileSize - TileSize + 40);
				FRotator Rotation = FRotator::ZeroRotator;
				FVector Scale = FVector(0.5, 2, 0.5);

				AActor* NewCube = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation);

				// Make the blocking cube
				UStaticMeshComponent* CubeMeshComponent = NewObject<UStaticMeshComponent>(NewCube);
				CubeMeshComponent->SetRelativeScale3D(Scale);
				CubeMeshComponent->SetupAttachment(NewCube->GetRootComponent());
				CubeMeshComponent->SetStaticMesh(this->CubeMesh);
				CubeMeshComponent->SetHiddenInGame(true);
				CubeMeshComponent->SetWorldLocation(Location);
				//NewCube->SetActorScale3D(Scale);
				CubeMeshComponent->RegisterComponent();
			}
		}
	}
}