// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEElementusExporter

#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>
#include "TableObject.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTableUpdated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTableExportProgressChanged, const float, Progress);

/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew, NotPlaceable, Category = "Elementus Exporter | Classes")
class ELEMENTUSEXPORTER_API UTableObject final : public UObject
{
	GENERATED_BODY()

public:
	explicit UTableObject(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	TMap<FVector2D, FString> GetTableElements() const;

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void InsertElement(const FVector2D& InPosition, const FString& InValue);

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void AppendElements(const TMap<FVector2D, FString>& InElements);

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	FString GetElement(const FVector2D& InPosition) const;

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void RemoveElement(const FVector2D& InPosition);

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void ClearTable();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void ExportTable(const bool bClearAtComplete = true, const float TimeoutSeconds = 150.f);

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void CancelExport();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	bool SetDestinationFilePath(const FString& InPath = "None");

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	FString GetDestinationFilePath() const;

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions", meta = (DisplayName = "Open Save CSV Dialog"))
	static FString OpenSaveCSVDialog();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	static UTableObject* CreateTableObject();

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void Destroy();

	UPROPERTY(BlueprintAssignable, Category = "Elementus Exporter | Delegates")
	FOnTableUpdated OnTableUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Elementus Exporter | Delegates")
	FOnTableExportProgressChanged OnTableExportProgressChanged;

	UFUNCTION(BlueprintCallable, Category = "Elementus Exporter | Functions")
	void InsertionTest(const int32 MaxNum = 999);

protected:
	bool IsPendingCancel() const;

	virtual void BeginDestroy() override;

	void UpdateMaxValues_Internal();

	void NotifyUpdate_Internal() const;
	void NotifyProgress_Internal(const float InProgress) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Elementus Exporter | Properties", meta = (AllowPrivateAccess = "true"))
	TMap<FVector2D, FString> Elements;

	UPROPERTY(VisibleAnywhere, Category = "Elementus Exporter | Properties", meta = (AllowPrivateAccess = "true", Getter = "GetDestinationFilePath", Setter = "SetDestinationFilePath"))
	FString DestinationFilePath;

	uint32 MaxLines = 0, MaxColumns = 0;

	mutable bool bIsPendingCancel = false;

	mutable FCriticalSection Mutex;
};
