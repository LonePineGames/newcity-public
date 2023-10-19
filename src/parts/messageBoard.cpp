#include "messageBoard.hpp"

#include "../cup.hpp"
#include "../economy.hpp"
#include "../icons.hpp"

#include "../parts/blank.hpp"
#include "../parts/button.hpp"
#include "../parts/part.hpp"
#include "../parts/panel.hpp"
#include "../parts/scrollbox.hpp"

#include "spdlog/spdlog.h"

#include "messageBoardTypes.cpp"

static Cup<Message> messages_o;
static ScrollState messageScroll;
static bool messageBoardVisible = true;

MessageCallback messageCallback[numMessageTypes] = {
  achievementMessage, personMessage, businessMessage, buildingMessage,
  graphMessage, vehicleMessage, infoMessage, budgetMessage, chartMessage,
  zoneDemandMessage, amenityEffectMessage, transitDesignMessage,
  newspaperMessage, finNewsMessage,
};

void addMessage(MessageType type, item object, item data) {
  removeMessageByObject(type, object);
  messageScroll.amount = 0;

  messageBoardVisible = true;
  Message m;
  m.type = type;
  m.object = object;
  m.data = data;
  messages_o.push_back(m);
}

void addMessage(MessageType type, item object) {
  addMessage(type, object, 0);
}

// Add a message of a given type and object to the board
// For messages like ZoneDemand, object == 0 (null)
void toggleMessage(MessageType type, item object, item data) {
  SPDLOG_INFO("toggleMessage {} {} {}",
      type, object, data);
  bool removed = false;
  for (int i = messages_o.size() - 1; i >= 0; i--) {
    if (messages_o[i].type == type && (messages_o[i].object == object ||
      (type == ChartMessage &&
         abs(messages_o[i].object)/10 == abs(object)/10)) &&
        messages_o[i].data == data) {
      messages_o.remove(i);
      removed = true;
    }
  }
  if (removed) return;

  messageBoardVisible = true;
  messageScroll.amount = 0;
  Message m;
  m.type = type;
  m.object = object;
  m.data = data;
  messages_o.push_back(m);
}

void toggleMessage(MessageType type, item object) {
  item data = 0;
  if (type == ChartMessage) data = ourCityEconNdx();
  toggleMessage(type, object, data);
}

void removeMessage(item ndx) {
  messages_o.remove(ndx);
}

void removeMessageByObject(MessageType type, item object) {
  for (int i = messages_o.size() - 1; i >= 0; i--) {
    if (messages_o[i].type == type && messages_o[i].object == object) {
      messages_o.remove(i);
    }
  }
}

bool hasMessage(MessageType type, item object, item data) {
  for (int i = 0; i < messages_o.size(); i++) {
    if (messages_o[i].type == type && messages_o[i].object == object &&
        data == messages_o[i].data) {
      return true;
    }
  }
  return false;
}

bool hasMessage(MessageType type, item object) {
  for (int i = 0; i < messages_o.size(); i++) {
    if (messages_o[i].type == type && messages_o[i].object == object) {
      return true;
    }
  }
  return false;
}

bool removeMessage(Part* part, InputEvent event) {
  removeMessage(part->itemData);
  return true;
}

bool minimizeMessage(Part* part, InputEvent event) {
  item ndx = part->itemData;
  Message msg = messages_o[ndx];
  msg.object *= -1;
  messages_o.set(ndx, msg);
  return true;
}

Part* messageBoard(float xSize, float ySize) {
  if (!messageBoardVisible) {
    return blank();
  }

  float usableYSize = ySize - 2;//8;
  Part* scroll = scrollbox(vec2(0,0), vec2(messageWidthOuter, usableYSize));
  scroll->padding = 0;

  float y = 0.0f;
  for (int i = messages_o.size() - 1; i >= 0; i--) {
    Message m = messages_o[i];

    Part* messagePart = panel(line(vec3(0.6f,y,0.0f),
          vec3(messageWidthOuter, 1.0f, 0.0f)));
    messagePart->padding = messagePadding;
    messagePart->itemData = m.object;

    messageCallback[m.type](messagePart, m);

    if (m.type == ChartMessage) {
      float xclose = messageWidthOuter-msgScl;
      Part* target = messagePart->contents[0];
        //m.object < 0 ? messagePart : messagePart->contents[0];
      r(target, button(vec2(xclose, messagePadding),
          iconX, vec2(msgScl, msgScl), removeMessage, i));
      r(target, button(vec2(xclose-msgScl, messagePadding),
          m.object < 0 ? iconPlus : iconMinus,
          vec2(msgScl, msgScl), minimizeMessage, i));

    } else {
      r(messagePart, button(vec2(messageWidthOuter-msgScl-messagePadding,0),
            iconX, vec2(msgScl, msgScl), removeMessage, i));
      messagePart->dim.end.y += messagePadding*2;
    }

    r(scroll, messagePart);
    y += messagePart->dim.end.y + 0.2f;
    messagePart->dim.end.z += messageScroll.amount;
  }

  Part* scrollFrame = scrollboxFrame(
      vec2(xSize-messageWidthOuter-0.6, 1.5-messagePadding),
      vec2(messageWidthOuter+messagePadding+1, usableYSize+messagePadding*2),
      &messageScroll, scroll, true);
  scrollFrame->renderMode = RenderTransparent;
  scrollFrame->flags &= ~_partLowered;

  return scrollFrame;
}

void toggleMessageBoard() {
  messageBoardVisible = !messageBoardVisible;
}

bool isMessageBoardVisible() {
  return messageBoardVisible;
}

void resetMessages() {
  messages_o.clear();
}

void writeMessages(FileBuffer* file) {
  messages_o.write(file);
}

void readMessage(FileBuffer* file, int ndx) {
  Message* m = messages_o.get(ndx);
  m->type = (MessageType) fread_int(file);
  m->object = fread_int(file);
  if (file->version >= 53) {
    m->data = fread_int(file);
  } else {
    m->data = 0;
  }

  if (m->type == ChartMessage && m->data == 0) m->data = ourCityEconNdx();
  if (m->type == AmenityEffectMessage && m->object == 0 &&
      (file->version < 58 || (file->version == 58 && file->patchVersion < 15))) {
    m->object = EducationEffect;
  }

  SPDLOG_INFO("readMessage type{} object{} data{}",
      m->type, m->object, m->data);
}

void readMessages(FileBuffer* file, int version) {
  if (version >= 29) {
    int num = fread_int(file);
    messages_o.resize(num);
    for (int i = 0; i < num; i++) {
      readMessage(file, i);
    }
  } else {
    messages_o.clear();
  }
}

