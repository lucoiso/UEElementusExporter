// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEElementusExporter

#include "TableObject.h"
#include "Async/Async.h"
#include "Misc/ScopeLock.h"

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
	FScopeLock Lock(&Mutex);

	return Elements;
}

void UTableObject::InsertElement(const FVector2D& InPosition, const FString& InValue)
{
	FScopeLock Lock(&Mutex);

	Elements.Add(InPosition, InValue);
	NotifyUpdate_Internal();
}

void UTableObject::AppendElements(const TMap<FVector2D, FString> InElements)
{
	FScopeLock Lock(&Mutex);

	Elements.Append(InElements);
	NotifyUpdate_Internal();
}

FString UTableObject::GetElement(const FVector2D& InPosition) const
{
	FScopeLock Lock(&Mutex);

	return Elements.FindRef(InPosition);
}

void UTableObject::RemoveElement(const FVector2D& InPosition)
{
	FScopeLock Lock(&Mutex);

	Elements.Remove(InPosition);
	NotifyUpdate_Internal();
}

void UTableObject::ClearTable()
{
	FScopeLock Lock(&Mutex);

	Elements.Empty();
	NotifyUpdate_Internal();
}

void UTableObject::ExportTable(const FString& InPath)
{
	FString DestinationPath = InPath;
	if (DestinationPath.Equals("None", ESearchCase::IgnoreCase) || DestinationPath.IsEmpty())
	{
		DestinationPath = GetNewFilePath();

		if (DestinationPath.IsEmpty())
		{
			return;
		}
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [DestinationPath, this]
	{
		const TFuture<TArray<FString>>& OutputStr_Future = Async(EAsyncExecution::Thread, [&]
		{
			FScopeLock Lock(&Mutex);

			UE_LOG(LogTemp, Warning, TEXT("Exporting table to %s"), *DestinationPath);
			Elements.KeySort([](const FVector2D& InKey1, const FVector2D& InKey2)
			{
				return InKey1 < InKey2;
			});

			UpdateMaxValues_Internal();

			TArray<FString> LinesArr;
			for (uint32 Line = 0; Line <= MaxLines; ++Line)
			{
				FString ColumnStr;
				for (uint32 Column = 0; Column <= MaxColumns; ++Column)
				{
					ColumnStr += GetElement(FVector2D(Column, Line));
				}
				LinesArr.Add(ColumnStr);
			}

			return LinesArr;
		});

		if (!OutputStr_Future.WaitFor(FTimespan::FromSeconds(5)))
		{
			UE_LOG(LogTemp, Display, TEXT("Elementus Exporter - ExportTable: Result: Failed to generate the output string."));
			return;
		}
		if (FFileHelper::SaveStringArrayToFile(OutputStr_Future.Get(), *DestinationPath))
		{
			UE_LOG(LogTemp, Display, TEXT("Elementus Exporter - ExportTable: Result: Success"));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Elementus Exporter - ExportTable: Result: Failed to save the file."));
		}
	});
}

FString UTableObject::GetNewFilePath()
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
		UE_LOG(LogTemp, Display, TEXT("Elementus Exporter - %s: Result: Success"), *FString(__func__));

		OutputPath = FileName_Arr[0];
	}
	else
	{
		UE_LOG(LogTemp, Error,
		       TEXT("Elementus Exporter - %s: Result: Failed to open a folder picker or the user cancelled the operation"),
		       *FString(__func__));
	}
#else
	UE_LOG(LogTemp, Error,
		   TEXT("Elementus Exporter - %s: Platform %s is not supported"),
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
	MarkAsGarbage();
}

void UTableObject::InsertionTest(const int32 MaxNum)
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [MaxNum, this]
	{
		FScopeLock Lock(&Mutex);

		for (int32 i = 0; i < MaxNum; ++i)
		{
			InsertElement(FVector2D(0, i), "test");
		}
	});
}

void UTableObject::UpdateMaxValues_Internal()
{
	FScopeLock Lock(&Mutex);

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
	OnTableUpdated.Broadcast();
}
