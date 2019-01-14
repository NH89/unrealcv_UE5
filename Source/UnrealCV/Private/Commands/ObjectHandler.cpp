// Weichao Qiu @ 2017
#include "ObjectHandler.h"

#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Runtime/CoreUObject/Public/UObject/Package.h"

#include "Controller/ActorController.h"
#include "VertexSensor.h"
#include "Utils/StrFormatter.h"
#include "Utils/UObjectUtils.h"
#include "VisionBPLib.h"
#include "CubeActor.h"

void FObjectHandler::RegisterCommands()
{
	CommandDispatcher->BindCommand(
		"vget /objects",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectList),
		"Get the name of all objects"
	);

	CommandDispatcher->BindCommand(
		"vset /objects/spawn_cube",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SpawnBox),
		"Spawn a box in the scene for debugging purpose."
	);

	CommandDispatcher->BindCommand(
		"vset /objects/spawn_cube [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SpawnBox),
		"Spawn a box in the scene for debugging purpose, with optional argument name."
	);

	CommandDispatcher->BindCommand(
		"vset /objects/spawn [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::Spawn),
		"Spawn an object with UClassName as the argument."
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/location",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetLocation),
		"Get object location [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/location [float] [float] [float]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetLocation),
		"Set object location [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/rotation",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetRotation),
		"Get object rotation [pitch, yaw, roll]"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/rotation [float] [float] [float]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetRotation),
		"Set object rotation [pitch, yaw, roll]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/vertex_location",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectVertexLocation),
		"Get vertex location"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/color",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetAnnotationColor),
		"Get the labeling color of an object (used in object instance mask)"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/color [uint] [uint] [uint]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetAnnotationColor),
		"Set the labeling color of an object [r, g, b]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/mobility",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetMobility),
		"Is the object static or movable?"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/show",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetShow),
		"Show object"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/hide",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetHide),
		"Hide object"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/destroy",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::Destroy),
		"Destroy object"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/uclass_name",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetUClassName),
		"Get the UClass name for filtering objects"
	);

#if WITH_EDITOR
	CommandDispatcher->BindCommand(
		"vget /object/[str]/label",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetActorLabel),
		"Get actor label"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/label [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetActorLabel),
		"Set actor label"
	);
#endif
}

AActor* GetActor(const TArray<FString>& Args)
{
	FString ActorId = Args[0];
	AActor* Actor = GetActorById(FUnrealcvServer::Get().GetGameWorld(), ActorId);
	return Actor;
}

FExecStatus FObjectHandler::GetObjectList(const TArray<FString>& Args)
{
	TArray<AActor*> ActorList;
	UVisionBPLib::GetActorList(ActorList);

	FString StrActorList;
	for (AActor* Actor : ActorList)
	{
		StrActorList += FString::Printf(TEXT("%s "), *Actor->GetName());
	}
	return FExecStatus::OK(StrActorList);
}

FExecStatus FObjectHandler::GetLocation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	FVector Location = Controller.GetLocation();

	FStrFormatter Ar;
	Ar << Location;

	return FExecStatus::OK(Ar.ToString());
}

/** There is no guarantee this will always succeed, for example, hitting a wall */
FExecStatus FObjectHandler::SetLocation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
	FActorController Controller(Actor);

	// TODO: Check whether this object is movable
	float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
	FVector NewLocation = FVector(X, Y, Z);
	Controller.SetLocation(NewLocation);

	return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetRotation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	FRotator Rotation = Controller.GetRotation();

	FStrFormatter Ar;
	Ar << Rotation;

	return FExecStatus::OK(Ar.ToString());
}

FExecStatus FObjectHandler::SetRotation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
	FActorController Controller(Actor);

	// TODO: Check whether this object is movable
	float Pitch = FCString::Atof(*Args[1]), Yaw = FCString::Atof(*Args[2]), Roll = FCString::Atof(*Args[3]);
	FRotator Rotator = FRotator(Pitch, Yaw, Roll);
	Controller.SetRotation(Rotator);

	return FExecStatus::OK();
}


FExecStatus FObjectHandler::GetAnnotationColor(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
	FActorController Controller(Actor);

	FColor AnnotationColor;
	Controller.GetAnnotationColor(AnnotationColor);
	return FExecStatus::OK(AnnotationColor.ToString());
}

FExecStatus FObjectHandler::SetAnnotationColor(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
	FActorController Controller(Actor);

	// ObjectName, R, G, B, A = 255
	// The color format is RGBA
	uint32 R = FCString::Atoi(*Args[1]), G = FCString::Atoi(*Args[2]), B = FCString::Atoi(*Args[3]), A = 255; // A = FCString::Atoi(*Args[4]);
	FColor AnnotationColor(R, G, B, A);

	Controller.SetAnnotationColor(AnnotationColor);
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetMobility(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
	FActorController Controller(Actor);

	FString MobilityName = "";
	EComponentMobility::Type Mobility = Controller.GetMobility();
	switch (Mobility)
	{
		case EComponentMobility::Type::Movable: MobilityName = "Movable"; break;
		case EComponentMobility::Type::Static: MobilityName = "Static"; break;
		case EComponentMobility::Type::Stationary: MobilityName = "Stationary"; break;
		default: MobilityName = "Unknown";
	}
	return FExecStatus::OK(MobilityName);
}

FExecStatus FObjectHandler::SetShow(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	Controller.Show();
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::SetHide(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	Controller.Hide();
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetObjectVertexLocation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	FVertexSensor VertexSensor(Actor);
	TArray<FVector> VertexArray = VertexSensor.GetVertexArray();

	// Serialize it to json?
	FString Str = "";
	for (auto Vertex : VertexArray)
	{
		FString VertexLocation = FString::Printf(
			TEXT("%.5f     %.5f     %.5f"),
			Vertex.X, Vertex.Y, Vertex.Z);
		Str += VertexLocation + "\n";
	}

	return FExecStatus::OK(Str);
}

/** A debug utility function to create StaticBox through python API */
FExecStatus FObjectHandler::SpawnBox(const TArray<FString>& Args)
{
	FString ObjectName;
	if (Args.Num() == 1) { ObjectName = Args[0]; }

	AActor* Actor = GWorld->SpawnActor(ACubeActor::StaticClass());

	return FExecStatus::OK();
}

FExecStatus FObjectHandler::Spawn(const TArray<FString>& Args)
{
	FString UClassName;
	if (Args.Num() == 1) { UClassName = Args[0]; }
	// Lookup UClass with a string
	UClass*	Class = FindObject<UClass>(ANY_PACKAGE, *UClassName);

	if (!IsValid(Class))
	{
		return FExecStatus::Error(FString::Printf(TEXT("Can not find a class with name '%s'"), *UClassName));
	}
	AActor* Actor = GWorld->SpawnActor(Class);
	if (IsValid(Actor))
	{
		return FExecStatus::OK(Actor->GetName());
	}
	else
	{
		return FExecStatus::Error("Failed to spawn actor");
	}
}

FExecStatus FObjectHandler::Destroy(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	Actor->Destroy();
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetUClassName(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FString UClassName = Actor->StaticClass()->GetName();
	return FExecStatus::OK(UClassName);
}

/** Get component names of the object */
FExecStatus FObjectHandler::GetComponents(const TArray<FString>& Args)
{
	return FExecStatus::OK();
}

#if WITH_EDITOR
FExecStatus FObjectHandler::SetActorLabel(const TArray<FString>& Args)
{
	FString ActorLabel;
	if (Args.Num() == 2) { ActorLabel = Args[1]; }

	AActor* Actor = GetActor(Args);
	if (!IsValid(Actor)) return FExecStatus::Error("Can not find object");

	Actor->SetActorLabel(ActorLabel);
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetActorLabel(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!IsValid(Actor)) return FExecStatus::Error("Can not find object");

	FString ActorLabel = Actor->GetActorLabel();
	return FExecStatus::OK(ActorLabel);
}
#endif 
