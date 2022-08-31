// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEElementusExporter

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TableObject.generated.h"
/**
 * 
 */
UCLASS(Blueprintable, NotPlaceable, Category = "Elementus Exporter | Classes")
class ELEMENTUSEXPORTER_API UTableObject final : public UObject
{
	GENERATED_BODY()

public:
	explicit UTableObject(const FObjectInitializer& Initializer);

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	TMap<FVector2D, FString> GetTableElements() const;

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void InsertElement(const FVector2D& InPosition, const FString& InValue);

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	FString GetElement(const FVector2D& InPosition) const;

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void RemoveElement(const FVector2D& InPosition);

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void ClearTable();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void ExportTable(const FString& InPath = "None");

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	static FString GetNewFilePath();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	static UTableObject* CreateTableObject();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void Destroy();
	
private:
	FCriticalSection Mutex;
	
	TMap<FVector2D, FString> Elements;
	uint8 MaxLines = 0, MaxColumns = 0;

	void UpdateMaxValues_Internal();
};
