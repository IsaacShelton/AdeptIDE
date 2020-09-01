
#ifndef ADEPTIDE_H_INCLUDED
#define ADEPTIDE_H_INCLUDED

#include <vector>

#include "INTERFACE/TextEditor.h"
#include "INTERFACE/Assets.h"
#include "INTERFACE/MenuBar.h"
#include "INTERFACE/Message.h"
#include "INTERFACE/Settings.h"
#include "INTERFACE/Explorer.h"
#include "INTERFACE/FileLooker.h"
#include "INTERFACE/LineNavigator.h"
#include "INTERFACE/CommandRunner.h"
#include "INTERFACE/SymbolNavigator.h"
#include "INTERFACE/Finder.h"
#include "INTERFACE/Terminal.h"
#include "INTERFACE/PopUp.h"
#include "PROCESS/FolderWatcher.h"

class AdeptIDE : public AdeptIDEAssets { // Inheritance not symbolic
private:
    Font font;
    FolderWatcher rootWatcher;
    std::string importFolderLocation;

    void handleInput();
    void update();
    void renderEditorFilenames();
    void renderEmblem();
    void renderCurrentEditor();
    void loadSettings();
    void updateImportFolder();

public:
    std::string root;
    GLFWwindow *window;
    int width, height;
    Matrix4f projectionMatrix;
    Matrix4f transformationMatrix;
    Settings settings;
    
    MenuBar menubar;
    std::vector<GenericEditor*> editors;
    size_t currentEditorIndex;

    bool mouseReleased;
    float mouseDownX, mouseDownY, mouseDownNetXOffset, mouseDownNetYOffset;
    size_t mouseDownCaretPosition;

    std::string assetsFolder;
    Shader *shader, *fontShader, *solidShader;

    Message *message;
    Explorer *explorer;
    Terminal *terminal;
    FileLooker *fileLooker;
    LineNavigator *lineNavigator;
    CommandRunner *commandRunner;
    SymbolNavigator *symbolNavigator;
    Finder *finder;
    PopUp *popup;

    AdeptIDE();
    ~AdeptIDE();
    int main(int argc, const char **argv);

    void openFile();
    void openFolder();
    void openEditor(const std::string& filename);
    void newFile(FileType fileType);
    GenericEditor* getCurrentEditor();
    TextEditor *getCurrentEditorAsTextEditor();
    ImageEditor *getCurrentEditorAsImageEditor();
    TextEditor* addTextEditor();
    ImageEditor* addImageEditor();
    void removeEditor(size_t index);
    void removeCurrentEditor();
    void moveToPreviousEditorTab();
    void moveToNextEditorTab();
    void updateTitle();
    void setCurrentEditor(size_t index);
    void updateInsight(bool showSuccessMessage = false);

    void type(const std::string& text);
    void type(char character);
    void typeBlock();
    void typeExpression();
    void typeArrayAccess();
    void typeString();
    void typeCString();
    void backspace();
    void del();
    void smartBackspace();
    void smartDel();
    void backspaceLine();
    void delLine();
    void startSelection();
    void endSelection();
    void destroySelection();
    void deleteSelected();
    void copySelected();
    void cutSelected();
    void paste();
    void tab();
    void nextLine();
    void nextPrecedingLine();
    void finishSuggestion();

    void moveCaret(double xpos, double ypos);
    void moveCaretLeft();
    void moveCaretRight();
    void moveCaretUp();
    void moveCaretDown();
    void moveCaretBeginningOfLine();
    void moveCaretEndOfLine();
    void moveCaretBeginningOfWord();
    void moveCaretEndOfWord();
    void moveCaretBeginningOfSubWord();
    void moveCaretEndOfSubWord();
    void moveCaretOutside();
    void scrollDown(int lineCount);
    void scrollUp(int lineCount);
    void pageUp();
    void pageDown();
    void selectAll();
    void selectLine();

    void duplicateCaretUp();
    void duplicateCaretDown();

    void saveFile();
    void runFile();

    void lookForFile();
    void gotoLine();
    void runCommand();
    void gotoSymbol();
    void findInFile();
    bool cdFile();

    void hideAnyTextBars();
    void hideAnyTextBarsExcept(TextBar *textbar);

    void createMessage(const std::string& message, double seconds);
    void createPopUp(const std::string& message, PopUp::Kind kind);

    void handleEnterKey(int mods);
    void handleEscapeKey(int mods);
};

void scroll_callback(GLFWwindow *window, double xOffset, double yOffset);

void character_callback(GLFWwindow *window, unsigned int codepoint);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void handle_left_click(AdeptIDE *adeptide, double xpos, double ypos);

void drop_callback(GLFWwindow *window, int count, const char **paths);

void file_menu(void *data);
void view_menu(void *data);
void selection_menu(void *data);
void build_menu(void *data);
void help_menu(void *data);

void new_adept_file(void *data);
void new_plain_text_file(void *data);
void new_java_file(void *data);
void new_json_file(void *data);
void new_painting_file(void *data);

void open_playground_menu(void *data);
void open_file_menu(void *data);
void open_folder_menu(void *data);
void save_file_menu(void *data);
void close_file_menu(void *data);
void settings_menu(void *data);
void quit_menu(void *data);

void select_all_menu(void *data);

void language_plain(void *data);
void language_adept(void *data);
void language_java(void *data);
void language_html(void *data);

void theme_visual_studio(void *data);
void theme_fruit_smoothie(void *data);
void theme_tropical_ocean(void *data);
void theme_island_campfire(void *data);
void theme_one_dark(void *data);

void typing_tips(void *data);
void about_menu(void *data);

void toggle_explorer(void *data);
void toggle_terminal(void *data);
void maximize(void *data);

void build_adept_project(void *data);
void build_and_run_adept_project(void *data);

double distance(double x1, double y1, double x2, double y2);

#ifdef __APPLE__
#define QUICKTYPE_KEY GLFW_KEY_LEFT_SUPER
#define CMDCTRL_MOD(a) (a & GLFW_MOD_SUPER || a & GLFW_MOD_CONTROL)
#else
#define QUICKTYPE_KEY GLFW_KEY_RIGHT_ALT
#define CMDCTRL_MOD(a) (a & GLFW_MOD_CONTROL)
#endif

#endif // ADEPTIDE_H_INCLUDED
