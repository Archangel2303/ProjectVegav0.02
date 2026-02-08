#include "ProjectVegaGameMode.h"
#include "ProjectVegaPlayerController.h"
#include "ProjectVegaHUD.h"
#include "ProjectVegaPlayerCharacter.h"

AProjectVegaGameMode::AProjectVegaGameMode()
{
    PlayerControllerClass = AProjectVegaPlayerController::StaticClass();
    HUDClass = AProjectVegaHUD::StaticClass();
    DefaultPawnClass = AProjectVegaPlayerCharacter::StaticClass();
}
