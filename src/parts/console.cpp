#include "console.hpp"

const char* conDefText = "Enter command...";
const int conDimZ = 14;

static bool conClicked = false;
static bool conFocusText = false;
static float lastMLSize = 0;
static vec2 conSourceMousePos = vec2(0, 0);
static ScrollState conScrollState;
static TextBoxState conTextBoxState;
uint32_t conCursorIndex = 0;
std::string conTextDisplay = strdup_s("");
std::string conTextEntry = strdup_s("");

bool consoleClose(Part* part, InputEvent event) {
  consoleSetOpen(false);
  return true;
}

bool consoleEntryClick(Part* part,InputEvent event) {
  if(!consoleIsOpen()) {
    return false;
  }

  if(event.isButtonDown[0] && !isInDim(event.mouseLoc,part->dim)) {
      conFocusText = false;
      return false;
  }

  conFocusText = true;
  return true;
}

bool consoleEntryKey(Part* part,InputEvent event) {
  if(!consoleIsOpen()) {
    return false;
  }

  int len = conTextEntry.length();

  // Do action key check first
  // If we're pressing the ActToggleConsole or ActRootEscape key, close console
  bool consolePressed = (event.key == getKeyBind((int)InputAction::ActToggleConsole).key 
    && getKeyBind((int)InputAction::ActToggleConsole).active);
  bool escapePressed = (event.key == getKeyBind((int)InputAction::ActRootEscape).key
    && getKeyBind((int)InputAction::ActRootEscape).active);
  if (consolePressed || escapePressed) {
    consoleToggle();
    return true;
  }

  switch(event.key) {
    case GLFW_KEY_BACKSPACE:
      if(len == 0) {
        return true;
      }
      if(conCursorIndex > 0) {
        conCursorIndex--;
      }
      conTextEntry.erase(conCursorIndex,1);
      return true;
    case GLFW_KEY_DELETE:
      if(len == 0) {
        return true;
      }
      if(conCursorIndex < conTextEntry.length()) {
        conTextEntry.erase(conCursorIndex,1);
      }
      return true;
    case GLFW_KEY_UP:
      if(consoleTrySetInputMemIndex(consoleGetInputMemIndex()-1)) {
        conTextEntry = consoleGetInputMemAtIndex();
        conCursorIndex = conTextEntry.length();
      }
      return true;
    case GLFW_KEY_DOWN:
      if(consoleTrySetInputMemIndex(consoleGetInputMemIndex()+1)) {
        conTextEntry = consoleGetInputMemAtIndex();
        conCursorIndex = conTextEntry.length();
      }
      return true;
    case GLFW_KEY_LEFT:
      if(conCursorIndex > 0) {
        conCursorIndex--;
      }
      return true;
    case GLFW_KEY_RIGHT:
      if(conCursorIndex < len) {
        conCursorIndex++;
      }
      return true;
    case GLFW_KEY_ENTER:
      if(conTextEntry.length() == 0) {
        return true;
      }
      int status = consoleParseInput(conTextEntry);
      if(status != 0) {
        consolePrintError(consoleGetError((ConStatus)status),conTextEntry);
      }
      conTextEntry.clear();
      conCursorIndex = 0;
      return true;
  }

  return false;
}

bool consoleEntryText(Part* part, InputEvent event) {
  if(event.key == GLFW_KEY_GRAVE_ACCENT
    || event.key == GLFW_KEY_ESCAPE) {
    return true;
  }

  if(event.unicode > 0) {
    conTextEntry.insert(conCursorIndex,sprintf_o("%c",(char)event.unicode));
    conCursorIndex++;
  }
  return true;
}

bool consolePanelClick(Part* part, InputEvent event) {
  if(!consoleIsOpen()) {
    return false;
  }

  if(!event.isButtonDown[0]) {
    return false;
  }

  if(!conClicked) {
    conClicked = true;
    conSourceMousePos = transformMouseLoc(event.cameraSpaceMouseLoc);
  }

  return true;
}

bool consolePanelDrag(Part* part, InputEvent event) {
  if(!conClicked) {
    return false;
  }

  if(!event.isButtonDown[0]) {
    conClicked = false;
    return false;
  }

  vec2 newMousePos = transformMouseLoc(event.cameraSpaceMouseLoc);
  if(newMousePos != conSourceMousePos) {
    vec2 offset = conSourceMousePos - consolePos();
    conSourceMousePos = newMousePos;
    consoleSetPos(newMousePos - offset);
  }

  return true;
}

Part* console() {
  float aspectRatio = getAspectRatio();
  float center = uiGridSizeX * aspectRatio * .5f;
  float conWidth = center-9;
  float conHeight = 9.0f;
  vec2 conPos = vec2(center,uiGridSizeX-conHeight);
  vec2 conSize = vec2(conWidth, conHeight);

  Part* conPanel = panel(conPos, conSize);
  conPanel->itemData = CONSOLE_VAL;
  conPanel->dim.start.z = conDimZ;
  conPanel->dim.end.z = conDimZ;

  vec2 scrollStart = vec2(0,0);
  vec2 scrollSize = vec2(conWidth, conSize.y-conScaleText);

  float mlSize = 0.0f;
  Part* conML = multiline(vec2(0,0),vec2(conWidth, conScaleText),
    strdup_s(consoleGetText().c_str()), &mlSize);
  conML->itemData = CONSOLE_VAL;

  //float yEnd = scrollSize.y+(conScaleText*mlSize);

  //conML->dim.start.y -= mlSize-(conScaleText*consoleGetNumLines())-conPad*2;
  conML->dim.start.z = conDimZ;
  conML->dim.end.z = conDimZ;

  Part* conScroll = scrollbox(vec2(0, 0),vec2(conWidth, scrollSize.y));
  conScroll->itemData = CONSOLE_VAL;
  conScroll->dim.start.z = conDimZ;
  conScroll->dim.end.z = conDimZ;

  r(conScroll, conML);

  Part* conTextBox = textBox(vec2(0,scrollSize.y),
    vec2(conWidth, conScaleText), strdup_s(conTextEntry.c_str()),
    conCursorIndex);
  conTextBox->onClick = consoleEntryClick;
  conTextBox->onKeyDown = consoleEntryKey;
  conTextBox->onText = consoleEntryText;
  conTextBox->itemData = CONSOLE_VAL;
  conTextBox->dim.start.z = conDimZ;
  conTextBox->dim.end.z = conDimZ;

  r(conPanel, conTextBox);

  r(conPanel, scrollboxFrame(scrollStart, scrollSize,
    &conScrollState, conScroll));

  if (mlSize != lastMLSize) {
    conScrollState.target = conScrollState.maxTarget;
    lastMLSize = mlSize;
  }

  return conPanel;
}

Part* console(vec2 pos) {
  consoleSetPos(pos);
  return console();
}
