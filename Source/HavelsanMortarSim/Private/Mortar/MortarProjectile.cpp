#include "Mortar/MortarProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

AMortarProjectile::AMortarProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 60.f;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(15.f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComp->SetNotifyRigidBodyCollision(true);
	CollisionComp->OnComponentHit.AddDynamic(this, &AMortarProjectile::OnProjectileHit);
	RootComponent = CollisionComp;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComp);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TrailParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailParticles"));
	TrailParticles->SetupAttachment(MeshComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 1.f;

	InitialLifeSpan = 10.f;
}

void AMortarProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* MyOwner = GetOwner())
	{
		if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetRootComponent()))
		{
			Root->IgnoreActorWhenMoving(MyOwner, true);
		}
	}

	if (!HasAuthority() && ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
	}
}

void AMortarProjectile::SetInitialSpeed(float NewSpeed)
{
	if (!ProjectileMovement) return;

	ProjectileMovement->InitialSpeed = NewSpeed;
	ProjectileMovement->MaxSpeed = NewSpeed;
	ProjectileMovement->Velocity = GetActorForwardVector() * NewSpeed;
}

void AMortarProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	Multicast_PlayExplosion(Hit.ImpactPoint);
	Destroy();
}

void AMortarProjectile::Multicast_PlayExplosion_Implementation(FVector Location)
{
	if (ImpactSounds.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, ImpactSounds.Num() - 1);
		if (USoundBase* Sound = ImpactSounds[Index])
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
		}
	}

	if (HitParticles)
	{
		UNiagaraComponent* BurnFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitParticles, Location, FRotator::ZeroRotator);
		if (BurnFX)
		{
			FTimerHandle Handle;
			FTimerDelegate Delegate;
			Delegate.BindLambda([BurnFX]()
			{
				if (IsValid(BurnFX))
				{
					BurnFX->DestroyComponent();
				}
			});
			GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, HitParticlesLifeTime, false);
		}
	}

	OnExplosion(Location);
}
