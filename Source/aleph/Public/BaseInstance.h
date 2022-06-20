#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "BaseInstance.generated.h"

UCLASS()
class ALEPH_API UBaseInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UBaseInstance();
	virtual void Init() override;
	virtual void Shutdown() override;

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Client|Preferences", meta = (GetOptions = "GameTypeArray"))
		FString GameType = "Singleplayer";
	UFUNCTION(BlueprintCallable, Category = "Client|Preferences")
		TArray<FString> GameTypeArray() const
		{
			return
			{
				"Singleplayer",
				"Singleplayer with Online features",
				"Multiplayer",
				"Both"
			};
		};

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Client|Online|Status")
		bool bIsLoggedIn;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Client|Online|Status")
		bool bIsSessionLive;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Client|Online|Info")
		FString FriendName;
	UFUNCTION(BlueprintPure, Category = "Client|Online|Info")
		static FString GetPlayerName();

public:
	UFUNCTION(Exec, BlueprintCallable, Category = "Client|Online|Login")
		void Login();
		void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	
	UFUNCTION(Exec, BlueprintCallable, Category = "Client|Online|Sessions")
		void CreateSession();
		void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(Exec, BlueprintCallable, Category = "Client|Online|Sessions")
		void DestroySession();
		void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(Exec, BlueprintCallable, Category = "Client|Online|Sessions")
		void FindSession();
		void OnFindSessionComplete(bool bWasSuccessful);
		TSharedPtr<FOnlineSessionSearch> SearchSettings;
		void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION(Exec, BlueprintCallable, Category = "Client|Friends")
		void GetAllFriends();
		void OnGetAllFriendsComplete(int32 LocalUserNumber, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	UFUNCTION(Exec, BlueprintCallable, Category = "Client|UI")
		void ShowInviteUI();
	UFUNCTION(Exec, BlueprintCallable, Category = "Client|UI")
		void ShowFriendsUI();
	UFUNCTION(Exec, BlueprintCallable, Category = "Client|UI")
		void ShowAchievementsUI();
};