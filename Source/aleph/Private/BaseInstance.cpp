#include "BaseInstance.h"
#include "CSL_Window.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetSystemLibrary.h"

IOnlineSubsystem* OnlineSubsystem;
FString StaticOnlineIgn;
FString StaticGameType;
FString Self;

int SesionNumber = FMath::RandRange(1, 9999);

const FString SessionMap_const = "default";
const FName SessionName_const = FName(FString::Printf(TEXT("Session %i"), FMath::RandRange(1, SesionNumber)));
const FString LobbyName_const = FString::Printf(TEXT("%s's Lobby"), *StaticOnlineIgn);

UBaseInstance::UBaseInstance()
{
	bIsLoggedIn = false;
	bIsSessionLive = false;
	Self = this->GetName();
}

void UBaseInstance::Init()
{
	Super::Init();
	StaticGameType = GameType;
	OnlineSubsystem = IOnlineSubsystem::Get();
	if(StaticGameType == "Singleplayer with Online features" || StaticGameType == "Multiplayer"
			|| StaticGameType == "Both")
	{
		Login();
	}
}

void UBaseInstance::Shutdown()
{
	Super::Shutdown();
}

FString UBaseInstance::GetPlayerName()
{
	if(OnlineSubsystem)
	{
		if(StaticGameType == "Singleplayer with Online features" || StaticGameType == "Multiplayer"
			|| StaticGameType == "Both")
		{
			return StaticOnlineIgn;
		}
	}
	
	if(!OnlineSubsystem || StaticOnlineIgn.IsEmpty() || StaticOnlineIgn == "DummyDisplayName")
	{
		return UKismetSystemLibrary::GetPlatformUserName();
	}

	return FString("null");
}

void UBaseInstance::Login()
{
	if (StaticGameType == "Singleplayer with Online features" || StaticGameType == "Multiplayer"
		|| StaticGameType == "Both")
	{
		if(OnlineSubsystem)
		{
			if (IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
			{
				FOnlineAccountCredentials Credentials;
				Credentials.Id = FString("localhost:6300");
				Credentials.Token = FString("Context_1");
				Credentials.Type = FString("developer");

				Identity->OnLoginCompleteDelegates->AddUObject(this, &UBaseInstance::OnLoginComplete);
				Identity->Login(0, Credentials);
			}
		}
	} else {
		UCSL_Window::PrintToConsole(Self, "Warning",
			"You can't login while the GameType is set to: Singleplayer.");
	}
}

void UBaseInstance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
	const FString& Error)
{
	bIsLoggedIn = bWasSuccessful;
	if(OnlineSubsystem)
	{
		UCSL_Window::PrintToConsole(Self, "Warning",
			FString::Printf(TEXT("Client successfully logged in. Code: %d"), bWasSuccessful));
		if(IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
		{
			StaticOnlineIgn = Identity->GetUserAccount(UserId)->GetDisplayName();
			Identity->ClearOnLoginCompleteDelegates(0, this);
		}
	} else {
		UCSL_Window::PrintToConsole(Self, "Warning",
			FString::Printf(TEXT("Client failed to log in. Code: %d"), bWasSuccessful));
	}
}

void UBaseInstance::CreateSession()
{
	if(StaticGameType == "Multiplayer" || StaticGameType == "Both") {
		if(bIsLoggedIn && OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				FOnlineSessionSettings SessionSettings;
				SessionSettings.bIsDedicated = false;
				SessionSettings.bShouldAdvertise = true;
				SessionSettings.bIsLANMatch = false;
				SessionSettings.NumPublicConnections = 5;
				SessionSettings.bAllowJoinInProgress = true;
				SessionSettings.bAllowJoinViaPresence = true;
				SessionSettings.bUsesPresence = true;
				SessionSettings.bUseLobbiesIfAvailable = true;
				SessionSettings.bUseLobbiesVoiceChatIfAvailable = true;
				SessionSettings.Set(SEARCH_KEYWORDS, LobbyName_const, EOnlineDataAdvertisementType::ViaOnlineService);

				SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UBaseInstance::OnCreateSessionComplete);
				SessionPtr->CreateSession(0, SessionName_const, SessionSettings);
			}
		}
	} else {
		UCSL_Window::PrintToConsole(Self, "Warning",
			"You can't create a session while the GameType is set to: Singleplayer.");
	}
}

void UBaseInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	bIsSessionLive = bWasSuccessful;
	FName SessionNameOutput = SessionName_const;
	FString LobbyNameOutput = LobbyName_const;
	UCSL_Window::PrintToConsole(Self, "Warning",
		FString::Printf(TEXT("Session created with success. Client connected with success. Code: %d. SessionName: %s. LobbyName: %s"), bIsSessionLive, *SessionNameOutput.ToString(), *LobbyNameOutput));
	if(OnlineSubsystem)
	{
		if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionPtr->ClearOnCreateSessionCompleteDelegates(this);
			GetWorld()->ServerTravel(SessionMap_const, false);
		}
	}
}

void UBaseInstance::DestroySession()
{
	if(StaticGameType == "Multiplayer" || StaticGameType == "Both") {
		if(bIsLoggedIn && OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SessionPtr->OnDestroySessionCompleteDelegates.AddUObject(this, &UBaseInstance::OnDestroySessionComplete);
				SessionPtr->DestroySession(SessionName_const);
			}
		}
	} else {
		UCSL_Window::PrintToConsole(Self, "Warning",
			"You can't destroy a session while the GameType is set to: Singleplayer.");
	}
}

void UBaseInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	bIsSessionLive = bWasSuccessful;
	UCSL_Window::PrintToConsole(Self, "Warning",
		FString::Printf(TEXT("Session destroyed with success. Client disconnected with success. Code: %d."), bIsSessionLive));
	if(OnlineSubsystem)
	{
		if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionPtr->ClearOnDestroySessionCompleteDelegates(this);
		}
	}
}

void UBaseInstance::FindSession()
{
	if(StaticGameType == "Multiplayer" || StaticGameType == "Both") {
		if(bIsLoggedIn && OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SearchSettings = MakeShareable(new FOnlineSessionSearch());
				SearchSettings->MaxSearchResults = 5000;
				SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, LobbyName_const, EOnlineComparisonOp::Equals);
				SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

				SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UBaseInstance::OnFindSessionComplete);
				SessionPtr->FindSessions(0, SearchSettings.ToSharedRef());
			}
		}
	} else {
		UCSL_Window::PrintToConsole(Self, "Warning",
			"You can't search for a session while the GameType is set to: Singleplayer.");
	}
}

void UBaseInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	UCSL_Window::PrintToConsole(Self, "Warning", FString::Printf(TEXT("Session(s) found with success. Code: %d"), bWasSuccessful));
	if(bWasSuccessful)
	{
		UCSL_Window::PrintToConsole(Self, "Warning", FString::Printf(TEXT("Lobbie(s) found: %d."), SearchSettings->SearchResults.Num()));
		if(OnlineSubsystem)
		{
			if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{

				if(SearchSettings->SearchResults.Num())
				{
					SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UBaseInstance::OnJoinSessionComplete);
					SessionPtr->JoinSession(0, SessionName_const, SearchSettings->SearchResults[0]);
				}
			}
		}
	} else {
		UCSL_Window::PrintToConsole(Self, "Warning",
			FString::Printf(TEXT("Session failed to be found. Code: %d."), bWasSuccessful));
	}

	if(OnlineSubsystem)
	{
		if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionPtr->ClearOnFindSessionsCompleteDelegates(this);
		}
	}
}

void UBaseInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if(OnlineSubsystem)
	{
		if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			FString ConInfo = FString();
			SessionPtr->GetResolvedConnectString(SessionName_const, ConInfo);
			if (!ConInfo.IsEmpty() && GetWorld()->IsValidLowLevel())
			{
				if (APlayerController* PController = GetWorld()->GetFirstPlayerController())
				{
					PController->ClientTravel(ConInfo, ETravelType::TRAVEL_Absolute);
				}
			}
		}
	}
}

void UBaseInstance::GetAllFriends()
{
	if(StaticGameType == "Singleplayer with Online features" || StaticGameType == "Multiplayer"
			|| StaticGameType == "Both")
	{
		if(bIsLoggedIn && OnlineSubsystem)
		{
			if(IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface())
			{
				FriendsPtr->ReadFriendsList(0, FString(""), FOnReadFriendsListComplete::CreateUObject(this, &UBaseInstance::OnGetAllFriendsComplete));
			}
		}
	} else {
		UCSL_Window::PrintToConsole(Self, "Warning",
			"You can't get the friend list while the GameType is set to: Singleplayer.");
	}
}

void UBaseInstance::OnGetAllFriendsComplete(int32 LocalUserNumber, bool bWasSuccessful, const FString& ListName,
	const FString& ErrorStr)
{
	UCSL_Window::PrintToConsole(Self, "Warning", FString::Printf(TEXT("Friends retrieved with success. Code: %d."), bWasSuccessful));
	if(bWasSuccessful && OnlineSubsystem)
	{
		if(IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface())
		{
			TArray<TSharedRef<FOnlineFriend>> FriendsList;
			if(FriendsPtr->GetFriendsList(0, ListName, FriendsList))
			{
				for(TSharedRef<FOnlineFriend> Friend : FriendsList)
				{
					FriendName = Friend.Get().GetRealName();
					UCSL_Window::PrintToConsole(Self, "Warning", FString::Printf(TEXT("%s"), *FriendName));
				}
			} else {
				UCSL_Window::PrintToConsole(Self, "Warning", "Failed to retrieve the friends list.");
			}
		}
	}
}

void UBaseInstance::ShowInviteUI()
{
	if(StaticGameType == "Singleplayer with Online features" || StaticGameType == "Multiplayer"
			|| StaticGameType == "Both")
	{
		if(bIsLoggedIn && OnlineSubsystem)
		{
			if(IOnlineExternalUIPtr UIPtr = OnlineSubsystem->GetExternalUIInterface())
			{
				UIPtr->ShowInviteUI(0);
			}
		}
	} else {
		UCSL_Window::PrintToConsole(Self, "Warning",
			"You can't invite people while the GameType is set to: Singleplayer.");
	}
}

void UBaseInstance::ShowFriendsUI()
{
	if(StaticGameType == "Singleplayer with Online features" || StaticGameType == "Multiplayer"
			|| StaticGameType == "Both")
	{
		if(bIsLoggedIn && OnlineSubsystem)
		{
			if(IOnlineExternalUIPtr UIPtr = OnlineSubsystem->GetExternalUIInterface())
			{
				UIPtr->ShowFriendsUI(0);
			}
		}
	} else {
		UCSL_Window::PrintToConsole(Self, "Warning",
			"You can't get the friend list while the GameType is set to: Singleplayer.");
	}
}

void UBaseInstance::ShowAchievementsUI()
{
	if(StaticGameType == "Singleplayer with Online features" || StaticGameType == "Multiplayer"
			|| StaticGameType == "Both")
	{
		if(bIsLoggedIn && OnlineSubsystem)
		{
			if(IOnlineExternalUIPtr UIPtr = OnlineSubsystem->GetExternalUIInterface())
			{
				UIPtr->ShowAchievementsUI(0);
			}
		}
	} else {
		UCSL_Window::PrintToConsole(Self, "Warning",
			"You can't get the achievements list while the GameType is set to: Singleplayer.");
	}
}
