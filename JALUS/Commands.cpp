#include "Commands.h"
#include "ServerLoop.h"
#include "Accounts.h"
#include "SHA512.h"
#include "TimeUtils.h"
#include "Bans.h"
#include "CDClient.h"
#include "ObjectsManager.h"
#include "Chat.h"
#include "PacketUtils.h"
#include "Characters.h"
#include "Validate.h"
#include "Objects.h"
#include "GameMessages.h"
#include "Config.h"
#include "TransitionInfos.h"
#include "Sessions.h"
#include "General.h"
#include "PingTool.h"
#include "LUZCache.h"
#include "Scheduler.h"
#include "Helpers.h"

char Commands::getCommandID(string cmd)
{
	if (iequals(cmd, "stop") || iequals(cmd, "off") || iequals(cmd, "quit"))
		return 1;
	else if (iequals(cmd, "test"))
		return 2;
	else if (iequals(cmd, "ping"))
		return 3;
	else if (iequals(cmd, "createAccount"))
		return 4;
	else if (iequals(cmd, "deleteAccount"))
		return 5;
	else if (iequals(cmd, "banAccount"))
		return 6;
	else if (iequals(cmd, "setTrust"))
		return 7;
	else if (iequals(cmd, "banIP"))
		return 8;
	else if (iequals(cmd, "spawn"))
		return 9;
	else if (iequals(cmd, "despawn"))
		return 10;
	else if (iequals(cmd, "nearme") || iequals(cmd, "aroundme"))
		return 11;
	else if (iequals(cmd, "tp") || iequals(cmd, "tele") || iequals(cmd, "teleport"))
		return 12;
	else if (iequals(cmd, "testmap") || iequals(cmd, "map"))
		return 13;
	else if (iequals(cmd, "loc"))
		return 14;
	else
		return 0;
}

char Commands::getTrustForCommand(string cmd)
{
	if (iequals(cmd, "stop") || iequals(cmd, "off") || iequals(cmd, "quit"))
		return 6;
	else if (iequals(cmd, "test"))
		return 6;
	else if (iequals(cmd, "ping"))
		return 1;
	else if (iequals(cmd, "createAccount"))
		return 1;
	else if (iequals(cmd, "deleteAccount"))
		return 3;
	else if (iequals(cmd, "banAccount"))
		return 1;
	else if (iequals(cmd, "setTrust"))
		return 2;
	else if (iequals(cmd, "banIP"))
		return 1;
	else if (iequals(cmd, "spawn"))
		return 1;
	else if (iequals(cmd, "despawn"))
		return 1;
	else if (iequals(cmd, "nearme") || iequals(cmd, "aroundme"))
		return 1;
	else if (iequals(cmd, "tp") || iequals(cmd, "tele") || iequals(cmd, "teleport"))
		return 0;
	else if (iequals(cmd, "testmap") || iequals(cmd, "map"))
		return 0;
	else if (iequals(cmd, "loc"))
		return 1;
	else
		return 0;
}

char Commands::getTrustForCommand(char id)
{
	if (id == 1)
		return 6;
	if (id == 2)
		return 6;
	if (id == 3)
		return 1;
	if (id == 4)
		return 1;
	if (id == 5)
		return 3;
	if (id == 6)
		return 1;
	if (id == 7)
		return 2;
	if (id == 8)
		return 1;
	if (id == 9)
		return 1;
	if (id == 10)
		return 1;
	if (id == 11)
		return 1;
	if (id == 12)
		return 0;
	if (id == 13)
		return 0;
	if (id == 14)
		return 1;
	else
		return 0;
}

void Commands::performCommand(CommandSender sender, string cmd, vector<string> args)
{
	if (Commands::getCommandID(cmd) != 0)
	{
		if (Accounts::getTrustLevel(Characters::getAccountID(sender.getSenderID())) >= Commands::getTrustForCommand(cmd) || sender.getSenderID() == -1)
		{
			if (Commands::getCommandID(cmd) == 1) // /stop
			{
				ServerLoop::stop();
			}

			else if (Commands::getCommandID(cmd) == 2)
			{
				if (args.size() == 1)
				{
					Scheduler::cancelTask(stoul(args.at(0)));
				}
			}

			else if (Commands::getCommandID(cmd) == 3) // /ping <string:address> <unsigned short:port>
			{
				if (sender.getSenderID() != -1)
				{
					if (args.size() == 2)
					{
						string address = args.at(0);
						string strPort = args.at(1);
						unsigned short port = 0;

						if (Validate::isValidS32(strPort))
						{
							long p = stol(strPort);

							if (p > 0 && p <= USHRT_MAX)
							{
								port = p;
							}
						}

						if (port > 0)
						{
							SystemAddress clientAddress = sender.getClientAddress();

							sender.sendMessage("Trying to reach " + address + ":" + to_string(port) + "...");

							async([address, port, clientAddress]() {
								bool b = PingTool::ping(address.c_str(), port, "3.25 ND1", 8);

								Session* session = Sessions::getSession(clientAddress);
								if (session != nullptr)
								{
									Chat::sendChatMessage(L"PingResult received!\n{\n   Address: " + to_wstring(address) + L",\n   Port: " + to_wstring(port) + L",\n   IsReachable: " + to_wstring(b) + L"\n}", clientAddress);
								}
							});
						}
						else
							sender.sendMessage("The second argument has to be a valid unsigned short int!");
					}
					else
						sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " <address> <port>");
				}
			}

			else if (Commands::getCommandID(cmd) == 4) // /createAccount <string:username> <string:password>
			{
				if (args.size() == 2)
				{
					string username = args.at(0);
					string password = args.at(1);

					long long accountID = Accounts::getAccountID(username);
					if (accountID == -1)
					{
						Accounts::createAccount(username, sha512(password));
						sender.sendMessage("Successfully created account! (Username: " + username + ")");
					}
					else
						sender.sendMessage("The username '" + username + "' is already in use!");
				}
				else
					sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " <username> <password>");
			}

			else if (Commands::getCommandID(cmd) == 5) // /deleteAccount <string:username>
			{
				if (args.size() == 1)
				{
					string username = args.at(0);

					long long accountID = Accounts::getAccountID(username);
					if (accountID != -1)
					{
						Accounts::deleteAccount(accountID);
						sender.sendMessage("Successfully deleted account! (Username: " + username + ")");
					}
					else
						sender.sendMessage("There is no account with the username '" + username + "'!");
				}
				else
					sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " <username>");
			}

			else if (Commands::getCommandID(cmd) == 6) // /banAccount <string:username> <string:time> <string:reason>
			{
				if (args.size() == 3)
				{
					string username = args.at(0);
					string time = args.at(1);
					string reason = args.at(2);

					long long accountID = Accounts::getAccountID(username);
					if (accountID != -1)
					{
						long long howLong = TimeUtils::parse(time);
						if (howLong != -2)
						{
							Bans::ban(to_string(accountID), Characters::getAccountID(sender.getSenderID()), false, reason, howLong);
							sender.sendMessage(username + " got banned successfully! (Duration: " + Bans::toTimeString(howLong) + ") (Reason: " + reason + ")");

							if (Server::isWorldInstance())
							{
								for (int i = 0; i < Server::getReplicaManager()->GetParticipantCount(); i++)
								{
									SystemAddress participant = Server::getReplicaManager()->GetParticipantAtIndex(i);

									if (ObjectsManager::getObjectBySystemAddress(participant)->characterIndex->account_id == accountID)
									{
										BitStream* response = PacketUtils::createPacketBase(RCT_GENERAL, GENERAL_PACKET_DISCONNECT_NOTIFY);
										response->Write(DisconnectID::DISCONNECT_KICK);
										response->Write((unsigned long)0);
										Server::sendPacket(response, participant);
									}
								}
							}
						}
						else
							sender.sendMessage("This command requires a valid time specification! Example: '/banAccount ExampleUsername 120sec \"Example Reason\"'");
					}
					else
						sender.sendMessage("There is no account with the username '" + username + "'!");
				}
				else
					sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " <username> <time> <reason>");
			}

			else if (Commands::getCommandID(cmd) == 7) // /setTrust <string:username> <char:level>
			{
				if (args.size() == 2)
				{
					string username = args.at(0);
					string trustLevel = args.at(1);

					long long accountID = Accounts::getAccountID(username);
					if (accountID != -1)
					{
						if (Validate::isValidS32(trustLevel))
						{
							long l = stol(trustLevel);
							if (l > 0 && l <= 255)
							{
								Accounts::setTrustLevel(l, accountID);
								sender.sendMessage(username + "'s trust was edited successfully! New trust level " + trustLevel);
							}
							else
								sender.sendMessage("This command requires a valid trust level, under 255! Example: '/setTrust ExampleUsername 0'");
						}
						else
							sender.sendMessage("This command requires a valid numerical trust level! Example: '/setTrust ExampleUsername 0'");
					}
					else
						sender.sendMessage("There is no account with the username '" + username + "'!");
				}
				else
					sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " <username> <time> <reason>");
			}

			else if (Commands::getCommandID(cmd) == 8) // /banAccount <string:ip> <string:time> <string:reason>
			{
				if (args.size() == 3)
				{
					string ip = args.at(0);
					string time = args.at(1);
					string reason = args.at(2);

					if (isValidIPAddress(ip.c_str()))
					{
						long long howLong = TimeUtils::parse(time);
						if (howLong != -2)
						{
							Bans::ban(ip, Characters::getAccountID(sender.getSenderID()), true, reason, howLong);
							sender.sendMessage(ip + " got banned successfully! (Duration: " + Bans::toTimeString(howLong) + ") (Reason: " + reason + ")");

							if (Server::isWorldInstance())
							{
								for (int i = 0; i < Server::getReplicaManager()->GetParticipantCount(); i++)
								{
									SystemAddress participant = Server::getReplicaManager()->GetParticipantAtIndex(i);
									if (string(participant.ToString(false)) == ip)
									{
										BitStream* response = PacketUtils::createPacketBase(RCT_GENERAL, GENERAL_PACKET_DISCONNECT_NOTIFY);
										response->Write(DisconnectID::DISCONNECT_KICK);
										response->Write((unsigned long)0);
										Server::sendPacket(response, participant);
									}
								}
							}
						}
						else
							sender.sendMessage("This command requires a valid time specification! Example: '/banIP 127.0.0.1 120sec \"Example Reason\"'");
					}
					else
						sender.sendMessage("'" + ip + "' is not a valid IP address!");
				}
				else
					sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " <ip> <time> <reason>");
			}

			else if (Commands::getCommandID(cmd) == 9) // /spawn <long:LOT> [string:name]
			{
				if (sender.getSenderID() != -1)
				{
					if (args.size() == 1 || args.size() == 2)
					{
						string strLOT = args.at(0);
						string name = "";

						if (args.size() == 2)
							name = args.at(1);

						if (Validate::isValidS32(strLOT))
						{
							long lot = stol(strLOT);

							ReplicaObject* player = ObjectsManager::getObjectByID(sender.getSenderID());

							Position pos = Position();
							pos.x = player->controllablePhysicsIndex->pos_x;
							pos.y = player->controllablePhysicsIndex->pos_y;
							pos.z = player->controllablePhysicsIndex->pos_z;

							Rotation rot = Rotation();
							rot.x = player->controllablePhysicsIndex->rot_x;
							rot.y = player->controllablePhysicsIndex->rot_y;
							rot.z = player->controllablePhysicsIndex->rot_z;
							rot.w = player->controllablePhysicsIndex->rot_w;

							long long id = Objects::generateObjectID();
							ReplicaObject* other = new ReplicaObject(id, lot, to_wstring(name), 0, pos, rot);
							ObjectsManager::spawnObject(other);

							if (name == "")
							{
								sender.sendMessage("You successfully spawned a new object!\n{\n   LOT: " + to_string(lot) + ",\n   ObjectID: " + to_string(id) + "\n}");
							}
							else
							{
								sender.sendMessage("You successfully spawned a new object!\n{\n   LOT: " + to_string(lot) + ",\n   ObjectID: " + to_string(id) + ",\n   Name: " + name + "\n}");
							}
						}
						else
							sender.sendMessage("The first argument has to be a valid long int!");
					}
					else
						sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " <LOT> [name]");
				}
				else
					sender.sendMessage("You can't use this command here!");
			}

			else if (Commands::getCommandID(cmd) == 10) // /despawn <long long:ObjectID>
			{
				if (sender.getSenderID() != -1)
				{
					if (args.size() == 1)
					{
						string strObjectID = args.at(0);

						if (Validate::isValidS64(strObjectID))
						{
							long long objectID = stoll(strObjectID);

							ReplicaObject* replica = ObjectsManager::getObjectByID(objectID);

							if (replica != nullptr)
							{
								long lot = replica->lot;
								string name = to_string(replica->name);

								ObjectsManager::despawnObject(replica);

								if (name == "")
								{
									sender.sendMessage("You successfully despawned an object!\n{\n   LOT: " + to_string(lot) + ",\n   ObjectID: " + to_string(objectID) + "\n}");
								}
								else
								{
									sender.sendMessage("You successfully despawned an object!\n{\n   LOT: " + to_string(lot) + ",\n   ObjectID: " + to_string(objectID) + ",\n   Name: " + name + "\n}");
								}
							}
							else
								sender.sendMessage("There's no object with ObjectID " + to_string(objectID) + "!");
						}
						else
							sender.sendMessage("The first argument has to be a valid long long int!");
					}
					else
						sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " <ObjectID>");
				}
				else
					sender.sendMessage("You can't use this command here!");
			}

			else if (Commands::getCommandID(cmd) == 11) // /nearme <float:radius>
			{
				if (sender.getSenderID() != -1)
				{
					if (args.size() == 1)
					{
						string strRadius = args.at(0);

						if (Validate::isValidFloat(strRadius))
						{
							float radius = stof(strRadius);

							ReplicaObject* player = ObjectsManager::getObjectByID(sender.getSenderID());

							float min_x = player->controllablePhysicsIndex->pos_x - radius;
							float min_y = player->controllablePhysicsIndex->pos_y - radius;
							float min_z = player->controllablePhysicsIndex->pos_z - radius;

							float max_x = player->controllablePhysicsIndex->pos_x + radius;
							float max_y = player->controllablePhysicsIndex->pos_y + radius;
							float max_z = player->controllablePhysicsIndex->pos_z + radius;

							vector<ReplicaObject*> match = vector<ReplicaObject*>();

							for (int i = 0; i < Server::getReplicaManager()->GetReplicaCount(); i++)
							{
								ReplicaObject* other = (ReplicaObject*)Server::getReplicaManager()->GetReplicaAtIndex(i);

								if (other->clientAddress == UNASSIGNED_SYSTEM_ADDRESS)
								{
									if (other->controllablePhysicsIndex != nullptr)
									{
										ControllablePhysicsIndex* index = other->controllablePhysicsIndex;

										if (index->pos_x >= min_x && index->pos_x <= max_x)
										{
											if (index->pos_y >= min_y && index->pos_y <= max_y)
											{
												if (index->pos_z >= min_z && index->pos_z <= max_z)
												{
													match.push_back(other);
												}
											}
										}
									}

									if (other->simplePhysicsIndex != nullptr)
									{
										SimplePhysicsIndex* index = other->simplePhysicsIndex;

										if (index->pos_x >= min_x && index->pos_x <= max_x)
										{
											if (index->pos_y >= min_y && index->pos_y <= max_y)
											{
												if (index->pos_z >= min_z && index->pos_z <= max_z)
												{
													match.push_back(other);
												}
											}
										}
									}
									if (other->phantomPhysicsIndex != nullptr)
									{
										PhantomPhysicsIndex* index = other->phantomPhysicsIndex;
										if (index->pos_x >= min_x && index->pos_x <= max_x)
										{
											if (index->pos_y >= min_y && index->pos_y <= max_y)
											{
												if (index->pos_z >= min_z && index->pos_z <= max_z)
												{
													match.push_back(other);
												}
											}
										}
									}
								}
							}

							if (match.size() > 0)
							{
								sender.sendMessage("The following objects are around you in a radius of " + strRadius + ":");

								for (int i = 0; i < match.size(); i++)
								{
									if (match.at(i)->name == L"")
									{
										sender.sendMessage("{\n   ListID: " + to_string(i + 1) + ",\n   LOT: " + to_string(match.at(i)->lot) + ",\n   ObjectID: " + to_string(match.at(i)->objectID) + "\n}");
									}
									else
									{
										sender.sendMessage("{\n   ListID: " + to_string(i + 1) + ",\n   LOT: " + to_string(match.at(i)->lot) + ",\n   ObjectID: " + to_string(match.at(i)->objectID) + ",\n   Name: " + to_string(match.at(i)->name) + "\n}");
									}
								}
							}
							else
								sender.sendMessage("There are no objects around you in a radius of " + strRadius + "!");
						}
						else
							sender.sendMessage("The first argument has to be a valid float!");
					}
					else
						sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " <radius>");
				}
				else
					sender.sendMessage("You can't use this command here!");
			}

			else if (Commands::getCommandID(cmd) == 12) // /tp (<string:player> | <float:x> <float:y> <float:z>)
			{
				if (sender.getSenderID() != -1)
				{
					if (args.size() == 1 || args.size() == 3)
					{
						if (args.size() == 1)
						{
							string name = args.at(0);

							ReplicaObject* f = nullptr;

							for (int i = 0; i < Server::getReplicaManager()->GetReplicaCount(); i++)
							{
								ReplicaObject* other = (ReplicaObject*)Server::getReplicaManager()->GetReplicaAtIndex(i);

								if (other->clientAddress != UNASSIGNED_SYSTEM_ADDRESS)
								{
									if (other->name == to_wstring(name))
										f = other;
								}
							}

							if (f != nullptr)
							{
								Position pos = Position();
								pos.x = f->controllablePhysicsIndex->pos_x;
								pos.y = f->controllablePhysicsIndex->pos_y;
								pos.z = f->controllablePhysicsIndex->pos_z;

								Rotation rot = Rotation();
								rot.x = f->controllablePhysicsIndex->rot_x;
								rot.y = f->controllablePhysicsIndex->rot_y;
								rot.z = f->controllablePhysicsIndex->rot_z;
								rot.w = f->controllablePhysicsIndex->rot_w;

								for (int i = 0; i < Server::getReplicaManager()->GetParticipantCount(); i++)
								{
									SystemAddress participant = Server::getReplicaManager()->GetParticipantAtIndex(i);

									GameMessages::teleport(sender.getSenderID(), false, false, true, true, pos, participant, rot);
								}

								sender.sendMessage("You magically appeared at " + to_string(f->name) + "'s location!");
							}
							else
							{
								name = toLowerCase(name);

								vector<ReplicaObject*> match = vector<ReplicaObject*>();

								for (int i = 0; i < Server::getReplicaManager()->GetReplicaCount(); i++)
								{
									ReplicaObject* other = (ReplicaObject*)Server::getReplicaManager()->GetReplicaAtIndex(i);

									if (other->clientAddress != UNASSIGNED_SYSTEM_ADDRESS)
									{
										if (startsWith(toLowerCase(to_string(other->name)), name))
										{
											match.push_back(other);
										}
									}
								}

								if (match.size() == 0)
								{
									sender.sendMessage("Player not found!");
								}
								else if (match.size() == 1)
								{
									f = match.at(0);

									Position pos = Position();
									pos.x = f->controllablePhysicsIndex->pos_x;
									pos.y = f->controllablePhysicsIndex->pos_y;
									pos.z = f->controllablePhysicsIndex->pos_z;

									Rotation rot = Rotation();
									rot.x = f->controllablePhysicsIndex->rot_x;
									rot.y = f->controllablePhysicsIndex->rot_y;
									rot.z = f->controllablePhysicsIndex->rot_z;
									rot.w = f->controllablePhysicsIndex->rot_w;

									for (int i = 0; i < Server::getReplicaManager()->GetParticipantCount(); i++)
									{
										SystemAddress participant = Server::getReplicaManager()->GetParticipantAtIndex(i);

										GameMessages::teleport(sender.getSenderID(), false, false, true, true, pos, participant, rot);
									}

									sender.sendMessage("You magically appeared at " + to_string(f->name) + "'s location!");
								}
								else
								{
									sender.sendMessage(to_string(match.size()) + " players found! Please be a bit more specific!");
								}
							}
						}

						if (args.size() == 3)
						{
							string strX = args.at(0);
							string strY = args.at(1);
							string strZ = args.at(2);

							if (Validate::isValidFloat(strX) && Validate::isValidFloat(strY) && Validate::isValidFloat(strZ))
							{
								Position pos = Position();
								pos.x = stof(strX);
								pos.y = stof(strY);
								pos.z = stof(strZ);

								for (int i = 0; i < Server::getReplicaManager()->GetParticipantCount(); i++)
								{
									SystemAddress participant = Server::getReplicaManager()->GetParticipantAtIndex(i);

									GameMessages::teleport(sender.getSenderID(), false, false, false, true, pos, participant);
								}

								sender.sendMessage("You magically appeared at the specified location!");
							}
							else
								sender.sendMessage("All 3 arguments have to be valid floats!");
						}
					}
					else
						sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " (<player> | <x> <y> <z>)");
				}
				else
					sender.sendMessage("You can't use this command here!");
			}

			else if (Commands::getCommandID(cmd) == 13) // /testmap (<unsigned short:ZoneID> | <string:ZoneName>)
			{
				if (sender.getSenderID() != -1)
				{
					if (args.size() == 1)
					{
						string strZoneID = args.at(0);

						ZoneID zoneID = ZoneIDs::fromString(strZoneID);

						if (zoneID != ZoneID::INVALID_ZONE_ID)
						{
							if (zoneID != ServerRoles::toZoneID(Server::getServerRole()))
							{
								SystemAddress clientAddress = sender.getClientAddress();
								string address = Config::getWorldInstanceAddress(zoneID);
								unsigned short port = Config::getWorldInstancePort(zoneID);

								sender.sendMessage("Trying to reach " + ServerRoles::toString(ServerRoles::fromZoneID(zoneID)) + "...");

								async([address, port, zoneID, clientAddress]()
								{
									bool b = PingTool::ping(address.c_str(), port, "3.25 ND1", 8);

									if (b)
									{
										Session* session = Sessions::getSession(clientAddress);
										if (session != nullptr)
										{
											Chat::sendChatMessage(L"Alright! Let's go to " + to_wstring(ZoneIDs::toPrintString(zoneID)) + L"!", clientAddress);

											if (zoneID != ZoneID::INVALID_ZONE_ID)
											{
												Location loc;

												loc.zoneID = zoneID;
												loc.mapClone = 0;

												Position spp = LUZCache::getByZoneID(loc.zoneID)->spawnPointPos;
												Rotation spr = LUZCache::getByZoneID(loc.zoneID)->spawnPointRot;

												loc.position.x = spp.x;
												loc.position.y = spp.y;
												loc.position.z = spp.z;
												loc.rotation.x = spr.x;
												loc.rotation.y = spr.y;
												loc.rotation.z = spr.z;
												loc.rotation.w = spr.w;

												Locations::saveLocation(loc, session->charID);
											}

											TransitionInfos::insertTransitionInfo(session->clientAddress, session->accountID, session->charID, session->transitionKey);
											General::redirectToServer(session->clientAddress, address, port, false);
											session->gotRedirected = true;
										}
									}
									else
									{
										Chat::sendChatMessage(L"The instance you want to connect to is not available at the moment!\nPlease try again later.", clientAddress);
									}
								});
							}
							else
								sender.sendMessage("You already are in/on " + ZoneIDs::toPrintString(zoneID) + "!");
						}
						else
							sender.sendMessage("The input '" + strZoneID + "' is not a valid ZoneID or ZoneName!");
					}
					else
						sender.sendMessage("Invalid parameters! Syntax: /" + cmd + " (<ZoneID> | <ZoneName>)");
				}
				else
					sender.sendMessage("You can't use this command here!");
			}

			else if (Commands::getCommandID(cmd) == 14) { /* This is handled on the client side. */ }

			else
				sender.sendMessage("Please report the following ID: " + to_string(Commands::getCommandID(cmd)));
		}
		else
			sender.sendMessage("You don't have permission to use this command!");
	}
	else
		sender.sendMessage("The input '" + cmd + "' is not a valid command!");
}

CommandSender::CommandSender(long long senderID, SystemAddress clientAddress)
{
	this->senderID = senderID;
	this->clientAddress = clientAddress;
}

void CommandSender::sendMessage(string msg)
{
	if (senderID == -1)
	{
		cout << msg << endl;
	}
	else
	{
		Chat::sendChatMessage(to_wstring(msg), clientAddress);
	}
}

long long CommandSender::getSenderID()
{
	return senderID;
}

SystemAddress CommandSender::getClientAddress()
{
	return clientAddress;
}
