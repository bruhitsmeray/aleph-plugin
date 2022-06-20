// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BaseHUD.generated.h"

/**
 * 
 */
UCLASS()
class ALEPH_API ABaseHUD : public AHUD
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseHUD();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HUD", meta = (GetOptions = "Skins"))
		FString Skin = "Classic";

	UFUNCTION(BlueprintCallable, Category = "HUD")
		TArray<FString> Skins() const
		{
			return {
				"Off", // Disables the HUD completely.
				"Default", // Default is going to be whatever HUD you want to use in your own game.
				"Classic" // This is the built-in HUD of the plugin.
			};
		}

protected:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	void DrawCrosshair(int X, int Y);

	UFUNCTION(BlueprintCallable, Category = "HUD")
		void DrawWidget(TSubclassOf<UUserWidget> Class);
};
