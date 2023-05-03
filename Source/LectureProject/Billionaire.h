// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Billionaire.generated.h"

UCLASS()
class LECTUREPROJECT_API ABillionaire : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABillionaire();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
		class UPaperSprite* TokenSprite;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
		class UPaperSprite* BillionaireSprite;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
		FString Team;
	UPROPERTY(VisibleAnywhere)
		class UPaperSpriteComponent* SpriteComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetupBillionaire(FString team, UPaperSprite* BillionaireSprite, UPaperSprite* TokenSprite);
};
