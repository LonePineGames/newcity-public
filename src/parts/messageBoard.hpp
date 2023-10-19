#pragma once

#include "../serialize.hpp"

#include "../parts/part.hpp"

const float messagePadding = 0.1;
const float messageWidthOuter = 8+messagePadding*2;
const float messageWidth = messageWidthOuter-messagePadding*2;
const float msgScl = 0.75;

enum MessageType {
  AchievementMessage, PersonMessage, BusinessMessage, BuildingMessage,
  GraphMessage, VehicleMessage, InfoMessage, BudgetMessage, ChartMessage,
  ZoneDemandMessage, AmenityEffectMessage, TransitDesignMessage,
  NewspaperMessage, FinNewsMessage,
  numMessageTypes
};

struct Message {
  MessageType type;
  item object;
  item data;
};

enum InfoMessageID {
  LogUploadNotification,
  numInfoMessages
};

typedef void (*MessageCallback)(Part* result, Message message);

void addMessage(MessageType type, item object);
void addMessage(MessageType type, item object, item data);
void toggleMessage(MessageType type, item object);
void toggleMessage(MessageType type, item object, item data);
bool hasMessage(MessageType type, item object);
bool hasMessage(MessageType type, item object, item data);
void removeMessage(item ndx);
void removeMessageByObject(MessageType type, item object);
void promoteNewspaperMessage();

Part* messageBoard(float xSize, float ySize);
void toggleMessageBoard();
bool isMessageBoardVisible();

void resetMessages();
void readMessages(FileBuffer* file, int version);
void writeMessages(FileBuffer* file);

