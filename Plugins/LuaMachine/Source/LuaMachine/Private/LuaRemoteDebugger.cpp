// Copyright 2025 - Roberto De Ioris

#include "LuaCode.h"
#include "LuaMachine.h"

#include "Common/TcpSocketBuilder.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

#include "HAL/ThreadManager.h"

#if LUAMACHINE_LUA53
class FLuaRemoteDebugger : public FRunnable
{

public:
	FLuaRemoteDebugger() = delete;

	FLuaRemoteDebugger(const FString& InHostAndPort)
	{
		CachedSocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		HostAndPort = InHostAndPort;
	}

	virtual bool Init() override
	{
		FIPv4Endpoint Endpoint;
		// is the address valid?
		if (!FIPv4Endpoint::FromHostAndPort(HostAndPort, Endpoint))
		{
			return false;
		}

		const FString SocketName = FString::Printf(TEXT("LuaRemoteDebuggerSocket@%s"), *HostAndPort);

		Socket = FTcpSocketBuilder(*SocketName)
			.AsReusable()
			.AsNonBlocking()
			.BoundToEndpoint(Endpoint)
			.Listening(1);

		if (!Socket)
		{
			return false;
		}

		bBound = true;

		return true;
	}

	virtual uint32 Run() override;

	virtual void Exit() override
	{

	}

	virtual void Stop() override
	{
		bThreadRunning = false;
	}

	virtual bool IsDead() const
	{
		return bDead;
	}

	virtual bool IsBound() const
	{
		return bBound;
	}

	virtual bool ParseMessage(const TArray<uint8>& Data, const int32 DataNum, int32& BytesToRemove, TSharedPtr<FJsonObject>& JsonObject);

	virtual TSharedRef<FJsonObject> CreateEvent(const FString& EventName, TFunction<void(TSharedRef<FJsonObject>)> BodyFiller);

	virtual void AppendMessage(TArray<uint8>& OutputBuffer, const TSharedRef<FJsonObject>& JsonMessage);

	TQueue<TSharedPtr<FJsonObject>, EQueueMode::Spsc> ClientToGameThreadQueue;
	TQueue<TSharedPtr<FJsonObject>, EQueueMode::Spsc> GameThreadToClientQueue;

	virtual TSharedRef<FJsonObject> PrepareResponse(const int64 Seq, const FString Command);

	bool bPaused = false;
	int32 CurrentModeMask = LUA_MASKLINE | LUA_MASKCALL;
	int32 WaitingForStackDepth = -1;

protected:
	bool bThreadRunning = false;
	bool bDead = false;
	bool bBound = false;
	FString HostAndPort;
	FSocket* Socket = nullptr;
	ISocketSubsystem* CachedSocketSubsystem = nullptr;
	std::atomic<int64> ResponseSeq = 0;
};

TSharedRef<FJsonObject> FLuaRemoteDebugger::PrepareResponse(const int64 Seq, const FString Command)
{
	TSharedRef<FJsonObject> JsonResponse = MakeShared<FJsonObject>();
	JsonResponse->SetNumberField(TEXT("seq"), ResponseSeq++);
	JsonResponse->SetStringField(TEXT("type"), "response");
	JsonResponse->SetNumberField(TEXT("request_seq"), Seq);
	JsonResponse->SetStringField(TEXT("command"), Command);

	return JsonResponse;
}

static bool LuaFindField_Internal(lua_State* L, const int32 ObjectIndex, const int32 Level)
{
	if (Level == 0 || !lua_istable(L, -1))
	{
		return false;
	}
	lua_pushnil(L);  /* start 'next' loop */
	while (lua_next(L, -2)) {  /* for each pair in table */
		if (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
			if (lua_rawequal(L, ObjectIndex, -1)) {  /* found object? */
				lua_pop(L, 1);  /* remove value (but keep name) */
				return true;
			}
			else if (LuaFindField_Internal(L, ObjectIndex, Level - 1)) {  /* try recursively */
				lua_remove(L, -2);  /* remove table (but keep name) */
				lua_pushliteral(L, ".");
				lua_insert(L, -2);  /* place '.' between the two names */
				lua_concat(L, 3);
				return true;
			}
		}
		lua_pop(L, 1);  /* remove value */
	}
	return false;  /* not found */
};

static void RemoteDebugger_Hook(lua_State* L, lua_Debug* Ar)
{
	ULuaState* LuaState = ULuaState::GetFromExtraSpace(L);

	FLuaRemoteDebugger* RemoteDebugger = LuaState->GetLuaRemoteDebugger();

	// check if the event is interesting
	if (((1 << Ar->event) & RemoteDebugger->CurrentModeMask) == 0)
	{
		return;
	}

	if (RemoteDebugger->WaitingForStackDepth >= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("RemoteDebugger_Hook %d %d %d"), Ar->event, RemoteDebugger->WaitingForStackDepth, Ar->currentline);
		if (RemoteDebugger->WaitingForStackDepth > 0 && LuaState->GetStackDepth() > RemoteDebugger->WaitingForStackDepth)
		{
			return;
		}
		RemoteDebugger->WaitingForStackDepth = -1;
		RemoteDebugger->bPaused = true;
		RemoteDebugger->GameThreadToClientQueue.Enqueue(RemoteDebugger->CreateEvent("stopped",
			[](TSharedRef<FJsonObject> Body)
			{
				Body->SetStringField(TEXT("reason"), "step");
				Body->SetStringField(TEXT("description"), "GameThread paused");
				Body->SetNumberField(TEXT("threadId"), GGameThreadId);
			}));
	}

	int32 VariablesCounter = 1;
	TMap<int32, TPair<FString, FLuaValue>> GlobalVariables;
	TArray<TTuple<FString, FString, int32>> StackTrace;

	auto PushGlobalFuncName = [](lua_State* L, lua_Debug* Ar)
		{
			int32 Top = lua_gettop(L);
			lua_getinfo(L, "f", Ar);  /* push function */
			lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
			if (LuaFindField_Internal(L, Top + 1, 2)) {
				const char* Name = lua_tostring(L, -1);
				if (FCStringAnsi::Strncmp(Name, "_G.", 3) == 0) {  /* name start with '_G.'? */
					lua_pushstring(L, Name + 3);  /* push name without prefix */
					lua_remove(L, -2);  /* remove original name */
				}
				lua_copy(L, -1, Top + 1);  /* move name to proper place */
				lua_pop(L, 2);  /* remove pushed values */
				return true;
			}
			else {
				lua_settop(L, Top);  /* remove function and global table */
				return false;
			}
		};

	auto GetFunctionName = [PushGlobalFuncName](lua_State* L, lua_Debug* Ar) -> FString
		{
			if (PushGlobalFuncName(L, Ar)) {  /* try first a global name */
				const FString FuncName = UTF8_TO_TCHAR(lua_tostring(L, -1));
				lua_remove(L, -2);  /* remove name */
				return FuncName;
			}

			if (*Ar->namewhat != '\0')
			{
				return UTF8_TO_TCHAR(Ar->name);
			}

			if (*Ar->what == 'm')  /* main? */
			{
				return "<main>";
			}

			if (*Ar->what != 'C')
			{
				return FString::Printf(TEXT("function <%s:%d>"), UTF8_TO_TCHAR(Ar->short_src), Ar->linedefined);
			}

			return "?";
		};

	auto CollectDebugInfo = [&]()
		{
			// build snapshot
			VariablesCounter = 1;
			GlobalVariables.Empty();
			StackTrace.Empty();
			lua_getinfo(L, "lSn", Ar);

			UE_LOG(LogTemp, Error, TEXT("Calling %s! %s %d %d - %s %s"), UTF8_TO_TCHAR(Ar->what), UTF8_TO_TCHAR(Ar->source), Ar->currentline, Ar->event, UTF8_TO_TCHAR(Ar->name), UTF8_TO_TCHAR(Ar->namewhat));

			LuaState->PushGlobalTable();
			LuaState->PushNil(); // first key
			while (LuaState->Next(-2))
			{
				const FLuaValue CurrentLuaKey = LuaState->ToLuaValue(-2);
				const FLuaValue CurrentLuaValue = LuaState->ToLuaValue(-1);
				GlobalVariables.Add(VariablesCounter++, TPair<FString, FLuaValue>(CurrentLuaKey.ToString(), CurrentLuaValue));
				LuaState->Pop(); // pop the value
			}

			LuaState->Pop(); // pop the table

			int32 StackLevel = 0;

			printf("Lua stack trace:\n");

			lua_Debug StackTraceAr;
			while (lua_getstack(L, StackLevel, &StackTraceAr))
			{
				if (lua_getinfo(L, "Snl", &StackTraceAr))
				{
					const char* Source = StackTraceAr.source ? StackTraceAr.source : "unknown";
					const int32 CurrentLine = StackTraceAr.currentline;

					StackTrace.Add(TTuple<FString, FString, int32>(GetFunctionName(L, &StackTraceAr), UTF8_TO_TCHAR(Source), CurrentLine));
				}
				StackLevel++;
			}
		};

	// wait for a message before continuing
	while (RemoteDebugger->bPaused)
	{
		if (!RemoteDebugger->ClientToGameThreadQueue.IsEmpty())
		{
			break;
		}
	}

	TSharedPtr<FJsonObject> JsonMessage = nullptr;
	while (RemoteDebugger->ClientToGameThreadQueue.Dequeue(JsonMessage))
	{
		// signal disconnection
		if (!JsonMessage)
		{
			RemoteDebugger->bPaused = false;
			RemoteDebugger->WaitingForStackDepth = -1;
			continue;
		}

		int64 Seq;
		FString Type;
		FString Command;

		if (!JsonMessage->TryGetNumberField(TEXT("seq"), Seq))
		{
			continue;
		}

		if (!JsonMessage->TryGetStringField(TEXT("type"), Type))
		{
			continue;
		}

		if (!JsonMessage->TryGetStringField(TEXT("command"), Command))
		{
			continue;
		}

		const TSharedPtr<FJsonObject>* JsonArguments = nullptr;
		if (!JsonMessage->TryGetObjectField(TEXT("arguments"), JsonArguments))
		{
			JsonArguments = nullptr;
		}

		UE_LOG(LogTemp, Warning, TEXT("seq: %lld type: %s command: %s"), Seq, *Type, *Command);

		if (Type == "request")
		{
			TArray<TSharedRef<FJsonObject>> JsonEvents;

			TSharedRef<FJsonObject> JsonResponse = RemoteDebugger->PrepareResponse(Seq, Command);

			if (Command == "initialize")
			{
				JsonEvents.Add(RemoteDebugger->CreateEvent("initialized", nullptr));
			}
			else if (Command == "attach")
			{
				JsonEvents.Add(RemoteDebugger->CreateEvent("output", [](TSharedRef<FJsonObject> Body)
					{
						Body->SetStringField(TEXT("category"), "console");
						Body->SetStringField(TEXT("output"), "Hello World\n");
					}));
				JsonEvents.Add(RemoteDebugger->CreateEvent("output", [](TSharedRef<FJsonObject> Body)
					{
						Body->SetStringField(TEXT("category"), "important");
						Body->SetStringField(TEXT("output"), "Hello World 2\n");
					}));
				JsonEvents.Add(RemoteDebugger->CreateEvent("output", [](TSharedRef<FJsonObject> Body)
					{
						Body->SetStringField(TEXT("category"), "stdout");
						Body->SetStringField(TEXT("output"), "Hello World 3\n");
					}));
				JsonEvents.Add(RemoteDebugger->CreateEvent("output", [](TSharedRef<FJsonObject> Body)
					{
						Body->SetStringField(TEXT("category"), "stderr");
						Body->SetStringField(TEXT("output"), "Hello World 4\n");
					}));
				JsonEvents.Add(RemoteDebugger->CreateEvent("output", [](TSharedRef<FJsonObject> Body)
					{
						Body->SetStringField(TEXT("category"), "telemetry");
						Body->SetStringField(TEXT("output"), "Hello World 5\n");
					}));
			}
			else if (Command == "threads")
			{
				TArray<TSharedPtr<FJsonValue>> JsonThreads;

				TSharedRef<FJsonObject> JsonGameThread = MakeShared<FJsonObject>();
				JsonGameThread->SetNumberField(TEXT("id"), GGameThreadId);
				JsonGameThread->SetStringField(TEXT("name"), FThreadManager::Get().GetThreadName(GGameThreadId));

				JsonThreads.Add(MakeShared<FJsonValueObject>(JsonGameThread));

				TSharedRef<FJsonObject> JsonResponseBody = MakeShared<FJsonObject>();
				JsonResponseBody->SetArrayField(TEXT("threads"), JsonThreads);
				JsonResponse->SetObjectField(TEXT("body"), JsonResponseBody);
			}
			else if (Command == "stackTrace")
			{
				CollectDebugInfo();
				int32 StartFrame = 0;
				int32 Levels = StackTrace.Num();
				if (JsonArguments)
				{
					if (!(*JsonArguments)->TryGetNumberField(TEXT("startFrame"), StartFrame))
					{
						StartFrame = 0;
					}

					if (!(*JsonArguments)->TryGetNumberField(TEXT("levels"), Levels))
					{
						Levels = StackTrace.Num();
					}
				}

				TArray<TSharedPtr<FJsonValue>> JsonStackFrames;

				for (int32 FrameIndex = StartFrame; FrameIndex < FMath::Min(StartFrame + Levels, StackTrace.Num()); FrameIndex++)
				{
					TSharedRef<FJsonObject> JsonStackFrame = MakeShared<FJsonObject>();
					JsonStackFrame->SetNumberField(TEXT("id"), FrameIndex);
					JsonStackFrame->SetStringField(TEXT("name"), StackTrace[FrameIndex].Get<0>());

					FString Path = StackTrace[FrameIndex].Get<1>();
					// valid path?
					if (Path.StartsWith("@"))
					{
						TSharedRef<FJsonObject> JsonSource = MakeShared<FJsonObject>();
						Path = Path.Mid(1);
						JsonSource->SetStringField(TEXT("path"), Path);
						JsonStackFrame->SetObjectField(TEXT("source"), JsonSource);
					}

					JsonStackFrame->SetNumberField(TEXT("line"), StackTrace[FrameIndex].Get<2>());
					JsonStackFrame->SetNumberField(TEXT("column"), 0);

					JsonStackFrames.Add(MakeShared<FJsonValueObject>(JsonStackFrame));
				}

				TSharedRef<FJsonObject> JsonResponseBody = MakeShared<FJsonObject>();
				JsonResponseBody->SetArrayField(TEXT("stackFrames"), JsonStackFrames);
				JsonResponseBody->SetNumberField(TEXT("totalFrames"), StackTrace.Num());
				JsonResponse->SetObjectField(TEXT("body"), JsonResponseBody);
			}
			else if (Command == "scopes")
			{
				TArray<TSharedPtr<FJsonValue>> JsonScopes;

				TSharedRef<FJsonObject> JsonScopeGlobal = MakeShared<FJsonObject>();
				JsonScopeGlobal->SetStringField(TEXT("name"), "Globals");
				JsonScopeGlobal->SetNumberField(TEXT("variablesReference"), 100);

				JsonScopes.Add(MakeShared<FJsonValueObject>(JsonScopeGlobal));

				TSharedRef<FJsonObject> JsonScopeLocal = MakeShared<FJsonObject>();
				JsonScopeLocal->SetStringField(TEXT("name"), "Locals");
				JsonScopeLocal->SetNumberField(TEXT("variablesReference"), 200);

				JsonScopes.Add(MakeShared<FJsonValueObject>(JsonScopeLocal));

				TSharedRef<FJsonObject> JsonScopeUpValues = MakeShared<FJsonObject>();
				JsonScopeUpValues->SetStringField(TEXT("name"), "UpValues");
				JsonScopeUpValues->SetNumberField(TEXT("variablesReference"), 300);

				JsonScopes.Add(MakeShared<FJsonValueObject>(JsonScopeUpValues));

				TSharedRef<FJsonObject> JsonResponseBody = MakeShared<FJsonObject>();
				JsonResponseBody->SetArrayField(TEXT("scopes"), JsonScopes);
				JsonResponse->SetObjectField(TEXT("body"), JsonResponseBody);
			}
			else if (Command == "variables")
			{
				CollectDebugInfo();
				TArray<TSharedPtr<FJsonValue>> JsonVariables;

				for (const TPair<int32, TPair<FString, FLuaValue>>& Pair : GlobalVariables)
				{
					TSharedRef<FJsonObject> JsonVariable = MakeShared<FJsonObject>();
					JsonVariable->SetStringField(TEXT("name"), Pair.Value.Key);
					JsonVariable->SetStringField(TEXT("value"), Pair.Value.Value.ToString());
					JsonVariable->SetNumberField(TEXT("variablesReference"), Pair.Value.Value.Type == ELuaValueType::Table ? Pair.Key : 0);
					JsonVariables.Add(MakeShared<FJsonValueObject>(JsonVariable));
				}

				TSharedRef<FJsonObject> JsonResponseBody = MakeShared<FJsonObject>();
				JsonResponseBody->SetArrayField(TEXT("variables"), JsonVariables);
				JsonResponse->SetObjectField(TEXT("body"), JsonResponseBody);
			}
			else if (Command == "pause")
			{
				JsonEvents.Add(RemoteDebugger->CreateEvent("stopped", [](TSharedRef<FJsonObject> Body)
					{
						Body->SetStringField(TEXT("reason"), "pause");
						Body->SetStringField(TEXT("description"), "GameThread paused");
						Body->SetNumberField(TEXT("threadId"), GGameThreadId);
					}));
				RemoteDebugger->bPaused = true;
			}
			else if (Command == "continue" && RemoteDebugger->bPaused)
			{
				RemoteDebugger->bPaused = false;
				RemoteDebugger->CurrentModeMask = LUA_MASKLINE | LUA_MASKCALL;
			}
			else if (Command == "next" && RemoteDebugger->bPaused)
			{
				RemoteDebugger->bPaused = false;
				RemoteDebugger->CurrentModeMask = LUA_MASKLINE | LUA_MASKRET;
				RemoteDebugger->WaitingForStackDepth = LuaState->GetStackDepth();
			}
			else if (Command == "stepIn" && RemoteDebugger->bPaused)
			{
				RemoteDebugger->bPaused = false;
				RemoteDebugger->CurrentModeMask = LUA_MASKLINE | LUA_MASKCALL;
				RemoteDebugger->WaitingForStackDepth = 0;
			}
			else if (Command == "stepOut" && RemoteDebugger->bPaused)
			{
				RemoteDebugger->bPaused = false;
				RemoteDebugger->CurrentModeMask = LUA_MASKRET;
				RemoteDebugger->WaitingForStackDepth = LuaState->GetStackDepth();
			}
			else if (Command == "evaluate")
			{
				if (JsonArguments)
				{
					FString Expression;
					if ((*JsonArguments)->TryGetStringField(TEXT("expression"), Expression))
					{
						if (!Expression.IsEmpty())
						{
							const int DebugMask = LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET;
							lua_sethook(L, nullptr, DebugMask, 0);
							const bool bLogError = LuaState->bLogError;
							LuaState->bLogError = false;
							FLuaValue EvaluationResult = LuaState->RunString("return " + Expression, "");
							if (!LuaState->LastError.IsEmpty())
							{
								EvaluationResult = LuaState->RunString(Expression, "");
							}
							LuaState->bLogError = bLogError;
							LuaState->LastError = "";
							lua_sethook(L, RemoteDebugger_Hook, DebugMask, 0);

							TSharedRef<FJsonObject> JsonResponseBody = MakeShared<FJsonObject>();
							JsonResponseBody->SetStringField(TEXT("result"), EvaluationResult.ToString());
							JsonResponse->SetObjectField(TEXT("body"), JsonResponseBody);
						}
					}
				}
			}

			JsonResponse->SetBoolField(TEXT("success"), true);

			RemoteDebugger->GameThreadToClientQueue.Enqueue(JsonResponse);

			for (const TSharedRef<FJsonObject>& JsonEvent : JsonEvents)
			{
				RemoteDebugger->GameThreadToClientQueue.Enqueue(JsonEvent);
			}

			if (RemoteDebugger->bPaused)
			{
				while (RemoteDebugger->ClientToGameThreadQueue.IsEmpty())
				{
					continue;
				}
			}
		}
	}
}

uint32 FLuaRemoteDebugger::Run()
{
	// no need to lock as it is a one-shot variable
	bThreadRunning = true;

	while (bThreadRunning)
	{
		bool bHasPendingConnection = false;
		if (!Socket->WaitForPendingConnection(bHasPendingConnection, FTimespan::FromMilliseconds(500)))
		{
			bThreadRunning = false;
			break;
		}

		if (!bHasPendingConnection)
		{
			continue;
		}

		const int32 ReceiveBufferChunkSize = 4096;

		TSharedRef<FInternetAddr> NewClientInternetAddr = CachedSocketSubsystem->CreateInternetAddr();
		FString NewClientSocketDescription;
		FSocket* NewClientSocket = Socket->Accept(NewClientInternetAddr.Get(), NewClientSocketDescription);
		if (NewClientSocket)
		{
			if (!NewClientSocket->SetNonBlocking())
			{
				break;
			}

			TArray<uint8> ReceiveBuffer;
			ReceiveBuffer.AddUninitialized(ReceiveBufferChunkSize);
			int32 ReceiveBufferOffset = 0;
			UE_LOG(LogLuaMachine, Log, TEXT("Lua Remote Debugger %s attached"), *NewClientInternetAddr->ToString(true));
			// reset seq for responses
			ResponseSeq = 1;
			while (bThreadRunning)
			{
				// first check if there are messages to enqueue
				TArray<uint8> ResponseBuffer;
				TSharedPtr<FJsonObject> JsonResponse;
				while (GameThreadToClientQueue.Dequeue(JsonResponse))
				{
					AppendMessage(ResponseBuffer, JsonResponse.ToSharedRef());
				}

				// something to send ?
				if (ResponseBuffer.Num() > 0)
				{
					int32 Remains = ResponseBuffer.Num();
					while (Remains > 0)
					{
						if (!NewClientSocket->Wait(ESocketWaitConditions::WaitForWrite, FTimespan::FromSeconds(30)))
						{
							break;
						}
						int32 BytesSent = 0;
						if (!NewClientSocket->Send(ResponseBuffer.GetData() + (ResponseBuffer.Num() - Remains), Remains, BytesSent) || BytesSent < 0)
						{
							break;
						}
						Remains -= BytesSent;
					}
				}

				// wait for socket...
				if (!NewClientSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromMilliseconds(10)))
				{
					// retry on timeout
					continue;
				}

				if (ReceiveBuffer.Num() - ReceiveBufferOffset < ReceiveBufferChunkSize)
				{
					ReceiveBuffer.AddUninitialized(ReceiveBufferChunkSize);
				}
				int32 BytesRead = 0;
				if (!NewClientSocket->Recv(ReceiveBuffer.GetData() + ReceiveBufferOffset, ReceiveBufferChunkSize, BytesRead))
				{
					break;
				}

				if (BytesRead <= 0)
				{
					break;
				}

				ReceiveBufferOffset += BytesRead;
				bool bBroken = false;
				while (ReceiveBufferOffset > 0)
				{
					int32 BytesToRemove = 0;
					TArray<uint8> Response;
					TSharedPtr<FJsonObject> JsonObject = nullptr;
					if (!ParseMessage(ReceiveBuffer, ReceiveBufferOffset, BytesToRemove, JsonObject))
					{
						bBroken = true;
						break;
					}

					if (BytesToRemove >= 0)
					{
						ReceiveBuffer.RemoveAt(0, BytesToRemove, EAllowShrinking::Default);
						ReceiveBufferOffset -= BytesToRemove;

						if (JsonObject)
						{
							ClientToGameThreadQueue.Enqueue(JsonObject);
						}
					}
					else
					{
						break;
					}
				}

				if (bBroken)
				{
					break;
				}
			}
			ClientToGameThreadQueue.Enqueue(nullptr);
			UE_LOG(LogLuaMachine, Log, TEXT("Lua Remote Debugger %s detached"), *NewClientInternetAddr->ToString(true));
			NewClientSocket->Shutdown(ESocketShutdownMode::ReadWrite);
			NewClientSocket->Close();
			CachedSocketSubsystem->DestroySocket(NewClientSocket);
		}
	}

	Socket->Shutdown(ESocketShutdownMode::ReadWrite);
	Socket->Close();
	CachedSocketSubsystem->DestroySocket(Socket);
	bDead = true;

	return 0;
}

bool FLuaRemoteDebugger::ParseMessage(const TArray<uint8>& Data, const int32 DataNum, int32& BytesToRemove, TSharedPtr<FJsonObject>& JsonObject)
{
	TArray<FString> Headers;
	TArray<uint8> CurrentHeader;

	bool bFoundCR = false;
	bool bReadyForBody = false;

	int32 Index = 0;

	for (Index = 0; Index < DataNum; Index++)
	{
		const uint8 Byte = Data[Index];
		if (bFoundCR)
		{
			if (Byte == '\n')
			{
				bFoundCR = false;
				// \r\n\r\n ?
				if (CurrentHeader.Num() == 0)
				{
					bReadyForBody = true;
					break;
				}
				CurrentHeader.Add(0);
				Headers.Add(UTF8_TO_TCHAR(CurrentHeader.GetData()));
				CurrentHeader.Empty();
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (Byte == '\n')
			{
				return false;
			}
			else if (Byte == '\r')
			{
				bFoundCR = true;
			}
			else
			{
				CurrentHeader.Add(Byte);
			}
		}
	}

	if (!bReadyForBody)
	{
		return true;
	}

	bool bFoundContentLength = false;
	int32 ContentLength = 0;

	for (const FString& Header : Headers)
	{
		if (Header.StartsWith("Content-Length:"))
		{
			ContentLength = FCString::Atoi(*Header.Mid(15).TrimStartAndEnd());
			bFoundContentLength = true;
			break;
		}
	}

	if (!bFoundContentLength || ContentLength < 0)
	{
		return false;
	}

	TArray<uint8> Body;
	if (DataNum - (Index + 1) >= ContentLength)
	{
		Body.Append(Data.GetData() + Index + 1, ContentLength);
		BytesToRemove = (Index + 1) + ContentLength;
		Body.Add(0);

		FString JsonMessage = UTF8_TO_TCHAR(Body.GetData());

		UE_LOG(LogTemp, Error, TEXT("Body: %s"), *JsonMessage);

		TSharedPtr<FJsonValue> JsonValue = nullptr;
		const TSharedPtr<FJsonObject>* JsonObjectPtr = nullptr;

		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonMessage);

		if (!FJsonSerializer::Deserialize(JsonReader, JsonValue))
		{
			return false;
		}

		if (!JsonValue->TryGetObject(JsonObjectPtr))
		{
			return false;
		}

		JsonObject = *JsonObjectPtr;
	}

	return true;
}

TSharedRef<FJsonObject> FLuaRemoteDebugger::CreateEvent(const FString& EventName, TFunction<void(TSharedRef<FJsonObject>)> BodyFiller)
{
	TSharedRef<FJsonObject> JsonEventRoot = MakeShared<FJsonObject>();
	JsonEventRoot->SetNumberField(TEXT("seq"), ResponseSeq++);
	JsonEventRoot->SetStringField(TEXT("type"), "event");
	JsonEventRoot->SetStringField(TEXT("event"), EventName);

	if (BodyFiller)
	{
		TSharedRef<FJsonObject> JsonEventBody = MakeShared<FJsonObject>();
		BodyFiller(JsonEventBody);
		JsonEventRoot->SetObjectField(TEXT("body"), JsonEventBody);
	}

	return JsonEventRoot;
}

void FLuaRemoteDebugger::AppendMessage(TArray<uint8>& OutputBuffer, const TSharedRef<FJsonObject>& JsonMessage)
{
	FString Json;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Json);
	FJsonSerializer::Serialize(MakeShared<FJsonValueObject>(JsonMessage), "", JsonWriter);

	FTCHARToUTF8 UTF8StringMessage(*Json);

	const FString Header = FString::Printf(TEXT("Content-Length: %d\r\n\r\n"), UTF8StringMessage.Length());

	FTCHARToUTF8 UTF8StringHeader(*Header);

	OutputBuffer.Append(reinterpret_cast<const uint8*>(UTF8StringHeader.Get()), UTF8StringHeader.Length());
	OutputBuffer.Append(reinterpret_cast<const uint8*>(UTF8StringMessage.Get()), UTF8StringMessage.Length());
}

bool ULuaState::StartRemoteDebugger(const FString& HostAndPort)
{
	if (bRemoteDebuggerStarted || LuaRemoteDebugger || LuaRemoteDebuggerThread)
	{
		return false;
	}

	const FString SocketThreadName = FString::Printf(TEXT("FLuaRemoteDebuggerThread@%s"), *HostAndPort);

	LuaRemoteDebugger = new FLuaRemoteDebugger(HostAndPort);

	if (!LuaRemoteDebugger)
	{
		return false;
	}

	LuaRemoteDebuggerThread = FRunnableThread::Create(LuaRemoteDebugger, *SocketThreadName);

	if (!LuaRemoteDebuggerThread)
	{
		delete LuaRemoteDebugger;
		LuaRemoteDebugger = nullptr;
		return false;
	}

	if (!LuaRemoteDebugger->IsBound())
	{
		LuaRemoteDebuggerThread->Kill();

		delete LuaRemoteDebuggerThread;
		LuaRemoteDebuggerThread = nullptr;

		delete LuaRemoteDebugger;
		LuaRemoteDebugger = nullptr;
		return false;
	}

#if LUAMACHINE_LUA53 || LUAMACHINE_LUAJIT
	const int DebugMask = LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET;
	lua_sethook(L, RemoteDebugger_Hook, DebugMask, 0);
#elif LUAMACHINE_LUAU
	lua_Callbacks* Callbacks = lua_callbacks(L);
	Callbacks->debugstep = RemoteDebugger_Hook;
#endif

	bRemoteDebuggerStarted = true;

	return true;
}

void ULuaState::StopRemoteDebugger()
{
	if (bRemoteDebuggerStarted)
	{
#if LUAMACHINE_LUA53 || LUAMACHINE_LUAJIT
		const int DebugMask = LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET;
		lua_sethook(L, nullptr, DebugMask, 0);
#elif LUAMACHINE_LUAU
		lua_Callbacks* Callbacks = lua_callbacks(L);
		Callbacks->debugstep = nullptr;
#endif
	}

	if (LuaRemoteDebugger)
	{
		if (!LuaRemoteDebugger->IsDead())
		{
			if (LuaRemoteDebuggerThread)
			{
				LuaRemoteDebuggerThread->Kill();
			}
		}
	}

	// delete FRunnableThread
	if (LuaRemoteDebuggerThread)
	{
		delete LuaRemoteDebuggerThread;
		LuaRemoteDebuggerThread = nullptr;
	}

	// delete FRunnable
	if (LuaRemoteDebugger)
	{
		delete LuaRemoteDebugger;
		LuaRemoteDebugger = nullptr;
	}

	bRemoteDebuggerStarted = false;
}
#else

bool ULuaState::StartRemoteDebugger(const FString& HostAndPort)
{
	return false;
}

void ULuaState::StopRemoteDebugger()
{
}
#endif