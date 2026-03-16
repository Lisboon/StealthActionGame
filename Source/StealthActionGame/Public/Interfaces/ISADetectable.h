// Copyright 2026, Lisboon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISADetectable.generated.h"

UENUM(BlueprintType)
enum class ESADetectionProfile : uint8
{
    /** Silhouette completa, caminhada normal */
    Standing     UMETA(DisplayName = "Standing"),
    /** Crouch: velocidade de detecção reduzida em 40% */
    Crouching    UMETA(DisplayName = "Crouching"),
    /** Em cobertura atrás de obstáculo: apenas sliver visível */
    InCover      UMETA(DisplayName = "In Cover"),
    /** Distração ativa (roleando objeto, etc.) */
    Distracted   UMETA(DisplayName = "Distracted"),
};

// UInterface boilerplate — NUNCA adicionar lógica aqui
UINTERFACE(MinimalAPI, Blueprintable)
class USADetectable : public UInterface
{
    GENERATED_BODY()
};

class STEALTHACTIONGAME_API ISADetectable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Detection")
    bool CanBeDetected() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Detection")
    ESADetectionProfile GetDetectionProfile() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Detection")
    void OnDetectedByEnemy(AActor* DetectingEnemy, ESADetectionProfile ProfileAtDetection);
};
