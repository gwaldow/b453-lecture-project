// Fill out your copyright notice in the Description page of Project Settings.


#include "Billionaire.h"
#include <Components/SphereComponent.h>
#include <PaperSpriteComponent.h>

// Sets default values
ABillionaire::ABillionaire()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Root Sphere"));
    SphereComponent->SetSphereRadius(1.f);
    SphereComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
    RootComponent = SphereComponent;

    // Add a PaperSpriteComponent and set the sprite to 'TokenSprite'
    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("BillionaireSprite"));
    SpriteComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABillionaire::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABillionaire::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABillionaire::SetupBillionaire(FString team, UPaperSprite* bs, UPaperSprite* ts)
{
	Team = team;
    BillionaireSprite = bs;
    TokenSprite = ts;
    SpriteComponent->SetSprite(BillionaireSprite);
}

