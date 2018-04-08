#pragma once
#include "Common.h"

class CommandSender
{
private:
	long long senderID;
	SystemAddress clientAddress;
public:
	CommandSender(long long senderID, SystemAddress clientAddress = UNASSIGNED_SYSTEM_ADDRESS);

	void sendMessage(string msg);

	long long getSenderID();
	SystemAddress getClientAddress();
};

class Commands
{
public:
	static char getCommandID(string cmd);
	static char getTrustForCommand(string cmd);
	static char getTrustForCommand(char id);
	static void performCommand(CommandSender sender, string cmd, vector<string> args);
};