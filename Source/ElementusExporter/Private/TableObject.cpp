// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEElementusExporter

#include "TableObject.h"
#include "Async/Async.h"
#include "Misc/ScopeLock.h"
#include "Misc/FileHelper.h"

#ifndef ENGINE_MAJOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
#include "DesktopPlatformModule.h"
#else
#include "Kismet/GameplayStatics.h"
#endif

UTableObject::UTableObject(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
}

TMap<FVector2D, FString> UTableObject::GetTableElements() const
{
	return Elements;
}

void UTableObject::InsertElement(const FVector2D& InPosition, const FString& InValue)
{
	Elements.Add(InPosition, InValue);
	NotifyUpdate_Internal();
}

void UTableObject::AppendElements(const TMap<FVector2D, FString> InElements)
{
	Elements.Append(InElements);
	NotifyUpdate_Internal();
}

FString UTableObject::GetElement(const FVector2D& InPosition) const
{
	return Elements.FindRef(InPosition);
}

void UTableObject::RemoveElement(const FVector2D& InPosition)
{
	Elements.Remove(InPosition);
	NotifyUpdate_Internal();
}

void UTableObject::ClearTable()
{
	Elements.Empty();
	NotifyUpdate_Internal();
}

void UTableObject::ExportTable(const bool bClearAtComplete, const float TimeoutSeconds)
{
	if (DestinationFilePath.IsEmpty() && !SetFilePath())
	{
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [bClearAtComplete, TimeoutSeconds, this]
	{
		const TFuture<TArray<FString>>& OutputStr_Future = Async(EAsyncExecution::Thread, [&]
		{
			FScopeLock Lock(&Mutex);
			
			UE_LOG(LogTemp, Display, TEXT("Elementus Exporter - ExportTable: Task Initialized"));			
			UE_LOG(LogTemp, Display, TEXT("Elementus Exporter - ExportTable: Exporting to: %s"), *DestinationFilePath);
			Elements.KeySort([](const FVector2D& InKey1, const FVector2D& InKey2)
			{
				return InKey1 < InKey2;
			});
						
			UpdateMaxValues_Internal();
			
			const int32 MaxValues = MaxColumns * MaxLines;
			const float ProgressStep = 1.f / MaxValues;
			float CurrentProgress = 0.f;

			uint8 PreviousPercentInt = 0;
			
			NotifyProgress_Internal(CurrentProgress);
			
			TArray<FString> LinesArr;
			for (uint32 Line = 0; Line <= MaxLines; ++Line)
			{
				FString ColumnStr;
				for (uint32 Column = 0; Column <= MaxColumns; ++Column)
				{
					if (IsPendingCancel() || IsPendingKill())
					{
						bIsPendingCancel = false;
						return TArray<FString>();
					}
					
					ColumnStr += GetElement(FVector2D(Column, Line)) + ",";

					CurrentProgress += ProgressStep;
					
					// Only notify if the progress change is greater than 1% 
					// to avoid throw a exception due to +1mi iterations if the table is too big
					if (const uint16 CurrentIntegerProgress = static_cast<uint16>(CurrentProgress * 100);
						CurrentIntegerProgress > PreviousPercentInt)
					{
						NotifyProgress_Internal(CurrentProgress);
						PreviousPercentInt = CurrentIntegerProgress;
					}
				}
				LinesArr.Add(ColumnStr);
			}

			return LinesArr;
		});
		
		if (!OutputStr_Future.WaitFor(FTimespan::FromSeconds(TimeoutSeconds)))
		{
			UE_LOG(LogTemp, Warning, 
				TEXT("Elementus Exporter - ExportTable: Result: Fail - Took too long to export the table"));
			
			return;
		}

		if (const TArray<FString> OutputStrArr = OutputStr_Future.Get();
		#if ENGINE_MAJOR_VERSION >= 5
			OutputStrArr.IsEmpty()
		#else
			OutputStrArr.Num() == 0
		#endif
		)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Elementus Exporter - ExportTable: Result: Fail - The process was cancelled."));
			
			NotifyProgress_Internal(-1.f);
		}
		else if (FFileHelper::SaveStringArrayToFile(OutputStr_Future.Get(), *DestinationFilePath))
		{
			UE_LOG(LogTemp, Display, TEXT("Elementus Exporter - ExportTable: Result: Success - Table exported"));
			
			// Ensure that the 100% progress is notified
			NotifyProgress_Internal(1.f);

			if (bClearAtComplete)
			{
				DestinationFilePath.Empty();
				ClearTable();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Elementus Exporter - ExportTable: Result: Fail - Failed to save the file."));
			NotifyProgress_Internal(-1.f);
		}
	});
}

void UTableObject::CancelExport()
{
	bIsPendingCancel = true;
}

bool UTableObject::IsPendingCancel() const
{
	return bIsPendingCancel;
}

bool UTableObject::SetFilePath(const FString InPath)
{
	if (InPath.Equals("None", ESearchCase::IgnoreCase) || InPath.IsEmpty())
	{
		const FString Candidate = OpenSaveCSVDialog();

		if (!Candidate.IsEmpty())
		{
			DestinationFilePath = Candidate;			
			return true;
		}
	}
	else if (FText OutMsg;
		InPath.EndsWith(".csv")
		&& FFileHelper::IsFilenameValidForSaving(InPath, OutMsg))
	{
		DestinationFilePath = InPath;
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, 
			TEXT("ElementusExporter - %s: Result: Fail - Failed to set new file path: %s"), 
			*FString(__func__), *OutMsg.ToString());
	}
	
	return false;
}

FString UTableObject::GetDestinationFilePath() const
{
	return DestinationFilePath;
}

FString UTableObject::OpenSaveCSVDialog()
{
	FString OutputPath;

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	TArray<FString> FileName_Arr;
	if (IDesktopPlatform* Platform = FDesktopPlatformModule::Get();
		Platform->SaveFileDialog(nullptr,
		                         "Save File",
		                         FString(),
		                         "OutputData.csv",
		                         "CSV files (*.csv)|*.csv",
		                         EFileDialogFlags::None,
		                         FileName_Arr))
	{
		UE_LOG(LogTemp, Display, TEXT("Elementus Exporter - %s: Result: Success - File path updated"), *FString(__func__));

		OutputPath = FileName_Arr[0];
	}
	else
	{
		UE_LOG(LogTemp, Error, 
			TEXT("Elementus Exporter - %s: Result: Fail - Failed to open a folder picker or the user cancelled the operation"), 
			*FString(__func__));
	}
#else
	UE_LOG(LogTemp, Error, 
		TEXT("Elementus Exporter - %s: Result: Fail - Platform %s is not supported"), 
		*FString(__func__), *UGameplayStatics::GetPlatformName());
#endif

	return OutputPath;
}

UTableObject* UTableObject::CreateTableObject()
{
	return NewObject<UTableObject>();
}

void UTableObject::Destroy()
{
#if ENGINE_MAJOR_VERSION >= 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UTableObject::InsertionTest(const int32 MaxNum)
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [MaxNum, this]
	{
		FScopeLock Lock(&Mutex);

		for (int32 Line = 0; Line < MaxNum; ++Line)
		{
			for (int32 Column = 0; Column < MaxNum; ++Column)
			{
				Elements.Add(FVector2D(Column, Line), FString::Printf(TEXT("TESTING_L%d_C%d"), Line, Column));
			}
		}

		NotifyUpdate_Internal();
	});
}

void UTableObject::UpdateMaxValues_Internal()
{
	for (const auto& Iterator : Elements)
	{
		if (Iterator.Key.X > MaxColumns)
		{
			MaxColumns = Iterator.Key.X;
		}

		if (Iterator.Key.Y > MaxLines)
		{
			MaxLines = Iterator.Key.Y;
		}
	}
}

void UTableObject::NotifyUpdate_Internal() const
{
	if (OnTableUpdated.IsBound())
	{
		OnTableUpdated.Broadcast();
	}
}

void UTableObject::NotifyProgress_Internal(const float InProgress) const
{
	if (OnTableExportProgressChanged.IsBound())
	{
		OnTableExportProgressChanged.Broadcast(InProgress);
	}
}
