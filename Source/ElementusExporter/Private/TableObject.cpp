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
	return Elements;
}

void UTableObject::InsertElement(const FVector2D& InPosition, const FString& InValue)
{
	Elements.Add(InPosition, InValue);
}

FString UTableObject::GetElement(const FVector2D& InPosition) const
{
	return Elements.FindRef(InPosition);
}

void UTableObject::RemoveElement(const FVector2D& InPosition)
{
	Elements.Remove(InPosition);
}

void UTableObject::ClearTable()
{
	Elements.Empty();
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
		const TFuture<FString>& OutputStr_Future = Async(EAsyncExecution::Thread, [&]
		{
		    FScopeLock Lock(&Mutex);
         	Elements.KeySort([](const FVector2D& InKey1, const FVector2D& InKey2)
         	{
         		return InKey1 < InKey2;
         	});
         	
			UpdateMaxValues_Internal();			

			FString OutputStr;
			for (uint16 Line = 0; Line <= MaxLines; ++Line)
			{
				for (uint16 Column = 0; Column <= MaxColumns; ++Column)
				{
					OutputStr += GetElement(FVector2D(Column, Line)) + FString(Column == MaxColumns ? "\n" : ",");
				}
			}
			
			return OutputStr;
		});
		
		if (!OutputStr_Future.WaitFor(FTimespan::FromSeconds(5)))
		{
			UE_LOG(LogTemp, Display, TEXT("Elementus Exporter - ExportTable: Result: Failed to generate the output string."));
			return;
		}
		
		if (FFileHelper::SaveStringToFile(OutputStr_Future.Get(), *DestinationPath))
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
