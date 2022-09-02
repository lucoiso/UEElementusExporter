// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEElementusExporter

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TableObject.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTableUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTableExportProgressChanged, const float, Progress);

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
	void AppendElements(const TMap<FVector2D, FString> InElements);

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	FString GetElement(const FVector2D& InPosition) const;

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void RemoveElement(const FVector2D& InPosition);

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void ClearTable();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void ExportTable(const bool bClearAtComplete = true);

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void CancelExport();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	bool SetFilePath(const FString InPath = "None");

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	FString GetDestinationFilePath() const;
	
	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions", meta = (DisplayName = "Save New CSV"))
	static FString SaveNewCSV();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	static UTableObject* CreateTableObject();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void Destroy();

	UPROPERTY(BlueprintAssignable, Category = "Elementus Exporter | Delegates")
	FOnTableUpdated OnTableUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Elementus Exporter | Delegates")
	FOnTableExportProgressChanged OnTableExportProgressChanged;

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void InsertionTest(const int32 MaxNum = 100);

protected:
	bool IsPendingCancel() const;

	void UpdateMaxValues_Internal();

	void NotifyUpdate_Internal() const;
	void NotifyProgress_Internal(const float InProgress) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Elementus Exporter | Properties", meta = (AllowPrivateAccess = "true"))
	TMap<FVector2D, FString> Elements;

	mutable FCriticalSection Mutex;
	uint32 MaxLines = 0, MaxColumns = 0;
	FString DestinationFilePath;

	bool bIsPendingCancel = false;
};
